#ifndef NES_AUDIO_H
#define NES_AUDIO_H
#include <stdint.h>
void NES_AudioReset(void);
int NES_AudioOpen(int samples, int rate);
void NES_AudioClose(void);
void NES_AudioOutput(int samples, const uint8_t *a, const uint8_t *b,
                     const uint8_t *c, const uint8_t *d, const uint8_t *e);
extern volatile uint32_t nes_audio_ready, nes_audio_running, nes_audio_error;
extern volatile uint32_t nes_audio_underruns, nes_audio_callbacks, nes_audio_frames;
#endif
