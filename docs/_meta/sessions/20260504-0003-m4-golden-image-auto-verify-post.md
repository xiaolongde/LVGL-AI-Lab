---
type: session-post
task: m4-golden-image-auto-verify
pre: 20260504-0003-m4-golden-image-auto-verify-pre.md
ended: 2026-05-04T00:12:56+08:00
outcome: success
---

## 实际遇到

预判全部命中：

1. **rc=-1 第一次跑**：lv_snapshot_take 返 NULL → LV_MEM_SIZE=128K 不够。改 1MB 后 OK。预判 #1 验证。
2. **stdout 静默**：mingw windowed exe 在 git bash 下 `2>&1` 没捕获到 — 写文件日志才看到。预判 #3 验证。
3. **CWD 问题**：`./build_host/lvgl-ai-lab-host.exe` 跑时 CWD=项目根，fonts 在 build_host/ 找不到 → 改 `cd build_host && ./lvgl-ai-lab-host.exe`。预判 #2 验证。

无意外：

- snapshot_screen_to_ppm 一次写对（PPM P6 binary 格式简单：`P6\n<w> <h>\n255\n` + raw RGB888 bytes）
- auto_verify.py 一次写对（纯 stdlib，无 PIL）
- threshold mean<4 max<32 一次合理（实测 baseline=baseline = 0/0 PASS）

## 修复路径

**完整 pipeline**：
```
tools/auto-verify.sh:
  cmake -B build_host -DBUILD_HOST_SIM=ON ...
  cmake --build build_host
  cd build_host && ./lvgl-ai-lab-host.exe --snapshot ./snapshots
  python tools/auto_verify.py build_host/snapshots tools/golden
```

snapshot mode 流程：
```
hw_boot → lv_init → hw_lv_disp_init (Win32 window)
  → load 5 fonts → install + load 3 styles
if (--snapshot dir):
  for each of 3 themes:
    lv_obj_clean(scr)
    theme.render(scr, fixed_state, theme.style)
    8 × lv_timer_handler()        # let LVGL flush
    lv_obj_update_layout(scr)
    snapshot_screen_to_ppm(...)   # lv_snapshot → PPM
  exit(0)
```

diff: per-pixel abs(a-g) — mean + max。相同 build = 0/0；像素位移 1-2 是 1-2/N。threshold mean<4 max<32 容许小漂移（字体浮点 / antialiasing）但 catch 真正视觉变化。

## 学到的（方法论素材）

**lesson 候选 1：v0.2 北极星（hands-off 视觉验收）落地路径明朗**

完整闭环 = m1 host simulator + m4 自截屏自验。配合：
- (a) `auto-verify.sh` 在 CI / pre-commit hook 自动跑
- (b) 每次改 theme → 自动看 diff，如果意外破坏其他 theme（cross-theme 回归）立刻被 catch
- (c) 设计师改 .tstyle → AI 跑 verify → "看着不变" / "比预期变了 X%" 给反馈

**lesson 候选 2：PPM 格式是嵌入式视觉验收的最佳选择**

不依赖 libpng/libjpeg/zlib，单 fwrite 字节流。Python 端纯 stdlib 解析（45 行）。LVGL 用 lv_snapshot 出 RGB565，转 RGB888 一次扫描。**用 PNG 就要 stb_image_write 或者 zlib，依赖增 5KB**；PPM 增 0KB 依赖。这是嵌入式自动验收的标准选择。

**lesson 候选 3：mingw windowed exe 在 git bash 下 stdout 不可靠**

不要假设 `printf` 能被 `2>&1 | tee` 抓到。Windowed app 的 stdout 在 console host (cmd.exe) 默认连接到 console，但 git bash 用的是 mintty + ConPTY，与 mingw exe 的 stdio 兼容性不一致。**调试时优先写日志文件**，stdout 仅在 cmd.exe 里可靠。

**lesson 候选 4：lv_snapshot OOM 静默 NULL**

lv_snapshot_take 内部 lv_draw_buf_create 失败就返 NULL，没 LV_LOG 提示（除非 LOG_LEVEL=WARN+）。又一个 LVGL OOM 静默失败案例（前有 lv_label_set_text_fmt LEZSTN）。**新症状归到 LXO38R 大组合下**：LVGL 任何 alloc 失败大概率静默，遇 NULL 返回先怀疑 heap。

## 落地状态

- m4 完成，3 PPM baseline 在 tools/golden/
- auto-verify.sh 一键跑 PASS
- 后续 theme/style 改动建议 auto-verify 在 commit 前跑
