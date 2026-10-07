# stm32f769-discovery-nes
NES emulator for the STM32F769 Discovery board, featuring touchscreen controls and WM8994 audio output. Built with STM32CubeIDE.

## STM32F769 Discovery NES Changelog

1. v0.1: Ported the STM32F469 project to STM32F769I-DISCO, with touchscreen controls and WM8994 audio output.
2. v0.2: Added SuperDodgeBall, moved PRG/CHR storage to a dedicated SDRAM area, and added ROM read bounds checks.

## Test Hardware

1. STM32F769I-DISCO with the original DSI touchscreen (MCU: STM32F769NIH6).

## Programming and Testing

1. Connect headphones or powered speakers to the board's 3.5 mm headphone output jack.
2. Use STM32CubeProgrammer to program `Debug/STM32F769_Disco_NES.hex`. After programming, press the Reset button on the board.
3. Press USER or the on-screen START button to start the game. Test background music and game sound effects. For Super Mario, also test jumping and coin collection sounds.
4. The default codec volume is 75/100. Adjust `NES_AUDIO_VOLUME` in `main/nes_config.h` to change it.

## Building the Project

1. In STM32CubeIDE, select **File → Import → General → Existing Projects into Workspace** and choose the extracted project folder. Select `STM32F769_Disco_NES_CubeIDE`, then **Build Project**. This project uses a Makefile and does not require CubeMX regeneration. The earlier F469 workflow used CubeIDE 1.16.1; the automated F769 build was verified with the standalone GNU toolchain listed below.
2. After a successful build, a `Debug` directory is generated containing the following files:

   - `STM32F769_Disco_NES.bin`
   - `STM32F769_Disco_NES.elf`
   - `STM32F769_Disco_NES.hex`
   - `STM32F769_Disco_NES.map`

3. Use STM32CubeProgrammer to program `STM32F769_Disco_NES.hex` onto the development board for testing.

## How to Switch Test Games
1. Modify NES_GAME_INDEX in main/nes_config.h.
2. Set NES_GAME_INDEX to 0 for Super Mario.
3. Set NES_GAME_INDEX to 1 for yingzichuanshuo.
4. Set NES_GAME_INDEX to 2 for SuperDodgeBall (the v0.2 default).
5. Rebuild and program the new HEX after changing the selection.

## Audio Processing and Synchronization

For each emulated frame: `InfoNES_pAPUVsync` generates five APU waveforms → `InfoNES_SoundOutput` → `NES_AudioOutput`.

- The sample rate is 44,100 Hz, with 735 samples per frame, corresponding to approximately 60 emulated frames per second.
- The initial audio implementation uses linear mixing, DC removal, and signed 16-bit PCM. The left and right channels carry identical audio; this is not native stereo.
- The main program writes to a two-block mono queue. DMA half-transfer and transfer-complete interrupts copy completed blocks into the buffer half that has finished playing.
- The circular DMA buffer has two halves. Each half contains 735 left/right sample pairs, representing approximately 16.67 ms of audio.
- Two frames are prepared before playback starts. When the queue is full, the main program waits for DMA to consume data, using the audio clock to pace the game.
- The queue and DMA buffer together can hold approximately four frames of audio (66.7 ms), introducing a corresponding audio delay.
- If data is unavailable, silence is inserted and the underrun counter is incremented. Repeated underruns indicate that audio generation and/or game rendering cannot keep up.
- The maximum wait for DMA is 100 ms. If audio fails, output is stopped and the game falls back to approximately 60 Hz pacing using the HAL tick.
- `FrameSkip = 3` is retained, so the screen is rendered once every four emulated frames. Audio is generated for every emulated frame.

## Configurable Settings

In `main/nes_config.h`:

| Setting | Default | Description |
|---|---|---|
| NES_AUDIO_ENABLE | 1 | Set to 0 to disable hardware audio while retaining approximately 60 Hz pacing. |
| NES_AUDIO_VOLUME | 75 | Codec volume, from 0 to 100. Rebuild after changing this value. |
| NES_AUDIO_TEST_TONE | 0 | Set to 1 to replace game audio with a 440 Hz square-wave test tone. The game display continues running. |
| NES_GAME_INDEX | 2 | 0: SuperMario; 1: yingzichuanshuo; 2: SuperDodgeBall. |
| NES_DISPLAY_SCALE | 2 | Integer display scaling factor. Set to 1 to reduce the display workload for comparison. |
| NES_TOUCH_FLIP_X/Y | 0 | Reverse the horizontal/vertical touch coordinates. |

If there is no sound, start the game first (the title screen may be silent), then check the headphone connection and volume. To isolate the issue, build with `NES_AUDIO_TEST_TONE = 1`. If the test tone is audible, the basic codec/DMA path is working; then investigate game audio generation.

## Debug Variables

| Variable | Meaning |
|---|---|
| nes_audio_ready | 1: Audio initialization succeeded. |
| nes_audio_running | 1: DMA playback has started. |
| nes_audio_frames | Number of times NES has called the audio output function. |
| nes_audio_callbacks | Number of DMA half-transfer/transfer-complete callbacks. This should keep increasing during normal playback. |
| nes_audio_underruns | Number of buffer halves filled with silence because no completed audio data was available at the callback. |
| nes_audio_error | Error code listed below. 0 means no error has been recorded. |

| Error Code | Meaning |
|---|---|
| 1 | The sample rate or samples per frame does not match 44100/735. |
| 2 | Audio PLLI2S configuration failed. |
| 3 | BSP/codec initialization failed. |
| 4 | DMA playback failed to start. |
| 5 | SAI/DMA error callback occurred. |
| 6 | The queue was full, but DMA did not free any space within 100 ms. |

The red LED (LED1 / LED_RED) turns on after an audio failure. Check `nes_touch_ok` and `nes_audio_error` to distinguish the cause. The DMA ISR does not perform blocking I2C operations or call `HAL_Delay`.

## Retained Board Debug Information

- Green LED (LED2 / LED_GREEN): Turns on after the LCD/SDRAM/touch initialization sequence and toggles after every 30 displayed game frames.
- Red LED (LED1 / LED_RED): Indicates a touch/audio error; repeating fatal-error blink counts mean 1 = HSE/PLL, 2 = system clock/OverDrive, 3 = LCD, 4 = SDRAM, 5 = ROM loading failed or NES returned.
- `nes_boot_stage`: 1 = before clock configuration; 2 = clock configuration complete; 3 = LCD initialization; 4 = SDRAM check; 5 = touch initialization; 6 = NES execution.
- `nes_error_code`: 0 = normal; 1–5 = as listed above; 10 = HardFault; 11 = MemManage; 12 = BusFault; 13 = UsageFault. Fault handlers halt execution for debugger inspection.

## Memory and Hardware

- Cortex-M7 at 200 MHz, with instruction/data caches enabled; HSE is 25 MHz. SDRAM runs at 100 MHz with the current BSP timings.
- Internal Flash: 2 MiB. SRAM1: 368 KiB for data/BSS. SRAM2: 16 KiB reserved as non-cacheable memory for audio DMA. DTCM: 128 KiB, with a 64 KiB heap and a 24 KiB stack.
- The audio DMA buffer uses the `.dma_audio` section in SRAM2 at `0x2007C000`. The CPU-only mono queue stays in SRAM1. PRG/CHR data no longer uses the DTCM heap in v0.2.
- LCD: 800 × 480 logical framebuffer; NES rendering: 256 × 240, scaled 2× by default.
- LCD framebuffer: `0xC0000000`; NES WorkFrame: `0xC0200000`; SDRAM test area: `0xC0300000`.
- v0.2 reserves `0xC0400000–0xC04FFFFF` (1 MiB) for the selected game's PRG/CHR data in `.nes_rom` (NOLOAD). It is filled only after SDRAM initialization.
- MPU configuration makes the first 2 MiB of SDRAM non-cacheable for LCD/DMA2D access; CPU work and ROM data use cacheable SDRAM.
- SAI1 Block A: PG7 = MCLK, PE4 = FS, PE5 = SCK, PE6 = SD.
- DMA2 Stream1 Channel0 with half-transfer/transfer-complete interrupts; SAI1 error interrupt.
- Audio uses PLLI2S; LCD uses PLLSAI. WM8994 output uses slots 0/2 of the 64-bit SAI frame, carrying identical signed 16-bit samples for the left and right channels.

## Validation Scope

The v0.2 package built with Arm GNU Toolchain 12.3.Rel1 and `-O3`, with 0 errors and 0 warnings.

Automated checks verified the linked memory layout, interrupt vectors, project XML, embedded ROM bytes, and ROM reader bounds/error cases. Test source files are in `tests/`; build output and verification notes are in `Firmware/`. The prebuilt `Firmware/STM32F769_Disco_NES.hex` is not updated by subsequent builds; flash the new `Debug` HEX after source changes.

Board testing reported that SuperDodgeBall runs with music. However, in the first match against Hanazono High School, only the upper-left portion of the background is visible while the other portions are black; sprites remain visible. This issue remains unresolved. The MMC1 8 KiB CHR low-bit adjustment did not improve this scene, so full game compatibility is not claimed.

The DTCM usage report can show 100% because the stack is placed at the top of the region. This does not mean that data/BSS exhausted internal RAM. The reserved 1 MiB ROM arena also appears fully allocated regardless of the selected game's actual size.

See `DEPENDENCIES.txt` for dependency versions. License information is provided in `Licenses/` and the source file headers.
