---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# 板载 LED 点亮（hello-world / 工具链验证）

## 意图
最小可验证起点：板载 LED 闪烁。这一步验证整个工具链（GCC ARM + 链接脚本 + 烧录 + 启动文件 + 时钟初始化）能否端到端跑通——LED 闪不起来意味着后面屏驱动等更难任务无从谈起。

方法论价值：AI 在"完全空白工程 → 第一个能跑的二进制"这一段的能力是个关键 datapoint——很多 LLM 在这段会卡在工具链/Makefile/链接脚本细节。

## 验收
- ✅ `src/main.c` + 启动文件 + 链接脚本 + Makefile（或 CMake）齐
- ✅ `make` 能在干净环境产出 `out/lvgl-ai-lab.elf`，size < 8KB
- ✅ 烧录后板载 LED 以 1Hz 闪烁
- ✅ 录屏（5s LED 闪）入 qa-report
- ✅ session post 写出"AI 在选 firmware lib (GD32 official vs libopencm3 vs CMSIS-only) 的决策路径"

## 计划步骤
1. session pre：AI 先写"我打算用什么 firmware lib + Makefile vs CMake + 时钟配置策略"
2. AI 决定 firmware 库（候选：GD32F30x_Firmware_Library / libopencm3 / 裸 CMSIS）
3. 写 main.c（GPIO init + 简单 delay 闪 LED）
4. 写启动文件 + 链接脚本（GD32F303RC: 256KB Flash, 48KB SRAM）
5. 写 Makefile，产出 .elf + .bin + .map
6. `make` 能编（无 warning）
7. 烧录到板，LED 闪
8. 录视频 + qa-report
9. session post：实际遇到 vs 预判 vs 学到的（重点：库选型理由 / Makefile 哪些坑 / 时钟配置卡点）
10. commit (1 commit, msg 含 `session: led-bringup`)

## 风险 / Open Questions
- GD32F303RC 启动文件需要 GD 官方版（与 ST 不完全等价，GD 主频/时钟树有差异）
- AI 是否会陷在"找不到合适 GD32 example 工程"——这本身就是方法论数据点（资料稀缺平台）
- LED 引脚位置：用户确认（板上原理图 / 印丝 / 现成 example）

## 依赖
- [[2026-05-01-pre-post-automation]]
- [[2026-05-01-flash-record-flow]]

## 方法论 hook
- Lessons 章节: `lessons/empty-to-first-binary.md`
- Capability report 维度: bring-up 层 / 工具链 / 时钟初始化
