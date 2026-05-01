---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# 第一次周 Retro：从 sessions 抽 lessons + capability report v0.1

## 意图
v0.1 走完前 9 个 BACKLOG 后，必须做一次完整的 retro，把 `_meta/sessions/` 里累积的 pre/post 自评素材**真的**抽到 `lessons/` 章节 + `capability-report/` 章节里。这是 Approach B 的关键产出动作——**没这一步，方法论就只是 raw 自评散件，不算完成 v0.1**。

后续每个 BACKLOG 完成（含 v0.2+ 的 case 扩展）可复用本 retro 的脚本/模板。

## 验收
- ✅ `tools/retro.sh week` 列出本周 sessions/ 文件 + 关联 commits
- ✅ `lessons/INDEX.md` 至少 3 条 lessons（每条配 frontmatter + 一段提炼 + 引用至少 2 个 session post）
- ✅ `capability-report/v0.1.md` 至少覆盖 3 层（bring-up / 屏驱动 / LVGL 应用），每层有：
  - AI 独立完成度（百分比 + 主观打分）
  - 1-2 个失败 mini-case（含 link 到具体 session post）
  - 1 条 "下一阶段需 AI 改善什么"
- ✅ `_meta/sessions/` 累计 pre/post 对 ≥20（设计 doc success criteria）
- ✅ 跨项目 lessons 候选清单：哪些 lessons 通用度高、应该 promote 到 `Atlas/Cards/`
- ✅ retro 本身也写一对 pre/post

## 计划步骤
1. session pre：列本周完成的 BACKLOG / 计划 retro 范围 / 想抽的 lessons 候选
2. 写/优化 `tools/retro.sh week`（grep sessions/ 按周分组 + 提取 outcome 字段）
3. 跑 `tools/retro.sh week`，产生原始材料清单
4. 用户参与：盘点 candidate lessons（用户判断哪些是真 lesson 哪些是噪音）
5. 写 lessons 文件（每条 ≤ 100 行，含具体 example session 链接）
6. 写 lessons/INDEX.md
7. 写 capability-report/v0.1.md（按层组织）
8. 标记跨项目 lessons → 列到 retro post 里供 promote 决策
9. CHANGELOG 加 "v0.1 release notes"（也是手表 demo + 方法论 v0.1 的发布点）
10. session post（重点：retro 流程本身的痛点/可改进点）
11. commit

## 风险 / Open Questions
- 用户参与时长：抽 lessons 是用户最重的工作，可能要 2-4h；预算内能否容纳
- "至少 20 对 pre/post" 是数量目标，但**单 case 跑下来可能数量不够**——若不到 20，retro 时改成"按现有素材抽尽"，记录 dataset 不足并 calibrate v0.2 期望
- 跨项目 promote 到 Atlas/Cards 的标准：lesson 必须能脱离 LVGL/嵌入式上下文还有意义。容易过度 promote。
- v0.1 的 capability report 一定单薄（单 case），坦诚标注"v0.1 维度浅，需 v0.2 多 case 拓宽"

## 依赖
- [[2026-05-01-watch-face-layout]]（需要 v0.1 demo 跑通才能 retro）
- 实际上依赖前 9 项全部完成

## 方法论 hook
- 这个任务**就是** Lessons 章节: `lessons/weekly-retro-flow-for-ai-projects.md` + `capability-report/v0.1.md` 的产出
- 元 retro：retro post 自身就是 retro 工具的 lesson
