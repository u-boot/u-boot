/*
 * Copyright (C) ST-Ericsson SA 2009
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

#ifndef __ASM_ARCH_CLOCK
#define __ASM_ARCH_CLOCK

struct prcmu {
	unsigned int armclkfix_mgt;
	unsigned int armclk_mgt;
	unsigned int svammdspclk_mgt;
	unsigned int siammdspclk_mgt;
	unsigned int reserved;
	unsigned int sgaclk_mgt;
	unsigned int uartclk_mgt;
	unsigned int msp02clk_mgt;
	unsigned int i2cclk_mgt;
	unsigned int sdmmcclk_mgt;
	unsigned int slimclk_mgt;
	unsigned int per1clk_mgt;
	unsigned int per2clk_mgt;
	unsigned int per3clk_mgt;
	unsigned int per5clk_mgt;
	unsigned int per6clk_mgt;
	unsigned int per7clk_mgt;
	unsigned int lcdclk_mgt;
	unsigned int reserved1;
	unsigned int bmlclk_mgt;
	unsigned int hsitxclk_mgt;
	unsigned int hsirxclk_mgt;
	unsigned int hdmiclk_mgt;
	unsigned int apeatclk_mgt;
	unsigned int apetraceclk_mgt;
	unsigned int mcdeclk_mgt;
	unsigned int ipi2cclk_mgt;
	unsigned int dsialtclk_mgt;
	unsigned int spare2clk_mgt;
	unsigned int dmaclk_mgt;
	unsigned int b2r2clk_mgt;
	unsigned int tvclk_mgt;
	unsigned int unused[82];
	unsigned int tcr;
	unsigned int unused1[23];
	unsigned int ape_softrst;
};

extern void u8500_clock_enable(int periph, int kern, int cluster);

void db8500_clocks_init(void);

#endif /* __ASM_ARCH_CLOCK */
