#include "nes_audio.h"
#include "nes_config.h"
#include "stm32f769i_discovery_audio.h"
#include <string.h>

/* DMA buffer is in MPU non-cacheable SRAM2; mono queue is CPU-only cached SRAM1.
 * 735 samples / 44100 Hz = one 60 Hz emulation frame.
 * Main publishes complete mono blocks; DMA ISR consumes them. */
#define SAMPLES 735U
#define QUEUE_BLOCKS 2U
static int16_t queue[QUEUE_BLOCKS][SAMPLES];
static int16_t dma_pcm[2][SAMPLES * 2] __attribute__((section(".dma_audio"), aligned(32)));
static volatile uint32_t produced, consumed;
static int32_t previous_x, previous_y;
static uint32_t fallback_tick, fallback_fraction;
static uint8_t codec_initialized;
volatile uint32_t nes_audio_ready, nes_audio_running, nes_audio_error;
volatile uint32_t nes_audio_underruns, nes_audio_callbacks, nes_audio_frames;
extern SAI_HandleTypeDef haudio_out_sai;
_Static_assert(NES_AUDIO_VOLUME <= 100 && NES_AUDIO_VOLUME >= 0, "Volume must be 0..100");

/* Used only when audio is disabled or failed. Do not try to catch up after a stall. */
static void PaceWithoutAudio(void)
{
    fallback_fraction += 1000U;
    uint32_t interval = fallback_fraction / 60U;
    fallback_fraction %= 60U;
    uint32_t elapsed = HAL_GetTick() - fallback_tick;
    if (elapsed < interval) HAL_Delay(interval - elapsed);
    fallback_tick = HAL_GetTick();
}
static void Fail(uint32_t code)
{
    if (!nes_audio_error) nes_audio_error = code;
    nes_audio_ready = 0;
    nes_audio_running = 0;
    /* This function is called in main context only (codec uses I2C/delays). */
    if (codec_initialized) {
        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
        codec_initialized = 0;
    }
    HAL_NVIC_DisableIRQ(SAI1_IRQn);
    BSP_LED_On(LED_RED);
    fallback_tick = HAL_GetTick();
}
void NES_AudioClose(void)
{
    nes_audio_running = 0;
    nes_audio_ready = 0;
    if (codec_initialized) {
        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
        codec_initialized = 0;
    }
    HAL_NVIC_DisableIRQ(SAI1_IRQn);
}
void NES_AudioReset(void)
{
    NES_AudioClose();
    produced = consumed = 0;
    previous_x = previous_y = 0;
    fallback_fraction = 0;
    fallback_tick = HAL_GetTick();
    nes_audio_error = nes_audio_underruns = nes_audio_callbacks = nes_audio_frames = 0;
    memset(dma_pcm, 0, sizeof dma_pcm);
}
/* Override ST's weak clock setup so timeout errors are observable.
 * PLLI2S is independent of the LCD's PLLSAI. HSE/PLLM = 1 MHz. */
void BSP_AUDIO_OUT_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t freq, void *params)
{
    RCC_PeriphCLKInitTypeDef clk = {0};
    (void)hsai; (void)params;
    if (freq != 44100U) { nes_audio_error = 1; return; }
    HAL_RCCEx_GetPeriphCLKConfig(&clk);
    clk.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    clk.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLI2S;
    clk.PLLI2S.PLLI2SN = 429;
    clk.PLLI2S.PLLI2SQ = 2;
    clk.PLLI2SDivQ = 19;
    if (HAL_RCCEx_PeriphCLKConfig(&clk) != HAL_OK) nes_audio_error = 2;
}
int NES_AudioOpen(int samples, int rate)
{
    if (samples != (int)SAMPLES || rate != 44100) { Fail(1); return 0; }
    if (!NES_AUDIO_ENABLE) return 1;
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_BOTH, NES_AUDIO_VOLUME, rate) != AUDIO_OK) {
        Fail(3); return 0;
    }
    codec_initialized = 1;
    if (nes_audio_error) { Fail(nes_audio_error); return 0; }
    /* WM8994 uses slots 0/2, matching ST's F769 BSP playback example.
     * Sending all four slots would halve the effective PCM sample rate. */
    BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
    if (HAL_SAI_GetState(&haudio_out_sai) != HAL_SAI_STATE_READY) { Fail(3); return 0; }
    HAL_NVIC_SetPriority(SAI1_IRQn, AUDIO_OUT_IRQ_PREPRIO, 0);
    HAL_NVIC_EnableIRQ(SAI1_IRQn);
    nes_audio_ready = 1;
    return 1;
}
static void FillHalf(unsigned half)
{
    uint32_t rd = consumed;
    uint32_t wr = produced;
    __DMB();
    if (rd == wr) {
        memset(dma_pcm[half], 0, sizeof dma_pcm[half]);
        ++nes_audio_underruns;
        return;
    }
    const int16_t *src = queue[rd % QUEUE_BLOCKS];
    for (unsigned i = 0; i < SAMPLES; ++i) {
        dma_pcm[half][2*i] = src[i];
        dma_pcm[half][2*i+1] = src[i];
    }
    __DMB();
    consumed = rd + 1U;
}
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    if (nes_audio_running) { FillHalf(0); ++nes_audio_callbacks; }
}
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    if (nes_audio_running) { FillHalf(1); ++nes_audio_callbacks; }
}
void BSP_AUDIO_OUT_Error_CallBack(void)
{
    /* Never call the blocking codec APIs inside the IRQ. */
    nes_audio_error = 5;
    nes_audio_running = 0;
    nes_audio_ready = 0;
}
void NES_AudioOutput(int samples, const uint8_t *a, const uint8_t *b,
                     const uint8_t *c, const uint8_t *d, const uint8_t *e)
{
    ++nes_audio_frames;
    if (nes_audio_error) { if (codec_initialized) Fail(nes_audio_error); PaceWithoutAudio(); return; }
    if (!nes_audio_ready) { PaceWithoutAudio(); return; }
    if (samples != (int)SAMPLES) { Fail(1); PaceWithoutAudio(); return; }
    uint32_t began = HAL_GetTick();
    while ((uint32_t)(produced - consumed) >= QUEUE_BLOCKS) {
        if (nes_audio_error) { Fail(nes_audio_error); PaceWithoutAudio(); return; }
        if ((uint32_t)(HAL_GetTick() - began) >= 100U) {
            Fail(6); PaceWithoutAudio(); return;
        }
        HAL_Delay(1); /* DMA interrupts continue while producer waits. */
    }
    uint32_t wr = produced;
    int16_t *dst = queue[wr % QUEUE_BLOCKS];
    for (unsigned i = 0; i < SAMPLES; ++i) {
#if NES_AUDIO_TEST_TONE
        static uint32_t phase;
        phase = (phase + 440U) % 44100U;
        dst[i] = phase < 22050U ? 2500 : -2500;
#else
        /* Linear first-version mixer, then DC blocker (~27 Hz at 44.1 kHz).
         * All 5 legacy APU channels are unsigned; subtracting a fixed bias
         * would leave a DC jump when the game changes channel activity. */
        int32_t x = ((int32_t)a[i] + b[i] + c[i] + d[i] + e[i]) * 24;
        int32_t y = x - previous_x + (previous_y * 255) / 256;
        previous_x = x;
        previous_y = y;
        if (y > 32767) y = 32767;
        if (y < -32768) y = -32768;
        dst[i] = (int16_t)y;
#endif
    }
    __DMB();
    produced = wr + 1U; /* Publish only after the complete block is ready. */
    if (nes_audio_error) { Fail(nes_audio_error); return; }
    if (!nes_audio_running && (uint32_t)(produced - consumed) >= 2U) {
        FillHalf(0);
        FillHalf(1);
        __DMB();
        nes_audio_running = 1;
        if (BSP_AUDIO_OUT_Play((uint16_t *)dma_pcm, sizeof dma_pcm) != AUDIO_OK)
            Fail(4);
    }
}
