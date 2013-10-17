/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002 Jun Gu <jung@artesyncp.com>
 * Add support for Am29F016D and dynamic switch setting.
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

static unsigned long flash_addr_table[1][CONFIG_SYS_MAX_FLASH_BANKS] = {
	{0xff900000, 0xff980000, 0xffc00000},	/* 0:000: configuraton 3 */
};

/*
 * include common flash code (for amcc boards)
 */
#include "../common/flash.c"

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(vu_long * addr, flash_info_t * info);

unsigned long flash_init(void)
{
	unsigned long total_b = 0;
	unsigned long size_b[CONFIG_SYS_MAX_FLASH_BANKS];
	unsigned short index = 0;
	int i;

	/* read FPGA base register FPGA_REG0 */

	DEBUGF("\n");
	DEBUGF("FLASH: Index: %d\n", index);

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].sector_count = -1;
		flash_info[i].size = 0;

		/* check whether the address is 0 */
		if (flash_addr_table[index][i] == 0) {
			continue;
		}

		/* call flash_get_size() to initialize sector address */
		size_b[i] = flash_get_size((vu_long *)
					   flash_addr_table[index][i],
					   &flash_info[i]);
		flash_info[i].size = size_b[i];
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
			       i, size_b[i], size_b[i] << 20);
			flash_info[i].sector_count = -1;
			flash_info[i].size = 0;
		}

		/* Monitor protection ON by default */
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_SYS_MONITOR_BASE,
				    CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN - 1,
				    &flash_info[2]);
#ifdef CONFIG_ENV_IS_IN_FLASH
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR,
				    CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
				    &flash_info[2]);
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR_REDUND,
				    CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
				    &flash_info[2]);
#endif

		total_b += flash_info[i].size;
	}

	return total_b;
}
