/*
 * (C) Copyright 2008
 * Martha J Marx, Silicon Turnkey Express, mmarx@silicontkx.com
 * mpc512x I/O pin/pad initialization for the ADS5121 board
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
#include <linux/types.h>
#include "iopin.h"

/* IO pin fields */
#define IO_PIN_FMUX(v)	((v) << 7)	/* pin function */
#define IO_PIN_HOLD(v)	((v) << 5)	/* hold time, pci only */
#define IO_PIN_PUD(v)	((v) << 4)	/* if PUE, 0=pull-down, 1=pull-up */
#define IO_PIN_PUE(v)	((v) << 3)	/* pull up/down enable */
#define IO_PIN_ST(v)	((v) << 2)	/* schmitt trigger */
#define IO_PIN_DS(v)	((v))		/* slew rate */

static struct iopin_t {
	int p_offset;		/* offset from IOCTL_MEM_OFFSET */
	int nr_pins;		/* number of pins to set this way */
	int bit_or;		/* or in the value instead of overwrite */
	u_long val;		/* value to write or or */
} ioregs_init[] = {
	/* FUNC1=FEC_RX_DV Sets Next 3 to FEC pads */
	{
		IOCTL_SPDIF_TXCLK, 3, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* Set highest Slew on 9 PATA pins */
	{
		IOCTL_PATA_CE1, 9, 1,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=FEC_COL Sets Next 15 to FEC pads */
	{
		IOCTL_PSC0_0, 15, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=SPDIF_TXCLK */
	{
		IOCTL_LPC_CS1, 1, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=SPDIF_TX and sets Next pin to SPDIF_RX */
	{
		IOCTL_I2C1_SCL, 2, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=DIU CLK */
	{
		IOCTL_PSC6_0, 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=DIU_HSYNC */
	{
		IOCTL_PSC6_1, 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC2=DIUVSYNC Sets Next 26 to DIU Pads */
	{
		IOCTL_PSC6_4, 26, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	}
};

void iopin_initialize(void)
{
	short i, j, n, p;
	u_long *reg;
	immap_t *im = (immap_t *)CFG_IMMR;

	reg = (u_long *)&(im->io_ctrl.regs[0]);

	if (sizeof(ioregs_init) == 0)
		return;

	n = sizeof(ioregs_init) / sizeof(ioregs_init[0]);

	for (i = 0; i < n; i++) {
		for (p = 0, j = ioregs_init[i].p_offset / sizeof(u_long);
			p < ioregs_init[i].nr_pins; p++, j++) {
			if (ioregs_init[i].bit_or)
				reg[j] |= ioregs_init[i].val;
			else
				reg[j] = ioregs_init[i].val;
		}
	}
	return;
}
