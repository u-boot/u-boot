/*
 * (C) Copyright 2010 Andreas Bießmann <andreas.devel@gmail.com>
 *
 * derived from previous work
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
int board_init(void)
{
	at91_pio_t *pio = (at91_pio_t *)AT91_PIO_BASE;

	/*
	 * Correct IRDA resistor problem
	 * Set PA23_TXD in Output
	 */
	writel(ATMEL_PMX_AA_TXD2, &pio->pioa.oer);

	/* arch number of AT91RM9200EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91RM9200EK;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}

int dram_init (void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((volatile long *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_DRIVER_AT91EMAC
int board_eth_init(bd_t *bis)
{
	return at91emac_register(bis, (u32) ATMEL_BASE_EMAC);
}
#endif
