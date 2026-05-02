/* rtc.h — GD32F303 RTC (LSE 32.768kHz, 32-bit counter, F1 风格) */

#ifndef RTC_H
#define RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** 初始化 RTC：启用 LSE、配 prescaler=32767 (1Hz)、counter 设默认 12:00:00。
 *  本函数无 BKP magic 持久化—— 每次复位时间归零（v0.2 加持久化）。 */
void rtc_user_init(void);

/** 读当前时间（24h 制，counter 取模 86400 秒一天）。
 *  @param h, m, s 输出指针，0..23 / 0..59 / 0..59 */
void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s);

#ifdef __cplusplus
}
#endif

#endif
