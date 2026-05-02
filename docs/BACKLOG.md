# LVGL-AI-Lab BACKLOG

> 需求池。新增用追加格式：`- [[plans/YYYY-MM-DD-<slug>]]`

## v0.1 起跑前已完成
- 立项 (effort-bootstrap) + design v0 APPROVED
- KIT 资料解压 + `docs/references/{INDEX, mcu, board, screens, sdk}.md` 建立

## v0.1 ✅ 完成 (2026-05-02)
- [x] [[plans/2026-05-01-card1-minimal-debuggable]] — Card 1: CMake + GCC + pyOCD + GDB
- [x] [[plans/2026-05-01-card2-lvgl-watch-face]] — Card 2: LCD bring-up + LVGL 移植 + RTC + 仿 GT3 watchface

## v0.2 待开发

### Card 3: SD 卡 + FATFS + GUI 资源动态加载
- [[plans/2026-05-02-card3-sd-fatfs-gui-resources]] — sg sdio + sh fatfs + si lv_fs + sj font conv + sk 大字 watchface + sl multi-theme

### Card 4: Hands-off 自动化（design.md v0.2 北极星）
**待写 plan** — GDB 周期 halt + 读 partial buffer / sysmon label / PC 异常检测，**消除"用户报屏视觉"** 这个最大的 user-in-the-loop。验证实验：清空 conversation context AI 重做 LVGL 工程 0 介入。

### Card 5: 性能 multi-evidence FPS 测试
**待写 plan** — 应用 lesson LTIRBE 多论据，测 (a) SPI bound (b) CPU compute bound (c) heap pressure 三场景 FPS。当前 v0.1 只测了 (a)。

## 进行中

## 已完成
- ✅ Card 1 (st1-st4): 工程 + 烧录 + GDB
- ✅ Card 2 (sa-sd): LCD + LVGL + RTC + GT3 watchface
- ✅ sf-card2-retro: lessons + capability-report v0.1
