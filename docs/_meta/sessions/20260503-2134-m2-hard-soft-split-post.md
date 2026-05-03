---
type: session-post
task: m2-hard-soft-split
pre: 20260503-2134-m2-hard-soft-split-pre.md
ended: 2026-05-03T23:37:39+08:00
outcome: partial
---

## 实际遇到

代码层重构顺利：抽 desktop_logic.{c,h} + 3 个 theme_*.c 文件，main.c 收成 30 行。一次重构成功，host build 通过，3 主题轮播无回归。

但 **outcome 是 partial 因为术语理解错**：
- AI 把"硬装/软装"读为代码架构（控制层 vs 渲染层）
- 用户实际意图是产品节奏（MVP 粗糙版 vs 逐页精雕）
- AI 在 m3 又错读为"declarative UI engine"，再次被纠正
- 第三次才对齐：软装 = 进入精雕迭代周期，不是代码重构

## 修复路径

代码重构本身保留 — 解耦控制层 / 渲染层 / boot wire 仍是好工程。但 m2 不是用户原意的"硬装"。真·硬装 = sk 已交付的"3 主题大概能跑"MVP；真·软装 = m3 启动后的精雕迭代。

## 学到的（方法论素材）

**lesson 候选 1：术语跨语境匹配前必须确认歧义**

"硬装/软装"在用户语境是装修行业产品节奏术语（毛坯/精装），AI 读为代码组件分离。同字面有 ≥2 个解读路径时，**直接做之前应该 1 句确认 + 给 2 个候选解读让用户拍板**。这次的 cost：m2 + 错版 m3 + 重做 m3，浪费约 1h。

**lesson 候选 2：autonomous mode 在术语歧义点不应 default-yes**

"我相信你的选择"≠"任何含糊处都自决"。术语解读、风格选择、产品方向是用户专属判断。autonomous mode 的 default-yes 范围应限于：技术实现细节、库选型、错误恢复路径。**业务/产品语义层永远先确认**。

**lesson 候选 3：重构本身不浪费**

虽然 m2 的代码层分离不是用户的"硬装/软装"，但 desktop_logic / theme_*.c / main.c boot wire 的解耦在 m3 软装迭代时**真的提供了便利**：改 theme_zen.c 不会动到 control loop，改一个 theme 不影响其他。**误打误撞做对了基础设施**，下一次软装迭代周期受益。
