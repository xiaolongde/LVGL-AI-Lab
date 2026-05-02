---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# Card 2: LVGL 支持 + 表盘 demo

## 意图
Card 1 把工程 + 调试链路立起来；本 card 让屏亮起来 + LVGL 跑起来 + 屏上显示真实走时的表盘。这是 v0.1 的最终 demo——拍出来一眼能识别为"手表"。

## 前置
- ✅ Card 1 完成（CMake/OpenOCD/GDB/LED/log 都通）
- ✅ `docs/references/screens.md` 已对屏选型 + ST7789V3 关键参数齐备
- ⚠ 屏选型：默认走 **1.14" ST7789V3 (240x135)**；实物若是 0.96" ST7735S 退路也可（KIT 自带两屏 example）

## 验收
- ✅ 屏 bring-up：上电后整屏循环纯红/纯绿/纯蓝填充（验证 SPI + DC/RST/CS 接线 + init sequence 全通）
- ✅ LVGL v9 移植成功：`lv_label("LVGL ok")` 居中显示，partial buffer 6KB，Flash < 200KB / RAM < 40KB
- ✅ RTC + LSE 时间走起来：`rtc_get_time(&h, &m, &s)` 返回单调递增；NRST 复位后时间继续（前提是有 RTC 后备电池或不掉电）
- ✅ Time widget：lv_label + lv_timer 每秒刷新，显示 "HH:MM:SS"，秒钟跳动平滑无 tearing
- ✅ 表盘 layout：屏中央大字号时间 + 上方 date label（硬编码 `2026-05-01`）+ 下方 battery bar（假数据 75%）
- ✅ 实机录 1 分钟视频（看 `:59 → :00` 进位 + 整体不抖）+ qa-report
- ✅ session 累计 ≥6 对 pre/post

## Sub-task（Daily MVP — 每天交付一个可烧录可见的最小 demo，2026-05-02 用户拍板）

应用 lesson LTIRB3（按天而非按周）：每个 sub-task 必须在 1 天内完成 + 给用户一个屏上能看到的可运行 demo（哪怕是临时的）。如果某项估计 > 1 天，强制再拆。

| Day | Sub-task | Demo 形态（屏上能看到的） |
|---|---|---|
| D1 | sa-lcd-bringup | KIT TFTLCD demo 字符显示 ✅完成 (2026-05-02) |
| D1 | sb-lvgl-port | LVGL hello world + 动态 tick label + bar ✅完成 (2026-05-02) |
| D1 | sc-rtc-source | HH:MM:SS 真时间走起来 + 全屏 spinner FPS 数据 ✅完成 (2026-05-02) |
| D2 | sd-watch-face-layout | 真表盘 layout：date 顶部 / 大字时间居中 / 电量条底部 | 待做 |
| D2 | se-fps-multi-evidence | 多论据 FPS 测试（SPI bound / CPU bound / heap pressure 三场景），lesson LTIRBE 应用 | 待做 |
| D2 | sf-card2-retro | Card 2 收口：抽 lessons / capability-report v0.1 / qa-report | 待做 |

(原 sub-task 2 "屏驱动抽象 + host mock 单测" 已 deferred 到 v0.2 — 按 LTIRB3 daily MVP 优先级)

## 原 7 步 sub-task（参考用，已被 daily 划分覆盖）

1. ~~屏 bring-up~~ → sa
2. ~~屏驱动抽象 + host mock~~ → defer v0.2
3. ~~LVGL submodule + lv_conf~~ → sb
4. ~~RTC~~ → sc
5. ~~Time widget~~ → 含在 sc
6. **表盘 layout** → sd
7. ~~录视频 + qa-report~~ → sf 含

## 风险 / Open Questions
- **partial buffer 6KB 够不够**：240×135 屏一行是 480B，6KB ≈ 12 行 buffer——足够 LVGL 工作但每屏需多次 flush；若 tearing 明显可加到 10-15KB（48KB 紧但够）
- **字体大小 vs Flash 占用**：内嵌 montserrat 28 含全部 ASCII 约 +20-30KB Flash，可只内嵌 0-9 + ":" 共 11 字符省一半
- **GD32 RTC 沿袭 F1 风格**（32 位计数器，需软件转 H:M:S，不像 F4 自带 calendar）—— 经典坑，方法论 datapoint
- **LVGL v9 vs v8**：v9 是最新但部分 example 没更新；如果 AI 决定回 v8 必须在 post 写明理由
- 屏 PWR 是 PMOS 控的（PB7=低有效），AI 容易写反

## 依赖
- [[2026-05-01-card1-minimal-debuggable]]

## 方法论 hook
- Lessons 候选: `lessons/lvgl-on-tight-sram.md`、`lessons/spi-screen-init-from-vendor-example.md`（重点：抄 vs 改写）、`lessons/gd32f1-style-rtc-quirks.md`、`lessons/lvgl-watch-face-layout.md`
- Capability report 维度: 驱动层（SPI + 屏 IC）+ LVGL 应用层 + 资源工程
