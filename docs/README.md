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

## Sessions Discipline（B 路径双轨记录）

本项目对 AI 写代码强制 **pre/post 自评**——每个 sub-task 动手前写 pre（任务理解 + 计划 + 卡点预判），完成后写 post（实际遇到 + 修复路径 + 学到的）。pre/post 文件存到 `docs/_meta/sessions/`，commit message 必须含 `session: <slug>` 关联。

零摩擦工具：

```bash
tools/session.sh new <slug>     # 任务开始前：创建 pre 文件骨架
# ...动手做事 + commit（commit msg 含 session: <slug>）
tools/session.sh close <slug>   # 任务完成后：创建 matching post 文件
tools/session.sh list           # 列出所有未 close 的 pre（提醒）
```

Smoke gate 强制：

```bash
node tests/smoke-test.js
```

阶段 1 检查 sessions 完整性：每个触及 `src/` `tests/` `tools/` 的非 merge commit 必须含 `session: <slug>` + 对应 pre/post 双件齐。纯 `docs/` commit 豁免。阶段 2/3 在 CMake 工程建立后激活（构建 size + 静态分析）。

详细约定见 [`../CLAUDE.md`](../CLAUDE.md)。
