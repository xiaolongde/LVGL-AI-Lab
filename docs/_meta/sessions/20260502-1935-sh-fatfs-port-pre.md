---
type: session-pre
task: sh-fatfs-port
backlog_item: card3-sd-fatfs-gui-resources
started: 2026-05-02T19:35:29+08:00
---

## 任务理解

v0.2 Card 3 第二站：在 sg 验证的 SDIO 硬件层之上挂 FatFs，让 SD 卡变 mountable filesystem。读 README.TXT 显示在屏上证明端到端工作。

**为什么要 FatFs**：LVGL `lv_fs_drv_t` 期望 POSIX-like API（fopen/fread/fseek），FatFs 提供 f_open/f_read/f_lseek。si-lvgl-fs-drv 步骤会把 LVGL 接到 FatFs。

## 计划

1. **抄 FatFs R0.13c**（来自 KIT `Library/.../Utilities/Third_Party/fat_fs/`）：
   - inc/{ff.h, ffconf.h, diskio.h, ff_gen_drv.h, integer.h, fattime.h} → `external/fatfs/inc/`
   - src/{ff.c, ff_gen_drv.c, ffsystem.c, ffunicode.c, diskio.c, fattime.c} → `external/fatfs/src/`
   - **不抄** `Library/.../Utilities/Third_Party/lcd/` 等无关
   - 用 `external/fatfs/` 而不是 `vendor/`：FatFs 是 third-party 不是 vendor SoC lib

2. **写 sd_diskio.c**（`src/devices/sd/sd_diskio.{c,h}`）：
   - 5 个 callback：disk_initialize / status / read / write / ioctl
   - read/write 走 sg 验证过的 `sd_block_read` / `sd_multiblocks_read`（>1 block 用 multi 提速）
   - ioctl GET_SECTOR_COUNT 调 `sd_card_information_get` + `card_capacity` / 512
   - 暴露 `Diskio_drvTypeDef sd_diskio_drv` 单实例

3. **ffconf.h 裁剪**：
   - `_VOLUMES=1`, `_USE_LFN=0`（暂关，省 ~12KB），`_USE_EXFAT=0`，`_CODE_PAGE=437`
   - `_USE_MKFS=0`（不在 MCU 上格盘，PC 上 format 后插入）
   - `_FS_READONLY=0`（si 步骤可能要 write）

4. **CMakeLists**：
   - 加 `add_library(fatfs STATIC ${FATFS_SRC})` + include path
   - app target link fatfs
   - GLOB `external/fatfs/src/*.c`

5. **main.c 改造**（继续无 LVGL，KIT TFTLCD 直显）：
   - 保留 sg 的 sd_init() + 卡信息显示
   - 加 `FATFS_LinkDriver(&sd_diskio_drv, path)` → `f_mount(&fs, "0:", 1)` → 显 mount res code
   - `f_open(&fp, "0:README.TXT", FA_READ)` + `f_read(&fp, buf, 64, &br)` → 显 br + buf content
   - **多论据 sign-off**：(a) mount 0=OK (b) f_open FR_OK (c) br > 0 (d) buf 字符可识别

6. **build**：估 Flash + 30K (FatFs ~25K) → 60K / 256K，仍宽裕
7. **flash + reset → 用户先 PC 上往 SD 写 README.TXT**（用户介入：插 SD 到读卡器 写文件 拔 插回板子）

## 卡点预判

1. **FATFS_LinkDriver / Diskio_drvTypeDef 接口**：KIT 例子是否用 ff_gen_drv？要确认，因为有些 FatFs 集成是直接重写 diskio.c 的全局函数（旧式），KIT 用的是 ST 风格的 ff_gen_drv 包装（多 driver 注册）。看代码定。
2. **sd_block_read 阻塞 vs DMA**：KIT sdcard.c 默认 polling 还是 DMA？要 grep "DMA" 确认。如果 DMA 但中断未挂会死锁。
3. **fattime 需 RTC**：ffsystem 的 get_fattime() 给文件 timestamp。sg 没启 RTC，可先返 fixed value（2026-05-02 12:00）；si 之后再接真 RTC。
4. **README.TXT 字符集**：FAT32 SFN 限 8.3 大写 ASCII。LFN 关了所以 "README.TXT" OK，但路径分隔符要 `/` 不是 `\`，且首字符可能要 `0:/README.TXT`。
5. **FatFs version vs gd32 SDK 兼容性**：KIT 自带是 R0.13c (2018)，应没问题。但 _USE_MUTEX 等新选项要在 ffconf.h 里关。

## 验收

- [ ] FatFs 文件抄到 `external/fatfs/`，共 ~10 个文件
- [ ] sd_diskio.c 5 callback 实现 + Diskio_drvTypeDef 实例
- [ ] cmake build 通过（fatfs 静态库 + app link 通）
- [ ] Flash < 80KB（FatFs + sg + KIT TFTLCD）
- [ ] 用户在 PC 写 README.TXT 到 SD 卡（"hello FatFs from SD"）
- [ ] 板上 reset 后屏显：mount=0, f_open=0, br=N, buf="hello FatFs..."
- [ ] post 含 1-2 lessons 候选

## 元注释（autonomous mode）

- sd_diskio 实现细节（block_read 的多 sector handling）→ self-recommend，不问用户
- 决策点 default-yes（参 LTRYSL）
- ≥ 3 次失败（如 mount 一直 NOT_READY） → BFS 回 sg post 重看 sd_block_read 是否真工作（写一个 raw block read test 隔离）
