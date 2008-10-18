/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
