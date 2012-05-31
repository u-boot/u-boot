/*
 * Palm Tungsten|C Support
 *
 * Copyright (C) 2009-2010 Marek Vasut <marek.vasut@gmail.com>
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
#include <command.h>
#include <serial.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	/* Arch number of Palm Tungsten|C */
	gd->bd->bi_arch_number = MACH_TYPE_PALMTC;

	/* Adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* Set PWM for LCD */
	writel(0x5f, PWM_CTRL1);
	writel(0x3ff, PWM_PERVAL1);
	writel(892, PWM_PWDUTY1);

	return 0;
}

struct serial_device *default_serial_console(void)
{
	return &serial_ffuart_device;
}

extern void pxa_dram_init(void);
int dram_init(void)
{
	pxa_dram_init();
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}
