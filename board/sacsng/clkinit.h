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

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#define SLRCLK_EN_MASK  0x00040000 /* PA13 - SLRCLK_EN*     */

#define MIN_SAMPLE_RATE       4000 /* Minimum sample rate */
#define MAX_128x_SAMPLE_RATE 43402 /* Maximum 128x sample rate */
#define MAX_64x_SAMPLE_RATE  86805 /* Maximum  64x sample rate */

#define KHZ          ((uint)1000)
#define MHZ          ((uint)(1000 * KHZ))

#define MCLK_BRG     3        /* MCLK, Master CLocK for the A/D & D/A   */
#define SCLK_BRG     7        /* SCLK, Sample CLocK for the A/D & D/A   */
#define LRCLK_BRG    5        /* LRCLK, L/R CLocK for the A/D & D/A     */
			      /*   0 == BRG1 (used for SMC1)            */
			      /*   1 == BRG2 (used for SMC2)            */
			      /*   2 == BRG3 (used for SCC1)            */
			      /*   3 == BRG4 (MCLK)                     */
			      /*   4 == BRG5                            */
			      /*   5 == BRG6 (LRCLK)                    */
			      /*   6 == BRG7                            */
			      /*   7 == BRG8 (SCLK)                     */

#define MCLK_DIVISOR  4       /*  SCLK = MCLK / MCLK_DIVISOR */
#define SCLK_DIVISOR (Daq64xSampling ? 64 : 128)
			      /* LRCLK = SCLK / SCLK_DIVISOR */

#define TIGHTEN_UP_BRG_EN_TIMING /* Tighten up the BRG enable timing      */
#define RUN_SCLK_ON_BRG_INT      /* Run SCLK on BRG_INT instead of MCLK   */
				 /* The 8260 (Mask B.3) seems to have     */
				 /* problems generating SCLK from MCLK    */
				 /* via CLK9.                             */
#define RUN_LRCLK_ON_BRG_INT     /* Run LRCLK on BRG_INT instead of SCLK  */
				 /* The 8260 (Mask B.3) seems to have     */
				 /* problems generating LRCLK from SCLK   */

#define NUM_LRCLKS_TO_STABILIZE 1  /* Number of LRCLK period (sample)     */
				   /* to wait for the clock to stabilize  */

#define CPM_CLK      (gd->bd->bi_cpmfreq)
#define DFBRG        4
#define BRG_INT_CLK  (CPM_CLK * 2 / DFBRG)
			      /* BRG = CPM * 2 / DFBRG (Sect 9.8) */
			      /* BRG = CPM * 2 / 4                */
			      /* BRG = CPM / 2                    */

#define CPM_BRG_EXTC_MASK	((uint)0x0000C000)
#define CPM_BRG_EXTC_SHIFT      14

#define CPM_BRG_DIV16_MASK	((uint)0x00000001)
#define CPM_BRG_DIV16_SHIFT     1

#define CPM_BRG_EXTC_BRGCLK     0
#define CPM_BRG_EXTC_CLK3       1
#define CPM_BRG_EXTC_CLK9       CPM_BRG_EXTC_CLK3
#define CPM_BRG_EXTC_CLK5       2
#define CPM_BRG_EXTC_CLK15      CPM_BRG_EXTC_CLK5

#define IM_BRGC1 ((uint *)0xf00119f0)
#define IM_BRGC2 ((uint *)0xf00119f4)
#define IM_BRGC3 ((uint *)0xf00119f8)
#define IM_BRGC4 ((uint *)0xf00119fc)
#define IM_BRGC5 ((uint *)0xf00115f0)
#define IM_BRGC6 ((uint *)0xf00115f4)
#define IM_BRGC7 ((uint *)0xf00115f8)
#define IM_BRGC8 ((uint *)0xf00115fc)

/*
 * External declarations
 */

extern int Daq64xSampling;

extern void Daq_BRG_Reset(uint brg);
extern void Daq_BRG_Run(uint brg);

extern void Daq_BRG_Disable(uint brg);
extern void Daq_BRG_Enable(uint brg);

extern uint Daq_BRG_Get_Div16(uint brg);
extern void Daq_BRG_Set_Div16(uint brg, uint div16);

extern uint Daq_BRG_Get_Count(uint brg);
extern void Daq_BRG_Set_Count(uint brg, uint brg_cnt);

extern uint Daq_BRG_Get_ExtClk(uint brg);
extern char* Daq_BRG_Get_ExtClk_Description(uint brg);
extern void Daq_BRG_Set_ExtClk(uint brg, uint extc);

extern uint Daq_BRG_Rate(uint brg);

extern uint Daq_Get_SampleRate(void);

extern void Daq_Init_Clocks(int sample_rate, int sample_64x);
extern void Daq_Stop_Clocks(void);
extern void Daq_Start_Clocks(int sample_rate);
extern void Daq_Display_Clocks(void);
