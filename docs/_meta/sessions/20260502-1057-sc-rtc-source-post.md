---
type: session-post
task: sc-rtc-source
pre: 20260502-1057-sc-rtc-source-pre.md
ended: 2026-05-02T11:48:12+08:00
outcome: success
---

## 实际遇到

按 plan 一次跑通，零卡点。`src/bsp/rtc.{c,h}` 写完即工作：
- LSE 32.768 kHz 起振 OK（板上 XT2 优质）
- BKP 写保护流程对（PMU+BKP 时钟 → backup_write_enable → bkp_reset → osci_on）
- prescaler 32767 → 1Hz tick 准
- counter 默认 12:00:00 (43200) 工作

main.c 加 second_timer_cb 每秒读 `rtc_get_time` 更新 lv_label，时间正常递增 ✅。pre 预判的 6 个卡点都没真触发——LSE 起振无超时、BKP 写保护一次过、prescaler 算对、lwoff_wait 工作。

附带：测全屏 FPS @ 120×120 spinner = **30 FPS**，对应 28800 B/帧 × 30 = **864 KB/s 实测**，SPI2 @ 7.5 MHz 理论 940 KB/s → **92% 利用率**。整屏 110KB 推算 ~8.5 FPS 理论上限。

## 修复路径

无需修复。

## 学到的（方法论素材）

1. **GD32F303 RTC 是 STM32F1 风格 32-bit counter**：vendor lib `gd32f30x_rtc.h` API 简洁（counter_get/set + prescaler）。无 calendar 寄存器需软件转 H:M:S。LSE 起振 + BKP 解锁 + 配 prescaler 三步即工作。Lessons 候选: `lessons/gd32-rtc-bringup-recipe.md`

2. **LSE 起振在板载 XT2 + 32.768kHz 晶振配置下稳定**：rcu_osci_stab_wait 一次过，不需要 LSI fallback。Lessons 候选 datapoint: 良好板载晶振 LSE 起振时间 < 100ms。

3. **AI 在 vendor lib API 调度 + 简单 BSP 模块的能力非常稳**：从写 rtc.{c,h} 到时间 label 走起来 ~3 分钟（含 cmake build + flash），零卡点。**capability report 维度**: 当 vendor 文档齐 + API 签名清晰 + 无外部依赖时，AI 在嵌入式 BSP 这类任务能一把完成。

4. **Card 2 整屏 FPS 8.5 是 SPI2 物理上限**（@7.5MHz / 8bit）：未来要更高 FPS 需 (a) DMA 而非 polling SPI (b) 提高 SPI 时钟（实测可推到 30+ MHz on ST7789）(c) 减少 dirty area。Lessons 候选: `lessons/lcd-spi-fps-physical-limit.md`

## 结果归档

- `src/bsp/rtc.{c,h}` 上线
- main.c 加 RTC 时间显示 + 1 秒 timer
- 全屏 FPS 实测：30 @ 120×120 spinner（92% SPI 利用率）
- LSE + BKP + RTC 一次跑通 ✅

---

## 元反思 — sb 后期 sysmon + spinner 问题为什么走了 6 个回合

用户 sb 后期连续 4 次反馈"X 没看到"才让所有元素都正确显示。复盘根因：

### 失败 1：LV_USE_PERF_MONITOR 启用后 sysmon overlay 不可见
- **根因**：LV_USE_PERF_MONITOR + LV_USE_MEM_MONITOR 都依赖 LV_USE_SYSMON=1，但 SYSMON 默认 0
- **本可避免**：开 PERF_MONITOR 时一并 grep `LV_USE_SYSMON` 看是否需要联动开。**没读 lv_conf 完整依赖链** = 经典"配置项孤岛思维"
- **Lessons**: `lessons/lvgl-conf-dependency-chains.md`（开 X 前 grep 同区域所有 LV_USE_* 看依赖）

### 失败 2：sysmon overlay 启用后又和 banner 撞车
- **根因**：banner 居中 + 字"GD32F303 + ST7789V3"长，与 PERF overlay TOP_RIGHT 横向重叠
- **本可避免**：每加新 widget 应**画一遍坐标布局图**确认无重叠。我直接照抄 KIT 的 banner 文字（2 行 + 长字）没对齐 sysmon
- **Lessons**: `lessons/widget-layout-collision-check.md`

### 失败 3：spinner 创建但不可见
- **根因**：sb 早期为减 size 关 `LV_DRAW_SW_COMPLEX = 0`；arc widget 渲染依赖 SW_COMPLEX，spinner 是 arc 子类 → 创建成功但无像素
- **本可避免**：关 SW_COMPLEX 时应**逐字读** `lv_draw_sw_arc.c` 里的 `LV_LOG_WARN("Can't draw arc with LV_DRAW_SW_COMPLEX == 0")` —— LVGL 自己说了死话
- **Lessons**: `lessons/lvgl-draw-sw-complex-affects-arc.md`

### 失败 4："banner 残影"诊断走 3 次错方向
- **根因**：用户说"残影"，我**没问清具体长什么样**就猜了 3 次都是"GRAM 残留 + LVGL 没刷"。实际上是 banner 字本身的 AA 边缘视觉。
- **本可避免**：systematic-debugging 的 Step 2 维度 1 "逐字读失败信号"——我把"残影"这个**模糊词**当具体描述用。**当用户的描述歧义 ≥2 种解读时，先问澄清而不是猜**
- **触发了 pua-debugging skill**，矩阵化假设后 5 分钟定位（vs 之前 30+ 分钟瞎试）

### 失败 5：spinner 显示恢复后 LVGL 卡死
- **根因**：开 SW_COMPLEX + sysmon + spinner + 多 label 同时启用 → LV_MEM_SIZE=8KB OOM → TLSF allocator 死循环（GDB 5 次采样 PC 都在 `lv_draw_unit_draw_letter → insert_free_block`）
- **本可避免**：sb 初期定 LV_MEM_SIZE 8KB 时是预留多 widget 余量 — 但我没建立"加 widget 时同步检视 heap"的习惯
- **Lessons**: `lessons/lvgl-mem-size-grows-with-widgets.md`

### 共同模式（这是真 lesson）

5 个失败合起来的元 pattern：**AI 倾向局部解决（关一个 flag、加一个 widget）而忘记检查全局影响**。
- 关 LV_DRAW_SW_COMPLEX → 没看哪些 widget 依赖
- 加 PERF_MONITOR → 没看 LV_USE_SYSMON 联动
- 加 widget → 没核对布局碰撞
- 改 widget 集合 → 没复审 LV_MEM_SIZE
- 用户说"残影" → 没确认是哪种残影

**修法**：每次 lv_conf.h 改动 / widget 增加，过一遍 5 项核检：
1. 这个开关有没有依赖（grep 同名宏在 LVGL src 出现的位置）
2. 这个 widget 实际渲染需要哪些 draw flag
3. 与现有 widget 的坐标 / z-order 冲突
4. heap 估算是否仍 fit
5. 用户给的描述是不是有歧义需要澄清

代价：每次改动 +30 秒核检，省后续 5-10 分钟来回修。**ROI 极高，但本能反应是"快、改一行就 build"**。这是 AI 嵌入式开发**纪律 ≫ 灵感**的最佳印证。

Lessons 候选: `lessons/local-vs-global-config-discipline.md`（重要 + 跨任务可复用）
