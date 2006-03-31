/*
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
#include <ppc4xx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

#define ONE_BILLION        1000000000


#if defined(CONFIG_405GP) || defined(CONFIG_405CR)

void get_sys_info (PPC405_SYS_INFO * sysInfo)
{
	unsigned long pllmr;
	unsigned long sysClkPeriodPs = ONE_BILLION / (CONFIG_SYS_CLK_FREQ / 1000);
	uint pvr = get_pvr();
	unsigned long psr;
	unsigned long m;

	/*
	 * Read PLL Mode register
	 */
	pllmr = mfdcr (pllmd);

	/*
	 * Read Pin Strapping register
	 */
	psr = mfdcr (strap);

	/*
	 * Determine FWD_DIV.
	 */
	sysInfo->pllFwdDiv = 8 - ((pllmr & PLLMR_FWD_DIV_MASK) >> 29);

	/*
	 * Determine FBK_DIV.
	 */
	sysInfo->pllFbkDiv = ((pllmr & PLLMR_FB_DIV_MASK) >> 25);
	if (sysInfo->pllFbkDiv == 0) {
		sysInfo->pllFbkDiv = 16;
	}

	/*
	 * Determine PLB_DIV.
	 */
	sysInfo->pllPlbDiv = ((pllmr & PLLMR_CPU_TO_PLB_MASK) >> 17) + 1;

	/*
	 * Determine PCI_DIV.
	 */
	sysInfo->pllPciDiv = ((pllmr & PLLMR_PCI_TO_PLB_MASK) >> 13) + 1;

	/*
	 * Determine EXTBUS_DIV.
	 */
	sysInfo->pllExtBusDiv = ((pllmr & PLLMR_EXB_TO_PLB_MASK) >> 11) + 2;

	/*
	 * Determine OPB_DIV.
	 */
	sysInfo->pllOpbDiv = ((pllmr & PLLMR_OPB_TO_PLB_MASK) >> 15) + 1;

	/*
	 * Check if PPC405GPr used (mask minor revision field)
	 */
	if ((pvr & 0xfffffff0) == (PVR_405GPR_RB & 0xfffffff0)) {
		/*
		 * Determine FWD_DIV B (only PPC405GPr with new mode strapping).
		 */
		sysInfo->pllFwdDivB = 8 - (pllmr & PLLMR_FWDB_DIV_MASK);

		/*
		 * Determine factor m depending on PLL feedback clock source
		 */
		if (!(psr & PSR_PCI_ASYNC_EN)) {
			if (psr & PSR_NEW_MODE_EN) {
				/*
				 * sync pci clock used as feedback (new mode)
				 */
				m = 1 * sysInfo->pllFwdDivB * 2 * sysInfo->pllPciDiv;
			} else {
				/*
				 * sync pci clock used as feedback (legacy mode)
				 */
				m = 1 * sysInfo->pllFwdDivB * sysInfo->pllPlbDiv * sysInfo->pllPciDiv;
			}
		} else if (psr & PSR_NEW_MODE_EN) {
			if (psr & PSR_PERCLK_SYNC_MODE_EN) {
				/*
				 * PerClk used as feedback (new mode)
				 */
				m = 1 * sysInfo->pllFwdDivB * 2 * sysInfo->pllExtBusDiv;
			} else {
				/*
				 * CPU clock used as feedback (new mode)
				 */
				m = sysInfo->pllFbkDiv * sysInfo->pllFwdDiv;
			}
		} else if (sysInfo->pllExtBusDiv == sysInfo->pllFbkDiv) {
			/*
			 * PerClk used as feedback (legacy mode)
			 */
			m = 1 * sysInfo->pllFwdDivB * sysInfo->pllPlbDiv * sysInfo->pllExtBusDiv;
		} else {
			/*
			 * PLB clock used as feedback (legacy mode)
			 */
			m = sysInfo->pllFbkDiv * sysInfo->pllFwdDivB * sysInfo->pllPlbDiv;
		}

		sysInfo->freqVCOHz = (1000000000000LL * (unsigned long long)m) /
			(unsigned long long)sysClkPeriodPs;
		sysInfo->freqProcessor = sysInfo->freqVCOHz / sysInfo->pllFwdDiv;
		sysInfo->freqPLB = sysInfo->freqVCOHz / (sysInfo->pllFwdDivB * sysInfo->pllPlbDiv);
	} else {
		/*
		 * Check pllFwdDiv to see if running in bypass mode where the CPU speed
		 * is equal to the 405GP SYS_CLK_FREQ. If not in bypass mode, check VCO
		 * to make sure it is within the proper range.
		 *    spec:    VCO = SYS_CLOCK x FBKDIV x PLBDIV x FWDDIV
		 * Note freqVCO is calculated in Mhz to avoid errors introduced by rounding.
		 */
		if (sysInfo->pllFwdDiv == 1) {
			sysInfo->freqProcessor = CONFIG_SYS_CLK_FREQ;
			sysInfo->freqPLB = CONFIG_SYS_CLK_FREQ / sysInfo->pllPlbDiv;
		} else {
			sysInfo->freqVCOHz = ( 1000000000000LL *
					       (unsigned long long)sysInfo->pllFwdDiv *
					       (unsigned long long)sysInfo->pllFbkDiv *
					       (unsigned long long)sysInfo->pllPlbDiv
				) / (unsigned long long)sysClkPeriodPs;
			sysInfo->freqPLB = (ONE_BILLION / ((sysClkPeriodPs * 10) /
							   sysInfo->pllFbkDiv)) * 10000;
			sysInfo->freqProcessor = sysInfo->freqPLB * sysInfo->pllPlbDiv;
		}
	}
}


/********************************************
 * get_OPB_freq
 * return OPB bus freq in Hz
 *********************************************/
ulong get_OPB_freq (void)
{
	ulong val = 0;

	PPC405_SYS_INFO sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB / sys_info.pllOpbDiv;

	return val;
}


/********************************************
 * get_PCI_freq
 * return PCI bus freq in Hz
 *********************************************/
ulong get_PCI_freq (void)
{
	ulong val;
	PPC405_SYS_INFO sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB / sys_info.pllPciDiv;
	return val;
}


#elif defined(CONFIG_440)

#if  defined(CONFIG_440EP) || defined(CONFIG_440GR)
void get_sys_info (sys_info_t *sysInfo)
{
	unsigned long temp;
	unsigned long reg;
	unsigned long lfdiv;
	unsigned long m;
	unsigned long prbdv0;
	/*
	  WARNING: ASSUMES the following:
	  ENG=1
	  PRADV0=1
	  PRBDV0=1
	*/

	/* Decode CPR0_PLLD0 for divisors */
	mfclk(clk_plld, reg);
	temp = (reg & PLLD_FWDVA_MASK) >> 16;
	sysInfo->pllFwdDivA = temp ? temp : 16;
	temp = (reg & PLLD_FWDVB_MASK) >> 8;
	sysInfo->pllFwdDivB = temp ? temp: 8 ;
	temp = (reg & PLLD_FBDV_MASK) >> 24;
	sysInfo->pllFbkDiv = temp ? temp : 32;
	lfdiv = reg & PLLD_LFBDV_MASK;

	mfclk(clk_opbd, reg);
	temp = (reg & OPBDDV_MASK) >> 24;
	sysInfo->pllOpbDiv = temp ? temp : 4;

	mfclk(clk_perd, reg);
	temp = (reg & PERDV_MASK) >> 24;
	sysInfo->pllExtBusDiv = temp ? temp : 8;

	mfclk(clk_primbd, reg);
	temp = (reg & PRBDV_MASK) >> 24;
	prbdv0 = temp ? temp : 8;

	mfclk(clk_spcid, reg);
	temp = (reg & SPCID_MASK) >> 24;
	sysInfo->pllPciDiv = temp ? temp : 4;

	/* Calculate 'M' based on feedback source */
	mfsdr(sdr_sdstp0, reg);
	temp = (reg & PLLSYS0_SEL_MASK) >> 27;
	if (temp == 0) { /* PLL output */
		/* Figure which pll to use */
		mfclk(clk_pllc, reg);
		temp = (reg & PLLC_SRC_MASK) >> 29;
		if (!temp) /* PLLOUTA */
			m = sysInfo->pllFbkDiv * lfdiv * sysInfo->pllFwdDivA;
		else       /* PLLOUTB */
			m = sysInfo->pllFbkDiv * lfdiv * sysInfo->pllFwdDivB;
	}
	else if (temp == 1) /* CPU output */
		m = sysInfo->pllFbkDiv * sysInfo->pllFwdDivA;
	else /* PerClk */
		m = sysInfo->pllExtBusDiv * sysInfo->pllOpbDiv * sysInfo->pllFwdDivB;

	/* Now calculate the individual clocks */
	sysInfo->freqVCOMhz = (m * CONFIG_SYS_CLK_FREQ) + (m>>1);
	sysInfo->freqProcessor = sysInfo->freqVCOMhz/sysInfo->pllFwdDivA;
	sysInfo->freqPLB = sysInfo->freqVCOMhz/sysInfo->pllFwdDivB/prbdv0;
	sysInfo->freqOPB = sysInfo->freqPLB/sysInfo->pllOpbDiv;
	sysInfo->freqEPB = sysInfo->freqPLB/sysInfo->pllExtBusDiv;
	sysInfo->freqPCI = sysInfo->freqPLB/sysInfo->pllPciDiv;

	/* Figure which timer source to use */
	if (mfspr(ccr1) & 0x0080) { /* External Clock, assume same as SYS_CLK */
		temp = sysInfo->freqProcessor / 2;  /* Max extern clock speed */
		if (CONFIG_SYS_CLK_FREQ > temp)
			sysInfo->freqTmrClk = temp;
		else
			sysInfo->freqTmrClk = CONFIG_SYS_CLK_FREQ;
	}
	else  /* Internal clock */
		sysInfo->freqTmrClk = sysInfo->freqProcessor;
}
/********************************************
 * get_PCI_freq
 * return PCI bus freq in Hz
 *********************************************/
ulong get_PCI_freq (void)
{
	sys_info_t sys_info;
	get_sys_info (&sys_info);
	return sys_info.freqPCI;
}

#elif !defined(CONFIG_440GX) && !defined(CONFIG_440SP)
void get_sys_info (sys_info_t * sysInfo)
{
	unsigned long strp0;
	unsigned long temp;
	unsigned long m;

	/* Extract configured divisors */
	strp0 = mfdcr( cpc0_strp0 );
	sysInfo->pllFwdDivA = 8 - ((strp0 & PLLSYS0_FWD_DIV_A_MASK) >> 15);
	sysInfo->pllFwdDivB = 8 - ((strp0 & PLLSYS0_FWD_DIV_B_MASK) >> 12);
	temp = (strp0 & PLLSYS0_FB_DIV_MASK) >> 18;
	sysInfo->pllFbkDiv = temp ? temp : 16;
	sysInfo->pllOpbDiv = 1 + ((strp0 & PLLSYS0_OPB_DIV_MASK) >> 10);
	sysInfo->pllExtBusDiv = 1 + ((strp0 & PLLSYS0_EPB_DIV_MASK) >> 8);

	/* Calculate 'M' based on feedback source */
	if( strp0 & PLLSYS0_EXTSL_MASK )
		m = sysInfo->pllExtBusDiv * sysInfo->pllOpbDiv * sysInfo->pllFwdDivB;
	else
		m = sysInfo->pllFbkDiv * sysInfo->pllFwdDivA;

	/* Now calculate the individual clocks */
	sysInfo->freqVCOMhz = (m * CONFIG_SYS_CLK_FREQ) + (m>>1);
	sysInfo->freqProcessor = sysInfo->freqVCOMhz/sysInfo->pllFwdDivA;
	sysInfo->freqPLB = sysInfo->freqVCOMhz/sysInfo->pllFwdDivB;
	if( get_pvr() == PVR_440GP_RB ) /* Rev B divs an extra 2 -- geez! */
		sysInfo->freqPLB >>= 1;
	sysInfo->freqOPB = sysInfo->freqPLB/sysInfo->pllOpbDiv;
	sysInfo->freqEPB = sysInfo->freqOPB/sysInfo->pllExtBusDiv;

}
#else
void get_sys_info (sys_info_t * sysInfo)
{
	unsigned long strp0;
	unsigned long strp1;
	unsigned long temp;
	unsigned long temp1;
	unsigned long lfdiv;
	unsigned long m;
	unsigned long prbdv0;

	/* Extract configured divisors */
	mfsdr( sdr_sdstp0,strp0 );
	mfsdr( sdr_sdstp1,strp1 );

	temp = ((strp0 & PLLSYS0_FWD_DIV_A_MASK) >> 8);
	sysInfo->pllFwdDivA = temp ? temp : 16 ;
	temp = ((strp0 & PLLSYS0_FWD_DIV_B_MASK) >> 5);
	sysInfo->pllFwdDivB = temp ? temp: 8 ;
	temp = (strp0 & PLLSYS0_FB_DIV_MASK) >> 12;
	sysInfo->pllFbkDiv = temp ? temp : 32;
	temp = (strp0 & PLLSYS0_OPB_DIV_MASK);
	sysInfo->pllOpbDiv = temp ? temp : 4;
	temp = (strp1 & PLLSYS1_PERCLK_DIV_MASK) >> 24;
	sysInfo->pllExtBusDiv = temp ? temp : 4;
	prbdv0 = (strp0 >> 2) & 0x7;

	/* Calculate 'M' based on feedback source */
	temp = (strp0 & PLLSYS0_SEL_MASK) >> 27;
	temp1 = (strp1 & PLLSYS1_LF_DIV_MASK) >> 26;
	lfdiv = temp1 ? temp1 : 64;
	if (temp == 0) { /* PLL output */
		/* Figure which pll to use */
		temp = (strp0 & PLLSYS0_SRC_MASK) >> 30;
		if (!temp)
			m = sysInfo->pllFbkDiv * lfdiv * sysInfo->pllFwdDivA;
		else
			m = sysInfo->pllFbkDiv * lfdiv * sysInfo->pllFwdDivB;
	}
	else if (temp == 1) /* CPU output */
		m = sysInfo->pllFbkDiv * sysInfo->pllFwdDivA;
	else /* PerClk */
		m = sysInfo->pllExtBusDiv * sysInfo->pllOpbDiv * sysInfo->pllFwdDivB;

	/* Now calculate the individual clocks */
	sysInfo->freqVCOMhz = (m * CONFIG_SYS_CLK_FREQ) + (m>>1);
	sysInfo->freqProcessor = sysInfo->freqVCOMhz/sysInfo->pllFwdDivA;
	sysInfo->freqPLB = sysInfo->freqVCOMhz/sysInfo->pllFwdDivB/prbdv0;
	sysInfo->freqOPB = sysInfo->freqPLB/sysInfo->pllOpbDiv;
	sysInfo->freqEPB = sysInfo->freqOPB/sysInfo->pllExtBusDiv;

}
#endif

ulong get_OPB_freq (void)
{

	sys_info_t sys_info;
	get_sys_info (&sys_info);
	return sys_info.freqOPB;
}

#elif defined(CONFIG_XILINX_ML300)
extern void get_sys_info (sys_info_t * sysInfo);
extern ulong get_PCI_freq (void);

#elif defined(CONFIG_AP1000)
void get_sys_info (sys_info_t * sysInfo) {
	sysInfo->freqProcessor = 240 * 1000 * 1000;
	sysInfo->freqPLB = 80 * 1000 * 1000;
	sysInfo->freqPCI = 33 * 1000 * 1000;
}

#elif defined(CONFIG_405)

void get_sys_info (sys_info_t * sysInfo) {

	sysInfo->freqVCOMhz=3125000;
	sysInfo->freqProcessor=12*1000*1000;
	sysInfo->freqPLB=50*1000*1000;
	sysInfo->freqPCI=66*1000*1000;

}

#elif defined(CONFIG_405EP)
void get_sys_info (PPC405_SYS_INFO * sysInfo)
{
	unsigned long pllmr0;
	unsigned long pllmr1;
	unsigned long sysClkPeriodPs = ONE_BILLION / (CONFIG_SYS_CLK_FREQ / 1000);
	unsigned long m;
	unsigned long pllmr0_ccdv;

	/*
	 * Read PLL Mode registers
	 */
	pllmr0 = mfdcr (cpc0_pllmr0);
	pllmr1 = mfdcr (cpc0_pllmr1);

	/*
	 * Determine forward divider A
	 */
	sysInfo->pllFwdDiv = 8 - ((pllmr1 & PLLMR1_FWDVA_MASK) >> 16);

	/*
	 * Determine forward divider B (should be equal to A)
	 */
	sysInfo->pllFwdDivB = 8 - ((pllmr1 & PLLMR1_FWDVB_MASK) >> 12);

	/*
	 * Determine FBK_DIV.
	 */
	sysInfo->pllFbkDiv = ((pllmr1 & PLLMR1_FBMUL_MASK) >> 20);
	if (sysInfo->pllFbkDiv == 0) {
		sysInfo->pllFbkDiv = 16;
	}

	/*
	 * Determine PLB_DIV.
	 */
	sysInfo->pllPlbDiv = ((pllmr0 & PLLMR0_CPU_TO_PLB_MASK) >> 16) + 1;

	/*
	 * Determine PCI_DIV.
	 */
	sysInfo->pllPciDiv = (pllmr0 & PLLMR0_PCI_TO_PLB_MASK) + 1;

	/*
	 * Determine EXTBUS_DIV.
	 */
	sysInfo->pllExtBusDiv = ((pllmr0 & PLLMR0_EXB_TO_PLB_MASK) >> 8) + 2;

	/*
	 * Determine OPB_DIV.
	 */
	sysInfo->pllOpbDiv = ((pllmr0 & PLLMR0_OPB_TO_PLB_MASK) >> 12) + 1;

	/*
	 * Determine the M factor
	 */
	m = sysInfo->pllFbkDiv * sysInfo->pllFwdDivB;

	/*
	 * Determine VCO clock frequency
	 */
	sysInfo->freqVCOHz = (1000000000000LL * (unsigned long long)m) /
		(unsigned long long)sysClkPeriodPs;

	/*
	 * Determine CPU clock frequency
	 */
	pllmr0_ccdv = ((pllmr0 & PLLMR0_CPU_DIV_MASK) >> 20) + 1;
	if (pllmr1 & PLLMR1_SSCS_MASK) {
		/*
		 * This is true if FWDVA == FWDVB:
		 * sysInfo->freqProcessor = (CONFIG_SYS_CLK_FREQ * sysInfo->pllFbkDiv)
		 *	/ pllmr0_ccdv;
		 */
		sysInfo->freqProcessor = (CONFIG_SYS_CLK_FREQ * sysInfo->pllFbkDiv * sysInfo->pllFwdDivB)
			/ sysInfo->pllFwdDiv / pllmr0_ccdv;
	} else {
		sysInfo->freqProcessor = CONFIG_SYS_CLK_FREQ / pllmr0_ccdv;
	}

	/*
	 * Determine PLB clock frequency
	 */
	sysInfo->freqPLB = sysInfo->freqProcessor / sysInfo->pllPlbDiv;
}


/********************************************
 * get_OPB_freq
 * return OPB bus freq in Hz
 *********************************************/
ulong get_OPB_freq (void)
{
	ulong val = 0;

	PPC405_SYS_INFO sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB / sys_info.pllOpbDiv;

	return val;
}


/********************************************
 * get_PCI_freq
 * return PCI bus freq in Hz
 *********************************************/
ulong get_PCI_freq (void)
{
	ulong val;
	PPC405_SYS_INFO sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB / sys_info.pllPciDiv;
	return val;
}

#endif

int get_clocks (void)
{
#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_440) || defined(CONFIG_405) || defined(CONFIG_405EP)
	sys_info_t sys_info;

	get_sys_info (&sys_info);
	gd->cpu_clk = sys_info.freqProcessor;
	gd->bus_clk = sys_info.freqPLB;

#endif	/* defined(CONFIG_405GP) || defined(CONFIG_405CR) */

#ifdef CONFIG_IOP480
	gd->cpu_clk = 66000000;
	gd->bus_clk = 66000000;
#endif
	return (0);
}


/********************************************
 * get_bus_freq
 * return PLB bus freq in Hz
 *********************************************/
ulong get_bus_freq (ulong dummy)
{
	ulong val;

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_405) || defined(CONFIG_440) || defined(CONFIG_405EP)
	sys_info_t sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB;

#elif defined(CONFIG_IOP480)

	val = 66;

#else
# error get_bus_freq() not implemented
#endif

	return val;
}
