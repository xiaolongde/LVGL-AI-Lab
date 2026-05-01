---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# Card 2: LVGL 支持 + 表盘 demo

## 意图
Card 1 把工程 + 调试链路立起来；本 card 让屏亮起来 + LVGL 跑起来 + 屏上显示真实走时的表盘。这是 v0.1 的最终 demo——拍出来一眼能识别为"手表"。

## 前置
- ✅ Card 1 完成（CMake/OpenOCD/GDB/LED/log 都通）
- ✅ `docs/references/screens.md` 已对屏选型 + ST7789V3 关键参数齐备
- ⚠ 屏选型：默认走 **1.14" ST7789V3 (240x135)**；实物若是 0.96" ST7735S 退路也可（KIT 自带两屏 example）

## 验收
- ✅ 屏 bring-up：上电后整屏循环纯红/纯绿/纯蓝填充（验证 SPI + DC/RST/CS 接线 + init sequence 全通）
- ✅ LVGL v9 移植成功：`lv_label("LVGL ok")` 居中显示，partial buffer 6KB，Flash < 200KB / RAM < 40KB
- ✅ RTC + LSE 时间走起来：`rtc_get_time(&h, &m, &s)` 返回单调递增；NRST 复位后时间继续（前提是有 RTC 后备电池或不掉电）
- ✅ Time widget：lv_label + lv_timer 每秒刷新，显示 "HH:MM:SS"，秒钟跳动平滑无 tearing
- ✅ 表盘 layout：屏中央大字号时间 + 上方 date label（硬编码 `2026-05-01`）+ 下方 battery bar（假数据 75%）
- ✅ 实机录 1 分钟视频（看 `:59 → :00` 进位 + 整体不抖）+ qa-report
- ✅ session 累计 ≥6 对 pre/post

## Sub-task（AI 卡内自决顺序）

1. **屏 bring-up**：从 `Examples/LCD 1.14inch/Components/Devices/TFTLCD/tftlcd.c` 抄 init sequence（**post 必须明示**：抄了哪些行、改了哪些、为什么——这是 Playbook "AI 在有现成代码时的取向"素材）；用 references/board.md 的引脚（PB3/PB5 SPI0 remap + PB4 DC + PB6 RST + PA15 CS + PB7 PWR）
2. **屏驱动抽象 + host mock 单测**：`screen.h` 暴露 `init / flush(x1,y1,x2,y2,buf) / fill(...)`；写 host mock 用 `#ifdef HOST_TEST` 拦截 SPI 调用，跑边界 case 单测
3. **LVGL submodule + lv_conf.h 裁剪 + partial buffer 验证**：`external/lvgl/` (v9 stable)；裁掉所有不用的 widget；partial buffer 6KB；接 `lv_port_disp.c` flush_cb；SysTick 1ms 给 `lv_tick_inc(1)`；先跑 hello world 验证整链路
4. **RTC + LSE + BKP 上电保持**：抄 `Examples/RTC/` 的 init；写 `rtc_get/set_time` API；BKP DR1 写 magic 检测首次启动，未初始化用编译时 default
5. **Time widget**：lv_label + lv_timer 每秒调 rtc_get_time → label_set_text；选字体（推荐 montserrat 24 或 28）
6. **表盘 layout**：lv_obj 容器 + flex/grid 排版；time 大字、date 中字、battery bar 占下方；硬编码 date 文本；电量假数据
7. **录视频 + qa-report**：1 分钟连续运行视频 + 静态截图（手机翻拍即可）+ "看一眼像手表吗"自评

## 风险 / Open Questions
- **partial buffer 6KB 够不够**：240×135 屏一行是 480B，6KB ≈ 12 行 buffer——足够 LVGL 工作但每屏需多次 flush；若 tearing 明显可加到 10-15KB（48KB 紧但够）
- **字体大小 vs Flash 占用**：内嵌 montserrat 28 含全部 ASCII 约 +20-30KB Flash，可只内嵌 0-9 + ":" 共 11 字符省一半
- **GD32 RTC 沿袭 F1 风格**（32 位计数器，需软件转 H:M:S，不像 F4 自带 calendar）—— 经典坑，方法论 datapoint
- **LVGL v9 vs v8**：v9 是最新但部分 example 没更新；如果 AI 决定回 v8 必须在 post 写明理由
- 屏 PWR 是 PMOS 控的（PB7=低有效），AI 容易写反

## 依赖
- [[2026-05-01-card1-minimal-debuggable]]

## 方法论 hook
- Lessons 候选: `lessons/lvgl-on-tight-sram.md`、`lessons/spi-screen-init-from-vendor-example.md`（重点：抄 vs 改写）、`lessons/gd32f1-style-rtc-quirks.md`、`lessons/lvgl-watch-face-layout.md`
- Capability report 维度: 驱动层（SPI + 屏 IC）+ LVGL 应用层 + 资源工程
