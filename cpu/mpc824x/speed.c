/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Gregory E. Allen, gallen@arlut.utexas.edu
 * Applied Research Laboratories, The University of Texas at Austin
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
#include <mpc824x.h>
#include <asm/processor.h>

/* ------------------------------------------------------------------------- */
/* NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency on
 * PCI_SYNC_IN .
 *
 * CONFIG_PLL_PCI_TO_MEM_MULTIPLIER is only required on MPC8240
 * boards. It should be defined as the PCI to Memory Multiplier as
 * documented in the MPC8240 Hardware Specs.
 *
 * Other mpc824x boards don't need CONFIG_PLL_PCI_TO_MEM_MULTIPLIER
 * because they can determine it from the PCR.
 *
 * Gary Milliorn <gary.milliorn@motorola.com> (who should know since
 * he designed the Sandpoint) told us that the PCR is not in all revs
 * of the MPC8240 CPU, so it's not guaranteeable and we cannot do
 * away with CONFIG_PLL_PCI_TO_MEM_MULTIPLIER altogether.
 */
/* ------------------------------------------------------------------------- */

/* This gives the PCI to Memory multiplier times 10 */
/* The index is the value of PLL_CFG[0:4] */
/* This is documented in the MPC8240/5 Hardware Specs */

short pll_pci_to_mem_multiplier[] = {
#if defined(CONFIG_MPC8240)
	30, 30, 10, 10, 20, 10,  0, 10,
	10,  0, 20,  0, 20,  0, 20,  0,
	30,  0, 15,  0, 20,  0, 20,  0,
	25,  0, 10,  0, 15, 15,  0,  0,
#elif defined(CONFIG_MPC8245)
	30, 30, 10, 10, 20, 10, 10, 10,
	10, 20, 20, 15, 20, 15, 20, 30,
	30, 40, 15, 40, 20, 25, 20, 40,
	25, 20, 10, 20, 15, 15, 15,  0,
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif
};

#define CU824_PLL_STATE_REG	0xFE80002F
#define PCR			0x800000E2

/* ------------------------------------------------------------------------- */

/* compute the memory bus clock frequency */
ulong get_bus_freq (ulong dummy)
{
	unsigned char pll_cfg;
#if defined(CONFIG_MPC8240) && !defined(CONFIG_CU824)
	return (CONFIG_SYS_CLK_FREQ) * (CONFIG_PLL_PCI_TO_MEM_MULTIPLIER);
#elif defined(CONFIG_CU824)
	pll_cfg = *(volatile unsigned char *) (CU824_PLL_STATE_REG);
	pll_cfg &= 0x1f;
#else
	CONFIG_READ_BYTE(PCR, pll_cfg);
	pll_cfg = (pll_cfg >> 3) & 0x1f;
#endif
	return ((CONFIG_SYS_CLK_FREQ) * pll_pci_to_mem_multiplier[pll_cfg] + 5) / 10;
}


/* ------------------------------------------------------------------------- */

/* This gives the Memory to CPU Core multiplier times 10 */
/* The index is the value of PLLRATIO in HID1 */
/* This is documented in the MPC8240 Hardware Specs */
/* This is not documented for MPC8245 ? FIXME */
short pllratio_to_factor[] = {
     0,  0,  0, 10, 20, 20, 25, 45,
    30,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0, 10,  0,  0,  0, 45,
    30,  0, 40,  0,  0,  0, 35,  0,
};

/* compute the CPU and memory bus clock frequencies */
int get_clocks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	uint hid1 = mfspr(HID1);
	hid1 = (hid1 >> (32-5)) & 0x1f;
	gd->cpu_clk = (pllratio_to_factor[hid1] * get_bus_freq(0) + 5)
			  / 10;
	gd->bus_clk = get_bus_freq(0);
	return (0);
}
