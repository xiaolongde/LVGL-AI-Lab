# LVGL-AI-Lab CHANGELOG

## [Unreleased]
- 立项 + design v0 APPROVED + bootstrap 骨架
- v0.1 BACKLOG 重构为 2 张大卡（用户反馈"10 张拖沓"后简化）：Card 1 闭环调试系统 + Card 2 LVGL 表盘；卡内 sub-task 由 AI 自决
- KIT 资料解压 (D:\projects\GD\GD32F303RxT6 KIT\) + `docs/references/{INDEX, mcu, board, screens, sdk}.md` 资料库建立
- lessons/ + capability-report/ 目录预建（v0.1 retro 时填充）
- 工具链已钉死：CMake + arm-none-eabi-gcc + OpenOCD + GDB；烧录器 ST-Link；log USART1 PA9 TX 115200
- **Card 1 / st1**: pre/post 自评工具 `tools/session.sh` (new/close/list) + smoke gate `tests/smoke-test.js`（sessions 完整性检查阶段 1 启用，构建 size + 静态分析阶段 2/3 待激活）
- **Card 1 / st2**: CMake 工程从 KIT/Examples/RGB LED 抄出（vendor/gd32f30x + src + GCC 版 startup_gd32f303rc.S + linker/gd32f303rc.ld + cmake/arm-none-eabi.toolchain.cmake + 顶层 CMakeLists.txt）。`cmake --build build` 干净通过产 .elf/.bin/.hex/.map。size: Flash 1692B (0.83%) / RAM 3120B (7.62%)。smoke-test 阶段 2 (build size) 同步激活。
- **Card 1 / st3**: 烧录 + 调试 chain 通。pyOCD 0.44.0 装于 user 目录（替代 OpenOCD，pip 免 admin），ST-Link/V2 + target stm32f103rc 完美驱动 GD32F303RC SWD。`tools/flash.sh` 一键烧录 + `tools/gdb-server.sh` 启 GDB server :3333。烧录后 PC=0x0800030c / SP=0x2000c000=_estack 完美匹配 linker。references/INDEX.md 烧录段更新为 pyOCD 主路径。✅ 板上 RGB LED 1Hz 三色循环闪（用户视觉 sign-off）。
- **Card 1 / st4**: GDB attach + 断点 chain 通。`tools/gdb-init.txt` 非交互脚本：connect → reset halt → load → break main → stepi → detach。实测 break main 命中 src/main.c:16，PC=0x80002da, lr 链回 Reset_Handler+53，stepi 进 systick_config()。**load 段大小 sum = 1692 B 与 build size 字节级一致**，链路完整性双重验证。Card 1 主线（最小可闭环调试系统）✅ 完成。
- **Card 2 / sa**: LCD 1.47" ST7789V3 (320x172) bring-up。从 KIT/Examples/LCD 1.47inch 抄 spi BSP + tftlcd driver + tft_font 字模到 src/bsp/ + src/devices/lcd/。SPI2 默认引脚 (PA15/PB3/PB4/PB5/PB6/PB7) + 软件 NSS + Mode 3 + /8 prescaler。屏初始化 + 文字 + tick 心跳。Flash 25420B (9.70%)，RAM 4720B (9.60%)。references/board.md SPI 分配修正（之前误写 SPI0 remap）+ references/screens.md 1.47" 部分填实。中途用户澄清两次（IIC→SPI / 1.14→1.47），AI 切换路径 < 5 分钟。
