---
status: backlog
project: LVGL-AI-Lab
type: lesson
updated: 2026-05-02
lesson_id: LXO38N
trigger_when: pyOCD 用 stm32f103rc target 驱动 GD32F303
keywords: ["pyOCD", "GD32", "stm32", "SWD", "flash"]
scope: project
evidence_count: 1
last_triggered: 2026-05-02
revision_history:
  - { date: 2026-05-02, action: created, source: sediment-manual, commit: pending }
card_link: null
---
# LXO38N — pyOCD 用 stm32f103rc target 驱动 GD32F303
**Rule:** GD32F303 在 SWD/CoreSight/Cortex-M4 寄存器层与 STM32F103 100% 兼容 (GD 沿袭 F1 系列设计)。pyOCD `-t stm32f103rc` builtin target 直接驱动 GD32F303RC 烧录 + GDB attach + flash erase 全功能 OK。
**Why:** st3 期间 OpenOCD 在 Windows 装麻烦 (choco/xpack/手装)，pyOCD pip install --user 免 admin 5 分钟搞定。stm32f103rc target 256K Flash + 48K SRAM 与 GD32F303RC 一致，无需 override。

**How to apply:** 国产 ARM Cortex-M (GD32 / AT32 等) 起 SWD 调试链时优先 try ST 对应型号 target。GD32F30x → stm32f1x.x；GD32F4 / GD32H7 → stm32f4 / stm32h7 系列。flash 容量大小如果不一致需 override (如 GD32F303RC 与 stm32f103rc 都是 256K, 直接可用)。

**Evidence:**
- st3 实测烧录 + GDB attach + load 全通