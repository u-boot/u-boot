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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
  */
#ifndef _OMAP24XX_CLOCKS_H_
#define _OMAP24XX_CLOCKS_H_

#define COMMIT_DIVIDERS  0x1

#define MODE_BYPASS_FAST 0x2
#define APLL_LOCK        0xc
#ifdef CONFIG_APTIX
#define DPLL_LOCK        0x1   /* stay in bypass mode */
#else
#define DPLL_LOCK        0x3   /* DPLL lock */
#endif

/****************************************************************************;
; PRCM Scheme II
;
; Enable clocks and DPLL for:
;  DPLL=300, 	DPLLout=600   	M=1,N=50   CM_CLKSEL1_PLL[21:8]  12/2*50
;  Core=600  	(core domain)   DPLLx2     CM_CLKSEL2_PLL[1:0]
;  MPUF=300   	(mpu domain)    2          CM_CLKSEL_MPU[4:0]
;  DSPF=200    (dsp domain)    3          CM_CLKSEL_DSP[4:0]
;  DSPI=100                    6          CM_CLKSEL_DSP[6:5]
;  DSP_S          bypass	               CM_CLKSEL_DSP[7]
;  IVAF=200    (dsp domain)    3          CM_CLKSEL_DSP[12:8]
;  IVAF=100        auto
;  IVAI            auto
;  IVA_MPU         auto
;  IVA_S          bypass                  CM_CLKSEL_DSP[13]
;  GFXF=50      (gfx domain)	12         CM_CLKSEL_FGX[2:0]
;  SSI_SSRF=200                 3         CM_CLKSEL1_CORE[24:20]
;  SSI_SSTF=100     auto
;  L3=100Mhz (sdram)            6         CM_CLKSEL1_CORE[4:0]
;  L4=100Mhz                    6
;  C_L4_USB=50                 12         CM_CLKSEL1_CORE[6:5]
***************************************************************************/
#define II_DPLL_OUT_X2   0x2    /* x2 core out */
#define II_MPU_DIV       0x2    /* mpu = core/2 */
#define II_DSP_DIV       0x343  /* dsp & iva divider */
#define II_GFX_DIV       0x2
#define II_BUS_DIV       0x04600C26
#define II_BUS_DIV_ES1   0x04601026
#define II_DPLL_300      0x01832100

/* set defaults for boot up */
#ifdef PRCM_CONFIG_II
#define DPLL_OUT         II_DPLL_OUT_X2
#define MPU_DIV          II_MPU_DIV
#define DSP_DIV          II_DSP_DIV
#define GFX_DIV          II_GFX_DIV
#define BUS_DIV          II_BUS_DIV
#define BUS_DIV_ES1      II_BUS_DIV_ES1
#define DPLL_VAL         II_DPLL_300
#endif

/* lock delay time out */
#define LDELAY           12000000

#endif
