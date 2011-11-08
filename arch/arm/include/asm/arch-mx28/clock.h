/*
 * Freescale i.MX28 Clock
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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
 *
 */

#ifndef __CLOCK_H__
#define __CLOCK_H__

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_AHB_CLK,
	MXC_IPG_CLK,
	MXC_EMI_CLK,
	MXC_GPMI_CLK,
	MXC_IO0_CLK,
	MXC_IO1_CLK,
	MXC_SSP0_CLK,
	MXC_SSP1_CLK,
	MXC_SSP2_CLK,
	MXC_SSP3_CLK,
};

enum mxs_ioclock {
	MXC_IOCLK0 = 0,
	MXC_IOCLK1,
};

enum mxs_sspclock {
	MXC_SSPCLK0 = 0,
	MXC_SSPCLK1,
	MXC_SSPCLK2,
	MXC_SSPCLK3,
};

uint32_t mxc_get_clock(enum mxc_clock clk);

void mx28_set_ioclk(enum mxs_ioclock io, uint32_t freq);
void mx28_set_sspclk(enum mxs_sspclock ssp, uint32_t freq, int xtal);
void mx28_set_ssp_busclock(unsigned int bus, uint32_t freq);

/* Compatibility with the FEC Ethernet driver */
#define	imx_get_fecclk()	mxc_get_clock(MXC_AHB_CLK)

#endif	/* __CLOCK_H__ */
