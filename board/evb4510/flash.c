/*
 *
 * Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 * Curt Brune <curt@cucy.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/hardware.h>
#include <flash.h>

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

typedef enum {
	FLASH_DEV_U9_512KB = 0,
	FLASH_DEV_U7_2MB = 1
} FLASH_DEV;

#define FLASH_DQ7		(0x80)
#define FLASH_DQ5		(0x20)

#define PROG_ADDR		(0xAAA)
#define SETUP_ADDR		(0xAAA)
#define ID_ADDR			(0xAAA)
#define UNLOCK_ADDR1		(0xAAA)
#define UNLOCK_ADDR2		(0x555)

#define UNLOCK_CMD1		(0xAA)
#define UNLOCK_CMD2		(0x55)
#define ERASE_SUSPEND_CMD	(0xB0)
#define ERASE_RESUME_CMD	(0x30)
#define RESET_CMD		(0xF0)
#define ID_CMD			(0x90)
#define SELECT_CMD		(0x90)
#define CHIPERASE_CMD		(0x10)
#define BYPASS_CMD		(0x20)
#define SECERASE_CMD		(0x30)
#define PROG_CMD		(0xa0)
#define SETUP_CMD		(0x80)

#if 0
#define WRITE_UNLOCK(addr) { \
	PUT__U8( addr + UNLOCK_ADDR1, UNLOCK_CMD1); \
	PUT__U8( addr + UNLOCK_ADDR2, UNLOCK_CMD2); \
}

/* auto select command */
#define CMD_ID(addr) WRITE_UNLOCK(addr); { \
	PUT__U8( addr + ID_ADDR, ID_CMD); \
}

#define CMD_RESET(addr) WRITE_UNLOCK(addr); { \
	PUT__U8( addr + ID_ADDR, RESET_CMD); \
}

#define CMD_ERASE_SEC(base, addr) WRITE_UNLOCK(base); \
	PUT__U8( base + SETUP_ADDR, SETUP_CMD); \
	WRITE_UNLOCK(base); \
	PUT__U8( addr, SECERASE_CMD);

#define CMD_ERASE_CHIP(base) WRITE_UNLOCK(base); \
	PUT__U8( base + SETUP_ADDR, SETUP_CMD); \
	WRITE_UNLOCK(base); \
	PUT__U8( base + SETUP_ADDR, CHIPERASE_CMD);

/* prepare for bypass programming */
#define CMD_UNLOCK_BYPASS(addr) WRITE_UNLOCK(addr); { \
	PUT__U8( addr + ID_ADDR, 0x20); \
}

/* terminate bypass programming */
#define CMD_BYPASS_RESET(addr) { \
	PUT__U8(addr, 0x90); \
	PUT__U8(addr, 0x00); \
}
#endif

inline static void FLASH_CMD_UNLOCK (FLASH_DEV dev, u32 base)
{
	switch (dev) {
	case FLASH_DEV_U7_2MB:
		PUT__U8 (base + 0xAAA, 0xAA);
		PUT__U8 (base + 0x555, 0x55);
		break;
	case FLASH_DEV_U9_512KB:
		PUT__U8 (base + 0x555, 0xAA);
		PUT__U8 (base + 0x2AA, 0x55);
		break;
	}
}

inline static void FLASH_CMD_SELECT (FLASH_DEV dev, u32 base)
{
	switch (dev) {
	case FLASH_DEV_U7_2MB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0xAAA, SELECT_CMD);
		break;
	case FLASH_DEV_U9_512KB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0x555, SELECT_CMD);
		break;
	}
}

inline static void FLASH_CMD_RESET (FLASH_DEV dev, u32 base)
{
	switch (dev) {
	case FLASH_DEV_U7_2MB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0xAAA, RESET_CMD);
		break;
	case FLASH_DEV_U9_512KB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0x555, RESET_CMD);
		break;
	}
}

inline static void FLASH_CMD_ERASE_SEC (FLASH_DEV dev, u32 base, u32 addr)
{
	switch (dev) {
	case FLASH_DEV_U7_2MB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0xAAA, SETUP_CMD);
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (addr, SECERASE_CMD);
		break;
	case FLASH_DEV_U9_512KB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0x555, SETUP_CMD);
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (addr, SECERASE_CMD);
		break;
	}
}

inline static void FLASH_CMD_ERASE_CHIP (FLASH_DEV dev, u32 base)
{
	switch (dev) {
	case FLASH_DEV_U7_2MB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0xAAA, SETUP_CMD);
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base, CHIPERASE_CMD);
		break;
	case FLASH_DEV_U9_512KB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0x555, SETUP_CMD);
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base, CHIPERASE_CMD);
		break;
	}
}

inline static void FLASH_CMD_UNLOCK_BYPASS (FLASH_DEV dev, u32 base)
{
	switch (dev) {
	case FLASH_DEV_U7_2MB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0xAAA, BYPASS_CMD);
		break;
	case FLASH_DEV_U9_512KB:
		FLASH_CMD_UNLOCK (dev, base);
		PUT__U8 (base + 0x555, BYPASS_CMD);
		break;
	}
}

inline static void FLASH_CMD_BYPASS_RESET (FLASH_DEV dev, u32 base)
{
	PUT__U8 (base, SELECT_CMD);
	PUT__U8 (base, 0x0);
}

/* poll for flash command completion */
static u16 _flash_poll (FLASH_DEV dev, u32 addr, u16 data, ulong timeOut)
{
	u32 done = 0;
	ulong t0;

	u16 error = 0;
	volatile u16 flashData;

	data = data & 0xFF;
	t0 = get_timer (0);
	while (get_timer (t0) < timeOut) {
		/*	for( i = 0; i < POLL_LOOPS; i++) { */
		/*  Read the Data */
		flashData = GET__U8 (addr);

		/*  FLASH_DQ7 = Data? */
		if ((flashData & FLASH_DQ7) == (data & FLASH_DQ7)) {
			done = 1;
			break;
		}

		/*  Check Timeout (FLASH_DQ5==1) */
		if (flashData & FLASH_DQ5) {
			/*  Read the Data */
			flashData = GET__U8 (addr);

			/*  FLASH_DQ7 = Data? */
			if (!((flashData & FLASH_DQ7) == (data & FLASH_DQ7))) {
				printf ("_flash_poll(): FLASH_DQ7 & flashData not equal to write value\n");
				error = ERR_PROG_ERROR;
			}
			FLASH_CMD_RESET (dev, addr);
			done = 1;
			break;
		}
		/*  spin delay */
		udelay (10);
	}


	/*  error update */
	if (!done) {
		printf ("_flash_poll(): Timeout\n");
		error = ERR_TIMOUT;
	}

	/*  Check the data */
	if (!error) {
		/*  Read the Data */
		flashData = GET__U8 (addr);
		if (flashData != data) {
			error = ERR_PROG_ERROR;
			printf ("_flash_poll(): flashData(0x%04x) not equal to data(0x%04x)\n",
				flashData, data);
		}
	}

	return error;
}

/*-----------------------------------------------------------------------
 */
static int _flash_check_protection (flash_info_t * info, int s_first, int s_last)
{
	int sect, prot = 0;

	for (sect = s_first; sect <= s_last; sect++)
		if (info->protect[sect]) {
			printf ("  Flash sector %d protected.\n", sect);
			prot++;
		}
	return prot;
}

static int _detectFlash (FLASH_DEV dev, u32 base, u8 venId, u8 devId)
{

	u32 baseAddr = base | CACHE_DISABLE_MASK;
	u8 vendorId, deviceId;

	/*	printf(__FUNCTION__"(): detecting flash @ 0x%08x\n", base); */

	/* Send auto select command and read manufacturer info */
	FLASH_CMD_SELECT (dev, baseAddr);
	vendorId = GET__U8 (baseAddr);
	FLASH_CMD_RESET (dev, baseAddr);

	/* Send auto select command and read device info */
	FLASH_CMD_SELECT (dev, baseAddr);

	if (dev == FLASH_DEV_U7_2MB) {
		deviceId = GET__U8 (baseAddr + 2);
	} else if (dev == FLASH_DEV_U9_512KB) {
		deviceId = GET__U8 (baseAddr + 1);
	} else {
		return 0;
	}

	FLASH_CMD_RESET (dev, baseAddr);

	/* printf (__FUNCTION__"(): found vendorId 0x%04x, deviceId 0x%04x\n",
		vendorId, deviceId);
	 */

	return (vendorId == venId) && (deviceId == devId);

}

/******************************************************************************
 *
 * Public u-boot interface functions below
 *
 *****************************************************************************/

/***************************************************************************
 *
 * Flash initialization
 *
 * This board has two banks of flash, but the base addresses depend on
 * how the board is jumpered.
 *
 * The two flash types are:
 *
 *   AMD Am29LV160DB (2MB) sectors layout 16KB, 2x8KB, 32KB, 31x64KB
 *
 *   AMD Am29LV040B  (512KB)  sectors: 8x64KB
 *****************************************************************************/

unsigned long flash_init (void)
{
	flash_info_t *info;
	u16 i;
	u32 flashtest;
	s16 amd160 = -1;
	u32 amd160base = 0;

#if CFG_MAX_FLASH_BANKS == 2
	s16 amd040 = -1;
	u32 amd040base = 0;
#endif

	/* configure PHYS_FLASH_1 */
	if (_detectFlash (FLASH_DEV_U7_2MB, PHYS_FLASH_1, 0x1, 0x49)) {
		amd160 = 0;
		amd160base = PHYS_FLASH_1;
#if CFG_MAX_FLASH_BANKS == 1
	}
#else
		if (_detectFlash
		    (FLASH_DEV_U9_512KB, PHYS_FLASH_2, 0x1, 0x4F)) {
			amd040 = 1;
			amd040base = PHYS_FLASH_2;
		} else {
			printf (__FUNCTION__
				"(): Unable to detect PHYS_FLASH_2: 0x%08x\n",
				PHYS_FLASH_2);
		}
	} else if (_detectFlash (FLASH_DEV_U9_512KB, PHYS_FLASH_1, 0x1, 0x4F)) {
		amd040 = 0;
		amd040base = PHYS_FLASH_1;
		if (_detectFlash (FLASH_DEV_U7_2MB, PHYS_FLASH_2, 0x1, 0x49)) {
			amd160 = 1;
			amd160base = PHYS_FLASH_2;
		} else {
			printf (__FUNCTION__
				"(): Unable to detect PHYS_FLASH_2: 0x%08x\n",
				PHYS_FLASH_2);
		}
	}
#endif
	else {
		printf ("flash_init(): Unable to detect PHYS_FLASH_1: 0x%08x\n",
			PHYS_FLASH_1);
	}

	/* Configure AMD Am29LV160DB (2MB) */
	info = &flash_info[amd160];
	info->flash_id = FLASH_DEV_U7_2MB;
	info->sector_count = 35;
	info->size = 2 * 1024 * 1024;	/* 2MB */
	/* 1*16K Boot Block
	   2*8K Parameter Block
	   1*32K Small Main Block */
	info->start[0] = amd160base;
	info->start[1] = amd160base + 0x4000;
	info->start[2] = amd160base + 0x6000;
	info->start[3] = amd160base + 0x8000;
	for (i = 1; i < info->sector_count; i++)
		info->start[3 + i] = amd160base + i * (64 * 1024);

	for (i = 0; i < info->sector_count; i++) {
		/* Write auto select command sequence and query sector protection */
		FLASH_CMD_SELECT (info->flash_id,
				  info->start[i] | CACHE_DISABLE_MASK);
		flashtest =
			GET__U8 (((info->start[i] + 4) | CACHE_DISABLE_MASK));
		FLASH_CMD_RESET (info->flash_id,
				 amd160base | CACHE_DISABLE_MASK);
		info->protect[i] = (flashtest & 0x0001);
	}

	/*
	 * protect monitor and environment sectors in 2MB flash
	 */
	flash_protect (FLAG_PROTECT_SET,
		       amd160base, amd160base + monitor_flash_len - 1, info);

	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR, CFG_ENV_ADDR + CFG_ENV_SIZE - 1, info);

#if CFG_MAX_FLASH_BANKS == 2
	/* Configure AMD Am29LV040B (512KB) */
	info = &flash_info[amd040];
	info->flash_id = FLASH_DEV_U9_512KB;
	info->sector_count = 8;
	info->size = 512 * 1024;	/* 512KB, 8 x 64KB */
	for (i = 0; i < info->sector_count; i++) {
		info->start[i] = amd040base + i * (64 * 1024);
		/* Write auto select command sequence and query sector protection */
		FLASH_CMD_SELECT (info->flash_id,
				  info->start[i] | CACHE_DISABLE_MASK);
		flashtest =
			GET__U8 (((info->start[i] + 2) | CACHE_DISABLE_MASK));
		FLASH_CMD_RESET (info->flash_id,
				 amd040base | CACHE_DISABLE_MASK);
		info->protect[i] = (flashtest & 0x0001);
	}
#endif

	return flash_info[0].size
#if CFG_MAX_FLASH_BANKS == 2
		+ flash_info[1].size
#endif
		;
}

void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_DEV_U7_2MB) {
		printf ("AMD Am29LV160DB (2MB) 16KB,2x8KB,32KB,31x64KB\n");
	} else if (info->flash_id == FLASH_DEV_U9_512KB) {
		printf ("AMD Am29LV040B	 (512KB) 8x64KB\n");
	} else {
		printf ("Unknown flash_id ...\n");
		return;
	}

	printf ("  Size: %ld KB in %d Sectors\n",
		info->size >> 10, info->sector_count);
	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 4) == 0)
			printf ("\n   ");
		printf (" S%02d @ 0x%08lX%s", i,
			info->start[i], info->protect[i] ? " !" : "  ");
	}
	printf ("\n");
}

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	u16 i, error = 0;

	printf ("\n");

	/* check flash protection bits */
	if (_flash_check_protection (info, s_first, s_last)) {
		printf ("  Flash erase aborted due to protected sectors\n");
		return ERR_PROTECTED;
	}

	if ((s_first < info->sector_count) && (s_first <= s_last)) {
		for (i = s_first; i <= s_last && !error; i++) {
			printf ("  Erasing Sector %d @ 0x%08lx ... ", i,
				info->start[i]);
			/* bypass the cache to access the flash memory */
			FLASH_CMD_ERASE_SEC (info->flash_id,
					     (info->
					      start[0] | CACHE_DISABLE_MASK),
					     (info->
					      start[i] | CACHE_DISABLE_MASK));
			/* look for sector to become 0xFF after erase */
			error = _flash_poll (info->flash_id,
					     info->
					     start[i] | CACHE_DISABLE_MASK,
					     0xFF, CFG_FLASH_ERASE_TOUT);
			FLASH_CMD_RESET (info->flash_id,
					 (info->
					  start[0] | CACHE_DISABLE_MASK));
			printf ("done\n");
			if (error) {
				break;
			}
		}
	} else
		error = ERR_INVAL;

	return error;
}

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	u16 error = 0, i;
	u32 n;
	u8 *bp, *bps;

	/*  Write Setup */
	/* bypass the cache to access the flash memory */
	FLASH_CMD_UNLOCK_BYPASS (info->flash_id,
				 (info->start[0] | CACHE_DISABLE_MASK));

	/*  Write the Data to Flash */

	bp = (u8 *) (addr | CACHE_DISABLE_MASK);
	bps = (u8 *) src;

	for (n = 0; n < cnt && !error; n++, bp++, bps++) {

		if (!(n % (cnt / 15))) {
			printf (".");
		}

		/*  write the flash command for flash memory */
		*bp = 0xA0;

		/*  Write the data */
		*bp = *bps;

		/*  Check if the write is done */
		for (i = 0; i < 0xff; i++);
		error = _flash_poll (info->flash_id, (u32) bp, *bps,
				     CFG_FLASH_WRITE_TOUT);
		if (error) {
			return error;
		}
	}

	/*  Reset the Flash Mode to read */
	FLASH_CMD_BYPASS_RESET (info->flash_id, info->start[0]);

	printf (" ");

	return error;
}
