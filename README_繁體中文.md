# stm32f769-discovery-nes

適用於 STM32F769 Discovery 開發板的 NES 模擬器，支援觸控按鍵與 WM8994 音效輸出，使用 STM32CubeIDE 編譯。

## STM32F769 Discovery NES 更新紀錄

1. v0.1：由 STM32F469 專案移植至 STM32F769I-DISCO，支援觸控按鍵與 WM8994 音效。
2. v0.2：新增 SuperDodgeBall，將 PRG/CHR 資料移至獨立 SDRAM 區域，並加入 ROM 讀取邊界檢查。

## 測試硬體

1. STM32F769I-DISCO，搭配原廠 DSI 觸控螢幕（MCU：STM32F769NIH6）。

## 燒錄測試

1. 接上開發板的 3.5 mm 耳機輸出孔，使用耳機或有源喇叭。
2. 使用STM32CubeProgrammer燒錄 `Debug/STM32F769_Disco_NES.hex`，燒錄完畢後按下開發板上的Reset按鈕。
3. 按 USER 或觸控 START 開始，測試背景音樂及遊戲音效；使用 Super Mario 時可測試跳躍與吃金幣音效。
4. 預設 codec 音量 75/100。可在 `main/nes_config.h` 修改 `NES_AUDIO_VOLUME`。

## 如何編譯專案

1. 在 STM32CubeIDE 選擇 **File → Import → General → Existing Projects into Workspace**，選取解壓縮後的專案資料夾。匯入 `STM32F769_Disco_NES_CubeIDE` 後按 **Build Project**。此專案使用 Makefile，不需要 CubeMX 重新產生程式碼。先前 F469 流程使用 CubeIDE 1.16.1；F769 自動建置使用下方所列的獨立 GNU toolchain 驗證。
2. 編譯成功後, 會生成一個Debug目錄, 底下會有以下的檔案:

   - `STM32F769_Disco_NES.bin`
   - `STM32F769_Disco_NES.elf`
   - `STM32F769_Disco_NES.hex`
   - `STM32F769_Disco_NES.map`

3. 可以使用STM32CubeProgrammer燒錄STM32F769_Disco_NES.hex到開發板進行測試。

## 如何切換測試遊戲
1. 在main/nes_config.h修改NES_GAME_INDEX參數。
2. NES_GAME_INDEX => 0, Super Mario（超級瑪利歐)。
3. NES_GAME_INDEX => 1, yingzichuanshuo（影子傳說）。
4. NES_GAME_INDEX => 2, SuperDodgeBall（v0.2 預設）。
5. 修改後重新編譯，再燒錄新的 HEX。

## 音訊流程與同步

每個模擬影格：InfoNES_pAPUVsync 產生五路 APU 波形 → InfoNES_SoundOutput → NES_AudioOutput。
- 44,100 Hz；每影格 735 個取樣，約 60 個模擬影格/秒。
- 首版使用線性混音、DC 去除、16-bit 有號 PCM；左右聲道相同，不是原生立體聲。
- 主程式寫入兩格 mono queue；DMA half/full IRQ 把完成的區塊複製到已播放完的半區。
- DMA circular buffer 共兩個半區，每半區 735 組左右取樣，約 16.67 ms。
- 開始播放前先準備兩個影格；queue 填滿時主程式等待 DMA 消耗，以音訊時鐘控制遊戲節奏。
- queue 與 DMA 合計最多約四個影格的儲存量（66.7 ms），會有相應聲音延遲。
- 資料不足時補靜音並累加 underruns；若持續發生，代表產生聲音/遊戲畫面的速度跟不上。
- 等候 DMA 最長 100 ms；音訊故障後停止輸出，遊戲改用 HAL tick 約 60 Hz 節拍。
- 保留 FrameSkip=3，因此每4個模擬影格畫一次螢幕；音訊每個模擬影格都有產生。

## 可調設定

在 `main/nes_config.h`：

| 設定 | 預設 | 用途 |
|---|---|---|
| NES_AUDIO_ENABLE | 1 | 改為 0 可停用硬體音訊，保留約 60 Hz 節拍 |
| NES_AUDIO_VOLUME | 75 | 0–100 codec 音量；更改後重新編譯 |
| NES_AUDIO_TEST_TONE | 0 | 改為 1，將遊戲聲音替換為 440 Hz 測試方波；遊戲畫面仍執行 |
| NES_GAME_INDEX | 2 | 0 SuperMario；1 yingzichuanshuo；2 SuperDodgeBall |
| NES_DISPLAY_SCALE | 2 | 畫面整數放大倍率；改 1 可降低顯示負擔作比較 |
| NES_TOUCH_FLIP_X/Y | 0 | 觸控左右/上下反轉 |

若沒有聲音，先啟動遊戲（標題畫面不一定有聲音），再檢查耳機接孔與音量。需要隔離問題時可編譯 NES_AUDIO_TEST_TONE=1：能聽到測試音代表基本 codec/DMA 路徑可工作，再回頭檢查遊戲音訊。

## 除錯變數

| 變數 | 意義 |
|---|---|
| nes_audio_ready | 1：音訊初始化成功 |
| nes_audio_running | 1：已啟動 DMA 播放 |
| nes_audio_frames | NES 呼叫聲音輸出的次數 |
| nes_audio_callbacks | DMA half/full 回呼次數，正常會持續增加 |
| nes_audio_underruns | 回呼時無完成資料可取用，補靜音的半區次數 |
| nes_audio_error | 下表錯誤碼；0 表示尚無已記錄錯誤 |

| 錯誤碼 | 意義 |
|---|---|
| 1 | 取樣率或每影格取樣數不符 44100/735 |
| 2 | 音訊 PLLI2S 設定失敗 |
| 3 | BSP/codec 初始化失敗 |
| 4 | 啟動 DMA 播放失敗 |
| 5 | SAI/DMA 錯誤回呼 |
| 6 | queue 已滿，但 100 ms 內 DMA 沒有釋出空間 | 

音訊失敗後紅色 LED（LED1 / LED_RED）會亮；需搭配 nes_touch_ok 與 nes_audio_error 分辨。DMA ISR 不執行阻塞式 I2C 或 HAL_Delay。

## 保留的板子除錯資訊

- 綠色 LED（LED2 / LED_GREEN）：LCD/SDRAM/觸控初始化流程結束後亮起，每輸出 30 張遊戲畫面翻轉一次。
- 紅色 LED（LED1 / LED_RED）：觸控或音訊失敗時亮起；重大錯誤時重複閃爍，次數為 1=HSE/PLL、2=系統時鐘/OverDrive、3=LCD、4=SDRAM、5=ROM 載入失敗或 NES 返回。

`nes_boot_stage`：1 時鐘前、2 時鐘完成、3 LCD、4 SDRAM 檢查、5 觸控、6 NES。
`nes_error_code`：0 正常；1–5 同上；10 HardFault、11 MemManage、12 BusFault、13 UsageFault（Fault 停住供 debugger 查看）。

## 記憶體與硬體

- Cortex-M7 時脈 200 MHz，啟用指令與資料 Cache；HSE 為 25 MHz。SDRAM 配合目前 BSP 時序使用 100 MHz。
- 內部 Flash 2 MiB；SRAM1 368 KiB 放置 data/BSS；SRAM2 16 KiB 設為 non-cacheable 音訊 DMA 區；DTCM 128 KiB，其中 heap 64 KiB、stack 24 KiB。
- 音訊 DMA buffer 放在 SRAM2 的 `.dma_audio` 區段（`0x2007C000`）；CPU 使用的 mono queue 放在 SRAM1。v0.2 的 PRG/CHR 不再使用 DTCM heap。
- LCD 邏輯 framebuffer 為 800 × 480；NES 畫面為 256 × 240，預設放大 2 倍。
- LCD framebuffer：`0xC0000000`；NES WorkFrame：`0xC0200000`；SDRAM 測試區：`0xC0300000`。
- v0.2 預留 `0xC0400000–0xC04FFFFF`（1 MiB）作為所選遊戲的 PRG/CHR 儲存區，區段為 `.nes_rom`（NOLOAD），僅在 SDRAM 初始化後載入。
- MPU 將 SDRAM 前 2 MiB 設為 non-cacheable，供 LCD/DMA2D 使用；CPU 工作畫面與 ROM 資料使用 cacheable SDRAM。
- SAI1 Block A：PG7 MCLK、PE4 FS、PE5 SCK、PE6 SD。
- DMA2 Stream1 Channel0，half/full interrupt；SAI1 error interrupt。
- 音訊使用 PLLI2S，LCD 使用 PLLSAI；WM8994 使用 64-bit SAI frame 的 slots 0/2，左右聲道輸出相同的 16-bit 有號 PCM。

## 驗證範圍

v0.2 使用 Arm GNU Toolchain 12.3.Rel1、`-O3` 編譯，0 errors、0 warnings。

自動檢查涵蓋連結後記憶體配置、中斷向量、專案 XML、嵌入 ROM 位元組，以及 ROM 讀取邊界與錯誤情況。測試來源在 `tests/`，建置紀錄與驗證說明在 `Firmware/`。`Firmware/STM32F769_Disco_NES.hex` 是預編譯檔，後續 Build 不會更新它；修改程式後請燒錄 `Debug` 下的新 HEX。

實機測試已回報 SuperDodgeBall 可執行並播放音樂。不過，第一場對花園高校時，背景只有左上區域正常，其餘區域為黑色，人物仍可顯示。此問題尚未解決；MMC1 的 8 KiB CHR 最低位元修正未改善該場景，因此尚不能宣稱完整遊戲相容。

Linker 的 DTCM 使用量可能顯示 100%，是因為 stack 固定放在區域頂端，不表示 data/BSS 已耗盡內部 RAM。預留的 1 MiB ROM 區也會顯示完整配置，與目前遊戲實際大小不同。

相依版本見 `DEPENDENCIES.txt`，授權資訊見 `Licenses/` 與各檔案檔頭。
