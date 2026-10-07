#include "stm32f769i_discovery.h"
#include "nes_board.h"
extern void nesStart(void);

/* Configure before enabling the M7 caches.
 * SDRAM is Normal WB memory for the CPU-only NES work frame.
 * Override the first 2 MiB as non-cacheable for LTDC/DMA2D framebuffer,
 * and reserve SRAM2 (16 KiB) as non-cacheable for SAI DMA. */
static void MPU_Config(void)
{
    MPU_Region_InitTypeDef r = {0};
    HAL_MPU_Disable();
    /* Prevent speculative accesses into uninitialized external devices. */
    r.Enable = MPU_REGION_ENABLE;
    r.Number = MPU_REGION_NUMBER0;
    r.BaseAddress = 0;
    r.Size = MPU_REGION_SIZE_4GB;
    r.SubRegionDisable = 0x87;
    r.AccessPermission = MPU_REGION_NO_ACCESS;
    r.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    r.IsShareable = MPU_ACCESS_SHAREABLE;
    HAL_MPU_ConfigRegion(&r);

    r.Number = MPU_REGION_NUMBER1;
    r.BaseAddress = 0xC0000000;
    r.Size = MPU_REGION_SIZE_16MB;
    r.SubRegionDisable = 0;
    r.AccessPermission = MPU_REGION_FULL_ACCESS;
    r.TypeExtField = MPU_TEX_LEVEL1;
    r.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    r.IsCacheable = MPU_ACCESS_CACHEABLE;
    r.IsBufferable = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&r);

    r.Number = MPU_REGION_NUMBER2;
    r.BaseAddress = 0xC0000000;
    r.Size = MPU_REGION_SIZE_2MB;
    r.IsShareable = MPU_ACCESS_SHAREABLE;
    r.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    r.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&r);

    r.Number = MPU_REGION_NUMBER3;
    r.BaseAddress = 0x2007C000;
    r.Size = MPU_REGION_SIZE_16KB;
    HAL_MPU_ConfigRegion(&r);

    r.Number = MPU_REGION_NUMBER4;
    r.BaseAddress = 0xA0000000;
    r.Size = MPU_REGION_SIZE_8KB;
    r.TypeExtField = MPU_TEX_LEVEL0;
    r.IsBufferable = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&r);
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM = 25;
    osc.PLL.PLLN = 400;
    osc.PLL.PLLP = RCC_PLLP_DIV2;
    osc.PLL.PLLQ = 8;
    osc.PLL.PLLR = 7;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) NES_BoardFatal(1);
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) NES_BoardFatal(2);
    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV4;
    clk.APB2CLKDivider = RCC_HCLK_DIV2;
    /* ST BSP SDRAM timing/refresh assumes HCLK/2 = 100 MHz. */
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_6) != HAL_OK) NES_BoardFatal(2);
    SystemCoreClockUpdate();
}
int main(void)
{
    MPU_Config();
    SCB_EnableICache();
    SCB_EnableDCache();
    HAL_Init();
    BSP_LED_Init(LED_RED);
    BSP_LED_Init(LED_GREEN);
    nes_boot_stage = 1;
    SystemClock_Config();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55UL; /* Unlock Cortex-M7 DWT registers. */
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    nes_boot_stage = 2;
    NES_BoardInit();
    nes_boot_stage = 6;
    nesStart();
    NES_BoardFatal(5);
}
