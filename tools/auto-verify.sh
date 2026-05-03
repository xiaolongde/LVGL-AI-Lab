#!/usr/bin/env bash
# auto-verify.sh — full host build → snapshot → diff baseline → PASS/FAIL
# 用于 v0.2 北极星："消除用户视觉验收"。
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HOST_DIR="$ROOT/build_host"
SNAP_DIR="$HOST_DIR/snapshots"
GOLD_DIR="$ROOT/tools/golden"

PATH="/c/msys64/mingw64/bin:$PATH"

if [ ! -d "$HOST_DIR" ]; then
    echo "[1/4] cmake configure host build"
    cmake -B "$HOST_DIR" -G Ninja -DBUILD_HOST_SIM=ON \
        -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -S "$ROOT" >/dev/null
fi

echo "[2/4] cmake build host"
cmake --build "$HOST_DIR" --target lvgl-ai-lab-host >/dev/null

# kill any running host instance to avoid file lock
taskkill //F //IM lvgl-ai-lab-host.exe >/dev/null 2>&1 || true

mkdir -p "$SNAP_DIR"
echo "[3/4] capture snapshots"
( cd "$HOST_DIR" && timeout 8 ./lvgl-ai-lab-host.exe --snapshot ./snapshots ) || true

echo "[4/4] diff vs baseline"
python "$ROOT/tools/auto_verify.py" "$SNAP_DIR" "$GOLD_DIR"
