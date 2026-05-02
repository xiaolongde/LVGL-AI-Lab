#!/usr/bin/env bash
# flash.sh — 烧录 ELF 到 GD32F303RC via ST-Link + pyOCD
#
# Usage:
#   tools/flash.sh                          # 默认烧 build/lvgl-ai-lab.elf
#   tools/flash.sh path/to/firmware.elf
#
# 平台: GD32F303RCT6 (Cortex-M4F, 256K Flash)
# Probe: ST-Link/V2 via SWD
# Target: stm32f103rc（GD32F303 的 SWD/registers 与 STM32F103 兼容；GD 的 256K Flash > F103 标准 256K，pyOCD pack 已正确）

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEFAULT_ELF="$ROOT/build/lvgl-ai-lab.elf"
ELF="${1:-$DEFAULT_ELF}"

if [[ ! -f "$ELF" ]]; then
    echo "ERR: ELF 不存在: $ELF"
    echo "先 cmake --build build"
    exit 1
fi

echo "Flashing $ELF to GD32F303RC via ST-Link..."
python -m pyocd flash \
    -t stm32f103rc \
    --erase chip \
    "$ELF"
echo ""
echo "Done. 复位后可看 LED。"
