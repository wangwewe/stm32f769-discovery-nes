#ifndef NES_BOARD_H
#define NES_BOARD_H
#include <stdint.h>
#include "nes_config.h"
#include "InfoNES_Types.h"
void NES_BoardInit(void);
void NES_BoardPresent(const volatile WORD *frame);
uint8_t NES_BoardReadPad(void);
void NES_BoardFatal(uint32_t code);
extern volatile uint32_t nes_boot_stage;
extern volatile uint32_t nes_error_code;
extern volatile uint32_t nes_touch_ok;
extern volatile uint32_t nes_frame_count;
#endif
