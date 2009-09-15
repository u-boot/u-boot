/*
 * (C) Copyright 2001-2003
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
#include <ppc4xx.h>
#include <asm/processor.h>

/*
 * include common flash code (for esd boards)
 */
#include "../common/flash.c"

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long * addr, flash_info_t * info);
static void flash_get_offsets (ulong base, flash_info_t * info);

/*-----------------------------------------------------------------------
 */

unsigned long calc_size(unsigned long size)
{
	switch (size) {
	case 1 << 20:
		return 0;
	case 2 << 20:
		return 1;
	case 4 << 20:
		return 2;
	case 8 << 20:
		return 3;
	case 16 << 20:
		return 4;
	default:
		return 0;
	}
}


unsigned long flash_init (void)
{
	unsigned long size_b0, size_b1;
	int i;
	uint pbcr;
	unsigned long base_b0, base_b1;

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	base_b0 = FLASH_BASE0_PRELIM;
	size_b0 = flash_get_size ((vu_long *) base_b0, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
				size_b0, size_b0 << 20);
	}

	base_b1 = FLASH_BASE1_PRELIM;
	size_b1 = flash_get_size ((vu_long *) base_b1, &flash_info[1]);

	/* Re-do sizing to get full correct info */

	if (size_b1) {
		if (size_b1 < (1 << 20)) {
			/* minimum CS size on PPC405GP is 1MB !!! */
			size_b1 = 1 << 20;
		}
		base_b1 = -size_b1;
		mtdcr (EBC0_CFGADDR, PB0CR);
		pbcr = mfdcr (EBC0_CFGDATA);
		mtdcr (EBC0_CFGADDR, PB0CR);
		pbcr = (pbcr & 0x0001ffff) | base_b1 | (calc_size(size_b1) << 17);
		mtdcr (EBC0_CFGDATA, pbcr);
#if 0 /* test-only */
		printf("size_b1=%x base_b1=%x PB1CR = %x\n",
		       size_b1, base_b1, pbcr); /* test-only */
#endif
	}

	if (size_b0) {
		if (size_b0 < (1 << 20)) {
			/* minimum CS size on PPC405GP is 1MB !!! */
			size_b0 = 1 << 20;
		}
		base_b0 = base_b1 - size_b0;
		mtdcr (EBC0_CFGADDR, PB1CR);
		pbcr = mfdcr (EBC0_CFGDATA);
		mtdcr (EBC0_CFGADDR, PB1CR);
		pbcr = (pbcr & 0x0001ffff) | base_b0 | (calc_size(size_b0) << 17);
		mtdcr (EBC0_CFGDATA, pbcr);
#if 0 /* test-only */
		printf("size_b0=%x base_b0=%x PB0CR = %x\n",
		       size_b0, base_b0, pbcr); /* test-only */
#endif
	}

	size_b0 = flash_get_size ((vu_long *) base_b0, &flash_info[0]);

	flash_get_offsets (base_b0, &flash_info[0]);

	/* monitor protection ON by default */
	flash_protect (FLAG_PROTECT_SET,
			base_b0 + size_b0 - monitor_flash_len,
			base_b0 + size_b0 - 1, &flash_info[0]);

	if (size_b1) {
		/* Re-do sizing to get full correct info */
		size_b1 = flash_get_size ((vu_long *) base_b1, &flash_info[1]);

		flash_get_offsets (base_b1, &flash_info[1]);

		/* monitor protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
				base_b1 + size_b1 - monitor_flash_len,
				base_b1 + size_b1 - 1, &flash_info[1]);
		/* monitor protection OFF by default (one is enough) */
		flash_protect (FLAG_PROTECT_CLEAR,
				base_b0 + size_b0 - monitor_flash_len,
				base_b0 + size_b0 - 1, &flash_info[0]);
	} else {
		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[1].sector_count = -1;
	}

	flash_info[0].size = size_b0;
	flash_info[1].size = size_b1;

	return (size_b0 + size_b1);
}
