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
#ifdef __DEBUG_START_FROM_SRAM__
	return CONFIG_SYS_DUMMY_FLASH_SIZE;
#else
	unsigned long size;
	int i;
	uint pbcr;
	unsigned long base;
	int size_val = 0;

	debug("[%s, %d] Entering ...\n", __FUNCTION__, __LINE__);
	debug("[%s, %d] flash_info = 0x%p ...\n", __func__, __LINE__,
						flash_info);

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	debug("[%s, %d] Calling flash_get_size ...\n", __FUNCTION__, __LINE__);
	size = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size, size<<20);
	}

	debug("[%s, %d] Test point ...\n", __FUNCTION__, __LINE__);

	/* Setup offsets */
	flash_get_offsets (-size, &flash_info[0]);
	debug("[%s, %d] Test point ...\n", __FUNCTION__, __LINE__);

	/* Re-do sizing to get full correct info */
	mtdcr(EBC0_CFGADDR, PB0CR);
	pbcr = mfdcr(EBC0_CFGDATA);
	mtdcr(EBC0_CFGADDR, PB0CR);
	base = -size;
	switch (size) {
	case 1 << 20:
		size_val = 0;
		break;
	case 2 << 20:
		size_val = 1;
		break;
	case 4 << 20:
		size_val = 2;
		break;
	case 8 << 20:
		size_val = 3;
		break;
	case 16 << 20:
		size_val = 4;
		break;
	}
	pbcr = (pbcr & 0x0001ffff) | base | (size_val << 17);
	mtdcr(EBC0_CFGDATA, pbcr);
	debug("[%s, %d] Test point ...\n", __FUNCTION__, __LINE__);

	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CONFIG_SYS_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[0]);

	debug("[%s, %d] Test point ...\n", __FUNCTION__, __LINE__);
	flash_info[0].size  = size;

	return (size);
#endif
}
