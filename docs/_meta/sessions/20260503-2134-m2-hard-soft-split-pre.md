---
type: session-pre
task: m2-hard-soft-split
backlog_item: v0.3-pc-dev-pipeline
started: 2026-05-03T21:34:02+08:00
---

## 任务理解

最初理解为代码层"硬装/软装"分离：硬装=控制层（state/timer/loop），软装=渲染层（颜色/字体/layout）。后被用户纠正：硬装/软装实为产品节奏术语（MVP 大概版 vs 逐页精雕）。本 session 实际做的是代码层组件分离（仍有价值——解耦控制逻辑和渲染），但不是用户原意。m3 才进入真·软装迭代。

## 计划

1. desktop_logic.{c,h}：state + 主循环 + theme 轮播 timer
2. themes/themes.h + theme_terminal.c / theme_pixel.c / theme_zen.c
3. main.c 收成 boot + 注入 theme array

## 卡点预判

1. font_big 共享 → extern 暴露
2. theme_descriptor_t 接口 = name + render fn
3. main.c 无平台特定调用

## 验收

- [ ] 3 个 theme 文件分离独立编辑
- [ ] desktop_logic 无 LVGL widget 创建
- [ ] host build 仍能跑
