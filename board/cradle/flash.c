/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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

#define FLASH_BANK_SIZE 0x400000
#define MAIN_SECT_SIZE  0x20000

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];


/*-----------------------------------------------------------------------
 */

ulong flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		flash_info[i].flash_id =
			(INTEL_MANUFACT & FLASH_VENDMASK) |
			(INTEL_ID_28F128J3 & FLASH_TYPEMASK);
		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CFG_MAX_FLASH_SECT;
		memset (flash_info[i].protect, 0, CFG_MAX_FLASH_SECT);
		switch (i) {
		case 0:
			flashbase = PHYS_FLASH_1;
			break;
		case 1:
			flashbase = PHYS_FLASH_2;
			break;
		default:
			panic ("configured too many flash banks!\n");
			break;
		}
		for (j = 0; j < flash_info[i].sector_count; j++) {
			flash_info[i].start[j] =
				flashbase + j * MAIN_SECT_SIZE;
		}
		size += flash_info[i].size;
	}

	/* Protect monitor and environment sectors
	 */
	flash_protect (FLAG_PROTECT_SET,
		       CFG_FLASH_BASE,
		       CFG_FLASH_BASE + monitor_flash_len - 1,
		       &flash_info[0]);

	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR,
		       CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[0]);

	return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i, j;

	for (j = 0; j < CFG_MAX_FLASH_BANKS; j++) {
		switch (info->flash_id & FLASH_VENDMASK) {
		case (INTEL_MANUFACT & FLASH_VENDMASK):
			printf ("Intel: ");
			break;
		default:
			printf ("Unknown Vendor ");
			break;
		}

		switch (info->flash_id & FLASH_TYPEMASK) {
		case (INTEL_ID_28F320J3A & FLASH_TYPEMASK):
			printf ("28F320J3A (32Mbit)\n");
			break;
		case (INTEL_ID_28F128J3 & FLASH_TYPEMASK):
			printf ("28F128J3 (128Mbit)\n");
			break;
		default:
			printf ("Unknown Chip Type\n");
			goto Done;
			break;
		}

		printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

		printf ("  Sector Start Addresses:");
		for (i = 0; i < info->sector_count; i++) {
			if ((i % 5) == 0) {
				printf ("\n   ");
			}
			printf (" %08lX%s", info->start[i],
				info->protect[i] ? " (RO)" : "     ");
		}
		printf ("\n");
		info++;
	}

Done:	;
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int flag, prot, sect;
	int rc = ERR_OK;

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
	if (prot)
		return ERR_PROTECTED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	flag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc (); sect++) {

		printf ("Erasing sector %2d ... ", sect);

		/* arm simple, non interrupt dependent timer */
		reset_timer_masked ();

		if (info->protect[sect] == 0) {	/* not protected */
			vu_short *addr = (vu_short *) (info->start[sect]);

			*addr = 0x20;	/* erase setup */
			*addr = 0xD0;	/* erase confirm */

			while ((*addr & 0x80) != 0x80) {
				if (get_timer_masked () >
				    CFG_FLASH_ERASE_TOUT) {
					*addr = 0xB0;	/* suspend erase */
					*addr = 0xFF;	/* reset to read mode */
					rc = ERR_TIMOUT;
					goto outahere;
				}
			}

			/* clear status register command */
			*addr = 0x50;
			/* reset to read mode */
			*addr = 0xFF;
		}
		printf ("ok.\n");
	}
	if (ctrlc ())
		printf ("User Interrupt!\n");

outahere:

	/* allow flash to settle - wait 10 ms */
	udelay_masked (10000);

	if (flag)
		enable_interrupts ();

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash
 */

static int write_word (flash_info_t * info, ulong dest, ushort data)
{
	vu_short *addr = (vu_short *) dest, val;
	int rc = ERR_OK;
	int flag;

	/* Check if Flash is (sufficiently) erased
	 */
	if ((*addr & data) != data)
		return ERR_NOT_ERASED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	flag = disable_interrupts ();

	/* clear status register command */
	*addr = 0x50;

	/* program set-up command */
	*addr = 0x40;

	/* latch address/data */
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked ();

	/* wait while polling the status register */
	while (((val = *addr) & 0x80) != 0x80) {
		if (get_timer_masked () > CFG_FLASH_WRITE_TOUT) {
			rc = ERR_TIMOUT;
			/* suspend program command */
			*addr = 0xB0;
			goto outahere;
		}
	}

	if (val & 0x1A) {	/* check for error */
		printf ("\nFlash write error %02x at address %08lx\n",
			(int) val, (unsigned long) dest);
		if (val & (1 << 3)) {
			printf ("Voltage range error.\n");
			rc = ERR_PROG_ERROR;
			goto outahere;
		}
		if (val & (1 << 1)) {
			printf ("Device protect error.\n");
			rc = ERR_PROTECTED;
			goto outahere;
		}
		if (val & (1 << 4)) {
			printf ("Programming error.\n");
			rc = ERR_PROG_ERROR;
			goto outahere;
		}
		rc = ERR_PROG_ERROR;
		goto outahere;
	}

outahere:
	/* read array command */
	*addr = 0xFF;

	if (flag)
		enable_interrupts ();

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp;
	ushort data;
	int l;
	int i, rc;

	wp = (addr & ~1);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 8);
		}
		for (; i < 2 && cnt > 0; ++i) {
			data = (data >> 8) | (*src++ << 8);
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 2; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 8);
		}

		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		wp += 2;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 2) {
		data = *((vu_short *) src);
		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		src += 2;
		wp += 2;
		cnt -= 2;
	}

	if (cnt == 0) {
		return ERR_OK;
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 2 && cnt > 0; ++i, ++cp) {
		data = (data >> 8) | (*src++ << 8);
		--cnt;
	}
	for (; i < 2; ++i, ++cp) {
		data = (data >> 8) | (*(uchar *) cp << 8);
	}

	return write_word (info, wp, data);
}
