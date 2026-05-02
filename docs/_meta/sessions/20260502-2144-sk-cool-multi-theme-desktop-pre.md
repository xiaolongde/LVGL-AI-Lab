---
type: session-pre
task: sk-cool-multi-theme-desktop
backlog_item: card3-sd-fatfs-gui-resources
started: 2026-05-02T21:44:58+08:00
---

## 任务理解

v0.2 Card 3 收尾：用 sj 通的"SD 加载 28px 大字"做 3 个酷桌面主题（cyberpunk / modular / minimal），每 5s 自动切换，全部素材从 SD 加载。证明 v0.2 的"突破 256K Flash + LVGL 资源 SD 加载"真正能撑起完整应用。

## 计划

1. 共享：font_big = lv_binfont_create("S:/montserrat_28.bin")（已通）+ fake desktop_state（hh/mm/steps/hr/batt/uptime）
2. 3 个 theme 函数：theme_cyberpunk / theme_modular / theme_minimal，签名 `void(lv_obj_t* scr, const desktop_state_t*)`
3. 主循环：每 50ms 推进 lv_timer_handler；每 60 tick (3s 加速) +1 分钟+步数；每 100 tick (5s) lv_obj_clean(scr) + 切下个 theme
4. 静态 fake 数据，不接真传感器（v0.3）

## 卡点预判

1. **Flash 紧**：sj 已 95.7%，加 3 theme + lv_arc 估 +2-5K，可能超
2. **LV_MEM_SIZE OOM**：3 theme 各自创建 widget，切换时 lv_obj_clean 应释放，但 binfont 常驻 + 单 theme 渲染 + DMA buffer 总和可能 > 24K
3. **lv_arc 在 partial buffer 模式**：sd 时 transform_scale + partial 失败过；arc 是基础几何应该 OK
4. **lv_obj_clean 是否真的全释放 child widget**：如不彻底，每次切换泄漏 → 几轮后 OOM

## 验收

- [ ] 3 个 theme 都能渲染（视觉 sign-off）
- [ ] 切换 ≥10 次后还稳定（无 OOM）
- [ ] post 含 lessons 候选

## 元注释（autonomous mode）

- 视觉风格用户给"3 个都做" → self-decide layout 细节
- LV_MEM_SIZE 超就升 24K → 32K（虽然 RAM 紧）
- lv_arc 失败时降级用纯 label
