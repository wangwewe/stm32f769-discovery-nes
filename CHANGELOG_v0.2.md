# v0.2 — SuperDodgeBall

預設 main/nes_config.h 的 NES_GAME_INDEX = 2。
0 = SuperMario，1 = yingzichuanshuo，2 = SuperDodgeBall。

新增 ROM、遊戲表與 Makefile 相依性，索引檢查改用 GAME_FILE_NUM。
nesReadFile 加入檔案長度檢查；載入器檢查每次讀取結果，拒絕 NES 2.0 與過大資料。
PRG/CHR 改放到 0xC0400000–0xC04FFFFF 的 1 MiB CPU-only cached SDRAM arena。
此 NOLOAD 區不由啟動碼清除；必須在 SDRAM 初始化後呼叫載入器。
與 LCD framebuffer、WorkFrame、SDRAM test 及音效 DMA 記憶體不重疊。
DTCM heap 保留 64 KiB，但 PRG/CHR 不再使用 malloc/free。

提供的 ROM 標頭：iNES、Mapper 1 (MMC1)、PRG 128 KiB、CHR 128 KiB，無 trainer。
標頭所需檔案長度 262160 bytes，原始檔案實際 786464 bytes。
保留使用者提供的原始檔案；尾端額外 524304 bytes 不由載入器讀取。
不根據檔名修改 Mapper 或猜測尾端資料格式。
程式已包含 Mapper 1，但實際遊戲相容性、畫面與音效仍需上板確認。

解壓縮後依 README 匯入 CubeIDE。若 workspace 已有同名專案，先移除舊專案參照，
不要勾選刪除磁碟內容，再匯入本版。預編譯 HEX 位於 Firmware。
重新編譯後請燒錄 Debug 下的新 HEX，Firmware 不會自動更新。
保留 -O3、FrameSkip=3、2 倍顯示與音效設定。
