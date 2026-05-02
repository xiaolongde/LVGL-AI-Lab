---
type: session-pre
task: sc-rtc-source
backlog_item: card2-lvgl-watch-face
started: 2026-05-02T10:57:48+08:00
---

## 任务理解

让 RTC 走起来：用板载 LSE 32.768 kHz 晶振 + GD32F30x 内置 RTC，输出 HH:MM:SS 给 LVGL label 显示。**KIT/Examples/RTC 名字误导**——它实际是 USART demo（没用 RTC API），所以 rtc.c/h 要从头写，API 调 vendor lib `gd32f30x_rtc.h`。

GD32F30x RTC = STM32F1 风格（32-bit counter，不是 F4 的 calendar 型）。秒数从 prescaler 31999 起算（32768/32000 ≈ 1Hz 太接近，用 32767 → 0.99996Hz 实际跑 32768/32768=1Hz）。

## 计划

1. 写 `src/bsp/rtc.{c,h}`：
   - `void rtc_user_init(void)` — 配 LSE + prescaler + counter=0（首次开机假设是 12:00:00）
   - `void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s)` — 把 32-bit counter 转 H:M:S，按一天周期循环
   - 启用顺序：`rcu_periph_clock_enable(RCU_PMU)` → `pmu_backup_write_enable` → `rcu_periph_clock_enable(RCU_BKPI)` → start LSE → wait stable → set RTCCLK to LSE → enable RTC clock → `rtc_register_sync_wait` → `rtc_lwoff_wait` → set prescaler 32767 → set counter
2. **不做 BKP magic 持久化**（v0.2 加）：每次烧录或上电从 12:00:00 开始（设 counter = 12*3600=43200 秒）
3. main.c 修改：
   - 调 `rtc_user_init()` 在 lv_init 之前
   - tick_timer_cb 周期改 1000ms（每秒更新一次）
   - 显示文本改为 "HH:MM:SS"（format from rtc_get_time），而不是 tick=N
   - 保留进度条作为秒级动态信号
4. CMakeLists.txt 已 GLOB src/bsp/*.c，rtc.c 自动被包含
5. Build → flash → reset → 用户视觉确认时间在走

## 卡点预判

1. **LSE 起振慢**：板载晶振在某些情况下要 1-2 秒才稳定，rcu_osci_stab_wait 默认超时若不够长会 fail。Plan B：用 LSI（内部 ~40kHz，精度差，但 sanity 够）。
2. **BKP 写保护**：必须先 `pmu_backup_write_enable()` 才能写 RTC/BKP 寄存器。漏这步看似过编译但运行时寄存器写入静默失败。
3. **Prescaler 设错**：32767 (32768-1) 给 1Hz；如果用 31999 计数会偏快（多算 ~2.4%）。Datasheet 关键值。
4. **counter 写入需要 lwoff_wait**：写完 prescaler/counter 必须等 RTOFF 标志，否则下条配置覆盖。
5. **多次复位 counter 重置**：因为没接 BKP magic，每次复位 counter 回 12:00:00。这是预期行为，v0.2 加 BKP magic 即可保持。
6. **rtc_register_sync_wait 卡死**：如果 RTC 时钟未真正 enable，sync_wait 会无限等。GDB 看 PC 是否卡在它就知道。

## 验收

- [ ] cmake --build 通过
- [ ] flash + reset → 屏显示 "12:00:0X" 秒数每秒递增
- [ ] 进度条仍正常循环
- [ ] 时间精度肉眼可比对（30 秒内不应明显偏差超 1 秒）
- [ ] smoke 阶段 1+2 PASS
- [ ] post 含 ≥2 lessons 候选
