#!/usr/bin/env bash
# gdb-server.sh — 启 pyOCD GDB 服务器，给 arm-none-eabi-gdb attach 用
#
# Usage:
#   tools/gdb-server.sh                    # 默认监听 :3333
#
# 配套用法（另开终端）：
#   arm-none-eabi-gdb build/lvgl-ai-lab.elf
#   (gdb) target extended-remote :3333
#   (gdb) load                              # 烧 elf 进 chip
#   (gdb) monitor reset halt
#   (gdb) break main
#   (gdb) continue
#
# 平台: GD32F303RCT6 / ST-Link/V2 / target stm32f103rc

set -euo pipefail

echo "Starting pyOCD GDB server on :3333 for stm32f103rc..."
echo "Connect with: arm-none-eabi-gdb -ex 'target extended-remote :3333' build/lvgl-ai-lab.elf"
echo ""
exec python -m pyocd gdbserver -t stm32f103rc
