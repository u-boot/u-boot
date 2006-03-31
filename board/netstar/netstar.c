/*
 * (C) Copyright 2005 2N TELEKOMUNIKACE, Ladislav Michl
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

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* arch number of NetStar board */
	/* TODO: use define from asm/mach-types.h */
	gd->bd->bi_arch_number = 692;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x10000100;

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	/* Take the Ethernet controller out of reset and wait
	 * for the EEPROM load to complete. */
	*((volatile unsigned short *) GPIO_DATA_OUTPUT_REG) |= 0x80;
	udelay(10);	/* doesn't work before interrupt_init call */
	*((volatile unsigned short *) GPIO_DATA_OUTPUT_REG) &= ~0x80;
	udelay(500);

	return 0;
}

extern void partition_flash(void);

int misc_init_r(void)
{
	return 0;
}

extern void nand_init(void);

int board_late_init(void)
{
	return 0;
}
