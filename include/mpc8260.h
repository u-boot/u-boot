/*
 * (C) Copyright 2000, 2001
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

/*
 * mpc8260.h
 *
 * MPC8255 / MPC8260 specific definitions
 */

#ifndef __MPC8260_H__
#define __MPC8260_H__

#ifdef CONFIG_MPC8255
#define CPU_ID_STR	"MPC8255"
#endif
#ifndef CPU_ID_STR
#if defined(CONFIG_MPC8272_FAMILY)
#define CPU_ID_STR	"MPC8272"
#else
#define CPU_ID_STR	"MPC8260"
#endif
#endif /* !CPU_ID_STR */

/*-----------------------------------------------------------------------
 * Exception offsets (PowerPC standard)
 */
#define EXC_OFF_SYS_RESET	0x0100	/* System reset			*/


/*-----------------------------------------------------------------------
 * BCR - Bus Configuration Register					 4-25
 */
#define BCR_EBM		0x80000000	/* External Bus Mode		*/
#define BCR_APD_MSK	0x70000000	/* Address Phase Delay Mask	*/
#define BCR_L2C		0x08000000	/* Secondary Cache Controller	*/
#define BCR_L2D_MSK	0x07000000	/* L2 Cache Hit Delay Mask	*/
#define BCR_PLDP	0x00800000	/* Pipeline Maximum Depth	*/
#define BCR_EAV		0x00400000	/* Enable Address Visibility	*/
#define BCR_ETM		0x00080000	/* Compatibility Mode Enable	*/
#define BCR_LETM	0x00040000	/* LocalBus Compatibility Mode Enable*/
#define BCR_EPAR	0x00020000	/* Even Parity			*/
#define BCR_LEPAR	0x00010000	/* Local Bus Even Parity	*/
#define BCR_NPQM0	0x00008000	/* Non PowerQUICC-II Master 0	*/
#define BCR_NPQM1	0x00004000	/* Non PowerQUICC-II Master 1	*/
#define BCR_NPQM2	0x00002000	/* Non PowerQUICC-II Master 2	*/
#define BCR_EXDD	0x00000400	/* External Master Delay Disable*/
#define BCR_ISPS	0x00000010	/* Internal Space Port Size	*/

/*-----------------------------------------------------------------------
 * PPC_ACR - 60x Bus Arbiter Configuration Register			 4-28
 */
#define PPC_ACR_DBGD	0x20		/* Data Bus Grant Delay		*/
#define PPC_ACR_EARB	0x10		/* External Arbitration		*/
#define PPC_ACR_PRKM_MSK 0x0f		/* Parking Master		*/

#define PPC_ACR_PRKM_CPMH 0x00		/* CPM high request level	*/
#define PPC_ACR_PRKM_CPMM 0x01		/* CPM middle request level	*/
#define PPC_ACR_PRKM_CPML 0x02		/* CPM low request level	*/
#define PPC_ACR_PRKM_CORE 0x06		/* Internal Core		*/
#define PPC_ACR_PRKM_EXT1 0x07		/* External Master 1		*/
#define PPC_ACR_PRKM_EXT2 0x08		/* External Master 2		*/
#define PPC_ACR_PRKM_EXT3 0x09		/* External Master 3		*/

/*-----------------------------------------------------------------------
 * PPC_ALRH/PPC_ALRL - 60x Bus Arbitration-Level Registers		 4-28
 */
#define PPC_ALRH_PF0_MSK  0xf0000000	/* Priority Field  0 Mask	*/
#define PPC_ALRH_PF1_MSK  0x0f000000	/* Priority Field  1 Mask	*/
#define PPC_ALRH_PF2_MSK  0x00f00000	/* Priority Field  2 Mask	*/
#define PPC_ALRH_PF3_MSK  0x000f0000	/* Priority Field  3 Mask	*/
#define PPC_ALRH_PF4_MSK  0x0000f000	/* Priority Field  4 Mask	*/
#define PPC_ALRH_PF5_MSK  0x00000f00	/* Priority Field  5 Mask	*/
#define PPC_ALRH_PF6_MSK  0x000000f0	/* Priority Field  6 Mask	*/
#define PPC_ALRH_PF7_MSK  0x0000000f	/* Priority Field  7 Mask	*/
#define PPC_ALRL_PF8_MSK  0xf0000000	/* Priority Field  8 Mask	*/
#define PPC_ALRL_PF9_MSK  0x0f000000	/* Priority Field  9 Mask	*/
#define PPC_ALRL_PF10_MSK 0x00f00000	/* Priority Field 10 Mask	*/
#define PPC_ALRL_PF11_MSK 0x000f0000	/* Priority Field 11 Mask	*/
#define PPC_ALRL_PF12_MSK 0x0000f000	/* Priority Field 12 Mask	*/
#define PPC_ALRL_PF13_MSK 0x00000f00	/* Priority Field 13 Mask	*/
#define PPC_ALRL_PF14_MSK 0x000000f0	/* Priority Field 14 Mask	*/
#define PPC_ALRL_PF15_MSK 0x0000000f	/* Priority Field 15 Mask	*/

/*-----------------------------------------------------------------------
 * LCL_ACR - Local Bus Arbiter Configuration Register			 4-29
 */
#define LCL_ACR_DBGD	0x20		/* Data Bus Grant Delay		*/
#define LCL_ACR_PRKM_MSK 0x0f		/* Parking Master		*/

#define LCL_ACR_PRKM_CPMH 0x00		/* CPM high request level	*/
#define LCL_ACR_PRKM_CPMM 0x01		/* CPM middle request level	*/
#define LCL_ACR_PRKM_CPML 0x02		/* CPM low request level	*/
#define LCL_ACR_PRKM_HOST 0x03		/* Host Bridge			*/

/*-----------------------------------------------------------------------
 * LCL_ALRH/LCL_ALRL - Local Bus Arbitration Level Registers		 4-30
 */
#define LCL_ALRH_PF0_MSK  0xf0000000	/* Priority Field  0 Mask	*/
#define LCL_ALRH_PF1_MSK  0x0f000000	/* Priority Field  1 Mask	*/
#define LCL_ALRH_PF2_MSK  0x00f00000	/* Priority Field  2 Mask	*/
#define LCL_ALRH_PF3_MSK  0x000f0000	/* Priority Field  3 Mask	*/
#define LCL_ALRH_PF4_MSK  0x0000f000	/* Priority Field  4 Mask	*/
#define LCL_ALRH_PF5_MSK  0x00000f00	/* Priority Field  5 Mask	*/
#define LCL_ALRH_PF6_MSK  0x000000f0	/* Priority Field  6 Mask	*/
#define LCL_ALRH_PF7_MSK  0x0000000f	/* Priority Field  7 Mask	*/
#define LCL_ALRL_PF8_MSK  0xf0000000	/* Priority Field  8 Mask	*/
#define LCL_ALRL_PF9_MSK  0x0f000000	/* Priority Field  9 Mask	*/
#define LCL_ALRL_PF10_MSK 0x00f00000	/* Priority Field 10 Mask	*/
#define LCL_ALRL_PF11_MSK 0x000f0000	/* Priority Field 11 Mask	*/
#define LCL_ALRL_PF12_MSK 0x0000f000	/* Priority Field 12 Mask	*/
#define LCL_ALRL_PF13_MSK 0x00000f00	/* Priority Field 13 Mask	*/
#define LCL_ALRL_PF14_MSK 0x000000f0	/* Priority Field 14 Mask	*/
#define LCL_ALRL_PF15_MSK 0x0000000f	/* Priority Field 15 Mask	*/

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration Register				 4-31
 */
#define SIUMCR_BBD	0x80000000	/* Bus Busy Disable		*/
#define SIUMCR_ESE	0x40000000	/* External Snoop Enable	*/
#define SIUMCR_PBSE	0x20000000	/* Parity Byte Select Enable	*/
#define SIUMCR_CDIS	0x10000000	/* Core Disable			*/
#define SIUMCR_DPPC00	0x00000000	/* Data Parity Pins Configuration*/
#define SIUMCR_DPPC01	0x04000000	/* - " -			*/
#define SIUMCR_DPPC10	0x08000000	/* - " -			*/
#define SIUMCR_DPPC11	0x0c000000	/* - " -			*/
#define SIUMCR_L2CPC00	0x00000000	/* L2 Cache Pins Configuration	*/
#define SIUMCR_L2CPC01	0x01000000	/* - " -			*/
#define SIUMCR_L2CPC10	0x02000000	/* - " -			*/
#define SIUMCR_L2CPC11	0x03000000	/* - " -			*/
#define SIUMCR_LBPC00	0x00000000	/* Local Bus Pins Configuration	*/
#define SIUMCR_LBPC01	0x00400000	/* - " -			*/
#define SIUMCR_LBPC10	0x00800000	/* - " -			*/
#define SIUMCR_LBPC11	0x00c00000	/* - " -			*/
#define SIUMCR_APPC00	0x00000000	/* Address Parity Pins Configuration*/
#define SIUMCR_APPC01	0x00100000	/* - " -			*/
#define SIUMCR_APPC10	0x00200000	/* - " -			*/
#define SIUMCR_APPC11	0x00300000	/* - " -			*/
#define SIUMCR_CS10PC00	0x00000000	/* CS10 Pin Configuration	*/
#define SIUMCR_CS10PC01	0x00040000	/* - " -			*/
#define SIUMCR_CS10PC10	0x00080000	/* - " -			*/
#define SIUMCR_CS10PC11	0x000c0000	/* - " -			*/
#define SIUMCR_BCTLC00	0x00000000	/* Buffer Control Configuration	*/
#define SIUMCR_BCTLC01	0x00010000	/* - " -			*/
#define SIUMCR_BCTLC10	0x00020000	/* - " -			*/
#define SIUMCR_BCTLC11	0x00030000	/* - " -			*/
#define SIUMCR_MMR00	0x00000000	/* Mask Masters Requests	*/
#define SIUMCR_MMR01	0x00004000	/* - " -			*/
#define SIUMCR_MMR10	0x00008000	/* - " -			*/
#define SIUMCR_MMR11	0x0000c000	/* - " -			*/
#define SIUMCR_LPBSE	0x00002000	/* LocalBus Parity Byte Select Enable*/

/*-----------------------------------------------------------------------
 * IMMR - Internal Memory Map Register					 4-34
 */
#define IMMR_ISB_MSK	 0xfffe0000	/* Internal Space base		*/
#define IMMR_PARTNUM_MSK 0x0000ff00	/* Part number			*/
#define IMMR_MASKNUM_MSK 0x000000ff	/* Mask number			*/

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control Register				 4-35
 */
#define SYPCR_SWTC	0xffff0000	/* Software Watchdog Timer Count*/
#define SYPCR_BMT	0x0000ff00	/* Bus Monitor Timing		*/
#define SYPCR_PBME	0x00000080	/* 60x Bus Monitor Enable	*/
#define SYPCR_LBME	0x00000040	/* Local Bus Monitor Enable	*/
#define SYPCR_SWE	0x00000004	/* Software Watchdog Enable	*/
#define SYPCR_SWRI	0x00000002	/* Software Watchdog Reset/Int Select*/
#define SYPCR_SWP	0x00000001	/* Software Watchdog Prescale	*/

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control Register			 4-40
 */
#define TMCNTSC_SEC	0x0080		/* Once Per Second Interrupt	*/
#define TMCNTSC_ALR	0x0040		/* Alarm Interrupt		*/
#define TMCNTSC_SIE	0x0008		/* Second Interrupt Enable	*/
#define TMCNTSC_ALE	0x0004		/* Alarm Interrupt Enable	*/
#define TMCNTSC_TCF	0x0002		/* Time Counter Frequency	*/
#define TMCNTSC_TCE	0x0001		/* Time Counter Enable		*/

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control Register		 4-42
 */
#if 0	/* already defined in asm/immap_8260.h */
#define PISCR_PS	0x0080		/* Periodic Interrupt Status	*/
#define PISCR_PIE	0x0004		/* Periodic Interrupt Enable	*/
#define PISCR_PTF	0x0002		/* Periodic Timer Frequency	*/
#define PISCR_PTE	0x0001		/* Periodic Timer Enable	*/
#endif

/*-----------------------------------------------------------------------
 * RSR - Reset Status Register						 5-4
 */
#define RSR_JTRS	0x00000020	/* JTAG Reset Status		*/
#define RSR_CSRS	0x00000010	/* Check Stop Reset Status	*/
#define RSR_SWRS	0x00000008	/* Software Watchdog Reset Status*/
#define RSR_BMRS	0x00000004	/* Bus Monitor Reset Status	*/
#define RSR_ESRS	0x00000002	/* External Soft Reset Status	*/
#define RSR_EHRS	0x00000001	/* External Hard Reset Status	*/

#define RSR_ALLBITS	(RSR_JTRS|RSR_CSRS|RSR_SWRS|RSR_BMRS|RSR_ESRS|RSR_EHRS)

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register						 5-5
 */
#define RMR_CSRE	0x00000001	/* Checkstop Reset Enable	*/

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Word					 5-8
 */
#define HRCW_EARB	0x80000000	/* External Arbitration		*/
#define HRCW_EXMC	0x40000000	/* External Memory Controller	*/
#define HRCW_CDIS	0x20000000	/* Core Disable			*/
#define HRCW_EBM	0x10000000	/* External Bus Mode		*/
#define HRCW_BPS00	0x00000000	/* Boot Port Size		*/
#define HRCW_BPS01	0x04000000	/* - " -			*/
#define HRCW_BPS10	0x08000000	/* - " -			*/
#define HRCW_BPS11	0x0c000000	/* - " -			*/
#define HRCW_CIP	0x02000000	/* Core Initial Prefix		*/
#define HRCW_ISPS	0x01000000	/* Internal Space Port Size	*/
#define HRCW_L2CPC00	0x00000000	/* L2 Cache Pins Configuration	*/
#define HRCW_L2CPC01	0x00400000	/* - " -			*/
#define HRCW_L2CPC10	0x00800000	/* - " -			*/
#define HRCW_L2CPC11	0x00c00000	/* - " -			*/
#define HRCW_DPPC00	0x00000000	/* Data Parity Pin Configuration*/
#define HRCW_DPPC01	0x00100000	/* - " -			*/
#define HRCW_DPPC10	0x00200000	/* - " -			*/
#define HRCW_DPPC11	0x00300000	/* - " -			*/
#define HRCW_reserved1	0x00080000	/* reserved			*/
#define HRCW_ISB000	0x00000000	/* Initial Internal Space Base	*/
#define HRCW_ISB001	0x00010000	/* - " -			*/
#define HRCW_ISB010	0x00020000	/* - " -			*/
#define HRCW_ISB011	0x00030000	/* - " -			*/
#define HRCW_ISB100	0x00040000	/* - " -			*/
#define HRCW_ISB101	0x00050000	/* - " -			*/
#define HRCW_ISB110	0x00060000	/* - " -			*/
#define HRCW_ISB111	0x00070000	/* - " -			*/
#define HRCW_BMS	0x00008000	/* Boot Memory Space		*/
#define HRCW_BBD	0x00004000	/* Bus Busy Disable		*/
#define HRCW_MMR00	0x00000000	/* Mask Masters Requests	*/
#define HRCW_MMR01	0x00001000	/* - " -			*/
#define HRCW_MMR10	0x00002000	/* - " -			*/
#define HRCW_MMR11	0x00003000	/* - " -			*/
#define HRCW_LBPC00	0x00000000	/* Local Bus Pin Configuration	*/
#define HRCW_LBPC01	0x00000400	/* - " -			*/
#define HRCW_LBPC10	0x00000800	/* - " -			*/
#define HRCW_LBPC11	0x00000c00	/* - " -			*/
#define HRCW_APPC00	0x00000000	/* Address Parity Pin Configuration*/
#define HRCW_APPC01	0x00000100	/* - " -			*/
#define HRCW_APPC10	0x00000200	/* - " -			*/
#define HRCW_APPC11	0x00000300	/* - " -			*/
#define HRCW_CS10PC00	0x00000000	/* CS10 Pin Configuration	*/
#define HRCW_CS10PC01	0x00000040	/* - " -			*/
#define HRCW_CS10PC10	0x00000080	/* - " -			*/
#define HRCW_CS10PC11	0x000000c0	/* - " -			*/
#define HRCW_MODCK_H0000 0x00000000	/* High-order bits of MODCK Bus	*/
#define HRCW_MODCK_H0001 0x00000001	/* - " -			*/
#define HRCW_MODCK_H0010 0x00000002	/* - " -			*/
#define HRCW_MODCK_H0011 0x00000003	/* - " -			*/
#define HRCW_MODCK_H0100 0x00000004	/* - " -			*/
#define HRCW_MODCK_H0101 0x00000005	/* - " -			*/
#define HRCW_MODCK_H0110 0x00000006	/* - " -			*/
#define HRCW_MODCK_H0111 0x00000007	/* - " -			*/
#define HRCW_MODCK_H1000 0x00000008	/* - " -			*/
#define HRCW_MODCK_H1001 0x00000009	/* - " -			*/
#define HRCW_MODCK_H1010 0x0000000a	/* - " -			*/
#define HRCW_MODCK_H1011 0x0000000b	/* - " -			*/
#define HRCW_MODCK_H1100 0x0000000c	/* - " -			*/
#define HRCW_MODCK_H1101 0x0000000d	/* - " -			*/
#define HRCW_MODCK_H1110 0x0000000e	/* - " -			*/
#define HRCW_MODCK_H1111 0x0000000f	/* - " -			*/

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control Register					 9-8
 */
#define SCCR_PCI_MODE	0x00000100	/* PCI Mode	*/
#define SCCR_PCI_MODCK	0x00000080	/* Value of PCI_MODCK pin	*/
#define SCCR_PCIDF_MSK	0x00000078	/* PCI division factor	*/
#define SCCR_PCIDF_SHIFT 3
#define SCCR_CLPD	0x00000004	/* CPM Low Power Disable	*/
#define SCCR_DFBRG_MSK	0x00000003	/* Division factor of BRGCLK Mask */
#define SCCR_DFBRG_SHIFT 0

#define SCCR_DFBRG00	0x00000000	/* BRGCLK division by 4		*/
#define SCCR_DFBRG01	0x00000001	/* BRGCLK division by 16 (normal op.)*/
#define SCCR_DFBRG10	0x00000002	/* BRGCLK division by 64	*/
#define SCCR_DFBRG11	0x00000003	/* BRGCLK division by 128	*/

/*-----------------------------------------------------------------------
 * SCMR - System Clock Mode Register					 9-9
 */
#define SCMR_CORECNF_MSK   0x1f000000	/* Core Configuration Mask	*/
#define SCMR_CORECNF_SHIFT 24
#define SCMR_BUSDF_MSK	   0x00f00000	/* 60x Bus Division Factor Mask	*/
#define SCMR_BUSDF_SHIFT   20
#define SCMR_CPMDF_MSK	   0x000f0000	/* CPM Division Factor Mask	*/
#define SCMR_CPMDF_SHIFT   16
#define SCMR_PLLDF	   0x00001000	/* PLL Pre-divider Value	*/
#define SCMR_PLLMF_MSK	   0x00000fff	/* PLL Multiplication Factor Mask*/
#define SCMR_PLLMF_MSKH7   0x0000000f	/* for HiP7 processors */
#define SCMR_PLLMF_SHIFT 0


/*-----------------------------------------------------------------------
 * MxMR - Machine A/B/C Mode Registers					10-13
 */
#define MxMR_BSEL	0x80000000	/* Bus Select			*/
#define MxMR_RFEN	0x40000000	/* Refresh Enable		*/
#define MxMR_OP_MSK	0x30000000	/* Command Opcode Mask		*/
#define MxMR_AMx_MSK	0x07000000	/* Addess Multiplex Size Mask	*/
#define MxMR_DSx_MSK	0x00c00000	/* Disable Timer Period Mask	*/
#define MxMR_G0CLx_MSK	0x00380000	/* General Line 0 Control Mask	*/
#define MxMR_GPL_x4DIS	0x00040000	/* GPL_A4 Ouput Line Disable	*/
#define MxMR_RLFx_MSK	0x0003c000	/* Read Loop Field Mask		*/
#define MxMR_WLFx_MSK	0x00003c00	/* Write Loop Field Mask	*/
#define MxMR_TLFx_MSK	0x000003c0	/* Refresh Loop Field Mask	*/
#define MxMR_MAD_MSK	0x0000003f	/* Machine Address Mask		*/

#define MxMR_OP_NORM	0x00000000	/* Normal Operation		*/
#define MxMR_OP_WARR	0x10000000	/* Write to Array		*/
#define MxMR_OP_RARR	0x20000000	/* Read from Array		*/
#define MxMR_OP_RUNP	0x30000000	/* Run Pattern			*/

#define MxMR_AMx_TYPE_0 0x00000000	/* Addess Multiplexing Type 0	*/
#define MxMR_AMx_TYPE_1 0x01000000	/* Addess Multiplexing Type 1	*/
#define MxMR_AMx_TYPE_2 0x02000000	/* Addess Multiplexing Type 2	*/
#define MxMR_AMx_TYPE_3 0x03000000	/* Addess Multiplexing Type 3	*/
#define MxMR_AMx_TYPE_4 0x04000000	/* Addess Multiplexing Type 4	*/
#define MxMR_AMx_TYPE_5 0x05000000	/* Addess Multiplexing Type 5	*/

#define MxMR_DSx_1_CYCL 0x00000000	/* 1 cycle Disable Period	*/
#define MxMR_DSx_2_CYCL 0x00400000	/* 2 cycle Disable Period	*/
#define MxMR_DSx_3_CYCL 0x00800000	/* 3 cycle Disable Period	*/
#define MxMR_DSx_4_CYCL 0x00c00000	/* 4 cycle Disable Period	*/

#define MxMR_G0CLx_A12	0x00000000	/* General Line 0 : A12		*/
#define MxMR_G0CLx_A11	0x00080000	/* General Line 0 : A11		*/
#define MxMR_G0CLx_A10	0x00100000	/* General Line 0 : A10		*/
#define MxMR_G0CLx_A9	0x00180000	/* General Line 0 : A9		*/
#define MxMR_G0CLx_A8	0x00200000	/* General Line 0 : A8		*/
#define MxMR_G0CLx_A7	0x00280000	/* General Line 0 : A7		*/
#define MxMR_G0CLx_A6	0x00300000	/* General Line 0 : A6		*/
#define MxMR_G0CLx_A5	0x00380000	/* General Line 0 : A5		*/

#define MxMR_RLFx_1X	0x00004000	/* Read Loop is executed 1 time	*/
#define MxMR_RLFx_2X	0x00008000	/* Read Loop is executed 2 times*/
#define MxMR_RLFx_3X	0x0000c000	/* Read Loop is executed 3 times*/
#define MxMR_RLFx_4X	0x00010000	/* Read Loop is executed 4 times*/
#define MxMR_RLFx_5X	0x00014000	/* Read Loop is executed 5 times*/
#define MxMR_RLFx_6X	0x00018000	/* Read Loop is executed 6 times*/
#define MxMR_RLFx_7X	0x0001c000	/* Read Loop is executed 7 times*/
#define MxMR_RLFx_8X	0x00020000	/* Read Loop is executed 8 times*/
#define MxMR_RLFx_9X	0x00024000	/* Read Loop is executed 9 times*/
#define MxMR_RLFx_10X	0x00028000	/* Read Loop is executed 10 times*/
#define MxMR_RLFx_11X	0x0002c000	/* Read Loop is executed 11 times*/
#define MxMR_RLFx_12X	0x00030000	/* Read Loop is executed 12 times*/
#define MxMR_RLFx_13X	0x00034000	/* Read Loop is executed 13 times*/
#define MxMR_RLFx_14X	0x00038000	/* Read Loop is executed 14 times*/
#define MxMR_RLFx_15X	0x0003c000	/* Read Loop is executed 15 times*/
#define MxMR_RLFx_16X	0x00000000	/* Read Loop is executed 16 times*/

#define MxMR_WLFx_1X	0x00000400	/* Write Loop is executed 1 time*/
#define MxMR_WLFx_2X	0x00000800	/* Write Loop is executed 2 times*/
#define MxMR_WLFx_3X	0x00000c00	/* Write Loop is executed 3 times*/
#define MxMR_WLFx_4X	0x00001000	/* Write Loop is executed 4 times*/
#define MxMR_WLFx_5X	0x00001400	/* Write Loop is executed 5 times*/
#define MxMR_WLFx_6X	0x00001800	/* Write Loop is executed 6 times*/
#define MxMR_WLFx_7X	0x00001c00	/* Write Loop is executed 7 times*/
#define MxMR_WLFx_8X	0x00002000	/* Write Loop is executed 8 times*/
#define MxMR_WLFx_9X	0x00002400	/* Write Loop is executed 9 times*/
#define MxMR_WLFx_10X	0x00002800	/* Write Loop is executed 10 times*/
#define MxMR_WLFx_11X	0x00002c00	/* Write Loop is executed 11 times*/
#define MxMR_WLFx_12X	0x00003000	/* Write Loop is executed 12 times*/
#define MxMR_WLFx_13X	0x00003400	/* Write Loop is executed 13 times*/
#define MxMR_WLFx_14X	0x00003800	/* Write Loop is executed 14 times*/
#define MxMR_WLFx_15X	0x00003c00	/* Write Loop is executed 15 times*/
#define MxMR_WLFx_16X	0x00000000	/* Write Loop is executed 16 times*/

#define MxMR_TLFx_1X	0x00000040	/* Timer Loop is executed 1 time*/
#define MxMR_TLFx_2X	0x00000080	/* Timer Loop is executed 2 times*/
#define MxMR_TLFx_3X	0x000000c0	/* Timer Loop is executed 3 times*/
#define MxMR_TLFx_4X	0x00000100	/* Timer Loop is executed 4 times*/
#define MxMR_TLFx_5X	0x00000140	/* Timer Loop is executed 5 times*/
#define MxMR_TLFx_6X	0x00000180	/* Timer Loop is executed 6 times*/
#define MxMR_TLFx_7X	0x000001c0	/* Timer Loop is executed 7 times*/
#define MxMR_TLFx_8X	0x00000200	/* Timer Loop is executed 8 times*/
#define MxMR_TLFx_9X	0x00000240	/* Timer Loop is executed 9 times*/
#define MxMR_TLFx_10X	0x00000280	/* Timer Loop is executed 10 times*/
#define MxMR_TLFx_11X	0x000002c0	/* Timer Loop is executed 11 times*/
#define MxMR_TLFx_12X	0x00000300	/* Timer Loop is executed 12 times*/
#define MxMR_TLFx_13X	0x00000340	/* Timer Loop is executed 13 times*/
#define MxMR_TLFx_14X	0x00000380	/* Timer Loop is executed 14 times*/
#define MxMR_TLFx_15X	0x000003c0	/* Timer Loop is executed 15 times*/
#define MxMR_TLFx_16X	0x00000000	/* Timer Loop is executed 16 times*/


/*-----------------------------------------------------------------------
 * BRx - Memory Controller: Base Register				10-14
 */
#define BRx_BA_MSK	0xffff8000	/* Base Address Mask		*/
#define BRx_PS_MSK	0x00001800	/* Port Size Mask		*/
#define BRx_DECC_MSK	0x00000600	/* Data Error Correct+Check Mask*/
#define BRx_WP		0x00000100	/* Write Protect		*/
#define BRx_MS_MSK	0x000000e0	/* Machine Select Mask		*/
#define BRx_EMEMC	0x00000010	/* External MEMC Enable		*/
#define BRx_ATOM_MSK	0x0000000c	/* Atomic Operation Mask	*/
#define BRx_DR		0x00000002	/* Data Pipelining		*/
#define BRx_V		0x00000001	/* Bank Valid			*/

#define BRx_PS_64	0x00000000	/* 64 bit port size (60x bus only)*/
#define BRx_PS_8	0x00000800	/*  8 bit port size		*/
#define BRx_PS_16	0x00001000	/* 16 bit port size		*/
#define BRx_PS_32	0x00001800	/* 32 bit port size		*/

#define BRx_DECC_NONE	0x00000000	/* Data Errors Checking Disabled*/
#define BRx_DECC_NORMAL	0x00000200	/* Normal Parity Checking	*/
#define BRx_DECC_RMWPC	0x00000400	/* Read-Modify-Write Parity Checking*/
#define BRx_DECC_ECC	0x00000600	/* ECC Correction and Checking	*/

#define BRx_MS_GPCM_P	0x00000000	/* G.P.C.M. 60x Bus Machine Select*/
#define BRx_MS_GPCM_L	0x00000020	/* G.P.C.M. Local Bus Machine Select*/
#define BRx_MS_SDRAM_P	0x00000040	/* SDRAM 60x Bus Machine Select	*/
#define BRx_MS_SDRAM_L	0x00000060	/* SDRAM Local Bus Machine Select*/
#define BRx_MS_UPMA	0x00000080	/* U.P.M.A Machine Select	*/
#define BRx_MS_UPMB	0x000000a0	/* U.P.M.B Machine Select	*/
#define BRx_MS_UPMC	0x000000c0	/* U.P.M.C Machine Select	*/

#define BRx_ATOM_RAWA	0x00000004	/* Read-After-Write-Atomic	*/
#define BRx_ATOM_WARA	0x00000008	/* Write-After-Read-Atomic	*/

/*-----------------------------------------------------------------------
 * ORx - Memory Controller: Option Register - SDRAM Mode		10-16
 */
#define ORxS_SDAM_MSK	0xfff00000	/* SDRAM Address Mask Mask	*/
#define ORxS_LSDAM_MSK	0x000f8000	/* Lower SDRAM Address Mask Mask*/
#define ORxS_BPD_MSK	0x00006000	/* Banks Per Device Mask	*/
#define ORxS_ROWST_MSK	0x00001e00	/* Row Start Address Bit Mask	*/
#define ORxS_NUMR_MSK	0x000001c0	/* Number of Row Addr Lines Mask*/
#define ORxS_PMSEL	0x00000020	/* Page Mode Select		*/
#define ORxS_IBID	0x00000010	/* Internal Bank Interleaving Disable*/

#define ORxS_BPD_2	0x00000000	/* 2 Banks Per Device		*/
#define ORxS_BPD_4	0x00002000	/* 4 Banks Per Device		*/
#define ORxS_BPD_8	0x00004000	/* 8 Banks Per Device		*/

/* ROWST values for xSDMR[PBI] = 0 */
#define ORxS_ROWST_PBI0_A7  0x00000400	/* Row Start Address Bit is A7	*/
#define ORxS_ROWST_PBI0_A8  0x00000800	/* Row Start Address Bit is A8	*/
#define ORxS_ROWST_PBI0_A9  0x00000c00	/* Row Start Address Bit is A9	*/
#define ORxS_ROWST_PBI0_A10 0x00001000	/* Row Start Address Bit is A10	*/
#define ORxS_ROWST_PBI0_A11 0x00001400	/* Row Start Address Bit is A11	*/
#define ORxS_ROWST_PBI0_A12 0x00001800	/* Row Start Address Bit is A12	*/
#define ORxS_ROWST_PBI0_A13 0x00001c00	/* Row Start Address Bit is A13	*/

/* ROWST values for xSDMR[PBI] = 1 */
#define ORxS_ROWST_PBI1_A0  0x00000000	/* Row Start Address Bit is A0	*/
#define ORxS_ROWST_PBI1_A1  0x00000200	/* Row Start Address Bit is A1	*/
#define ORxS_ROWST_PBI1_A2  0x00000400	/* Row Start Address Bit is A2	*/
#define ORxS_ROWST_PBI1_A3  0x00000600	/* Row Start Address Bit is A3	*/
#define ORxS_ROWST_PBI1_A4  0x00000800	/* Row Start Address Bit is A4	*/
#define ORxS_ROWST_PBI1_A5  0x00000a00	/* Row Start Address Bit is A5	*/
#define ORxS_ROWST_PBI1_A6  0x00000c00	/* Row Start Address Bit is A6	*/
#define ORxS_ROWST_PBI1_A7  0x00000e00	/* Row Start Address Bit is A7	*/
#define ORxS_ROWST_PBI1_A8  0x00001000	/* Row Start Address Bit is A8	*/
#define ORxS_ROWST_PBI1_A9  0x00001200	/* Row Start Address Bit is A9	*/
#define ORxS_ROWST_PBI1_A10 0x00001400	/* Row Start Address Bit is A10	*/
#define ORxS_ROWST_PBI1_A11 0x00001600	/* Row Start Address Bit is A11	*/
#define ORxS_ROWST_PBI1_A12 0x00001800	/* Row Start Address Bit is A12	*/

#define ORxS_NUMR_9	0x00000000	/*  9 Row Address Lines		*/
#define ORxS_NUMR_10	0x00000040	/* 10 Row Address Lines		*/
#define ORxS_NUMR_11	0x00000080	/* 11 Row Address Lines		*/
#define ORxS_NUMR_12	0x000000c0	/* 12 Row Address Lines		*/
#define ORxS_NUMR_13	0x00000100	/* 13 Row Address Lines		*/
#define ORxS_NUMR_14	0x00000140	/* 14 Row Address Lines		*/
#define ORxS_NUMR_15	0x00000180	/* 15 Row Address Lines		*/
#define ORxS_NUMR_16	0x000001c0	/* 16 Row Address Lines		*/

/* helper to determine the AM for a given size (SDRAM mode) */
#define ORxS_SIZE_TO_AM(s) ((~((s) - 1)) & 0xffff8000)	/* must be pow of 2 */

/*-----------------------------------------------------------------------
 * ORx - Memory Controller: Option Register - GPCM Mode			10-18
 */
#define ORxG_AM_MSK	0xffff8000	/* Address Mask Mask		*/
#define ORxG_BCTLD	0x00001000	/* Data Buffer Control Disable	*/
#define ORxG_CSNT	0x00000800	/* Chip Select Negation Time	*/
#define ORxG_ACS_MSK	0x00000600	/* Address to Chip Select Setup mask*/
#define ORxG_SCY_MSK	0x000000f0	/* Cycle Lenght in Clocks	*/
#define ORxG_SETA	0x00000008	/* External Access Termination	*/
#define ORxG_TRLX	0x00000004	/* Timing Relaxed		*/
#define ORxG_EHTR	0x00000002	/* Extended Hold Time on Read	*/

#define ORxG_ACS_DIV1	0x00000000	/* CS is output at the same time*/
#define ORxG_ACS_DIV4	0x00000400	/* CS is output 1/4 a clock later*/
#define ORxG_ACS_DIV2	0x00000600	/* CS is output 1/2 a clock later*/

#define ORxG_SCY_0_CLK	0x00000000	/*  0 clock cycles wait states	*/
#define ORxG_SCY_1_CLK	0x00000010	/*  1 clock cycles wait states	*/
#define ORxG_SCY_2_CLK	0x00000020	/*  2 clock cycles wait states	*/
#define ORxG_SCY_3_CLK	0x00000030	/*  3 clock cycles wait states	*/
#define ORxG_SCY_4_CLK	0x00000040	/*  4 clock cycles wait states	*/
#define ORxG_SCY_5_CLK	0x00000050	/*  5 clock cycles wait states	*/
#define ORxG_SCY_6_CLK	0x00000060	/*  6 clock cycles wait states	*/
#define ORxG_SCY_7_CLK	0x00000070	/*  7 clock cycles wait states	*/
#define ORxG_SCY_8_CLK	0x00000080	/*  8 clock cycles wait states	*/
#define ORxG_SCY_9_CLK	0x00000090	/*  9 clock cycles wait states	*/
#define ORxG_SCY_10_CLK	0x000000a0	/* 10 clock cycles wait states	*/
#define ORxG_SCY_11_CLK	0x000000b0	/* 11 clock cycles wait states	*/
#define ORxG_SCY_12_CLK	0x000000c0	/* 12 clock cycles wait states	*/
#define ORxG_SCY_13_CLK	0x000000d0	/* 13 clock cycles wait states	*/
#define ORxG_SCY_14_CLK	0x000000e0	/* 14 clock cycles wait states	*/
#define ORxG_SCY_15_CLK	0x000000f0	/* 15 clock cycles wait states	*/

/*-----------------------------------------------------------------------
 * ORx - Memory Controller: Option Register - UPM Mode			10-20
 */
#define ORxU_AM_MSK	0xffff8000	/* Address Mask Mask		*/
#define ORxU_BCTLD	0x00001000	/* Data Buffer Control Disable	*/
#define ORxU_BI		0x00000100	/* Burst Inhibit		*/
#define ORxU_EHTR_MSK	0x00000006	/* Extended Hold Time on Read Mask*/

#define ORxU_EHTR_NORM	0x00000000	/* Normal Timing		*/
#define ORxU_EHTR_1IDLE	0x00000002	/* One Idle Clock Cycle Inserted*/
#define ORxU_EHTR_4IDLE	0x00000004	/* Four Idle Clock Cycles Inserted*/
#define ORxU_EHTR_8IDLE	0x00000006	/* Eight Idle Clock Cycles Inserted*/


/* helpers to convert values into an OR address mask (GPCM mode) */
#define P2SZ_TO_AM(s)	((~((s) - 1)) & 0xffff8000)	/* must be pow of 2 */
#define MEG_TO_AM(m)	P2SZ_TO_AM((m) << 20)


/*-----------------------------------------------------------------------
 * PSDMR - 60x SDRAM Mode Register					10-21
 */
#define PSDMR_PBI	     0x80000000	/* Page-based Interleaving	*/
#define PSDMR_RFEN	     0x40000000	/* Refresh Enable		*/
#define PSDMR_OP_MSK	     0x38000000	/* SDRAM Operation Mask		*/
#define PSDMR_SDAM_MSK	     0x07000000	/* SDRAM Address Multiplex Mask	*/
#define PSDMR_BSMA_MSK	     0x00e00000	/* Bank Select Muxd Addr Line Mask*/
#define PSDMR_SDA10_MSK	     0x001c0000	/* A10 Control Mask		*/
#define PSDMR_RFRC_MSK	     0x00038000	/* Refresh Recovery Mask	*/
#define PSDMR_PRETOACT_MSK   0x00007000	/* Precharge to Activate Intvl Mask*/
#define PSDMR_ACTTORW_MSK    0x00000e00	/* Activate to Read/Write Intvl Mask*/
#define PSDMR_BL	     0x00000100	/* Burst Length			*/
#define PSDMR_LDOTOPRE_MSK   0x000000c0	/* Last Data Out to Precharge Mask*/
#define PSDMR_WRC_MSK	     0x00000030	/* Write Recovery Time Mask	*/
#define PSDMR_EAMUX	     0x00000008	/* External Address Multiplexing*/
#define PSDMR_BUFCMD	     0x00000004	/* SDRAM ctl lines asrtd for 2 cycles*/
#define PSDMR_CL_MSK	     0x00000003	/* CAS Latency Mask		*/

#define PSDMR_OP_NORM	     0x00000000	/* Normal Operation		*/
#define PSDMR_OP_CBRR	     0x08000000	/* CBR Refresh			*/
#define PSDMR_OP_SELFR	     0x10000000	/* Self Refresh			*/
#define PSDMR_OP_MRW	     0x18000000	/* Mode Register Write		*/
#define PSDMR_OP_PREB	     0x20000000	/* Precharge Bank		*/
#define PSDMR_OP_PREA	     0x28000000	/* Precharge All Banks		*/
#define PSDMR_OP_ACTB	     0x30000000	/* Activate Bank		*/
#define PSDMR_OP_RW	     0x38000000	/* Read/Write			*/

#define PSDMR_SDAM_A13_IS_A5 0x00000000	/* SDRAM Address Multiplex A13 is A5 */
#define PSDMR_SDAM_A14_IS_A5 0x01000000	/* SDRAM Address Multiplex A14 is A5 */
#define PSDMR_SDAM_A15_IS_A5 0x02000000	/* SDRAM Address Multiplex A15 is A5 */
#define PSDMR_SDAM_A16_IS_A5 0x03000000	/* SDRAM Address Multiplex A16 is A5 */
#define PSDMR_SDAM_A17_IS_A5 0x04000000	/* SDRAM Address Multiplex A17 is A5 */
#define PSDMR_SDAM_A18_IS_A5 0x05000000	/* SDRAM Address Multiplex A18 is A5 */

#define PSDMR_BSMA_A12_A14   0x00000000	/* A12 - A14			*/
#define PSDMR_BSMA_A13_A15   0x00200000	/* A13 - A15			*/
#define PSDMR_BSMA_A14_A16   0x00400000	/* A14 - A16			*/
#define PSDMR_BSMA_A15_A17   0x00600000	/* A15 - A17			*/
#define PSDMR_BSMA_A16_A18   0x00800000	/* A16 - A18			*/
#define PSDMR_BSMA_A17_A19   0x00a00000	/* A17 - A19			*/
#define PSDMR_BSMA_A18_A20   0x00c00000	/* A18 - A20			*/
#define PSDMR_BSMA_A19_A21   0x00e00000	/* A19 - A21			*/

/* SDA10 values for xSDMR[PBI] = 0 */
#define PSDMR_SDA10_PBI0_A12 0x00000000	/* "A10" Control is A12		*/
#define PSDMR_SDA10_PBI0_A11 0x00040000	/* "A10" Control is A11		*/
#define PSDMR_SDA10_PBI0_A10 0x00080000	/* "A10" Control is A10		*/
#define PSDMR_SDA10_PBI0_A9  0x000c0000	/* "A10" Control is A9		*/
#define PSDMR_SDA10_PBI0_A8  0x00100000	/* "A10" Control is A8		*/
#define PSDMR_SDA10_PBI0_A7  0x00140000	/* "A10" Control is A7		*/
#define PSDMR_SDA10_PBI0_A6  0x00180000	/* "A10" Control is A6		*/
#define PSDMR_SDA10_PBI0_A5  0x001c0000	/* "A10" Control is A5		*/

/* SDA10 values for xSDMR[PBI] = 1 */
#define PSDMR_SDA10_PBI1_A10 0x00000000	/* "A10" Control is A10		*/
#define PSDMR_SDA10_PBI1_A9  0x00040000	/* "A10" Control is A9		*/
#define PSDMR_SDA10_PBI1_A8  0x00080000	/* "A10" Control is A8		*/
#define PSDMR_SDA10_PBI1_A7  0x000c0000	/* "A10" Control is A7		*/
#define PSDMR_SDA10_PBI1_A6  0x00100000	/* "A10" Control is A6		*/
#define PSDMR_SDA10_PBI1_A5  0x00140000	/* "A10" Control is A5		*/
#define PSDMR_SDA10_PBI1_A4  0x00180000	/* "A10" Control is A4		*/
#define PSDMR_SDA10_PBI1_A3  0x001c0000	/* "A10" Control is A3		*/

#define PSDMR_RFRC_3_CLK     0x00008000	/*  3 Clocks			*/
#define PSDMR_RFRC_4_CLK     0x00010000	/*  4 Clocks			*/
#define PSDMR_RFRC_5_CLK     0x00018000	/*  5 Clocks			*/
#define PSDMR_RFRC_6_CLK     0x00020000	/*  6 Clocks			*/
#define PSDMR_RFRC_7_CLK     0x00028000	/*  7 Clocks			*/
#define PSDMR_RFRC_8_CLK     0x00030000	/*  8 Clocks			*/
#define PSDMR_RFRC_16_CLK    0x00038000	/* 16 Clocks			*/

#define PSDMR_PRETOACT_8W    0x00000000	/* 8 Clock-cycle Wait States	*/
#define PSDMR_PRETOACT_1W    0x00001000	/* 1 Clock-cycle Wait States	*/
#define PSDMR_PRETOACT_2W    0x00002000	/* 2 Clock-cycle Wait States	*/
#define PSDMR_PRETOACT_3W    0x00003000	/* 3 Clock-cycle Wait States	*/
#define PSDMR_PRETOACT_4W    0x00004000	/* 4 Clock-cycle Wait States	*/
#define PSDMR_PRETOACT_5W    0x00005000	/* 5 Clock-cycle Wait States	*/
#define PSDMR_PRETOACT_6W    0x00006000	/* 6 Clock-cycle Wait States	*/
#define PSDMR_PRETOACT_7W    0x00007000	/* 7 Clock-cycle Wait States	*/

#define PSDMR_ACTTORW_8W     0x00000000	/* 8 Clock-cycle Wait States	*/
#define PSDMR_ACTTORW_1W     0x00000200	/* 1 Clock-cycle Wait States	*/
#define PSDMR_ACTTORW_2W     0x00000400	/* 2 Clock-cycle Wait States	*/
#define PSDMR_ACTTORW_3W     0x00000600	/* 3 Clock-cycle Wait States	*/
#define PSDMR_ACTTORW_4W     0x00000800	/* 4 Clock-cycle Wait States	*/
#define PSDMR_ACTTORW_5W     0x00000a00	/* 5 Clock-cycle Wait States	*/
#define PSDMR_ACTTORW_6W     0x00000c00	/* 6 Clock-cycle Wait States	*/
#define PSDMR_ACTTORW_7W     0x00000e00	/* 7 Clock-cycle Wait States	*/

#define PSDMR_LDOTOPRE_0C    0x00000000	/* 0 Clock Cycles		*/
#define PSDMR_LDOTOPRE_1C    0x00000040	/* 1 Clock Cycles		*/
#define PSDMR_LDOTOPRE_2C    0x00000080	/* 2 Clock Cycles		*/

#define PSDMR_WRC_4C	     0x00000000	/* 4 Clock Cycles		*/
#define PSDMR_WRC_1C	     0x00000010	/* 1 Clock Cycles		*/
#define PSDMR_WRC_2C	     0x00000020	/* 2 Clock Cycles		*/
#define PSDMR_WRC_3C	     0x00000030	/* 3 Clock Cycles		*/

#define PSDMR_CL_1	     0x00000001	/* CAS Latency = 1		*/
#define PSDMR_CL_2	     0x00000002	/* CAS Latency = 2		*/
#define PSDMR_CL_3	     0x00000003	/* CAS Latency = 3		*/

/*-----------------------------------------------------------------------
 * LSDMR - Local Bus SDRAM Mode Register			 	10-24
 */

/*
 * No definitions here - the LSDMR has the same fields as the PSDMR.
 */

/*-----------------------------------------------------------------------
 * MPTPR - Memory Refresh Timer Prescaler Register			10-32
 * See User's Manual Errata for the changed definition (matches the
 * 8xx now).  The wrong prescaler definition causes excessive refreshes
 * (typically "divide by 2" when "divide by 32" is intended) which will
 * cause unnecessary memory subsystem slowdown.
 */
#define MPTPR_PTP_MSK	0xff00		/* Periodic Timers Prescaler Mask */
#define MPTPR_PTP_DIV2	0x2000		/* BRGCLK divided by 2		*/
#define MPTPR_PTP_DIV4	0x1000		/* BRGCLK divided by 4		*/
#define MPTPR_PTP_DIV8	0x0800		/* BRGCLK divided by 8		*/
#define MPTPR_PTP_DIV16	0x0400		/* BRGCLK divided by 16		*/
#define MPTPR_PTP_DIV32	0x0200		/* BRGCLK divided by 32		*/
#define MPTPR_PTP_DIV64	0x0100		/* BRGCLK divided by 64		*/


/*-----------------------------------------------------------------------
 * TGCR1/TGCR2 - Timer Global Configuration Registers			17-4
 */
#define TGCR1_CAS2	0x80		/* Cascade Timer 1 and 2	*/
#define TGCR1_STP2	0x20		/* Stop timer   2		*/
#define TGCR1_RST2	0x10		/* Reset timer  2		*/
#define TGCR1_GM1	0x08		/* Gate Mode for Pin 1		*/
#define TGCR1_STP1	0x02		/* Stop timer   1		*/
#define TGCR1_RST1	0x01		/* Reset timer  1		*/
#define TGCR2_CAS4	0x80		/* Cascade Timer 3 and 4	*/
#define TGCR2_STP4	0x20		/* Stop timer   4		*/
#define TGCR2_RST4	0x10		/* Reset timer  4		*/
#define TGCR2_GM2	0x08		/* Gate Mode for Pin 2		*/
#define TGCR2_STP3	0x02		/* Stop timer   3		*/
#define TGCR2_RST3	0x01		/* Reset timer  3		*/


/*-----------------------------------------------------------------------
 * TMR1-TMR4 - Timer Mode Registers					17-6
 */
#define TMRx_PS_MSK		0xff00	/* Prescaler Value 		*/
#define TMRx_CE_MSK		0x00c0	/* Capture Edge and Enable Interrupt*/
#define TMRx_OM			0x0020	/* Output Mode 			*/
#define TMRx_ORI		0x0010	/* Output Reference Interrupt Enable*/
#define TMRx_FRR		0x0008	/* Free Run/Restart 		*/
#define TMRx_ICLK_MSK		0x0006	/* Timer Input Clock Source mask */
#define TMRx_GE			0x0001	/* Gate Enable  		*/

#define TMRx_CE_INTR_DIS	0x0000	/* Disable Interrupt on capture event*/
#define TMRx_CE_RISING		0x0040	/* Capture on Rising TINx edge only */
#define TMRx_CE_FALLING		0x0080	/* Capture on Falling TINx edge only */
#define TMRx_CE_ANY		0x00c0	/* Capture on any TINx edge 	*/

#define TMRx_ICLK_IN_CAS	0x0000	/* Internally cascaded input 	*/
#define TMRx_ICLK_IN_GEN	0x0002	/* Internal General system clock*/
#define TMRx_ICLK_IN_GEN_DIV16	0x0004	/* Internal General system clk div 16*/
#define TMRx_ICLK_TIN_PIN	0x0006	/* TINx pin 			*/


/*-----------------------------------------------------------------------
 * CMXFCR - CMX FCC Clock Route Register				15-12
 */
#define CMXFCR_FC1	   0x40000000	/* FCC1 connection		*/
#define CMXFCR_RF1CS_MSK   0x38000000	/* Receive FCC1 Clock Source Mask */
#define CMXFCR_TF1CS_MSK   0x07000000	/* Transmit FCC1 Clock Source Mask */
#define CMXFCR_FC2	   0x00400000	/* FCC2 connection		*/
#define CMXFCR_RF2CS_MSK   0x00380000	/* Receive FCC2 Clock Source Mask */
#define CMXFCR_TF2CS_MSK   0x00070000	/* Transmit FCC2 Clock Source Mask */
#define CMXFCR_FC3	   0x00004000	/* FCC3 connection		*/
#define CMXFCR_RF3CS_MSK   0x00003800	/* Receive FCC3 Clock Source Mask */
#define CMXFCR_TF3CS_MSK   0x00000700	/* Transmit FCC3 Clock Source Mask */

#define CMXFCR_RF1CS_BRG5  0x00000000	/* Receive FCC1 Clock Source is BRG5 */
#define CMXFCR_RF1CS_BRG6  0x08000000	/* Receive FCC1 Clock Source is BRG6 */
#define CMXFCR_RF1CS_BRG7  0x10000000	/* Receive FCC1 Clock Source is BRG7 */
#define CMXFCR_RF1CS_BRG8  0x18000000	/* Receive FCC1 Clock Source is BRG8 */
#define CMXFCR_RF1CS_CLK9  0x20000000	/* Receive FCC1 Clock Source is CLK9 */
#define CMXFCR_RF1CS_CLK10 0x28000000	/* Receive FCC1 Clock Source is CLK10 */
#define CMXFCR_RF1CS_CLK11 0x30000000	/* Receive FCC1 Clock Source is CLK11 */
#define CMXFCR_RF1CS_CLK12 0x38000000	/* Receive FCC1 Clock Source is CLK12 */

#define CMXFCR_TF1CS_BRG5  0x00000000	/* Transmit FCC1 Clock Source is BRG5 */
#define CMXFCR_TF1CS_BRG6  0x01000000	/* Transmit FCC1 Clock Source is BRG6 */
#define CMXFCR_TF1CS_BRG7  0x02000000	/* Transmit FCC1 Clock Source is BRG7 */
#define CMXFCR_TF1CS_BRG8  0x03000000	/* Transmit FCC1 Clock Source is BRG8 */
#define CMXFCR_TF1CS_CLK9  0x04000000	/* Transmit FCC1 Clock Source is CLK9 */
#define CMXFCR_TF1CS_CLK10 0x05000000	/* Transmit FCC1 Clock Source is CLK10 */
#define CMXFCR_TF1CS_CLK11 0x06000000	/* Transmit FCC1 Clock Source is CLK11 */
#define CMXFCR_TF1CS_CLK12 0x07000000	/* Transmit FCC1 Clock Source is CLK12 */

#define CMXFCR_RF2CS_BRG5  0x00000000	/* Receive FCC2 Clock Source is BRG5 */
#define CMXFCR_RF2CS_BRG6  0x00080000	/* Receive FCC2 Clock Source is BRG6 */
#define CMXFCR_RF2CS_BRG7  0x00100000	/* Receive FCC2 Clock Source is BRG7 */
#define CMXFCR_RF2CS_BRG8  0x00180000	/* Receive FCC2 Clock Source is BRG8 */
#define CMXFCR_RF2CS_CLK13 0x00200000	/* Receive FCC2 Clock Source is CLK13 */
#define CMXFCR_RF2CS_CLK14 0x00280000	/* Receive FCC2 Clock Source is CLK14 */
#define CMXFCR_RF2CS_CLK15 0x00300000	/* Receive FCC2 Clock Source is CLK15 */
#define CMXFCR_RF2CS_CLK16 0x00380000	/* Receive FCC2 Clock Source is CLK16 */

#define CMXFCR_TF2CS_BRG5  0x00000000	/* Transmit FCC2 Clock Source is BRG5 */
#define CMXFCR_TF2CS_BRG6  0x00010000	/* Transmit FCC2 Clock Source is BRG6 */
#define CMXFCR_TF2CS_BRG7  0x00020000	/* Transmit FCC2 Clock Source is BRG7 */
#define CMXFCR_TF2CS_BRG8  0x00030000	/* Transmit FCC2 Clock Source is BRG8 */
#define CMXFCR_TF2CS_CLK13 0x00040000	/* Transmit FCC2 Clock Source is CLK13 */
#define CMXFCR_TF2CS_CLK14 0x00050000	/* Transmit FCC2 Clock Source is CLK14 */
#define CMXFCR_TF2CS_CLK15 0x00060000	/* Transmit FCC2 Clock Source is CLK15 */
#define CMXFCR_TF2CS_CLK16 0x00070000	/* Transmit FCC2 Clock Source is CLK16 */

#define CMXFCR_RF3CS_BRG5  0x00000000	/* Receive FCC3 Clock Source is BRG5 */
#define CMXFCR_RF3CS_BRG6  0x00000800	/* Receive FCC3 Clock Source is BRG6 */
#define CMXFCR_RF3CS_BRG7  0x00001000	/* Receive FCC3 Clock Source is BRG7 */
#define CMXFCR_RF3CS_BRG8  0x00001800	/* Receive FCC3 Clock Source is BRG8 */
#define CMXFCR_RF3CS_CLK13 0x00002000	/* Receive FCC3 Clock Source is CLK13 */
#define CMXFCR_RF3CS_CLK14 0x00002800	/* Receive FCC3 Clock Source is CLK14 */
#define CMXFCR_RF3CS_CLK15 0x00003000	/* Receive FCC3 Clock Source is CLK15 */
#define CMXFCR_RF3CS_CLK16 0x00003800	/* Receive FCC3 Clock Source is CLK16 */

#define CMXFCR_TF3CS_BRG5  0x00000000	/* Transmit FCC3 Clock Source is BRG5 */
#define CMXFCR_TF3CS_BRG6  0x00000100	/* Transmit FCC3 Clock Source is BRG6 */
#define CMXFCR_TF3CS_BRG7  0x00000200	/* Transmit FCC3 Clock Source is BRG7 */
#define CMXFCR_TF3CS_BRG8  0x00000300	/* Transmit FCC3 Clock Source is BRG8 */
#define CMXFCR_TF3CS_CLK13 0x00000400	/* Transmit FCC3 Clock Source is CLK13 */
#define CMXFCR_TF3CS_CLK14 0x00000500	/* Transmit FCC3 Clock Source is CLK14 */
#define CMXFCR_TF3CS_CLK15 0x00000600	/* Transmit FCC3 Clock Source is CLK15 */
#define CMXFCR_TF3CS_CLK16 0x00000700	/* Transmit FCC3 Clock Source is CLK16 */

/*-----------------------------------------------------------------------
 * CMXSCR - CMX SCC Clock Route Register				15-14
 */
#define CMXSCR_GR1	   0x80000000	/* Grant Support of SCC1	*/
#define CMXSCR_SC1	   0x40000000	/* SCC1 connection		*/
#define CMXSCR_RS1CS_MSK   0x38000000	/* Receive SCC1 Clock Source Mask */
#define CMXSCR_TS1CS_MSK   0x07000000	/* Transmit SCC1 Clock Source Mask */
#define CMXSCR_GR2	   0x00800000	/* Grant Support of SCC2	*/
#define CMXSCR_SC2	   0x00400000	/* SCC2 connection		*/
#define CMXSCR_RS2CS_MSK   0x00380000	/* Receive SCC2 Clock Source Mask */
#define CMXSCR_TS2CS_MSK   0x00070000	/* Transmit SCC2 Clock Source Mask */
#define CMXSCR_GR3	   0x00008000	/* Grant Support of SCC3	*/
#define CMXSCR_SC3	   0x00004000	/* SCC3 connection		*/
#define CMXSCR_RS3CS_MSK   0x00003800	/* Receive SCC3 Clock Source Mask */
#define CMXSCR_TS3CS_MSK   0x00000700	/* Transmit SCC3 Clock Source Mask */
#define CMXSCR_GR4	   0x00000080	/* Grant Support of SCC4	*/
#define CMXSCR_SC4	   0x00000040	/* SCC4 connection		*/
#define CMXSCR_RS4CS_MSK   0x00000038	/* Receive SCC4 Clock Source Mask */
#define CMXSCR_TS4CS_MSK   0x00000007	/* Transmit SCC4 Clock Source Mask */

#define CMXSCR_RS1CS_BRG1  0x00000000	/* SCC1 Rx Clock Source is BRG1 */
#define CMXSCR_RS1CS_BRG2  0x08000000	/* SCC1 Rx Clock Source is BRG2 */
#define CMXSCR_RS1CS_BRG3  0x10000000	/* SCC1 Rx Clock Source is BRG3 */
#define CMXSCR_RS1CS_BRG4  0x18000000	/* SCC1 Rx Clock Source is BRG4 */
#define CMXSCR_RS1CS_CLK11 0x20000000	/* SCC1 Rx Clock Source is CLK11 */
#define CMXSCR_RS1CS_CLK12 0x28000000	/* SCC1 Rx Clock Source is CLK12 */
#define CMXSCR_RS1CS_CLK3  0x30000000	/* SCC1 Rx Clock Source is CLK3 */
#define CMXSCR_RS1CS_CLK4  0x38000000	/* SCC1 Rx Clock Source is CLK4 */

#define CMXSCR_TS1CS_BRG1  0x00000000	/* SCC1 Tx Clock Source is BRG1 */
#define CMXSCR_TS1CS_BRG2  0x01000000	/* SCC1 Tx Clock Source is BRG2 */
#define CMXSCR_TS1CS_BRG3  0x02000000	/* SCC1 Tx Clock Source is BRG3 */
#define CMXSCR_TS1CS_BRG4  0x03000000	/* SCC1 Tx Clock Source is BRG4 */
#define CMXSCR_TS1CS_CLK11 0x04000000	/* SCC1 Tx Clock Source is CLK11 */
#define CMXSCR_TS1CS_CLK12 0x05000000	/* SCC1 Tx Clock Source is CLK12 */
#define CMXSCR_TS1CS_CLK3  0x06000000	/* SCC1 Tx Clock Source is CLK3 */
#define CMXSCR_TS1CS_CLK4  0x07000000	/* SCC1 Tx Clock Source is CLK4 */

#define CMXSCR_RS2CS_BRG1  0x00000000	/* SCC2 Rx Clock Source is BRG1 */
#define CMXSCR_RS2CS_BRG2  0x00080000	/* SCC2 Rx Clock Source is BRG2 */
#define CMXSCR_RS2CS_BRG3  0x00100000	/* SCC2 Rx Clock Source is BRG3 */
#define CMXSCR_RS2CS_BRG4  0x00180000	/* SCC2 Rx Clock Source is BRG4 */
#define CMXSCR_RS2CS_CLK11 0x00200000	/* SCC2 Rx Clock Source is CLK11 */
#define CMXSCR_RS2CS_CLK12 0x00280000	/* SCC2 Rx Clock Source is CLK12 */
#define CMXSCR_RS2CS_CLK3  0x00300000	/* SCC2 Rx Clock Source is CLK3 */
#define CMXSCR_RS2CS_CLK4  0x00380000	/* SCC2 Rx Clock Source is CLK4 */

#define CMXSCR_TS2CS_BRG1  0x00000000	/* SCC2 Tx Clock Source is BRG1 */
#define CMXSCR_TS2CS_BRG2  0x00010000	/* SCC2 Tx Clock Source is BRG2 */
#define CMXSCR_TS2CS_BRG3  0x00020000	/* SCC2 Tx Clock Source is BRG3 */
#define CMXSCR_TS2CS_BRG4  0x00030000	/* SCC2 Tx Clock Source is BRG4 */
#define CMXSCR_TS2CS_CLK11 0x00040000	/* SCC2 Tx Clock Source is CLK11 */
#define CMXSCR_TS2CS_CLK12 0x00050000	/* SCC2 Tx Clock Source is CLK12 */
#define CMXSCR_TS2CS_CLK3  0x00060000	/* SCC2 Tx Clock Source is CLK3 */
#define CMXSCR_TS2CS_CLK4  0x00070000	/* SCC2 Tx Clock Source is CLK4 */

#define CMXSCR_RS3CS_BRG1  0x00000000	/* SCC3 Rx Clock Source is BRG1 */
#define CMXSCR_RS3CS_BRG2  0x00000800	/* SCC3 Rx Clock Source is BRG2 */
#define CMXSCR_RS3CS_BRG3  0x00001000	/* SCC3 Rx Clock Source is BRG3 */
#define CMXSCR_RS3CS_BRG4  0x00001800	/* SCC3 Rx Clock Source is BRG4 */
#define CMXSCR_RS3CS_CLK5  0x00002000	/* SCC3 Rx Clock Source is CLK5 */
#define CMXSCR_RS3CS_CLK6  0x00002800	/* SCC3 Rx Clock Source is CLK6 */
#define CMXSCR_RS3CS_CLK7  0x00003000	/* SCC3 Rx Clock Source is CLK7 */
#define CMXSCR_RS3CS_CLK8  0x00003800	/* SCC3 Rx Clock Source is CLK8 */

#define CMXSCR_TS3CS_BRG1  0x00000000	/* SCC3 Tx Clock Source is BRG1 */
#define CMXSCR_TS3CS_BRG2  0x00000100	/* SCC3 Tx Clock Source is BRG2 */
#define CMXSCR_TS3CS_BRG3  0x00000200	/* SCC3 Tx Clock Source is BRG3 */
#define CMXSCR_TS3CS_BRG4  0x00000300	/* SCC3 Tx Clock Source is BRG4 */
#define CMXSCR_TS3CS_CLK5  0x00000400	/* SCC3 Tx Clock Source is CLK5 */
#define CMXSCR_TS3CS_CLK6  0x00000500	/* SCC3 Tx Clock Source is CLK6 */
#define CMXSCR_TS3CS_CLK7  0x00000600	/* SCC3 Tx Clock Source is CLK7 */
#define CMXSCR_TS3CS_CLK8  0x00000700	/* SCC3 Tx Clock Source is CLK8 */

#define CMXSCR_RS4CS_BRG1  0x00000000	/* SCC4 Rx Clock Source is BRG1 */
#define CMXSCR_RS4CS_BRG2  0x00000008	/* SCC4 Rx Clock Source is BRG2 */
#define CMXSCR_RS4CS_BRG3  0x00000010	/* SCC4 Rx Clock Source is BRG3 */
#define CMXSCR_RS4CS_BRG4  0x00000018	/* SCC4 Rx Clock Source is BRG4 */
#define CMXSCR_RS4CS_CLK5  0x00000020	/* SCC4 Rx Clock Source is CLK5 */
#define CMXSCR_RS4CS_CLK6  0x00000028	/* SCC4 Rx Clock Source is CLK6 */
#define CMXSCR_RS4CS_CLK7  0x00000030	/* SCC4 Rx Clock Source is CLK7 */
#define CMXSCR_RS4CS_CLK8  0x00000038	/* SCC4 Rx Clock Source is CLK8 */

#define CMXSCR_TS4CS_BRG1  0x00000000	/* SCC4 Tx Clock Source is BRG1 */
#define CMXSCR_TS4CS_BRG2  0x00000001	/* SCC4 Tx Clock Source is BRG2 */
#define CMXSCR_TS4CS_BRG3  0x00000002	/* SCC4 Tx Clock Source is BRG3 */
#define CMXSCR_TS4CS_BRG4  0x00000003	/* SCC4 Tx Clock Source is BRG4 */
#define CMXSCR_TS4CS_CLK5  0x00000004	/* SCC4 Tx Clock Source is CLK5 */
#define CMXSCR_TS4CS_CLK6  0x00000005	/* SCC4 Tx Clock Source is CLK6 */
#define CMXSCR_TS4CS_CLK7  0x00000006	/* SCC4 Tx Clock Source is CLK7 */
#define CMXSCR_TS4CS_CLK8  0x00000007	/* SCC4 Tx Clock Source is CLK8 */

/*-----------------------------------------------------------------------
 * CMXSMR - CMX SMC Clock Route Register				15-17
 */
#define CMXSMR_SMC1	    0x80	/* SMC1 Connection		*/
#define CMXSMR_SMC1CS_MSK   0x30	/* SMC1 Clock Source		*/
#define CMXSMR_SMC2	    0x08	/* SMC2 Connection		*/
#define CMXSMR_SMC2CS_MSK   0x03	/* SMC2 Clock Source		*/

#define CMXSMR_SMC1CS_BRG1  0x00	/* SMC1 Tx and Rx Clocks are BRG1 */
#define CMXSMR_SMC1CS_BRG7  0x10	/* SMC1 Tx and Rx Clocks are BRG7 */
#define CMXSMR_SMC1CS_CLK7  0x20	/* SMC1 Tx and Rx Clocks are CLK7 */
#define CMXSMR_SMC1CS_CLK9  0x30	/* SMC1 Tx and Rx Clocks are CLK9 */

#define CMXSMR_SMC2CS_BRG2  0x00	/* SMC2 Tx and Rx Clocks are BRG2 */
#define CMXSMR_SMC2CS_BRG8  0x01	/* SMC2 Tx and Rx Clocks are BRG8 */
#define CMXSMR_SMC2CS_CLK19 0x02	/* SMC2 Tx and Rx Clocks are CLK19 */
#define CMXSMR_SMC2CS_CLK20 0x03	/* SMC2 Tx and Rx Clocks are CLK20 */

/*-----------------------------------------------------------------------
 * miscellaneous
 */

#define UPMA			1
#define UPMB			2
#define UPMC			3

#if !defined(__ASSEMBLY__) && defined(CONFIG_WATCHDOG)
extern __inline__ void
reset_8260_watchdog(volatile immap_t *immr)
{
    immr->im_siu_conf.sc_swsr = 0x556c;
    immr->im_siu_conf.sc_swsr = 0xaa39;
}
#endif /* !__ASSEMBLY && CONFIG_WATCHDOG */

#endif	/* __MPC8260_H__ */
