/*!
    \file    sd_diskio.h
    \brief   FatFs ff_gen_drv glue for KIT sdcard.c (SDIO 4-bit polling/DMA driver)
*/

#ifndef SD_DISKIO_H
#define SD_DISKIO_H

#include "ff_gen_drv.h"

/* Single global driver instance, register via FATFS_LinkDriver(&sd_diskio_drv, path) */
extern const Diskio_drvTypeDef sd_diskio_drv;

#endif /* SD_DISKIO_H */
