/*
 * MOUSSE/MPC8240 Board definitions.
 * Flash Routines for MOUSSE onboard AMD29LV106DB devices
 *
 * (C) Copyright 2000
 * Marius Groeger <mgroeger@sysgo.de>
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 1999, by Curt McDowell, 08-06-99, Broadcom Corp.
 * (C) Copyright 2001, James Dougherty, 07/18/01, Broadcom Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>
#include <malloc.h>
#include "mousse.h"
#include "flash.h"

int flashLibDebug = 0;
int flashLibInited = 0;

#define OK  0
#define ERROR -1
#define STATUS int
#define PRINTF			if (flashLibDebug) printf
#if 0
#define PRIVATE			static
#else
#define PRIVATE
#endif

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

#define SLEEP_DELAY    166
#define FLASH_SECTOR_SIZE   (64*1024)
/***********************************************************************
 *
 * Virtual Flash Devices on Mousse board
 *
 * These must be kept in sync with the definitions in flashLib.h.
 *
 ***********************************************************************/

PRIVATE flash_dev_t flashDev[] = {
    /* Bank 0 sector SA0 (16 kB) */
    {	"SA0",FLASH0_BANK, FLASH0_SEG0_START, 1, 14,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sector SA1 (8 kB) */
    {	"SA1", FLASH0_BANK, FLASH0_SEG0_START + 0x4000, 1, 13,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sector SA2 (8 kB) */
    {	"SA2", FLASH0_BANK, FLASH0_SEG0_START + 0x6000, 1, 13,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sector SA3 is occluded by Mousse I/O devices */
    /* Bank 0 sectors SA4-SA18, after Mousse devices up to PLCC (960 kB)  */
    {	"KERNEL", FLASH0_BANK, FLASH0_SEG1_START, 15, 16,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sectors SA19-SA26, jumper can occlude this by PLCC (512 kB) */
    /* This is where the Kahlua boot vector and boot ROM code resides. */
    {	"BOOT",FLASH0_BANK, FLASH0_SEG2_START, 8, 16,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
    /* Bank 0 sectors SA27-SA34 (512 kB) */
    {	"RAMDISK",FLASH0_BANK, FLASH0_SEG3_START, 8, 16,
	FLASH0_VENDOR_ID, FLASH0_DEVICE_ID
    },
};

int flashDevCount = (sizeof (flashDev) / sizeof (flashDev[0]));

#define DEV(no)			(&flashDev[no])
#define DEV_NO(dev)		((dev) - flashDev)

/***********************************************************************
 *
 * Private Flash Routines
 *
 ***********************************************************************/

/*
 * The convention is:
 *
 * "addr" is always the PROM raw address, which is the address of an
 * 8-bit quantity for flash 0 and 16-bit quantity for flash 1.
 *
 * "pos" is always a logical byte position from the PROM beginning.
 */

#define FLASH0_ADDR(dev, addr) \
	((unsigned char *) ((dev)->base + (addr)))

#define FLASH0_WRITE(dev, addr, value) \
	(*FLASH0_ADDR(dev, addr) = (value))

#define FLASH0_READ(dev, addr) \
	(*FLASH0_ADDR(dev, addr))

PRIVATE int flashCheck (flash_dev_t * dev)
{
	if (!flashLibInited) {
		printf ("flashCheck: flashLib not initialized\n");
		return ERROR;
	}

	if (dev < &flashDev[0] || dev >= &flashDev[flashDevCount]) {
		printf ("flashCheck: Bad dev parameter\n");
		return ERROR;
	}

	if (!dev->found) {
		printf ("flashCheck: Device %d not available\n", DEV_NO (dev));
		return ERROR;
	}

	return OK;
}

PRIVATE void flashReset (flash_dev_t * dev)
{
	PRINTF ("flashReset: dev=%d\n", DEV_NO (dev));

	if (dev->bank == FLASH0_BANK) {
		FLASH0_WRITE (dev, 0x555, 0xaa);
		FLASH0_WRITE (dev, 0xaaa, 0x55);
		FLASH0_WRITE (dev, 0x555, 0xf0);
	}

	udelay (SLEEP_DELAY);

	PRINTF ("flashReset: done\n");
}

PRIVATE int flashProbe (flash_dev_t * dev)
{
	int rv, deviceID, vendorID;

	PRINTF ("flashProbe: dev=%d\n", DEV_NO (dev));

	if (dev->bank != FLASH0_BANK) {
		rv = ERROR;
		goto DONE;
	}

	FLASH0_WRITE (dev, 0xaaa, 0xaa);
	FLASH0_WRITE (dev, 0x555, 0x55);
	FLASH0_WRITE (dev, 0xaaa, 0x90);

	udelay (SLEEP_DELAY);

	vendorID = FLASH0_READ (dev, 0);
	deviceID = FLASH0_READ (dev, 2);

	FLASH0_WRITE (dev, 0, 0xf0);

	PRINTF ("flashProbe: vendor=0x%x device=0x%x\n", vendorID, deviceID);

	if (vendorID == dev->vendorID && deviceID == dev->deviceID)
		rv = OK;
	else
		rv = ERROR;

  DONE:
	PRINTF ("flashProbe: rv=%d\n", rv);

	return rv;
}

PRIVATE int flashWait (flash_dev_t * dev, int addr, int expect, int erase)
{
	int rv = ERROR;
	int i, data;
	int polls;

#if 0
	PRINTF ("flashWait: dev=%d addr=0x%x expect=0x%x erase=%d\n",
		DEV_NO (dev), addr, expect, erase);
#endif

	if (dev->bank != FLASH0_BANK) {
		rv = ERROR;
		goto done;
	}

	if (erase)
		polls = FLASH_ERASE_SECTOR_TIMEOUT;	/* Ticks */
	else
		polls = FLASH_PROGRAM_POLLS;	/* Loops */

	for (i = 0; i < polls; i++) {
		if (erase)
			udelay (SLEEP_DELAY);

		data = FLASH0_READ (dev, addr);

		if (((data ^ expect) & 0x80) == 0) {
			rv = OK;
			goto done;
		}

		if (data & 0x20) {
			/*
			 * If the 0x20 bit has come on, it could actually be because
			 * the operation succeeded, so check the done bit again.
			 */

			data = FLASH0_READ (dev, addr);

			if (((data ^ expect) & 0x80) == 0) {
				rv = OK;
				goto done;
			}

			printf ("flashWait: Program error (dev: %d, addr: 0x%x)\n",
					DEV_NO (dev), addr);

			flashReset (dev);
			rv = ERROR;
			goto done;
		}
	}

	printf ("flashWait: Timeout %s (dev: %d, addr: 0x%x)\n",
		erase ? "erasing sector" : "programming byte",
		DEV_NO (dev), addr);

  done:

#if 0
	PRINTF ("flashWait: rv=%d\n", rv);
#endif

	return rv;
}

/***********************************************************************
 *
 * Public Flash Routines
 *
 ***********************************************************************/

STATUS flashLibInit (void)
{
	int i;

	PRINTF ("flashLibInit: devices=%d\n", flashDevCount);

	for (i = 0; i < flashDevCount; i++) {
		flash_dev_t *dev = &flashDev[i];

		/*
		 * For bank 1, probe both without and with byte swappage,
		 * so that this module works on both old and new Mousse boards.
		 */

		flashReset (dev);

		if (flashProbe (dev) != ERROR)
			dev->found = 1;

		flashReset (dev);

		if (flashProbe (dev) != ERROR)
			dev->found = 1;

		dev->swap = 0;

		if (dev->found) {
			PRINTF ("\n  FLASH %s[%d]: iobase=0x%x - %d sectors %d KB",
				flashDev[i].name, i, flashDev[i].base,
				flashDev[i].sectors,
				(flashDev[i].sectors * FLASH_SECTOR_SIZE) / 1024);

		}
	}

	flashLibInited = 1;

	PRINTF ("flashLibInit: done\n");

	return OK;
}

STATUS flashEraseSector (flash_dev_t * dev, int sector)
{
	int pos, addr;

	PRINTF ("flashErasesector: dev=%d sector=%d\n", DEV_NO (dev), sector);

	if (flashCheck (dev) == ERROR)
		return ERROR;

	if (sector < 0 || sector >= dev->sectors) {
		printf ("flashEraseSector: Sector out of range (dev: %d, sector: %d)\n", DEV_NO (dev), sector);
		return ERROR;
	}

	pos = FLASH_SECTOR_POS (dev, sector);

	if (dev->bank != FLASH0_BANK) {
		return ERROR;
	}

	addr = pos;

	FLASH0_WRITE (dev, 0xaaa, 0xaa);
	FLASH0_WRITE (dev, 0x555, 0x55);
	FLASH0_WRITE (dev, 0xaaa, 0x80);
	FLASH0_WRITE (dev, 0xaaa, 0xaa);
	FLASH0_WRITE (dev, 0x555, 0x55);
	FLASH0_WRITE (dev, addr, 0x30);

	return flashWait (dev, addr, 0xff, 1);
}

/*
 * Note: it takes about as long to flash all sectors together with Chip
 * Erase as it does to flash them one at a time (about 30 seconds for 2
 * MB).  Also since we want to be able to treat subsets of sectors as if
 * they were complete devices, we don't use Chip Erase.
 */

STATUS flashErase (flash_dev_t * dev)
{
	int sector;

	PRINTF ("flashErase: dev=%d sectors=%d\n", DEV_NO (dev), dev->sectors);

	if (flashCheck (dev) == ERROR)
		return ERROR;

	for (sector = 0; sector < dev->sectors; sector++) {
		if (flashEraseSector (dev, sector) == ERROR)
			return ERROR;
	}
	return OK;
}

/*
 * Read and write bytes
 */

STATUS flashRead (flash_dev_t * dev, int pos, char *buf, int len)
{
	int addr, words;

	PRINTF ("flashRead: dev=%d pos=0x%x buf=0x%x len=0x%x\n",
		DEV_NO (dev), pos, (int) buf, len);

	if (flashCheck (dev) == ERROR)
		return ERROR;

	if (pos < 0 || len < 0 || pos + len > FLASH_MAX_POS (dev)) {
		printf ("flashRead: Position out of range "
			"(dev: %d, pos: 0x%x, len: 0x%x)\n",
			DEV_NO (dev), pos, len);
		return ERROR;
	}

	if (len == 0)
		return OK;

	if (dev->bank == FLASH0_BANK) {
		addr = pos;
		words = len;

		PRINTF ("flashRead: memcpy(0x%x, 0x%x, 0x%x)\n",
			(int) buf, (int) FLASH0_ADDR (dev, pos), len);

		memcpy (buf, FLASH0_ADDR (dev, addr), words);

	}
	PRINTF ("flashRead: rv=OK\n");

	return OK;
}

STATUS flashWrite (flash_dev_t * dev, int pos, char *buf, int len)
{
	int addr, words;

	PRINTF ("flashWrite: dev=%d pos=0x%x buf=0x%x len=0x%x\n",
		DEV_NO (dev), pos, (int) buf, len);

	if (flashCheck (dev) == ERROR)
		return ERROR;

	if (pos < 0 || len < 0 || pos + len > FLASH_MAX_POS (dev)) {
		printf ("flashWrite: Position out of range "
			"(dev: %d, pos: 0x%x, len: 0x%x)\n",
			DEV_NO (dev), pos, len);
		return ERROR;
	}

	if (len == 0)
		return OK;

	if (dev->bank == FLASH0_BANK) {
		unsigned char tmp;

		addr = pos;
		words = len;

		while (words--) {
			tmp = *buf;
			if (~FLASH0_READ (dev, addr) & tmp) {
				printf ("flashWrite: Attempt to program 0 to 1 "
					"(dev: %d, addr: 0x%x, data: 0x%x)\n",
					DEV_NO (dev), addr, tmp);
				return ERROR;
			}
			FLASH0_WRITE (dev, 0xaaa, 0xaa);
			FLASH0_WRITE (dev, 0x555, 0x55);
			FLASH0_WRITE (dev, 0xaaa, 0xa0);
			FLASH0_WRITE (dev, addr, tmp);
			if (flashWait (dev, addr, tmp, 0) < 0)
				return ERROR;
			buf++;
			addr++;
		}
	}

	PRINTF ("flashWrite: rv=OK\n");

	return OK;
}

/*
 * flashWritable returns true if a range contains all F's.
 */

STATUS flashWritable (flash_dev_t * dev, int pos, int len)
{
	int addr, words;
	int rv = ERROR;

	PRINTF ("flashWritable: dev=%d pos=0x%x len=0x%x\n",
			DEV_NO (dev), pos, len);

	if (flashCheck (dev) == ERROR)
		goto done;

	if (pos < 0 || len < 0 || pos + len > FLASH_MAX_POS (dev)) {
		printf ("flashWritable: Position out of range "
			"(dev: %d, pos: 0x%x, len: 0x%x)\n",
			DEV_NO (dev), pos, len);
		goto done;
	}

	if (len == 0) {
		rv = 1;
		goto done;
	}

	if (dev->bank == FLASH0_BANK) {
		addr = pos;
		words = len;

		while (words--) {
			if (FLASH0_READ (dev, addr) != 0xff) {
				rv = 0;
				goto done;
			}
			addr++;
		}
	}

	rv = 1;

  done:
	PRINTF ("flashWrite: rv=%d\n", rv);
	return rv;
}


/*
 * NOTE: the below code cannot run from FLASH!!!
 */
/***********************************************************************
 *
 * Flash Diagnostics
 *
 ***********************************************************************/

STATUS flashDiag (flash_dev_t * dev)
{
	unsigned int *buf = 0;
	int i, len, sector;
	int rv = ERROR;

	if (flashCheck (dev) == ERROR)
		return ERROR;

	printf ("flashDiag: Testing device %d, "
		"base: 0x%x, %d sectors @ %d kB = %d kB\n",
		DEV_NO (dev), dev->base,
		dev->sectors,
		1 << (dev->lgSectorSize - 10),
		dev->sectors << (dev->lgSectorSize - 10));

	len = 1 << dev->lgSectorSize;

	printf ("flashDiag: Erasing\n");

	if (flashErase (dev) == ERROR) {
		printf ("flashDiag: Erase failed\n");
		goto done;
	}
	printf ("%d bytes requested ...\n", len);
	buf = malloc (len);
	printf ("allocated %d bytes ...\n", len);
	if (buf == 0) {
		printf ("flashDiag: Out of memory\n");
		goto done;
	}

	/*
	 * Write unique counting pattern to each sector
	 */

	for (sector = 0; sector < dev->sectors; sector++) {
		printf ("flashDiag: Write sector %d\n", sector);

		for (i = 0; i < len / 4; i++)
			buf[i] = sector << 24 | i;

		if (flashWrite (dev,
				sector << dev->lgSectorSize,
				(char *) buf, len) == ERROR) {
			printf ("flashDiag: Write failed (dev: %d, sector: %d)\n",
				DEV_NO (dev), sector);
			goto done;
		}
	}

	/*
	 * Verify
	 */

	for (sector = 0; sector < dev->sectors; sector++) {
		printf ("flashDiag: Verify sector %d\n", sector);

		if (flashRead (dev,
				   sector << dev->lgSectorSize,
				   (char *) buf, len) == ERROR) {
			printf ("flashDiag: Read failed (dev: %d, sector: %d)\n",
				DEV_NO (dev), sector);
			goto done;
		}

		for (i = 0; i < len / 4; i++) {
			if (buf[i] != (sector << 24 | i)) {
				printf ("flashDiag: Verify error "
					"(dev: %d, sector: %d, offset: 0x%x)\n",
					DEV_NO (dev), sector, i);
				printf ("flashDiag: Expected 0x%08x, got 0x%08x\n",
					sector << 24 | i, buf[i]);

				goto done;
			}
		}
	}

	printf ("flashDiag: Erasing\n");

	if (flashErase (dev) == ERROR) {
		printf ("flashDiag: Final erase failed\n");
		goto done;
	}

	rv = OK;

  done:
	if (buf)
		free (buf);

	if (rv == OK)
		printf ("flashDiag: Device %d passed\n", DEV_NO (dev));
	else
		printf ("flashDiag: Device %d failed\n", DEV_NO (dev));

	return rv;
}

STATUS flashDiagAll (void)
{
	int i;
	int rv = OK;

	PRINTF ("flashDiagAll: devices=%d\n", flashDevCount);

	for (i = 0; i < flashDevCount; i++) {
		flash_dev_t *dev = &flashDev[i];

		if (dev->found && flashDiag (dev) == ERROR)
			rv = ERROR;
	}

	if (rv == OK)
		printf ("flashDiagAll: Passed\n");
	else
		printf ("flashDiagAll: Failed because of earlier errors\n");

	return OK;
}


/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	flash_dev_t *dev = NULL;

	flashLibInit ();

	/*
	 * Provide info for FLASH (up to 960K) of Kernel Image data.
	 */
	dev = FLASH_DEV_BANK0_LOW;
	flash_info[FLASH_BANK_KERNEL].flash_id =
			(dev->vendorID << 16) | dev->deviceID;
	flash_info[FLASH_BANK_KERNEL].sector_count = dev->sectors;
	flash_info[FLASH_BANK_KERNEL].size =
			flash_info[FLASH_BANK_KERNEL].sector_count * FLASH_SECTOR_SIZE;
	flash_info[FLASH_BANK_KERNEL].start[FIRST_SECTOR] = dev->base;
	size += flash_info[FLASH_BANK_KERNEL].size;

	/*
	 * Provide info for 512K PLCC FLASH ROM (U-Boot)
	 */
	dev = FLASH_DEV_BANK0_BOOT;
	flash_info[FLASH_BANK_BOOT].flash_id =
			(dev->vendorID << 16) | dev->deviceID;
	flash_info[FLASH_BANK_BOOT].sector_count = dev->sectors;
	flash_info[FLASH_BANK_BOOT].size =
			flash_info[FLASH_BANK_BOOT].sector_count * FLASH_SECTOR_SIZE;
	flash_info[FLASH_BANK_BOOT].start[FIRST_SECTOR] = dev->base;
	size += flash_info[FLASH_BANK_BOOT].size;


	/*
	 * Provide info for 512K FLASH0 segment (U-Boot)
	 */
	dev = FLASH_DEV_BANK0_HIGH;
	flash_info[FLASH_BANK_AUX].flash_id =
			(dev->vendorID << 16) | dev->deviceID;
	flash_info[FLASH_BANK_AUX].sector_count = dev->sectors;
	flash_info[FLASH_BANK_AUX].size =
			flash_info[FLASH_BANK_AUX].sector_count * FLASH_SECTOR_SIZE;
	flash_info[FLASH_BANK_AUX].start[FIRST_SECTOR] = dev->base;
	size += flash_info[FLASH_BANK_AUX].size;


	return size;
}

/*
 * Get flash device from U-Boot flash info.
 */
flash_dev_t *getFlashDevFromInfo (flash_info_t * info)
{
	int i;

	if (!info)
		return NULL;

	for (i = 0; i < flashDevCount; i++) {
		flash_dev_t *dev = &flashDev[i];

		if (dev->found && (dev->base == info->start[0]))
			return dev;
	}
	printf ("ERROR: notice, no FLASH mapped at address 0x%x\n",
			(unsigned int) info->start[0]);
	return NULL;
}

ulong flash_get_size (vu_long * addr, flash_info_t * info)
{
	int i;

	for (i = 0; i < flashDevCount; i++) {
		flash_dev_t *dev = &flashDev[i];

		if (dev->found) {
			if (dev->base == (unsigned int) addr) {
				info->flash_id = (dev->vendorID << 16) | dev->deviceID;
				info->sector_count = dev->sectors;
				info->size = info->sector_count * FLASH_SECTOR_SIZE;
				return dev->sectors * FLASH_SECTOR_SIZE;
			}
		}
	}
	return 0;
}

void flash_print_info (flash_info_t * info)
{
	int i;
	unsigned int chip;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch ((info->flash_id >> 16) & 0xff) {
	case 0x1:
		printf ("AMD ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}
	chip = (unsigned int) info->flash_id & 0x000000ff;

	switch (chip) {

	case AMD_ID_F040B:
		printf ("AM29F040B (4 Mbit)\n");
		break;

	case AMD_ID_LV160B:
	case FLASH_AM160LV:
	case 0x49:
		printf ("AM29LV160B (16 Mbit / 2M x 8bit)\n");
		break;

	default:
		printf ("Unknown Chip Type:0x%x\n", chip);
		break;
	}

	printf ("  Size: %ld bytes in %d Sectors\n",
		info->size, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[FIRST_SECTOR] + i * FLASH_SECTOR_SIZE,
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
}


/*
 * Erase a range of flash sectors.
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int prot, sect;
	flash_dev_t *dev = NULL;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	/* Start erase on unprotected sectors */
	dev = getFlashDevFromInfo (info);
	if (dev) {
		printf ("Erase FLASH[%s] -%d sectors:", dev->name, dev->sectors);
		for (sect = s_first; sect <= s_last; sect++) {
			if (info->protect[sect] == 0) {	/* not protected */
				printf (".");
				if (ERROR == flashEraseSector (dev, sect)) {
					printf ("ERROR: could not erase sector %d on FLASH[%s]\n", sect, dev->name);
					return 1;
				}
			}
		}
	}
	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t * info, ulong dest, ulong data)
{

	flash_dev_t *dev = getFlashDevFromInfo (info);
	int addr = dest - info->start[0];

	if (!dev)
		return 1;

	if (OK != flashWrite (dev, addr, (char *) &data, sizeof (ulong))) {
		printf ("ERROR: could not write to addr=0x%x, data=0x%x\n",
			(unsigned int) addr, (unsigned) data);
		return 1;
	}

	if ((addr % FLASH_SECTOR_SIZE) == 0)
		printf (".");


	PRINTF ("write_word:0x%x, base=0x%x, addr=0x%x, data=0x%x\n",
		(unsigned) info->start[0],
		(unsigned) dest,
		(unsigned) (dest - info->start[0]), (unsigned) data);

	return (0);
}


/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;
	flash_dev_t *dev = getFlashDevFromInfo (info);

	if (dev) {
		printf ("FLASH[%s]:", dev->name);
		wp = (addr & ~3);	/* get lower word aligned address */

		/*
		 * handle unaligned start bytes
		 */
		if ((l = addr - wp) != 0) {
			data = 0;
			for (i = 0, cp = wp; i < l; ++i, ++cp) {
				data = (data << 8) | (*(uchar *) cp);
			}
			for (; i < 4 && cnt > 0; ++i) {
				data = (data << 8) | *src++;
				--cnt;
				++cp;
			}
			for (; cnt == 0 && i < 4; ++i, ++cp) {
				data = (data << 8) | (*(uchar *) cp);
			}
			if ((rc = write_word (info, wp, data)) != 0) {
				return (rc);
			}
			wp += 4;
		}

		/*
		 * handle word aligned part
		 */
		while (cnt >= 4) {
			data = 0;
			for (i = 0; i < 4; ++i) {
				data = (data << 8) | *src++;
			}
			if ((rc = write_word (info, wp, data)) != 0) {
				return (rc);
			}
			wp += 4;
			cnt -= 4;
		}

		if (cnt == 0) {
			return (0);
		}

		/*
		 * handle unaligned tail bytes
		 */
		data = 0;
		for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
			data = (data << 8) | *src++;
			--cnt;
		}
		for (; i < 4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}

		return (write_word (info, wp, data));
	}
	return 1;
}

/*-----------------------------------------------------------------------
 */
