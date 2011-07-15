/*
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

#define FLASH_BANK_SIZE 0x800000
#define MAIN_SECT_SIZE  0x20000
#define PARAM_SECT_SIZE 0x4000

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];


/*-----------------------------------------------------------------------
 */

ulong flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		flash_info[i].flash_id =
			(INTEL_MANUFACT & FLASH_VENDMASK) |
			(INTEL_ID_28F320B3T & FLASH_TYPEMASK);
		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
		memset (flash_info[i].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);
		if (i == 0)
			flashbase = PHYS_FLASH_1;
		else if (i == 1)
			flashbase = PHYS_FLASH_2;
		else
			panic ("configured too many flash banks!\n");
		for (j = 0; j < flash_info[i].sector_count; j++) {
			if (j <= 7) {
				flash_info[i].start[j] =
					flashbase + j * PARAM_SECT_SIZE;
			} else {
				flash_info[i].start[j] =
					flashbase + (j - 7) * MAIN_SECT_SIZE;
			}
		}
		size += flash_info[i].size;
	}

	/* Protect monitor and environment sectors
	 */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_SYS_FLASH_BASE,
		       CONFIG_SYS_FLASH_BASE + monitor_flash_len - 1,
		       &flash_info[0]);

	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_ENV_ADDR,
		       CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1, &flash_info[0]);

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
	case (INTEL_ID_28F320B3T & FLASH_TYPEMASK):
		printf ("28F320F3B (16Mbit)\n");
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

      Done:;
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int flag, prot, sect;
	int rc = ERR_OK;
	ulong start;

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
		start = get_timer(0);

		if (info->protect[sect] == 0) {	/* not protected */
			vu_long *addr = (vu_long *) (info->start[sect]);

			*addr = 0x00200020;	/* erase setup */
			*addr = 0x00D000D0;	/* erase confirm */

			while ((*addr & 0x00800080) != 0x00800080) {
				if (get_timer(start) >
				    CONFIG_SYS_FLASH_ERASE_TOUT) {
					*addr = 0x00B000B0;	/* suspend erase */
					*addr = 0x00FF00FF;	/* reset to read mode */
					rc = ERR_TIMOUT;
					goto outahere;
				}
			}

			*addr = 0x00FF00FF;	/* reset to read mode */
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

static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long *) dest;
	ulong barf;
	int rc = ERR_OK;
	int flag;
	ulong start;

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
	*addr = 0x00500050;

	/* program set-up command */
	*addr = 0x00400040;

	/* latch address/data */
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	start = get_timer(0);

	/* read status register command */
	*addr = 0x00700070;

	/* wait while polling the status register */
	while ((*addr & 0x00800080) != 0x00800080) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			rc = ERR_TIMOUT;
			/* suspend program command */
			*addr = 0x00B000B0;
			goto outahere;
		}

		if (*addr & 0x003A003A) {	/* check for error */
			barf = *addr;
			if (barf & 0x003A0000) {
				barf >>= 16;
			} else {
				barf &= 0x0000003A;
			}
			printf ("\nFlash write error %02lx at address %08lx\n", barf, (unsigned long) dest);
			if (barf & 0x0002) {
				printf ("Block locked, not erased.\n");
				rc = ERR_NOT_ERASED;
				goto outahere;
			}
			if (barf & 0x0010) {
				printf ("Programming error.\n");
				rc = ERR_PROG_ERROR;
				goto outahere;
			}
			if (barf & 0x0008) {
				printf ("Vpp Low error.\n");
				rc = ERR_PROG_ERROR;
				goto outahere;
			}
			rc = ERR_PROG_ERROR;
			goto outahere;
		}
	}


      outahere:
	/* read array command */
	*addr = 0x00FF00FF;

	if (flag)
		enable_interrupts ();

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

	wp = (addr & ~3);	/* get lower word aligned address */

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
