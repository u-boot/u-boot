/*
 * (C) Copyright 2002
 * Custom IDEAS, Inc. <www.cideas.com>
 * Jon Diekema <diekema@cideas.com>
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
#include <ioports.h>
#include <mpc8260.h>
#include <asm/cpm_8260.h>
#include <configs/sacsng.h>

#include "clkinit.h"

int Daq64xSampling = 0;


void Daq_BRG_Reset(uint brg)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     volatile uint *brg_ptr;

     brg_ptr = (uint *)&immr->im_brgc1;

     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;
     *brg_ptr |=  CPM_BRG_RST;
     *brg_ptr &= ~CPM_BRG_RST;
}

void Daq_BRG_Disable(uint brg)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     volatile uint *brg_ptr;

     brg_ptr = (uint *)&immr->im_brgc1;

     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;
     *brg_ptr &= ~CPM_BRG_EN;
}

void Daq_BRG_Enable(uint brg)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     volatile uint *brg_ptr;

     brg_ptr = (uint *)&immr->im_brgc1;
     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;
     *brg_ptr |= CPM_BRG_EN;
}

uint Daq_BRG_Get_Div16(uint brg)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     uint *brg_ptr;

     brg_ptr = (uint *)&immr->im_brgc1;
     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;

     if (*brg_ptr & CPM_BRG_DIV16) {
	 /* DIV16 active */
	 return (TRUE);
     }
     else {
	 /* DIV16 inactive */
	 return (FALSE);
     }
}

void Daq_BRG_Set_Div16(uint brg, uint div16)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     uint *brg_ptr;

     brg_ptr = (uint *)&immr->im_brgc1;
     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;

     if (div16) {
	 /* DIV16 active */
	 *brg_ptr |=  CPM_BRG_DIV16;
     }
     else {
	 /* DIV16 inactive */
	 *brg_ptr &= ~CPM_BRG_DIV16;
     }
}

uint Daq_BRG_Get_Count(uint brg)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     uint *brg_ptr;
     uint brg_cnt;

     brg_ptr = (uint *)&immr->im_brgc1;
     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;

     /* Get the clock divider
      *
      * Note: A clock divider of 0 means divide by 1,
      *       therefore we need to add 1 to the count.
      */
     brg_cnt = (*brg_ptr & CPM_BRG_CD_MASK) >> CPM_BRG_DIV16_SHIFT;
     brg_cnt++;
     if (*brg_ptr & CPM_BRG_DIV16) {
	 brg_cnt *= 16;
     }

    return (brg_cnt);
}

void Daq_BRG_Set_Count(uint brg, uint brg_cnt)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     uint *brg_ptr;

     brg_ptr = (uint *)&immr->im_brgc1;
     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;

     /*
      * Note: A clock divider of 0 means divide by 1,
      *	 therefore we need to subtract 1 from the count.
      */
     if (brg_cnt > 4096) {
	 /* Prescale = Divide by 16 */
	 *brg_ptr = (*brg_ptr & ~CPM_BRG_CD_MASK)   |
	     (((brg_cnt / 16) - 1) << CPM_BRG_DIV16_SHIFT);
	 *brg_ptr |= CPM_BRG_DIV16;
     }
     else {
	 /* Prescale = Divide by 1 */
	 *brg_ptr = (*brg_ptr & ~CPM_BRG_CD_MASK) |
	     ((brg_cnt - 1) << CPM_BRG_DIV16_SHIFT);
	 *brg_ptr &= ~CPM_BRG_DIV16;
     }
}

uint Daq_BRG_Get_ExtClk(uint brg)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     uint *brg_ptr;

     brg_ptr = (uint *)&immr->im_brgc1;
     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;

     return ((*brg_ptr & CPM_BRG_EXTC_MASK) >> CPM_BRG_EXTC_SHIFT);
}

char* Daq_BRG_Get_ExtClk_Description(uint brg)
{
     uint extc;

     extc = Daq_BRG_Get_ExtClk(brg);

     switch (brg + 1) {
	 case 1:
	 case 2:
	 case 5:
	 case 6: {
	     switch (extc) {
		 case 0: {
		     return ("BRG_INT");
		 }
		 case 1: {
		     return ("CLK3");
		 }
		 case 2: {
		     return ("CLK5");
		 }
	     }
	     return ("??1245??");
	 }
	 case 3:
	 case 4:
	 case 7:
	 case 8: {
	     switch (extc) {
		 case 0: {
		     return ("BRG_INT");
		 }
		 case 1: {
		     return ("CLK9");
		 }
		 case 2: {
		     return ("CLK15");
		 }
	     }
	     return ("??3478??");
	 }
     }
     return ("??9876??");
}

void Daq_BRG_Set_ExtClk(uint brg, uint extc)
{
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     uint *brg_ptr;

     brg_ptr = (uint *)&immr->im_brgc1;
     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg -= 4;
     }
     brg_ptr += brg;

     *brg_ptr = (*brg_ptr & ~CPM_BRG_EXTC_MASK) |
		((extc << CPM_BRG_EXTC_SHIFT) & CPM_BRG_EXTC_MASK);
}

uint Daq_BRG_Rate(uint brg)
{
     DECLARE_GLOBAL_DATA_PTR;
     volatile immap_t *immr = (immap_t *)CFG_IMMR;
     uint *brg_ptr;
     uint brg_cnt;
     uint brg_freq = 0;

     brg_ptr = (uint *)&immr->im_brgc1;
     brg_ptr += brg;
     if (brg >= 5) {
	 brg_ptr = (uint *)&immr->im_brgc5;
	 brg_ptr += (brg - 4);
     }

    brg_cnt = Daq_BRG_Get_Count(brg);

    switch (Daq_BRG_Get_ExtClk(brg)) {
	case CPM_BRG_EXTC_CLK3:
	case CPM_BRG_EXTC_CLK5: {
	    brg_freq = brg_cnt;
	    break;
	}
	default: {
	    brg_freq = (uint)BRG_INT_CLK / brg_cnt;
	}
    }
    return (brg_freq);
}

uint Daq_Get_SampleRate(void)
{
     /*
      * Read the BRG's to return the actual sample rate.
      */
     return (Daq_BRG_Rate(MCLK_BRG) / (MCLK_DIVISOR * SCLK_DIVISOR));
}

void Daq_Init_Clocks(int sample_rate, int sample_64x)
{
    DECLARE_GLOBAL_DATA_PTR;
    volatile ioport_t *iopa = ioport_addr((immap_t *)CFG_IMMR, 0 /* port A */);
    uint mclk_divisor; /* MCLK divisor */
    int  flag;         /* Interrupt state */

    /* Save off the clocking data */
    Daq64xSampling = sample_64x;

    /*
     * Limit the sample rate to some sensible values.
     */
    if (sample_rate > MAX_64x_SAMPLE_RATE) {
	sample_rate = MAX_64x_SAMPLE_RATE;
    }
    if (sample_rate < MIN_SAMPLE_RATE) {
	sample_rate = MIN_SAMPLE_RATE;
    }

    /*
     * Initialize the MCLK/SCLK/LRCLK baud rate generators.
     */

    /* Setup MCLK */
    Daq_BRG_Set_ExtClk(MCLK_BRG, CPM_BRG_EXTC_BRGCLK);

    /* Setup SCLK */
#   ifdef RUN_SCLK_ON_BRG_INT
	Daq_BRG_Set_ExtClk(SCLK_BRG, CPM_BRG_EXTC_BRGCLK);
#   else
	Daq_BRG_Set_ExtClk(SCLK_BRG, CPM_BRG_EXTC_CLK9);
#   endif

    /* Setup LRCLK */
#   ifdef RUN_LRCLK_ON_BRG_INT
	Daq_BRG_Set_ExtClk(LRCLK_BRG, CPM_BRG_EXTC_BRGCLK);
#   else
	Daq_BRG_Set_ExtClk(LRCLK_BRG, CPM_BRG_EXTC_CLK5);
#   endif

    /*
     * Dynamically adjust MCLK based on the new sample rate.
     */

    /* Compute the divisors */
    mclk_divisor = BRG_INT_CLK / (sample_rate * MCLK_DIVISOR * SCLK_DIVISOR);

    /*
     * Disable interrupt and save the current state
     */
    flag = disable_interrupts();

    /* Setup MCLK */
    Daq_BRG_Set_Count(MCLK_BRG, mclk_divisor);

    /* Setup SCLK */
#   ifdef RUN_SCLK_ON_BRG_INT
	Daq_BRG_Set_Count(SCLK_BRG, mclk_divisor * MCLK_DIVISOR);
#   else
	Daq_BRG_Set_Count(SCLK_BRG, MCLK_DIVISOR);
#   endif

#   ifdef RUN_LRCLK_ON_BRG_INT
	Daq_BRG_Set_Count(LRCLK_BRG,
			  mclk_divisor * MCLK_DIVISOR * SCLK_DIVISOR);
#   else
	Daq_BRG_Set_Count(LRCLK_BRG, SCLK_DIVISOR);
#   endif

    /*
     * Restore the Interrupt state
     */
     if (flag) {
	 enable_interrupts();
     }

    /* Enable the clock drivers */
    iopa->pdat &= ~SLRCLK_EN_MASK;
}

void Daq_Stop_Clocks(void)

{
#ifdef TIGHTEN_UP_BRG_TIMING
    volatile immap_t *immr = (immap_t *)CFG_IMMR;
    register uint mclk_brg;       /* MCLK  BRG value */
    register uint sclk_brg;       /* SCLK  BRG value */
    register uint lrclk_brg;      /* LRCLK BRG value */
    unsigned long flag;           /* Interrupt flags */
#endif

#   ifdef TIGHTEN_UP_BRG_TIMING
	/*
	 * Obtain MCLK BRG reset/disabled value
	 */
#       if (MCLK_BRG == 0)
	    mclk_brg = (*IM_BRGC1 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 1)
	    mclk_brg = (*IM_BRGC2 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 2)
	    mclk_brg = (*IM_BRGC3 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 3)
	    mclk_brg = (*IM_BRGC4 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 4)
	    mclk_brg = (*IM_BRGC5 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 5)
	    mclk_brg = (*IM_BRGC6 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 6)
	    mclk_brg = (*IM_BRGC7 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 7)
	    mclk_brg = (*IM_BRGC8 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif

	/*
	 * Obtain SCLK BRG reset/disabled value
	 */
#       if (SCLK_BRG == 0)
	    sclk_brg = (*IM_BRGC1 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 1)
	    sclk_brg = (*IM_BRGC2 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 2)
	    sclk_brg = (*IM_BRGC3 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 3)
	    sclk_brg = (*IM_BRGC4 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 4)
	    sclk_brg = (*IM_BRGC5 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 5)
	    sclk_brg = (*IM_BRGC6 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 6)
	    sclk_brg = (*IM_BRGC7 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 7)
	    sclk_brg = (*IM_BRGC8 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif

	/*
	 * Obtain LRCLK BRG reset/disabled value
	 */
#       if (LRCLK_BRG == 0)
	    lrclk_brg = (*IM_BRGC1 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 1)
	    lrclk_brg = (*IM_BRGC2 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 2)
	    lrclk_brg = (*IM_BRGC3 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 3)
	    lrclk_brg = (*IM_BRGC4 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 4)
	    lrclk_brg = (*IM_BRGC5 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 5)
	    lrclk_brg = (*IM_BRGC6 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 6)
	    lrclk_brg = (*IM_BRGC7 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 7)
	    lrclk_brg = (*IM_BRGC8 | CPM_BRG_RST) & ~CPM_BRG_EN;
#       endif

	/*
	 * Disable interrupt and save the current state
	 */
	flag = disable_interrupts();

	/*
	 * Set reset on MCLK BRG
	 */
#       if (MCLK_BRG == 0)
	    *IM_BRGC1 = mclk_brg;
#       endif
#       if (MCLK_BRG == 1)
	    *IM_BRGC2 = mclk_brg;
#       endif
#       if (MCLK_BRG == 2)
	    *IM_BRGC3 = mclk_brg;
#       endif
#       if (MCLK_BRG == 3)
	    *IM_BRGC4 = mclk_brg;
#       endif
#       if (MCLK_BRG == 4)
	    *IM_BRGC5 = mclk_brg;
#       endif
#       if (MCLK_BRG == 5)
	    *IM_BRGC6 = mclk_brg;
#       endif
#       if (MCLK_BRG == 6)
	    *IM_BRGC7 = mclk_brg;
#       endif
#       if (MCLK_BRG == 7)
	    *IM_BRGC8 = mclk_brg;
#       endif

	/*
	 * Set reset on SCLK BRG
	 */
#       if (SCLK_BRG == 0)
	    *IM_BRGC1 = sclk_brg;
#       endif
#       if (SCLK_BRG == 1)
	    *IM_BRGC2 = sclk_brg;
#       endif
#       if (SCLK_BRG == 2)
	    *IM_BRGC3 = sclk_brg;
#       endif
#       if (SCLK_BRG == 3)
	    *IM_BRGC4 = sclk_brg;
#       endif
#       if (SCLK_BRG == 4)
	    *IM_BRGC5 = sclk_brg;
#       endif
#       if (SCLK_BRG == 5)
	    *IM_BRGC6 = sclk_brg;
#       endif
#       if (SCLK_BRG == 6)
	    *IM_BRGC7 = sclk_brg;
#       endif
#       if (SCLK_BRG == 7)
	    *IM_BRGC8 = sclk_brg;
#       endif

	/*
	 * Set reset on LRCLK BRG
	 */
#       if (LRCLK_BRG == 0)
	    *IM_BRGC1 = lrclk_brg;
#       endif
#       if (LRCLK_BRG == 1)
	    *IM_BRGC2 = lrclk_brg;
#       endif
#       if (LRCLK_BRG == 2)
	    *IM_BRGC3 = lrclk_brg;
#       endif
#       if (LRCLK_BRG == 3)
	    *IM_BRGC4 = lrclk_brg;
#       endif
#       if (LRCLK_BRG == 4)
	    *IM_BRGC5 = lrclk_brg;
#       endif
#       if (LRCLK_BRG == 5)
	    *IM_BRGC6 = lrclk_brg;
#       endif
#       if (LRCLK_BRG == 6)
	    *IM_BRGC7 = lrclk_brg;
#       endif
#       if (LRCLK_BRG == 7)
	    *IM_BRGC8 = lrclk_brg;
#       endif

	/*
	 * Clear reset on MCLK BRG
	 */
#       if (MCLK_BRG == 0)
	    *IM_BRGC1 = mclk_brg & ~CPM_BRG_RST;
#       endif
#       if (MCLK_BRG == 1)
	    *IM_BRGC2 = mclk_brg & ~CPM_BRG_RST;
#       endif
#       if (MCLK_BRG == 2)
	    *IM_BRGC3 = mclk_brg & ~CPM_BRG_RST;
#       endif
#       if (MCLK_BRG == 3)
	    *IM_BRGC4 = mclk_brg & ~CPM_BRG_RST;
#       endif
#       if (MCLK_BRG == 4)
	    *IM_BRGC5 = mclk_brg & ~CPM_BRG_RST;
#       endif
#       if (MCLK_BRG == 5)
	    *IM_BRGC6 = mclk_brg & ~CPM_BRG_RST;
#       endif
#       if (MCLK_BRG == 6)
	    *IM_BRGC7 = mclk_brg & ~CPM_BRG_RST;
#       endif
#       if (MCLK_BRG == 7)
	    *IM_BRGC8 = mclk_brg & ~CPM_BRG_RST;
#       endif

	/*
	 * Clear reset on SCLK BRG
	 */
#       if (SCLK_BRG == 0)
	    *IM_BRGC1 = sclk_brg & ~CPM_BRG_RST;
#       endif
#       if (SCLK_BRG == 1)
	    *IM_BRGC2 = sclk_brg & ~CPM_BRG_RST;
#       endif
#       if (SCLK_BRG == 2)
	    *IM_BRGC3 = sclk_brg & ~CPM_BRG_RST;
#       endif
#       if (SCLK_BRG == 3)
	    *IM_BRGC4 = sclk_brg & ~CPM_BRG_RST;
#       endif
#       if (SCLK_BRG == 4)
	    *IM_BRGC5 = sclk_brg & ~CPM_BRG_RST;
#       endif
#       if (SCLK_BRG == 5)
	    *IM_BRGC6 = sclk_brg & ~CPM_BRG_RST;
#       endif
#       if (SCLK_BRG == 6)
	    *IM_BRGC7 = sclk_brg & ~CPM_BRG_RST;
#       endif
#       if (SCLK_BRG == 7)
	    *IM_BRGC8 = sclk_brg & ~CPM_BRG_RST;
#       endif

	/*
	 * Clear reset on LRCLK BRG
	 */
#       if (LRCLK_BRG == 0)
	    *IM_BRGC1 = lrclk_brg & ~CPM_BRG_RST;
#       endif
#       if (LRCLK_BRG == 1)
	    *IM_BRGC2 = lrclk_brg & ~CPM_BRG_RST;
#       endif
#       if (LRCLK_BRG == 2)
	    *IM_BRGC3 = lrclk_brg & ~CPM_BRG_RST;
#       endif
#       if (LRCLK_BRG == 3)
	    *IM_BRGC4 = lrclk_brg & ~CPM_BRG_RST;
#       endif
#       if (LRCLK_BRG == 4)
	    *IM_BRGC5 = lrclk_brg & ~CPM_BRG_RST;
#       endif
#       if (LRCLK_BRG == 5)
	    *IM_BRGC6 = lrclk_brg & ~CPM_BRG_RST;
#       endif
#       if (LRCLK_BRG == 6)
	    *IM_BRGC7 = lrclk_brg & ~CPM_BRG_RST;
#       endif
#       if (LRCLK_BRG == 7)
	    *IM_BRGC8 = lrclk_brg & ~CPM_BRG_RST;
#       endif

	/*
	 * Restore the Interrupt state
	 */
	if (flag) {
	    enable_interrupts();
	}
#   else
	/*
	 * Reset the clocks
	 */
	Daq_BRG_Reset(MCLK_BRG);
	Daq_BRG_Reset(SCLK_BRG);
	Daq_BRG_Reset(LRCLK_BRG);
#   endif
}

void Daq_Start_Clocks(int sample_rate)

{
#ifdef TIGHTEN_UP_BRG_TIMING
    volatile immap_t *immr = (immap_t *)CFG_IMMR;

    register uint mclk_brg;       /* MCLK  BRG value */
    register uint sclk_brg;       /* SCLK  BRG value */
    register uint temp_lrclk_brg; /* Temporary LRCLK BRG value */
    register uint real_lrclk_brg; /* Permanent LRCLK BRG value */
    uint          lrclk_brg;      /* LRCLK BRG value */
    unsigned long flags;          /* Interrupt flags */
    uint          sclk_cnt;       /* SCLK count */
    uint          delay_cnt;      /* Delay count */
#endif

#   ifdef TIGHTEN_UP_BRG_TIMING
	/*
	 * Obtain the enabled MCLK BRG value
	 */
#       if (MCLK_BRG == 0)
	    mclk_brg = (*IM_BRGC1 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 1)
	    mclk_brg = (*IM_BRGC2 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 2)
	    mclk_brg = (*IM_BRGC3 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 3)
	    mclk_brg = (*IM_BRGC4 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 4)
	    mclk_brg = (*IM_BRGC5 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 5)
	    mclk_brg = (*IM_BRGC6 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 6)
	    mclk_brg = (*IM_BRGC7 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (MCLK_BRG == 7)
	    mclk_brg = (*IM_BRGC8 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif

	/*
	 * Obtain the enabled SCLK BRG value
	 */
#       if (SCLK_BRG == 0)
	    sclk_brg = (*IM_BRGC1 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 1)
	    sclk_brg = (*IM_BRGC2 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 2)
	    sclk_brg = (*IM_BRGC3 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 3)
	    sclk_brg = (*IM_BRGC4 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 4)
	    sclk_brg = (*IM_BRGC5 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 5)
	    sclk_brg = (*IM_BRGC6 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 6)
	    sclk_brg = (*IM_BRGC7 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (SCLK_BRG == 7)
	    sclk_brg = (*IM_BRGC8 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif

	/*
	 * Obtain the enabled LRCLK BRG value
	 */
#       if (LRCLK_BRG == 0)
	    lrclk_brg = (*IM_BRGC1 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 1)
	    lrclk_brg = (*IM_BRGC2 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 2)
	    lrclk_brg = (*IM_BRGC3 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 3)
	    lrclk_brg = (*IM_BRGC4 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 4)
	    lrclk_brg = (*IM_BRGC5 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 5)
	    lrclk_brg = (*IM_BRGC6 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 6)
	    lrclk_brg = (*IM_BRGC7 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif
#       if (LRCLK_BRG == 7)
	    lrclk_brg = (*IM_BRGC8 & ~CPM_BRG_RST) | CPM_BRG_EN;
#       endif

	/* Save off the real LRCLK value */
	real_lrclk_brg = lrclk_brg;

	/* Obtain the current SCLK count */
	sclk_cnt  = ((sclk_brg & 0x00001FFE) >> 1) + 1;

	/* Compute the delay as a function of SCLK count */
	delay_cnt = ((sclk_cnt / 4) - 2) * 10 + 6;
	if (DaqSampleRate == 43402) {
	  delay_cnt++;
	}

	/* Clear out the count */
	temp_lrclk_brg = sclk_brg & ~0x00001FFE;

	/* Insert the count */
	temp_lrclk_brg |= ((delay_cnt + (sclk_cnt / 2) - 1) << 1) &  0x00001FFE;

	/*
	 * Disable interrupt and save the current state
	 */
	flag = disable_interrupts();

	/*
	 * Enable MCLK BRG
	 */
#       if (MCLK_BRG == 0)
	    *IM_BRGC1 = mclk_brg;
#       endif
#       if (MCLK_BRG == 1)
	    *IM_BRGC2 = mclk_brg;
#       endif
#       if (MCLK_BRG == 2)
	    *IM_BRGC3 = mclk_brg;
#       endif
#       if (MCLK_BRG == 3)
	    *IM_BRGC4 = mclk_brg;
#       endif
#       if (MCLK_BRG == 4)
	    *IM_BRGC5 = mclk_brg;
#       endif
#       if (MCLK_BRG == 5)
	    *IM_BRGC6 = mclk_brg;
#       endif
#       if (MCLK_BRG == 6)
	    *IM_BRGC7 = mclk_brg;
#       endif
#       if (MCLK_BRG == 7)
	    *IM_BRGC8 = mclk_brg;
#       endif

	/*
	 * Enable SCLK BRG
	 */
#       if (SCLK_BRG == 0)
	    *IM_BRGC1 = sclk_brg;
#       endif
#       if (SCLK_BRG == 1)
	    *IM_BRGC2 = sclk_brg;
#       endif
#       if (SCLK_BRG == 2)
	    *IM_BRGC3 = sclk_brg;
#       endif
#       if (SCLK_BRG == 3)
	    *IM_BRGC4 = sclk_brg;
#       endif
#       if (SCLK_BRG == 4)
	    *IM_BRGC5 = sclk_brg;
#       endif
#       if (SCLK_BRG == 5)
	    *IM_BRGC6 = sclk_brg;
#       endif
#       if (SCLK_BRG == 6)
	    *IM_BRGC7 = sclk_brg;
#       endif
#       if (SCLK_BRG == 7)
	    *IM_BRGC8 = sclk_brg;
#       endif

	/*
	 * Enable LRCLK BRG (1st time - temporary)
	 */
#       if (LRCLK_BRG == 0)
	    *IM_BRGC1 = temp_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 1)
	    *IM_BRGC2 = temp_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 2)
	    *IM_BRGC3 = temp_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 3)
	    *IM_BRGC4 = temp_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 4)
	    *IM_BRGC5 = temp_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 5)
	    *IM_BRGC6 = temp_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 6)
	    *IM_BRGC7 = temp_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 7)
	    *IM_BRGC8 = temp_lrclk_brg;
#       endif

	/*
	 * Enable LRCLK BRG (2nd time - permanent)
	 */
#       if (LRCLK_BRG == 0)
	    *IM_BRGC1 = real_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 1)
	    *IM_BRGC2 = real_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 2)
	    *IM_BRGC3 = real_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 3)
	    *IM_BRGC4 = real_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 4)
	    *IM_BRGC5 = real_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 5)
	    *IM_BRGC6 = real_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 6)
	    *IM_BRGC7 = real_lrclk_brg;
#       endif
#       if (LRCLK_BRG == 7)
	    *IM_BRGC8 = real_lrclk_brg;
#       endif

	/*
	 * Restore the Interrupt state
	 */
	if (flag) {
	    enable_interrupts();
	}
#   else
	/*
	 * Enable the clocks
	 */
	Daq_BRG_Enable(LRCLK_BRG);
	Daq_BRG_Enable(SCLK_BRG);
	Daq_BRG_Enable(MCLK_BRG);
#   endif
}

void Daq_Display_Clocks(void)

{
    volatile immap_t *immr = (immap_t *)CFG_IMMR;
    uint mclk_divisor; /* Detected MCLK divisor */
    uint sclk_divisor; /* Detected SCLK divisor */

    printf("\nBRG:\n");
    if (immr->im_brgc4 != 0) {
	printf("\tbrgc4\t0x%08x @ 0x%08x, %5d count, %d extc, %8s,  MCLK\n",
	       immr->im_brgc4,
	       (uint)&(immr->im_brgc4),
	       Daq_BRG_Get_Count(3),
	       Daq_BRG_Get_ExtClk(3),
	       Daq_BRG_Get_ExtClk_Description(3));
    }
    if (immr->im_brgc8 != 0) {
	printf("\tbrgc8\t0x%08x @ 0x%08x, %5d count, %d extc, %8s,  SCLK\n",
	       immr->im_brgc8,
	       (uint)&(immr->im_brgc8),
	       Daq_BRG_Get_Count(7),
	       Daq_BRG_Get_ExtClk(7),
	       Daq_BRG_Get_ExtClk_Description(7));
    }
    if (immr->im_brgc6 != 0) {
	printf("\tbrgc6\t0x%08x @ 0x%08x, %5d count, %d extc, %8s,  LRCLK\n",
	       immr->im_brgc6,
	       (uint)&(immr->im_brgc6),
	       Daq_BRG_Get_Count(5),
	       Daq_BRG_Get_ExtClk(5),
	       Daq_BRG_Get_ExtClk_Description(5));
    }
    if (immr->im_brgc1 != 0) {
	printf("\tbrgc1\t0x%08x @ 0x%08x, %5d count, %d extc, %8s,  SMC1\n",
	       immr->im_brgc1,
	       (uint)&(immr->im_brgc1),
	       Daq_BRG_Get_Count(0),
	       Daq_BRG_Get_ExtClk(0),
	       Daq_BRG_Get_ExtClk_Description(0));
    }
    if (immr->im_brgc2 != 0) {
	printf("\tbrgc2\t0x%08x @ 0x%08x, %5d count, %d extc, %8s,  SMC2\n",
	       immr->im_brgc2,
	       (uint)&(immr->im_brgc2),
	       Daq_BRG_Get_Count(1),
	       Daq_BRG_Get_ExtClk(1),
	       Daq_BRG_Get_ExtClk_Description(1));
    }
    if (immr->im_brgc3 != 0) {
	printf("\tbrgc3\t0x%08x @ 0x%08x, %5d count, %d extc, %8s,  SCC1\n",
	       immr->im_brgc3,
	       (uint)&(immr->im_brgc3),
	       Daq_BRG_Get_Count(2),
	       Daq_BRG_Get_ExtClk(2),
	       Daq_BRG_Get_ExtClk_Description(2));
    }
    if (immr->im_brgc5 != 0) {
	printf("\tbrgc5\t0x%08x @ 0x%08x, %5d count, %d extc, %8s\n",
	       immr->im_brgc5,
	       (uint)&(immr->im_brgc5),
	       Daq_BRG_Get_Count(4),
	       Daq_BRG_Get_ExtClk(4),
	       Daq_BRG_Get_ExtClk_Description(4));
    }
    if (immr->im_brgc7 != 0) {
	printf("\tbrgc7\t0x%08x @ 0x%08x, %5d count, %d extc, %8s\n",
	       immr->im_brgc7,
	       (uint)&(immr->im_brgc7),
	       Daq_BRG_Get_Count(6),
	       Daq_BRG_Get_ExtClk(6),
	       Daq_BRG_Get_ExtClk_Description(6));
    }

#   ifdef RUN_SCLK_ON_BRG_INT
	mclk_divisor = Daq_BRG_Rate(MCLK_BRG) / Daq_BRG_Rate(SCLK_BRG);
#   else
	mclk_divisor = Daq_BRG_Get_Count(SCLK_BRG);
#   endif
#   ifdef RUN_LRCLK_ON_BRG_INT
	sclk_divisor = Daq_BRG_Rate(SCLK_BRG) / Daq_BRG_Rate(LRCLK_BRG);
#   else
	sclk_divisor = Daq_BRG_Get_Count(LRCLK_BRG);
#   endif

    printf("\nADC/DAC Clocking (%d/%d):\n", sclk_divisor, mclk_divisor);
    printf("\tMCLK  %8d Hz, or %3dx SCLK, or %3dx LRCLK\n",
	   Daq_BRG_Rate(MCLK_BRG),
	   mclk_divisor,
	   mclk_divisor * sclk_divisor);
#   ifdef RUN_SCLK_ON_BRG_INT
	printf("\tSCLK  %8d Hz, or %3dx LRCLK\n",
	       Daq_BRG_Rate(SCLK_BRG),
	       sclk_divisor);
#   else
	printf("\tSCLK  %8d Hz, or %3dx LRCLK\n",
	       Daq_BRG_Rate(MCLK_BRG) / mclk_divisor,
	       sclk_divisor);
#   endif
#   ifdef RUN_LRCLK_ON_BRG_INT
	printf("\tLRCLK %8d Hz\n",
	       Daq_BRG_Rate(LRCLK_BRG));
#   else
#       ifdef RUN_SCLK_ON_BRG_INT
	    printf("\tLRCLK %8d Hz\n",
		   Daq_BRG_Rate(SCLK_BRG) / sclk_divisor);
#       else
	    printf("\tLRCLK %8d Hz\n",
		   Daq_BRG_Rate(MCLK_BRG) / (mclk_divisor * sclk_divisor));
#       endif
#   endif
    printf("\n");
}
