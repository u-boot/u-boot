/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

/*#if 0 */
#if (PHYS_SDRAM_1_SIZE != SZ_32M)

#include "crcek.h"

#if (CFG_MAX_FLASH_BANKS > 1)
#error There is always only _one_ flash chip
#endif

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

#define CMD_READ_ARRAY		0x000000f0
#define CMD_UNLOCK1		0x000000aa
#define CMD_UNLOCK2		0x00000055
#define CMD_ERASE_SETUP		0x00000080
#define CMD_ERASE_CONFIRM	0x00000030
#define CMD_PROGRAM		0x000000a0
#define CMD_UNLOCK_BYPASS	0x00000020

#define MEM_FLASH_ADDR1		(*(volatile u16 *)(CFG_FLASH_BASE + (0x00000555 << 1)))
#define MEM_FLASH_ADDR2		(*(volatile u16 *)(CFG_FLASH_BASE + (0x000002aa << 1)))

#define BIT_ERASE_DONE		0x00000080
#define BIT_RDY_MASK		0x00000080
#define BIT_PROGRAM_ERROR	0x00000020
#define BIT_TIMEOUT		0x80000000	/* our flag */

/*-----------------------------------------------------------------------
 */

ulong flash_init(void)
{
	int i;

	flash_info[0].flash_id = (AMD_MANUFACT & FLASH_VENDMASK) |
				 (AMD_ID_LV800B & FLASH_TYPEMASK);
	flash_info[0].size = PHYS_FLASH_1_SIZE;
	flash_info[0].sector_count = CFG_MAX_FLASH_SECT;
	memset(flash_info[0].protect, 0, CFG_MAX_FLASH_SECT);

	for (i = 0; i < flash_info[0].sector_count; i++) {
		switch (i) {
		case 0: /* 16kB */
			flash_info[0].start[0] = CFG_FLASH_BASE;
			break;
		case 1: /* 8kB */
			flash_info[0].start[1] = CFG_FLASH_BASE + 0x4000;
			break;
		case 2: /* 8kB */
			flash_info[0].start[2] = CFG_FLASH_BASE + 0x4000 +
						 0x2000;
			break;
		case 3: /* 32 KB */
			flash_info[0].start[3] = CFG_FLASH_BASE + 0x4000 +
						 2 * 0x2000;
			break;
		case 4:
			flash_info[0].start[4] = CFG_FLASH_BASE + 0x4000 +
						 2 * 0x2000 + 0x8000;
			break;
		default: /* 64kB */
			flash_info[0].start[i] = flash_info[0].start[i-1] +
						 0x10000;
			break;
		}
	}

	/* U-Boot */
	flash_protect(FLAG_PROTECT_SET,
		      LOADER1_OFFSET,
		      LOADER1_OFFSET + LOADER_SIZE - 1, flash_info);
	/* Protect crcek, env and r_env as well */
	flash_protect(FLAG_PROTECT_SET, 0, 0x8000 - 1, flash_info);

	return flash_info[0].size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info(flash_info_t *info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (AMD_MANUFACT & FLASH_VENDMASK):
		puts("AMD: ");
		break;
	default:
		puts("Unknown vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case (AMD_ID_LV800B & FLASH_TYPEMASK):
		puts("AM29LV800BB (8Mb)\n");
		break;
	default:
		puts("Unknown chip type\n");
		return;
	}

	printf("  Size: %ld MB in %d sectors\n",
	       info->size >> 20, info->sector_count);

	puts("  Sector start addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0)
			puts("\n   ");

		printf(" %08lX%s", info->start[i],
		       info->protect[i] ? " (RO)" : "     ");
	}
	puts("\n");
}

/*-----------------------------------------------------------------------
 */

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	ushort result;
	int prot, sect;
	int rc = ERR_OK;

	/* first look for protection bits */

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last))
		return ERR_INVAL;

	if ((info->flash_id & FLASH_VENDMASK) !=
	    (AMD_MANUFACT & FLASH_VENDMASK))
		return ERR_UNKNOWN_FLASH_VENDOR;

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect)
		if (info->protect[sect])
			prot++;

	if (prot)
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	else
		putc('\n');

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc (); sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_short *addr = (vu_short *) (info->start[sect]);

			/* arm simple, non interrupt dependent timer */
			reset_timer_masked();

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			MEM_FLASH_ADDR1 = CMD_ERASE_SETUP;

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			*addr = CMD_ERASE_CONFIRM;

			/* wait until flash is ready */
			while (1) {
				result = *addr;

				/* check timeout */
				if (get_timer_masked() > CFG_FLASH_ERASE_TOUT) {
					MEM_FLASH_ADDR1 = CMD_READ_ARRAY;
					rc = ERR_TIMOUT;
					break;
				}

				if ((result & 0xfff) & BIT_ERASE_DONE)
					break;

				if ((result & 0xffff) & BIT_PROGRAM_ERROR) {
					rc = ERR_PROG_ERROR;
					break;
				}
			}

			MEM_FLASH_ADDR1 = CMD_READ_ARRAY;

			if (rc != ERR_OK)
				goto out;

			putc('.');
		}
	}
out:
	/* allow flash to settle - wait 10 ms */
	udelay_masked(10000);

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash
 */

static int write_hword(flash_info_t *info, ulong dest, ushort data)
{
	vu_short *addr = (vu_short *) dest;
	ushort result;
	int rc = ERR_OK;

	/* check if flash is (sufficiently) erased */
	result = *addr;
	if ((result & data) != data)
		return ERR_NOT_ERASED;

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_PROGRAM;
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked();

	/* wait until flash is ready */
	while (1) {
		result = *addr;

		/* check timeout */
		if (get_timer_masked () > CFG_FLASH_ERASE_TOUT) {
			rc = ERR_TIMOUT;
			break;
		}

		if ((result & 0x80) == (data & 0x80))
			break;

		if ((result & 0xffff) & BIT_PROGRAM_ERROR) {
			result = *addr;

			if ((result & 0x80) != (data & 0x80))
				rc = ERR_PROG_ERROR;
		}
	}

	*addr = CMD_READ_ARRAY;

	if (*addr != data)
		rc = ERR_PROG_ERROR;

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp;
	int l;
	int i, rc;
	ushort data;

	wp = (addr & ~1);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp)
			data = (data >> 8) | (*(uchar *) cp << 8);
		for (; i < 2 && cnt > 0; ++i) {
			data = (data >> 8) | (*src++ << 8);
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 2; ++i, ++cp)
			data = (data >> 8) | (*(uchar *) cp << 8);

		if ((rc = write_hword(info, wp, data)) != 0)
			return (rc);
		wp += 2;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 2) {
		data = *((vu_short *) src);
		if ((rc = write_hword(info, wp, data)) != 0)
			return (rc);
		src += 2;
		wp += 2;
		cnt -= 2;
	}

	if (cnt == 0)
		return ERR_OK;

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 2 && cnt > 0; ++i, ++cp) {
		data = (data >> 8) | (*src++ << 8);
		--cnt;
	}
	for (; i < 2; ++i, ++cp)
		data = (data >> 8) | (*(uchar *) cp << 8);

	return write_hword(info, wp, data);
}

#endif
