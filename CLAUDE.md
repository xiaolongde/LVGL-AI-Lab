# LVGL-AI-Lab — Claude Code 项目说明

## 项目性质
本项目是一次**方法论提炼实验**：用"AI 完整构建 GD32F303 + LVGL 智能手表"作为 case study，反向产出 AI + 嵌入式 GUI 开发最佳实践（Playbook + 脚手架 + 能力边界报告 三件套）。**真正的产物不是手表，是方法论**——必要时为方法论牺牲 demo 完整度。

设计 source of truth：`docs/designs/2026-05-01-AI嵌入式GUI方法论-design.md`（Approach B）。

## 技术栈
- **MCU**: GD32F303RCT6（Cortex-M4F, 256KB Flash, 48KB SRAM, 120MHz, 无 2D 加速器）
- **屏**: 1.5"-2.4" 240x240 / 320x240 SPI（无触控；具体型号 bring-up 时确定）
- **GUI**: LVGL（partial buffer 5-10KB；不能整屏 framebuffer）
- **工具链**: GCC ARM + GD32 官方 firmware library / 或 libopencm3（待实验阶段决策）
- **烧录**: J-Link / DAP-Link（待硬件确认）

## 角色定位（B 路径核心约束）
- **AI 全权做技术决策**：选库、定架构、写代码、查 datasheet、出 BOM 都是 AI 主导，**不要每一步都问用户**
- **用户角色**：烧录、看实机效果、提反馈、做周 retro、提炼方法论
- AI 遇到歧义时优先推进而不是停下来问，把"我假设了 X，理由 Y，如果不对回头改"写到 post 自评里

## 双轨记录硬约束（B 路径独有，**不可妥协**）

每个 AI 子任务（粒度 = 1 个 LVGL widget / 1 个驱动模块 / 1 个 BSP 步骤）必须配对产出：

### Pre 自评（任务开始前）
路径：`docs/_meta/sessions/YYYY-MM-DD-HHMM-<task-slug>-pre.md`

模板：
```markdown
---
type: session-pre
task: <slug>
backlog_item: <wikilink>
started: <iso datetime>
---

## 任务理解
<我以为这件事要做什么>

## 计划
<打算分几步、用什么方案>

## 卡点预判
<我猜哪几步会卡，预备的 plan B>
```

### Post 自评（任务完成后）
路径：`docs/_meta/sessions/YYYY-MM-DD-HHMM-<task-slug>-post.md`

模板：
```markdown
---
type: session-post
task: <slug>
pre: <pre 文件路径>
ended: <iso datetime>
outcome: success | partial | failed
---

## 实际遇到
<跟预判的差别、意料外的事>

## 修复路径
<怎么解决的；如果没解决，卡在哪>

## 学到的（方法论素材）
<这次经验里能抽到 Playbook / capability report 的点是什么>
```

### Smoke-gate 检查
**`docs/_meta/sessions/` 不齐 → 不允许 merge**。具体规则：
- 每个 commit 必须能映射到一对 pre/post（commit message 里写 `session: <slug>`）
- 缺 pre：阻断（说明没规划就动手）
- 缺 post：阻断（说明没沉淀就 merge）
- 触发位置：`tests/smoke-test.js` 的第一步（在跑功能 test 之前）

## Commit 颗粒度
- **1 个 LVGL widget** = 1 个 commit（最小可逆单元）
- **1 个驱动模块**（如屏 init / SPI DMA / 时钟源切换）= 1 个 commit
- **1 步 bring-up**（如：板子上电点亮 LED / SPI 通了 / 屏出第一像素）= 1 个 commit
- commit message 格式：`<type>(<area>): <一句话>` + 一行 `session: <slug>`
- 反例（禁止）：一次 commit 里既搞屏驱动又写表盘 widget

## 周 Retro 节奏
- 每周末用户 retro 一次：把当周 `docs/_meta/sessions/` 的 post 抽成 `docs/playbook/` 章节
- Playbook 章节命名：`docs/playbook/<topic>.md`（例如 `lvgl-partial-buffer-on-48kb-sram.md`）
- Capability report 章节：`docs/capability-report/<layer>.md`（按层：bring-up / 屏驱动 / LVGL 应用 / RTOS / 中断）
- retro 漏 1 周 → BACKLOG 阻断（不让起新任务）；漏 2 周 → 升级到用户

## 偏离全局方法论的覆盖项
- **改后端必重启**：本项目没有 server。改驱动后必须**烧录到实机验证 + 录屏**，等价语义。
- **部署一气呵成**：本项目"部署"= 烧录 + 录视频附到 qa-report。push 到 GitHub 不算交付完成。
- **多 API 并行不阻塞**：N/A（本项目无网络层）

## smoke 测试（嵌入式特殊定义）
1. **Sessions 完整性检查**（`tests/smoke-test.js`）：每 commit 有 pre/post 配对
2. **静态分析**：`-Wall -Wextra -Werror` 必须过
3. **构建 size 检查**：Flash < 200KB, RAM < 40KB（留 8KB 余量）
4. **实机录屏**（用户做）：每个 BACKLOG 完成时录 ≤30s 视频，附到 `docs/qa-reports/YYYY-MM-DD-<slug>.md`

## 不要做的事
- 不要把 demo 视觉炫酷度优先于方法论素材沉淀
- 不要在没有 pre 文件的情况下开始 coding（即使是"快速试试"）
- 不要把 commit 攒大（每 widget / 每模块切开）
- 不要静默忽略 GD32 资料稀缺的痛苦——把痛苦点写进 post 是这个项目最有价值的产出之一
