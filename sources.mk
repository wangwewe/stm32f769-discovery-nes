C_SOURCES := \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma2d.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sdram.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc_ex.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dsi.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai_ex.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dfsdm.c \
 Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_fmc.c \
 Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.c \
 Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.c \
 Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.c \
 Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_ts.c \
 Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.c \
 Drivers/BSP/Components/ft6x06/ft6x06.c \
 Drivers/BSP/Components/otm8009a/otm8009a.c \
 Drivers/BSP/Components/nt35510/nt35510.c \
 Drivers/BSP/Components/wm8994/wm8994.c \
 Drivers/BSP/Components/adv7533/adv7533.c \
 nes/src/InfoNES.c \
 nes/src/InfoNES_Mapper.c \
 nes/src/InfoNES_pAPU.c \
 nes/src/K6502.c \
 nes/port/nes_port.c \
 nes/games/nes_game.c \
 main/main.c \
 main/nes_board.c \
 main/nes_audio.c \
 main/stm32f7xx_it.c \
 GCC/system_stm32f7xx.c \
 GCC/sysmem.c
ASM_SOURCES := GCC/startup_stm32f769xx.s GCC/game_resources.S
INCLUDES := -Imain -ICMSIS/Include -ICMSIS/Device/ST/STM32F7xx/Include -IDrivers/STM32F7xx_HAL_Driver/Inc -IDrivers/BSP/STM32F769I-Discovery -IUtilities/Fonts -Ines/src -Ines/port -Ines/games
