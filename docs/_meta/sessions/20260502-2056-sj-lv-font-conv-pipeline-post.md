---
type: session-post
task: sj-lv-font-conv-pipeline
pre: 20260502-2056-sj-lv-font-conv-pipeline-pre.md
ended: 2026-05-02T21:36:30+08:00
outcome: success
---

## 实际遇到

预判 #2（格式不兼容）**误判，但症状高度伪装**：
- 第一次 `lv_binfont_create` 返 NULL，看起来像格式问题
- 加诊断 dump 文件 header → `30000000|head|01000000` 跟 LVGL v9.5 binfont_loader 期望完全一致
- **真因**：LV_MEM_SIZE=16K → binfont_loader 内部 lv_malloc 分配字体 dsc + cmap + glyph 索引时 heap 不够 → 静默返 NULL

预判外坑：
- **lv_label_set_text_fmt 在 OOM 边缘静默失败**：诊断标签使用 _fmt 全部空白，改 `sprintf+set_text` 才显出来。LVGL 内部 lv_snprintf 走 lv_malloc，OOM 时不报，只是 label 没文本。
- **用户手动拷贝 friction 比预期大**：sj 验收要"用户把 .bin 拷到 SD"。多次"reset → 屏报 not_found → 用户没拷成"。**应用 v0.2 北极星 hands-off**：嵌入 blob + 首次启动 `f_write` 自装到 SD。代价：固件 +16K Flash（250K / 256K = 95.6%，紧但能装）。**自动化让用户零拆卡**。

预判内成立：
- 卡点 #5 LV_MEM_SIZE 不够 → 升 24K 后 binfont 一次过
- 卡点 #4 LFN 必须开（si 已开）→ "montserrat_28.bin" 19 字符顺利写入读出

## 修复路径

**自装流程（dogfood v0.2 hands-off 北极星）**：

```
开机 boot
 → SD full init (sd_io_init pipeline)
 → FATFS_LinkDriver + f_mount
 → install_font_if_missing()
   - f_open(file, FA_READ) 试读 → FR_OK 则跳过
   - 否则 f_open(FA_WRITE | FA_CREATE_ALWAYS) + f_write 嵌入 blob → 屏显 "install OK 16480 B"
 → lv_init + lv_port_disp_init + lv_fs_fatfs_init
 → lv_binfont_create("S:/montserrat_28.bin")
 → lv_obj_set_style_text_font(label, font_big, 0)
```

**关键发现**：sd_disk_write 数据路径在 sg/sh 都没真测过，sj 通过自装首次端到端验证（CMD24 写 + DMA 写 + flush）。**多论据 sign-off 升级**：sg "SD OK" → sh "FATFS read OK" → sj "FATFS write OK + binfont parse OK + LVGL render OK" 形成 3 层栈穿透链。

**LV_MEM_SIZE 16K → 24K** 修了"binfont_create 返 NULL" 假阳性。Flash 250692 B (95.63%) RAM 39336 B (80.03%)。

## 学到的（方法论素材）

**lesson 候选 1：lv_binfont_create FAIL 不是格式问题，先查 LV_MEM_SIZE**

LVGL v9.5 binfont_loader 内部用 lv_malloc 分配多个 buffer（font_dsc / cmap / glyph_dsc / kern table）。LV_MEM_SIZE 不够 → 静默 lv_malloc fail → lv_binfont_create 返 NULL。**症状跟"格式不兼容"完全一样**，但根因是 heap。诊断顺序应该是：(1) **先**确认 LV_MEM_SIZE 至少有 32px font 的 2 倍空间；(2) 再查文件 header；(3) 再查 lv_fs 路径。**LXO38R 第 4 次重现**——lv_conf 配置项 LV_MEM_SIZE 在每次新增重型 LVGL 功能时都要重估。

**lesson 候选 2：嵌入 blob + 首次启动 f_write 自装 = hands-off 资源安装模式**

跟用户手动拷贝相比，自装：
- 零拆卡 friction → 用户体验提升
- f_write 顺带验证 sd_disk_write 数据路径（多论据，比 sg sign-off 更深）
- 代价：固件 size + 资源大小（16K 可接受 / >100K 字体或图片不可接受）
- 自动 idempotent：`f_open(FA_READ)` 探测，存在则跳过

**应用 patch**：v0.2 Card 3+ 所有 SD 资源（字体/图片/配置 JSON）首选自装模式；超大资源（>32KB）才让用户拷贝并写明文件名 + 验收路径。

**lesson 候选 3：LVGL widget 创建 OOM 静默失败**

`lv_label_set_text_fmt` 在 LV_MEM_SIZE 不够时不抛错，label 只是空白。**调试时优先用 sprintf+set_text** 避免 lv_snprintf 内部分配。这是嵌入式 LVGL 调试的反直觉点：widget 看着创建了，文本进不去，开发者会以为是 layout 或 string 问题，实为 heap 问题。

**lesson 候选 4：lv_font_conv 1.5.3 + LVGL v9.5 二进制格式兼容**

实测：`30000000|head|01000000` 这个 header 序列在 v9.5 binfont_loader 直接 parse OK。**version=1 的 .bin 不需要升版**。lv_font_conv 输出的格式跟 LVGL v9.x 是兼容的，预判 #2 是误判。

**autonomous mode 评估**：

- self-recommend：用 lvgl repo 自带 ttf（避下载）+ 嵌入自装（避用户拆卡）+ LV_MEM_SIZE 16→24（直接升）→ 全部正确
- BFS backtrack 没启用（错误次数 < 3 同模式；这次"FAIL → 加 hex dump 诊断 → header 对 → 反推 OOM"是单次链式排查）
- LXO38R 第 4 次：lv_conf 隐式约束在功能扩展时永远要重检；该提到 P0
