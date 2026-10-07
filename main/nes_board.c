#include "nes_board.h"
#include "InfoNES.h"
#include "nes_game.h"
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_ts.h"

volatile uint32_t nes_boot_stage;
volatile uint32_t nes_error_code;
volatile uint32_t nes_touch_ok;
volatile uint32_t nes_frame_count;
volatile uint32_t nes_present_us, nes_present_max_us, nes_touch_us, nes_touch_max_us;

typedef struct {
    uint16_t x, y, w, h;
    uint8_t mask;
    const char *label;
} PadButton;
static const PadButton buttons[] = {
    {48, 148, 48, 60, 0x10, "UP"},
    { 0, 212, 48, 60, 0x40, "<"},
    {96, 212, 48, 60, 0x80, ">"},
    {48, 276, 48, 60, 0x20, "DN"},
    {12, 386,120, 54, 0x04, "SELECT"},
    {668,150,120, 70, 0x02, "B"},
    {668,238,120, 70, 0x01, "A"},
    {668,386,120, 54, 0x08, "START"}
};
_Static_assert(NES_FRAMEBUFFER_ADDR == 0xC0000000UL && NES_LCD_WIDTH * NES_LCD_HEIGHT * 4 <= 0x200000UL, "Framebuffer must fit MPU window");
_Static_assert(NES_GAME_INDEX >= 0 && NES_GAME_INDEX < GAME_FILE_NUM, "Invalid NES_GAME_INDEX");
_Static_assert(NES_DISP_WIDTH * NES_DISPLAY_SCALE <= NES_LCD_WIDTH, "Frame too wide");
_Static_assert(NES_DISP_HEIGHT * NES_DISPLAY_SCALE <= NES_LCD_HEIGHT, "Frame too tall");
_Static_assert(NES_FRAMEBUFFER_ADDR + NES_LCD_WIDTH * NES_LCD_HEIGHT * 4 <= NES_WORK_FRAME_ADDR, "Framebuffers overlap");
_Static_assert(NES_WORK_FRAME_ADDR + NES_DISP_WIDTH * NES_DISP_HEIGHT * 2 <= NES_SDRAM_TEST_ADDR, "SDRAM test overlaps frame");

static void DrawControls(void)
{
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
    for (unsigned i = 0; i < sizeof buttons / sizeof buttons[0]; ++i) {
        const PadButton *b = &buttons[i];
        BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
        BSP_LCD_FillRect(b->x + 2, b->y + 2, b->w - 4, b->h - 4);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayStringAt(b->x + 8, b->y + b->h / 2 - 8,
                               (uint8_t *)b->label, LEFT_MODE);
    }
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(12, 24, (uint8_t *)"NES F769", LEFT_MODE);
    BSP_LCD_DisplayStringAt(668, 24, (uint8_t *)"USER", LEFT_MODE);
    BSP_LCD_DisplayStringAt(668, 46, (uint8_t *)"= START", LEFT_MODE);
}
static int SdramTest(void)
{
    volatile uint32_t *mem = (volatile uint32_t *)NES_SDRAM_TEST_ADDR;
    for (uint32_t i = 0; i < 256; ++i) mem[i] = 0xA55A0000UL ^ i;
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)NES_SDRAM_TEST_ADDR, 1024);
    __DSB();
    for (uint32_t i = 0; i < 256; ++i) if (mem[i] != (0xA55A0000UL ^ i)) return 0;
    for (uint32_t i = 0; i < 256; ++i) mem[i] = ~(0xA55A0000UL ^ i);
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)NES_SDRAM_TEST_ADDR, 1024);
    __DSB();
    for (uint32_t i = 0; i < 256; ++i) if (mem[i] != ~(0xA55A0000UL ^ i)) return 0;
    return 1;
}
void NES_BoardInit(void)
{
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);
    nes_boot_stage = 3;
    /* ST BSP initializes DSI video mode, LTDC, DMA2D and external SDRAM. */
    if (BSP_LCD_InitEx(LCD_ORIENTATION_LANDSCAPE) != LCD_OK) NES_BoardFatal(3);
    nes_boot_stage = 4;
    if (!SdramTest()) NES_BoardFatal(4);
    BSP_LCD_LayerDefaultInit(0, NES_FRAMEBUFFER_ADDR);
    BSP_LCD_SelectLayer(0);
    BSP_LCD_SetLayerVisible(1, DISABLE);
    BSP_LCD_SetLayerVisible(0, ENABLE);
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_DisplayOn();
    nes_boot_stage = 5;
    nes_touch_ok = (BSP_TS_Init(NES_LCD_WIDTH, NES_LCD_HEIGHT) == TS_OK);
    DrawControls();
    if (!nes_touch_ok) {
        BSP_LED_On(LED_RED);
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(12, 70, (uint8_t *)"TS ERROR", LEFT_MODE);
    }
    BSP_LED_On(LED_GREEN);
}
void NES_BoardPresent(const volatile WORD *frame)
{
    uint32_t started = DWT->CYCCNT;
    const uint32_t x0 = (NES_LCD_WIDTH - NES_DISP_WIDTH * NES_DISPLAY_SCALE) / 2;
    const uint32_t y0 = (NES_LCD_HEIGHT - NES_DISP_HEIGHT * NES_DISPLAY_SCALE) / 2;
    volatile uint32_t *fb = (volatile uint32_t *)NES_FRAMEBUFFER_ADDR;
    for (uint32_t y = 0; y < NES_DISP_HEIGHT; ++y) {
        for (uint32_t x = 0; x < NES_DISP_WIDTH; ++x) {
            uint32_t c = frame[y * NES_DISP_WIDTH + x];
            /* Same RGB555 palette conversion as the tested F429 port. */
            uint32_t rgb = 0xFF000000UL | ((c & 0x7C00U) << 9) |
                           ((c & 0x03E0U) << 6) | ((c & 0x001FU) << 3);
            uint32_t pos = (y0 + y * NES_DISPLAY_SCALE) * NES_LCD_WIDTH + x0 + x * NES_DISPLAY_SCALE;
            for (uint32_t dy = 0; dy < NES_DISPLAY_SCALE; ++dy)
                for (uint32_t dx = 0; dx < NES_DISPLAY_SCALE; ++dx)
                    fb[pos + dy * NES_LCD_WIDTH + dx] = rgb;
        }
    }
    __DSB();
    nes_present_us = (DWT->CYCCNT - started) / (SystemCoreClock / 1000000U);
    if (nes_present_us > nes_present_max_us) nes_present_max_us = nes_present_us;
    ++nes_frame_count;
    if ((nes_frame_count % 30U) == 0) BSP_LED_Toggle(LED_GREEN);
}
uint8_t NES_BoardReadPad(void)
{
    uint32_t started = DWT->CYCCNT;
    static uint32_t last_poll, changed_at;
    static uint8_t touch_mask, previous_raw, stable_user;
    uint32_t now = HAL_GetTick();
    uint8_t raw = (BSP_PB_GetState(BUTTON_USER) != 0);
    if (raw != previous_raw) { previous_raw = raw; changed_at = now; }
    if ((uint32_t)(now - changed_at) >= 20U) stable_user = raw;
    if (nes_touch_ok && (uint32_t)(now - last_poll) >= 5U) {
        TS_StateTypeDef ts = {0};
        last_poll = now;
        touch_mask = 0;
        if (BSP_TS_GetState(&ts) == TS_OK) {
            for (unsigned t = 0; t < ts.touchDetected && t < TS_MAX_NB_TOUCH; ++t) {
                uint32_t x = ts.touchX[t], y = ts.touchY[t];
                if (x >= NES_LCD_WIDTH || y >= NES_LCD_HEIGHT) continue;
#if NES_TOUCH_FLIP_X
                x = NES_LCD_WIDTH - 1 - x;
#endif
#if NES_TOUCH_FLIP_Y
                y = NES_LCD_HEIGHT - 1 - y;
#endif
                for (unsigned i = 0; i < sizeof buttons / sizeof buttons[0]; ++i) {
                    const PadButton *b = &buttons[i];
                    if (x >= b->x && x < b->x + b->w && y >= b->y && y < b->y + b->h)
                        touch_mask |= b->mask;
                }
            }
        }
    }
    nes_touch_us = (DWT->CYCCNT - started) / (SystemCoreClock / 1000000U);
    if (nes_touch_us > nes_touch_max_us) nes_touch_max_us = nes_touch_us;
    return touch_mask | (stable_user ? 0x08U : 0U);
}
void NES_BoardFatal(uint32_t code)
{
    nes_error_code = code;
    /* Repeating LED_RED pulse count; use debugger nes_boot_stage for extra context. */
    for (;;) {
        for (uint32_t i = 0; i < code; ++i) {
            BSP_LED_On(LED_RED); HAL_Delay(180);
            BSP_LED_Off(LED_RED); HAL_Delay(180);
        }
        HAL_Delay(1000);
    }
}
