---
type: session-pre
task: sf-card2-retro
backlog_item: card2-lvgl-watch-face
started: 2026-05-02T13:58:00+08:00
---

## 任务理解

v0.1 收口任务：从 8 对 sub-task pre/post（st1-st4 + sa-sd）抽 lessons + 写 capability-report v0.1。**这是 design doc 里 v0.1 success criteria 的最后一项**（"Capability Report v0.1: 覆盖 ≥3 层"）。

按 LTIRBE 多论据原则，每条 lesson 必须有 ≥2 个 sub-task evidence；单 sub-task 看到的 quirk 暂不写成 lesson（写在 sessions post 里）。

## 计划

1. **核心 lessons**（项目特定，写到 `docs/lessons/`）：
   - `lvgl-v9-on-tight-flash-min-config.md` — LVGL v9 在 256K Flash 上最小可用配置（evidence: sb size 调试 + sd 字体溢出）
   - `lvgl-v9-rgb565-byte-order.md` — LE/BE 字节序坑 + lv_draw_sw_rgb565_swap 修法（evidence: sb 字节序 + sd 颜色 token）
   - `lvgl-v9-transform-partial-broken.md` — transform + partial mode 4 次实证不兼容（evidence: sd 4 次）
   - `embedded-3step-hardware-diagnosis.md` — LED→LCD_Fill→真数据三步法（evidence: sb 银幕诊断 + sd 屏验证）
   - `keil-to-gcc-startup-translation.md` — KIT .s → GCC .S 翻译 recipe（evidence: st2 移植）
   - `gd32-via-stm32f1x-pyocd.md` — stm32f103rc target 驱动 GD32F303（evidence: st3 实测 + sa 验证）
   - `lvgl-conf-dependency-chains.md` — lv_conf 配置项依赖（evidence: sd PERF→SYSMON / spinner→SW_COMPLEX 都触发 ）
   - `local-vs-global-config-discipline.md` — 改 widget/conf 时复审 5 项核检（evidence: sb 5 次 debug 失败的元 pattern）

2. **lessons/INDEX.md**：列所有 lesson 文件 + 关联 sessions

3. **capability-report v0.1.md**（4 层 + 总评）：
   - bring-up 层（CMake / linker / 启动汇编 / 烧录链）：完成度 + 失败模式 + 时间
   - 屏驱动层（SPI / ST7789 / Address_Set）：抄 vs 改写比例 / 字节序坑率
   - LVGL 应用层（widget / theme / size 调试 / transform）：v9.5 兼容性观察
   - 调试方法论（GDB / 硬件信号 / pua-debugging）：诊断回合数据
   总评：AI 在哪几层能独立完成 + 介入点统计

4. **qa-report v0.1.md**：v0.1 主要 demo（GT3 watchface）的视觉/功能 sign-off 记录

5. **README.md** 更新：v0.1 done 状态 + lessons/capability-report 链接

6. **state.md** 推到 idle - card2 done

7. **commit + push**

## 卡点预判

1. **lesson 写多详细的取舍**：完整描述每个 lesson 会几小时；浓缩到关键 rule + why + how 30 分钟。LTIRB3 daily MVP 原则下选浓缩
2. **capability-report 主观打分客观化**：完成度百分比怎么定？用"AI 介入次数 + 用户介入次数 + 净 retry 次数"做基线
3. **v0.1 仍未 hands-off**：这是事实，capability-report 必须诚实记录"sd 期间用户介入 5 次"，不美化

## 验收

- [ ] `docs/lessons/` 至少 ≥3 lesson 文件（多论据）
- [ ] `docs/lessons/INDEX.md` 索引齐
- [ ] `docs/capability-report/v0.1.md` 含 ≥3 层分析 + 介入点统计
- [ ] `docs/qa-reports/2026-05-02-card2-watchface.md` GT3 视觉 sign-off
- [ ] README v0.1 done + 链接更新
- [ ] state.md 推到 card2 done
- [ ] sf post 含 ≥1 lesson 候选
- [ ] commit + push GitHub
