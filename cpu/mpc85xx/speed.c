/*
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2003 Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ppc_asm.tmpl>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/* --------------------------------------------------------------- */

void get_sys_info (sys_info_t * sysInfo)
{
	volatile immap_t    *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint plat_ratio,e500_ratio,half_freqSystemBus;

	plat_ratio = (gur->porpllsr) & 0x0000003e;
	plat_ratio >>= 1;
	sysInfo->freqSystemBus = plat_ratio * CONFIG_SYS_CLK_FREQ;
	e500_ratio = (gur->porpllsr) & 0x003f0000;
	e500_ratio >>= 16;

	/* Divide before multiply to avoid integer
	 * overflow for processor speeds above 2GHz */
	half_freqSystemBus = sysInfo->freqSystemBus/2;
	sysInfo->freqProcessor = e500_ratio*half_freqSystemBus;
}


int get_clocks (void)
{
	sys_info_t sys_info;
#if defined(CONFIG_CPM2)
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	uint sccr, dfbrg;

	/* set VCO = 4 * BRG */
	immap->im_cpm.im_cpm_intctl.sccr &= 0xfffffffc;
	sccr = immap->im_cpm.im_cpm_intctl.sccr;
	dfbrg = (sccr & SCCR_DFBRG_MSK) >> SCCR_DFBRG_SHIFT;
#endif
	get_sys_info (&sys_info);
	gd->cpu_clk = sys_info.freqProcessor;
	gd->bus_clk = sys_info.freqSystemBus;
#if defined(CONFIG_CPM2)
	gd->vco_out = 2*sys_info.freqSystemBus;
	gd->cpm_clk = gd->vco_out / 2;
	gd->scc_clk = gd->vco_out / 4;
	gd->brg_clk = gd->vco_out / (1 << (2 * (dfbrg + 1)));
#endif

	if(gd->cpu_clk != 0) return (0);
	else return (1);
}


/********************************************
 * get_bus_freq
 * return system bus freq in Hz
 *********************************************/
ulong get_bus_freq (ulong dummy)
{
	ulong val;

	sys_info_t sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqSystemBus;

	return val;
}
