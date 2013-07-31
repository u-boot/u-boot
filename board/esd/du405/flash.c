/*
 * (C) Copyright 2001
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ppc4xx.h>
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
		mtdcr (EBC0_CFGADDR, PB0CR);
		pbcr = mfdcr (EBC0_CFGDATA);
		mtdcr (EBC0_CFGADDR, PB0CR);
		base_b1 = -size_b1;
		pbcr = (pbcr & 0x0001ffff) | base_b1 |
				(((size_b1 / 1024 / 1024) - 1) << 17);
		mtdcr (EBC0_CFGDATA, pbcr);
		/*          printf("PB1CR = %x\n", pbcr); */
	}

	if (size_b0) {
		mtdcr (EBC0_CFGADDR, PB1CR);
		pbcr = mfdcr (EBC0_CFGDATA);
		mtdcr (EBC0_CFGADDR, PB1CR);
		base_b0 = base_b1 - size_b0;
		pbcr = (pbcr & 0x0001ffff) | base_b0 |
				(((size_b0 / 1024 / 1024) - 1) << 17);
		mtdcr (EBC0_CFGDATA, pbcr);
		/*            printf("PB0CR = %x\n", pbcr); */
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
