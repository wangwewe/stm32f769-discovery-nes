#include "stm32f7xx_hal.h"
#include "nes_board.h"
extern LTDC_HandleTypeDef hltdc_discovery;
extern DSI_HandleTypeDef hdsi_discovery;
extern DMA2D_HandleTypeDef hdma2d_discovery;
extern SAI_HandleTypeDef haudio_out_sai;
void NMI_Handler(void) {}
void HardFault_Handler(void) { nes_error_code = 10; while (1) {} }
void MemManage_Handler(void) { nes_error_code = 11; while (1) {} }
void BusFault_Handler(void) { nes_error_code = 12; while (1) {} }
void UsageFault_Handler(void) { nes_error_code = 13; while (1) {} }
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}
void SysTick_Handler(void) { HAL_IncTick(); }
void LTDC_IRQHandler(void) { HAL_LTDC_IRQHandler(&hltdc_discovery); }
void LTDC_ER_IRQHandler(void) { HAL_LTDC_IRQHandler(&hltdc_discovery); }
void DSI_IRQHandler(void) { HAL_DSI_IRQHandler(&hdsi_discovery); }
void DMA2D_IRQHandler(void) { HAL_DMA2D_IRQHandler(&hdma2d_discovery); }
void DMA2_Stream1_IRQHandler(void) { HAL_DMA_IRQHandler(haudio_out_sai.hdmatx); }
void SAI1_IRQHandler(void) { HAL_SAI_IRQHandler(&haudio_out_sai); }
