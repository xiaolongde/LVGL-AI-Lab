---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# LVGL 移植 + partial buffer 在 48KB SRAM 上验证

## 意图
把 LVGL submodule 拉进来，配 `lv_conf.h`，接 screen flush_cb + tick，跑官方 demo 例子（如 lv_demo_widgets 中的最小化版）。**重点：partial buffer 5-10KB 是否真能在 48KB SRAM 跑稳**。如果 SRAM 紧到跑不了 demo，需要立即重新评估资源（设计 doc P4 前提是否成立的实证）。

方法论价值：这是 design doc P4 前提的 sink-or-swim 验证点。即便 P4 失败，也是 capability report 里宝贵的实证数据。

## 验收
- ✅ LVGL v9（最新稳定）作为 git submodule 加入 `external/lvgl/`
- ✅ `src/lvgl_port/lv_conf.h` 配置：色深 RGB565 / partial buffer 5-10KB / 双 buffer 关掉省 RAM / 关掉所有不用的 widget（lv_chart 等）
- ✅ `src/lvgl_port/lv_port_disp.c` 接 screen_flush_cb
- ✅ `src/lvgl_port/lv_port_tick.c` 用 SysTick 提供 lv_tick_inc()
- ✅ 实机：跑一个最小化的 LVGL hello world（屏中央显示 "LVGL ok"），帧率稳定（不需要快，不卡死即可）
- ✅ 构建 size 报告：Flash 占用 < 200KB / RAM 占用 < 40KB（含 LVGL 全部静态 + 5-10KB partial buffer）
- ✅ session post 重点：lv_conf.h 关 widget 的过程 / SRAM 卡点 / 帧率瓶颈在哪

## 计划步骤
1. session pre：AI 计划 partial buffer 大小 + lv_conf 关哪些 widget + tick 用 SysTick 还是 TIM
2. 拉 LVGL submodule
3. 复制 LVGL 提供的 `lv_conf_template.h` 到项目，逐项裁剪
4. 实现 lv_port_disp.c（buf 分配 + flush_cb 注册 + flush 完调 lv_disp_flush_ready）
5. 实现 lv_port_tick.c（SysTick handler 调 lv_tick_inc(1)）
6. 写 `app/main.c` LVGL hello world：lv_label "LVGL ok" 居中
7. 编（看 size 报告，必要时回头调 lv_conf 减小）
8. 烧录看屏
9. 录屏（30s 看是否卡死/重启）
10. session post + size 报告附 qa-report
11. commit

## 风险 / Open Questions
- **P4 实证关键节点**：如果 partial buffer 5KB 不够 LVGL 工作（lv_obj 渲染中间需要更多），可能要扩到 10KB 甚至 15KB；如果 15KB 仍不够，需要重新评估硬件
- LVGL v9 vs v8：v9 是最新但部分老 example 没更新；若 AI 决定回 v8，要在 post 记录理由
- SysTick 优先级 vs SPI 中断（如果用）的优先级排布

## 依赖
- [[2026-05-01-screen-driver.md]]

## 方法论 hook
- Lessons 章节: `lessons/lvgl-on-tight-sram.md`（48KB SRAM 上跑 LVGL 的实战配置）
- Capability report 维度: LVGL 应用层 + 资源工程
