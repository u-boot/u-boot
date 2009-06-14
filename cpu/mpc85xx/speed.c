/*
 * Copyright 2004, 2007-2009 Freescale Semiconductor Inc.
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
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* --------------------------------------------------------------- */

void get_sys_info (sys_info_t * sysInfo)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	uint plat_ratio,e500_ratio,half_freqSystemBus;
	uint lcrr_div;
	int i;
#ifdef CONFIG_QE
	u32 qe_ratio;
#endif

	plat_ratio = (gur->porpllsr) & 0x0000003e;
	plat_ratio >>= 1;
	sysInfo->freqSystemBus = plat_ratio * CONFIG_SYS_CLK_FREQ;

	/* Divide before multiply to avoid integer
	 * overflow for processor speeds above 2GHz */
	half_freqSystemBus = sysInfo->freqSystemBus/2;
	for (i = 0; i < CONFIG_NUM_CPUS; i++) {
		e500_ratio = ((gur->porpllsr) >> (i * 8 + 16)) & 0x3f;
		sysInfo->freqProcessor[i] = e500_ratio * half_freqSystemBus;
	}

	/* Note: freqDDRBus is the MCLK frequency, not the data rate. */
	sysInfo->freqDDRBus = sysInfo->freqSystemBus;

#ifdef CONFIG_DDR_CLK_FREQ
	{
		u32 ddr_ratio = ((gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO)
			>> MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
		if (ddr_ratio != 0x7)
			sysInfo->freqDDRBus = ddr_ratio * CONFIG_DDR_CLK_FREQ;
	}
#endif

#ifdef CONFIG_QE
	qe_ratio = ((gur->porpllsr) & MPC85xx_PORPLLSR_QE_RATIO)
			>> MPC85xx_PORPLLSR_QE_RATIO_SHIFT;
	sysInfo->freqQE = qe_ratio * CONFIG_SYS_CLK_FREQ;
#endif

#if defined(CONFIG_SYS_LBC_LCRR)
	/* We will program LCRR to this value later */
	lcrr_div = CONFIG_SYS_LBC_LCRR & LCRR_CLKDIV;
#else
	{
	    volatile ccsr_lbc_t *lbc = (void *)(CONFIG_SYS_MPC85xx_LBC_ADDR);
	    lcrr_div = in_be32(&lbc->lcrr) & LCRR_CLKDIV;
	}
#endif
	if (lcrr_div == 2 || lcrr_div == 4 || lcrr_div == 8) {
#if !defined(CONFIG_MPC8540) && !defined(CONFIG_MPC8541) && \
    !defined(CONFIG_MPC8555) && !defined(CONFIG_MPC8560)
		/*
		 * Yes, the entire PQ38 family use the same
		 * bit-representation for twice the clock divider values.
		 */
		lcrr_div *= 2;
#endif
		sysInfo->freqLocalBus = sysInfo->freqSystemBus / lcrr_div;
	} else {
		/* In case anyone cares what the unknown value is */
		sysInfo->freqLocalBus = lcrr_div;
	}
}


int get_clocks (void)
{
	sys_info_t sys_info;
#ifdef CONFIG_MPC8544
	volatile ccsr_gur_t *gur = (void *) CONFIG_SYS_MPC85xx_GUTS_ADDR;
#endif
#if defined(CONFIG_CPM2)
	volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
	uint sccr, dfbrg;

	/* set VCO = 4 * BRG */
	cpm->im_cpm_intctl.sccr &= 0xfffffffc;
	sccr = cpm->im_cpm_intctl.sccr;
	dfbrg = (sccr & SCCR_DFBRG_MSK) >> SCCR_DFBRG_SHIFT;
#endif
	get_sys_info (&sys_info);
	gd->cpu_clk = sys_info.freqProcessor[0];
	gd->bus_clk = sys_info.freqSystemBus;
	gd->mem_clk = sys_info.freqDDRBus;
	gd->lbc_clk = sys_info.freqLocalBus;

#ifdef CONFIG_QE
	gd->qe_clk = sys_info.freqQE;
	gd->brg_clk = gd->qe_clk / 2;
#endif
	/*
	 * The base clock for I2C depends on the actual SOC.  Unfortunately,
	 * there is no pattern that can be used to determine the frequency, so
	 * the only choice is to look up the actual SOC number and use the value
	 * for that SOC. This information is taken from application note
	 * AN2919.
	 */
#if defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
	defined(CONFIG_MPC8560) || defined(CONFIG_MPC8555)
	gd->i2c1_clk = sys_info.freqSystemBus;
#elif defined(CONFIG_MPC8544)
	/*
	 * On the 8544, the I2C clock is the same as the SEC clock.  This can be
	 * either CCB/2 or CCB/3, depending on the value of cfg_sec_freq. See
	 * 4.4.3.3 of the 8544 RM.  Note that this might actually work for all
	 * 85xx, but only the 8544 has cfg_sec_freq, so it's unknown if the
	 * PORDEVSR2_SEC_CFG bit is 0 on all 85xx boards that are not an 8544.
	 */
	if (gur->pordevsr2 & MPC85xx_PORDEVSR2_SEC_CFG)
		gd->i2c1_clk = sys_info.freqSystemBus / 3;
	else
		gd->i2c1_clk = sys_info.freqSystemBus / 2;
#else
	/* Most 85xx SOCs use CCB/2, so this is the default behavior. */
	gd->i2c1_clk = sys_info.freqSystemBus / 2;
#endif
	gd->i2c2_clk = gd->i2c1_clk;

#if defined(CONFIG_MPC8536)
	gd->sdhc_clk = gd->bus_clk / 2;
#endif

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
	return gd->bus_clk;
}

/********************************************
 * get_ddr_freq
 * return ddr bus freq in Hz
 *********************************************/
ulong get_ddr_freq (ulong dummy)
{
	return gd->mem_clk;
}
