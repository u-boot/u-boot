/*
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __ASM_PPC_FSL_LBC_H
#define __ASM_PPC_FSL_LBC_H

#include <config.h>

/* BR - Base Registers
 */
#define BR0				0x5000		/* Register offset to immr */
#define BR1				0x5008
#define BR2				0x5010
#define BR3				0x5018
#define BR4				0x5020
#define BR5				0x5028
#define BR6				0x5030
#define BR7				0x5038

#define BR_BA				0xFFFF8000
#define BR_BA_SHIFT			15
#define BR_PS				0x00001800
#define BR_PS_SHIFT			11
#define BR_PS_8				0x00000800	/* Port Size 8 bit */
#define BR_PS_16			0x00001000	/* Port Size 16 bit */
#define BR_PS_32			0x00001800	/* Port Size 32 bit */
#define BR_DECC				0x00000600
#define BR_DECC_SHIFT			9
#define BR_DECC_OFF			0x00000000
#define BR_DECC_CHK			0x00000200
#define BR_DECC_CHK_GEN			0x00000400
#define BR_WP				0x00000100
#define BR_WP_SHIFT			8
#define BR_MSEL				0x000000E0
#define BR_MSEL_SHIFT			5
#define BR_MS_GPCM			0x00000000	/* GPCM */
#define BR_MS_FCM			0x00000020	/* FCM */
#ifdef CONFIG_MPC83xx
#define BR_MS_SDRAM			0x00000060	/* SDRAM */
#elif defined(CONFIG_MPC85xx)
#define BR_MS_SDRAM			0x00000000	/* SDRAM */
#endif
#define BR_MS_UPMA			0x00000080	/* UPMA */
#define BR_MS_UPMB			0x000000A0	/* UPMB */
#define BR_MS_UPMC			0x000000C0	/* UPMC */
#if !defined(CONFIG_MPC834X)
#define BR_ATOM				0x0000000C
#define BR_ATOM_SHIFT			2
#endif
#define BR_V				0x00000001
#define BR_V_SHIFT			0

#define UPMA			0
#define UPMB			1
#define UPMC			2

#if defined(CONFIG_MPC834X)
#define BR_RES				~(BR_BA | BR_PS | BR_DECC | BR_WP | BR_MSEL | BR_V)
#else
#define BR_RES				~(BR_BA | BR_PS | BR_DECC | BR_WP | BR_MSEL | BR_ATOM | BR_V)
#endif

/* OR - Option Registers
 */
#define OR0				0x5004		/* Register offset to immr */
#define OR1				0x500C
#define OR2				0x5014
#define OR3				0x501C
#define OR4				0x5024
#define OR5				0x502C
#define OR6				0x5034
#define OR7				0x503C

#define OR_GPCM_AM			0xFFFF8000
#define OR_GPCM_AM_SHIFT		15
#define OR_GPCM_BCTLD			0x00001000
#define OR_GPCM_BCTLD_SHIFT		12
#define OR_GPCM_CSNT			0x00000800
#define OR_GPCM_CSNT_SHIFT		11
#define OR_GPCM_ACS			0x00000600
#define OR_GPCM_ACS_SHIFT		9
#define OR_GPCM_ACS_DIV2		0x00000600
#define OR_GPCM_ACS_DIV4		0x00000400
#define OR_GPCM_XACS			0x00000100
#define OR_GPCM_XACS_SHIFT		8
#define OR_GPCM_SCY			0x000000F0
#define OR_GPCM_SCY_SHIFT		4
#define OR_GPCM_SCY_1			0x00000010
#define OR_GPCM_SCY_2			0x00000020
#define OR_GPCM_SCY_3			0x00000030
#define OR_GPCM_SCY_4			0x00000040
#define OR_GPCM_SCY_5			0x00000050
#define OR_GPCM_SCY_6			0x00000060
#define OR_GPCM_SCY_7			0x00000070
#define OR_GPCM_SCY_8			0x00000080
#define OR_GPCM_SCY_9			0x00000090
#define OR_GPCM_SCY_10			0x000000a0
#define OR_GPCM_SCY_11			0x000000b0
#define OR_GPCM_SCY_12			0x000000c0
#define OR_GPCM_SCY_13			0x000000d0
#define OR_GPCM_SCY_14			0x000000e0
#define OR_GPCM_SCY_15			0x000000f0
#define OR_GPCM_SETA			0x00000008
#define OR_GPCM_SETA_SHIFT		3
#define OR_GPCM_TRLX			0x00000004
#define OR_GPCM_TRLX_SHIFT		2
#define OR_GPCM_EHTR			0x00000002
#define OR_GPCM_EHTR_SHIFT		1
#define OR_GPCM_EAD			0x00000001
#define OR_GPCM_EAD_SHIFT		0

/* helpers to convert values into an OR address mask (GPCM mode) */
#define P2SZ_TO_AM(s)	((~((s) - 1)) & 0xffff8000)	/* must be pow of 2 */
#define MEG_TO_AM(m)	P2SZ_TO_AM((m) << 20)

#define OR_FCM_AM			0xFFFF8000
#define OR_FCM_AM_SHIFT				15
#define OR_FCM_BCTLD			0x00001000
#define OR_FCM_BCTLD_SHIFT			12
#define OR_FCM_PGS			0x00000400
#define OR_FCM_PGS_SHIFT			10
#define OR_FCM_CSCT			0x00000200
#define OR_FCM_CSCT_SHIFT			 9
#define OR_FCM_CST			0x00000100
#define OR_FCM_CST_SHIFT			 8
#define OR_FCM_CHT			0x00000080
#define OR_FCM_CHT_SHIFT			 7
#define OR_FCM_SCY			0x00000070
#define OR_FCM_SCY_SHIFT			 4
#define OR_FCM_SCY_1			0x00000010
#define OR_FCM_SCY_2			0x00000020
#define OR_FCM_SCY_3			0x00000030
#define OR_FCM_SCY_4			0x00000040
#define OR_FCM_SCY_5			0x00000050
#define OR_FCM_SCY_6			0x00000060
#define OR_FCM_SCY_7			0x00000070
#define OR_FCM_RST			0x00000008
#define OR_FCM_RST_SHIFT			 3
#define OR_FCM_TRLX			0x00000004
#define OR_FCM_TRLX_SHIFT			 2
#define OR_FCM_EHTR			0x00000002
#define OR_FCM_EHTR_SHIFT			 1

#define OR_UPM_AM			0xFFFF8000
#define OR_UPM_AM_SHIFT			15
#define OR_UPM_XAM			0x00006000
#define OR_UPM_XAM_SHIFT		13
#define OR_UPM_BCTLD			0x00001000
#define OR_UPM_BCTLD_SHIFT		12
#define OR_UPM_BI			0x00000100
#define OR_UPM_BI_SHIFT			8
#define OR_UPM_TRLX			0x00000004
#define OR_UPM_TRLX_SHIFT		2
#define OR_UPM_EHTR			0x00000002
#define OR_UPM_EHTR_SHIFT		1
#define OR_UPM_EAD			0x00000001
#define OR_UPM_EAD_SHIFT		0

#define OR_SDRAM_AM			0xFFFF8000
#define OR_SDRAM_AM_SHIFT		15
#define OR_SDRAM_XAM			0x00006000
#define OR_SDRAM_XAM_SHIFT		13
#define OR_SDRAM_COLS			0x00001C00
#define OR_SDRAM_COLS_SHIFT		10
#define OR_SDRAM_ROWS			0x000001C0
#define OR_SDRAM_ROWS_SHIFT		6
#define OR_SDRAM_PMSEL			0x00000020
#define OR_SDRAM_PMSEL_SHIFT		5
#define OR_SDRAM_EAD			0x00000001
#define OR_SDRAM_EAD_SHIFT		0

#define OR_AM_32KB			0xFFFF8000
#define OR_AM_64KB			0xFFFF0000
#define OR_AM_128KB			0xFFFE0000
#define OR_AM_256KB			0xFFFC0000
#define OR_AM_512KB			0xFFF80000
#define OR_AM_1MB			0xFFF00000
#define OR_AM_2MB			0xFFE00000
#define OR_AM_4MB			0xFFC00000
#define OR_AM_8MB			0xFF800000
#define OR_AM_16MB			0xFF000000
#define OR_AM_32MB			0xFE000000
#define OR_AM_64MB			0xFC000000
#define OR_AM_128MB			0xF8000000
#define OR_AM_256MB			0xF0000000
#define OR_AM_512MB			0xE0000000
#define OR_AM_1GB			0xC0000000
#define OR_AM_2GB			0x80000000
#define OR_AM_4GB			0x00000000

/* MxMR - UPM Machine A/B/C Mode Registers
 */
#define MxMR_MAD_MSK		0x0000003f /* Machine Address Mask	   */
#define MxMR_TLFx_MSK		0x000003c0 /* Refresh Loop Field Mask	   */
#define MxMR_WLFx_MSK		0x00003c00 /* Write Loop Field Mask	   */
#define MxMR_WLFx_1X		0x00000400 /*	executed 1 time		   */
#define MxMR_WLFx_2X		0x00000800 /*	executed 2 times	   */
#define MxMR_WLFx_3X		0x00000c00 /*	executed 3 times	   */
#define MxMR_WLFx_4X		0x00001000 /*	executed 4 times	   */
#define MxMR_WLFx_5X		0x00001400 /*	executed 5 times	   */
#define MxMR_WLFx_6X		0x00001800 /*	executed 6 times	   */
#define MxMR_WLFx_7X		0x00001c00 /*	executed 7 times	   */
#define MxMR_WLFx_8X		0x00002000 /*	executed 8 times	   */
#define MxMR_WLFx_9X		0x00002400 /*	executed 9 times	   */
#define MxMR_WLFx_10X		0x00002800 /*	executed 10 times	   */
#define MxMR_WLFx_11X		0x00002c00 /*	executed 11 times	   */
#define MxMR_WLFx_12X		0x00003000 /*	executed 12 times	   */
#define MxMR_WLFx_13X		0x00003400 /*	executed 13 times	   */
#define MxMR_WLFx_14X		0x00003800 /*	executed 14 times	   */
#define MxMR_WLFx_15X		0x00003c00 /*	executed 15 times	   */
#define MxMR_WLFx_16X		0x00000000 /*	executed 16 times	   */
#define MxMR_RLFx_MSK		0x0003c000 /* Read Loop Field Mask	   */
#define MxMR_GPL_x4DIS		0x00040000 /* GPL_A4 Ouput Line Disable	   */
#define MxMR_G0CLx_MSK		0x00380000 /* General Line 0 Control Mask  */
#define MxMR_DSx_1_CYCL		0x00000000 /* 1 cycle Disable Period	   */
#define MxMR_DSx_2_CYCL		0x00400000 /* 2 cycle Disable Period	   */
#define MxMR_DSx_3_CYCL		0x00800000 /* 3 cycle Disable Period	   */
#define MxMR_DSx_4_CYCL		0x00c00000 /* 4 cycle Disable Period	   */
#define MxMR_DSx_MSK		0x00c00000 /* Disable Timer Period Mask	   */
#define MxMR_AMx_MSK		0x07000000 /* Addess Multiplex Size Mask   */
#define MxMR_OP_NORM		0x00000000 /* Normal Operation		   */
#define MxMR_OP_WARR		0x10000000 /* Write to Array		   */
#define MxMR_OP_RARR		0x20000000 /* Read from Array		   */
#define MxMR_OP_RUNP		0x30000000 /* Run Pattern		   */
#define MxMR_OP_MSK		0x30000000 /* Command Opcode Mask	   */
#define MxMR_RFEN		0x40000000 /* Refresh Enable		   */
#define MxMR_BSEL		0x80000000 /* Bus Select		   */

#define LBLAWAR_EN			0x80000000
#define LBLAWAR_4KB			0x0000000B
#define LBLAWAR_8KB			0x0000000C
#define LBLAWAR_16KB			0x0000000D
#define LBLAWAR_32KB			0x0000000E
#define LBLAWAR_64KB			0x0000000F
#define LBLAWAR_128KB			0x00000010
#define LBLAWAR_256KB			0x00000011
#define LBLAWAR_512KB			0x00000012
#define LBLAWAR_1MB			0x00000013
#define LBLAWAR_2MB			0x00000014
#define LBLAWAR_4MB			0x00000015
#define LBLAWAR_8MB			0x00000016
#define LBLAWAR_16MB			0x00000017
#define LBLAWAR_32MB			0x00000018
#define LBLAWAR_64MB			0x00000019
#define LBLAWAR_128MB			0x0000001A
#define LBLAWAR_256MB			0x0000001B
#define LBLAWAR_512MB			0x0000001C
#define LBLAWAR_1GB			0x0000001D
#define LBLAWAR_2GB			0x0000001E

/* LBCR - Local Bus Configuration Register
 */
#define LBCR_LDIS			0x80000000
#define LBCR_LDIS_SHIFT			31
#define LBCR_BCTLC			0x00C00000
#define LBCR_BCTLC_SHIFT		22
#define LBCR_LPBSE			0x00020000
#define LBCR_LPBSE_SHIFT		17
#define LBCR_EPAR			0x00010000
#define LBCR_EPAR_SHIFT			16
#define LBCR_BMT			0x0000FF00
#define LBCR_BMT_SHIFT			8

/* LCRR - Clock Ratio Register
 */
#define LCRR_DBYP			0x80000000
#define LCRR_DBYP_SHIFT			31
#define LCRR_BUFCMDC			0x30000000
#define LCRR_BUFCMDC_SHIFT		28
#define LCRR_BUFCMDC_1			0x10000000
#define LCRR_BUFCMDC_2			0x20000000
#define LCRR_BUFCMDC_3			0x30000000
#define LCRR_BUFCMDC_4			0x00000000
#define LCRR_ECL			0x03000000
#define LCRR_ECL_SHIFT			24
#define LCRR_ECL_4			0x00000000
#define LCRR_ECL_5			0x01000000
#define LCRR_ECL_6			0x02000000
#define LCRR_ECL_7			0x03000000
#define LCRR_EADC			0x00030000
#define LCRR_EADC_SHIFT			16
#define LCRR_EADC_1			0x00010000
#define LCRR_EADC_2			0x00020000
#define LCRR_EADC_3			0x00030000
#define LCRR_EADC_4			0x00000000
#define LCRR_CLKDIV			0x0000000F
#define LCRR_CLKDIV_SHIFT		0
#define LCRR_CLKDIV_2			0x00000002
#define LCRR_CLKDIV_4			0x00000004
#define LCRR_CLKDIV_8			0x00000008

/* LTEDR - Transfer Error Check Disable Register
 */
#define LTEDR_BMD	0x80000000 /* Bus monitor disable				*/
#define LTEDR_PARD	0x20000000 /* Parity error checking disabled			*/
#define LTEDR_WPD	0x04000000 /* Write protect error checking diable		*/
#define LTEDR_WARA	0x00800000 /* Write-after-read-atomic error checking diable	*/
#define LTEDR_RAWA	0x00400000 /* Read-after-write-atomic error checking disable	*/
#define LTEDR_CSD	0x00080000 /* Chip select error checking disable		*/

#endif /* __ASM_PPC_FSL_LBC_H */
