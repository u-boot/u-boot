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
#include <asm/global_data.h>

#include <gdsys_fpga.h>

#define LATCH0_BASE (CONFIG_SYS_LATCH_BASE)
#define LATCH1_BASE (CONFIG_SYS_LATCH_BASE + 0x100)
#define LATCH2_BASE (CONFIG_SYS_LATCH_BASE + 0x200)

#define REFLECTION_TESTPATTERN 0xdede
#define REFLECTION_TESTPATTERN_INV (~REFLECTION_TESTPATTERN & 0xffff)

DECLARE_GLOBAL_DATA_PTR;

int get_fpga_state(unsigned dev)
{
	return gd->fpga_state[dev];
}

void print_fpga_state(unsigned dev)
{
	if (gd->fpga_state[dev] & FPGA_STATE_DONE_FAILED)
		puts("       Waiting for FPGA-DONE timed out.\n");
	if (gd->fpga_state[dev] & FPGA_STATE_REFLECTION_FAILED)
		puts("       FPGA reflection test failed.\n");
}

int board_early_init_f(void)
{
	unsigned k;
	unsigned ctr;

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k)
		gd->fpga_state[k] = 0;

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
	 */
	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k) {
		ctr = 0;
		while (!(in_le16((void *)LATCH2_BASE)
			& CONFIG_SYS_FPGA_DONE(k))) {
			udelay(100000);
			if (ctr++ > 5) {
				gd->fpga_state[k] |= FPGA_STATE_DONE_FAILED;
				break;
			}
		}
	}

	/*
	 * setup io-latches for boot (stop reset)
	 */
	udelay(10);
	out_le16((void *)LATCH0_BASE, CONFIG_SYS_LATCH0_BOOT);
	out_le16((void *)LATCH1_BASE, CONFIG_SYS_LATCH1_BOOT);

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k) {
		ihs_fpga_t *fpga = (ihs_fpga_t *) CONFIG_SYS_FPGA_BASE(k);
#ifdef CONFIG_SYS_FPGA_NO_RFL_HI
		u16 *reflection_target = &fpga->reflection_low;
#else
		u16 *reflection_target = &fpga->reflection_high;
#endif
		/*
		 * wait for fpga out of reset
		 */
		ctr = 0;
		while (1) {
			out_le16(&fpga->reflection_low,
				REFLECTION_TESTPATTERN);

			if (in_le16(reflection_target) ==
				REFLECTION_TESTPATTERN_INV)
				break;

			udelay(100000);
			if (ctr++ > 5) {
				gd->fpga_state[k] |=
					FPGA_STATE_REFLECTION_FAILED;
				break;
			}
		}
	}

	return 0;
}
