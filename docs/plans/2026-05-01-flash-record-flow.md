---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# 烧录 + 录屏 + qa-report 流程模板

## 意图
项目 CLAUDE.md 把"实机录屏"定为 smoke 必做项（替代 Web 项目的"重启 server"语义）。需要一次性把烧录链路 + 录屏归档流程定下，后续每个 BACKLOG 完成时直接套。先选烧录工具（J-Link / DAP-Link / WCH-LinkE），打通烧录命令；再固化"录视频 → 命名规则 → 链接到 qa-report"链路。

## 验收
- ✅ 选定烧录器型号 + 烧录命令固化到 `tools/flash.sh`（输入 .elf/.bin 一行命令烧）
- ✅ 录屏规范：≤30s, 720p, 命名 `qa-reports/YYYY-MM-DD-<slug>.mp4`（视频 + 同名 .md report）
- ✅ qa-report 模板：测试用例 + 实机现象 + 视频 link + pass/fail
- ✅ 用本任务自己 dogfood：录一段"项目刚 bootstrap、空白屏" 的视频作为 baseline
- ✅ 项目 README 加"自测流程"段

## 计划步骤
1. 调研：手上 GD32 板自带哪种烧录器（DAP-Link/CMSIS-DAP/J-Link OB？）
2. 选工具链命令（OpenOCD + 配置文件 / SEGGER J-Link / pyOCD）
3. 写 `tools/flash.sh` 一行命令封装
4. 写 `qa-reports/_template.md` 模板
5. dogfood：录 baseline 视频 + 写 baseline qa-report
6. README 加段
7. commit

## 风险 / Open Questions
- GD32F303 在 OpenOCD 上游 cfg 的兼容性（可能需要"--target stm32f1x"借用，因为 GD32F30x 沿袭 STM32F103 寄存器）
- 录屏用什么工具：手机直拍 vs OBS 抓屏 vs USB 视频采集卡。v0.1 先用手机直拍，简单可靠
- 视频文件大小是否 commit 到 repo（建议：v0.1 先 commit，超过阈值后转 LFS 或外链）

## 依赖
- [[2026-05-01-pre-post-automation]] (sessions 工具先就位)

## 方法论 hook
- Lessons 章节: `lessons/embedded-smoke-with-video.md`（嵌入式项目的"重启 server"等价语义）
- Capability report 维度: 工程 infra
