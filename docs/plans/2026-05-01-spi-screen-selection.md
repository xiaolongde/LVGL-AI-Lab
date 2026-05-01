---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# SPI bring-up + 屏型号确定 + 出第一像素

## 意图
LED 闪通后，下一步是把屏点亮到能填一个色块——这是"硬件能控"的真正起点。本任务包含两件事：(a) 选具体屏（IC + 分辨率 + 接线）；(b) SPI bring-up + 屏 init sequence + 单色块填充。

方法论价值：AI 在"读 SPI 屏 datasheet → 提取 init sequence → 写 SPI 驱动" 这条链上的能力是 capability report 的关键章节。屏 init 序列容易在"魔法常量"上栽——AI 是直接抄网络例子还是从 datasheet 推导，是值得记录的取向。

## 验收
- ✅ 屏 IC 型号确定（候选 ILI9341 / ST7789 / GC9A01 等），原理图/接线图入 `docs/specs/screen-wiring.md`
- ✅ `src/bsp/spi.c` SPI 主机驱动（不带 DMA，先轮询 + 同步），能稳定收发
- ✅ `src/drivers/<screen-ic>/init.c` 屏 init sequence（必须**注释每条魔法常量从 datasheet 哪页来**）
- ✅ 实机：上电后整屏纯红 → 1s 后纯蓝 → 1s 后纯绿（颜色块循环）
- ✅ 录屏入 qa-report
- ✅ session post 记录：AI 在 init sequence 是抄例子还是查 datasheet（重要！这是 capability 维度）

## 计划步骤
1. session pre：AI 列候选屏 IC + 选择理由
2. 用户确认实物屏 IC（看屏背面丝印 / 卖家页 / 自己量管脚）
3. AI 查 datasheet 提取 init sequence + 写注释
4. 实现 `bsp/spi.c`（GD32 SPI0 主模式 + 8bit + Mode 0/3 看屏要求）
5. 实现 `drivers/<ic>/init.c` 包含 reset/cs/dc 引脚控制
6. 写颜色块填充测试程序
7. 烧录 + 实机看颜色循环
8. 录屏 + qa-report
9. session post（重点：datasheet 阅读路径 + 卡过的常量含义）
10. commit

## 风险 / Open Questions
- DMA 这一步先不做（48KB SRAM 紧 + 复杂度高），轮询能跑起来即可；DMA 移到 v0.2
- 屏 reset 时序（datasheet 通常要 ≥120ms），AI 容易省略 → 看 post 是否记录
- 颜色端序（RGB565 / BGR565）：很多廉价屏跟示例代码颜色错位，是经典坑

## 依赖
- [[2026-05-01-led-bringup]]（GPIO + 时钟链路已验证）

## 方法论 hook
- Lessons 章节: `lessons/spi-screen-init-from-datasheet.md`（AI 是抄例子还是读 datasheet 的实证）
- Capability report 维度: 驱动层（SPI master + 屏 IC init）
