# LVGL-AI-Lab

> 用"完全 AI 构建一块基于 GD32F303 + LVGL 的智能手表"作为 case study，反向沉淀**AI + 嵌入式 GUI 开发最佳实践方法论**（Playbook + 脚手架 + 能力边界报告 三件套）。手表是载体，方法论是真正交付物。

## 状态
当前阶段：立项；v0.1 进行中（种子 case：GD32F303 + 240x240 SPI 静态表盘）

## 平台
- MCU: GD32F303RCT6（Cortex-M4F, 256KB Flash, 48KB SRAM, 120MHz, 无 2D 加速）
- 屏: 1.5"-2.4" 240x240 / 320x240 SPI（无触控）
- LVGL: partial buffer (5-10KB) 模式

## 路径选择
**Approach B — 双轨记录强方法论**：每个 AI 子任务前后强制写 pre/post 自评到 `docs/_meta/sessions/`，每周用户 retro 把素材抽到 Playbook 章节。详见 [`docs/designs/2026-05-01-AI嵌入式GUI方法论-design.md`](designs/2026-05-01-AI嵌入式GUI方法论-design.md)。

## 设计文档
<!-- 自动维护：由 /office-hours 产出后刷新 -->
- [[designs/2026-05-01-AI嵌入式GUI方法论-design]] — 项目 v0 设计（APPROVED 2026-05-01）

## 实现计划
<!-- 自动维护：由 writing-plans 产出后刷新 -->

## 自测
<!-- 自动维护：最近 QA 报告链接 -->
