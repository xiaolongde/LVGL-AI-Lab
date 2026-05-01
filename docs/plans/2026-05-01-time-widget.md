---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# 时间显示 widget（LVGL label + RTC 数据流）

## 意图
把 RTC 接到 LVGL label，让屏上显示真实走动的时间 "12:34:56"。这是项目第一个"业务 widget"——验证 LVGL 应用层 + 数据源接入 + 定时刷新的链路。

方法论价值：AI 在"如何高效更新 LVGL widget（避免每秒全屏重绘 / 用 lv_label_set_text vs `set_text_fmt` / 是否要节流）"这件事上的取向，是 LVGL 应用层 capability 的重点观察点。

## 验收
- ✅ `src/app/widget_time.c/h` 暴露 `widget_time_create(parent_obj) / widget_time_update()`
- ✅ widget 内部用一个 lv_timer 每秒调 rtc_get_time + 更新 label 文本
- ✅ 实机：屏中央显示 "HH:MM:SS"，秒钟跳动平滑
- ✅ 性能 sanity：CPU 占用率 < 5%（用 idle hook 统计）
- ✅ session post：lv_timer vs RTOS task 的取舍 / 字体大小对 partial buffer 的影响 / 重绘抖动观察

## 计划步骤
1. session pre：AI 决定 widget 接口形态 + lv_timer 周期 + 字体选择
2. 写 widget_time.h 接口
3. 实现 widget_time.c（含 lv_label 创建 + lv_timer 注册）
4. 选字体（LVGL 内置 montserrat 14/24/40 等，或自定义）—— 24 或 28 比较合适
5. 在 main 替换之前的 hello world，改成创建 time widget
6. 编（看 size 是否仍 OK——大字体会吃 Flash）
7. 烧录看实机
8. 录 1 分钟视频（看 :59 → :00 进位无抖动）
9. session post（重点：字体 size impact / lv_timer 精度 / 是否出现重绘 tearing）
10. commit

## 风险 / Open Questions
- partial buffer 5KB 时，重绘大字体 label 可能需要多次 flush——观察是否 tearing
- LVGL 默认字体只到 48px；若想做大表盘字体可能要内嵌字体生成（lv_font_conv）—— 留 v0.2
- lv_timer 精度依赖主 loop 调用 lv_timer_handler 的频率；保证 main loop 不被长期阻塞

## 依赖
- [[2026-05-01-lvgl-port-partial-buffer]]
- [[2026-05-01-rtc-source]]

## 方法论 hook
- Lessons 章节: `lessons/lvgl-widget-update-pattern.md`（怎样高效更新 widget）
- Capability report 维度: LVGL 应用层（widget 设计 + 数据流）
