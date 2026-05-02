---
type: session-pre
task: st4-gdb-attach
backlog_item: card1-minimal-debuggable
started: 2026-05-02T08:41:37+08:00
---

## 任务理解

验证完整调试链：起 pyocd gdbserver → arm-none-eabi-gdb attach → 断在 main → 单步 → 看寄存器 → detach。这是 Card 1 收口动作（"可闭环调试"的"调试"那部分），证明开发循环里 AI 能自查代码运行情况。

## 计划

1. 后台起 `python -m pyocd gdbserver -t stm32f103rc`（监听 :3333）
2. 写 `tools/gdb-init.txt`（gdb 启动脚本）：connect → reset halt → break main → continue → 命中后 info registers / info locals / step → detach → quit
3. `arm-none-eabi-gdb -batch -x tools/gdb-init.txt build/lvgl-ai-lab.elf` 跑非交互验证
4. 验证输出：能看到 "Breakpoint 1 at <addr>: file src/main.c, line N"，stop on breakpoint，PC 落在 main 范围
5. 停 gdbserver
6. session post + commit

## 卡点预判

1. **gdbserver 启动慢 / 端口未就绪**：pyocd gdbserver 启动需 1-2 秒，gdb 太早 connect 会 connection refused。Plan B：bash sleep 2 后再 gdb。
2. **gdb -batch 退出处理不当**：detach 后 quit 应该让 server 持续；但 gdbserver 可能因 client 断开自己退出。会主动 kill background process。
3. **breakpoint at main 不命中**：main 可能被 inline 优化掉——但 -Og -g3 应保留。备选：break Reset_Handler。
4. **gdb 找不到源文件**：DWARF 路径 Windows 转码可能出问题。备选：`directory <src>`。

## 验收

- [ ] gdbserver 起来 + 端口 :3333 可连
- [ ] gdb attach + load + reset halt
- [ ] break main 命中（看到 "Breakpoint 1, main () at src/main.c:N"）
- [ ] info registers PC 在 main 范围 / SP 在 SRAM
- [ ] step 至少一次
- [ ] detach + gdbserver 进程清理
- [ ] 1-2 lessons 候选
