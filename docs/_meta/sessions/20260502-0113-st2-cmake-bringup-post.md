---
type: session-post
task: st2-cmake-bringup
pre: 20260502-0113-st2-cmake-bringup-pre.md
ended: 2026-05-02T01:18:12+08:00
outcome: success
---

## 实际遇到

按 plan 走通，第一次 cmake configure 一发过、第一次 build 出 1 个真错（`GD32F30X_HD` 宏未定义），加上后第二次 build 直接 link 成功。最终产物：

```
text    data    bss     filename
1680    12      3108    build/lvgl-ai-lab.elf
Flash 1692 B / 256 KB (0.65%)
RAM   3120 B /  48 KB (7.62%)
```

3 个意外（都不挡路）：

1. **GD32F30X_HD 宏未定义**：vendor 库 `gd32f30x_rcu.c` 里 `RCU_CTL` / `RCU_CFG0` / `RCU_DSV` 等寄存器名 undeclared。原因：`gd32f30x.h:46-49` 要求外部定义 `GD32F30X_HD`/`XD`/`CL` 之一，否则不进入任何家族的寄存器定义块。pre 里漏写。修复：CMake 加 `target_compile_definitions(gd32f30x PUBLIC GD32F30X_HD HXTAL_VALUE=8000000U)`。

2. **HXTAL_VALUE 双重定义**（warning 级，不挡）：vendor 自己在 `gd32f30x.h:59-61` 同时 `#define` 了 25M 和 8M 两个值（第二个赢）。我外部再 -D 一遍触发 `-Wmacro-redefined`。v0.1 留着，未来要么删 header 双定义、要么 `-Wno-...` 抑制。

3. **Linker RWX LOAD segment 警告**：`warning: lvgl-ai-lab.elf has a LOAD segment with RWX permissions`。ARM toolchain 12.x 的 W^X 严格化检查。embedded 上无害，留 v0.2。

3 个 newlib stub warning (`_close/_lseek/_read is not implemented`) 是预期——nano libc 没装这些 syscalls，main 不调用不会真触发。

## 修复路径

错误 1 一行 CMake 改完就过：
```cmake
target_compile_definitions(gd32f30x PUBLIC
    GD32F30X_HD
    HXTAL_VALUE=8000000U
)
```

错误 2 / 3 / newlib warnings 都不挡路，留 v0.2 处理。

## 学到的（方法论素材）

**新的 lessons 候选**：

1. **从 vendor SDK 起 CMake 工程的标准 sequence**：先理清 vendor 目录（CMSIS / 标准外设库 / app / startup / linker 5 部分），按关注点分离映到项目（`vendor/` + `src/` + `linker/` + `startup/` + `cmake/`）。Lessons 候选: `lessons/vendor-sdk-to-cmake-skeleton.md`

2. **Keil .s → GCC .S 翻译标准模板**：核心是 `.section .isr_vector / .global g_pfnVectors / .word <handler>` 向量表 + Reset_Handler（data 段复制 + bss 清零 + SystemInit + __libc_init_array + main + 死循环）+ `.weak + .thumb_set` 把所有 IRQ alias 到 Default_Handler。一次写对的关键：**抄 KIT 的 Keil 启动汇编看完整向量表**（GD32F30x HD 共 76 项，含 USBD_HP_CAN0_TX 等 GD 特有），别从 STM32F1 example 抄少了。Lessons 候选: `lessons/keil-to-gcc-startup-translation.md`

3. **`CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY`** 是裸机交叉编译必加：cmake try_compile 默认 link，无 startup 永远 fail。Lessons 候选: `lessons/cmake-cross-compile-bare-metal.md`

4. **GD32 firmware lib 必须显式定义家族宏**（HD/XD/CL）：vendor header 不给默认，编译就 undefine。一致体验是 ST 的 STM32F4 series 有 `STM32F407xx` 这种型号 specific 的——GD32 用 family 粒度，更粗。Lessons 候选: `lessons/gd32-family-define-quirk.md`

5. **从干净起手到第一个 .elf 的真实成本约 30 分钟**（含读资料、写 startup、linker、CMake、修 1 个错）——对 capability report 是个 datapoint：AI 在"vendor 资料齐 + 工具链已装"时，从 0 到 .elf 高度自动化，需要 1 次 build 失败暴露 family 宏未定义这种 vendor 特定坑。

## 结果归档

- `cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.toolchain.cmake` ✅
- `cmake --build build` ✅，产 `lvgl-ai-lab.{elf,bin,hex,map}`
- Size: Flash 1692 B (0.83% of 200K limit) / RAM 3120 B (7.62% of 40K limit)
- smoke-test.js 阶段 1 + 2 双绿（阶段 2 size 检查已激活）
- 阶段 3（`-Werror`）继续 stub——清理 4 处 warning（systick old-style decl + usart unused params）后再激活；推到 st3 或 st6
