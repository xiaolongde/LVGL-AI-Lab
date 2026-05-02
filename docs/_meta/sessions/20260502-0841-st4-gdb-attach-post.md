---
type: session-post
task: st4-gdb-attach
pre: 20260502-0841-st4-gdb-attach-pre.md
ended: 2026-05-02T08:43:00+08:00
outcome: success
---

## 实际遇到

按 plan 一次跑通，零卡点。关键证据：

```
Loading section .isr_vector, size 0x130 lma 0x8000000
Loading section .text,        size 0x560 lma 0x8000130
Loading section .init_array,  size 0x4   lma 0x8000690
Loading section .fini_array,  size 0x4   lma 0x8000694
Loading section .data,        size 0x4   lma 0x8000698
Start address 0x0800030c, load size 1692
Transfer rate: 15 KB/sec, 338 bytes/write.

Breakpoint 1, main () at D:/projects/LVGL-AI-Lab/src/main.c:16
16    systick_config();

pc 0x80002da   <main+2>
sp 0x2000bff8  (= _estack - 8, 已 push lr/fp)
lr 0x8000343   <Reset_Handler+53>     ← perfect backtrace 链 main → Reset_Handler
```

stepi 后 PC 跳到 `0x8000250 <systick_config>` —— 真的进了下一函数。

**4 个亮点**：

1. **load 段大小总和 = 0x130+0x560+0x4+0x4+0x4 = 0x69C = 1692 bytes** ←→ build size 报告 `Flash 1692B`，**字节级一致**。链路完全对齐（编译器 + linker + flash + GDB 看到的都是同一份）。
2. **Reset_Handler 出现在 lr 寄存器**：证明启动汇编那条 `bl main` 指令 push lr 后调到 main，回链完整 —— GDB 能 backtrace 上去。
3. **GDB 能解析源文件路径**（`src/main.c:16`、`src/bsp/systick.c:53`）：DWARF 路径 embed 没出 Windows 转码问题，担心多余了。
4. **gdbserver 在 client detach 后自动退出**：pyOCD 默认行为，无需手动 kill background process。

唯一意外 warning（不阻断）：
```
warning: could not convert 'main' from the host encoding (CP1252) to UTF-32.
This normally should not happen, please file a bug report.
```
GDB 在 Windows CP1252 codepage 下处理 UTF-32 内部表示有点抓狂，但 fallback 后正常工作（断点照样命中）。属于 GDB on Windows + Chinese locale 的已知 bug。Lessons 候选: `lessons/gdb-windows-cp1252-warning.md`

## 修复路径

无 — 4 个 pre 卡点都没真触发：
- gdbserver 启动慢：sleep 3 后 connect 一次成功
- gdb -batch 退出：pyOCD 主动收，无僵尸
- main 被 inline 优化：-Og 保留了 main 符号
- DWARF 路径转码：Windows 上 GDB 直接吃绝对路径

## 学到的（方法论素材）

**新 lessons 候选**：

1. **GDB load 段 size sum = build size 是 0 成本完整性验证**：load 输出每段大小列出来，加起来与 `arm-none-eabi-size text+data` 完全一致 —— 不需要写额外测试，眼睛对数字就能确认编译/链接/烧录链路无字节丢失。Lessons 候选: `lessons/gdb-load-size-as-integrity-check.md`

2. **lr 寄存器 + 函数符号 → 0 成本调用栈验证**：在 main 入口看 lr，应该指向调用 main 的那条 bl 之后的地址（即 Reset_Handler 内部）。这种"看 lr 就懂栈"的模式比 `info backtrace` 更轻量，特别适合 startup 链路验证。Lessons 候选: `lessons/lr-as-startup-trace.md`

3. **AI 在 GDB 调试链路上的能力出乎意料地稳**：从写 gdb-init.txt 脚本 → 启 server → batch 跑 → 断点命中 → stepi → detach，全程 0 卡点 ~3 分钟。**capability report 维度**：调试链路的能力分布与传统认知（"调试是高门槛"）相反，AI 在标准化 GDB 操作上比人类快得多（不会忘命令、不会输错端口）。

4. **gdb-init.txt + -batch 的非交互模式是 AI 调试代码的关键 pattern**：交互式 GDB 不适合 AI 操作（无法保持 session），但 `-batch -x init.txt` 把整个调试 sequence 写成可重复的脚本，AI 可以反复跑。这种模式可推广到 chip bring-up 后所有"自动化 sanity check"。Lessons 候选: `lessons/gdb-batch-mode-for-ai.md`

## Card 1 收口状态

**Card 1 = 最小可闭环调试系统** 现在所有底层 chain 验证完毕：

| 链路 | 状态 | 证据 |
|---|---|---|
| 工具链 | ✅ | arm-none-eabi-gcc 12.2 编出 elf |
| 链接 | ✅ | linker script + 启动汇编正确（PC/SP 与符号一致） |
| 烧录 | ✅ | pyOCD flash 1692 B 烧入 |
| 复位运行 | ✅ | 板上 RGB LED 1Hz 三色循环（用户视觉 sign-off） |
| GDB 调试 | ✅ | break main 命中、stepi 进函数、看寄存器/源文件 |

**剩下未做**：USART1 log（st5 in original numbering） + qa-report baseline 录屏（st6）。这两件不挡住 Card 1 的"调试链路完整"主线，可以并行/选做。

## 结果归档

- `tools/gdb-init.txt`（GDB 非交互脚本）
- gdb -batch 跑通：load + break main + stepi + detach 全绿
- gdbserver 自动收（无僵尸进程）
- Card 1 主线 sign-off
