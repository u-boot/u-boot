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

/* ------------------------------------------------------------------------- */

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
	if ((pvr & 0xfffffff0) == (PVR_405GPR_RA & 0xfffffff0)) {
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

		sysInfo->freqVCOMhz = (1000000 * m) / sysClkPeriodPs;
		sysInfo->freqProcessor = (sysInfo->freqVCOMhz * 1000000) / sysInfo->pllFwdDiv;
		sysInfo->freqPLB = (sysInfo->freqVCOMhz * 1000000) /
			(sysInfo->pllFwdDivB * sysInfo->pllPlbDiv);
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
			sysInfo->freqVCOMhz = ( 1000000 *
						sysInfo->pllFwdDiv *
						sysInfo->pllFbkDiv *
						sysInfo->pllPlbDiv
				) / sysClkPeriodPs;
			if (sysInfo->freqVCOMhz >= VCO_MIN
			    && sysInfo->freqVCOMhz <= VCO_MAX) {
				sysInfo->freqPLB = (ONE_BILLION /
						    ((sysClkPeriodPs * 10) /
						     sysInfo->pllFbkDiv)) * 10000;
				sysInfo->freqProcessor = sysInfo->freqPLB * sysInfo->pllPlbDiv;
			} else {
				printf ("\nInvalid VCO frequency calculated :  %ld MHz \a\n",
					sysInfo->freqVCOMhz);
				printf ("It must be between %d-%d MHz \a\n",
					VCO_MIN, VCO_MAX);
				printf ("PLL Mode reg           :  %8.8lx\a\n",
					pllmr);
				hang ();
			}
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

ulong get_OPB_freq (void)
{

	sys_info_t sys_info;
	get_sys_info (&sys_info);
	return sys_info.freqOPB;
}

#elif defined(CONFIG_405)

void get_sys_info (sys_info_t * sysInfo) {

	sysInfo->freqVCOMhz=3125000;
	sysInfo->freqProcessor=12*1000*1000;
	sysInfo->freqPLB=50*1000*1000;
	sysInfo->freqPCI=66*1000*1000;

}

#endif

int get_clocks (void)
{
#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_440) || defined(CONFIG_405)
	DECLARE_GLOBAL_DATA_PTR;

	sys_info_t sys_info;

	get_sys_info (&sys_info);
	gd->cpu_clk = sys_info.freqProcessor;
	gd->bus_clk = sys_info.freqPLB;

#endif	/* defined(CONFIG_405GP) || defined(CONFIG_405CR) */

#ifdef CONFIG_IOP480
	DECLARE_GLOBAL_DATA_PTR;

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

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_405) || defined(CONFIG_440)
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
