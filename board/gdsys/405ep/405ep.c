/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
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
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>

#include "../common/fpga.h"

#define LATCH0_BASE (CONFIG_SYS_LATCH_BASE)
#define LATCH1_BASE (CONFIG_SYS_LATCH_BASE + 0x100)
#define LATCH2_BASE (CONFIG_SYS_LATCH_BASE + 0x200)

#define REFLECTION_TESTPATTERN 0xdede
#define REFLECTION_TESTPATTERN_INV (~REFLECTION_TESTPATTERN & 0xffff)

int board_early_init_f(void)
{
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr(UIC0CR, 0x00000000);	/* set all to be non-critical */
	mtdcr(UIC0PR, 0xFFFFFF80);	/* set int polarities */
	mtdcr(UIC0TR, 0x10000000);	/* set int trigger levels */
	mtdcr(UIC0VCR, 0x00000001);	/* set vect base=0,INT0 highest prio */
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */

	/*
	 * EBC Configuration Register: set ready timeout to 512 ebc-clks
	 * -> ca. 15 us
	 */
	mtebc(EBC0_CFG, 0xa8400000);	/* ebc always driven */

	/*
	 * setup io-latches for reset
	 */
	out_le16((void *)LATCH0_BASE, CONFIG_SYS_LATCH0_RESET);
	out_le16((void *)LATCH1_BASE, CONFIG_SYS_LATCH1_RESET);

	/*
	 * set "startup-finished"-gpios
	 */
	gpio_write_bit(21, 0);
	gpio_write_bit(22, 1);

	/*
	 * wait for fpga-done
	 * fail ungraceful if fpga is not configuring properly
	 */
	while (!(in_le16((void *)LATCH2_BASE) & 0x0010))
		;

	/*
	 * setup io-latches for boot (stop reset)
	 */
	udelay(10);
	out_le16((void *)LATCH0_BASE, CONFIG_SYS_LATCH0_BOOT);
	out_le16((void *)LATCH1_BASE, CONFIG_SYS_LATCH1_BOOT);

	/*
	 * wait for fpga out of reset
	 * fail ungraceful if fpga is not working properly
	 */
	while (1) {
		fpga_set_reg(CONFIG_SYS_FPGA_RFL_LOW, REFLECTION_TESTPATTERN);
		if (fpga_get_reg(CONFIG_SYS_FPGA_RFL_HIGH) ==
			REFLECTION_TESTPATTERN_INV)
			break;
	}

	return 0;
}
