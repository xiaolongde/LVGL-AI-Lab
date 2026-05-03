#!/usr/bin/env python3
"""
auto_verify.py — diff snapshot PPMs vs golden baseline, print PASS/FAIL.

Usage:
    python tools/auto_verify.py [SNAPSHOT_DIR] [GOLDEN_DIR]

Defaults:
    SNAPSHOT_DIR = build_host/snapshots
    GOLDEN_DIR   = tools/golden

Diff metric: per-pixel absolute RGB difference.
Threshold for PASS: mean diff < 4 / 255 (≈1.5%) AND max diff < 32 / 255 (≈12.5%).
PPM-only, no external libs (stdlib only).
"""
import sys
import os


def read_ppm(path):
    """Read binary P6 PPM. Return (width, height, raw_rgb_bytes)."""
    with open(path, 'rb') as f:
        data = f.read()
    # parse header: "P6\n<w> <h>\n255\n"
    idx = 0
    def next_token():
        nonlocal idx
        # skip whitespace + comments
        while idx < len(data) and data[idx:idx+1] in (b' ', b'\n', b'\r', b'\t'):
            idx += 1
        if data[idx:idx+1] == b'#':
            while idx < len(data) and data[idx:idx+1] != b'\n':
                idx += 1
            return next_token()
        start = idx
        while idx < len(data) and data[idx:idx+1] not in (b' ', b'\n', b'\r', b'\t'):
            idx += 1
        return data[start:idx]

    magic = next_token()
    if magic != b'P6':
        raise ValueError(f"not a P6 PPM: {path}")
    w = int(next_token())
    h = int(next_token())
    maxv = int(next_token())
    if maxv != 255:
        raise ValueError(f"PPM maxv != 255 ({maxv}): {path}")
    # skip exactly one whitespace
    idx += 1
    pixels = data[idx:idx + w * h * 3]
    if len(pixels) != w * h * 3:
        raise ValueError(f"PPM data short: {len(pixels)} vs {w*h*3}")
    return w, h, pixels


def diff(a_path, g_path):
    aw, ah, a = read_ppm(a_path)
    gw, gh, g = read_ppm(g_path)
    if (aw, ah) != (gw, gh):
        return None, f"dim mismatch {aw}x{ah} vs {gw}x{gh}"
    n = len(a)
    total = 0
    mx = 0
    for i in range(n):
        d = abs(a[i] - g[i])
        total += d
        if d > mx:
            mx = d
    mean = total / n
    return (mean, mx), None


def main():
    snap_dir = sys.argv[1] if len(sys.argv) > 1 else "build_host/snapshots"
    gold_dir = sys.argv[2] if len(sys.argv) > 2 else "tools/golden"

    if not os.path.isdir(snap_dir):
        print(f"ERR: snapshot dir not found: {snap_dir}")
        sys.exit(2)
    if not os.path.isdir(gold_dir):
        print(f"ERR: golden dir not found: {gold_dir}")
        sys.exit(2)

    snapshots = sorted(f for f in os.listdir(snap_dir) if f.endswith('.ppm'))
    golden = sorted(f for f in os.listdir(gold_dir) if f.endswith('.ppm'))

    if set(snapshots) != set(golden):
        print(f"WARN: file set mismatch")
        print(f"  snapshot: {snapshots}")
        print(f"  golden:   {golden}")

    overall_pass = True
    print(f"{'name':<16}  {'mean':>6}  {'max':>4}  result")
    print("-" * 44)
    for f in snapshots:
        if f not in golden:
            print(f"{f:<16}  {'-':>6}  {'-':>4}  ABSENT IN GOLDEN")
            overall_pass = False
            continue
        result, err = diff(os.path.join(snap_dir, f), os.path.join(gold_dir, f))
        if err:
            print(f"{f:<16}  {'-':>6}  {'-':>4}  ERR: {err}")
            overall_pass = False
            continue
        mean, mx = result
        # threshold: mean < 4 (~1.5%), max < 32 (~12.5%)
        passed = (mean < 4 and mx < 32)
        if not passed: overall_pass = False
        print(f"{f:<16}  {mean:>6.2f}  {mx:>4d}  {'PASS' if passed else 'FAIL'}")
    print("-" * 44)
    print(f"OVERALL: {'PASS' if overall_pass else 'FAIL'}")
    sys.exit(0 if overall_pass else 1)


if __name__ == '__main__':
    main()
