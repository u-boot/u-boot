/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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
#include <asm/arch/omap2420.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/clocks.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>

/************************************************************
 * sdelay() - simple spin loop.  Will be constant time as
 *  its generally used in 12MHz bypass conditions only.  This
 *  is necessary until timers are accessible.
 *
 *  not inline to increase chances its in cache when called
 *************************************************************/
void sdelay (unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

/*********************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h (config II default).
 *   -- called from SRAM, or Flash (using temp SRAM stack).
 *********************************************************************************/
void prcm_init(void)
{
	u32 div;
	void (*f_lock_pll) (u32, u32, u32, u32);
	extern void *_end_vect, *_start;

	f_lock_pll = (void *)((u32)&_end_vect - (u32)&_start + SRAM_VECT_CODE);

	__raw_writel(0, CM_FCLKEN1_CORE);	   /* stop all clocks to reduce ringing */
	__raw_writel(0, CM_FCLKEN2_CORE);	   /* may not be necessary */
	__raw_writel(0, CM_ICLKEN1_CORE);
	__raw_writel(0, CM_ICLKEN2_CORE);

	__raw_writel(DPLL_OUT, CM_CLKSEL2_PLL);	/* set DPLL out */
	__raw_writel(MPU_DIV, CM_CLKSEL_MPU);	/* set MPU divider */
	__raw_writel(DSP_DIV, CM_CLKSEL_DSP);	/* set dsp and iva dividers */
	__raw_writel(GFX_DIV, CM_CLKSEL_GFX);	/* set gfx dividers */

	div = BUS_DIV;
	__raw_writel(div, CM_CLKSEL1_CORE);/* set L3/L4/USB/Display/Vlnc/SSi dividers */
	sdelay(1000);

	if(running_in_sram()){
		/* If running fully from SRAM this is OK.  The Flash bus drops out for just a little.
		* but then comes back.  If running from Flash this sequence kills you, thus you need
		* to run it using CONFIG_PARTIAL_SRAM.
		*/
		__raw_writel(MODE_BYPASS_FAST, CM_CLKEN_PLL); /* go to bypass, fast relock */
		wait_on_value(BIT0|BIT1, BIT0, CM_IDLEST_CKGEN, LDELAY); /* wait till in bypass */
		sdelay(1000);
		/* set clock selection and dpll dividers. */
		__raw_writel(DPLL_VAL, CM_CLKSEL1_PLL);	 /* set pll for target rate */
		__raw_writel(COMMIT_DIVIDERS, PRCM_CLKCFG_CTRL); /* commit dividers */
		sdelay(10000);
		__raw_writel(DPLL_LOCK, CM_CLKEN_PLL); /* enable dpll */
		sdelay(10000);
		wait_on_value(BIT0|BIT1, BIT1, CM_IDLEST_CKGEN, LDELAY);  /*wait for dpll lock */
	}else if(running_in_flash()){
		/* if running from flash, need to jump to small relocated code area in SRAM.
		 * This is the only safe spot to do configurations from.
		 */
		(*f_lock_pll)(PRCM_CLKCFG_CTRL, CM_CLKEN_PLL, DPLL_LOCK, CM_IDLEST_CKGEN);
	}

	__raw_writel(DPLL_LOCK|APLL_LOCK, CM_CLKEN_PLL);   /* enable apll */
	wait_on_value(BIT8, BIT8, CM_IDLEST_CKGEN, LDELAY);	/* wait for apll lock */
	sdelay(1000);
}

/**************************************************************************
 * make_cs1_contiguous() - for es2 and above remap cs1 behind cs0 to allow
 *  command line mem=xyz use all memory with out discontigious support
 *  compiled in.  Could do it at the ATAG, but there really is two banks...
 * Called as part of 2nd phase DDR init.
 **************************************************************************/
void make_cs1_contiguous(void)
{
	u32 size, a_add_low, a_add_high;

	size = get_sdr_cs_size(SDRC_CS0_OSET);
	size /= SZ_32M;  /* find size to offset CS1 */
	a_add_high = (size & 3) << 8;   /* set up low field */
	a_add_low = (size & 0x3C) >> 2; /* set up high field */
	__raw_writel((a_add_high|a_add_low),SDRC_CS_CFG);

}

/********************************************************
 *  mem_ok() - test used to see if timings are correct
 *             for a part. Helps in gussing which part
 *             we are currently using.
 *******************************************************/
u32 mem_ok(void)
{
	u32 val1, val2;
	u32 pattern = 0x12345678;

	__raw_writel(0x0,OMAP2420_SDRC_CS0+0x400);   /* clear pos A */
	__raw_writel(pattern, OMAP2420_SDRC_CS0);    /* pattern to pos B */
	__raw_writel(0x0,OMAP2420_SDRC_CS0+4);       /* remove pattern off the bus */
	val1 = __raw_readl(OMAP2420_SDRC_CS0+0x400); /* get pos A value */
	val2 = __raw_readl(OMAP2420_SDRC_CS0);       /* get val2 */

	if ((val1 != 0) || (val2 != pattern))        /* see if pos A value changed*/
		return(0);
	else
		return(1);
}


/********************************************************
 *  sdrc_init() - init the sdrc chip selects CS0 and CS1
 *  - early init routines, called from flash or
 *  SRAM.
 *******************************************************/
void sdrc_init(void)
{
	#define EARLY_INIT 1
	do_sdrc_init(SDRC_CS0_OSET, EARLY_INIT);  /* only init up first bank here */
}

/*************************************************************************
 * do_sdrc_init(): initialize the SDRAM for use.
 *  -called from low level code with stack only.
 *  -code sets up SDRAM timing and muxing for 2422 or 2420.
 *  -optimal settings can be placed here, or redone after i2c
 *      inspection of board info
 *
 *  This is a bit ugly, but should handle all memory moduels
 *   used with the H4. The first time though this code from s_init()
 *   we configure the first chip select.  Later on we come back and
 *   will configure the 2nd chip select if it exists.
 *
 **************************************************************************/
void do_sdrc_init(u32 offset, u32 early)
{
	u32 cpu, dllen=0, rev, common=0, cs0=0, pmask=0, pass_type, mtype;
	sdrc_data_t *sdata;	 /* do not change type */
	u32 a, b, r;

	static const sdrc_data_t sdrc_2422 =
	{
		H4_2422_SDRC_SHARING, H4_2422_SDRC_MDCFG_0_DDR, 0 , H4_2422_SDRC_ACTIM_CTRLA_0,
		H4_2422_SDRC_ACTIM_CTRLB_0, H4_2422_SDRC_RFR_CTRL, H4_2422_SDRC_MR_0_DDR,
		0, H4_2422_SDRC_DLLAB_CTRL
	};
	static const sdrc_data_t sdrc_2420 =
	{
		H4_2420_SDRC_SHARING, H4_2420_SDRC_MDCFG_0_DDR, H4_2420_SDRC_MDCFG_0_SDR,
		H4_2420_SDRC_ACTIM_CTRLA_0, H4_2420_SDRC_ACTIM_CTRLB_0,
		H4_2420_SDRC_RFR_CTRL, H4_2420_SDRC_MR_0_DDR, H4_2420_SDRC_MR_0_SDR,
		H4_2420_SDRC_DLLAB_CTRL
	};

	if (offset == SDRC_CS0_OSET)
		cs0 = common = 1;  /* int regs shared between both chip select */

	cpu = get_cpu_type();
	rev = get_cpu_rev();

	/* warning generated, though code generation is correct. this may bite later,
	 * but is ok for now. there is only so much C code you can do on stack only
	 * operation.
	 */
	if (cpu == CPU_2422){
		sdata = (sdrc_data_t *)&sdrc_2422;
		pass_type = STACKED;
	} else{
		sdata = (sdrc_data_t *)&sdrc_2420;
		pass_type = IP_DDR;
	}

	__asm__ __volatile__("": : :"memory");  /* limit compiler scope */

	/* u-boot is compiled to run in DDR or SRAM at 8xxxxxxx or 4xxxxxxx.
	 * If we are running in flash prior to relocation and we use data
	 * here which is not pc relative we need to get the address correct.
	 * We need to find the current flash mapping to dress up the initial
	 * pointer load.  As long as this is const data we should be ok.
	 */
	if((early) && running_in_flash()){
		sdata = (sdrc_data_t *)(((u32)sdata & 0x0003FFFF) | get_gpmc0_base());
		/* NOR internal boot offset is 0x4000 from xloader signature */
		if(running_from_internal_boot())
			sdata = (sdrc_data_t *)((u32)sdata + 0x4000);
	}

	if (!early && (((mtype = get_mem_type()) == DDR_COMBO)||(mtype == DDR_STACKED))) {
		if(mtype == DDR_COMBO){
			pmask = BIT2;/* combo part has a shared CKE signal, can't use feature */
			pass_type = COMBO_DDR; /* CS1 config */
			__raw_writel((__raw_readl(SDRC_POWER)) & ~pmask, SDRC_POWER);
		}
		if(rev != CPU_2420_2422_ES1)	/* for es2 and above smooth things out */
			make_cs1_contiguous();
	}

next_mem_type:
	if (common) {	/* do a SDRC reset between types to clear regs*/
		__raw_writel(SOFTRESET, SDRC_SYSCONFIG);	/* reset sdrc */
		wait_on_value(BIT0, BIT0, SDRC_STATUS, 12000000);/* wait till reset done set */
		__raw_writel(0, SDRC_SYSCONFIG);		/* clear soft reset */
		__raw_writel(sdata->sdrc_sharing, SDRC_SHARING);
#ifdef POWER_SAVE
		__raw_writel(__raw_readl(SMS_SYSCONFIG)|SMART_IDLE, SMS_SYSCONFIG);
		__raw_writel(sdata->sdrc_sharing|SMART_IDLE, SDRC_SHARING);
		__raw_writel((__raw_readl(SDRC_POWER)|BIT6), SDRC_POWER);
#endif
	}

	if ((pass_type == IP_DDR) || (pass_type == STACKED)) /* (IP ddr-CS0),(2422-CS0/CS1) */
		__raw_writel(sdata->sdrc_mdcfg_0_ddr, SDRC_MCFG_0+offset);
	else if (pass_type == COMBO_DDR){ /* (combo-CS0/CS1) */
		__raw_writel(H4_2420_COMBO_MDCFG_0_DDR,SDRC_MCFG_0+offset);
	} else if (pass_type == IP_SDR){ /* ip sdr-CS0 */
		__raw_writel(sdata->sdrc_mdcfg_0_sdr, SDRC_MCFG_0+offset);
	}

	a = sdata->sdrc_actim_ctrla_0;
	b = sdata->sdrc_actim_ctrlb_0;
	r = sdata->sdrc_dllab_ctrl;

	/* work around ES1 DDR issues */
	if((pass_type != IP_SDR) && (rev == CPU_2420_2422_ES1)){
		a = H4_242x_SDRC_ACTIM_CTRLA_0_ES1;
		b = H4_242x_SDRC_ACTIM_CTRLB_0_ES1;
		r = H4_242x_SDRC_RFR_CTRL_ES1;
 	}

	if (cs0) {
		__raw_writel(a, SDRC_ACTIM_CTRLA_0);
		__raw_writel(b, SDRC_ACTIM_CTRLB_0);
	} else {
		__raw_writel(a, SDRC_ACTIM_CTRLA_1);
		__raw_writel(b, SDRC_ACTIM_CTRLB_1);
	}
	__raw_writel(r, SDRC_RFR_CTRL+offset);

	/* init sequence for mDDR/mSDR using manual commands (DDR is a bit different) */
	__raw_writel(CMD_NOP, SDRC_MANUAL_0+offset);
	sdelay(5000);  /* susposed to be 100us per design spec for mddr/msdr */
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0+offset);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0+offset);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0+offset);

	/*
	 * CSx SDRC Mode Register
	 * Burst length = (4 - DDR) (2-SDR)
	 * Serial mode
	 * CAS latency = x
	 */
	if(pass_type == IP_SDR)
		__raw_writel(sdata->sdrc_mr_0_sdr, SDRC_MR_0+offset);
	else
		__raw_writel(sdata->sdrc_mr_0_ddr, SDRC_MR_0+offset);

	/* NOTE: ES1 242x _BUG_ DLL + External Bandwidth fix*/
	if (rev == CPU_2420_2422_ES1){
		dllen = (BIT0|BIT3); /* es1 clear both bit0 and bit3 */
		__raw_writel((__raw_readl(SMS_CLASS_ARB0)|BURSTCOMPLETE_GROUP7)
			,SMS_CLASS_ARB0);/* enable bust complete for lcd */
	}
	else
		dllen = BIT0|BIT1; /* es2, clear bit0, and 1 (set phase to 72) */

	/* enable & load up DLL with good value for 75MHz, and set phase to 90
	 * ES1 recommends 90 phase, ES2 recommends 72 phase.
	 */
	if (common && (pass_type != IP_SDR)) {
		__raw_writel(sdata->sdrc_dllab_ctrl, SDRC_DLLA_CTRL);
		__raw_writel(sdata->sdrc_dllab_ctrl & ~(BIT2|dllen), SDRC_DLLA_CTRL);
		__raw_writel(sdata->sdrc_dllab_ctrl, SDRC_DLLB_CTRL);
		__raw_writel(sdata->sdrc_dllab_ctrl & ~(BIT2|dllen) , SDRC_DLLB_CTRL);
	}
	sdelay(90000);

	if(mem_ok())
		return; /* STACKED, other configued type */
	++pass_type; /* IPDDR->COMBODDR->IPSDR for CS0 */
	goto next_mem_type;
}

/*****************************************************
 * gpmc_init(): init gpmc bus
 * Init GPMC for x16, MuxMode (SDRAM in x32).
 * This code can only be executed from SRAM or SDRAM.
 *****************************************************/
void gpmc_init(void)
{
	u32 mux=0, mtype, mwidth, rev, tval;

	rev  = get_cpu_rev();
	if (rev == CPU_2420_2422_ES1)
		tval = 1;
	else
		tval = 0;  /* disable bit switched meaning */

	/* global settings */
	__raw_writel(0x10, GPMC_SYSCONFIG);	/* smart idle */
	__raw_writel(0x0, GPMC_IRQENABLE);	/* isr's sources masked */
	__raw_writel(tval, GPMC_TIMEOUT_CONTROL);/* timeout disable */
#ifdef CFG_NAND_BOOT
	__raw_writel(0x001, GPMC_CONFIG);	/* set nWP, disable limited addr */
#else
	__raw_writel(0x111, GPMC_CONFIG);	/* set nWP, disable limited addr */
#endif

	/* discover bus connection from sysboot */
	if (is_gpmc_muxed() == GPMC_MUXED)
		mux = BIT9;
	mtype = get_gpmc0_type();
	mwidth = get_gpmc0_width();

	/* setup cs0 */
	__raw_writel(0x0, GPMC_CONFIG7_0);	/* disable current map */
	sdelay(1000);

#ifdef CFG_NAND_BOOT
	__raw_writel(H4_24XX_GPMC_CONFIG1_0|mtype|mwidth, GPMC_CONFIG1_0);
#else
	__raw_writel(H4_24XX_GPMC_CONFIG1_0|mux|mtype|mwidth, GPMC_CONFIG1_0);
#endif

#ifdef PRCM_CONFIG_III
	__raw_writel(H4_24XX_GPMC_CONFIG2_0, GPMC_CONFIG2_0);
#endif
	__raw_writel(H4_24XX_GPMC_CONFIG3_0, GPMC_CONFIG3_0);
	__raw_writel(H4_24XX_GPMC_CONFIG4_0, GPMC_CONFIG4_0);
#ifdef PRCM_CONFIG_III
	__raw_writel(H4_24XX_GPMC_CONFIG5_0, GPMC_CONFIG5_0);
	__raw_writel(H4_24XX_GPMC_CONFIG6_0, GPMC_CONFIG6_0);
#endif
	__raw_writel(H4_24XX_GPMC_CONFIG7_0, GPMC_CONFIG7_0);/* enable new mapping */
	sdelay(2000);

	/* setup cs1 */
	__raw_writel(0, GPMC_CONFIG7_1); /* disable any mapping */
	sdelay(1000);
	__raw_writel(H4_24XX_GPMC_CONFIG1_1|mux, GPMC_CONFIG1_1);
	__raw_writel(H4_24XX_GPMC_CONFIG2_1, GPMC_CONFIG2_1);
	__raw_writel(H4_24XX_GPMC_CONFIG3_1, GPMC_CONFIG3_1);
	__raw_writel(H4_24XX_GPMC_CONFIG4_1, GPMC_CONFIG4_1);
	__raw_writel(H4_24XX_GPMC_CONFIG5_1, GPMC_CONFIG5_1);
	__raw_writel(H4_24XX_GPMC_CONFIG6_1, GPMC_CONFIG6_1);
	__raw_writel(H4_24XX_GPMC_CONFIG7_1, GPMC_CONFIG7_1); /* enable mapping */
	sdelay(2000);
}
