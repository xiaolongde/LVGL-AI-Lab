#include "gpio.h"

void gpio_config(void)
{
	/* enable  clock */
	rcu_periph_clock_enable(RCU_AF);

	rcu_periph_clock_enable(KEY_RCU);
	rcu_periph_clock_enable(LED_RGB_RCU);
	
	/* JTAG-DP disabled and SW-DP enabled */
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);
	
	/* configure button pin as input */
  gpio_init(KEY_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY_PIN);
	/* configure led pin as output */
	gpio_init(LED_RGB_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_R_PIN);
	gpio_init(LED_RGB_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_G_PIN);
	gpio_init(LED_RGB_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_B_PIN);	
	gpio_bit_set(LED_RGB_PORT,LED_R_PIN);
	gpio_bit_set(LED_RGB_PORT,LED_G_PIN);
	gpio_bit_set(LED_RGB_PORT,LED_B_PIN);	
}


void gd_led_on(uint32_t gpio_periph,uint32_t pin)
{
	GPIO_BOP(gpio_periph) = (uint32_t)pin;
}

void gd_led_off(uint32_t gpio_periph,uint32_t pin)
{
	GPIO_BC(gpio_periph) = (uint32_t)pin;
}

void gd_led_toggle(uint32_t gpio_periph,uint32_t pin)
{
    gpio_bit_write(gpio_periph, pin, (bit_status)(1-gpio_input_bit_get(gpio_periph, pin)));
}

