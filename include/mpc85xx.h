/*
 * Copyright 2004 Freescale Semiconductor.
 * Copyright(c) 2003 Motorola Inc.
 * Xianghua Xiao (x.xiao@motorola.com)
 */

#ifndef	__MPC85xx_H__
#define __MPC85xx_H__

#define EXC_OFF_SYS_RESET	0x0100	/* System reset	*/

#if defined(CONFIG_E500)
#include <e500.h>
#endif

/*
 * SCCR - System Clock Control Register, 9-8
 */
#define SCCR_CLPD       0x00000004      /* CPM Low Power Disable */
#define SCCR_DFBRG_MSK  0x00000003      /* Division by BRGCLK Mask */
#define SCCR_DFBRG_SHIFT 0

#define SCCR_DFBRG00    0x00000000      /* BRGCLK division by 4 */
#define SCCR_DFBRG01    0x00000001      /* BRGCLK div by 16 (normal) */
#define SCCR_DFBRG10    0x00000002      /* BRGCLK division by 64 */
#define SCCR_DFBRG11    0x00000003      /* BRGCLK division by 256 */

/*
 * Local Bus Controller - memory controller registers
 */
#define BRx_V		0x00000001	/* Bank Valid			*/
#define BRx_MS_GPCM	0x00000000	/* G.P.C.M. Machine Select	*/
#define BRx_MS_SDRAM	0x00000000	/* SDRAM Machine Select		*/
#define BRx_MS_UPMA	0x00000080	/* U.P.M.A Machine Select	*/
#define BRx_MS_UPMB	0x000000a0	/* U.P.M.B Machine Select	*/
#define BRx_MS_UPMC	0x000000c0	/* U.P.M.C Machine Select	*/
#define BRx_PS_8	0x00000800	/*  8 bit port size		*/
#define BRx_PS_32	0x00001800	/* 32 bit port size		*/
#define BRx_BA_MSK	0xffff8000	/* Base Address Mask		*/

#define ORxG_EAD	0x00000001	/* External addr latch delay	*/
#define ORxG_EHTR	0x00000002	/* Extended hold time on read	*/
#define ORxG_TRLX	0x00000004	/* Timing relaxed		*/
#define ORxG_SETA	0x00000008	/* External address termination	*/
#define ORxG_SCY_10_CLK	0x000000a0	/* 10 clock cycles wait states	*/
#define ORxG_SCY_15_CLK	0x000000f0	/* 15 clock cycles wait states	*/
#define ORxG_XACS	0x00000100	/* Extra addr to CS setup	*/
#define ORxG_ACS_DIV2	0x00000600	/* CS is output 1/2 a clock later*/
#define ORxG_CSNT	0x00000800	/* Chip Select Negation Time	*/

#define ORxU_BI		0x00000100	/* Burst Inhibit		*/
#define ORxU_AM_MSK	0xffff8000	/* Address Mask Mask		*/

#define MxMR_OP_NORM	0x00000000	/* Normal Operation		*/
#define MxMR_DSx_2_CYCL 0x00400000	/* 2 cycle Disable Period	*/
#define MxMR_OP_WARR	0x10000000	/* Write to Array		*/
#define MxMR_BSEL	0x80000000	/* Bus Select			*/

/* helpers to convert values into an OR address mask (GPCM mode) */
#define P2SZ_TO_AM(s)	((~((s) - 1)) & 0xffff8000)	/* must be pow of 2 */
#define MEG_TO_AM(m)	P2SZ_TO_AM((m) << 20)

#endif	/* __MPC85xx_H__ */
