/*
 * Copyright 2004 Freescale Semiconductor.
 * Jeff Brown (jeffrey@freescale.com)
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 *
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mpc86xx.h>
#include <asm/processor.h>

unsigned long get_board_sys_clk(ulong dummy);
unsigned long get_sysclk_from_px_regs(void);


void get_sys_info (sys_info_t *sysInfo)
{
	volatile immap_t    *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint plat_ratio, e600_ratio;

       	plat_ratio = (gur->porpllsr) & 0x0000003e;
	plat_ratio >>= 1;

	switch(plat_ratio) {
        case 0x0:
		sysInfo->freqSystemBus = 16 * CONFIG_SYS_CLK_FREQ;
		break;
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0c:
        case 0x10:
                sysInfo->freqSystemBus = plat_ratio * CONFIG_SYS_CLK_FREQ;
      	        break;
	default:
	        sysInfo->freqSystemBus = 0;
		break;
	}

#if 0
        printf("assigned system bus freq = %d for plat ratio 0x%08lx\n",
	       sysInfo->freqSystemBus, plat_ratio);
#endif

	e600_ratio = (gur->porpllsr) & 0x003f0000;
	e600_ratio >>= 16;

	switch (e600_ratio) {
	case 0x10:
		sysInfo->freqProcessor = 2 * sysInfo->freqSystemBus;
		break;
        case 0x19:
		sysInfo->freqProcessor = 5 * sysInfo->freqSystemBus/2;
		break;
	case 0x20:
		sysInfo->freqProcessor = 3 * sysInfo->freqSystemBus;
		break;
        case 0x39:
		sysInfo->freqProcessor = 7 * sysInfo->freqSystemBus/2;
		break;
	case 0x28:
		sysInfo->freqProcessor = 4 * sysInfo->freqSystemBus;
		break;
	case 0x1d:
		sysInfo->freqProcessor = 9 * sysInfo->freqSystemBus/2;
		break;
       	default:
		/* JB - Emulator workaround until real cop is plugged in */
		/* sysInfo->freqProcessor = 3 * sysInfo->freqSystemBus; */
		sysInfo->freqProcessor = e600_ratio + sysInfo->freqSystemBus;
		break;
	}
#if 0
        printf("assigned processor freq = %d for e600 ratio 0x%08lx\n",
	       sysInfo->freqProcessor, e600_ratio);
#endif
}


/*
 * Measure CPU clock speed (core clock GCLK1, GCLK2)
 * (Approx. GCLK frequency in Hz)
 */

int get_clocks(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	sys_info_t sys_info;

	get_sys_info(&sys_info);
	gd->cpu_clk = sys_info.freqProcessor;
	gd->bus_clk = sys_info.freqSystemBus;

	if (gd->cpu_clk != 0)
		return 0;
	else
		return 1;
}


/*
 * get_bus_freq
 *	Return system bus freq in Hz
 */
ulong get_bus_freq(ulong dummy)
{
	ulong val;
	sys_info_t sys_info;

	get_sys_info(&sys_info);
	val = sys_info.freqSystemBus;

	return val;
}

unsigned long get_sysclk_from_px_regs()
{
	ulong val;
	u8 vclkh, vclkl;

	vclkh = in8(PIXIS_BASE + PIXIS_VCLKH);
	vclkl = in8(PIXIS_BASE + PIXIS_VCLKL);
	
	if ((vclkh == 0x84) && (vclkl == 0x07)) {
		val = 33000000;
	}
	if ((vclkh == 0x3F) && (vclkl == 0x20)) {
		val = 40000000;
	}
	if ((vclkh == 0x3F) && (vclkl == 0x2A)) {
		val = 50000000;
	}
	if ((vclkh == 0x24) && (vclkl == 0x04)) {
		val = 66000000;
	}
	if ((vclkh == 0x3F) && (vclkl == 0x4B)) {
		val = 83000000;
	}
	if ((vclkh == 0x3F) && (vclkl == 0x5C)) {
		val = 100000000;
	}
	if ((vclkh == 0xDF) && (vclkl == 0x3B)) {
		val = 134000000;
	}
	if ((vclkh == 0xDF) && (vclkl == 0x4B)) {
		val = 166000000;
	}

	return val;
}


/*
 * get_board_sys_clk
 *	Reads the FPGA on board for CONFIG_SYS_CLK_FREQ
 */

unsigned long get_board_sys_clk(ulong dummy)
{
	u8 i, go_bit, rd_clks;
	ulong val;

	go_bit = in8(PIXIS_BASE + PIXIS_VCTL);
	go_bit &= 0x01;

	rd_clks = in8(PIXIS_BASE + PIXIS_VCFGEN0);
	rd_clks &= 0x1C;

	/*
	 * Only if both go bit and the SCLK bit in VCFGEN0 are set
	 * should we be using the AUX register. Remember, we also set the
	 * GO bit to boot from the alternate bank on the on-board flash
	 */

	if (go_bit) {
		if (rd_clks == 0x1c)
			i = in8(PIXIS_BASE + PIXIS_AUX);
		else
			i = in8(PIXIS_BASE + PIXIS_SPD);
	} else {
		i = in8(PIXIS_BASE + PIXIS_SPD);
	}

	i &= 0x07;

	switch (i) {
	case 0:
		val = 33000000;
		break;
	case 1:
		val = 40000000;
		break;
	case 2:
		val = 50000000;
		break;
	case 3:
		val = 66000000;
		break;
	case 4:
		val = 83000000;
		break;
	case 5:
		val = 100000000;
		break;
	case 6:
		val = 134000000;
		break;
	case 7:
		val = 166000000;
		break;
	}

	return val;
}
