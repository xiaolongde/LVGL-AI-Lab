---
type: session-post
task: sd-watch-face
pre: 20260502-1213-sd-watch-face-pre.md
ended: 2026-05-02T13:14:58+08:00
outcome: partial
---

## 实际遇到

任务**部分完成** —— GT3 仿表盘 layout 上线，但**大字时间方案失败 4 次后 BFS backtrack** 到 14px + 边框 framing。

### 完成的
- 仿 GT3 layout：顶部日期 / 左玫瑰金步数 ring / 中央时间 card / 右绿色心率 ring / 底蓝色秒条
- Figma-spec 文档 (`docs/designs/2026-05-02-watchface-spec-v1.md`) 含每元素精确标注
- 配色 tokens 系统化（`COL_BG/TRACK/ROSE/GREEN/BLUE/TXT/DIM`）
- 中央时间用 120×40 玫瑰金边框 card 包 14px 字 → 视觉权重通过 framing 提升

### 失败的：transform_scale 大字时间
连续 4 次实验全部触发"屏只显示一半" bug：

| 实验 | 假设 | 结果 |
|---|---|---|
| 1. transform_scale 2.0× 单 widget | 配置 OK 应工作 | 半屏 ❌ |
| 2. + main loop 强制 invalidate 整 scr | 全屏 dirty 绕开 partial 计算 bug | 半屏 ❌ |
| 3. + LV_DRAW_LAYER_SIMPLE_BUF_SIZE 24K → 8K | 与 LV_MEM 16K 一致解 OOM | 半屏 ❌ |
| 4. flush_cb stride 改 disp_width row-by-row 写 | px_map 实际 stride ≠ area_width | 没真做（grep 源码确认 LVGL partial mode 下 layer->buf_area = area_p stride 应 packed） |

**Root cause 部分定位**（lv_obj_style.c:1090 → LV_LAYER_TYPE_TRANSFORM 路径），但**精确失败点未找**——LVGL v9.5 + partial buffer + transform 在我的 lv_port_disp 配置下不可用，未深入到 layer composite 像素流的 byte-level 调试。

### 用户介入 5 次（v0.1 残留）
- "只显示了一半" → 报现象
- "现在又变成只刷一半了" → 报二次失败
- "你定位了原因吗" → push 我从 isolation 升级到源码调研
- "你基于当前的小内存情况，做一个版本的全屏刷新" → BFS 建议
- "确认下，目前的这个屏幕是个 240*320 的" → 屏尺寸 verify
- "确认" → 最终 sign-off

每次介入都有信息价值，但**也证明 v0.1 不是 hands-off**。每次"用户报屏现象"都是 v0.2 应用 GDB 读 buf + 像素 hash 自动化的目标。

### 字体方案的空间约束（数据点）
| 字体 | Flash 增量 | 可装 |
|---|---|---|
| Montserrat 14（默认） | 0 | ✅ |
| Montserrat 24 | +25K（实测 268K 总）| ❌ 超 256K 物理上限 |
| Montserrat 28 | +33K（实测 277K）| ❌ 超 |
| transform_scale 2.0× （0 字体增量）| 0 | ❌ partial mode 不兼容 |
| 自定义 0-9+":" 11 字符 mini font | ~1.5K | ✅ 但需 lv_font_conv 工具（AI 当前不能跑） |
| lv_canvas 自画大数字（lv_canvas_draw_polygon）| 不确定 | v0.2 试 |

## 修复路径

未真修。**BFS backtrack** 到 14px + framing 方案，接受 v0.1 视觉妥协。v0.2 backlog 加：
- (a) 用 lv_font_conv 工具离线生成 mini font（用户跑工具或 AI 跑跨平台 Python 版）
- (b) lv_canvas 路径研究
- (c) transform_scale partial buffer 真根因 deep dive（可能 LVGL upstream issue + 提交 bug report）

## 学到的（方法论素材）

**新 lessons 候选**：

1. **LVGL v9.5 + partial buffer + transform_scale on label 不兼容**（实证 4 次失败）。Lessons 候选: `lessons/lvgl-v9-transform-scale-partial-mode-broken.md`。该 lesson 应该附 **触发条件**（transform_scale != 256 + partial buffer mode + LV_MEM_SIZE 偏紧）+ **替代方案表**（mini custom font / canvas / 大字体）。

2. **嵌入式 GUI 大字渲染的真实选项谱**（when font Flash budget 紧）：
   - default 字体 + framing 提升视觉重量（最 cheap）
   - mini custom font 只内嵌需要字符（lv_font_conv，1-2KB）
   - lv_canvas 自画（动画灵活，CPU 负载）
   - 多内置字号（Flash 重，最直接）
   Lessons 候选: `lessons/embedded-large-text-options-when-flash-tight.md`

3. **"事不过三 + BFS backtrack" 实战触发**（lesson LTIRBB + LTRYSL 联合应用）：
   - 第 4 次 transform 半屏失败时，触发 L3 灵魂拷问 + 强制 BFS
   - 没在原 transform 方向死磕（v0.1 推不下去就推不下去，v0.2 解）
   - 时间窗口：15 分钟内识别"必须放弃"，比之前 banner 残影 3 次错方向 30 分钟好

4. **AI 在 LVGL 源码追踪上的能力**：本任务 4 次假设全错，但每次都基于源码读（lv_obj_style.c / lv_refr.c / lv_draw.c），不是凭空猜。**capability report**: AI 在"读 vendor 源码定位 bug"能力中等——能找到相关代码，但 fully understand 多文件 layer/buffer/stride 交互链上限较慢，需要更多 deep dive 才能 100% 定位。

5. **autonomous mode 在长 BFS 链下的必要性**（lesson LTRYSL 应用）：用户开启 autonomous mode 后，AI 4 次 transform 失败 + 1 次 layout polish 都没询问"换路径吗"，直接 BFS。每个 cycle ~3 分钟（build/flash/reset/视觉确认），4 次 12 分钟内完成 BFS 决策——远比每次问用户快。

## 结果归档

- main.c：仿 GT3 layout，14px + framing 时间，Flash 240980 B (91.93%)，全屏正常
- design spec：`docs/designs/2026-05-02-watchface-spec-v1.md`
- v0.1 sd 视觉 sign-off ✅（虽然字小）
- transform_scale + Montserrat 24/28 不可用 (实证 4×)，留 v0.2 backlog

## 下一步

v0.1 Card 2 主线 ≈ 完成（sa+sb+sc+sd 全部）。剩 sf-card2-retro 收口，写 capability-report v0.1 + lessons 提取。
