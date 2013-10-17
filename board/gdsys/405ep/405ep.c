/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/global_data.h>

#include "405ep.h"
#include <gdsys_fpga.h>

#define REFLECTION_TESTPATTERN 0xdede
#define REFLECTION_TESTPATTERN_INV (~REFLECTION_TESTPATTERN & 0xffff)

#ifdef CONFIG_SYS_FPGA_NO_RFL_HI
#define REFLECTION_TESTREG reflection_low
#else
#define REFLECTION_TESTREG reflection_high
#endif

DECLARE_GLOBAL_DATA_PTR;

int get_fpga_state(unsigned dev)
{
	return gd->arch.fpga_state[dev];
}

void print_fpga_state(unsigned dev)
{
	if (gd->arch.fpga_state[dev] & FPGA_STATE_DONE_FAILED)
		puts("       Waiting for FPGA-DONE timed out.\n");
	if (gd->arch.fpga_state[dev] & FPGA_STATE_REFLECTION_FAILED)
		puts("       FPGA reflection test failed.\n");
}

int board_early_init_f(void)
{
	unsigned k;

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k)
		gd->arch.fpga_state[k] = 0;

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
	return 0;
}

int board_early_init_r(void)
{
	unsigned k;
	unsigned ctr;

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k)
		gd->arch.fpga_state[k] = 0;

	/*
	 * reset FPGA
	 */
	gd405ep_init();

	gd405ep_set_fpga_reset(1);

	gd405ep_setup_hw();

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k) {
		ctr = 0;
		while (!gd405ep_get_fpga_done(k)) {
			udelay(100000);
			if (ctr++ > 5) {
				gd->arch.fpga_state[k] |=
					FPGA_STATE_DONE_FAILED;
				break;
			}
		}
	}

	udelay(10);

	gd405ep_set_fpga_reset(0);

	for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k) {
		/*
		 * wait for fpga out of reset
		 */
		ctr = 0;
		while (1) {
			u16 val;

			FPGA_SET_REG(k, reflection_low, REFLECTION_TESTPATTERN);

			FPGA_GET_REG(k, REFLECTION_TESTREG, &val);
			if (val == REFLECTION_TESTPATTERN_INV)
				break;

			udelay(100000);
			if (ctr++ > 5) {
				gd->arch.fpga_state[k] |=
					FPGA_STATE_REFLECTION_FAILED;
				break;
			}
		}
	}

	return 0;
}
