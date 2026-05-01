# LVGL-AI-Lab BACKLOG

> 需求池。新增用追加格式：`- [[plans/YYYY-MM-DD-<slug>]]`
>
> v0.1 拆分为 10 项，按执行顺序排列（infra 先行，因 CLAUDE.md 硬约束逼方法论工具先就位）。

## 待开发

### v0.1 — 种子 case（GD32F303 + 240x240 SPI 静态表盘）

**方法论 infra（必须最先）**
- [[plans/2026-05-01-pre-post-automation]] — 零摩擦 pre/post 自评工具 + smoke-gate sessions 完整性检查
- [[plans/2026-05-01-flash-record-flow]] — 烧录 + 录屏 + qa-report 模板（嵌入式版"重启 server"）

**Bring-up 链**
- [[plans/2026-05-01-led-bringup]] — 板载 LED 闪 / 工具链端到端验证（hello-world）
- [[plans/2026-05-01-spi-screen-selection]] — 屏选型 + SPI bring-up + 出第一像素（颜色块循环）

**驱动 + GUI 框架**
- [[plans/2026-05-01-screen-driver]] — 屏驱动抽象（screen_init / flush / fill）+ host mock 单测
- [[plans/2026-05-01-lvgl-port-partial-buffer]] — LVGL submodule + lv_conf 裁剪 + partial buffer 在 48KB SRAM 实证

**应用层 widget**
- [[plans/2026-05-01-rtc-source]] — RTC + BKP 真时间源（GD32F303 沿袭 F1 RTC 的特殊坑）
- [[plans/2026-05-01-time-widget]] — LVGL label + RTC 数据流 + lv_timer 每秒刷新
- [[plans/2026-05-01-watch-face-layout]] — 表盘 layout：date + time + battery（v0.1 demo 终态）

**Retro（关闭 v0.1）**
- [[plans/2026-05-01-first-weekly-retro]] — 抽 lessons + capability-report v0.1 + 跨项目 Cards 候选清单

## 进行中

## 已完成
