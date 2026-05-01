---
type: session-pre
task: st2-cmake-bringup
backlog_item: card1-minimal-debuggable
started: 2026-05-02T01:13:11+08:00
---

## 任务理解

从 KIT/Examples/RGB LED 抄出可复用的 GD32 firmware lib + BSP + 应用代码到本项目，写 CMake 工程，让 `cmake --build build` 干净通过产 `lvgl-ai-lab.elf`。本任务**不上板**——纯软件层验证工具链链路完整。LED 真闪在 st6 实机阶段。

## 计划

### 目录布局
```
LVGL-AI-Lab/
├── src/                    ← 应用代码
│   ├── main.c              ← 从 KIT/Examples/RGB LED/Core/Src/main.c
│   ├── gd32f30x_it.c       ← 中断 stub
│   ├── gd32f30x_libopt.h   ← lib 配置
│   ├── main.h              ← 项目级宏
│   └── bsp/
│       ├── gpio.{c,h}      ← LED + KEY 引脚
│       ├── systick.{c,h}   ← SysTick delay
│       └── usart.{c,h}     ← USART1 init（PA9/PA10, 115200）—— st7 用，但工程要先 link 通
├── vendor/gd32f30x/        ← vendored 标准外设库
│   ├── CMSIS/
│   │   ├── core_cm4*.h
│   │   └── GD/GD32F30x/
│   │       ├── Include/{gd32f30x.h, system_gd32f30x.h}
│   │       └── Source/system_gd32f30x.c
│   └── GD32F30x_standard_peripheral/
│       ├── Include/gd32f30x_*.h
│       └── Source/gd32f30x_*.c
├── linker/gd32f303rc.ld    ← 256K Flash @ 0x08000000, 48K SRAM @ 0x20000000
├── startup/startup_gd32f303rc.S  ← 我手写的 GCC 版（KIT 只给 Keil ARM 和 IAR）
├── cmake/arm-none-eabi.toolchain.cmake
└── CMakeLists.txt
```

### 步骤
1. 用 `cp -r` 从 KIT 复制 vendor 文件到 vendor/gd32f30x/（保留路径子结构以便 grep KIT 时对照）
2. 复制 BSP 文件（gpio/systick/usart）到 src/bsp/
3. 复制 Core/Inc/{main.h, gd32f30x_libopt.h, gd32f30x_it.h} 到 src/（重要：保留 libopt 配置）
4. 复制 Core/Src/{main.c, gd32f30x_it.c} 到 src/
5. **写 startup_gd32f303rc.S**（GCC 语法）：参考 KIT/.../ARM/startup_gd32f30x_hd.s 的向量表（已读，60 条 IRQ + 16 ARM exception），翻译指令。引用 `_estack`、`SystemInit`、`main`，bss/data 段初始化代码。
6. **写 gd32f303rc.ld**（GCC 链接脚本）：MEMORY 段定义 FLASH (256K @ 0x08000000) + RAM (48K @ 0x20000000)，SECTIONS 段定义 .isr_vector / .text / .rodata / .data / .bss / ._user_heap_stack / 提供符号 `_sidata _sdata _edata _sbss _ebss _estack _end`
7. **写 toolchain.cmake**：CMAKE_SYSTEM_NAME=Generic、TARGET=arm-none-eabi、CFLAGS 含 `-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections`、LDFLAGS 加 `-Wl,--gc-sections -T<linker.ld>`
8. **写 CMakeLists.txt**：project + add_executable（含 src/main.c + 所有 src/bsp/*.c + startup.S）+ add_library(gd32f30x STATIC ...)（含 vendor/ 的 .c）+ target_include_directories（vendor/CMSIS、vendor/CMSIS/GD/GD32F30x/Include、vendor/GD32F30x_standard_peripheral/Include、src、src/bsp）+ target_link_libraries
9. `cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.toolchain.cmake` + `cmake --build build`
10. 验证 `build/lvgl-ai-lab.elf` 产出 + `arm-none-eabi-size` 报告
11. session post + 更新 smoke-test.js 阶段 2 的 build size 检查从 stub 转为真检查
12. CHANGELOG + commit (1 commit, msg 含 `session: st2-cmake-bringup`)

## 卡点预判

1. **GCC 启动汇编**：Keil 用 `AREA RESET, DATA` 和 `EXPORT __Vectors`，GCC 要 `.section .isr_vector` 和 `.global g_pfnVectors`。指令翻译 1:1 不难，但**易漏 weak alias 声明**（`.weak NMI_Handler` + `.thumb_set NMI_Handler, Default_Handler`）。会写 8-10 个 ARM exception + 60 个 IRQ handler，全部 weak alias 到 Default_Handler。
2. **链接脚本符号**：`_estack`、`_sidata`、`_sdata`、`_edata`、`_sbss`、`_ebss`、`_end` 必须齐，启动汇编里用到。少一个就 fault。Heap 也要给（newlib 用 `_sbrk` 需要 `_end` 符号）。
3. **system_gd32f30x.c 初始化**：里面有 `SystemInit()` 配 PLL（HSE 8MHz → 120MHz），需要 `HXTAL_VALUE` 宏正确（默认 25MHz，但本板 8MHz）—— 先用 lib 默认看能不能编过，运行时频率不对的事 st3/st6 再说。
4. **lib 编译错误**：standard_peripheral 库 enet/usbd 部分依赖 PHY/USB，可能编译失败——先把这俩目录 exclude 掉（不放进 vendor/）。
5. **CMake `try_compile` for ARM**：ARM 交叉编译 cmake 默认会跑 try_compile（链接 main 函数），无 startup 时会失败。需要 `set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)` 在 toolchain.cmake 里 bypass。
6. **smoke-test.js 阶段 2 的 size 检查激活**：等 build 出 elf 后，把 stub 替换为真 `arm-none-eabi-size` 解析，阈值 Flash<200KB / RAM<40KB（含 stack/heap/bss/data）—— 不在本任务必须，可推 st3。

## 验收

- [ ] `cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.toolchain.cmake` 干净通过（无 fatal error）
- [ ] `cmake --build build` 干净通过（允许 -Wall 不允许 -Werror 一开始；先看 warning 数量）
- [ ] 产物：`build/lvgl-ai-lab.elf` 存在，size < 50KB Flash（RGB LED 仅 + 库 metadata）
- [ ] `arm-none-eabi-size build/lvgl-ai-lab.elf` 输出 reasonable（text + data + bss）
- [ ] smoke-test.js 仍 PASS（阶段 1 sessions OK；阶段 2 stub 现在可激活也可推后）
- [ ] post 文件填好，记 1-3 条 lessons 候选
