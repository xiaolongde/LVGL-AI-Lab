---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# RTC 时间源 + 上电保持

## 意图
让"显示时间"不是写死字符串。GD32F303 内置 RTC + BKP 寄存器可以用 LSE/LSI 当时钟源，掉电（如有钮扣电池）后保持时间。本任务不做触摸/按键校时，先让时间能从某个起点真的走起来。

方法论价值：RTC 是 STM32/GD32 上经典坑（LSE 上电时间长、BKP 写保护逻辑、Vbat 行为）。AI 在这块的能力 + 失败模式很值得记录。

## 验收
- ✅ `src/bsp/rtc.c` 提供 `rtc_init() / rtc_get_time(&hh, &mm, &ss) / rtc_set_time(hh, mm, ss)`
- ✅ 启动时从 BKP DR1 读 magic，未初始化则用编译时 default time（如 12:00:00）初始化 RTC + 写 magic
- ✅ 实机：上电后时间从设定起点开始走，秒钟可见跳动；复位后时间继续（不归零）
- ✅ 录屏（拍 30s+ 看分钟变化 + 复位后时间继续）
- ✅ session post：LSE 起振等待 / BKP 写保护流程 / Vbat 假设的明示

## 计划步骤
1. session pre：AI 列 LSE vs LSI 选择 + BKP 用哪个寄存器 + magic 值
2. 写 `bsp/rtc.c` 含 LSE 起振 + RTC 配置
3. 写 BKP 读写 helper（含 PWR 解锁 / BKP 解锁顺序）
4. 写"上电检查 BKP magic 决定是否首次初始化"逻辑
5. main 里加一个简单 print 路径（UART 或屏上 lv_label 显示 RTC 当前 H:M:S）
6. 编 + 烧录 + 实机看时间走
7. 复位再看（验证 BKP 保活）
8. 录屏 + qa-report
9. session post（重点：LSE 起振耗时实测 / BKP 写顺序的坑）
10. commit

## 风险 / Open Questions
- 板上是否有 LSE 32.768kHz 晶振（需用户实物确认）；无则改 LSI（精度差，但对 v0.1 够用）
- 板上是否有 Vbat 钮扣电池座子；无则掉电后 BKP 也丢——本任务的"复位保持"测试退化为"不掉电的复位"
- RTC 时间格式：BCD vs binary（GD32F303 沿袭 STM32F1，是 32 位计数器型 RTC，需软件转换 H:M:S，不像 F4/F7 自带 calendar）—— AI 容易踩这个差异

## 依赖
- [[2026-05-01-led-bringup]]（时钟树配置已就绪）

## 方法论 hook
- Lessons 章节: `lessons/gd32f1-style-rtc-quirks.md`（GD32F303 沿袭 F1 RTC 的特殊坑）
- Capability report 维度: 驱动层（RTC + BKP）
