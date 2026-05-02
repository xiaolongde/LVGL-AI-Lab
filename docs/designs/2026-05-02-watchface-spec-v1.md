---
status: in-progress
project: LVGL-AI-Lab
type: design
updated: 2026-05-02
---

# Watch Face Spec v1 — 仿华为 GT3（横屏 320×172 适配）

Figma 等价标注。每个元素精确到 px / hex / font / pad，实现时直接照单。

## 设计理念

GT3 主表盘核心元素 → 320×172 横屏适配：

| GT3 圆屏元素 | 横屏适配 |
|---|---|
| 中央 HH:MM 大字 | 中央，但用 HH:MM:SS（横屏宽足够） |
| 顶端日期 + 周 | 顶端中央 "SAT · MAY 02" |
| 外圈 3 同心环 | 横屏拆为 左 步数 ring + 右 心率 ring |
| 底部 status icon | 底部秒进度条（细线条） |

视觉权重：**时间 (50%) > ring (30%) > 日期/秒条 (20%)**

## Color Palette

| Token | Hex | 用途 |
|---|---|---|
| `BG` | `#0A0E1A` | 屏背景（深蓝黑，比纯黑高级，含一丢丢蓝调） |
| `TRACK` | `#1F2333` | ring/bar 暗轨道色 |
| `ROSE` | `#D4A373` | 玫瑰金 — GT3 标志色（步数 ring）|
| `GREEN` | `#4AD98E` | 心率 / 健康（鲜绿，比 LVGL 默认 #00FF00 不刺眼） |
| `BLUE` | `#5AC8FA` | 秒条（iOS 风浅蓝青）|
| `TXT` | `#FFFFFF` | 主文字 |
| `DIM` | `#8C8C9E` | 次要文字（日期、单位）|

## Typography

LVGL 默认 Montserrat 14（Flash 只够这一个字体）。**视觉层级靠 transform_scale + 颜色**而非多字号。

| Use | Font | Scale | Effective | Letter spacing | Color |
|---|---|---|---|---|---|
| 中央时间 HH:MM:SS | Montserrat 14 | 2.0× | ~28 px | +1 px | `TXT` |
| Ring 中心数字（6500 / 72） | Montserrat 14 | 1.0× | 14 px | 0 | `TXT` |
| Ring 单位（STEP / BPM） | Montserrat 14 | 0.7× | ~10 px | 0 | `DIM` |
| 顶部日期 SAT · MAY 02 | Montserrat 14 | 1.0× | 14 px | +1 px | `DIM` |

**为什么 transform_scale 2.0× 而不是 2.5×**：2.5× 把 14 px 放到 35 px，软件双线性插值会模糊；2.0× 到 28 px 在 ST7789 物理像素上较锐利。

## Layout（绝对坐标，320×172）

```
 0           80         160         240        320
 ┌───────────┬───────────┬───────────┬─────────┐
 │                                              │  ← y=0
 │   • • •  SAT · MAY 02  • • •                │  ← y=4   小字 dim
 │ ───────────────────────────────────────────  │  ← y=22  分隔线（虚拟）
 │                                              │
 │  ╭─────╮                          ╭─────╮   │  ← y=40
 │  │  6500│       12:34:56         │  72 │   │  ← y=86  中央
 │  │ STEP │       (28px white)     │ BPM │   │
 │  ╰─────╯                          ╰─────╯   │  ← y=116
 │   ROSE                              GREEN    │
 │                                              │
 │ ───────────────────────────────────────────  │  ← y=160
 │           ▓▓▓▓▓▓▓░░░░░░░░░░░░                │  ← y=164  秒条 BLUE
 └──────────────────────────────────────────────┘  ← y=172
```

### 元素精确标注

| 元素 | 类型 | 几何 | 样式 |
|---|---|---|---|
| `screen` | 容器 | 320×172 | bg `BG` / pad 0 |
| `date_label` | label | x≈110 y=4 / w 自适应 / "SAT · MAY 02" | font 14 / color `DIM` / letter_space 1 |
| `ring_step` | arc 容器 | x=20 y=46 / 80×80 | track `TRACK` / indicator `ROSE` / arc_width 4 / range 0-100 / value 65 / 270° (135→45 顺时针) |
| `ring_step.value` | label 子 | center y_offset=-4 / "6500" | font 14 / `TXT` |
| `ring_step.unit` | label 子 | center y_offset=+10 / "STEP" | font 14 / `DIM` |
| `ring_hr` | arc 容器 | x=220 y=46 / 80×80 | indicator `GREEN` / value 72 / 余同 step |
| `ring_hr.value` | label 子 | center y_offset=-4 / "72" | font 14 / `TXT` |
| `ring_hr.unit` | label 子 | center y_offset=+10 / "BPM" | font 14 / `DIM` |
| `time_label` | label | center / "12:34:56" | font 14 transform 2.0× / `TXT` / letter_space 1 |
| `sec_bar` | bar | bottom-mid y_offset=-6 / 220×3 | track `TRACK` / indicator `BLUE` / radius 2 / value=second*100/59 |

### 关键间距

- 顶部 padding: 4 px
- 底部 padding: 6 px
- ring 与屏边距: 20 px
- ring 与中央时间间距: ~20 px（80 ring 边缘 100，时间区中心 160 → 50 px gap → 时间宽 ~140 px → 时间边到 ring 边 ≈ 10 px，紧凑但 OK）
- arc track 起止角度: 135° → 45°（顺时针），开口在底部 = GT3 风（开口向下让用户视线引导到下方）

## 实现注意

1. `lv_arc_set_bg_angles(arc, 135, 45)` —— LVGL 角度系统：0° 在 3 点钟方向，顺时针。135° 在左下，45° 在右上。270° 弧。
2. `lv_obj_remove_style(arc, NULL, LV_PART_KNOB)` —— 隐藏 knob handle（GT3 ring 无拖拽）
3. `lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE)` —— 静态 ring 不响应触摸
4. `lv_obj_set_style_arc_rounded(arc, true, ...)` —— 圆头端点（GT3 视觉特征）
5. transform_scale 2.0× 用 `LV_SCALE_NONE * 2` = `256 * 2 = 512`
6. letter_space 用 `lv_obj_set_style_text_letter_space(label, 1, 0)`

## 验收

- [ ] cmake build pass
- [ ] flash + reset → 屏视觉与本 spec mockup 大致一致
- [ ] 时间居中清晰可读
- [ ] 两 ring 对称美观
- [ ] 秒条平滑递增
- [ ] 用户视觉 sign-off "像 GT3 风"
