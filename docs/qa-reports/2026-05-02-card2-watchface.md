---
type: qa-report
project: LVGL-AI-Lab
date: 2026-05-02
backlog_item: card2-lvgl-watch-face
status: passed
---

# QA Report — Card 2 GT3 Watchface (v0.1)

## 测试环境
- 板：GD32F303RxT6 KIT V5.1
- 屏：ST7789V3 1.47" 320×172 SPI
- 烧录：ST-Link/V2 + pyOCD 0.44.0
- 软件：LVGL v9.5.0, GCC arm-none-eabi 12.2

## Build 信息
- Commit: 7de714c (`feat(watchface): 仿华为 GT3 表盘 + design.md v0.2 hands-off 北极星 (sd)`)
- Flash: 240980 B / 256K (94.13%)
- RAM: 29344 B / 48K (71.64%)
- Smoke: 3/3 PASS

## 视觉 Sign-off

按 `docs/designs/2026-05-02-watchface-spec-v1.md` 验收清单：

| 元素 | Spec | 实测 | Pass |
|---|---|---|---|
| 背景色 | `#0A0E1A` 深蓝黑 | 显示深色（用户视觉确认） | ✅ |
| 顶部日期 | "SAT MAY 02" 浅灰 14px | 显示 ✅ | ✅ |
| 左 ring 步数 | 玫瑰金 70×70, 65%, 中心"6500/STEP" | 显示 ✅ | ✅ |
| 右 ring 心率 | 绿色 70×70, 72%, 中心"72/BPM" | 显示 ✅ | ✅ |
| 中央时间 | 14px 白字 + 玫瑰金边框 120×40 card | 显示 ✅ | ✅ |
| 底部秒条 | 蓝色 220×3 0..100% 循环 | 每秒递增可见 | ✅ |
| 全屏完整 | 无残影 / 无半屏 | 无 ✅ | ✅ |
| 时间走时 | RTC LSE 1Hz 秒级递增 | 走 ✅ | ✅ |

## 已知偏离 spec

1. **大字时间**：spec v1 写"14px transform_scale 2.0× → 28px effective"，实际用 14px + framing card（无 transform）。**原因**：LVGL v9.5 transform_scale + partial buffer 不兼容（4 次实证），BFS backtrack 路径。**Workaround**：玫瑰金边框 card 把 14px 字提升视觉权重。**v0.2 path**：custom mini font 0-9+":" 或 lv_canvas 自画。

2. **日期硬编码**：日期文字"SAT MAY 02"是固定字符串，未接 RTC 日历。**原因**：GD32F30x RTC 是 32-bit counter 不带 calendar，需软件转日期。**v0.2 path**：BKP magic 持久化 + 软件 calendar。

## Sub-task 进度

- [x] sa: ST7789V3 1.47" 屏 bring-up
- [x] sb: LVGL v9.5.0 移植 + hello world
- [x] sc: RTC LSE + 时间显示 + sysmon overlay
- [x] sd: 仿 GT3 表盘 layout + Figma-spec
- [x] sf: lessons + capability-report v0.1（本次 retro）

## 用户介入次数（hands-off 评估）

13 次（详见 capability-report v0.1）。其中 4 次"用户报屏视觉" → v0.2 自动化目标。

## 视频

(实机录屏文件在用户手机上，未上传仓库 — v0.2 上 SD 卡作为存储后可保存到板载 SD)
