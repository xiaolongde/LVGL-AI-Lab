/* rtc.c — GD32F303 RTC bring-up using vendor lib gd32f30x_rtc.h
 *
 * RTC 模型: 32-bit free-running counter @ 1 Hz (RCC LSE 32768 / prescaler 32768)
 * 时钟源: LSE (32.768 kHz 板载晶振 XT2)
 * 默认起点: 12:00:00 (counter = 43200)，复位重置（v0.2 加 BKP magic 持久化）
 */

#include "rtc.h"
#include "gd32f30x.h"

#define DEFAULT_HOUR    12u
#define DEFAULT_MINUTE  0u
#define DEFAULT_SECOND  0u
#define SECONDS_PER_DAY 86400u

void rtc_user_init(void)
{
    /* 1. 启用 PMU + BKP 时钟，解锁 backup domain (含 RTC 寄存器) */
    rcu_periph_clock_enable(RCU_PMU);
    rcu_periph_clock_enable(RCU_BKPI);
    pmu_backup_write_enable();

    /* 2. backup reset 一次（清掉之前可能残留的 RTC 配置） */
    rcu_bkp_reset_enable();
    rcu_bkp_reset_disable();

    /* 3. 启 LSE 32.768 kHz (XT2) */
    rcu_osci_on(RCU_LXTAL);
    /* 等 LSE 稳定（板上晶振典型 < 1s；但 vendor 等待是 polling，可能略慢） */
    while (rcu_osci_stab_wait(RCU_LXTAL) == ERROR) {
        /* 若一直失败可降级 LSI (RCU_IRC40K)，本任务只用 LSE */
    }

    /* 4. RTCCLK 选 LSE */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
    rcu_periph_clock_enable(RCU_RTC);

    /* 5. RTC 寄存器同步 + 等空闲 */
    rtc_register_sync_wait();
    rtc_lwoff_wait();

    /* 6. 进配置模式 → 设 prescaler 32767 (32768Hz / (32767+1) = 1Hz) → 设默认 counter → 退 */
    rtc_configuration_mode_enter();
    rtc_prescaler_set(32767u);
    rtc_lwoff_wait();
    uint32_t default_count = (uint32_t)DEFAULT_HOUR * 3600u
                           + (uint32_t)DEFAULT_MINUTE * 60u
                           + (uint32_t)DEFAULT_SECOND;
    rtc_counter_set(default_count);
    rtc_lwoff_wait();
    rtc_configuration_mode_exit();
}

void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s)
{
    uint32_t cnt = rtc_counter_get() % SECONDS_PER_DAY;
    *h = (uint8_t)(cnt / 3600u);
    *m = (uint8_t)((cnt % 3600u) / 60u);
    *s = (uint8_t)(cnt % 60u);
}
