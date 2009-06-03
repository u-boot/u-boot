/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * Derived from: board/omap3/zoom1/zoom1.h
 * Nishanth Menon <nm@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _BOARD_ZOOM2_H_
#define _BOARD_ZOOM2_H_

const omap3_sysinfo sysinfo = {
	DDR_STACKED,
	"OMAP3 Zoom2 ",
	"NAND",
};

typedef enum {
	ZOOM2_REVISION_UNKNOWN = 0,
	ZOOM2_REVISION_ALPHA,
	ZOOM2_REVISION_BETA,
	ZOOM2_REVISION_PRODUCTION
} zoom2_revision;

zoom2_revision zoom2_get_revision(void);

/*
 * IEN	- Input Enable
 * IDIS	- Input Disable
 * PTD	- Pull type Down
 * PTU	- Pull type Up
 * DIS	- Pull type selection is inactive
 * EN	- Pull type selection is active
 * M0	- Mode 0
 * The commented string gives the final mux configuration for that pin
 */
#define MUX_ZOOM2() \
 /* SDRC*/\
 MUX_VAL(CP(SDRC_D0), (IEN | PTD | DIS | M0))		/* SDRC_D0 */\
 MUX_VAL(CP(SDRC_D1), (IEN | PTD | DIS | M0))		/* SDRC_D1 */\
 MUX_VAL(CP(SDRC_D2), (IEN | PTD | DIS | M0))		/* SDRC_D2 */\
 MUX_VAL(CP(SDRC_D3), (IEN | PTD | DIS | M0))		/* SDRC_D3 */\
 MUX_VAL(CP(SDRC_D4), (IEN | PTD | DIS | M0))		/* SDRC_D4 */\
 MUX_VAL(CP(SDRC_D5), (IEN | PTD | DIS | M0))		/* SDRC_D5 */\
 MUX_VAL(CP(SDRC_D6), (IEN | PTD | DIS | M0))		/* SDRC_D6 */\
 MUX_VAL(CP(SDRC_D7), (IEN | PTD | DIS | M0))		/* SDRC_D7 */\
 MUX_VAL(CP(SDRC_D8), (IEN | PTD | DIS | M0))		/* SDRC_D8 */\
 MUX_VAL(CP(SDRC_D9), (IEN | PTD | DIS | M0))		/* SDRC_D9 */\
 MUX_VAL(CP(SDRC_D10), (IEN | PTD | DIS | M0))		/* SDRC_D10 */\
 MUX_VAL(CP(SDRC_D11), (IEN | PTD | DIS | M0))		/* SDRC_D11 */\
 MUX_VAL(CP(SDRC_D12), (IEN | PTD | DIS | M0))		/* SDRC_D12 */\
 MUX_VAL(CP(SDRC_D13), (IEN | PTD | DIS | M0))		/* SDRC_D13 */\
 MUX_VAL(CP(SDRC_D14), (IEN | PTD | DIS | M0))		/* SDRC_D14 */\
 MUX_VAL(CP(SDRC_D15), (IEN | PTD | DIS | M0))		/* SDRC_D15 */\
 MUX_VAL(CP(SDRC_D16), (IEN | PTD | DIS | M0))		/* SDRC_D16 */\
 MUX_VAL(CP(SDRC_D17), (IEN | PTD | DIS | M0))		/* SDRC_D17 */\
 MUX_VAL(CP(SDRC_D18), (IEN | PTD | DIS | M0))		/* SDRC_D18 */\
 MUX_VAL(CP(SDRC_D19), (IEN | PTD | DIS | M0))		/* SDRC_D19 */\
 MUX_VAL(CP(SDRC_D20), (IEN | PTD | DIS | M0))		/* SDRC_D20 */\
 MUX_VAL(CP(SDRC_D21), (IEN | PTD | DIS | M0))		/* SDRC_D21 */\
 MUX_VAL(CP(SDRC_D22), (IEN | PTD | DIS | M0))		/* SDRC_D22 */\
 MUX_VAL(CP(SDRC_D23), (IEN | PTD | DIS | M0))		/* SDRC_D23 */\
 MUX_VAL(CP(SDRC_D24), (IEN | PTD | DIS | M0))		/* SDRC_D24 */\
 MUX_VAL(CP(SDRC_D25), (IEN | PTD | DIS | M0))		/* SDRC_D25 */\
 MUX_VAL(CP(SDRC_D26), (IEN | PTD | DIS | M0))		/* SDRC_D26 */\
 MUX_VAL(CP(SDRC_D27), (IEN | PTD | DIS | M0))		/* SDRC_D27 */\
 MUX_VAL(CP(SDRC_D28), (IEN | PTD | DIS | M0))		/* SDRC_D28 */\
 MUX_VAL(CP(SDRC_D29), (IEN | PTD | DIS | M0))		/* SDRC_D29 */\
 MUX_VAL(CP(SDRC_D30), (IEN | PTD | DIS | M0))		/* SDRC_D30 */\
 MUX_VAL(CP(SDRC_D31), (IEN | PTD | DIS | M0))		/* SDRC_D31 */\
 MUX_VAL(CP(SDRC_CLK), (IEN | PTD | DIS | M0))		/* SDRC_CLK */\
 MUX_VAL(CP(SDRC_DQS0), (IEN | PTD | DIS | M0))		/* SDRC_DQS0 */\
 MUX_VAL(CP(SDRC_DQS1), (IEN | PTD | DIS | M0))		/* SDRC_DQS1 */\
 MUX_VAL(CP(SDRC_DQS2), (IEN | PTD | DIS | M0))		/* SDRC_DQS2 */\
 MUX_VAL(CP(SDRC_DQS3), (IEN | PTD | DIS | M0))		/* SDRC_DQS3 */\
/* GPMC */\
 MUX_VAL(CP(GPMC_A1), (IDIS | PTD | DIS | M0))		/* GPMC_A1 */\
 MUX_VAL(CP(GPMC_A2), (IDIS | PTD | DIS | M0))		/* GPMC_A2 */\
 MUX_VAL(CP(GPMC_A3), (IDIS | PTD | DIS | M0))		/* GPMC_A3 */\
 MUX_VAL(CP(GPMC_A4), (IDIS | PTD | DIS | M0))		/* GPMC_A4 */\
 MUX_VAL(CP(GPMC_A5), (IDIS | PTD | DIS | M0))		/* GPMC_A5 */\
 MUX_VAL(CP(GPMC_A6), (IDIS | PTD | DIS | M0))		/* GPMC_A6 */\
 MUX_VAL(CP(GPMC_A7), (IDIS | PTD | DIS | M0))		/* GPMC_A7 */\
 MUX_VAL(CP(GPMC_A8), (IDIS | PTD | DIS | M0))		/* GPMC_A8 */\
 MUX_VAL(CP(GPMC_A9), (IDIS | PTD | DIS | M0))		/* GPMC_A9 */\
 MUX_VAL(CP(GPMC_A10), (IDIS | PTD | DIS | M0))		/* GPMC_A10 */\
 MUX_VAL(CP(GPMC_D0), (IEN | PTD | DIS | M0))		/* GPMC_D0 */\
 MUX_VAL(CP(GPMC_D1), (IEN | PTD | DIS | M0))		/* GPMC_D1 */\
 MUX_VAL(CP(GPMC_D2), (IEN | PTD | DIS | M0))		/* GPMC_D2 */\
 MUX_VAL(CP(GPMC_D3), (IEN | PTD | DIS | M0))		/* GPMC_D3 */\
 MUX_VAL(CP(GPMC_D4), (IEN | PTD | DIS | M0))		/* GPMC_D4 */\
 MUX_VAL(CP(GPMC_D5), (IEN | PTD | DIS | M0))		/* GPMC_D5 */\
 MUX_VAL(CP(GPMC_D6), (IEN | PTD | DIS | M0))		/* GPMC_D6 */\
 MUX_VAL(CP(GPMC_D7), (IEN | PTD | DIS | M0))		/* GPMC_D7 */\
 MUX_VAL(CP(GPMC_D8), (IEN | PTD | DIS | M0))		/* GPMC_D8 */\
 MUX_VAL(CP(GPMC_D9), (IEN | PTD | DIS | M0))		/* GPMC_D9 */\
 MUX_VAL(CP(GPMC_D10), (IEN | PTD | DIS | M0))		/* GPMC_D10 */\
 MUX_VAL(CP(GPMC_D11), (IEN | PTD | DIS | M0))		/* GPMC_D11 */\
 MUX_VAL(CP(GPMC_D12), (IEN | PTD | DIS | M0))		/* GPMC_D12 */\
 MUX_VAL(CP(GPMC_D13), (IEN | PTD | DIS | M0))		/* GPMC_D13 */\
 MUX_VAL(CP(GPMC_D14), (IEN | PTD | DIS | M0))		/* GPMC_D14 */\
 MUX_VAL(CP(GPMC_D15), (IEN | PTD | DIS | M0))		/* GPMC_D15 */\
 MUX_VAL(CP(GPMC_NCS0), (IDIS | PTU | EN | M0))		/* GPMC_nCS0 */\
 MUX_VAL(CP(GPMC_NCS1), (IDIS | PTU | EN | M7))		/* GPMC_nCS1 */\
 MUX_VAL(CP(GPMC_NCS2), (IDIS | PTU | EN | M7))		/* GPMC_nCS2 */\
 MUX_VAL(CP(GPMC_NCS3), (IDIS | PTU | EN | M7))		/* GPMC_nCS3 */\
 MUX_VAL(CP(GPMC_NCS4), (IDIS | PTU | EN | M7))		/* GPMC_nCS4 */\
 MUX_VAL(CP(GPMC_NCS5), (IDIS | PTD | DIS | M7))	/* GPMC_nCS5 */\
 MUX_VAL(CP(GPMC_NCS6), (IEN | PTD | DIS | M7))		/* GPMC_nCS6 */\
 MUX_VAL(CP(GPMC_NCS7), (IEN | PTU | EN | M7))		/* GPMC_nCS7 */\
 MUX_VAL(CP(GPMC_CLK), (IDIS | PTD | DIS | M0))		/* GPMC_CLK */\
 MUX_VAL(CP(GPMC_NADV_ALE), (IDIS | PTD | DIS | M0))	/* GPMC_nADV_ALE */\
 MUX_VAL(CP(GPMC_NOE), (IDIS | PTD | DIS | M0))		/* GPMC_nOE */\
 MUX_VAL(CP(GPMC_NWE), (IDIS | PTD | DIS | M0))		/* GPMC_nWE */\
 MUX_VAL(CP(GPMC_NWP), (IDIS | PTU | DIS | M0))		/* GPMC_nWP */\
 MUX_VAL(CP(GPMC_NBE0_CLE), (IDIS | PTD | DIS | M0))	/* GPMC_nBE0_CLE */\
 MUX_VAL(CP(GPMC_NBE1), (IEN | PTD | DIS | M0))		/* GPMC_nBE1 */\
 MUX_VAL(CP(GPMC_WAIT0), (IEN | PTD | EN | M0))		/* GPMC_WAIT0 */\
 MUX_VAL(CP(GPMC_WAIT1), (IEN | PTU | EN | M0))		/* GPMC_WAIT1 */\
 MUX_VAL(CP(GPMC_WAIT2), (IEN | PTU | EN | M0))		/* GPMC_WAIT2 */\
 MUX_VAL(CP(GPMC_WAIT3), (IEN | PTU | EN | M0))		/* GPMC_WAIT3 */\
/* IDCC modem Power On */\
 MUX_VAL(CP(CAM_D11), (IEN | PTU | EN | M4))		/* GPIO_110 */\
 MUX_VAL(CP(CAM_D4), (IEN | PTU | EN | M4))		/* GPIO_103 */\
/* GPMC CS7 has LAN9211 device */\
 MUX_VAL(CP(GPMC_NCS7), (IDIS | PTU | EN | M0))		/* GPMC_nCS7 */\
 MUX_VAL(CP(MCBSP1_DX), (IEN | PTD | DIS | M4))		/* LAN9221 */\
 MUX_VAL(CP(MCSPI1_CS2), (IEN | PTD | EN | M0))		/* MCSPI1_CS2 */\
/* GPMC CS3 has Serial TL16CP754C device */\
 MUX_VAL(CP(GPMC_NCS3), (IDIS | PTU | EN | M0))		/* GPMC_nCS3 */\
/* Toggle Reset pin of TL16CP754C device */\
 MUX_VAL(CP(MCBSP4_CLKX), (IEN | PTU | EN | M4))	/* GPIO_152 */\
 udelay(10);\
 MUX_VAL(CP(MCBSP4_CLKX), (IEN | PTD | EN | M4))	/* GPIO_152 */\
 MUX_VAL(CP(SDRC_CKE1), (IDIS | PTU | EN | M0))		/* SDRC_CKE1 */\
/* LEDS */\
 MUX_VAL(CP(MCSPI1_SOMI), (IEN | PTD | EN | M4))	/* GPIO_173 red  */\
 MUX_VAL(CP(MCBSP4_DX), (IEN | PTD | EN | M4))		/* GPIO_154 blue  */\
 MUX_VAL(CP(GPMC_NBE1), (IEN | PTD | EN | M4))		/* GPIO_61 blue2  */

#endif /* _BOARD_ZOOM2_H_ */
