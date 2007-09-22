/*
 * Copyright 2004 Freescale Semiconductor.
 * Jeff Brown
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

DECLARE_GLOBAL_DATA_PTR;

void get_sys_info(sys_info_t *sysInfo)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint plat_ratio, e600_ratio;

	plat_ratio = (gur->porpllsr) & 0x0000003e;
	plat_ratio >>= 1;

	switch (plat_ratio) {
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

	e600_ratio = (gur->porpllsr) & 0x003f0000;
	e600_ratio >>= 16;

	switch (e600_ratio) {
	case 0x10:
		sysInfo->freqProcessor = 2 * sysInfo->freqSystemBus;
		break;
	case 0x19:
		sysInfo->freqProcessor = 5 * sysInfo->freqSystemBus / 2;
		break;
	case 0x20:
		sysInfo->freqProcessor = 3 * sysInfo->freqSystemBus;
		break;
	case 0x39:
		sysInfo->freqProcessor = 7 * sysInfo->freqSystemBus / 2;
		break;
	case 0x28:
		sysInfo->freqProcessor = 4 * sysInfo->freqSystemBus;
		break;
	case 0x1d:
		sysInfo->freqProcessor = 9 * sysInfo->freqSystemBus / 2;
		break;
	default:
		sysInfo->freqProcessor = e600_ratio + sysInfo->freqSystemBus;
		break;
	}
}


/*
 * Measure CPU clock speed (core clock GCLK1, GCLK2)
 * (Approx. GCLK frequency in Hz)
 */

int get_clocks(void)
{
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
