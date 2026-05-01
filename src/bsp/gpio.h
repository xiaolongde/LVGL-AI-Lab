#ifndef gpio_H
#define gpio_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "gd32f30x.h"

#define KEY_RCU  RCU_GPIOC
#define KEY_PORT GPIOC
#define KEY_PIN GPIO_PIN_13

#define LED_RGB_RCU  RCU_GPIOC
#define LED_RGB_PORT GPIOC
#define LED_R_PIN  GPIO_PIN_0
#define LED_G_PIN  GPIO_PIN_1
#define LED_B_PIN  GPIO_PIN_2

void gpio_config(void);
void gd_led_on(uint32_t gpio_periph,uint32_t pin);
void gd_led_off(uint32_t gpio_periph,uint32_t pin);
void gd_led_toggle(uint32_t gpio_periph,uint32_t pin);
#ifdef __cplusplus
}
#endif

#endif
