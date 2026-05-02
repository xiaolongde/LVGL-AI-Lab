---
type: session-pre
task: sj-lv-font-conv-pipeline
backlog_item: card3-sd-fatfs-gui-resources
started: 2026-05-02T20:56:46+08:00
---

## 任务理解

v0.2 Card 3 第 4 站：用 lv_font_conv 离线把 Montserrat-Medium.ttf 转成 .bin，放到 SD 卡，固件用 LVGL `lv_binfont_create("S:/font.bin")` 加载，作为大字标签的 font。这突破 256K Flash 上限——大字 28px 字体 ASCII 全集 ~16KB，过去 sd 时塞不进 Flash 让 GT3 表盘只能用 14px+framing 凑。

## 计划

1. 找 Montserrat-Medium.ttf 源（lvgl repo 自带 `external/lvgl/scripts/built_in_font/Montserrat-Medium.ttf`，无需下载）
2. `npx lv_font_conv --font ... --size 28 --bpp 4 --range 0x20-0x7F --format bin -o tools/font_out/montserrat_28.bin`
3. 启 LV_USE_BINFONT_LOADER（如有）+ 引 lv_binfont_loader.h
4. main.c 改：`lv_binfont_create("S:/montserrat_28.bin")` + `lv_obj_set_style_text_font(label, font_big, 0)` + `lv_label_set_text("12:34")` 中央大字
5. **用户拷贝 .bin 到 SD 卡根目录**

## 卡点预判

1. **Flash overflow**：si 已 88%，加 binfont_loader 还能装；估 +3K
2. **lv_font_conv 输出格式 vs LVGL v9.5 binfont_loader 不兼容**：v9.5 二进制格式可能要 v3 而 lv_font_conv 1.5.3 出 v1
3. **user 拷贝步骤**：手动操作有 friction，应用 v0.2 北极星 hands-off 思路 — 能否固件自装？
4. **LFN 文件名**：montserrat_28.bin 19 字符 > 8.3，依赖 FF_USE_LFN=1（si 已确认）
5. **LV_MEM_SIZE**：binfont 加载需 heap 分配，16K 可能不够

## 验收

- [ ] tools/font_out/montserrat_28.bin 生成（~16KB）
- [ ] lv_binfont_create 返非 NULL
- [ ] 屏中央 28px "12:34" 大字渲染（视觉 sign-off）
- [ ] post 含至少 1 个 lesson 候选

## 元注释（autonomous mode）

- 用 lvgl repo 自带 ttf 而不是下载新的 → self-recommend
- 卡点 #3 hands-off：如手动拷贝 friction 大，自决用嵌入 blob + 首次启动 f_write 自装
- 卡点 #5 LV_MEM_SIZE：先尝试 16K，OOM 时升 24K
