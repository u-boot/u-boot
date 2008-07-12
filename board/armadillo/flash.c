/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2005 Rowel Atienza <rowel@diwalabs.com>
 * Flash driver for armadillo board HT1070
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

/*value used by hermit is 0x200*/
/*document says sector size is either 64k in low mem reg and 8k in high mem reg*/
#define MAIN_SECT_SIZE  0x10000

#define UNALIGNED_MASK (3)
#define FL_WORD(addr) (*(volatile unsigned short*)(addr))
#define FLASH_TIMEOUT 20000000

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

/*-----------------------------------------------------------------------
 */

ulong flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		flash_info[i].flash_id = (FUJ_MANUFACT & FLASH_VENDMASK);
		/*(INTEL_ID_28F128J3 & FLASH_TYPEMASK); */
		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CFG_MAX_FLASH_SECT;
		memset (flash_info[i].protect, 0, CFG_MAX_FLASH_SECT);
		if (i == 0)
			flashbase = PHYS_FLASH_1;
		else
			panic ("configured too many flash banks!\n");
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
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (FUJ_MANUFACT & FLASH_VENDMASK):
		printf ("Fujitsu: ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}
/*
	switch (info->flash_id & FLASH_TYPEMASK) {
	case (INTEL_ID_28F128J3 & FLASH_TYPEMASK):
		printf ("28F128J3 (128Mbit)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		goto Done;
		break;
	}
*/
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

/*
Done:	;
*/
}

/*
 *  * Loop until both write state machines complete.
 *   */
static unsigned short flash_status_wait (unsigned long addr,
					 unsigned short value)
{
	unsigned short status;
	long timeout = FLASH_TIMEOUT;

	while (((status = (FL_WORD (addr))) != value) && timeout > 0) {
		timeout--;
	}
	return status;
}

/*
 * Loop until the Write State machine is ready, then do a full error
 * check.  Clear status and leave the flash in Read Array mode; return
 * 0 for no error, -1 for error.
 */
static int flash_status_full_check (unsigned long addr, unsigned short value1,
				    unsigned short value2)
{
	unsigned short status1, status2;

	status1 = flash_status_wait (addr, value1);
	status2 = flash_status_wait (addr + 2, value2);
	return (status1 != value1 || status2 != value2) ? -1 : 0;
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int flag, prot, sect;
	int rc = ERR_OK;
	unsigned long base;
	unsigned long addr;

	if ((info->flash_id & FLASH_VENDMASK) !=
	    (FUJ_MANUFACT & FLASH_VENDMASK)) {
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

	printf ("Erasing %d sectors starting at sector %2d.\n"
		"This make take some time ... ",
		s_last - s_first, sect);
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc (); sect++) {
		/* ARM simple, non interrupt dependent timer */
		reset_timer_masked ();

		if (info->protect[sect] == 0) {	/* not protected */

			addr = sect * MAIN_SECT_SIZE;
			addr &= ~(unsigned long) UNALIGNED_MASK;	/* word align */
			base = addr & 0xF0000000;

			FL_WORD (base + (0x555 << 1)) = 0xAA;
			FL_WORD (base + (0x2AA << 1)) = 0x55;
			FL_WORD (base + (0x555 << 1)) = 0x80;
			FL_WORD (base + (0x555 << 1)) = 0xAA;
			FL_WORD (base + (0x2AA << 1)) = 0x55;
			FL_WORD (addr) = 0x30;
			if (flash_status_full_check (addr, 0xFFFF, 0xFFFF))
				return ERR_PROTECTED;
		}
	}
	printf ("\nDone.\n");
	if (ctrlc ())
		printf ("User Interrupt!\n");

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
	int flag;
	unsigned long base;

	/* Check if Flash is (sufficiently) erased
	 */
	if ((FL_WORD (dest) & data) != data)
		return ERR_NOT_ERASED;

	/*if(dest & UNALIGNED_MASK) return ERR_ALIGN; */

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	flag = disable_interrupts ();

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked ();

	base = dest & 0xF0000000;
	FL_WORD (base + (0x555 << 1)) = 0xAA;
	FL_WORD (base + (0x2AA << 1)) = 0x55;
	FL_WORD (base + (0x555 << 1)) = 0xA0;
	FL_WORD (dest) = data;
	/*printf("writing 0x%p = 0x%x\n",dest,data); */
	if (flash_status_wait (dest, data) != data)
		return ERR_PROG_ERROR;

	if (flag)
		enable_interrupts ();

	return ERR_OK;
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
	printf ("Writing %lu short data to 0x%lx from 0x%p.\n ", cnt, wp, src);

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
		printf ("\nDone.\n");
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
