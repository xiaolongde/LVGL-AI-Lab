---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# pre/post 自评自动化模板

## 意图
项目 CLAUDE.md 把 pre/post 自评定为硬约束（无 pre 不让动手 / 无 post 不让 merge）。如果创建 pre/post 文件需要每次手敲 frontmatter + boilerplate，纪律会很快崩。需要一个零摩擦小工具：输入 task slug，自动建 pre 文件骨架；任务完成时一键建 post + link 回 pre。

这是方法论 infra，所有后续任务都依赖它。

## 验收
- ✅ `tools/session.sh new <slug>` 在 `docs/_meta/sessions/` 创建 `YYYY-MM-DD-HHMM-<slug>-pre.md`，frontmatter + 三段模板齐
- ✅ `tools/session.sh close <slug>` 找到对应 pre，建 post，frontmatter 自动 link 回 pre + 填 ended/outcome
- ✅ `tools/session.sh list` 列出所有未 close 的 pre（提醒未沉淀）
- ✅ `tests/smoke-test.sh` 含 sessions 完整性检查：每个非 merge commit 必须能映射到一对 pre/post（commit msg 含 `session: <slug>`），缺则 exit 1
- ✅ README 加一段"如何用 session 工具"

## 计划步骤
1. 起 task pre 文件（手动一次，作为零起点案例）
2. 实现 `tools/session.sh`：bash 脚本即可，跨平台用 git-bash
3. 实现 smoke-test.sh sessions 检查段（grep commit msg + ls _meta/sessions/）
4. 自己 dogfood：用 session.sh 完成本 task 的 pre/post（自指验证）
5. 写 README 用法
6. commit (msg 含 `session: pre-post-automation`)

## 风险 / Open Questions
- Windows + git-bash 兼容（不能假设 GNU date / readlink 行为）
- AI 自己执行 session.sh 的成本：是否需要在项目 CLAUDE.md 里加"动手前先 `tools/session.sh new <slug>`"硬规则
- post 的 outcome 字段（success/partial/failed）由 AI 自评是否客观——v0.1 先信任，retro 时校准

## 依赖
无（这是项目最早任务）

## 方法论 hook
- Lessons 章节: `lessons/zero-friction-session-discipline.md`
- Capability report 维度: 工程 infra（不属于 bring-up/驱动/LVGL 任一层）
