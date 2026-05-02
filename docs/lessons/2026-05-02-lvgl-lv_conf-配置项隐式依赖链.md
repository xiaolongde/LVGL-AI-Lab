---
status: backlog
project: LVGL-AI-Lab
type: lesson
updated: 2026-05-02
lesson_id: LXO38E
trigger_when: LVGL lv_conf 配置项隐式依赖链
keywords: ["LVGL", "lv_conf", "dependency", "config"]
scope: project
evidence_count: 1
last_triggered: 2026-05-02
revision_history:
  - { date: 2026-05-02, action: created, source: sediment-manual, commit: pending }
card_link: null
---
# LXO38E — LVGL lv_conf 配置项隐式依赖链
**Rule:** 开 LVGL feature 前 grep `LV_USE_<feature>` 在 LVGL src 出现的所有位置看依赖。某些功能模块有"开关-X-需要-Y"的依赖：PERF_MONITOR/MEM_MONITOR 都依赖 LV_USE_SYSMON；arc/spinner 依赖 LV_DRAW_SW_COMPLEX；slider widget 依赖 bar widget 的 enum；等等。
**Why:** sb 期间开 PERF_MONITOR 不显示 (缺 SYSMON 依赖) + spinner 不显示 (缺 SW_COMPLEX 依赖)，每次 build 一次浪费 30 秒+用户来回 1 分钟。

**How to apply:** 加任何 LV_USE_X / LV_FONT_X 之前：(1) grep "LV_USE_$X" LVGL src 看是否被其他地方 require (2) 看附近的 #if LV_USE_$X == 1 块下面那行 #define 是否启用了未启的依赖。**配置不是孤岛**。

**Evidence:**
- sd PERF_MONITOR→SYSMON 依赖
- sb spinner→DRAW_SW_COMPLEX 依赖