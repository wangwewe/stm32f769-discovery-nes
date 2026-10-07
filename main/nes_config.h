#ifndef NES_CONFIG_H
#define NES_CONFIG_H
/* First version: STM32F769I-DISCO, 25-MHz HSE / 200-MHz SYSCLK. */
#define NES_GAME_INDEX        2 /* 0: SuperMario, 1: yingzichuanshuo, 2: SuperDodgeBall */
#define NES_LCD_WIDTH         800U
#define NES_LCD_HEIGHT        480U
#define NES_DISPLAY_SCALE     2U
#define NES_FRAMEBUFFER_ADDR  0xC0000000UL
#define NES_WORK_FRAME_ADDR   0xC0200000UL
#define NES_SDRAM_TEST_ADDR   0xC0300000UL
/* FT6x06 BSP already supplies landscape coordinates. */
#define NES_TOUCH_FLIP_X      0
#define NES_TOUCH_FLIP_Y      0
/* Audio: headphones; 0..100 codec volume. Test tone replaces game audio only. */
#define NES_AUDIO_ENABLE 1
#define NES_AUDIO_VOLUME 35
#define NES_AUDIO_TEST_TONE 0
#endif
