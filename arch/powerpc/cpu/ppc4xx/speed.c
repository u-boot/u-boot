/*
 * (C) Copyright 2000-2008
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
#include <asm/ppc4xx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

#define ONE_BILLION        1000000000
#ifdef DEBUG
#define DEBUGF(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#if defined(CONFIG_405GP) || defined(CONFIG_405CR)

void get_sys_info (PPC4xx_SYS_INFO * sysInfo)
{
	unsigned long pllmr;
	unsigned long sysClkPeriodPs = ONE_BILLION / (CONFIG_SYS_CLK_FREQ / 1000);
	uint pvr = get_pvr();
	unsigned long psr;
	unsigned long m;

	/*
	 * Read PLL Mode register
	 */
	pllmr = mfdcr (CPC0_PLLMR);

	/*
	 * Read Pin Strapping register
	 */
	psr = mfdcr (CPC0_PSR);

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
		 * Note freqVCO is calculated in MHz to avoid errors introduced by rounding.
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

	sysInfo->freqOPB = sysInfo->freqPLB / sysInfo->pllOpbDiv;
	sysInfo->freqEBC = sysInfo->freqPLB / sysInfo->pllExtBusDiv;
	sysInfo->freqUART = sysInfo->freqProcessor;
}


/********************************************
 * get_PCI_freq
 * return PCI bus freq in Hz
 *********************************************/
ulong get_PCI_freq (void)
{
	ulong val;
	PPC4xx_SYS_INFO sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB / sys_info.pllPciDiv;
	return val;
}


#elif defined(CONFIG_440)

#if defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_460SX) || defined(CONFIG_APM821XX)
static u8 pll_fwdv_multi_bits[] = {
	/* values for:  1 - 16 */
	0x00, 0x01, 0x0f, 0x04, 0x09, 0x0a, 0x0d, 0x0e, 0x03, 0x0c,
	0x05, 0x08, 0x07, 0x02, 0x0b, 0x06
};

u32 get_cpr0_fwdv(unsigned long cpr_reg_fwdv)
{
	u32 index;

	for (index = 0; index < ARRAY_SIZE(pll_fwdv_multi_bits); index++)
		if (cpr_reg_fwdv == (u32)pll_fwdv_multi_bits[index])
			return index + 1;

	return 0;
}

static u8 pll_fbdv_multi_bits[] = {
	/* values for:  1 - 100 */
	0x00, 0xff, 0x7e, 0xfd, 0x7a, 0xf5, 0x6a, 0xd5, 0x2a, 0xd4,
	0x29, 0xd3, 0x26, 0xcc, 0x19, 0xb3, 0x67, 0xce, 0x1d, 0xbb,
	0x77, 0xee, 0x5d, 0xba, 0x74, 0xe9, 0x52, 0xa5, 0x4b, 0x96,
	0x2c, 0xd8, 0x31, 0xe3, 0x46, 0x8d, 0x1b, 0xb7, 0x6f, 0xde,
	0x3d, 0xfb, 0x76, 0xed, 0x5a, 0xb5, 0x6b, 0xd6, 0x2d, 0xdb,
	0x36, 0xec, 0x59, 0xb2, 0x64, 0xc9, 0x12, 0xa4, 0x48, 0x91,
	0x23, 0xc7, 0x0e, 0x9c, 0x38, 0xf0, 0x61, 0xc2, 0x05, 0x8b,
	0x17, 0xaf, 0x5f, 0xbe, 0x7c, 0xf9, 0x72, 0xe5, 0x4a, 0x95,
	0x2b, 0xd7, 0x2e, 0xdc, 0x39, 0xf3, 0x66, 0xcd, 0x1a, 0xb4,
	0x68, 0xd1, 0x22, 0xc4, 0x09, 0x93, 0x27, 0xcf, 0x1e, 0xbc,
	/* values for:  101 - 200 */
	0x78, 0xf1, 0x62, 0xc5, 0x0a, 0x94, 0x28, 0xd0, 0x21, 0xc3,
	0x06, 0x8c, 0x18, 0xb0, 0x60, 0xc1, 0x02, 0x84, 0x08, 0x90,
	0x20, 0xc0, 0x01, 0x83, 0x07, 0x8f, 0x1f, 0xbf, 0x7f, 0xfe,
	0x7d, 0xfa, 0x75, 0xea, 0x55, 0xaa, 0x54, 0xa9, 0x53, 0xa6,
	0x4c, 0x99, 0x33, 0xe7, 0x4e, 0x9d, 0x3b, 0xf7, 0x6e, 0xdd,
	0x3a, 0xf4, 0x69, 0xd2, 0x25, 0xcb, 0x16, 0xac, 0x58, 0xb1,
	0x63, 0xc6, 0x0d, 0x9b, 0x37, 0xef, 0x5e, 0xbd, 0x7b, 0xf6,
	0x6d, 0xda, 0x35, 0xeb, 0x56, 0xad, 0x5b, 0xb6, 0x6c, 0xd9,
	0x32, 0xe4, 0x49, 0x92, 0x24, 0xc8, 0x11, 0xa3, 0x47, 0x8e,
	0x1c, 0xb8, 0x70, 0xe1, 0x42, 0x85, 0x0b, 0x97, 0x2f, 0xdf,
	/* values for:  201 - 255 */
	0x3e, 0xfc, 0x79, 0xf2, 0x65, 0xca, 0x15, 0xab, 0x57, 0xae,
	0x5c, 0xb9, 0x73, 0xe6, 0x4d, 0x9a, 0x34, 0xe8, 0x51, 0xa2,
	0x44, 0x89, 0x13, 0xa7, 0x4f, 0x9e, 0x3c, 0xf8, 0x71, 0xe2,
	0x45, 0x8a, 0x14, 0xa8, 0x50, 0xa1, 0x43, 0x86, 0x0c, 0x98,
	0x30, 0xe0, 0x41, 0x82, 0x04, 0x88, 0x10, 0xa0, 0x40, 0x81,
	0x03, 0x87, 0x0f, 0x9f, 0x3f  /* END */
};

u32 get_cpr0_fbdv(unsigned long cpr_reg_fbdv)
{
	u32 index;

	for (index = 0; index < ARRAY_SIZE(pll_fbdv_multi_bits); index++)
		if (cpr_reg_fbdv == (u32)pll_fbdv_multi_bits[index])
			return index + 1;

	return 0;
}

#if defined(CONFIG_APM821XX)

void get_sys_info(sys_info_t *sysInfo)
{
	unsigned long plld;
	unsigned long temp;
	unsigned long mul;
	unsigned long cpudv;
	unsigned long plb2dv;
	unsigned long ddr2dv;

	/* Calculate Forward divisor A and Feeback divisor */
	mfcpr(CPR0_PLLD, plld);

	temp = CPR0_PLLD_FWDVA(plld);
	sysInfo->pllFwdDivA = get_cpr0_fwdv(temp);

	temp = CPR0_PLLD_FDV(plld);
	sysInfo->pllFbkDiv = get_cpr0_fbdv(temp);

	/* Calculate OPB clock divisor */
	mfcpr(CPR0_OPBD, temp);
	temp = CPR0_OPBD_OPBDV(temp);
	sysInfo->pllOpbDiv = temp ? temp : 4;

	/* Calculate Peripheral clock divisor */
	mfcpr(CPR0_PERD, temp);
	temp = CPR0_PERD_PERDV(temp);
	sysInfo->pllExtBusDiv = temp ? temp : 4;

	/* Calculate CPU clock divisor */
	mfcpr(CPR0_CPUD, temp);
	temp = CPR0_CPUD_CPUDV(temp);
	cpudv = temp ? temp : 8;

	/* Calculate PLB2 clock divisor */
	mfcpr(CPR0_PLB2D, temp);
	temp = CPR0_PLB2D_PLB2DV(temp);
	plb2dv = temp ? temp : 4;

	/* Calculate DDR2 clock divisor */
	mfcpr(CPR0_DDR2D, temp);
	temp = CPR0_DDR2D_DDR2DV(temp);
	ddr2dv = temp ? temp : 4;

	/* Calculate 'M' based on feedback source */
	mfcpr(CPR0_PLLC, temp);
	temp = CPR0_PLLC_SEL(temp);
	if (temp == 0) {
		/* PLL internal feedback */
		mul = sysInfo->pllFbkDiv;
	} else {
		/* PLL PerClk feedback */
		mul = sysInfo->pllFwdDivA * sysInfo->pllFbkDiv * cpudv
			* plb2dv * 2 * sysInfo->pllOpbDiv *
			  sysInfo->pllExtBusDiv;
	}

	/* Now calculate the individual clocks */
	sysInfo->freqVCOMhz = (mul * CONFIG_SYS_CLK_FREQ) + (mul >> 1);
	sysInfo->freqProcessor = sysInfo->freqVCOMhz /
		sysInfo->pllFwdDivA / cpudv;
	sysInfo->freqPLB = sysInfo->freqVCOMhz /
		sysInfo->pllFwdDivA / cpudv / plb2dv / 2;
	sysInfo->freqOPB = sysInfo->freqPLB / sysInfo->pllOpbDiv;
	sysInfo->freqEBC = sysInfo->freqOPB / sysInfo->pllExtBusDiv;
	sysInfo->freqDDR = sysInfo->freqVCOMhz /
		sysInfo->pllFwdDivA / cpudv / ddr2dv / 2;
	sysInfo->freqUART = sysInfo->freqPLB;
}

#else
/*
 * AMCC_TODO: verify this routine against latest EAS, cause stuff changed
 *            with latest EAS
 */
void get_sys_info (sys_info_t * sysInfo)
{
	unsigned long strp0;
	unsigned long strp1;
	unsigned long temp;
	unsigned long m;
	unsigned long plbedv0;

	/* Extract configured divisors */
	mfsdr(SDR0_SDSTP0, strp0);
	mfsdr(SDR0_SDSTP1, strp1);

	temp = ((strp0 & PLLSYS0_FWD_DIV_A_MASK) >> 4);
	sysInfo->pllFwdDivA = get_cpr0_fwdv(temp);

	temp = (strp0 & PLLSYS0_FWD_DIV_B_MASK);
	sysInfo->pllFwdDivB = get_cpr0_fwdv(temp);

	temp = (strp0 & PLLSYS0_FB_DIV_MASK) >> 8;
	sysInfo->pllFbkDiv = get_cpr0_fbdv(temp);

	temp = (strp1 & PLLSYS0_OPB_DIV_MASK) >> 26;
	sysInfo->pllOpbDiv = temp ? temp : 4;

	/* AMCC_TODO: verify the SDR0_SDSTP1.PERDV0 value sysInfo->pllExtBusDiv */
	temp = (strp1 & PLLSYS0_PERCLK_DIV_MASK) >> 24;
	sysInfo->pllExtBusDiv = temp ? temp : 4;

	temp = (strp1 & PLLSYS0_PLBEDV0_DIV_MASK) >> 29;
	plbedv0 = temp ? temp: 8;

	/* Calculate 'M' based on feedback source */
	temp = (strp0 & PLLSYS0_SEL_MASK) >> 27;
	if (temp == 0) {
		/* PLL internal feedback */
		m = sysInfo->pllFbkDiv;
	} else {
		/* PLL PerClk feedback */
		m = sysInfo->pllFwdDivA * plbedv0 * sysInfo->pllOpbDiv *
			sysInfo->pllExtBusDiv;
	}

	/* Now calculate the individual clocks */
	sysInfo->freqVCOMhz = (m * CONFIG_SYS_CLK_FREQ) + (m >> 1);
	sysInfo->freqProcessor = sysInfo->freqVCOMhz/sysInfo->pllFwdDivA;
	sysInfo->freqPLB = sysInfo->freqVCOMhz / sysInfo->pllFwdDivA / plbedv0;
	sysInfo->freqOPB = sysInfo->freqPLB / sysInfo->pllOpbDiv;
	sysInfo->freqEBC = sysInfo->freqOPB / sysInfo->pllExtBusDiv;
	sysInfo->freqDDR = sysInfo->freqPLB;
	sysInfo->freqUART = sysInfo->freqPLB;

	return;
}
#endif

#elif defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
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
	mfcpr(CPR0_PLLD, reg);
	temp = (reg & PLLD_FWDVA_MASK) >> 16;
	sysInfo->pllFwdDivA = temp ? temp : 16;
	temp = (reg & PLLD_FWDVB_MASK) >> 8;
	sysInfo->pllFwdDivB = temp ? temp: 8 ;
	temp = (reg & PLLD_FBDV_MASK) >> 24;
	sysInfo->pllFbkDiv = temp ? temp : 32;
	lfdiv = reg & PLLD_LFBDV_MASK;

	mfcpr(CPR0_OPBD0, reg);
	temp = (reg & OPBDDV_MASK) >> 24;
	sysInfo->pllOpbDiv = temp ? temp : 4;

	mfcpr(CPR0_PERD, reg);
	temp = (reg & PERDV_MASK) >> 24;
	sysInfo->pllExtBusDiv = temp ? temp : 8;

	mfcpr(CPR0_PRIMBD0, reg);
	temp = (reg & PRBDV_MASK) >> 24;
	prbdv0 = temp ? temp : 8;

	mfcpr(CPR0_SPCID, reg);
	temp = (reg & SPCID_MASK) >> 24;
	sysInfo->pllPciDiv = temp ? temp : 4;

	/* Calculate 'M' based on feedback source */
	mfsdr(SDR0_SDSTP0, reg);
	temp = (reg & PLLSYS0_SEL_MASK) >> 27;
	if (temp == 0) { /* PLL output */
		/* Figure which pll to use */
		mfcpr(CPR0_PLLC, reg);
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
	sysInfo->freqEBC = sysInfo->freqPLB/sysInfo->pllExtBusDiv;
	sysInfo->freqPCI = sysInfo->freqPLB/sysInfo->pllPciDiv;
	sysInfo->freqUART = sysInfo->freqPLB;

	/* Figure which timer source to use */
	if (mfspr(SPRN_CCR1) & 0x0080) {
		/* External Clock, assume same as SYS_CLK */
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

#elif !defined(CONFIG_440GX) && !defined(CONFIG_440SP) && !defined(CONFIG_440SPE) \
	&& !defined(CONFIG_XILINX_440)
void get_sys_info (sys_info_t * sysInfo)
{
	unsigned long strp0;
	unsigned long temp;
	unsigned long m;

	/* Extract configured divisors */
	strp0 = mfdcr( CPC0_STRP0 );
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
	sysInfo->freqEBC = sysInfo->freqOPB/sysInfo->pllExtBusDiv;
	sysInfo->freqUART = sysInfo->freqPLB;
}
#else

#if !defined(CONFIG_XILINX_440)
void get_sys_info (sys_info_t * sysInfo)
{
	unsigned long strp0;
	unsigned long strp1;
	unsigned long temp;
	unsigned long temp1;
	unsigned long lfdiv;
	unsigned long m;
	unsigned long prbdv0;

#if defined(CONFIG_YUCCA)
	unsigned long sys_freq;
	unsigned long sys_per=0;
	unsigned long msr;
	unsigned long pci_clock_per;
	unsigned long sdr_ddrpll;

	/*-------------------------------------------------------------------------+
	 | Get the system clock period.
	 +-------------------------------------------------------------------------*/
	sys_per = determine_sysper();

	msr = (mfmsr () & ~(MSR_EE));	/* disable interrupts */

	/*-------------------------------------------------------------------------+
	 | Calculate the system clock speed from the period.
	 +-------------------------------------------------------------------------*/
	sys_freq = (ONE_BILLION / sys_per) * 1000;
#endif

	/* Extract configured divisors */
	mfsdr( SDR0_SDSTP0,strp0 );
	mfsdr( SDR0_SDSTP1,strp1 );

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
#if defined(CONFIG_YUCCA)
	sysInfo->freqVCOMhz = (m * sys_freq) ;
#else
	sysInfo->freqVCOMhz = (m * CONFIG_SYS_CLK_FREQ) + (m >> 1);
#endif
	sysInfo->freqProcessor = sysInfo->freqVCOMhz/sysInfo->pllFwdDivA;
	sysInfo->freqPLB = sysInfo->freqVCOMhz/sysInfo->pllFwdDivB/prbdv0;
	sysInfo->freqOPB = sysInfo->freqPLB/sysInfo->pllOpbDiv;
	sysInfo->freqEBC = sysInfo->freqOPB/sysInfo->pllExtBusDiv;

#if defined(CONFIG_YUCCA)
	/* Determine PCI Clock Period */
	pci_clock_per = determine_pci_clock_per();
	sysInfo->freqPCI = (ONE_BILLION/pci_clock_per) * 1000;
	mfsdr(SDR0_DDR0, sdr_ddrpll);
	sysInfo->freqDDR = ((sysInfo->freqPLB) * SDR0_DDR0_DDRM_DECODE(sdr_ddrpll));
#endif

	sysInfo->freqUART = sysInfo->freqPLB;
}

#endif
#endif /* CONFIG_XILINX_440 */

#if defined(CONFIG_YUCCA)
unsigned long determine_sysper(void)
{
	unsigned int fpga_clocking_reg;
	unsigned int master_clock_selection;
	unsigned long master_clock_per = 0;
	unsigned long fb_div_selection;
	unsigned int vco_div_reg_value;
	unsigned long vco_div_selection;
	unsigned long sys_per = 0;
	int extClkVal;

	/*-------------------------------------------------------------------------+
	 | Read FPGA reg 0 and reg 1 to get FPGA reg information
	 +-------------------------------------------------------------------------*/
	fpga_clocking_reg = in16(FPGA_REG16);


	/* Determine Master Clock Source Selection */
	master_clock_selection = fpga_clocking_reg & FPGA_REG16_MASTER_CLK_MASK;

	switch(master_clock_selection) {
		case FPGA_REG16_MASTER_CLK_66_66:
			master_clock_per = PERIOD_66_66MHZ;
			break;
		case FPGA_REG16_MASTER_CLK_50:
			master_clock_per = PERIOD_50_00MHZ;
			break;
		case FPGA_REG16_MASTER_CLK_33_33:
			master_clock_per = PERIOD_33_33MHZ;
			break;
		case FPGA_REG16_MASTER_CLK_25:
			master_clock_per = PERIOD_25_00MHZ;
			break;
		case FPGA_REG16_MASTER_CLK_EXT:
			if ((extClkVal==EXTCLK_33_33)
					&& (extClkVal==EXTCLK_50)
					&& (extClkVal==EXTCLK_66_66)
					&& (extClkVal==EXTCLK_83)) {
				/* calculate master clock period from external clock value */
				master_clock_per=(ONE_BILLION/extClkVal) * 1000;
			} else {
				/* Unsupported */
				DEBUGF ("%s[%d] *** master clock selection failed ***\n", __FUNCTION__,__LINE__);
				hang();
			}
			break;
		default:
			/* Unsupported */
			DEBUGF ("%s[%d] *** master clock selection failed ***\n", __FUNCTION__,__LINE__);
			hang();
			break;
	}

	/* Determine FB divisors values */
	if ((fpga_clocking_reg & FPGA_REG16_FB1_DIV_MASK) == FPGA_REG16_FB1_DIV_LOW) {
		if ((fpga_clocking_reg & FPGA_REG16_FB2_DIV_MASK) == FPGA_REG16_FB2_DIV_LOW)
			fb_div_selection = FPGA_FB_DIV_6;
		else
			fb_div_selection = FPGA_FB_DIV_12;
	} else {
		if ((fpga_clocking_reg & FPGA_REG16_FB2_DIV_MASK) == FPGA_REG16_FB2_DIV_LOW)
			fb_div_selection = FPGA_FB_DIV_10;
		else
			fb_div_selection = FPGA_FB_DIV_20;
	}

	/* Determine VCO divisors values */
	vco_div_reg_value = fpga_clocking_reg & FPGA_REG16_VCO_DIV_MASK;

	switch(vco_div_reg_value) {
		case FPGA_REG16_VCO_DIV_4:
			vco_div_selection = FPGA_VCO_DIV_4;
			break;
		case FPGA_REG16_VCO_DIV_6:
			vco_div_selection = FPGA_VCO_DIV_6;
			break;
		case FPGA_REG16_VCO_DIV_8:
			vco_div_selection = FPGA_VCO_DIV_8;
			break;
		case FPGA_REG16_VCO_DIV_10:
		default:
			vco_div_selection = FPGA_VCO_DIV_10;
			break;
	}

	if (master_clock_selection == FPGA_REG16_MASTER_CLK_EXT) {
		switch(master_clock_per) {
			case PERIOD_25_00MHZ:
				if (fb_div_selection == FPGA_FB_DIV_12) {
					if (vco_div_selection == FPGA_VCO_DIV_4)
						sys_per = PERIOD_75_00MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_6)
						sys_per = PERIOD_50_00MHZ;
				}
				break;
			case PERIOD_33_33MHZ:
				if (fb_div_selection == FPGA_FB_DIV_6) {
					if (vco_div_selection == FPGA_VCO_DIV_4)
						sys_per = PERIOD_50_00MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_6)
						sys_per = PERIOD_33_33MHZ;
				}
				if (fb_div_selection == FPGA_FB_DIV_10) {
					if (vco_div_selection == FPGA_VCO_DIV_4)
						sys_per = PERIOD_83_33MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_10)
						sys_per = PERIOD_33_33MHZ;
				}
				if (fb_div_selection == FPGA_FB_DIV_12) {
					if (vco_div_selection == FPGA_VCO_DIV_4)
						sys_per = PERIOD_100_00MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_6)
						sys_per = PERIOD_66_66MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_8)
						sys_per = PERIOD_50_00MHZ;
				}
				break;
			case PERIOD_50_00MHZ:
				if (fb_div_selection == FPGA_FB_DIV_6) {
					if (vco_div_selection == FPGA_VCO_DIV_4)
						sys_per = PERIOD_75_00MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_6)
						sys_per = PERIOD_50_00MHZ;
				}
				if (fb_div_selection == FPGA_FB_DIV_10) {
					if (vco_div_selection == FPGA_VCO_DIV_6)
						sys_per = PERIOD_83_33MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_10)
						sys_per = PERIOD_50_00MHZ;
				}
				if (fb_div_selection == FPGA_FB_DIV_12) {
					if (vco_div_selection == FPGA_VCO_DIV_6)
						sys_per = PERIOD_100_00MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_8)
						sys_per = PERIOD_75_00MHZ;
				}
				break;
			case PERIOD_66_66MHZ:
				if (fb_div_selection == FPGA_FB_DIV_6) {
					if (vco_div_selection == FPGA_VCO_DIV_4)
						sys_per = PERIOD_100_00MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_6)
						sys_per = PERIOD_66_66MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_8)
						sys_per = PERIOD_50_00MHZ;
				}
				if (fb_div_selection == FPGA_FB_DIV_10) {
					if (vco_div_selection == FPGA_VCO_DIV_8)
						sys_per = PERIOD_83_33MHZ;
					if (vco_div_selection == FPGA_VCO_DIV_10)
						sys_per = PERIOD_66_66MHZ;
				}
				if (fb_div_selection == FPGA_FB_DIV_12) {
					if (vco_div_selection == FPGA_VCO_DIV_8)
						sys_per = PERIOD_100_00MHZ;
				}
				break;
			default:
				break;
		}

		if (sys_per == 0) {
			/* Other combinations are not supported */
			DEBUGF ("%s[%d] *** sys period compute failed ***\n", __FUNCTION__,__LINE__);
			hang();
		}
	} else {
		/* calcul system clock without cheking */
		/* if engineering option clock no check is selected */
		/* sys_per = master_clock_per * vco_div_selection / fb_div_selection */
		sys_per = (master_clock_per/fb_div_selection) * vco_div_selection;
	}

	return(sys_per);
}

/*-------------------------------------------------------------------------+
| determine_pci_clock_per.
+-------------------------------------------------------------------------*/
unsigned long determine_pci_clock_per(void)
{
	unsigned long pci_clock_selection,  pci_period;

	/*-------------------------------------------------------------------------+
	 | Read FPGA reg 6 to get PCI 0 FPGA reg information
	 +-------------------------------------------------------------------------*/
	pci_clock_selection = in16(FPGA_REG16);	/* was reg6 averifier */


	pci_clock_selection = pci_clock_selection & FPGA_REG16_PCI0_CLK_MASK;

	switch (pci_clock_selection) {
		case FPGA_REG16_PCI0_CLK_133_33:
			pci_period = PERIOD_133_33MHZ;
			break;
		case FPGA_REG16_PCI0_CLK_100:
			pci_period = PERIOD_100_00MHZ;
			break;
		case FPGA_REG16_PCI0_CLK_66_66:
			pci_period = PERIOD_66_66MHZ;
			break;
		default:
			pci_period = PERIOD_33_33MHZ;;
			break;
	}

	return(pci_period);
}
#endif

#elif defined(CONFIG_XILINX_405)
extern void get_sys_info (sys_info_t * sysInfo);
extern ulong get_PCI_freq (void);

#elif defined(CONFIG_405)

void get_sys_info (sys_info_t * sysInfo)
{
	sysInfo->freqVCOMhz=3125000;
	sysInfo->freqProcessor=12*1000*1000;
	sysInfo->freqPLB=50*1000*1000;
	sysInfo->freqPCI=66*1000*1000;
}

#elif defined(CONFIG_405EP)
void get_sys_info (PPC4xx_SYS_INFO * sysInfo)
{
	unsigned long pllmr0;
	unsigned long pllmr1;
	unsigned long sysClkPeriodPs = ONE_BILLION / (CONFIG_SYS_CLK_FREQ / 1000);
	unsigned long m;
	unsigned long pllmr0_ccdv;

	/*
	 * Read PLL Mode registers
	 */
	pllmr0 = mfdcr (CPC0_PLLMR0);
	pllmr1 = mfdcr (CPC0_PLLMR1);

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
	if (sysInfo->pllFbkDiv == 0)
		sysInfo->pllFbkDiv = 16;

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

	sysInfo->freqEBC = sysInfo->freqPLB / sysInfo->pllExtBusDiv;

	sysInfo->freqOPB = sysInfo->freqPLB / sysInfo->pllOpbDiv;

	sysInfo->freqUART = sysInfo->freqProcessor * pllmr0_ccdv;
}


/********************************************
 * get_PCI_freq
 * return PCI bus freq in Hz
 *********************************************/
ulong get_PCI_freq (void)
{
	ulong val;
	PPC4xx_SYS_INFO sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB / sys_info.pllPciDiv;
	return val;
}

#elif defined(CONFIG_405EZ)
void get_sys_info (PPC4xx_SYS_INFO * sysInfo)
{
	unsigned long cpr_plld;
	unsigned long cpr_pllc;
	unsigned long cpr_primad;
	unsigned long sysClkPeriodPs = ONE_BILLION / (CONFIG_SYS_CLK_FREQ/1000);
	unsigned long primad_cpudv;
	unsigned long m;
	unsigned long plloutb;

	/*
	 * Read PLL Mode registers
	 */
	mfcpr(CPR0_PLLD, cpr_plld);
	mfcpr(CPR0_PLLC, cpr_pllc);

	/*
	 * Determine forward divider A
	 */
	sysInfo->pllFwdDiv = ((cpr_plld & PLLD_FWDVA_MASK) >> 16);

	/*
	 * Determine forward divider B
	 */
	sysInfo->pllFwdDivB = ((cpr_plld & PLLD_FWDVB_MASK) >> 8);
	if (sysInfo->pllFwdDivB == 0)
		sysInfo->pllFwdDivB = 8;

	/*
	 * Determine FBK_DIV.
	 */
	sysInfo->pllFbkDiv = ((cpr_plld & PLLD_FBDV_MASK) >> 24);
	if (sysInfo->pllFbkDiv == 0)
		sysInfo->pllFbkDiv = 256;

	/*
	 * Read CPR_PRIMAD register
	 */
	mfcpr(CPR0_PRIMAD, cpr_primad);

	/*
	 * Determine PLB_DIV.
	 */
	sysInfo->pllPlbDiv = ((cpr_primad & PRIMAD_PLBDV_MASK) >> 16);
	if (sysInfo->pllPlbDiv == 0)
		sysInfo->pllPlbDiv = 16;

	/*
	 * Determine EXTBUS_DIV.
	 */
	sysInfo->pllExtBusDiv = (cpr_primad & PRIMAD_EBCDV_MASK);
	if (sysInfo->pllExtBusDiv == 0)
		sysInfo->pllExtBusDiv = 16;

	/*
	 * Determine OPB_DIV.
	 */
	sysInfo->pllOpbDiv = ((cpr_primad & PRIMAD_OPBDV_MASK) >> 8);
	if (sysInfo->pllOpbDiv == 0)
		sysInfo->pllOpbDiv = 16;

	/*
	 * Determine the M factor
	 */
	if (cpr_pllc & PLLC_SRC_MASK)
		m = sysInfo->pllFbkDiv * sysInfo->pllFwdDivB;
	else
		m = sysInfo->pllFbkDiv * sysInfo->pllFwdDiv;

	/*
	 * Determine VCO clock frequency
	 */
	sysInfo->freqVCOHz = (1000000000000LL * (unsigned long long)m) /
		(unsigned long long)sysClkPeriodPs;

	/*
	 * Determine CPU clock frequency
	 */
	primad_cpudv = ((cpr_primad & PRIMAD_CPUDV_MASK) >> 24);
	if (primad_cpudv == 0)
		primad_cpudv = 16;

	sysInfo->freqProcessor = (CONFIG_SYS_CLK_FREQ * m) /
		sysInfo->pllFwdDiv / primad_cpudv;

	/*
	 * Determine PLB clock frequency
	 */
	sysInfo->freqPLB = (CONFIG_SYS_CLK_FREQ * m) /
		sysInfo->pllFwdDiv / sysInfo->pllPlbDiv;

	sysInfo->freqOPB = (CONFIG_SYS_CLK_FREQ * sysInfo->pllFbkDiv) /
		sysInfo->pllOpbDiv;

	sysInfo->freqEBC = (CONFIG_SYS_CLK_FREQ * sysInfo->pllFbkDiv) /
		sysInfo->pllExtBusDiv;

	plloutb = ((CONFIG_SYS_CLK_FREQ * ((cpr_pllc & PLLC_SRC_MASK) ?
		sysInfo->pllFwdDivB : sysInfo->pllFwdDiv) * sysInfo->pllFbkDiv) /
		sysInfo->pllFwdDivB);
	sysInfo->freqUART = plloutb;
}

#elif defined(CONFIG_405EX)

/*
 * TODO: We need to get the CPR registers and calculate these values correctly!!!!
 *   We need the specs!!!!
 */
static unsigned char get_fbdv(unsigned char index)
{
	unsigned char ret = 0;
	/* This is table should be 256 bytes.
	 * Only take first 52 values.
	 */
	unsigned char fbdv_tb[] = {
		0x00, 0xff, 0x7f, 0xfd,
		0x7a, 0xf5, 0x6a, 0xd5,
		0x2a, 0xd4, 0x29, 0xd3,
		0x26, 0xcc, 0x19, 0xb3,
		0x67, 0xce, 0x1d, 0xbb,
		0x77, 0xee, 0x5d, 0xba,
		0x74, 0xe9, 0x52, 0xa5,
		0x4b, 0x96, 0x2c, 0xd8,
		0x31, 0xe3, 0x46, 0x8d,
		0x1b, 0xb7, 0x6f, 0xde,
		0x3d, 0xfb, 0x76, 0xed,
		0x5a, 0xb5, 0x6b, 0xd6,
		0x2d, 0xdb, 0x36, 0xec,

	};

	if ((index & 0x7f) == 0)
		return 1;
	while (ret < sizeof (fbdv_tb)) {
		if (fbdv_tb[ret] == index)
			break;
		ret++;
	}
	ret++;

	return ret;
}

#define PLL_FBK_PLL_LOCAL	0
#define PLL_FBK_CPU		1
#define PLL_FBK_PERCLK		5

void get_sys_info (sys_info_t * sysInfo)
{
	unsigned long sysClkPeriodPs = ONE_BILLION / (CONFIG_SYS_CLK_FREQ / 1000);
	unsigned long m = 1;
	unsigned int  tmp;
	unsigned char fwdva[16] = {
		1, 2, 14, 9, 4, 11, 16, 13,
		12, 5, 6, 15, 10, 7, 8, 3,
	};
	unsigned char sel, cpudv0, plb2xDiv;

	mfcpr(CPR0_PLLD, tmp);

	/*
	 * Determine forward divider A
	 */
	sysInfo->pllFwdDiv = fwdva[((tmp >> 16) & 0x0f)];	/* FWDVA */

	/*
	 * Determine FBK_DIV.
	 */
	sysInfo->pllFbkDiv = get_fbdv(((tmp >> 24) & 0x0ff)); /* FBDV */

	/*
	 * Determine PLBDV0
	 */
	sysInfo->pllPlbDiv = 2;

	/*
	 * Determine PERDV0
	 */
	mfcpr(CPR0_PERD, tmp);
	tmp = (tmp >> 24) & 0x03;
	sysInfo->pllExtBusDiv = (tmp == 0) ? 4 : tmp;

	/*
	 * Determine OPBDV0
	 */
	mfcpr(CPR0_OPBD0, tmp);
	tmp = (tmp >> 24) & 0x03;
	sysInfo->pllOpbDiv = (tmp == 0) ? 4 : tmp;

	/* Determine PLB2XDV0 */
	mfcpr(CPR0_PLBD, tmp);
	tmp = (tmp >> 16) & 0x07;
	plb2xDiv = (tmp == 0) ? 8 : tmp;

	/* Determine CPUDV0 */
	mfcpr(CPR0_CPUD, tmp);
	tmp = (tmp >> 24) & 0x07;
	cpudv0 = (tmp == 0) ? 8 : tmp;

	/* Determine SEL(5:7) in CPR0_PLLC */
	mfcpr(CPR0_PLLC, tmp);
	sel = (tmp >> 24) & 0x07;

	/*
	 * Determine the M factor
	 * PLL local: M = FBDV
	 * CPU clock: M = FBDV * FWDVA * CPUDV0
	 * PerClk	: M = FBDV * FWDVA * PLB2XDV0 * PLBDV0(2) * OPBDV0 * PERDV0
	 *
	 */
	switch (sel) {
	case PLL_FBK_CPU:
		m = sysInfo->pllFwdDiv * cpudv0;
		break;
	case PLL_FBK_PERCLK:
		m = sysInfo->pllFwdDiv * plb2xDiv * 2
			* sysInfo->pllOpbDiv * sysInfo->pllExtBusDiv;
		break;
	case PLL_FBK_PLL_LOCAL:
		break;
	default:
		printf("%s unknown m\n", __FUNCTION__);
		return;

	}
	m *= sysInfo->pllFbkDiv;

	/*
	 * Determine VCO clock frequency
	 */
	sysInfo->freqVCOHz = (1000000000000LL * (unsigned long long)m) /
		(unsigned long long)sysClkPeriodPs;

	/*
	 * Determine CPU clock frequency
	 */
	sysInfo->freqProcessor = sysInfo->freqVCOHz / (sysInfo->pllFwdDiv * cpudv0);

	/*
	 * Determine PLB clock frequency, ddr1x should be the same
	 */
	sysInfo->freqPLB = sysInfo->freqVCOHz / (sysInfo->pllFwdDiv * plb2xDiv * 2);
	sysInfo->freqOPB = sysInfo->freqPLB/sysInfo->pllOpbDiv;
	sysInfo->freqDDR = sysInfo->freqPLB;
	sysInfo->freqEBC = sysInfo->freqOPB / sysInfo->pllExtBusDiv;
	sysInfo->freqUART = sysInfo->freqPLB;
}

#endif

int get_clocks (void)
{
	sys_info_t sys_info;

	get_sys_info (&sys_info);
	gd->cpu_clk = sys_info.freqProcessor;
	gd->bus_clk = sys_info.freqPLB;

	return (0);
}


/********************************************
 * get_bus_freq
 * return PLB bus freq in Hz
 *********************************************/
ulong get_bus_freq (ulong dummy)
{
	ulong val;

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || \
    defined(CONFIG_405EP) || defined(CONFIG_405EZ) || \
    defined(CONFIG_405EX) || defined(CONFIG_405) || \
    defined(CONFIG_440)
	sys_info_t sys_info;

	get_sys_info (&sys_info);
	val = sys_info.freqPLB;
#else
# error get_bus_freq() not implemented
#endif

	return val;
}

ulong get_OPB_freq (void)
{
	PPC4xx_SYS_INFO sys_info;

	get_sys_info (&sys_info);

	return sys_info.freqOPB;
}
