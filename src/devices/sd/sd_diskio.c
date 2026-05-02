/*!
    \file    sd_diskio.c
    \brief   FatFs glue: bridges Diskio_drvTypeDef → KIT sdcard.c (sd_block_*)

    sdcard.c uses DMA with 32-bit memory width → buffers MUST be 4-byte aligned.
    FatFs may pass unaligned BYTE*; we bounce via an aligned static scratch in that case.
    addr passed to sd_block_*() is in BYTES (KIT examples use sector*512), driver
    internally divides by 512 for SDHC. We follow the same convention.
*/

#include "sd_diskio.h"
#include "sdcard.h"
#include <string.h>

#define SD_BLOCK_SIZE  512U

static uint32_t s_scratch[SD_BLOCK_SIZE / 4]; /* one-sector aligned bounce buffer */

static DSTATUS sd_disk_initialize(BYTE lun)
{
    (void)lun;
    /* sd_init() was already invoked at boot; if FatFs re-asks, just reaffirm OK */
    return 0;
}

static DSTATUS sd_disk_status(BYTE lun)
{
    (void)lun;
    return 0;
}

static DRESULT sd_disk_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
    (void)lun;
    sd_error_enum err;
    uint32_t addr = sector * SD_BLOCK_SIZE;

    if (((uintptr_t)buff & 0x3U) == 0U) {
        /* aligned: hand straight to driver */
        if (count == 1U) {
            err = sd_block_read((uint32_t *)buff, addr, SD_BLOCK_SIZE);
        } else {
            err = sd_multiblocks_read((uint32_t *)buff, addr, SD_BLOCK_SIZE, count);
        }
    } else {
        /* unaligned: bounce sector by sector */
        for (UINT i = 0; i < count; i++) {
            err = sd_block_read(s_scratch, addr + i * SD_BLOCK_SIZE, SD_BLOCK_SIZE);
            if (err != SD_OK) {
                return RES_ERROR;
            }
            memcpy(buff + i * SD_BLOCK_SIZE, s_scratch, SD_BLOCK_SIZE);
        }
        return RES_OK;
    }

    return (err == SD_OK) ? RES_OK : RES_ERROR;
}

#if _USE_WRITE == 1
static DRESULT sd_disk_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
    (void)lun;
    sd_error_enum err;
    uint32_t addr = sector * SD_BLOCK_SIZE;

    if (((uintptr_t)buff & 0x3U) == 0U) {
        if (count == 1U) {
            err = sd_block_write((uint32_t *)(uintptr_t)buff, addr, SD_BLOCK_SIZE);
        } else {
            err = sd_multiblocks_write((uint32_t *)(uintptr_t)buff, addr, SD_BLOCK_SIZE, count);
        }
    } else {
        for (UINT i = 0; i < count; i++) {
            memcpy(s_scratch, buff + i * SD_BLOCK_SIZE, SD_BLOCK_SIZE);
            err = sd_block_write(s_scratch, addr + i * SD_BLOCK_SIZE, SD_BLOCK_SIZE);
            if (err != SD_OK) {
                return RES_ERROR;
            }
        }
        return RES_OK;
    }

    return (err == SD_OK) ? RES_OK : RES_ERROR;
}
#endif

#if _USE_IOCTL == 1
static DRESULT sd_disk_ioctl(BYTE lun, BYTE cmd, void *buff)
{
    (void)lun;
    DRESULT res = RES_ERROR;

    switch (cmd) {
    case CTRL_SYNC:
        /* DMA sd_block_* are blocking; nothing to flush */
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT: {
        uint32_t cap_kb = sd_card_capacity_get();
        *(DWORD *)buff = (DWORD)((uint64_t)cap_kb * 1024U / SD_BLOCK_SIZE);
        res = RES_OK;
        break;
    }

    case GET_SECTOR_SIZE:
        *(WORD *)buff = SD_BLOCK_SIZE;
        res = RES_OK;
        break;

    case GET_BLOCK_SIZE:
        /* Erase block size in sectors. Unknown without parsing CSD allocation_unit;
           safe default 1 (no perf impact for small writes). */
        *(DWORD *)buff = 1;
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
        break;
    }

    return res;
}
#endif

const Diskio_drvTypeDef sd_diskio_drv = {
    sd_disk_initialize,
    sd_disk_status,
    sd_disk_read,
#if _USE_WRITE == 1
    sd_disk_write,
#endif
#if _USE_IOCTL == 1
    sd_disk_ioctl,
#endif
};
