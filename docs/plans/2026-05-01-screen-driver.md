---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# 屏驱动层封装（区域刷新接口）

## 意图
颜色块跑通后，把屏访问抽象成 LVGL 期望的接口形态：`flush(area, color_buf)` —— 给一块矩形区域 + 像素数据，写到屏。这层封装是 LVGL 移植的契约面，做不好后面 LVGL 集成会反复返工。

方法论价值：AI 在"为已有底层加一层抽象 + 接 GUI 框架契约"这件事上是否能不偷懒（如：是否会忘记 set_window 命令、是否处理边界 case）。

## 验收
- ✅ `src/drivers/<ic>/screen.h` 暴露三个 API：`screen_init() / screen_flush(x1,y1,x2,y2,buf) / screen_fill(x1,y1,x2,y2,color)`
- ✅ `screen_flush` 使用屏 IC 的 column/page address set + memory write 命令，带边界检查
- ✅ 单元测试（用 mock SPI）：写 1 像素、写一行、写一矩形、越界 reject
- ✅ 实机：用 `screen_fill` 画 5 个不同位置不同大小不同颜色的矩形（验证坐标准确）
- ✅ 录屏入 qa-report
- ✅ session post：抽象 API 设计的取舍点（同步 vs 异步、buf 字节序、错误返回值）

## 计划步骤
1. session pre：AI 列出 LVGL `disp_drv.flush_cb` 期望的契约 → 反推 screen API 形态
2. 写 `screen.h` 接口
3. 写 mock SPI + 单测（host 侧编译运行，不上板）—— 这是 v0.1 第一次有真测试
4. 实现 `screen.c`
5. 单测过
6. 写实机矩形 demo 程序
7. 烧录看矩形位置准
8. 录屏 + qa-report
9. session post
10. commit（可拆成 2 commits：单测 + 实现）

## 风险 / Open Questions
- mock SPI 的设计：用条件编译 `#ifdef HOST_TEST` 还是 link-time 替换？需在 pre 决定
- LVGL 的 flush_cb 期望 buf 是 lv_color_t 数组（默认 16-bit）；屏要 RGB565 字节序——可能需要字节序转换 / DMA byte-reverse
- 是否暴露 backlight 控制 API（如果板子有 PWM 背光），v0.1 先 fixed on

## 依赖
- [[2026-05-01-spi-screen-selection]]

## 方法论 hook
- Lessons 章节: `lessons/abstraction-layer-for-gui-port.md`（如何给 LVGL 准备适配层 + AI 在此处的偷懒模式）
- Capability report 维度: 驱动层（抽象 + 单测）
