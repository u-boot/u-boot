/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
#if defined(CONFIG_LH7A400)
#include <lh7a400.h>
#elif defined(CONFIG_LH7A404)
#include <lh7a404.h>
#else
#error "No CPU defined!"
#endif
#include <asm/mach-types.h>

#include <lpd7a400_cpld.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	/* set up the I/O ports */

	/* enable flash programming */
	*(LPD7A400_CPLD_REGPTR(LPD7A400_CPLD_FLASH_REG)) |= FLASH_FPEN;

	/* Auto wakeup, LCD disable, WLAN enable */
	*(LPD7A400_CPLD_REGPTR(LPD7A400_CPLD_CECTL_REG)) &=
		~(CECTL_AWKP|CECTL_LCDV|CECTL_WLPE);

	/* Status LED 2 on (leds are active low) */
	*(LPD7A400_CPLD_REGPTR(LPD7A400_CPLD_EXTGPIO_REG)) =
		(EXTGPIO_STATUS1|EXTGPIO_GPIO1) & ~(EXTGPIO_STATUS2);

#if defined(CONFIG_LH7A400)
	/* arch number of Logic-Board - MACH_TYPE_LPD7A400 */
	gd->bd->bi_arch_number = MACH_TYPE_LPD7A400;
#elif defined(CONFIG_LH7A404)
	/* arch number of Logic-Board - MACH_TYPE_LPD7A400 */
	gd->bd->bi_arch_number = MACH_TYPE_LPD7A404;
#endif

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xc0000100;

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}
