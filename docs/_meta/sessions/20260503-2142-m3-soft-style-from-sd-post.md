---
type: session-post
task: m3-soft-style-from-sd
pre: 20260503-2142-m3-soft-style-from-sd-pre.md
ended: 2026-05-03T23:37:40+08:00
outcome: success
---

## 实际遇到

3 轮术语对齐才进入正确模式。最终 m3 含 2 部分：

**(a) 软装数据化基础设施（小型，但有用）**：
- theme_style_t struct + key=value parser
- 每 theme 一个 .tstyle 文本文件（terminal/pixel/zen.tstyle）
- 启动 install_style_if_missing 自装到 SD（v0.2 hands-off 模式延续）
- style_load 加载 + parse；SD miss 用嵌入 default
- host (lv_fs_stdio) + MCU (lv_fs_fatfs) 都通

**(b) ZEN 软装迭代实操（多轮）**：
- 第 1 轮：字距 -2 / arc 端点圆角 / 视觉重心微调 → 用户："没差别"
- 第 2 轮（激进版）：半弧 → 全圆 activity ring 包住时间 / indicator 8px 加粗 / fake batt 100→0→100 sweep 让状态色（金/橙/红）切换可见 → 用户："有"
- 第 3 轮：1Hz redraw + cursor 闪烁（terminal）+ ASCII 边框 + halo dot（pixel）+ 字段对齐
- 用户："全力推进，不要老确认" → 进入连续推进模式

## 修复路径

**术语对齐 3 次重读**：
1. 第一次 AI 当代码架构 → 用户给 2 个具体问题（"PC 端开发结果直接显示在 MCU" + "拆硬装/软装"）→ AI 仍读为代码层 declarative engine
2. 用户："这种不能称之为 m3 杠杆，仅仅是通过参数来修改配色。你能认真点么"
3. AI 又读偏（提议 declarative UI engine 升级）→ 用户："你理解错了硬装和软装，硬装是MVP上用到的，给客户展示个大概即可。软装则是每个交互页面的精雕细琢"
4. AI 终于对齐 → 进入实际软装迭代

**软装迭代 host 5s 闭环**：
- 改 theme_zen.c → cmake --build build_host (3-5s) → ./build_host/lvgl-ai-lab-host.exe → 看效果
- 比 MCU 烧板慢 90s 快 ~20×
- 这正是 m1 PC simulator 的用武之地

**v0.3 北极星实证**：同份 src/themes/theme_zen.c 在 host + MCU 烧板都跑同样视觉效果。MCU Flash 255724B (97.55%) 紧但能装，RAM 40528B (82%)。

## 学到的（方法论素材）

**lesson 候选 1：术语跨语境对齐 — 业务术语先 1 句确认 ≫ 自决推 1 小时**

"硬装/软装"在装修行业 = 毛坯框架 vs 精装细节，对应产品节奏 = MVP 粗糙版 vs 逐页精雕。AI 训练数据里更常见的是软件架构语境，所以 default 读成代码组件分离。这是经典 cross-domain ambiguity。**前置 1 句"我理解你说硬装是 X、软装是 Y，对吗？"成本几乎 0，省下错读 1h 的成本**。autonomous mode 不应延伸到这类语义层。

**lesson 候选 2：软装迭代依赖 host 闭环（m1 是 m3 的前置）**

软装精雕的本质是"高频微调"——每次改 1-2 个细节看效果。在 MCU 上 90s/次循环不可行；host 5s/次循环可行。这就是为什么 m1 PC simulator 是 v0.3 关键。**没有 m1，软装迭代没法做**。

**lesson 候选 3：第一轮软装结果"没差别"是预期信号，不是失败**

字距 ±2、ofs ±4、arc 端点圆角这种"design-level 微调"在初始阶段视觉差不显著。用户回"没差别"不是要 AI 放弃方向，而是要**更显著的差异**。第二轮 AI 升级到"全圆 ring + 状态色 sweep"才有"有"的反馈。**软装迭代节奏：先做激进差异试探边界，再回头精调**。

**lesson 候选 4：autonomous mode 的边界 — 业务语义不入场**

用户给"我相信你的选择，先推进出结果"是技术细节授权，不是产品语义授权。技术决策（库选型、错误恢复、cmake config）AI 自决；视觉风格、术语解读、产品节奏 AI 必先确认。这次踩了 1 次坑，应该写入 LTRYSL meta lesson 的边界条款。

## 待 sediment

- 术语 cross-domain ambiguity / autonomous 边界 → 跨项目 meta lesson
