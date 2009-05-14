/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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

/* #define DEBUG */

#include <common.h>
#include <environment.h>

#define FLASH_BANK_SIZE 0x1000000	/* 16MB (2 x 8 MB) */
#define MAIN_SECT_SIZE  0x40000		/* 256KB (2 x 128kB) */

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];


#define CMD_READ_ARRAY		0x00FF00FF
#define CMD_IDENTIFY		0x00900090
#define CMD_ERASE_SETUP		0x00200020
#define CMD_ERASE_CONFIRM	0x00D000D0
#define CMD_PROGRAM		0x00400040
#define CMD_RESUME		0x00D000D0
#define CMD_SUSPEND		0x00B000B0
#define CMD_STATUS_READ		0x00700070
#define CMD_STATUS_RESET	0x00500050

#define BIT_BUSY		0x00800080
#define BIT_ERASE_SUSPEND	0x00400040
#define BIT_ERASE_ERROR		0x00200020
#define BIT_PROGRAM_ERROR	0x00100010
#define BIT_VPP_RANGE_ERROR	0x00080008
#define BIT_PROGRAM_SUSPEND	0x00040004
#define BIT_PROTECT_ERROR	0x00020002
#define BIT_UNDEFINED		0x00010001

#define BIT_SEQUENCE_ERROR	0x00300030
#define BIT_TIMEOUT		0x80000000

/*-----------------------------------------------------------------------
 */

ulong flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		flash_info[i].flash_id =
			(INTEL_MANUFACT     & FLASH_VENDMASK) |
			(INTEL_ID_28F640J3A & FLASH_TYPEMASK);
		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
		memset (flash_info[i].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);
		if (i == 0)
			flashbase = CONFIG_SYS_FLASH_BASE;
		else
			panic ("configured too many flash banks!\n");
		for (j = 0; j < flash_info[i].sector_count; j++) {
			flash_info[i].start[j] = flashbase;

			/* uniform sector size */
			flashbase += MAIN_SECT_SIZE;
		}
		size += flash_info[i].size;
	}

	/*
	 * Protect monitor and environment sectors
	 */
	flash_protect ( FLAG_PROTECT_SET,
			CONFIG_SYS_FLASH_BASE,
			CONFIG_SYS_FLASH_BASE + monitor_flash_len - 1,
			&flash_info[0]);

	flash_protect ( FLAG_PROTECT_SET,
			CONFIG_ENV_ADDR,
			CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1, &flash_info[0]);

#ifdef CONFIG_ENV_ADDR_REDUND
	flash_protect ( FLAG_PROTECT_SET,
			CONFIG_ENV_ADDR_REDUND,
			CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
			&flash_info[0]);
#endif

	return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (INTEL_MANUFACT & FLASH_VENDMASK):
		printf ("Intel: ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case (INTEL_ID_28F640J3A & FLASH_TYPEMASK):
		printf ("2x 28F640J3A (64Mbit)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		return;
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
}

/*-----------------------------------------------------------------------
 */

int flash_error (ulong code)
{
	/* Check bit patterns */
	/* SR.7=0 is busy, SR.7=1 is ready */
	/* all other flags indicate error on 1 */
	/* SR.0 is undefined */
	/* Timeout is our faked flag */

	/* sequence is described in Intel 290644-005 document */

	/* check Timeout */
	if (code & BIT_TIMEOUT) {
		puts ("Timeout\n");
		return ERR_TIMOUT;
	}

	/* check Busy, SR.7 */
	if (~code & BIT_BUSY) {
		puts ("Busy\n");
		return ERR_PROG_ERROR;
	}

	/* check Vpp low, SR.3 */
	if (code & BIT_VPP_RANGE_ERROR) {
		puts ("Vpp range error\n");
		return ERR_PROG_ERROR;
	}

	/* check Device Protect Error, SR.1 */
	if (code & BIT_PROTECT_ERROR) {
		puts ("Device protect error\n");
		return ERR_PROG_ERROR;
	}

	/* check Command Seq Error, SR.4 & SR.5 */
	if (code & BIT_SEQUENCE_ERROR) {
		puts ("Command seqence error\n");
		return ERR_PROG_ERROR;
	}

	/* check Block Erase Error, SR.5 */
	if (code & BIT_ERASE_ERROR) {
		puts ("Block erase error\n");
		return ERR_PROG_ERROR;
	}

	/* check Program Error, SR.4 */
	if (code & BIT_PROGRAM_ERROR) {
		puts ("Program error\n");
		return ERR_PROG_ERROR;
	}

	/* check Block Erase Suspended, SR.6 */
	if (code & BIT_ERASE_SUSPEND) {
		puts ("Block erase suspended\n");
		return ERR_PROG_ERROR;
	}

	/* check Program Suspended, SR.2 */
	if (code & BIT_PROGRAM_SUSPEND) {
		puts ("Program suspended\n");
		return ERR_PROG_ERROR;
	}

	/* OK, no error */
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	ulong result, result1;
	int iflag, prot, sect;
	int rc = ERR_OK;

#ifdef USE_920T_MMU
	int cflag;
#endif

	debug ("flash_erase: s_first %d  s_last %d\n", s_first, s_last);

	/* first look for protection bits */

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last)) {
		return ERR_INVAL;
	}

	if ((info->flash_id & FLASH_VENDMASK) !=
		(INTEL_MANUFACT & FLASH_VENDMASK)) {
		return ERR_UNKNOWN_FLASH_VENDOR;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
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

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
#ifdef USE_920T_MMU
	cflag = dcache_status ();
	dcache_disable ();
#endif
	iflag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc (); sect++) {

		debug ("Erasing sector %2d @ %08lX... ",
			sect, info->start[sect]);

		/* arm simple, non interrupt dependent timer */
		reset_timer_masked();

		if (info->protect[sect] == 0) {	/* not protected */
			vu_long *addr = (vu_long *) (info->start[sect]);
			ulong bsR7, bsR7_2, bsR5, bsR5_2;

			/* *addr = CMD_STATUS_RESET; */
			*addr = CMD_ERASE_SETUP;
			*addr = CMD_ERASE_CONFIRM;

			/* wait until flash is ready */
			do {
				/* check timeout */
				if (get_timer_masked () > CONFIG_SYS_FLASH_ERASE_TOUT) {
					*addr = CMD_STATUS_RESET;
					result = BIT_TIMEOUT;
					break;
				}

				*addr = CMD_STATUS_READ;
				result = *addr;
				bsR7 = result & (1 << 7);
				bsR7_2 = result & (1 << 23);
			} while (!bsR7 | !bsR7_2);

			*addr = CMD_STATUS_READ;
			result1 = *addr;
			bsR5 = result1 & (1 << 5);
			bsR5_2 = result1 & (1 << 21);
#ifdef SAMSUNG_FLASH_DEBUG
			printf ("bsR5 %lx bsR5_2 %lx\n", bsR5, bsR5_2);
			if (bsR5 != 0 && bsR5_2 != 0)
				printf ("bsR5 %lx bsR5_2 %lx\n", bsR5, bsR5_2);
#endif

			*addr = CMD_READ_ARRAY;
			*addr = CMD_RESUME;

			if ((rc = flash_error (result)) != ERR_OK)
				goto outahere;
#if 0
			printf ("ok.\n");
		} else {		/* it was protected */

			printf ("protected!\n");
#endif
		}
	}

outahere:
	/* allow flash to settle - wait 10 ms */
	udelay_masked (10000);

	if (iflag)
		enable_interrupts ();

#ifdef USE_920T_MMU
	if (cflag)
		dcache_enable ();
#endif
	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash
 */

static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long *) dest;
	ulong result;
	int rc = ERR_OK;
	int iflag;

#ifdef USE_920T_MMU
	int cflag;
#endif

	/*
	 * Check if Flash is (sufficiently) erased
	 */
	result = *addr;
	if ((result & data) != data)
		return ERR_NOT_ERASED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
#ifdef USE_920T_MMU
	cflag = dcache_status ();
	dcache_disable ();
#endif
	iflag = disable_interrupts ();

	/* *addr = CMD_STATUS_RESET; */
	*addr = CMD_PROGRAM;
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked ();

	/* wait until flash is ready */
	do {
		/* check timeout */
		if (get_timer_masked () > CONFIG_SYS_FLASH_ERASE_TOUT) {
			*addr = CMD_SUSPEND;
			result = BIT_TIMEOUT;
			break;
		}

		*addr = CMD_STATUS_READ;
		result = *addr;
	} while (~result & BIT_BUSY);

	/* *addr = CMD_READ_ARRAY; */
	*addr = CMD_STATUS_READ;
	result = *addr;

	rc = flash_error (result);

	if (iflag)
		enable_interrupts ();

#ifdef USE_920T_MMU
	if (cflag)
		dcache_enable ();
#endif
	*addr = CMD_READ_ARRAY;
	*addr = CMD_RESUME;
	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int l;
	int i, rc;

	wp = (addr & ~3);			/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 24);
		}
		for (; i < 4 && cnt > 0; ++i) {
			data = (data >> 8) | (*src++ << 24);
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 4; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 24);
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
		data = *((vu_long *) src);
		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		src += 4;
		wp += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		return ERR_OK;
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
		data = (data >> 8) | (*src++ << 24);
		--cnt;
	}
	for (; i < 4; ++i, ++cp) {
		data = (data >> 8) | (*(uchar *) cp << 24);
	}

	return write_word (info, wp, data);
}
