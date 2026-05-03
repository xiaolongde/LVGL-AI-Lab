---
type: session-pre
task: m3-soft-style-from-sd
backlog_item: v0.3-pc-dev-pipeline
started: 2026-05-03T21:42:34+08:00
---

## 任务理解（最终对齐版）

**第一版（错）**：把软装数据化 — 每个 theme 持有 theme_style_t（colors + 2 strings），从 .tstyle 文件加载。改文件即改风格。

**第二版（错）**：被用户指出"只是参数化颜色不算软装"，提议升级为 declarative UI engine：layout 也数据化，widget 类型/位置/大小全在 .tstyle。

**第三版（对）**：用户纠正——软装/硬装是产品节奏术语：硬装=MVP 大概版（sk 已交付）；软装=逐页精雕细琢。m3 应该是**进入软装迭代周期**：选一个页面（ZEN）开始用 PC 高频迭代细节。

## 计划

1. 实现 theme_style_t（colors + banner/sub_text）+ key=value parser — 这是辅助工具，让颜色/字符串可外置
2. .tstyle 文件 hands-off 自装到 SD（v0.2 北极星模式延续）
3. 启动软装迭代 ZEN：字距、arc 形态、状态色、视觉重心

## 卡点预判

1. **术语理解可能仍偏**：跑前确认 — 实际跑了 3 轮才对齐
2. **孤立颜色化不够"软装"**：用户期望细节精雕（动画/排版/装饰），不只是颜色
3. **MCU Flash 紧**：增加 parser + 3 个嵌入 default 字符串 估 +2K，从 252K 起，能装

## 验收

- [ ] .tstyle 系统在 host + MCU 都工作
- [ ] ZEN 软装迭代有显著视觉差异（>1 轮）
- [ ] post 含术语理解错的 lessons
