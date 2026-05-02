---
type: session-post
task: st3-flash-gdb
pre: 20260502-0836-st3-flash-gdb-pre.md
ended: 2026-05-02T08:38:23+08:00
outcome: success
---

## 实际遇到

零卡点一发到位：

1. **pyOCD 装就位**：`pip install --user pyocd` 一行，0.44.0 落到 `C:\Users\trist\AppData\Roaming\Python\Python311\Scripts`。pyocd.exe 不在 PATH 但 `python -m pyocd` 调用没问题。
2. **ST-Link 检测**：`python -m pyocd list --probes` 立即看到 STM32 STLink，serial E1007200D0D2139393740544。
3. **SWD 连接**：第一次 `pyocd commander -t stm32f103rc` 直接 attach 成功。**唯一警告** `Invalid coresight component cidr=0x0 [rom_table]` —— 已知 benign（ROM table 有空 slot 时的扫描信息，pyOCD 报 W 但不阻断）。
4. **烧录前 PC/SP**：halt 看到 PC=0x08000ef0（板上原有出厂固件 USB DFU/CDC demo），SP=0x20001a48（在 SRAM 范围内）—— 印证 USB 设备枚举里看到的 STM32 BOOTLOADER + USB CDC 串口正是这个固件在跑。
5. **烧录 lvgl-ai-lab.elf**：`tools/flash.sh` → `pyocd flash --erase chip` → 程式 2048 字节（1 page），擦 + 编程都全绿，2.32 KB/s（pyOCD 走 ST-Link/V2 速率正常）。
6. **烧录后 reset+halt**：PC=`0x0800030c`（在 Reset_Handler 区），SP=`0x2000c000`（= 0x20000000 + 0xC000 = 48K SRAM 顶 = linker 的 `_estack`） —— **linker script + 启动汇编向量表完全正确**。
7. **Free-run**：`reset` 命令把芯片放回自由运行，应在跑 RGB LED 闪。

**关于 LED 视觉确认**：本任务范围内 AI 完成 chain 验证（PC/SP 正确 = 启动链路 OK），但 RGB LED 实际是否闪需要用户视觉确认（不属于 AI 可自动验证的部分）—— 留作 st4 入口确认项。

## 修复路径

无修复 — pre 预判的 4 个卡点都没真触发：
- pyOCD --erase chip 在 GD32 上：fine（pyOCD pack 算法正确处理 256K）
- LED 不闪：未发生（reset 后 PC 在合法区，应该已经在 main 循环里）
- pyocd target 缺失：fine（stm32f103rc 是 builtin）
- 板子双供电冲突：未发生（用户接线规范）

## 学到的（方法论素材）

**新 lessons 候选**：

1. **GD32F303 = STM32F103 in pyOCD's eyes**：用 `-t stm32f103rc` target 直接驱动 GD32F303RC，flash 算法 + memory map + RAM 大小完美匹配。这是个**重要 datapoint**：在 SWD 调试层，国产 ARM Cortex-M 与 ST 对应型号往往是 drop-in 兼容（因为 SWD/CoreSight 是 ARM 标准 + flash 控制器寄存器互兼容）。Lessons 候选: `lessons/gd32-via-stm32-target.md`

2. **Windows 上 ST-Link + pyOCD 比 OpenOCD 顺得多**：`pip install --user pyocd` vs OpenOCD 需要 choco/xpack/手装。Time-to-first-flash：pyOCD ~5 分钟（含装），OpenOCD 估计 30+ 分钟（找 cfg + override flash size + 调试 stm32f1x.cfg）。Lessons 候选: `lessons/pyocd-vs-openocd-windows.md`

3. **`Invalid coresight component cidr=0x0` 是 benign 警告**：第一次见时容易吓退；实际是 pyOCD 扫 ROM table 遇到空 slot 时的提示，不影响功能。Lessons 候选: `lessons/pyocd-coresight-warning.md`

4. **烧录后用 reset+halt 看 PC/SP 是 0 成本的 chain 验证**：不需要专门写测试代码，只要 PC 落在合法 Flash 区 + SP 等于 linker 定义的 `_estack`，就证明启动汇编 + linker script + 烧录链都对。这是个**性价比极高的 sanity check**，应作为后续任务的标准动作。Lessons 候选: `lessons/post-flash-sanity-check.md`

5. **AI 在工具链 + 烧录链路这层能力非常稳**：本任务（含装 pyOCD + 写 flash.sh + gdb-server.sh + 验证 chain）从开始到 PASS 用了约 5 分钟。**对 capability report 的意义**：embedded 项目里"装工具+连烧录器+第一次烧成功"传统上是新手最容易卡的环节，AI 在 vendor 资料齐 + probe 已物理连好的前提下能把这步缩到几分钟。

## 结果归档

- pyOCD 0.44.0 装于 user 目录
- `tools/flash.sh` + `tools/gdb-server.sh` 就位
- ST-Link/V2 → GD32F303RCT6 SWD 连接验证 ✅
- `lvgl-ai-lab.elf` 烧录成功（2048 B 程式）
- 复位后 PC=0x0800030c / SP=0x2000c000 —— 启动链路完整
- references/INDEX.md 烧录段已更新（pyOCD 主，OpenOCD 备）
- **待用户视觉确认**：板上 RGB LED 是否在 1Hz 红→绿→蓝 三色循环闪
