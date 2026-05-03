---
type: session-pre
task: m4-golden-image-auto-verify
backlog_item: v0.2-northstar-hands-off
started: 2026-05-04T00:03:23+08:00
---

## 任务理解

v0.2 设计文档里的"hands-off"北极星另一半：消除"用户视觉验收"环节。AI 自截屏 + 比对 baseline + PASS/FAIL，零用户介入即可验证视觉无回归。

## 计划

1. snapshot_util.{c,h}：lv_snapshot_take(scr, RGB565) → 转 RGB888 → 写 PPM (P6 binary)
2. main.c 加 `--snapshot <dir>` cmdline mode：固定 fake state，每 theme 渲染后截屏 → 退出
3. tools/golden/：3 个 baseline PPM (TERMINAL/PIXEL/ZEN)
4. tools/auto_verify.py：纯 stdlib diff 工具（mean + max abs）
5. tools/auto-verify.sh：一键 build + snap + diff
6. host build 加 LV_USE_SNAPSHOT=1 + LV_MEM_SIZE 升 1MB（snapshot buf 108K + 5 fonts 50K）

## 卡点预判

1. **lv_snapshot_take 返 NULL**：可能是 LV_MEM_SIZE 不够（应验：第一次 128K + 5 fonts + 108K snapshot = OOM）
2. **CWD 不对**：lv_fs_stdio 用 fopen("./..."), CWD=进程当前目录而不是 exe dir → fonts 找不到。修：`cd build_host && ./lvgl-ai-lab-host.exe`
3. **stdout 不输出**：mingw exe + git bash + `2>&1` 不一定捕获到。fallback：写日志文件
4. **PPM 格式细节**：P6 binary RGB888，header 里 magic + w h + maxv + 一空白 + raw bytes
5. **threshold 选择**：完全相同 build = 0 diff；浮点字体/抗锯齿可能 1-2 像素 diff；threshold mean<4 max<32 给点余量

## 验收

- [ ] --snapshot mode 跑完不挂，3 个 PPM 写出
- [ ] auto_verify.py 能正确 diff 两个 PPM
- [ ] auto-verify.sh 全链路跑通 PASS
- [ ] post 含 lessons
