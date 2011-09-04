/*
 * Copyright (C) 2011 Samsung Electronics
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

#include<common.h>
#include<config.h>

typedef u32(*copy_sd_mmc_to_mem) \
	(u32 start_block, u32 block_count, u32 *dest_addr);


void copy_uboot_to_ram(void)
{
	copy_sd_mmc_to_mem copy_bl2 = (copy_sd_mmc_to_mem)(0x00002488);
	copy_bl2(BL2_START_OFFSET,\
		BL2_SIZE_BLOC_COUNT, (u32 *)CONFIG_SYS_TEXT_BASE);
}

void board_init_f(unsigned long bootflag)
{
	__attribute__((noreturn)) void (*uboot)(void);
	copy_uboot_to_ram();

	/* Jump to U-Boot image */
	uboot = (void *)CONFIG_SYS_TEXT_BASE;
	(*uboot)();
	/* Never returns Here */
}

/* Place Holders */
void board_init_r(gd_t *id, ulong dest_addr)
{
	/*Function attribute is no-return*/
	/*This Function never executes*/
	while (1)
		;
}

void save_boot_params(u32 r0, u32 r1, u32 r2, u32 r3)
{
}
