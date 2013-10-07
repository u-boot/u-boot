/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Adapted for Interphase 4539 by Wolfgang Grandegger <wg@denx.de>.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <flash.h>
#include <asm/io.h>

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

extern int hwc_flash_size(void);
static ulong flash_get_size (u32 addr, flash_info_t *info);
static int flash_get_offsets (u32 base, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_reset (u32 addr);

#define out8(a,v) *(volatile unsigned char*)(a) = v
#define in8(a)	  *(volatile unsigned char*)(a)
#define in32(a)	  *(volatile unsigned long*)(a)
#define iobarrier_rw() eieio()

unsigned long flash_init (void)
{
	unsigned int i;
	unsigned long flash_size = 0;
	unsigned long bank_size;
	unsigned int bank = 0;

	/* Init: no FLASHes known */
	for (i=0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].sector_count = 0;
		flash_info[i].size = 0;
	}

	/* Initialise the BOOT Flash */
	if (bank == CONFIG_SYS_MAX_FLASH_BANKS) {
		puts ("Warning: not all Flashes are initialised !");
		return flash_size;
	}

	bank_size = flash_get_size (CONFIG_SYS_FLASH_BASE, flash_info + bank);
	if (bank_size) {
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE && \
    CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MAX_FLASH_SIZE
		/* monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_SYS_MONITOR_BASE,
			      CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
			      flash_info + bank);
#endif

#ifdef CONFIG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_ENV_ADDR,
			      CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
			      flash_info + bank);
#endif

		/* HWC protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_SYS_FLASH_BASE,
			      CONFIG_SYS_FLASH_BASE + 0x10000 - 1,
			      flash_info + bank);

		flash_size += bank_size;
		bank++;
	} else {
		puts ("Warning: the BOOT Flash is not initialised !");
	}

	return flash_size;
}

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (u32 addr, flash_info_t *info)
{
	volatile uchar value;
#if 0
	int i;
#endif

	/* Write auto select command: read Manufacturer ID */
	out8(addr + 0x0555, 0xAA);
	iobarrier_rw();
	udelay(10);
	out8(addr + 0x02AA, 0x55);
	iobarrier_rw();
	udelay(10);
	out8(addr + 0x0555, 0x90);
	iobarrier_rw();
	udelay(10);

	value = in8(addr);
	iobarrier_rw();
	udelay(10);
	switch (value | (value << 16)) {
	case AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;

	case FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		flash_reset (addr);
		return 0;
	}

	value = in8(addr + 1);			/* device ID		*/
	iobarrier_rw();

	switch (value) {
	case AMD_ID_LV033C:
		info->flash_id += FLASH_AM033C;
		info->size = hwc_flash_size();
		if (info->size > CONFIG_SYS_MAX_FLASH_SIZE) {
			printf("U-Boot supports only %d MB\n",
			       CONFIG_SYS_MAX_FLASH_SIZE);
			info->size = CONFIG_SYS_MAX_FLASH_SIZE;
		}
		info->sector_count = info->size / 0x10000;
		break;				/* => 4 MB		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		flash_reset (addr);
		return (0);			/* => no or unknown flash */

	}

	if (!flash_get_offsets (addr, info)) {
		flash_reset (addr);
		return 0;
	}

#if 0
	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		value = in8(info->start[i] + 2);
		iobarrier_rw();
		info->protect[i] = (value & 1) != 0;
	}
#endif

	/*
	 * Reset bank to read mode
	 */
	flash_reset (addr);

	return (info->size);
}

static int flash_get_offsets (u32 base, flash_info_t *info)
{
	unsigned int i, size;

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM033C:
		/* set sector offsets for uniform sector type	*/
		size = info->size / info->sector_count;
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + i * size;
		}
		break;
	default:
		return 0;
	}

	return 1;
}

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	volatile u32 addr = info->start[0];
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	if (s_first < 0 || s_first > s_last) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if (info->flash_id == FLASH_UNKNOWN ||
	    info->flash_id > FLASH_AMD_COMP) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
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

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	out8(addr + 0x555, 0xAA);
	iobarrier_rw();
	out8(addr + 0x2AA, 0x55);
	iobarrier_rw();
	out8(addr + 0x555, 0x80);
	iobarrier_rw();
	out8(addr + 0x555, 0xAA);
	iobarrier_rw();
	out8(addr + 0x2AA, 0x55);
	iobarrier_rw();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = info->start[sect];
			out8(addr, 0x30);
			iobarrier_rw();
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer (0);
	last  = start;
	addr = info->start[l_sect];
	while ((in8(addr) & 0x80) != 0x80) {
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc ('.');
			last = now;
		}
		iobarrier_rw();
	}

DONE:
	/* reset to read mode */
	flash_reset (info->start[0]);

	printf (" done\n");
	return 0;
}

/*
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<4 && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i=0; i<4; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<4; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_word(info, wp, data));
}

/*
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	volatile u32 addr = info->start[0];
	ulong start;
	int flag, i;

	/* Check if Flash is (sufficiently) erased */
	if ((in32(dest) & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* first, perform an unlock bypass command to speed up flash writes */
	out8(addr + 0x555, 0xAA);
	iobarrier_rw();
	out8(addr + 0x2AA, 0x55);
	iobarrier_rw();
	out8(addr + 0x555, 0x20);
	iobarrier_rw();

	/* write each byte out */
	for (i = 0; i < 4; i++) {
		char *data_ch = (char *)&data;
		out8(addr, 0xA0);
		iobarrier_rw();
		out8(dest+i, data_ch[i]);
		iobarrier_rw();
		udelay(10); /* XXX */
	}

	/* we're done, now do an unlock bypass reset */
	out8(addr, 0x90);
	iobarrier_rw();
	out8(addr, 0x00);
	iobarrier_rw();

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	while ((in32(dest) & 0x80808080) != (data & 0x80808080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			return (1);
		}
		iobarrier_rw();
	}

	flash_reset (addr);

	return (0);
}

/*
 * Reset bank to read mode
 */
static void flash_reset (u32 addr)
{
	out8(addr, 0xF0);	/* reset bank */
	iobarrier_rw();
}

void flash_print_info (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_BM:	printf ("BRIGHT MICRO ");	break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM033C:	printf ("AM29LV033C (32 Mbit, uniform sectors)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	if (info->size % 0x100000 == 0) {
		printf ("  Size: %ld MB in %d Sectors\n",
			info->size / 0x100000, info->sector_count);
	}
	else if (info->size % 0x400 == 0) {
		printf ("  Size: %ld KB in %d Sectors\n",
			info->size / 0x400, info->sector_count);
	}
	else {
		printf ("  Size: %ld B in %d Sectors\n",
			info->size, info->sector_count);
	}

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	printf ("\n");
}
