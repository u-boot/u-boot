/*
 * iPAQ h2200 board configuration
 *
 * Copyright (C) 2012 Lukasz Dalek <luk0104@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <asm/arch/pxa.h>
#include <asm/arch/pxa-regs.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	gd->bd->bi_arch_number = MACH_TYPE_H2200;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	return 0;
}

int dram_init(void)
{
	/*
	 * Everything except MSC0 was already set up by
	 * 1st stage bootloader.
	 *
	 * This setting enables access to companion chip.
	 */
	clrsetbits_le32(MSC0, 0xffffffff, CONFIG_SYS_MSC0_VAL);
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}
