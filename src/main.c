#include "gd32f30x.h"
#include <stdio.h>

#include "systick.h"
#include "usart.h"
#include "gpio.h"

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
	systick_config();
	
	gpio_config();
	
	while(1)
	{
		gd_led_toggle(LED_RGB_PORT,LED_R_PIN);
		gd_led_toggle(LED_RGB_PORT,LED_G_PIN);
		gd_led_toggle(LED_RGB_PORT,LED_B_PIN);
		delay(500);
	}
	
}
