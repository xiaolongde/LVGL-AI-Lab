---
status: backlog
project: LVGL-AI-Lab
type: lesson
updated: 2026-05-02
lesson_id: LXO38A
trigger_when: LVGL v9 transform_scale + partial buffer mode 不兼容
keywords: ["LVGL", "transform_scale", "partial-buffer", "half-screen", "workaround"]
scope: project
evidence_count: 1
last_triggered: 2026-05-02
revision_history:
  - { date: 2026-05-02, action: created, source: sediment-manual, commit: pending }
card_link: null
---
# LXO38A — LVGL v9 transform_scale + partial buffer mode 不兼容
**Rule:** transform_scale != 256 (1.0×) 触发 LV_LAYER_TYPE_TRANSFORM 路径 (lv_obj_style.c:1090)，与 partial buffer mode 在 LVGL v9.5.0 实证不兼容（症状：屏只刷一半）。短 BFS 路径：放弃 transform，改用 LV_FONT_MONTSERRAT_24/28 (Flash 允许时) / 自定义 mini font (lv_font_conv 工具) / lv_canvas 自画。
**Why:** sd 期间 4 次假设 (单 widget transform / 全屏 invalidate / SIMPLE_BUF_SIZE 调小 / px_map stride) 全部验错仍半屏；源码追到 LV_LAYER_TYPE_TRANSFORM 路径但未 fully 定位失败点。结论：当前 LVGL 版本配 partial mode 不要用 transform_scale，省 12+ 分钟反复试错。

**How to apply:** 需要"放大显示"效果的场景：(a) 直接换大字体 (b) custom font 仅含必需字符（lv_font_conv 离线工具，AI 当前不能跑） (c) lv_canvas 自画 path。**绝不用 transform_scale on label in partial mode**。如果 LVGL 升级到更高版本可重测。

**Evidence:**
- sd transform 4 次实验全半屏
- lv_obj_style.c:1090 源码标记 LAYER_TYPE_TRANSFORM