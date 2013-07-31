/*
 * (C) Copyright 2008
 * Martha J Marx, Silicon Turnkey Express, mmarx@silicontkx.com
 * mpc512x I/O pin/pad initialization for the ADS5121 board
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/types.h>
#include <asm/io.h>

void iopin_initialize(iopin_t *ioregs_init, int len)
{
	short i, j, p;
	u32 *reg;
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

	reg = (u32 *)&(im->io_ctrl);

	if (sizeof(ioregs_init) == 0)
		return;

	for (i = 0; i < len; i++) {
		for (p = 0, j = ioregs_init[i].p_offset / sizeof(u_long);
			p < ioregs_init[i].nr_pins; p++, j++) {
			if (ioregs_init[i].bit_or)
				setbits_be32(reg + j, ioregs_init[i].val);
			else
				out_be32 (reg + j, ioregs_init[i].val);
		}
	}
	return;
}

void iopin_initialize_bits(iopin_t *ioregs_init, int len)
{
	short i, j, p;
	u32 *reg, mask;
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

	reg = (u32 *)&(im->io_ctrl);

	/* iterate over table entries */
	for (i = 0; i < len; i++) {
		/* iterate over pins within a table entry */
		for (p = 0, j = ioregs_init[i].p_offset / sizeof(u_long);
			p < ioregs_init[i].nr_pins; p++, j++) {
			if (ioregs_init[i].bit_or & IO_PIN_OVER_EACH) {
				/* replace all settings at once */
				out_be32(reg + j, ioregs_init[i].val);
			} else {
				/*
				 * only replace individual parts, but
				 * REPLACE them instead of just ORing
				 * them in and "inheriting" previously
				 * set bits which we don't want
				 */
				mask = 0;
				if (ioregs_init[i].bit_or & IO_PIN_OVER_FMUX)
					mask |= IO_PIN_FMUX(3);

				if (ioregs_init[i].bit_or & IO_PIN_OVER_HOLD)
					mask |= IO_PIN_HOLD(3);

				if (ioregs_init[i].bit_or & IO_PIN_OVER_PULL)
					mask |= IO_PIN_PUD(1) | IO_PIN_PUE(1);

				if (ioregs_init[i].bit_or & IO_PIN_OVER_STRIG)
					mask |= IO_PIN_ST(1);

				if (ioregs_init[i].bit_or & IO_PIN_OVER_DRVSTR)
					mask |= IO_PIN_DS(3);
				/*
				 * DON'T do the "mask, then insert"
				 * in place on the register, it may
				 * break access to external hardware
				 * (like boot ROMs) when configuring
				 * LPB related pins, while the code to
				 * configure the pin is read from this
				 * very address region
				 */
				clrsetbits_be32(reg + j, mask,
						ioregs_init[i].val & mask);
			}
		}
	}
}
