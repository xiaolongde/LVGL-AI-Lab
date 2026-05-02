---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-02
---

# Card 3 (v0.2): SD 卡 + FATFS + GUI 资源动态加载

## 意图

突破 v0.1 的两大约束：
1. **256K Flash 上限** — 字体、图片、themes 都挤在 Flash 里 (94% 占用，加任何资源都超限)
2. **GUI 资源静态** — watchface 切换需要重新烧录

引入 SD 卡 + FATFS 后：
- 字体（Montserrat 28/40/48 等大字号）从 SD 加载
- 表盘背景图、icon 集 从 SD 加载
- 多个 watchface 配置文件（theme1.json / theme2.json）+ 运行时切换

这是 v0.2 主线第一张卡，也是 v0.2 hands-off 自动化的前置（lv_canvas screenshot 落 SD 让 AI 自查屏内容）。

## 前置已就绪
- ✅ 板上 SD 卡槽 SDIO 4-bit 接口（PC8-12 + PD2，引脚已记 references/board.md）
- ✅ GD32F30x firmware lib 含 sdio.{c,h}
- ⚠ 需要：FATFS（开源 R0.15+），LVGL 的 lv_fs 适配层

## 验收
- ✅ SD 卡 mount 成功（FATFS 识别 FAT32 / exFAT）
- ✅ `f_open` / `f_read` / `f_write` 基础读写 work（用 GDB 验证读到正确字节）
- ✅ LVGL `lv_fs_drv` 注册 + 用 `S:/font.bin` URI 能加载 SD 上的字体文件
- ✅ Demo: SD 卡上 `font_montserrat_28.bin` (lv_font_conv 离线生成) 加载成功，watchface 时间用 28px 显示
- ✅ 实机视觉确认：屏上时间字明显比 v0.1 14px 大
- ✅ session 累计 ≥4 对 pre/post

## Sub-task（daily MVP 颗粒度）

| Day | Sub-task | Demo 形态 |
|---|---|---|
| D1 | sg-sdio-bringup | SD 卡通电检测 + GDB 验证 SDIO 寄存器 + log "SD card OK" |
| D1 | sh-fatfs-port | FATFS 移植 + 挂载 SD 卡 + `f_read` 读 README.txt 内容到内存 (GDB read) |
| D2 | si-lvgl-fs-drv | 写 `lv_port_fs_sd.c` 注册 lv_fs_drv，URI prefix "S:" |
| D2 | sj-lv-font-conv-pipeline | 离线脚本（Python or 用户跑 lv_font_conv）生成 montserrat_28.bin → 放 SD |
| D2 | sk-watchface-large-font | watchface 时间从 SD 加载 28px 字体显示，与 v0.1 14px 对比 |
| D3 | sl-multi-watchface-config | SD 上 `themes/gt3.cfg` / `themes/minimal.cfg` 各定义不同 layout，开机读 cfg 选 watchface |

## 风险 / Open Questions

1. **SD 卡硬件初始化的兼容性**：板上是 SDIO 4-bit 接口。GD32 SDIO 寄存器与 STM32F4 兼容但与 STM32F1 略不同（GD32F303 借 STM32F1 但加了 SDIO 上去）。Plan B：失败的话退到 SPI mode SD 卡（速度慢但简单）。
2. **FATFS 占用 Flash**：估算 +15-20K Flash + ~2K RAM。当前 Flash 94% 已极限。**必须先关 sysmon (-6K) 才能上**。
3. **lv_fs 与 LVGL v9 API**：v9 改了 fs API 与 v8 不同。需查 lvgl/examples/porting/lv_port_fs_template.c。
4. **lv_font_conv 工具链**：node.js npm 包，需 user 跑或 AI 装（pip-like）。可能 v0.2 这一段需要 user 介入 1 次（hands-off 不完整）。
5. **SD 卡读速度对 watchface FPS 影响**：每次切 watchface 需读字体到内存？若 LVGL cache 字体，仅启动期慢。需测。
6. **SD 卡 hot-swap**：v0.2 不支持，要求 SD 卡常驻。

## 依赖
- [[2026-05-01-card2-lvgl-watch-face]] (v0.1 LVGL 已 work)

## 方法论 hook
- Lessons 候选: `lessons/sdio-bringup-on-gd32f303.md`, `lessons/fatfs-lvgl-fs-integration.md`, `lessons/lv_font_conv-mini-font-pipeline.md`
- Capability report 维度: 文件系统层（SDIO + FATFS）+ LVGL 资源动态加载
