#ifndef	FLASH_LIB_H
#define	FLASH_LIB_H

#include <common.h>

/* PIO operations max */
#define	FLASH_PROGRAM_POLLS		100000

/* 10 Seconds default */
#define	FLASH_ERASE_SECTOR_TIMEOUT	(10*1000 /*SEC*/ )

/* Flash device info structure */
typedef struct flash_dev_s {
    char        name[24];               /* Bank Name */
    int		bank;			/* Bank 0 or 1 */
    unsigned int base;			/* Base address */
    int		sectors;		/* Sector count */
    int		lgSectorSize;		/* Log2(usable bytes/sector) */
    int		vendorID;		/* Expected vendor ID */
    int		deviceID;		/* Expected device ID */
    int		found;			/* Set if found by flashLibInit */
    int		swap;			/* Set for bank 1 if byte swap req'd */
} flash_dev_t;

#define FLASH_MAX_POS(dev) \
	((dev)->sectors << (dev)->lgSectorSize)

#define FLASH_SECTOR_POS(dev, sector) \
	((sector) << (dev)->lgSectorSize)

/* AMD 29F040 */
#define FLASH0_BANK			0
#define FLASH0_VENDOR_ID		0x01
#define FLASH0_DEVICE_ID		0x49

/* AMD29LV160DB */
#define FLASH1_BANK			1
#define FLASH1_VENDOR_ID		0x0001
#define FLASH1_DEVICE_ID		0x2249

extern	flash_dev_t			flashDev[];
extern	int				flashDevCount;

/*
 * Device pointers
 *
 * These must be kept in sync with the table in flashLib.c.
 */
#define FLASH_DEV_BANK0_SA0		(&flashDev[0])
#define FLASH_DEV_BANK0_SA1		(&flashDev[1])
#define FLASH_DEV_BANK0_SA2		(&flashDev[2])
#define FLASH_DEV_BANK0_LOW		(&flashDev[3]) /* 960K */
#define FLASH_DEV_BANK0_BOOT		(&flashDev[4]) /* PLCC */
#define FLASH_DEV_BANK0_HIGH		(&flashDev[5]) /* 512K PLCC shadow */

unsigned long flash_init(void);
int flashEraseSector(flash_dev_t *dev, int sector);
int flashErase(flash_dev_t *dev);
int flashRead(flash_dev_t *dev, int pos, char *buf, int len);
int flashWrite(flash_dev_t *dev, int pos, char *buf, int len);
int flashWritable(flash_dev_t *dev, int pos, int len);
int flashDiag(flash_dev_t *dev);
int flashDiagAll(void);

ulong flash_get_size (vu_long *addr, flash_info_t *info);
void flash_print_info  (flash_info_t *info);
int  flash_erase (flash_info_t *info, int s_first, int s_last);
int  write_buff  (flash_info_t *info, uchar *src, ulong addr, ulong cnt);

/*
 * Flash info indices.
 */
#define FLASH_BANK_KERNEL 0
#define FLASH_BANK_BOOT   1
#define FLASH_BANK_AUX    2
#define FIRST_SECTOR      0

#endif	/* !FLASH_LIB_H */
