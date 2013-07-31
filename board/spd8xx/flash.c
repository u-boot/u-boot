/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>

flash_info_t    flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	/* All Speech Design board memory (DRAM and EPROM) initialisation is
	done in dram_init().
	The caller of ths function here expects the total size and will hang,
	if we give here back 0. So we return the EPROM size. */

	return (1024 * 1024); /* 1 MB */
}

/*-----------------------------------------------------------------------
 */

void flash_print_info (flash_info_t *info)
{
	printf("no FLASH memory in MPC823TS board\n");
	return;
}

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	return 1;
}

/*-----------------------------------------------------------------------
 */
