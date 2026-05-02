#ifndef spi_H
#define spi_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "gd32f30x.h"

void spi_lcd_init(void);
void SPI2_WriteBytes(uint8_t *pbuffer, uint32_t length);
#ifdef __cplusplus
}
#endif

#endif

