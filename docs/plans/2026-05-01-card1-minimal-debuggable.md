---
status: backlog
project: LVGL-AI-Lab
type: plan
updated: 2026-05-01
---

# Card 1: 最小可闭环调试的嵌入式系统

## 意图
方法论的真正实验从"AI 能不能在没人指挥的情况下把工程链路打通"开始。本 card 让 GD32F303RxT6 V5.1 KIT 上电、LED 亮、能 GDB 单步调试、能 printf 收日志——这是后续所有 case 的基座。

工具链已钉死：**CMake + arm-none-eabi-gcc + OpenOCD + GDB + (RTT 或 USART) log**。前置资料已就位（见 `docs/references/`），所以本 card AI 不需要再"找资料"，只要按 references 指引执行。

## 前置已就绪
- ✅ KIT 解压：`D:\projects\GD\GD32F303RxT6 KIT\3. SDK\extracted\` 已解压
- ✅ References 资料库：`docs/references/{INDEX, mcu, board, screens, sdk}.md`
- ✅ 工具链已装：arm-none-eabi-gcc 12.2 / CMake / Ninja / Python 3.11（PATH 里都在）
- ✅ **烧录器：ST-Link**（OpenOCD interface stlink + transport hla_swd + target stm32f1x，flash size override 256K）
- ✅ **Log 通道：USART1（PA9 TX, 115200, 8N1）**——ST-Link 不支持 RTT，走串口 + USB-TTL 工具/putty 抓

## 验收
- ✅ `cmake -B build -G Ninja && cmake --build build` 一次过，产 `build/lvgl-ai-lab.elf` + size 报告（`arm-none-eabi-size`）
- ✅ `tools/flash.sh` 一行命令烧录到 GD32F303 板（OpenOCD 或 J-Link 任一）
- ✅ `arm-none-eabi-gdb build/lvgl-ai-lab.elf` 能 attach、断点、单步、看变量
- ✅ 板载 RGB LED 1Hz 闪（推荐用 R/G/B 轮换证明 PC0/PC1/PC2 都通）
- ✅ 日志通道：RTT (J-Link) 或 USART1 (PA9/PA10) `printf("hello\n")` 主机端能收
- ✅ pre/post 工具就位 (`tools/session.sh new/close/list`)，smoke-test.sh 含 sessions 完整性检查
- ✅ baseline 录屏 + qa-report
- ✅ session 累计 ≥4 对 pre/post

## Sub-task（AI 卡内自决顺序，每个配 1 对 pre/post + ≥1 commit）

1. **pre/post 工具 + smoke-gate sessions 检查**：本任务的元 infra，必须最先（动手做 sub-task 2 前已就位）；dogfood 自指验证
2. **CMake 工程 + GCC toolchain + linker script**：从 `KIT/3. SDK/extracted/.../Examples/RGB LED/` 抄出 `Components/BSP/`、`Core/`、`GD32/` 三个目录到本项目源码区；删 MDK-ARM；写 `cmake/arm-none-eabi.toolchain.cmake` + `linker/gd32f303rc.ld`（256K Flash @ 0x08000000, 48K SRAM @ 0x20000000）+ 顶层 `CMakeLists.txt`；GD32 启动汇编从 `Library/GD32F30x_AddOn_V2.1.0/` 找 GCC 版的 `.s` 文件（如无则手动改 Keil 的 .s）
3. **`cmake --build` 干净通过**：无 warning、有 size 报告
4. **OpenOCD 配置 + flash 脚本**：`tools/openocd-gd32f303.cfg`（GD32F303 借 stm32f1x target，但 flash size 256K 要 override；也可试 pyOCD）；`tools/flash.sh` 一行命令封装
5. **GDB attach + 断点验证**：`tools/gdb.sh` 启动 GDB + 加载 elf + connect to OpenOCD；在 main 设断点，单步几行
6. **RGB LED 闪**：用 references/board.md 的引脚（PC0/PC1/PC2）写 GPIO init + SysTick delay；R→G→B 轮换 0.3s 各
7. **日志通道（USART1）**：抄 `Examples/USART/Components/BSP/usart.c` init（PA9=TX, PA10=RX, 115200 8N1）+ `printf` 重定向到 USART_DR（GCC 走 `_write` syscall，重定义 `_write` 调 USART 发送）；主机端 USB-TTL 模块（用户需有，常见 CP2102/CH340）连 PA9/PA10 + GND，putty 或 minicom 抓
8. **录 baseline 视频 + qa-report**

## 风险 / Open Questions
- GD32 启动汇编 GCC 版可能需要从 Keil .s 改写（Keil 用 `THUMB`、`PRESERVE8`、`AREA RESET`，GCC 用 `.section .isr_vector` 等）—— 经典坑，方法论 datapoint
- OpenOCD 上游 stm32f1x.cfg 默认 flash 128K——**必须 override 到 256K**（`set FLASH_SIZE 0x40000`），否则烧到 128K 后失败
- 链接脚本写错最常见：忘记 `_estack`、`_etext`、`_sidata`、`_sdata`、`_edata`、`_sbss`、`_ebss` 这些符号，启动汇编里找不到会硬件 fault
- USART1 (PA9 TX) 出来是 TTL 电平 3.3V，**不能直接接电脑 RS232**，需要 USB-TTL 适配器（CP2102/CH340/FT232 任一）

## 依赖
无（v0.1 起点）

## 方法论 hook
- Lessons 候选: `lessons/empty-to-debuggable-binary.md`、`lessons/cmake-from-keil-port.md`、`lessons/zero-friction-session-discipline.md`、`lessons/embedded-smoke-with-video.md`
- Capability report 维度: bring-up + 工程 infra（CMake / OpenOCD / GDB / Keil→GCC 移植）
