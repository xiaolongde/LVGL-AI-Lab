#include "spi.h"
#include "systick.h"
/*!
    \brief      initialize SPI2 GPIO and parameter
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_lcd_init(void)
{
    spi_parameter_struct spi_init_struct;
		
		rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI2);
	
	
    /* SPI0_SCK(PB3)and SPI0_MOSI(PB5) GPIO pin configuration */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);
    /* 1 GPIO pin configuration */
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);
		gpio_bit_set(GPIOB,GPIO_PIN_4);
		gpio_bit_set(GPIOB,GPIO_PIN_6);
		gpio_bit_reset(GPIOB,GPIO_PIN_7);	

    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
		gpio_bit_reset(GPIOA,GPIO_PIN_15);	
	
    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_8;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI2, &spi_init_struct);

    /* enable SPI1 */
    spi_enable(SPI2);
}

/**
  * @brief          封装SPI2写函数
  * @param[in]     	TxData 发出的数据
  * @retval         RxData 收到的数据
  */
void SPI2_WriteBytes(uint8_t *pbuffer, uint32_t length)
{
	
  while(length--)
  {	
    /* loop while data register in not emplty */
    while (RESET == spi_i2s_flag_get(SPI2,SPI_FLAG_TBE));

    /* send byte through the SPI0 peripheral */
    spi_i2s_data_transmit(SPI2, *pbuffer);
	
		while (SET == spi_i2s_flag_get(SPI2,SPI_STAT_TRANS));
//		delay_us(100);
		
		pbuffer++;
	}
}


