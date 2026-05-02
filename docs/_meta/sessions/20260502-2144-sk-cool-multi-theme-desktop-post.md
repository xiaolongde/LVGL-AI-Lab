---
type: session-post
task: sk-cool-multi-theme-desktop
pre: 20260502-2144-sk-cool-multi-theme-desktop-pre.md
ended: 2026-05-02T22:06:04+08:00
outcome: success
---

## 实际遇到

预判 #2 严重命中（widget 数 OOM），其他都没成问题。

**意料外**：
- **第一次 cyberpunk theme 用 22 条 lv_obj scanline + 4 嵌套 chip box（每 chip 3 widget）= 30+ widget 直接 boot 卡死**。屏只显示 KIT TFTLCD 的"v0.2 sk: cool desktop"，LVGL 第一帧 flush 没发生（如果发生了应该覆盖那行字）。
- **诊断 8 阶段 LCD_ShowString progress log** 定位到"[8] first theme draw" 后挂死。砍 cyberpunk 到 3 widget (bg + time + tag) 就跑通 → OOM 确认。
- **设计太土反馈**：第一版 cyberpunk 霓虹粉/青/黄/紫 4 色 chip + scanline，被用户嫌"3 个都土"。重设计：A 改 mono terminal-green ASCII art / B 改单 accent 蓝色克制风 / C 加 lv_arc 半弧电池条 anchor。

**预判内**：
- 卡点 #1 Flash 紧但能装：252K / 256K (96.26%)
- 卡点 #2 LV_MEM_SIZE：保 24K 不动，靠"减少 widget 数"修，没升到 32K（RAM 风险）
- lv_obj_clean 在切换时正常释放，无泄漏

## 修复路径

**OOM 修复（根因 = widget 数 × 200B > LV_MEM_SIZE 余量）**：
1. cyberpunk 砍 22 scanline → 5 stripes（仍 OOM）→ 0 stripes（minimum probe），证实 OOM
2. 重设计 chip：移除嵌套 lv_obj_create box，改单 lv_label + LV_SYMBOL 内置图标 → 每个 stat 1 widget (从 3)
3. 装饰用 ASCII 字符画在单 label（terminal 主题的 "BAT [#####...]"  bar），不用 widget

最终 widget budget：
- A TERMINAL: 4 widgets (prompt + clock + bar label + stats label)
- B PIXEL: 7 widgets (date + battery + clock + dot + 2 stats)
- C ZEN: 5 widgets (day + clock + sub + arc + bat label) — 含 1 个 lv_arc

**视觉重设计**：
- A → 纯黑 + terminal-green 单色 + ASCII bar 装饰 + prompt-style 字符
- B → 软深灰 #101418 (不全黑) + 单 accent 蓝 #4FC3F7 + 灰阶层次 + 排版克制
- C → 黑 + 玫瑰金半弧（lv_arc 0-100% 表电池）

Flash 252168 B (96.19%) RAM 39208 B (79.75%)。lv_arc 在 partial buffer 模式正常工作，没出 sd 时 transform_scale 那种 bug。

## 学到的（方法论素材）

**lesson 已 sediment 3 条**（meta scope）：
- LEZSTA: LVGL widget 数量是 LV_MEM_SIZE 隐性约束，装饰用 ASCII/canvas 而非堆 widget
- LEZSTN: lv_label_set_text_fmt 在 LV_MEM_SIZE 紧时静默失败
- LEZSTS: 嵌入 blob + 首次启动 f_write 自装 = hands-off SD 资源安装模式（sj 已应用，sk 复用）

**capability 提升**：
- 这是 LVGL OOM 第 5 次踩 widget 数量坑（前 4 次：sb sysmon+spinner / sd transform partial / sj binfont 边缘 / sk 装饰）。lesson 已正式 sediment，不应再撞。
- v0.2 北极星"hands-off"再次实证：sk 全程零 SD 拆卡（font 由 sj 自装继续生效）

**autonomous mode 评估**：
- self-recommend 视觉风格 → 第一版"土"被打回，**可改进**：未来设计任务，先给 3 个简短 mood board 风格选项让用户拍板，不要 self-decide 美学
- 故障定位用三步硬件诊断法（progress log + 砍到最小复现）30min 解决，效率 ✓

**未做但应记**：
- v0.3 候选：再生成 18px / 14px 字体 .bin 让 modular/zen 用更小副字（避免 LVGL 内置 Montserrat 14 跟 SD 28 字体风格不一致）
- v0.3 候选：把 ASCII bar 替换为 lv_canvas 真正画线（更像设计师做的）
- v0.3 候选：接真 RTC（已有 src/bsp/rtc.c），fake hh:mm 换真时间
