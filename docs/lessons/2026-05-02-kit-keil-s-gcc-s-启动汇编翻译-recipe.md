---
status: backlog
project: LVGL-AI-Lab
type: lesson
updated: 2026-05-02
lesson_id: LXO38I
trigger_when: KIT Keil .s → GCC .S 启动汇编翻译 recipe
keywords: ["startup", "keil", "gcc", "arm", "cmake"]
scope: project
evidence_count: 1
last_triggered: 2026-05-02
revision_history:
  - { date: 2026-05-02, action: created, source: sediment-manual, commit: pending }
card_link: null
---
# LXO38I — KIT Keil .s → GCC .S 启动汇编翻译 recipe
**Rule:** 从 vendor (Keil ARM) startup file 翻 GCC startup 是嵌入式 GCC port 第一坑。模式：vector table 用 .word 数组（KIT .s 用 DCD），Reset_Handler 实现 data copy + bss zero + SystemInit + __libc_init_array + main + 死循环；所有 IRQ 用 .weak + .thumb_set alias 到 Default_Handler。
**Why:** st2 期间没有现成 GCC startup（KIT 只给 ARM/IAR），手翻一次成功。如果不抄完整 IRQ 向量表（GD32F30x HD 共 76 项），少一项硬件 fault。

**How to apply:** 看 vendor 的 ARM/<chip>.s 完整 vector table，翻成 GCC：DCD → .word，AREA RESET DATA → .section .isr_vector，PROC/ENDP → .type/.size，EXPORT [WEAK] → .weak + .thumb_set。Reset_Handler 模板：set sp + memcpy data + memset bss + bl SystemInit + bl __libc_init_array + bl main + 死循环。

**Evidence:**
- st2 GD32F303 startup 翻译一次跑通