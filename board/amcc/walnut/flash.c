/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Modified 4/5/2001
 * Wait for completion of each sector erase command issued
 * 4/5/2001
 * Chris Hallinan - DS4.COM, Inc. - clh@net1plus.com
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/processor.h>

#undef DEBUG
#ifdef DEBUG
#define DEBUGF(x...) printf(x)
#else
#define DEBUGF(x...)
#endif				/* DEBUG */

/*
 * include common flash code (for amcc boards)
 */
#include "../common/flash.c"

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(vu_long * addr, flash_info_t * info);
static void flash_get_offsets(ulong base, flash_info_t * info);

unsigned long flash_init(void)
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

	size_b0 =
	    flash_get_size((vu_long *) FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
		       size_b0, size_b0 << 20);
	}

	/* Only one bank */
	if (CONFIG_SYS_MAX_FLASH_BANKS == 1) {
		/* Setup offsets */
		flash_get_offsets(FLASH_BASE0_PRELIM, &flash_info[0]);

		/* Monitor protection ON by default */
		(void)flash_protect(FLAG_PROTECT_SET,
				    CONFIG_SYS_MONITOR_BASE,
				    CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN - 1,
				    &flash_info[0]);
#ifdef CONFIG_ENV_IS_IN_FLASH
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR,
				    CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
				    &flash_info[0]);
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR_REDUND,
				    CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
				    &flash_info[0]);
#endif

		size_b1 = 0;
		flash_info[0].size = size_b0;
	} else {
		/* 2 banks */
		size_b1 =
		    flash_get_size((vu_long *) FLASH_BASE1_PRELIM,
				   &flash_info[1]);

		/* Re-do sizing to get full correct info */

		if (size_b1) {
			mtdcr(EBC0_CFGADDR, PB0CR);
			pbcr = mfdcr(EBC0_CFGDATA);
			mtdcr(EBC0_CFGADDR, PB0CR);
			base_b1 = -size_b1;
			pbcr =
			    (pbcr & 0x0001ffff) | base_b1 |
			    (((size_b1 / 1024 / 1024) - 1) << 17);
			mtdcr(EBC0_CFGDATA, pbcr);
			/*          printf("PB1CR = %x\n", pbcr); */
		}

		if (size_b0) {
			mtdcr(EBC0_CFGADDR, PB1CR);
			pbcr = mfdcr(EBC0_CFGDATA);
			mtdcr(EBC0_CFGADDR, PB1CR);
			base_b0 = base_b1 - size_b0;
			pbcr =
			    (pbcr & 0x0001ffff) | base_b0 |
			    (((size_b0 / 1024 / 1024) - 1) << 17);
			mtdcr(EBC0_CFGDATA, pbcr);
			/*            printf("PB0CR = %x\n", pbcr); */
		}

		size_b0 = flash_get_size((vu_long *) base_b0, &flash_info[0]);

		flash_get_offsets(base_b0, &flash_info[0]);

		/* monitor protection ON by default */
		(void)flash_protect(FLAG_PROTECT_SET,
				    base_b0 + size_b0 - monitor_flash_len,
				    base_b0 + size_b0 - 1, &flash_info[0]);

		if (size_b1) {
			/* Re-do sizing to get full correct info */
			size_b1 =
			    flash_get_size((vu_long *) base_b1, &flash_info[1]);

			flash_get_offsets(base_b1, &flash_info[1]);

			/* monitor protection ON by default */
			(void)flash_protect(FLAG_PROTECT_SET,
					    base_b1 + size_b1 -
					    monitor_flash_len,
					    base_b1 + size_b1 - 1,
					    &flash_info[1]);
			/* monitor protection OFF by default (one is enough) */
			(void)flash_protect(FLAG_PROTECT_CLEAR,
					    base_b0 + size_b0 -
					    monitor_flash_len,
					    base_b0 + size_b0 - 1,
					    &flash_info[0]);
		} else {
			flash_info[1].flash_id = FLASH_UNKNOWN;
			flash_info[1].sector_count = -1;
		}

		flash_info[0].size = size_b0;
		flash_info[1].size = size_b1;
	}			/* else 2 banks */
	return (size_b0 + size_b1);
}


static void flash_get_offsets(ulong base, flash_info_t * info)
{
	int i;

	/* set up sector start address table */
	if (((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST) ||
	    (info->flash_id == FLASH_AM040)) {
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00010000);
	} else {
		if (info->flash_id & FLASH_BTYPE) {
			/* set sector offsets for bottom boot block type        */
			info->start[0] = base + 0x00000000;
			info->start[1] = base + 0x00004000;
			info->start[2] = base + 0x00006000;
			info->start[3] = base + 0x00008000;
			for (i = 4; i < info->sector_count; i++) {
				info->start[i] =
				    base + (i * 0x00010000) - 0x00030000;
			}
		} else {
			/* set sector offsets for top boot block type           */
			i = info->sector_count - 1;
			info->start[i--] = base + info->size - 0x00004000;
			info->start[i--] = base + info->size - 0x00006000;
			info->start[i--] = base + info->size - 0x00008000;
			for (; i >= 0; i--) {
				info->start[i] = base + i * 0x00010000;
			}
		}
	}
}
