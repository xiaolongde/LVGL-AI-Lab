---
type: session-post
task: sh-fatfs-port
pre: 20260502-1935-sh-fatfs-port-pre.md
ended: 2026-05-02T20:14:40+08:00
outcome: success
---

## 实际遇到

**意料外故障**：第一次 build flash → mount=1 (FR_DISK_ERR)。f_mount 的 disk_read 返回 RES_ERROR。

**应用三步硬件诊断法 LXO38V** + 隔离测试：在 f_mount 前加 raw `sd_block_read(buf, 0, 512)` + 显 sig + first8 bytes。屏显：`rd=CMD_TIMEOUT sig=00 00`。**根因落到数据路径而非 FatFs glue**。

**核心发现**：sg 调用的 `sd_init()` **不是完整 init**。看 KIT `Examples/SDIO/Read_write/main.c` 才发现 example 包了一层 `sd_io_init()` wrapper（不在 sdcard.c 内），其中 sd_init() 之后还要 3 步：
1. CMD7 `sd_card_select_deselect(rca)` → 把卡从 stand-by 切到 transfer state
2. `sd_bus_mode_config(SDIO_BUSMODE_4BIT)` → 切 4-bit bus（之前是 1-bit init 默认）
3. `sd_transfer_mode_config(SD_POLLING_MODE)` → 配 polling 数据传输模式

sg 只调了 1（sd_init）+ 2（sd_card_information_get），漏了 3/4/5。导致：
- identification 阶段命令（CMD2/CMD9）能跑 → CSD/CID/Capacity 都对 → sg 假阳性 sign-off
- transfer state 命令（CMD17）卡 timeout → CMD_TIMEOUT

**预判 5 个卡点全没说中**（GPIO/clock/DMA/电源/NVIC）— 真坑是"vendor 库 init 函数命名误导，完整序列在 example wrapper"。

**预判外 happy 路径**：
- ffconf.h 裁剪生效（FF_FS_EXFAT 1→0, FF_USE_MKFS 1→0, FF_CODE_PAGE 932→437, _VOLUMES 2→1）
- DMA buffer 4-byte 对齐：`__attribute__((aligned(4)))` + 我 sd_diskio.c 的 alignment guard 都覆盖到（FATFS 内部 win[] buffer 对齐由 FATFS struct 的对齐决定，实测无 issue）
- KIT integer.h `#include "usb_conf.h"` 寄生头要剔掉（KIT 是给 USB 上下文写的）

## 修复路径

**修一行**（main.c）：
```c
err = sd_init();
if (err == SD_OK) err = sd_card_information_get(&info);
if (err == SD_OK) err = sd_card_select_deselect(info.card_rca);
if (err == SD_OK) err = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
if (err == SD_OK) err = sd_transfer_mode_config(SD_POLLING_MODE);
```

build → flash → reset → 屏显：
- `SD OK (transfer+4bit)` ✓
- `rd=OK sig=... ...` ✓ raw read 通
- `mount=0 (FR_OK=0)` ✓
- `open=0 br=N` + 文件内容 ✓

**4 个独立 datapoint 互证**（多论据 LTIRBE 强应用）：raw block read OK + FatFs mount OK + f_open OK + f_read 数据语义正确。

Flash 41160 B / 256K (15.69%)，RAM 7.6K / 48K (15.92%)。FatFs+sd_diskio 净增 ~12K（裁剪后）。

## 学到的（方法论素材）

**lesson 候选 1：vendor 库 init 函数名是诱饵，完整 init 序列必须从 example main.c 反推**

抄 vendor lib 时，看到 `xx_init()` 函数 ≠ 完整初始化。要把 `Examples/.../main.c` 完整调用链当作"真 init recipe"，库函数本身只是积木。这是 LXO38R "局部 vs 全局配置纪律" 的**第二次重现**（第一次：LVGL lv_conf 配置项隐式依赖链 LXO38E）。

**应用 patch**：写新 plan 抄 vendor 时，pre 必加一项检查清单："已读 Examples/<对应模块>/main.c 完整调用序列"。

**lesson 候选 2：sg "SD OK" sign-off 是假阳性，identification 阶段成功 ≠ data path 工作**

sg post 我用"屏显 SD OK + Capacity/Type/RCA/CID 4 个 datapoint"作为多论据 sign-off。**但 4 个 datapoint 全在 identification 阶段**（CMD2/9/13），CMD17 数据路径根本没测到。这是**多论据原则的反例**：4 个论据全在同一层（识别层），不算多角度互证。

**应用 patch**：硬件 bring-up 类任务，sign-off 必须包含**至少 1 个端到端数据 round-trip**（写 → 读回 → 比对，或读已知 magic）。光识别 register 不算。

**lesson 候选 3：三步硬件诊断法第二次成功应用（LXO38V）**

从 mount=1 到根因（sd_io_init 序列缺失）只走了 1 个 build cycle，得益于：
- 第 1 步隔离：raw sd_block_read 把"FatFs glue bug"和"SDIO 数据路径 bug"分开
- 第 2 步定向：raw read 显 CMD_TIMEOUT → 立刻知道在 SDIO 命令层
- 第 3 步反推：grep KIT example main.c → 发现 sd_io_init wrapper

**对比 sg 之前的 banner 残影**：5 次 debug 失败 → "局部解决而忘检全局"。**这次反向应用得当**。

**lesson 候选 4：autonomous mode + default-yes self-recommended 验证**

我自决 sd_diskio.c 实现细节（alignment bounce buffer、ioctl GET_BLOCK_SIZE=1）+ ffconf.h 裁剪策略（关 EXFAT/MKFS、改 CODE_PAGE）→ 全部一次到位无返工。LTRYSL 在 vendor wrap 类任务上**适用度高**，因为多数决策有 vendor 文档默认值可参考。
