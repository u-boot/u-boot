/*----------------------------------------------------------------------------+
|
|	This source code has been made available to you by IBM on an AS-IS
|	basis.	Anyone receiving this source is licensed under IBM
|	copyrights to use it in any way he or she deems fit, including
|	copying it, modifying it, compiling it, and redistributing it either
|	with or without modifications.	No license under IBM patents or
|	patent applications is to be implied by the copyright license.
|
|	Any user of this software should understand that IBM cannot provide
|	technical support for this software and will not be responsible for
|	any consequences resulting from the use of this software.
|
|	Any person who transfers this source code or any derivative work
|	must include the IBM copyright notice, this paragraph, and the
|	preceding two paragraphs in the transferred software.
|
|	COPYRIGHT   I B M   CORPORATION 1999
|	LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+----------------------------------------------------------------------------*/

/*
 * (C) Copyright 2006
 * Sylvie Gohl,             AMCC/IBM, gohl.sylvie@fr.ibm.com
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Thierry Roman,           AMCC/IBM, thierry_roman@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 * Robert Snyder,           AMCC/IBM, rob.snyder@fr.ibm.com
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

#ifndef __PPC440_H__
#define __PPC440_H__

#define CFG_DCACHE_SIZE		(32 << 10)	/* For AMCC 440 CPUs	*/

/*--------------------------------------------------------------------- */
/* Special Purpose Registers						*/
/*--------------------------------------------------------------------- */
#define	 xer_reg 0x001
#define	 lr_reg	0x008
#define	 dec	0x016	/* decrementer */
#define	 srr0	0x01a	/* save/restore register 0 */
#define	 srr1	0x01b	/* save/restore register 1 */
#define	 pid	0x030	/* process id */
#define	 decar	0x036	/* decrementer auto-reload */
#define	 csrr0	0x03a	/* critical save/restore register 0 */
#define	 csrr1	0x03b	/* critical save/restore register 1 */
#define	 dear	0x03d	/* data exception address register */
#define	 esr	0x03e	/* exception syndrome register */
#define	 ivpr	0x03f	/* interrupt prefix register */
#define	 usprg0 0x100	/* user special purpose register general 0 */
#define	 usprg1 0x110	/* user special purpose register general 1 */
#define	 tblr	0x10c	/* time base lower, read only */
#define	 tbur	0x10d	/* time base upper, read only */
#define	 sprg1	0x111	/* special purpose register general 1 */
#define	 sprg2	0x112	/* special purpose register general 2 */
#define	 sprg3	0x113	/* special purpose register general 3 */
#define	 sprg4	0x114	/* special purpose register general 4 */
#define	 sprg5	0x115	/* special purpose register general 5 */
#define	 sprg6	0x116	/* special purpose register general 6 */
#define	 sprg7	0x117	/* special purpose register general 7 */
#define	 tbl	0x11c	/* time base lower (supervisor)*/
#define	 tbu	0x11d	/* time base upper (supervisor)*/
#define	 pir	0x11e	/* processor id register */
/*#define  pvr	0x11f	 processor version register */
#define	 dbsr	0x130	/* debug status register */
#define	 dbcr0	0x134	/* debug control register 0 */
#define	 dbcr1	0x135	/* debug control register 1 */
#define	 dbcr2	0x136	/* debug control register 2 */
#define	 iac1	0x138	/* instruction address compare 1 */
#define	 iac2	0x139	/* instruction address compare 2 */
#define	 iac3	0x13a	/* instruction address compare 3 */
#define	 iac4	0x13b	/* instruction address compare 4 */
#define	 dac1	0x13c	/* data address compare 1 */
#define	 dac2	0x13d	/* data address compare 2 */
#define	 dvc1	0x13e	/* data value compare 1 */
#define	 dvc2	0x13f	/* data value compare 2 */
#define	 tsr	0x150	/* timer status register */
#define	 tcr	0x154	/* timer control register */
#define	 ivor0	0x190	/* interrupt vector offset register 0 */
#define	 ivor1	0x191	/* interrupt vector offset register 1 */
#define	 ivor2	0x192	/* interrupt vector offset register 2 */
#define	 ivor3	0x193	/* interrupt vector offset register 3 */
#define	 ivor4	0x194	/* interrupt vector offset register 4 */
#define	 ivor5	0x195	/* interrupt vector offset register 5 */
#define	 ivor6	0x196	/* interrupt vector offset register 6 */
#define	 ivor7	0x197	/* interrupt vector offset register 7 */
#define	 ivor8	0x198	/* interrupt vector offset register 8 */
#define	 ivor9	0x199	/* interrupt vector offset register 9 */
#define	 ivor10 0x19a	/* interrupt vector offset register 10 */
#define	 ivor11 0x19b	/* interrupt vector offset register 11 */
#define	 ivor12 0x19c	/* interrupt vector offset register 12 */
#define	 ivor13 0x19d	/* interrupt vector offset register 13 */
#define	 ivor14 0x19e	/* interrupt vector offset register 14 */
#define	 ivor15 0x19f	/* interrupt vector offset register 15 */
#if defined(CONFIG_440)
#define	 mcsrr0 0x23a	/* machine check save/restore register 0 */
#define	 mcsrr1 0x23b	/* mahcine check save/restore register 1 */
#define	 mcsr	0x23c	/* machine check status register */
#endif
#define	 inv0	0x370	/* instruction cache normal victim 0 */
#define	 inv1	0x371	/* instruction cache normal victim 1 */
#define	 inv2	0x372	/* instruction cache normal victim 2 */
#define	 inv3	0x373	/* instruction cache normal victim 3 */
#define	 itv0	0x374	/* instruction cache transient victim 0 */
#define	 itv1	0x375	/* instruction cache transient victim 1 */
#define	 itv2	0x376	/* instruction cache transient victim 2 */
#define	 itv3	0x377	/* instruction cache transient victim 3 */
#define	 dnv0	0x390	/* data cache normal victim 0 */
#define	 dnv1	0x391	/* data cache normal victim 1 */
#define	 dnv2	0x392	/* data cache normal victim 2 */
#define	 dnv3	0x393	/* data cache normal victim 3 */
#define	 dtv0	0x394	/* data cache transient victim 0 */
#define	 dtv1	0x395	/* data cache transient victim 1 */
#define	 dtv2	0x396	/* data cache transient victim 2 */
#define	 dtv3	0x397	/* data cache transient victim 3 */
#define	 dvlim	0x398	/* data cache victim limit */
#define	 ivlim	0x399	/* instruction cache victim limit */
#define	 rstcfg 0x39b	/* reset configuration */
#define	 dcdbtrl 0x39c	/* data cache debug tag register low */
#define	 dcdbtrh 0x39d	/* data cache debug tag register high */
#define	 icdbtrl 0x39e	/* instruction cache debug tag register low */
#define	 icdbtrh 0x39f	/* instruction cache debug tag register high */
#define	 mmucr	0x3b2	/* mmu control register */
#define	 ccr0	0x3b3	/* core configuration register 0 */
#define  ccr1	0x378	/* core configuration for 440x5 only */
#define	 icdbdr 0x3d3	/* instruction cache debug data register */
#define	 dbdr	0x3f3	/* debug data register */

/******************************************************************************
 * DCRs & Related
 ******************************************************************************/

/*-----------------------------------------------------------------------------
 | Clocking Controller
 +----------------------------------------------------------------------------*/
/* values for clkcfga register - indirect addressing of these regs */
#define clk_clkukpd	0x0020
#define clk_pllc	0x0040
#define clk_plld	0x0060
#define clk_primad	0x0080
#define clk_primbd	0x00a0
#define clk_opbd	0x00c0
#define clk_perd	0x00e0
#define clk_mald	0x0100
#define clk_spcid	0x0120
#define clk_icfg	0x0140

/* 440gx sdr register definations */
#define sdr_sdstp0	0x0020	    /* */
#define sdr_sdstp1	0x0021	    /* */
#define SDR_PINSTP	0x0040
#define sdr_sdcs	0x0060
#define sdr_ecid0	0x0080
#define sdr_ecid1	0x0081
#define sdr_ecid2	0x0082
#define sdr_jtag	0x00c0
#if !defined(CONFIG_440EPX) && !defined(CONFIG_440GRX)
#define sdr_ddrdl	0x00e0
#else
#define sdr_cfg		0x00e0
#define SDR_CFG_LT2_MASK          0x01000000 /* Leakage test 2*/
#define SDR_CFG_64_32BITS_MASK    0x01000000 /* Switch DDR 64 bits or 32 bits */
#define SDR_CFG_32BITS            0x00000000  /* 32 bits */
#define SDR_CFG_64BITS            0x01000000  /* 64 bits */
#define SDR_CFG_MC_V2518_MASK     0x02000000 /* Low VDD2518 (2.5 or 1.8V) */
#define SDR_CFG_MC_V25            0x00000000  /* 2.5 V */
#define SDR_CFG_MC_V18            0x02000000  /* 1.8 V */
#endif /* !defined(CONFIG_440EPX) && !defined(CONFIG_440GRX) */
#define sdr_ebc		0x0100
#define sdr_uart0	0x0120	/* UART0 Config */
#define sdr_uart1	0x0121	/* UART1 Config */
#define sdr_uart2	0x0122	/* UART2 Config */
#define sdr_uart3	0x0123	/* UART3 Config */
#define sdr_cp440	0x0180
#define sdr_xcr		0x01c0
#define sdr_xpllc	0x01c1
#define sdr_xplld	0x01c2
#define sdr_srst	0x0200
#define sdr_slpipe	0x0220
#define sdr_amp0        0x0240  /* Override PLB4 prioritiy for up to 8 masters */
#define sdr_amp1        0x0241  /* Override PLB3 prioritiy for up to 8 masters */
#define sdr_mirq0	0x0260
#define sdr_mirq1	0x0261
#define sdr_maltbl	0x0280
#define sdr_malrbl	0x02a0
#define sdr_maltbs	0x02c0
#define sdr_malrbs	0x02e0
#define sdr_pci0	0x0300
#define sdr_usb0	0x0320
#define sdr_cust0	0x4000
#define sdr_cust1	0x4002
#define sdr_pfc0	0x4100	/* Pin Function 0 */
#define sdr_pfc1	0x4101	/* Pin Function 1 */
#define sdr_plbtr	0x4200
#define sdr_mfr		0x4300	/* SDR0_MFR reg */

/*-----------------------------------------------------------------------------
 | SDRAM Controller
 +----------------------------------------------------------------------------*/
/* values for memcfga register - indirect addressing of these regs	    */
#define mem_besr0_clr	0x0000	/* bus error status reg 0 (clr)		    */
#define mem_besr0_set	0x0004	/* bus error status reg 0 (set)		    */
#define mem_besr1_clr	0x0008	/* bus error status reg 1 (clr)		    */
#define mem_besr1_set	0x000c	/* bus error status reg 1 (set)		    */
#define mem_bear	0x0010	/* bus error address reg		    */
#define mem_mirq_clr	0x0011	/* bus master interrupt (clr)		    */
#define mem_mirq_set	0x0012	/* bus master interrupt (set)		    */
#define mem_slio	0x0018	/* ddr sdram slave interface options	    */
#define mem_cfg0	0x0020	/* ddr sdram options 0			    */
#define mem_cfg1	0x0021	/* ddr sdram options 1			    */
#define mem_devopt	0x0022	/* ddr sdram device options		    */
#define mem_mcsts	0x0024	/* memory controller status		    */
#define mem_rtr		0x0030	/* refresh timer register		    */
#define mem_pmit	0x0034	/* power management idle timer		    */
#define mem_uabba	0x0038	/* plb UABus base address		    */
#define mem_b0cr	0x0040	/* ddr sdram bank 0 configuration	    */
#define mem_b1cr	0x0044	/* ddr sdram bank 1 configuration	    */
#define mem_b2cr	0x0048	/* ddr sdram bank 2 configuration	    */
#define mem_b3cr	0x004c	/* ddr sdram bank 3 configuration	    */
#define mem_tr0		0x0080	/* sdram timing register 0		    */
#define mem_tr1		0x0081	/* sdram timing register 1		    */
#define mem_clktr	0x0082	/* ddr clock timing register		    */
#define mem_wddctr	0x0083	/* write data/dm/dqs clock timing reg	    */
#define mem_dlycal	0x0084	/* delay line calibration register	    */
#define mem_eccesr	0x0098	/* ECC error status			    */

#ifdef CONFIG_440GX
#define sdr_amp		0x0240
#define sdr_xpllc	0x01c1
#define sdr_xplld	0x01c2
#define sdr_xcr		0x01c0
#define sdr_sdstp2	0x4001
#define sdr_sdstp3	0x4003
#endif	/* CONFIG_440GX */

/*----------------------------------------------------------------------------+
| Core Configuration/MMU configuration for 440 (CCR1 for 440x5 only).
+----------------------------------------------------------------------------*/
#define CCR0_PRE		0x40000000
#define CCR0_CRPE		0x08000000
#define CCR0_DSTG		0x00200000
#define CCR0_DAPUIB		0x00100000
#define CCR0_DTB		0x00008000
#define CCR0_GICBT		0x00004000
#define CCR0_GDCBT		0x00002000
#define CCR0_FLSTA		0x00000100
#define CCR0_ICSLC_MASK		0x0000000C
#define CCR0_ICSLT_MASK		0x00000003
#define CCR1_TCS_MASK		0x00000080
#define CCR1_TCS_INTCLK		0x00000000
#define CCR1_TCS_EXTCLK		0x00000080
#define MMUCR_SWOA		0x01000000
#define MMUCR_U1TE		0x00400000
#define MMUCR_U2SWOAE		0x00200000
#define MMUCR_DULXE		0x00800000
#define MMUCR_IULXE		0x00400000
#define MMUCR_STS		0x00100000
#define MMUCR_STID_MASK		0x000000FF

#ifdef CONFIG_440SPE
#undef sdr_sdstp2
#define sdr_sdstp2	0x0022
#undef sdr_sdstp3
#define sdr_sdstp3	0x0023
#define sdr_ddr0	0x00E1
#define sdr_uart2	0x0122
#define sdr_xcr0	0x01c0
/* #define sdr_xcr1	0x01c3	only one PCIX - SG */
/* #define sdr_xcr2	0x01c6	only one PCIX - SG */
#define sdr_xpllc0	0x01c1
#define sdr_xplld0	0x01c2
#define sdr_xpllc1	0x01c4	/*notRCW  - SG */
#define sdr_xplld1	0x01c5	/*notRCW  - SG */
#define sdr_xpllc2	0x01c7	/*notRCW  - SG */
#define sdr_xplld2	0x01c8	/*notRCW  - SG */
#define sdr_amp0	0x0240
#define sdr_amp1	0x0241
#define sdr_cust2	0x4004
#define sdr_cust3	0x4006
#define sdr_sdstp4	0x4001
#define sdr_sdstp5	0x4003
#define sdr_sdstp6	0x4005
#define sdr_sdstp7	0x4007

/******************************************************************************
 * PCI express defines
 ******************************************************************************/
#define SDR0_PE0UTLSET1		0x00000300	/* PE0 Upper transaction layer conf setting */
#define SDR0_PE0UTLSET2		0x00000301	/* PE0 Upper transaction layer conf setting 2 */
#define SDR0_PE0DLPSET		0x00000302	/* PE0 Data link & logical physical configuration */
#define SDR0_PE0LOOP		0x00000303	/* PE0 Loopback interface status */
#define SDR0_PE0RCSSET		0x00000304	/* PE0 Reset, clock & shutdown setting */
#define SDR0_PE0RCSSTS		0x00000305	/* PE0 Reset, clock & shutdown status */
#define SDR0_PE0HSSSET1L0	0x00000306	/* PE0 HSS Control Setting 1: Lane 0 */
#define SDR0_PE0HSSSET2L0	0x00000307	/* PE0 HSS Control Setting 2: Lane 0 */
#define SDR0_PE0HSSSTSL0	0x00000308	/* PE0 HSS Control Status : Lane 0 */
#define SDR0_PE0HSSSET1L1	0x00000309	/* PE0 HSS Control Setting 1: Lane 1 */
#define SDR0_PE0HSSSET2L1	0x0000030A	/* PE0 HSS Control Setting 2: Lane 1 */
#define SDR0_PE0HSSSTSL1	0x0000030B	/* PE0 HSS Control Status : Lane 1 */
#define SDR0_PE0HSSSET1L2	0x0000030C	/* PE0 HSS Control Setting 1: Lane 2 */
#define SDR0_PE0HSSSET2L2	0x0000030D	/* PE0 HSS Control Setting 2: Lane 2 */
#define SDR0_PE0HSSSTSL2	0x0000030E	/* PE0 HSS Control Status : Lane 2 */
#define SDR0_PE0HSSSET1L3	0x0000030F	/* PE0 HSS Control Setting 1: Lane 3 */
#define SDR0_PE0HSSSET2L3	0x00000310	/* PE0 HSS Control Setting 2: Lane 3 */
#define SDR0_PE0HSSSTSL3	0x00000311	/* PE0 HSS Control Status : Lane 3 */
#define SDR0_PE0HSSSET1L4	0x00000312	/* PE0 HSS Control Setting 1: Lane 4 */
#define SDR0_PE0HSSSET2L4	0x00000313	/* PE0 HSS Control Setting 2: Lane 4 */
#define SDR0_PE0HSSSTSL4	0x00000314	/* PE0 HSS Control Status : Lane 4 */
#define SDR0_PE0HSSSET1L5	0x00000315	/* PE0 HSS Control Setting 1: Lane 5 */
#define SDR0_PE0HSSSET2L5	0x00000316	/* PE0 HSS Control Setting 2: Lane 5 */
#define SDR0_PE0HSSSTSL5	0x00000317	/* PE0 HSS Control Status : Lane 5 */
#define SDR0_PE0HSSSET1L6	0x00000318	/* PE0 HSS Control Setting 1: Lane 6 */
#define SDR0_PE0HSSSET2L6	0x00000319	/* PE0 HSS Control Setting 2: Lane 6 */
#define SDR0_PE0HSSSTSL6	0x0000031A	/* PE0 HSS Control Status : Lane 6 */
#define SDR0_PE0HSSSET1L7	0x0000031B	/* PE0 HSS Control Setting 1: Lane 7 */
#define SDR0_PE0HSSSET2L7	0x0000031C	/* PE0 HSS Control Setting 2: Lane 7 */
#define SDR0_PE0HSSSTSL7	0x0000031D	/* PE0 HSS Control Status : Lane 7 */
#define SDR0_PE0HSSSEREN	0x0000031E	/* PE0 Serdes Transmitter Enable */
#define SDR0_PE0LANEABCD	0x0000031F	/* PE0 Lanes ABCD affectation */
#define SDR0_PE0LANEEFGH	0x00000320	/* PE0 Lanes EFGH affectation */

#define SDR0_PE1UTLSET1		0x00000340	/* PE1 Upper transaction layer conf setting */
#define SDR0_PE1UTLSET2		0x00000341	/* PE1 Upper transaction layer conf setting 2 */
#define SDR0_PE1DLPSET		0x00000342	/* PE1 Data link & logical physical configuration */
#define SDR0_PE1LOOP		0x00000343	/* PE1 Loopback interface status */
#define SDR0_PE1RCSSET		0x00000344	/* PE1 Reset, clock & shutdown setting */
#define SDR0_PE1RCSSTS		0x00000345	/* PE1 Reset, clock & shutdown status */
#define SDR0_PE1HSSSET1L0	0x00000346	/* PE1 HSS Control Setting 1: Lane 0 */
#define SDR0_PE1HSSSET2L0	0x00000347	/* PE1 HSS Control Setting 2: Lane 0 */
#define SDR0_PE1HSSSTSL0	0x00000348	/* PE1 HSS Control Status : Lane 0 */
#define SDR0_PE1HSSSET1L1	0x00000349	/* PE1 HSS Control Setting 1: Lane 1 */
#define SDR0_PE1HSSSET2L1	0x0000034A	/* PE1 HSS Control Setting 2: Lane 1 */
#define SDR0_PE1HSSSTSL1	0x0000034B	/* PE1 HSS Control Status : Lane 1 */
#define SDR0_PE1HSSSET1L2	0x0000034C	/* PE1 HSS Control Setting 1: Lane 2 */
#define SDR0_PE1HSSSET2L2	0x0000034D	/* PE1 HSS Control Setting 2: Lane 2 */
#define SDR0_PE1HSSSTSL2	0x0000034E	/* PE1 HSS Control Status : Lane 2 */
#define SDR0_PE1HSSSET1L3	0x0000034F	/* PE1 HSS Control Setting 1: Lane 3 */
#define SDR0_PE1HSSSET2L3	0x00000350	/* PE1 HSS Control Setting 2: Lane 3 */
#define SDR0_PE1HSSSTSL3	0x00000351	/* PE1 HSS Control Status : Lane 3 */
#define SDR0_PE1HSSSEREN	0x00000352	/* PE1 Serdes Transmitter Enable */
#define SDR0_PE1LANEABCD	0x00000353	/* PE1 Lanes ABCD affectation */
#define SDR0_PE2UTLSET1		0x00000370	/* PE2 Upper transaction layer conf setting */
#define SDR0_PE2UTLSET2		0x00000371	/* PE2 Upper transaction layer conf setting 2 */
#define SDR0_PE2DLPSET		0x00000372	/* PE2 Data link & logical physical configuration */
#define SDR0_PE2LOOP		0x00000373	/* PE2 Loopback interface status */
#define SDR0_PE2RCSSET		0x00000374	/* PE2 Reset, clock & shutdown setting */
#define SDR0_PE2RCSSTS		0x00000375	/* PE2 Reset, clock & shutdown status */
#define SDR0_PE2HSSSET1L0	0x00000376	/* PE2 HSS Control Setting 1: Lane 0 */
#define SDR0_PE2HSSSET2L0	0x00000377	/* PE2 HSS Control Setting 2: Lane 0 */
#define SDR0_PE2HSSSTSL0	0x00000378	/* PE2 HSS Control Status : Lane 0 */
#define SDR0_PE2HSSSET1L1	0x00000379	/* PE2 HSS Control Setting 1: Lane 1 */
#define SDR0_PE2HSSSET2L1	0x0000037A	/* PE2 HSS Control Setting 2: Lane 1 */
#define SDR0_PE2HSSSTSL1	0x0000037B	/* PE2 HSS Control Status : Lane 1 */
#define SDR0_PE2HSSSET1L2	0x0000037C	/* PE2 HSS Control Setting 1: Lane 2 */
#define SDR0_PE2HSSSET2L2	0x0000037D	/* PE2 HSS Control Setting 2: Lane 2 */
#define SDR0_PE2HSSSTSL2	0x0000037E	/* PE2 HSS Control Status : Lane 2 */
#define SDR0_PE2HSSSET1L3	0x0000037F	/* PE2 HSS Control Setting 1: Lane 3 */
#define SDR0_PE2HSSSET2L3	0x00000380	/* PE2 HSS Control Setting 2: Lane 3 */
#define SDR0_PE2HSSSTSL3	0x00000381	/* PE2 HSS Control Status : Lane 3 */
#define SDR0_PE2HSSSEREN	0x00000382	/* PE2 Serdes Transmitter Enable */
#define SDR0_PE2LANEABCD	0x00000383	/* PE2 Lanes ABCD affectation */
#define SDR0_PEGPLLSET1		0x000003A0	/* PE Pll LC Tank Setting1 */
#define SDR0_PEGPLLSET2		0x000003A1	/* PE Pll LC Tank Setting2 */
#define SDR0_PEGPLLSTS		0x000003A2	/* PE Pll LC Tank Status */
#endif /* CONFIG_440SPE */

#if defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
/*----------------------------------------------------------------------------+
| SDRAM Controller
+----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------+
| SDRAM DLYCAL Options
+-----------------------------------------------------------------------------*/
#define SDRAM_DLYCAL_DLCV_MASK		0x000003FC
#define SDRAM_DLYCAL_DLCV_ENCODE(x)	(((x)<<2) & SDRAM_DLYCAL_DLCV_MASK)
#define SDRAM_DLYCAL_DLCV_DECODE(x)	(((x) & SDRAM_DLYCAL_DLCV_MASK)>>2)

/*----------------------------------------------------------------------------+
| Memory queue defines
+----------------------------------------------------------------------------*/
/* A REVOIR versus RWC  - SG*/
#define SDRAMQ_DCR_BASE	0x040

#define SDRAM_R0BAS	(SDRAMQ_DCR_BASE+0x0)	/* rank 0 base address & size  */
#define SDRAM_R1BAS	(SDRAMQ_DCR_BASE+0x1)	/* rank 1 base address & size  */
#define SDRAM_R2BAS	(SDRAMQ_DCR_BASE+0x2)	/* rank 2 base address & size  */
#define SDRAM_R3BAS	(SDRAMQ_DCR_BASE+0x3)	/* rank 3 base address & size  */
#define SDRAM_CONF1HB	(SDRAMQ_DCR_BASE+0x5)	/* configuration 1 HB          */
#define SDRAM_ERRSTATHB	(SDRAMQ_DCR_BASE+0x7)	/* error status HB             */
#define SDRAM_ERRADDUHB	(SDRAMQ_DCR_BASE+0x8)	/* error address upper 32 HB   */
#define SDRAM_ERRADDLHB	(SDRAMQ_DCR_BASE+0x9)	/* error address lower 32 HB   */
#define SDRAM_PLBADDULL	(SDRAMQ_DCR_BASE+0xA)	/* PLB base address upper 32 LL */
#define SDRAM_CONF1LL	(SDRAMQ_DCR_BASE+0xB)	/* configuration 1 LL          */
#define SDRAM_ERRSTATLL	(SDRAMQ_DCR_BASE+0xC)	/* error status LL             */
#define SDRAM_ERRADDULL	(SDRAMQ_DCR_BASE+0xD)	/* error address upper 32 LL   */
#define SDRAM_ERRADDLLL	(SDRAMQ_DCR_BASE+0xE)	/* error address lower 32 LL   */
#define SDRAM_CONFPATHB	(SDRAMQ_DCR_BASE+0xF)	/* configuration between paths */
#define SDRAM_PLBADDUHB	(SDRAMQ_DCR_BASE+0x10)	/* PLB base address upper 32 LL */

/*-----------------------------------------------------------------------------+
|  Memory Bank 0-7 configuration
+-----------------------------------------------------------------------------*/
#if defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define SDRAM_RXBAS_SDBA_MASK		0xFFE00000	/* Base address	*/
#define SDRAM_RXBAS_SDBA_ENCODE(n)	((((unsigned long)(n))&0xFFE00000)>>2)
#define SDRAM_RXBAS_SDBA_DECODE(n)	((((unsigned long)(n))&0xFFE00000)<<2)
#endif /* CONFIG_440SPE */
#if defined(CONFIG_440SP)
#define SDRAM_RXBAS_SDBA_MASK		0xFF800000	/* Base address	*/
#define SDRAM_RXBAS_SDBA_ENCODE(n)	((((unsigned long)(n))&0xFF800000))
#define SDRAM_RXBAS_SDBA_DECODE(n)	((((unsigned long)(n))&0xFF800000))
#endif /* CONFIG_440SP */
#define SDRAM_RXBAS_SDSZ_MASK		0x0000FFC0	/* Size		*/
#define SDRAM_RXBAS_SDSZ_ENCODE(n)	((((unsigned long)(n))&0x3FF)<<6)
#define SDRAM_RXBAS_SDSZ_DECODE(n)	((((unsigned long)(n))>>6)&0x3FF)
#define SDRAM_RXBAS_SDSZ_0		0x00000000	/*   0M		*/
#define SDRAM_RXBAS_SDSZ_8		0x0000FFC0	/*   8M		*/
#define SDRAM_RXBAS_SDSZ_16		0x0000FF80	/*  16M		*/
#define SDRAM_RXBAS_SDSZ_32		0x0000FF00	/*  32M		*/
#define SDRAM_RXBAS_SDSZ_64		0x0000FE00	/*  64M		*/
#define SDRAM_RXBAS_SDSZ_128		0x0000FC00	/* 128M		*/
#define SDRAM_RXBAS_SDSZ_256		0x0000F800	/* 256M		*/
#define SDRAM_RXBAS_SDSZ_512		0x0000F000	/* 512M		*/
#define SDRAM_RXBAS_SDSZ_1024		0x0000E000	/* 1024M	*/
#define SDRAM_RXBAS_SDSZ_2048		0x0000C000	/* 2048M	*/
#define SDRAM_RXBAS_SDSZ_4096		0x00008000	/* 4096M	*/

/*----------------------------------------------------------------------------+
| Memory controller defines
+----------------------------------------------------------------------------*/
/* A REVOIR versus specs 4 bank  - SG*/
#define SDRAM_MCSTAT	0x14	/* memory controller status                  */
#define SDRAM_MCOPT1	0x20	/* memory controller options 1               */
#define SDRAM_MCOPT2	0x21	/* memory controller options 2               */
#define SDRAM_MODT0	0x22	/* on die termination for bank 0             */
#define SDRAM_MODT1	0x23	/* on die termination for bank 1             */
#define SDRAM_MODT2	0x24	/* on die termination for bank 2             */
#define SDRAM_MODT3	0x25	/* on die termination for bank 3             */
#define SDRAM_CODT	0x26	/* on die termination for controller         */
#define SDRAM_VVPR	0x27	/* variable VRef programmming                */
#define SDRAM_OPARS	0x28	/* on chip driver control setup              */
#define SDRAM_OPART	0x29	/* on chip driver control trigger            */
#define SDRAM_RTR	0x30	/* refresh timer                             */
#define SDRAM_PMIT	0x34	/* power management idle timer               */
#define SDRAM_MB0CF	0x40	/* memory bank 0 configuration               */
#define SDRAM_MB1CF	0x44	/* memory bank 1 configuration               */
#define SDRAM_MB2CF	0x48
#define SDRAM_MB3CF	0x4C
#define SDRAM_INITPLR0	0x50	/* manual initialization control             */
#define SDRAM_INITPLR1	0x51	/* manual initialization control             */
#define SDRAM_INITPLR2	0x52	/* manual initialization control             */
#define SDRAM_INITPLR3	0x53	/* manual initialization control             */
#define SDRAM_INITPLR4	0x54	/* manual initialization control             */
#define SDRAM_INITPLR5	0x55	/* manual initialization control             */
#define SDRAM_INITPLR6	0x56	/* manual initialization control             */
#define SDRAM_INITPLR7	0x57	/* manual initialization control             */
#define SDRAM_INITPLR8	0x58	/* manual initialization control             */
#define SDRAM_INITPLR9	0x59	/* manual initialization control             */
#define SDRAM_INITPLR10	0x5a	/* manual initialization control             */
#define SDRAM_INITPLR11	0x5b	/* manual initialization control             */
#define SDRAM_INITPLR12	0x5c	/* manual initialization control             */
#define SDRAM_INITPLR13	0x5d	/* manual initialization control             */
#define SDRAM_INITPLR14	0x5e	/* manual initialization control             */
#define SDRAM_INITPLR15	0x5f	/* manual initialization control             */
#define SDRAM_RQDC	0x70	/* read DQS delay control                    */
#define SDRAM_RFDC	0x74	/* read feedback delay control               */
#define SDRAM_RDCC	0x78	/* read data capture control                 */
#define SDRAM_DLCR	0x7A	/* delay line calibration                    */
#define SDRAM_CLKTR	0x80	/* DDR clock timing                          */
#define SDRAM_WRDTR	0x81	/* write data, DQS, DM clock, timing         */
#define SDRAM_SDTR1	0x85	/* DDR SDRAM timing 1                        */
#define SDRAM_SDTR2	0x86	/* DDR SDRAM timing 2                        */
#define SDRAM_SDTR3	0x87	/* DDR SDRAM timing 3                        */
#define SDRAM_MMODE	0x88	/* memory mode                               */
#define SDRAM_MEMODE	0x89	/* memory extended mode                      */
#define SDRAM_ECCCR	0x98	/* ECC error status                          */
#define SDRAM_CID	0xA4	/* core ID                                   */
#define SDRAM_RID	0xA8	/* revision ID                               */
#define SDRAM_RTSR	0xB1	/* run time status tracking                  */

/*-----------------------------------------------------------------------------+
|  Memory Controller Status
+-----------------------------------------------------------------------------*/
#define SDRAM_MCSTAT_MIC_MASK		0x80000000	/* Memory init status mask	*/
#define SDRAM_MCSTAT_MIC_NOTCOMP	0x00000000	/* Mem init not complete	*/
#define SDRAM_MCSTAT_MIC_COMP		0x80000000	/* Mem init complete		*/
#define SDRAM_MCSTAT_SRMS_MASK		0x40000000	/* Mem self refresh stat mask	*/
#define SDRAM_MCSTAT_SRMS_NOT_SF	0x00000000	/* Mem not in self refresh	*/
#define SDRAM_MCSTAT_SRMS_SF		0x40000000	/* Mem in self refresh		*/
#define SDRAM_MCSTAT_IDLE_MASK		0x20000000	/* Mem self refresh stat mask	*/
#define SDRAM_MCSTAT_IDLE_NOT		0x00000000	/* Mem contr not idle		*/
#define SDRAM_MCSTAT_IDLE		0x20000000	/* Mem contr idle		*/

/*-----------------------------------------------------------------------------+
|  Memory Controller Options 1
+-----------------------------------------------------------------------------*/
#define SDRAM_MCOPT1_MCHK_MASK		0x30000000 /* Memory data err check mask*/
#define SDRAM_MCOPT1_MCHK_NON		0x00000000 /* No ECC generation		*/
#define SDRAM_MCOPT1_MCHK_GEN		0x20000000 /* ECC generation		*/
#define SDRAM_MCOPT1_MCHK_CHK		0x10000000 /* ECC generation and check	*/
#define SDRAM_MCOPT1_MCHK_CHK_REP	0x30000000 /* ECC generation, chk, report*/
#define SDRAM_MCOPT1_MCHK_CHK_DECODE(n)	((((unsigned long)(n))>>28)&0x3)
#define SDRAM_MCOPT1_RDEN_MASK		0x08000000 /* Registered DIMM mask	*/
#define SDRAM_MCOPT1_RDEN		0x08000000 /* Registered DIMM enable	*/
#define SDRAM_MCOPT1_PMU_MASK		0x06000000 /* Page management unit mask	*/
#define SDRAM_MCOPT1_PMU_CLOSE		0x00000000 /* PMU Close			*/
#define SDRAM_MCOPT1_PMU_OPEN		0x04000000 /* PMU Open			*/
#define SDRAM_MCOPT1_PMU_AUTOCLOSE	0x02000000 /* PMU AutoClose		*/
#define SDRAM_MCOPT1_DMWD_MASK		0x01000000 /* DRAM width mask		*/
#define SDRAM_MCOPT1_DMWD_32		0x00000000 /* 32 bits			*/
#define SDRAM_MCOPT1_DMWD_64		0x01000000 /* 64 bits			*/
#define SDRAM_MCOPT1_UIOS_MASK		0x00C00000 /* Unused IO State		*/
#define SDRAM_MCOPT1_BCNT_MASK		0x00200000 /* Bank count		*/
#define SDRAM_MCOPT1_4_BANKS		0x00000000 /* 4 Banks			*/
#define SDRAM_MCOPT1_8_BANKS		0x00200000 /* 8 Banks			*/
#define SDRAM_MCOPT1_DDR_TYPE_MASK	0x00100000 /* DDR Memory Type mask	*/
#define SDRAM_MCOPT1_DDR1_TYPE		0x00000000 /* DDR1 Memory Type		*/
#define SDRAM_MCOPT1_DDR2_TYPE		0x00100000 /* DDR2 Memory Type		*/
#define SDRAM_MCOPT1_QDEP		0x00020000 /* 4 commands deep		*/
#define SDRAM_MCOPT1_RWOO_MASK		0x00008000 /* Out of Order Read mask	*/
#define SDRAM_MCOPT1_RWOO_DISABLED	0x00000000 /* disabled			*/
#define SDRAM_MCOPT1_RWOO_ENABLED	0x00008000 /* enabled			*/
#define SDRAM_MCOPT1_WOOO_MASK		0x00004000 /* Out of Order Write mask	*/
#define SDRAM_MCOPT1_WOOO_DISABLED	0x00000000 /* disabled			*/
#define SDRAM_MCOPT1_WOOO_ENABLED	0x00004000 /* enabled			*/
#define SDRAM_MCOPT1_DCOO_MASK		0x00002000 /* All Out of Order mask	*/
#define SDRAM_MCOPT1_DCOO_DISABLED	0x00002000 /* disabled			*/
#define SDRAM_MCOPT1_DCOO_ENABLED	0x00000000 /* enabled			*/
#define SDRAM_MCOPT1_DREF_MASK		0x00001000 /* Deferred refresh mask	*/
#define SDRAM_MCOPT1_DREF_NORMAL	0x00000000 /* normal refresh		*/
#define SDRAM_MCOPT1_DREF_DEFER_4	0x00001000 /* defer up to 4 refresh cmd	*/

/*-----------------------------------------------------------------------------+
|  Memory Controller Options 2
+-----------------------------------------------------------------------------*/
#define SDRAM_MCOPT2_SREN_MASK		0x80000000 /* Self Test mask		*/
#define SDRAM_MCOPT2_SREN_EXIT		0x00000000 /* Self Test exit		*/
#define SDRAM_MCOPT2_SREN_ENTER		0x80000000 /* Self Test enter		*/
#define SDRAM_MCOPT2_PMEN_MASK		0x40000000 /* Power Management mask	*/
#define SDRAM_MCOPT2_PMEN_DISABLE	0x00000000 /* disable			*/
#define SDRAM_MCOPT2_PMEN_ENABLE	0x40000000 /* enable			*/
#define SDRAM_MCOPT2_IPTR_MASK		0x20000000 /* Init Trigger Reg mask	*/
#define SDRAM_MCOPT2_IPTR_IDLE		0x00000000 /* idle			*/
#define SDRAM_MCOPT2_IPTR_EXECUTE	0x20000000 /* execute preloaded init	*/
#define SDRAM_MCOPT2_XSRP_MASK		0x10000000 /* Exit Self Refresh Prevent	*/
#define SDRAM_MCOPT2_XSRP_ALLOW		0x00000000 /* allow self refresh exit	*/
#define SDRAM_MCOPT2_XSRP_PREVENT	0x10000000 /* prevent self refresh exit	*/
#define SDRAM_MCOPT2_DCEN_MASK		0x08000000 /* SDRAM Controller Enable	*/
#define SDRAM_MCOPT2_DCEN_DISABLE	0x00000000 /* SDRAM Controller Enable	*/
#define SDRAM_MCOPT2_DCEN_ENABLE	0x08000000 /* SDRAM Controller Enable	*/
#define SDRAM_MCOPT2_ISIE_MASK		0x04000000 /* Init Seq Interruptable mas*/
#define SDRAM_MCOPT2_ISIE_DISABLE	0x00000000 /* disable			*/
#define SDRAM_MCOPT2_ISIE_ENABLE	0x04000000 /* enable			*/

/*-----------------------------------------------------------------------------+
|  SDRAM Refresh Timer Register
+-----------------------------------------------------------------------------*/
#define SDRAM_RTR_RINT_MASK		0xFFF80000
#define SDRAM_RTR_RINT_ENCODE(n)	((((unsigned long)(n))&0xFFF8)<<16)
#define SDRAM_RTR_RINT_DECODE(n)	((((unsigned long)(n))>>16)&0xFFF8)

/*-----------------------------------------------------------------------------+
|  SDRAM Read DQS Delay Control Register
+-----------------------------------------------------------------------------*/
#define SDRAM_RQDC_RQDE_MASK		0x80000000
#define SDRAM_RQDC_RQDE_DISABLE		0x00000000
#define SDRAM_RQDC_RQDE_ENABLE		0x80000000
#define SDRAM_RQDC_RQFD_MASK		0x000001FF
#define SDRAM_RQDC_RQFD_ENCODE(n)	((((unsigned long)(n))&0x1FF)<<0)

#define SDRAM_RQDC_RQFD_MAX		0x1FF

/*-----------------------------------------------------------------------------+
|  SDRAM Read Data Capture Control Register
+-----------------------------------------------------------------------------*/
#define SDRAM_RDCC_RDSS_MASK		0xC0000000
#define SDRAM_RDCC_RDSS_T1		0x00000000
#define SDRAM_RDCC_RDSS_T2		0x40000000
#define SDRAM_RDCC_RDSS_T3		0x80000000
#define SDRAM_RDCC_RDSS_T4		0xC0000000
#define SDRAM_RDCC_RSAE_MASK		0x00000001
#define SDRAM_RDCC_RSAE_DISABLE		0x00000001
#define SDRAM_RDCC_RSAE_ENABLE		0x00000000

/*-----------------------------------------------------------------------------+
|  SDRAM Read Feedback Delay Control Register
+-----------------------------------------------------------------------------*/
#define SDRAM_RFDC_ARSE_MASK		0x80000000
#define SDRAM_RFDC_ARSE_DISABLE		0x80000000
#define SDRAM_RFDC_ARSE_ENABLE		0x00000000
#define SDRAM_RFDC_RFOS_MASK		0x007F0000
#define SDRAM_RFDC_RFOS_ENCODE(n)	((((unsigned long)(n))&0x7F)<<16)
#define SDRAM_RFDC_RFFD_MASK		0x000007FF
#define SDRAM_RFDC_RFFD_ENCODE(n)	((((unsigned long)(n))&0x7FF)<<0)

#define SDRAM_RFDC_RFFD_MAX		0x7FF

/*-----------------------------------------------------------------------------+
|  SDRAM Delay Line Calibration Register
+-----------------------------------------------------------------------------*/
#define SDRAM_DLCR_DCLM_MASK		0x80000000
#define SDRAM_DLCR_DCLM_MANUEL		0x80000000
#define SDRAM_DLCR_DCLM_AUTO		0x00000000
#define SDRAM_DLCR_DLCR_MASK		0x08000000
#define SDRAM_DLCR_DLCR_CALIBRATE	0x08000000
#define SDRAM_DLCR_DLCR_IDLE		0x00000000
#define SDRAM_DLCR_DLCS_MASK		0x07000000
#define SDRAM_DLCR_DLCS_NOT_RUN		0x00000000
#define SDRAM_DLCR_DLCS_IN_PROGRESS	0x01000000
#define SDRAM_DLCR_DLCS_COMPLETE	0x02000000
#define SDRAM_DLCR_DLCS_CONT_DONE	0x03000000
#define SDRAM_DLCR_DLCS_ERROR		0x04000000
#define SDRAM_DLCR_DLCV_MASK		0x000001FF
#define SDRAM_DLCR_DLCV_ENCODE(n)	((((unsigned long)(n))&0x1FF)<<0)
#define SDRAM_DLCR_DLCV_DECODE(n)	((((unsigned long)(n))>>0)&0x1FF)

/*-----------------------------------------------------------------------------+
|  SDRAM Controller On Die Termination Register
+-----------------------------------------------------------------------------*/
#define SDRAM_CODT_ODT_ON			0x80000000
#define SDRAM_CODT_ODT_OFF			0x00000000
#define SDRAM_CODT_DQS_VOLTAGE_DDR_MASK		0x00000020
#define SDRAM_CODT_DQS_2_5_V_DDR1		0x00000000
#define SDRAM_CODT_DQS_1_8_V_DDR2		0x00000020
#define SDRAM_CODT_DQS_MASK			0x00000010
#define SDRAM_CODT_DQS_DIFFERENTIAL		0x00000000
#define SDRAM_CODT_DQS_SINGLE_END		0x00000010
#define SDRAM_CODT_CKSE_DIFFERENTIAL		0x00000000
#define SDRAM_CODT_CKSE_SINGLE_END		0x00000008
#define SDRAM_CODT_FEEBBACK_RCV_SINGLE_END	0x00000004
#define SDRAM_CODT_FEEBBACK_DRV_SINGLE_END	0x00000002
#define SDRAM_CODT_IO_HIZ			0x00000000
#define SDRAM_CODT_IO_NMODE			0x00000001

/*-----------------------------------------------------------------------------+
|  SDRAM Mode Register
+-----------------------------------------------------------------------------*/
#define SDRAM_MMODE_WR_MASK		0x00000E00
#define SDRAM_MMODE_WR_DDR1		0x00000000
#define SDRAM_MMODE_WR_DDR2_3_CYC	0x00000400
#define SDRAM_MMODE_WR_DDR2_4_CYC	0x00000600
#define SDRAM_MMODE_WR_DDR2_5_CYC	0x00000800
#define SDRAM_MMODE_WR_DDR2_6_CYC	0x00000A00
#define SDRAM_MMODE_DCL_MASK		0x00000070
#define SDRAM_MMODE_DCL_DDR1_2_0_CLK	0x00000020
#define SDRAM_MMODE_DCL_DDR1_2_5_CLK	0x00000060
#define SDRAM_MMODE_DCL_DDR1_3_0_CLK	0x00000030
#define SDRAM_MMODE_DCL_DDR2_2_0_CLK	0x00000020
#define SDRAM_MMODE_DCL_DDR2_3_0_CLK	0x00000030
#define SDRAM_MMODE_DCL_DDR2_4_0_CLK	0x00000040
#define SDRAM_MMODE_DCL_DDR2_5_0_CLK	0x00000050
#define SDRAM_MMODE_DCL_DDR2_6_0_CLK	0x00000060
#define SDRAM_MMODE_DCL_DDR2_7_0_CLK	0x00000070

/*-----------------------------------------------------------------------------+
|  SDRAM Extended Mode Register
+-----------------------------------------------------------------------------*/
#define SDRAM_MEMODE_DIC_MASK		0x00000002
#define SDRAM_MEMODE_DIC_NORMAL		0x00000000
#define SDRAM_MEMODE_DIC_WEAK		0x00000002
#define SDRAM_MEMODE_DLL_MASK		0x00000001
#define SDRAM_MEMODE_DLL_DISABLE	0x00000001
#define SDRAM_MEMODE_DLL_ENABLE		0x00000000
#define SDRAM_MEMODE_RTT_MASK		0x00000044
#define SDRAM_MEMODE_RTT_DISABLED	0x00000000
#define SDRAM_MEMODE_RTT_75OHM		0x00000004
#define SDRAM_MEMODE_RTT_150OHM		0x00000040
#define SDRAM_MEMODE_DQS_MASK		0x00000400
#define SDRAM_MEMODE_DQS_DISABLE	0x00000400
#define SDRAM_MEMODE_DQS_ENABLE		0x00000000

/*-----------------------------------------------------------------------------+
|  SDRAM Clock Timing Register
+-----------------------------------------------------------------------------*/
#define SDRAM_CLKTR_CLKP_MASK		0xC0000000
#define SDRAM_CLKTR_CLKP_0_DEG		0x00000000
#define SDRAM_CLKTR_CLKP_180_DEG_ADV	0x80000000
#define SDRAM_CLKTR_CLKP_90_DEG_ADV	0x40000000
#define SDRAM_CLKTR_CLKP_270_DEG_ADV	0xC0000000

/*-----------------------------------------------------------------------------+
|  SDRAM Write Timing Register
+-----------------------------------------------------------------------------*/
#define SDRAM_WRDTR_LLWP_MASK		0x10000000
#define SDRAM_WRDTR_LLWP_DIS		0x10000000
#define SDRAM_WRDTR_LLWP_1_CYC		0x00000000
#define SDRAM_WRDTR_WTR_MASK		0x0E000000
#define SDRAM_WRDTR_WTR_0_DEG		0x06000000
#define SDRAM_WRDTR_WTR_90_DEG_ADV	0x04000000
#define SDRAM_WRDTR_WTR_180_DEG_ADV	0x02000000
#define SDRAM_WRDTR_WTR_270_DEG_ADV	0x00000000

/*-----------------------------------------------------------------------------+
|  SDRAM SDTR1 Options
+-----------------------------------------------------------------------------*/
#define SDRAM_SDTR1_LDOF_MASK		0x80000000
#define SDRAM_SDTR1_LDOF_1_CLK		0x00000000
#define SDRAM_SDTR1_LDOF_2_CLK		0x80000000
#define SDRAM_SDTR1_RTW_MASK		0x00F00000
#define SDRAM_SDTR1_RTW_2_CLK		0x00200000
#define SDRAM_SDTR1_RTW_3_CLK		0x00300000
#define SDRAM_SDTR1_WTWO_MASK		0x000F0000
#define SDRAM_SDTR1_WTWO_0_CLK		0x00000000
#define SDRAM_SDTR1_WTWO_1_CLK		0x00010000
#define SDRAM_SDTR1_RTRO_MASK		0x0000F000
#define SDRAM_SDTR1_RTRO_1_CLK		0x00001000
#define SDRAM_SDTR1_RTRO_2_CLK		0x00002000

/*-----------------------------------------------------------------------------+
|  SDRAM SDTR2 Options
+-----------------------------------------------------------------------------*/
#define SDRAM_SDTR2_RCD_MASK		0xF0000000
#define SDRAM_SDTR2_RCD_1_CLK		0x10000000
#define SDRAM_SDTR2_RCD_2_CLK		0x20000000
#define SDRAM_SDTR2_RCD_3_CLK		0x30000000
#define SDRAM_SDTR2_RCD_4_CLK		0x40000000
#define SDRAM_SDTR2_RCD_5_CLK		0x50000000
#define SDRAM_SDTR2_WTR_MASK		0x0F000000
#define SDRAM_SDTR2_WTR_1_CLK		0x01000000
#define SDRAM_SDTR2_WTR_2_CLK		0x02000000
#define SDRAM_SDTR2_WTR_3_CLK		0x03000000
#define SDRAM_SDTR2_WTR_4_CLK		0x04000000
#define SDRAM_SDTR3_WTR_ENCODE(n)	((((unsigned long)(n))&0xF)<<24)
#define SDRAM_SDTR2_XSNR_MASK		0x00FF0000
#define SDRAM_SDTR2_XSNR_8_CLK		0x00080000
#define SDRAM_SDTR2_XSNR_16_CLK		0x00100000
#define SDRAM_SDTR2_XSNR_32_CLK		0x00200000
#define SDRAM_SDTR2_XSNR_64_CLK		0x00400000
#define SDRAM_SDTR2_WPC_MASK		0x0000F000
#define SDRAM_SDTR2_WPC_2_CLK		0x00002000
#define SDRAM_SDTR2_WPC_3_CLK		0x00003000
#define SDRAM_SDTR2_WPC_4_CLK		0x00004000
#define SDRAM_SDTR2_WPC_5_CLK		0x00005000
#define SDRAM_SDTR2_WPC_6_CLK		0x00006000
#define SDRAM_SDTR3_WPC_ENCODE(n)	((((unsigned long)(n))&0xF)<<12)
#define SDRAM_SDTR2_RPC_MASK		0x00000F00
#define SDRAM_SDTR2_RPC_2_CLK		0x00000200
#define SDRAM_SDTR2_RPC_3_CLK		0x00000300
#define SDRAM_SDTR2_RPC_4_CLK		0x00000400
#define SDRAM_SDTR2_RP_MASK		0x000000F0
#define SDRAM_SDTR2_RP_3_CLK		0x00000030
#define SDRAM_SDTR2_RP_4_CLK		0x00000040
#define SDRAM_SDTR2_RP_5_CLK		0x00000050
#define SDRAM_SDTR2_RP_6_CLK		0x00000060
#define SDRAM_SDTR2_RP_7_CLK		0x00000070
#define SDRAM_SDTR2_RRD_MASK		0x0000000F
#define SDRAM_SDTR2_RRD_2_CLK		0x00000002
#define SDRAM_SDTR2_RRD_3_CLK		0x00000003

/*-----------------------------------------------------------------------------+
|  SDRAM SDTR3 Options
+-----------------------------------------------------------------------------*/
#define SDRAM_SDTR3_RAS_MASK		0x1F000000
#define SDRAM_SDTR3_RAS_ENCODE(n)	((((unsigned long)(n))&0x1F)<<24)
#define SDRAM_SDTR3_RC_MASK		0x001F0000
#define SDRAM_SDTR3_RC_ENCODE(n)	((((unsigned long)(n))&0x1F)<<16)
#define SDRAM_SDTR3_XCS_MASK		0x00001F00
#define SDRAM_SDTR3_XCS			0x00000D00
#define SDRAM_SDTR3_RFC_MASK		0x0000003F
#define SDRAM_SDTR3_RFC_ENCODE(n)	((((unsigned long)(n))&0x3F)<<0)

/*-----------------------------------------------------------------------------+
|  Memory Bank 0-1 configuration
+-----------------------------------------------------------------------------*/
#define SDRAM_BXCF_M_AM_MASK		0x00000F00	/* Addressing mode	*/
#define SDRAM_BXCF_M_AM_0		0x00000000	/*   Mode 0		*/
#define SDRAM_BXCF_M_AM_1		0x00000100	/*   Mode 1		*/
#define SDRAM_BXCF_M_AM_2		0x00000200	/*   Mode 2		*/
#define SDRAM_BXCF_M_AM_3		0x00000300	/*   Mode 3		*/
#define SDRAM_BXCF_M_AM_4		0x00000400	/*   Mode 4		*/
#define SDRAM_BXCF_M_AM_5		0x00000500	/*   Mode 5		*/
#define SDRAM_BXCF_M_AM_6		0x00000600	/*   Mode 6		*/
#define SDRAM_BXCF_M_AM_7		0x00000700	/*   Mode 7		*/
#define SDRAM_BXCF_M_AM_8		0x00000800	/*   Mode 8		*/
#define SDRAM_BXCF_M_AM_9		0x00000900	/*   Mode 9		*/
#define SDRAM_BXCF_M_BE_MASK		0x00000001	/* Memory Bank Enable	*/
#define SDRAM_BXCF_M_BE_DISABLE		0x00000000	/* Memory Bank Enable	*/
#define SDRAM_BXCF_M_BE_ENABLE		0x00000001	/* Memory Bank Enable	*/

#define SDRAM_RTSR_TRK1SM_MASK		0xC0000000	/* Tracking State Mach 1*/
#define SDRAM_RTSR_TRK1SM_ATBASE	0x00000000	/* atbase state		*/
#define SDRAM_RTSR_TRK1SM_MISSED	0x40000000	/* missed state		*/
#define SDRAM_RTSR_TRK1SM_ATPLS1	0x80000000	/* atpls1 state		*/
#define SDRAM_RTSR_TRK1SM_RESET		0xC0000000	/* reset  state		*/

#define SDR0_MFR_FIXD			0x10000000	/* Workaround for PCI/DMA */
#endif /* CONFIG_440SPE */

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
/*-----------------------------------------------------------------------------
 | SDRAM Controller
 +----------------------------------------------------------------------------*/
#define DDR0_00				0x00
#define DDR0_00_INT_ACK_MASK              0x7F000000	/* Write only */
#define DDR0_00_INT_ACK_ALL               0x7F000000
#define DDR0_00_INT_ACK_ENCODE(n)           ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_00_INT_ACK_DECODE(n)           ((((unsigned long)(n))>>24)&0x7F)
/* Status */
#define DDR0_00_INT_STATUS_MASK           0x00FF0000	/* Read only */
/* Bit0. A single access outside the defined PHYSICAL memory space detected. */
#define DDR0_00_INT_STATUS_BIT0           0x00010000
/* Bit1. Multiple accesses outside the defined PHYSICAL memory space detected. */
#define DDR0_00_INT_STATUS_BIT1           0x00020000
/* Bit2. Single correctable ECC event detected */
#define DDR0_00_INT_STATUS_BIT2           0x00040000
/* Bit3. Multiple correctable ECC events detected. */
#define DDR0_00_INT_STATUS_BIT3           0x00080000
/* Bit4. Single uncorrectable ECC event detected. */
#define DDR0_00_INT_STATUS_BIT4           0x00100000
/* Bit5. Multiple uncorrectable ECC events detected. */
#define DDR0_00_INT_STATUS_BIT5           0x00200000
/* Bit6. DRAM initialization complete. */
#define DDR0_00_INT_STATUS_BIT6           0x00400000
/* Bit7. Logical OR of all lower bits. */
#define DDR0_00_INT_STATUS_BIT7           0x00800000

#define DDR0_00_INT_STATUS_ENCODE(n)        ((((unsigned long)(n))&0xFF)<<16)
#define DDR0_00_INT_STATUS_DECODE(n)        ((((unsigned long)(n))>>16)&0xFF)
#define DDR0_00_DLL_INCREMENT_MASK        0x00007F00
#define DDR0_00_DLL_INCREMENT_ENCODE(n)     ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_00_DLL_INCREMENT_DECODE(n)     ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_00_DLL_START_POINT_MASK      0x0000007F
#define DDR0_00_DLL_START_POINT_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_00_DLL_START_POINT_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_01				0x01
#define DDR0_01_PLB0_DB_CS_LOWER_MASK     0x1F000000
#define DDR0_01_PLB0_DB_CS_LOWER_ENCODE(n)  ((((unsigned long)(n))&0x1F)<<24)
#define DDR0_01_PLB0_DB_CS_LOWER_DECODE(n)  ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_01_PLB0_DB_CS_UPPER_MASK     0x001F0000
#define DDR0_01_PLB0_DB_CS_UPPER_ENCODE(n)  ((((unsigned long)(n))&0x1F)<<16)
#define DDR0_01_PLB0_DB_CS_UPPER_DECODE(n)  ((((unsigned long)(n))>>16)&0x1F)
#define DDR0_01_OUT_OF_RANGE_TYPE_MASK    0x00000700	/* Read only */
#define DDR0_01_OUT_OF_RANGE_TYPE_ENCODE(n)               ((((unsigned long)(n))&0x7)<<8)
#define DDR0_01_OUT_OF_RANGE_TYPE_DECODE(n)               ((((unsigned long)(n))>>8)&0x7)
#define DDR0_01_INT_MASK_MASK             0x000000FF
#define DDR0_01_INT_MASK_ENCODE(n)          ((((unsigned long)(n))&0xFF)<<0)
#define DDR0_01_INT_MASK_DECODE(n)          ((((unsigned long)(n))>>0)&0xFF)
#define DDR0_01_INT_MASK_ALL_ON           0x000000FF
#define DDR0_01_INT_MASK_ALL_OFF          0x00000000

#define DDR0_02				0x02
#define DDR0_02_MAX_CS_REG_MASK           0x02000000	/* Read only */
#define DDR0_02_MAX_CS_REG_ENCODE(n)        ((((unsigned long)(n))&0x2)<<24)
#define DDR0_02_MAX_CS_REG_DECODE(n)        ((((unsigned long)(n))>>24)&0x2)
#define DDR0_02_MAX_COL_REG_MASK          0x000F0000	/* Read only */
#define DDR0_02_MAX_COL_REG_ENCODE(n)       ((((unsigned long)(n))&0xF)<<16)
#define DDR0_02_MAX_COL_REG_DECODE(n)       ((((unsigned long)(n))>>16)&0xF)
#define DDR0_02_MAX_ROW_REG_MASK          0x00000F00	/* Read only */
#define DDR0_02_MAX_ROW_REG_ENCODE(n)       ((((unsigned long)(n))&0xF)<<8)
#define DDR0_02_MAX_ROW_REG_DECODE(n)       ((((unsigned long)(n))>>8)&0xF)
#define DDR0_02_START_MASK                0x00000001
#define DDR0_02_START_ENCODE(n)             ((((unsigned long)(n))&0x1)<<0)
#define DDR0_02_START_DECODE(n)             ((((unsigned long)(n))>>0)&0x1)
#define DDR0_02_START_OFF                 0x00000000
#define DDR0_02_START_ON                  0x00000001

#define DDR0_03				0x03
#define DDR0_03_BSTLEN_MASK               0x07000000
#define DDR0_03_BSTLEN_ENCODE(n)            ((((unsigned long)(n))&0x7)<<24)
#define DDR0_03_BSTLEN_DECODE(n)            ((((unsigned long)(n))>>24)&0x7)
#define DDR0_03_CASLAT_MASK               0x00070000
#define DDR0_03_CASLAT_ENCODE(n)            ((((unsigned long)(n))&0x7)<<16)
#define DDR0_03_CASLAT_DECODE(n)            ((((unsigned long)(n))>>16)&0x7)
#define DDR0_03_CASLAT_LIN_MASK           0x00000F00
#define DDR0_03_CASLAT_LIN_ENCODE(n)        ((((unsigned long)(n))&0xF)<<8)
#define DDR0_03_CASLAT_LIN_DECODE(n)        ((((unsigned long)(n))>>8)&0xF)
#define DDR0_03_INITAREF_MASK             0x0000000F
#define DDR0_03_INITAREF_ENCODE(n)          ((((unsigned long)(n))&0xF)<<0)
#define DDR0_03_INITAREF_DECODE(n)          ((((unsigned long)(n))>>0)&0xF)

#define DDR0_04				0x04
#define DDR0_04_TRC_MASK                  0x1F000000
#define DDR0_04_TRC_ENCODE(n)               ((((unsigned long)(n))&0x1F)<<24)
#define DDR0_04_TRC_DECODE(n)               ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_04_TRRD_MASK                 0x00070000
#define DDR0_04_TRRD_ENCODE(n)              ((((unsigned long)(n))&0x7)<<16)
#define DDR0_04_TRRD_DECODE(n)              ((((unsigned long)(n))>>16)&0x7)
#define DDR0_04_TRTP_MASK                 0x00000700
#define DDR0_04_TRTP_ENCODE(n)              ((((unsigned long)(n))&0x7)<<8)
#define DDR0_04_TRTP_DECODE(n)              ((((unsigned long)(n))>>8)&0x7)

#define DDR0_05				0x05
#define DDR0_05_TMRD_MASK                 0x1F000000
#define DDR0_05_TMRD_ENCODE(n)              ((((unsigned long)(n))&0x1F)<<24)
#define DDR0_05_TMRD_DECODE(n)              ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_05_TEMRS_MASK                0x00070000
#define DDR0_05_TEMRS_ENCODE(n)             ((((unsigned long)(n))&0x7)<<16)
#define DDR0_05_TEMRS_DECODE(n)             ((((unsigned long)(n))>>16)&0x7)
#define DDR0_05_TRP_MASK                  0x00000F00
#define DDR0_05_TRP_ENCODE(n)               ((((unsigned long)(n))&0xF)<<8)
#define DDR0_05_TRP_DECODE(n)               ((((unsigned long)(n))>>8)&0xF)
#define DDR0_05_TRAS_MIN_MASK             0x000000FF
#define DDR0_05_TRAS_MIN_ENCODE(n)          ((((unsigned long)(n))&0xFF)<<0)
#define DDR0_05_TRAS_MIN_DECODE(n)          ((((unsigned long)(n))>>0)&0xFF)

#define DDR0_06				0x06
#define DDR0_06_WRITEINTERP_MASK          0x01000000
#define DDR0_06_WRITEINTERP_ENCODE(n)       ((((unsigned long)(n))&0x1)<<24)
#define DDR0_06_WRITEINTERP_DECODE(n)       ((((unsigned long)(n))>>24)&0x1)
#define DDR0_06_TWTR_MASK                 0x00070000
#define DDR0_06_TWTR_ENCODE(n)              ((((unsigned long)(n))&0x7)<<16)
#define DDR0_06_TWTR_DECODE(n)              ((((unsigned long)(n))>>16)&0x7)
#define DDR0_06_TDLL_MASK                 0x0000FF00
#define DDR0_06_TDLL_ENCODE(n)              ((((unsigned long)(n))&0xFF)<<8)
#define DDR0_06_TDLL_DECODE(n)              ((((unsigned long)(n))>>8)&0xFF)
#define DDR0_06_TRFC_MASK                 0x0000007F
#define DDR0_06_TRFC_ENCODE(n)              ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_06_TRFC_DECODE(n)              ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_07				0x07
#define DDR0_07_NO_CMD_INIT_MASK          0x01000000
#define DDR0_07_NO_CMD_INIT_ENCODE(n)       ((((unsigned long)(n))&0x1)<<24)
#define DDR0_07_NO_CMD_INIT_DECODE(n)       ((((unsigned long)(n))>>24)&0x1)
#define DDR0_07_TFAW_MASK                 0x001F0000
#define DDR0_07_TFAW_ENCODE(n)              ((((unsigned long)(n))&0x1F)<<16)
#define DDR0_07_TFAW_DECODE(n)              ((((unsigned long)(n))>>16)&0x1F)
#define DDR0_07_AUTO_REFRESH_MODE_MASK    0x00000100
#define DDR0_07_AUTO_REFRESH_MODE_ENCODE(n) ((((unsigned long)(n))&0x1)<<8)
#define DDR0_07_AUTO_REFRESH_MODE_DECODE(n) ((((unsigned long)(n))>>8)&0x1)
#define DDR0_07_AREFRESH_MASK             0x00000001
#define DDR0_07_AREFRESH_ENCODE(n)          ((((unsigned long)(n))&0x1)<<0)
#define DDR0_07_AREFRESH_DECODE(n)          ((((unsigned long)(n))>>0)&0x1)

#define DDR0_08				0x08
#define DDR0_08_WRLAT_MASK                0x07000000
#define DDR0_08_WRLAT_ENCODE(n)             ((((unsigned long)(n))&0x7)<<24)
#define DDR0_08_WRLAT_DECODE(n)             ((((unsigned long)(n))>>24)&0x7)
#define DDR0_08_TCPD_MASK                 0x00FF0000
#define DDR0_08_TCPD_ENCODE(n)              ((((unsigned long)(n))&0xFF)<<16)
#define DDR0_08_TCPD_DECODE(n)              ((((unsigned long)(n))>>16)&0xFF)
#define DDR0_08_DQS_N_EN_MASK             0x00000100
#define DDR0_08_DQS_N_EN_ENCODE(n)          ((((unsigned long)(n))&0x1)<<8)
#define DDR0_08_DQS_N_EN_DECODE(n)          ((((unsigned long)(n))>>8)&0x1)
#define DDR0_08_DDRII_SDRAM_MODE_MASK     0x00000001
#define DDR0_08_DDRII_ENCODE(n)             ((((unsigned long)(n))&0x1)<<0)
#define DDR0_08_DDRII_DECODE(n)             ((((unsigned long)(n))>>0)&0x1)

#define DDR0_09				0x09
#define DDR0_09_OCD_ADJUST_PDN_CS_0_MASK  0x1F000000
#define DDR0_09_OCD_ADJUST_PDN_CS_0_ENCODE(n) ((((unsigned long)(n))&0x1F)<<24)
#define DDR0_09_OCD_ADJUST_PDN_CS_0_DECODE(n) ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_09_RTT_0_MASK                0x00030000
#define DDR0_09_RTT_0_ENCODE(n)             ((((unsigned long)(n))&0x3)<<16)
#define DDR0_09_RTT_0_DECODE(n)             ((((unsigned long)(n))>>16)&0x3)
#define DDR0_09_WR_DQS_SHIFT_BYPASS_MASK  0x00007F00
#define DDR0_09_WR_DQS_SHIFT_BYPASS_ENCODE(n) ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_09_WR_DQS_SHIFT_BYPASS_DECODE(n) ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_09_WR_DQS_SHIFT_MASK         0x0000007F
#define DDR0_09_WR_DQS_SHIFT_ENCODE(n)      ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_09_WR_DQS_SHIFT_DECODE(n)      ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_10				0x0A
#define DDR0_10_WRITE_MODEREG_MASK        0x00010000	/* Write only */
#define DDR0_10_WRITE_MODEREG_ENCODE(n)     ((((unsigned long)(n))&0x1)<<16)
#define DDR0_10_WRITE_MODEREG_DECODE(n)     ((((unsigned long)(n))>>16)&0x1)
#define DDR0_10_CS_MAP_MASK               0x00000300
#define DDR0_10_CS_MAP_NO_MEM             0x00000000
#define DDR0_10_CS_MAP_RANK0_INSTALLED    0x00000100
#define DDR0_10_CS_MAP_RANK1_INSTALLED    0x00000200
#define DDR0_10_CS_MAP_ENCODE(n)            ((((unsigned long)(n))&0x3)<<8)
#define DDR0_10_CS_MAP_DECODE(n)            ((((unsigned long)(n))>>8)&0x3)
#define DDR0_10_OCD_ADJUST_PUP_CS_0_MASK  0x0000001F
#define DDR0_10_OCD_ADJUST_PUP_CS_0_ENCODE(n) ((((unsigned long)(n))&0x1F)<<0)
#define DDR0_10_OCD_ADJUST_PUP_CS_0_DECODE(n) ((((unsigned long)(n))>>0)&0x1F)

#define DDR0_11				0x0B
#define DDR0_11_SREFRESH_MASK             0x01000000
#define DDR0_11_SREFRESH_ENCODE(n)          ((((unsigned long)(n))&0x1)<<24)
#define DDR0_11_SREFRESH_DECODE(n)          ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_11_TXSNR_MASK                0x00FF0000
#define DDR0_11_TXSNR_ENCODE(n)             ((((unsigned long)(n))&0xFF)<<16)
#define DDR0_11_TXSNR_DECODE(n)             ((((unsigned long)(n))>>16)&0xFF)
#define DDR0_11_TXSR_MASK                 0x0000FF00
#define DDR0_11_TXSR_ENCODE(n)              ((((unsigned long)(n))&0xFF)<<8)
#define DDR0_11_TXSR_DECODE(n)              ((((unsigned long)(n))>>8)&0xFF)

#define DDR0_12				0x0C
#define DDR0_12_TCKE_MASK                 0x0000007
#define DDR0_12_TCKE_ENCODE(n)              ((((unsigned long)(n))&0x7)<<0)
#define DDR0_12_TCKE_DECODE(n)              ((((unsigned long)(n))>>0)&0x7)

#define DDR0_14				0x0E
#define DDR0_14_DLL_BYPASS_MODE_MASK      0x01000000
#define DDR0_14_DLL_BYPASS_MODE_ENCODE(n)   ((((unsigned long)(n))&0x1)<<24)
#define DDR0_14_DLL_BYPASS_MODE_DECODE(n)   ((((unsigned long)(n))>>24)&0x1)
#define DDR0_14_REDUC_MASK                0x00010000
#define DDR0_14_REDUC_64BITS              0x00000000
#define DDR0_14_REDUC_32BITS              0x00010000
#define DDR0_14_REDUC_ENCODE(n)             ((((unsigned long)(n))&0x1)<<16)
#define DDR0_14_REDUC_DECODE(n)             ((((unsigned long)(n))>>16)&0x1)
#define DDR0_14_REG_DIMM_ENABLE_MASK      0x00000100
#define DDR0_14_REG_DIMM_ENABLE_ENCODE(n)   ((((unsigned long)(n))&0x1)<<8)
#define DDR0_14_REG_DIMM_ENABLE_DECODE(n)   ((((unsigned long)(n))>>8)&0x1)

#define DDR0_17				0x11
#define DDR0_17_DLL_DQS_DELAY_0_MASK      0x7F000000
#define DDR0_17_DLL_DQS_DELAY_0_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_17_DLL_DQS_DELAY_0_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_17_DLLLOCKREG_MASK           0x00010000	/* Read only */
#define DDR0_17_DLLLOCKREG_LOCKED         0x00010000
#define DDR0_17_DLLLOCKREG_UNLOCKED       0x00000000
#define DDR0_17_DLLLOCKREG_ENCODE(n)        ((((unsigned long)(n))&0x1)<<16)
#define DDR0_17_DLLLOCKREG_DECODE(n)        ((((unsigned long)(n))>>16)&0x1)
#define DDR0_17_DLL_LOCK_MASK             0x00007F00	/* Read only */
#define DDR0_17_DLL_LOCK_ENCODE(n)          ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_17_DLL_LOCK_DECODE(n)          ((((unsigned long)(n))>>8)&0x7F)

#define DDR0_18				0x12
#define DDR0_18_DLL_DQS_DELAY_X_MASK      0x7F7F7F7F
#define DDR0_18_DLL_DQS_DELAY_4_MASK      0x7F000000
#define DDR0_18_DLL_DQS_DELAY_4_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_18_DLL_DQS_DELAY_4_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_3_MASK      0x007F0000
#define DDR0_18_DLL_DQS_DELAY_3_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_18_DLL_DQS_DELAY_3_DECODE(n)   ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_2_MASK      0x00007F00
#define DDR0_18_DLL_DQS_DELAY_2_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_18_DLL_DQS_DELAY_2_DECODE(n)   ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_1_MASK      0x0000007F
#define DDR0_18_DLL_DQS_DELAY_1_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_18_DLL_DQS_DELAY_1_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_19				0x13
#define DDR0_19_DLL_DQS_DELAY_X_MASK      0x7F7F7F7F
#define DDR0_19_DLL_DQS_DELAY_8_MASK      0x7F000000
#define DDR0_19_DLL_DQS_DELAY_8_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_19_DLL_DQS_DELAY_8_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_7_MASK      0x007F0000
#define DDR0_19_DLL_DQS_DELAY_7_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_19_DLL_DQS_DELAY_7_DECODE(n)   ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_6_MASK      0x00007F00
#define DDR0_19_DLL_DQS_DELAY_6_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_19_DLL_DQS_DELAY_6_DECODE(n)   ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_5_MASK      0x0000007F
#define DDR0_19_DLL_DQS_DELAY_5_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_19_DLL_DQS_DELAY_5_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_20				0x14
#define DDR0_20_DLL_DQS_BYPASS_3_MASK      0x7F000000
#define DDR0_20_DLL_DQS_BYPASS_3_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_20_DLL_DQS_BYPASS_3_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_2_MASK      0x007F0000
#define DDR0_20_DLL_DQS_BYPASS_2_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_20_DLL_DQS_BYPASS_2_DECODE(n)   ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_1_MASK      0x00007F00
#define DDR0_20_DLL_DQS_BYPASS_1_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_20_DLL_DQS_BYPASS_1_DECODE(n)   ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_0_MASK      0x0000007F
#define DDR0_20_DLL_DQS_BYPASS_0_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_20_DLL_DQS_BYPASS_0_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_21				0x15
#define DDR0_21_DLL_DQS_BYPASS_7_MASK      0x7F000000
#define DDR0_21_DLL_DQS_BYPASS_7_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_21_DLL_DQS_BYPASS_7_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_6_MASK      0x007F0000
#define DDR0_21_DLL_DQS_BYPASS_6_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_21_DLL_DQS_BYPASS_6_DECODE(n)   ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_5_MASK      0x00007F00
#define DDR0_21_DLL_DQS_BYPASS_5_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_21_DLL_DQS_BYPASS_5_DECODE(n)   ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_4_MASK      0x0000007F
#define DDR0_21_DLL_DQS_BYPASS_4_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_21_DLL_DQS_BYPASS_4_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_22				0x16
#define DDR0_22_CTRL_RAW_MASK             0x03000000
#define DDR0_22_CTRL_RAW_ECC_DISABLE      0x00000000	/* ECC not being used */
#define DDR0_22_CTRL_RAW_ECC_CHECK_ONLY   0x01000000	/* ECC checking is on, but no attempts to correct */
#define DDR0_22_CTRL_RAW_NO_ECC_RAM       0x02000000	/* No ECC RAM storage available */
#define DDR0_22_CTRL_RAW_ECC_ENABLE       0x03000000	/* ECC checking and correcting on */
#define DDR0_22_CTRL_RAW_ENCODE(n)          ((((unsigned long)(n))&0x3)<<24)
#define DDR0_22_CTRL_RAW_DECODE(n)          ((((unsigned long)(n))>>24)&0x3)
#define DDR0_22_DQS_OUT_SHIFT_BYPASS_MASK 0x007F0000
#define DDR0_22_DQS_OUT_SHIFT_BYPASS_ENCODE(n) ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_22_DQS_OUT_SHIFT_BYPASS_DECODE(n) ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_22_DQS_OUT_SHIFT_MASK        0x00007F00
#define DDR0_22_DQS_OUT_SHIFT_ENCODE(n)     ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_22_DQS_OUT_SHIFT_DECODE(n)     ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_22_DLL_DQS_BYPASS_8_MASK     0x0000007F
#define DDR0_22_DLL_DQS_BYPASS_8_ENCODE(n)  ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_22_DLL_DQS_BYPASS_8_DECODE(n)  ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_23				0x17
#define DDR0_23_ODT_RD_MAP_CS0_MASK       0x03000000
#define DDR0_23_ODT_RD_MAP_CS0_ENCODE(n)   ((((unsigned long)(n))&0x3)<<24)
#define DDR0_23_ODT_RD_MAP_CS0_DECODE(n)   ((((unsigned long)(n))>>24)&0x3)
#define DDR0_23_ECC_C_SYND_MASK           0x00FF0000	/* Read only */
#define DDR0_23_ECC_C_SYND_ENCODE(n)        ((((unsigned long)(n))&0xFF)<<16)
#define DDR0_23_ECC_C_SYND_DECODE(n)        ((((unsigned long)(n))>>16)&0xFF)
#define DDR0_23_ECC_U_SYND_MASK           0x0000FF00	/* Read only */
#define DDR0_23_ECC_U_SYND_ENCODE(n)        ((((unsigned long)(n))&0xFF)<<8)
#define DDR0_23_ECC_U_SYND_DECODE(n)        ((((unsigned long)(n))>>8)&0xFF)
#define DDR0_23_FWC_MASK                  0x00000001	/* Write only */
#define DDR0_23_FWC_ENCODE(n)               ((((unsigned long)(n))&0x1)<<0)
#define DDR0_23_FWC_DECODE(n)               ((((unsigned long)(n))>>0)&0x1)

#define DDR0_24				0x18
#define DDR0_24_RTT_PAD_TERMINATION_MASK  0x03000000
#define DDR0_24_RTT_PAD_TERMINATION_ENCODE(n) ((((unsigned long)(n))&0x3)<<24)
#define DDR0_24_RTT_PAD_TERMINATION_DECODE(n) ((((unsigned long)(n))>>24)&0x3)
#define DDR0_24_ODT_WR_MAP_CS1_MASK       0x00030000
#define DDR0_24_ODT_WR_MAP_CS1_ENCODE(n)    ((((unsigned long)(n))&0x3)<<16)
#define DDR0_24_ODT_WR_MAP_CS1_DECODE(n)    ((((unsigned long)(n))>>16)&0x3)
#define DDR0_24_ODT_RD_MAP_CS1_MASK       0x00000300
#define DDR0_24_ODT_RD_MAP_CS1_ENCODE(n)    ((((unsigned long)(n))&0x3)<<8)
#define DDR0_24_ODT_RD_MAP_CS1_DECODE(n)    ((((unsigned long)(n))>>8)&0x3)
#define DDR0_24_ODT_WR_MAP_CS0_MASK       0x00000003
#define DDR0_24_ODT_WR_MAP_CS0_ENCODE(n)    ((((unsigned long)(n))&0x3)<<0)
#define DDR0_24_ODT_WR_MAP_CS0_DECODE(n)    ((((unsigned long)(n))>>0)&0x3)

#define DDR0_25				0x19
#define DDR0_25_VERSION_MASK              0xFFFF0000	/* Read only */
#define DDR0_25_VERSION_ENCODE(n)           ((((unsigned long)(n))&0xFFFF)<<16)
#define DDR0_25_VERSION_DECODE(n)           ((((unsigned long)(n))>>16)&0xFFFF)
#define DDR0_25_OUT_OF_RANGE_LENGTH_MASK  0x000003FF	/* Read only */
#define DDR0_25_OUT_OF_RANGE_LENGTH_ENCODE(n) ((((unsigned long)(n))&0x3FF)<<0)
#define DDR0_25_OUT_OF_RANGE_LENGTH_DECODE(n) ((((unsigned long)(n))>>0)&0x3FF)

#define DDR0_26				0x1A
#define DDR0_26_TRAS_MAX_MASK             0xFFFF0000
#define DDR0_26_TRAS_MAX_ENCODE(n)          ((((unsigned long)(n))&0xFFFF)<<16)
#define DDR0_26_TRAS_MAX_DECODE(n)          ((((unsigned long)(n))>>16)&0xFFFF)
#define DDR0_26_TREF_MASK                 0x00003FFF
#define DDR0_26_TREF_ENCODE(n)              ((((unsigned long)(n))&0x3FFF)<<0)
#define DDR0_26_TREF_DECODE(n)              ((((unsigned long)(n))>>0)&0x3FFF)

#define DDR0_27				0x1B
#define DDR0_27_EMRS_DATA_MASK            0x3FFF0000
#define DDR0_27_EMRS_DATA_ENCODE(n)         ((((unsigned long)(n))&0x3FFF)<<16)
#define DDR0_27_EMRS_DATA_DECODE(n)         ((((unsigned long)(n))>>16)&0x3FFF)
#define DDR0_27_TINIT_MASK                0x0000FFFF
#define DDR0_27_TINIT_ENCODE(n)             ((((unsigned long)(n))&0xFFFF)<<0)
#define DDR0_27_TINIT_DECODE(n)             ((((unsigned long)(n))>>0)&0xFFFF)

#define DDR0_28				0x1C
#define DDR0_28_EMRS3_DATA_MASK           0x3FFF0000
#define DDR0_28_EMRS3_DATA_ENCODE(n)        ((((unsigned long)(n))&0x3FFF)<<16)
#define DDR0_28_EMRS3_DATA_DECODE(n)        ((((unsigned long)(n))>>16)&0x3FFF)
#define DDR0_28_EMRS2_DATA_MASK           0x00003FFF
#define DDR0_28_EMRS2_DATA_ENCODE(n)        ((((unsigned long)(n))&0x3FFF)<<0)
#define DDR0_28_EMRS2_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0x3FFF)

#define DDR0_31				0x1F
#define DDR0_31_XOR_CHECK_BITS_MASK       0x0000FFFF
#define DDR0_31_XOR_CHECK_BITS_ENCODE(n)    ((((unsigned long)(n))&0xFFFF)<<0)
#define DDR0_31_XOR_CHECK_BITS_DECODE(n)    ((((unsigned long)(n))>>0)&0xFFFF)

#define DDR0_32				0x20
#define DDR0_32_OUT_OF_RANGE_ADDR_MASK    0xFFFFFFFF	/* Read only */
#define DDR0_32_OUT_OF_RANGE_ADDR_ENCODE(n) ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_32_OUT_OF_RANGE_ADDR_DECODE(n) ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_33				0x21
#define DDR0_33_OUT_OF_RANGE_ADDR_MASK    0x00000001	/* Read only */
#define DDR0_33_OUT_OF_RANGE_ADDR_ENCODE(n) ((((unsigned long)(n))&0x1)<<0)
#define DDR0_33_OUT_OF_RANGE_ADDR_DECODE(n)               ((((unsigned long)(n))>>0)&0x1)

#define DDR0_34				0x22
#define DDR0_34_ECC_U_ADDR_MASK           0xFFFFFFFF	/* Read only */
#define DDR0_34_ECC_U_ADDR_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_34_ECC_U_ADDR_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_35				0x23
#define DDR0_35_ECC_U_ADDR_MASK           0x00000001	/* Read only */
#define DDR0_35_ECC_U_ADDR_ENCODE(n)        ((((unsigned long)(n))&0x1)<<0)
#define DDR0_35_ECC_U_ADDR_DECODE(n)        ((((unsigned long)(n))>>0)&0x1)

#define DDR0_36				0x24
#define DDR0_36_ECC_U_DATA_MASK           0xFFFFFFFF	/* Read only */
#define DDR0_36_ECC_U_DATA_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_36_ECC_U_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_37				0x25
#define DDR0_37_ECC_U_DATA_MASK           0xFFFFFFFF	/* Read only */
#define DDR0_37_ECC_U_DATA_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_37_ECC_U_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_38				0x26
#define DDR0_38_ECC_C_ADDR_MASK           0xFFFFFFFF	/* Read only */
#define DDR0_38_ECC_C_ADDR_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_38_ECC_C_ADDR_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_39				0x27
#define DDR0_39_ECC_C_ADDR_MASK           0x00000001	/* Read only */
#define DDR0_39_ECC_C_ADDR_ENCODE(n)        ((((unsigned long)(n))&0x1)<<0)
#define DDR0_39_ECC_C_ADDR_DECODE(n)        ((((unsigned long)(n))>>0)&0x1)

#define DDR0_40				0x28
#define DDR0_40_ECC_C_DATA_MASK           0xFFFFFFFF	/* Read only */
#define DDR0_40_ECC_C_DATA_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_40_ECC_C_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_41				0x29
#define DDR0_41_ECC_C_DATA_MASK           0xFFFFFFFF	/* Read only */
#define DDR0_41_ECC_C_DATA_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_41_ECC_C_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_42				0x2A
#define DDR0_42_ADDR_PINS_MASK            0x07000000
#define DDR0_42_ADDR_PINS_ENCODE(n)         ((((unsigned long)(n))&0x7)<<24)
#define DDR0_42_ADDR_PINS_DECODE(n)         ((((unsigned long)(n))>>24)&0x7)
#define DDR0_42_CASLAT_LIN_GATE_MASK      0x0000000F
#define DDR0_42_CASLAT_LIN_GATE_ENCODE(n)   ((((unsigned long)(n))&0xF)<<0)
#define DDR0_42_CASLAT_LIN_GATE_DECODE(n)   ((((unsigned long)(n))>>0)&0xF)

#define DDR0_43				0x2B
#define DDR0_43_TWR_MASK                  0x07000000
#define DDR0_43_TWR_ENCODE(n)               ((((unsigned long)(n))&0x7)<<24)
#define DDR0_43_TWR_DECODE(n)               ((((unsigned long)(n))>>24)&0x7)
#define DDR0_43_APREBIT_MASK              0x000F0000
#define DDR0_43_APREBIT_ENCODE(n)           ((((unsigned long)(n))&0xF)<<16)
#define DDR0_43_APREBIT_DECODE(n)           ((((unsigned long)(n))>>16)&0xF)
#define DDR0_43_COLUMN_SIZE_MASK          0x00000700
#define DDR0_43_COLUMN_SIZE_ENCODE(n)       ((((unsigned long)(n))&0x7)<<8)
#define DDR0_43_COLUMN_SIZE_DECODE(n)       ((((unsigned long)(n))>>8)&0x7)
#define DDR0_43_EIGHT_BANK_MODE_MASK      0x00000001
#define DDR0_43_EIGHT_BANK_MODE_8_BANKS     0x00000001
#define DDR0_43_EIGHT_BANK_MODE_4_BANKS     0x00000000
#define DDR0_43_EIGHT_BANK_MODE_ENCODE(n)   ((((unsigned long)(n))&0x1)<<0)
#define DDR0_43_EIGHT_BANK_MODE_DECODE(n)   ((((unsigned long)(n))>>0)&0x1)

#define DDR0_44				0x2C
#define DDR0_44_TRCD_MASK                 0x000000FF
#define DDR0_44_TRCD_ENCODE(n)              ((((unsigned long)(n))&0xFF)<<0)
#define DDR0_44_TRCD_DECODE(n)              ((((unsigned long)(n))>>0)&0xFF)

#endif /* CONFIG_440EPX */

/*-----------------------------------------------------------------------------
 | External Bus Controller
 +----------------------------------------------------------------------------*/
/* values for ebccfga register - indirect addressing of these regs */
#define pb0cr		0x00	/* periph bank 0 config reg		*/
#define pb1cr		0x01	/* periph bank 1 config reg		*/
#define pb2cr		0x02	/* periph bank 2 config reg		*/
#define pb3cr		0x03	/* periph bank 3 config reg		*/
#define pb4cr		0x04	/* periph bank 4 config reg		*/
#define pb5cr		0x05	/* periph bank 5 config reg		*/
#define pb6cr		0x06	/* periph bank 6 config reg		*/
#define pb7cr		0x07	/* periph bank 7 config reg		*/
#define pb0ap		0x10	/* periph bank 0 access parameters	*/
#define pb1ap		0x11	/* periph bank 1 access parameters	*/
#define pb2ap		0x12	/* periph bank 2 access parameters	*/
#define pb3ap		0x13	/* periph bank 3 access parameters	*/
#define pb4ap		0x14	/* periph bank 4 access parameters	*/
#define pb5ap		0x15	/* periph bank 5 access parameters	*/
#define pb6ap		0x16	/* periph bank 6 access parameters	*/
#define pb7ap		0x17	/* periph bank 7 access parameters	*/
#define pbear		0x20	/* periph bus error addr reg		*/
#define pbesr		0x21	/* periph bus error status reg		*/
#define xbcfg		0x23	/* external bus configuration reg	*/
#define EBC0_CFG	0x23	/* external bus configuration reg	*/
#define xbcid		0x24	/* external bus core id reg		*/

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

/* PLB4 to PLB3 Bridge OUT */
#define P4P3_DCR_BASE           0x020
#define p4p3_esr0_read          (P4P3_DCR_BASE+0x0)
#define p4p3_esr0_write         (P4P3_DCR_BASE+0x1)
#define p4p3_eadr               (P4P3_DCR_BASE+0x2)
#define p4p3_euadr              (P4P3_DCR_BASE+0x3)
#define p4p3_esr1_read          (P4P3_DCR_BASE+0x4)
#define p4p3_esr1_write         (P4P3_DCR_BASE+0x5)
#define p4p3_confg              (P4P3_DCR_BASE+0x6)
#define p4p3_pic                (P4P3_DCR_BASE+0x7)
#define p4p3_peir               (P4P3_DCR_BASE+0x8)
#define p4p3_rev                (P4P3_DCR_BASE+0xA)

/* PLB3 to PLB4 Bridge IN */
#define P3P4_DCR_BASE           0x030
#define p3p4_esr0_read          (P3P4_DCR_BASE+0x0)
#define p3p4_esr0_write         (P3P4_DCR_BASE+0x1)
#define p3p4_eadr               (P3P4_DCR_BASE+0x2)
#define p3p4_euadr              (P3P4_DCR_BASE+0x3)
#define p3p4_esr1_read          (P3P4_DCR_BASE+0x4)
#define p3p4_esr1_write         (P3P4_DCR_BASE+0x5)
#define p3p4_confg              (P3P4_DCR_BASE+0x6)
#define p3p4_pic                (P3P4_DCR_BASE+0x7)
#define p3p4_peir               (P3P4_DCR_BASE+0x8)
#define p3p4_rev                (P3P4_DCR_BASE+0xA)

/* PLB3 Arbiter */
#define PLB3_DCR_BASE           0x070
#define plb3_revid              (PLB3_DCR_BASE+0x2)
#define plb3_besr               (PLB3_DCR_BASE+0x3)
#define plb3_bear               (PLB3_DCR_BASE+0x6)
#define plb3_acr                (PLB3_DCR_BASE+0x7)

/* PLB4 Arbiter - PowerPC440EP Pass1 */
#define PLB4_DCR_BASE           0x080
#define plb4_acr                (PLB4_DCR_BASE+0x1)
#define plb4_revid              (PLB4_DCR_BASE+0x2)
#define plb4_besr               (PLB4_DCR_BASE+0x4)
#define plb4_bearl              (PLB4_DCR_BASE+0x6)
#define plb4_bearh              (PLB4_DCR_BASE+0x7)

#define PLB4_ACR_WRP		(0x80000000 >> 7)

/* Nebula PLB4 Arbiter - PowerPC440EP */
#define PLB_ARBITER_BASE   0x80

#define plb0_revid                (PLB_ARBITER_BASE+ 0x00)
#define plb0_acr                  (PLB_ARBITER_BASE+ 0x01)
#define   plb0_acr_ppm_mask             0xF0000000
#define   plb0_acr_ppm_fixed            0x00000000
#define   plb0_acr_ppm_fair             0xD0000000
#define   plb0_acr_hbu_mask             0x08000000
#define   plb0_acr_hbu_disabled         0x00000000
#define   plb0_acr_hbu_enabled          0x08000000
#define   plb0_acr_rdp_mask             0x06000000
#define   plb0_acr_rdp_disabled         0x00000000
#define   plb0_acr_rdp_2deep            0x02000000
#define   plb0_acr_rdp_3deep            0x04000000
#define   plb0_acr_rdp_4deep            0x06000000
#define   plb0_acr_wrp_mask             0x01000000
#define   plb0_acr_wrp_disabled         0x00000000
#define   plb0_acr_wrp_2deep            0x01000000

#define plb0_besrl                (PLB_ARBITER_BASE+ 0x02)
#define plb0_besrh                (PLB_ARBITER_BASE+ 0x03)
#define plb0_bearl                (PLB_ARBITER_BASE+ 0x04)
#define plb0_bearh                (PLB_ARBITER_BASE+ 0x05)
#define plb0_ccr                  (PLB_ARBITER_BASE+ 0x08)

#define plb1_acr                  (PLB_ARBITER_BASE+ 0x09)
#define   plb1_acr_ppm_mask             0xF0000000
#define   plb1_acr_ppm_fixed            0x00000000
#define   plb1_acr_ppm_fair             0xD0000000
#define   plb1_acr_hbu_mask             0x08000000
#define   plb1_acr_hbu_disabled         0x00000000
#define   plb1_acr_hbu_enabled          0x08000000
#define   plb1_acr_rdp_mask             0x06000000
#define   plb1_acr_rdp_disabled         0x00000000
#define   plb1_acr_rdp_2deep            0x02000000
#define   plb1_acr_rdp_3deep            0x04000000
#define   plb1_acr_rdp_4deep            0x06000000
#define   plb1_acr_wrp_mask             0x01000000
#define   plb1_acr_wrp_disabled         0x00000000
#define   plb1_acr_wrp_2deep            0x01000000

#define plb1_besrl                (PLB_ARBITER_BASE+ 0x0A)
#define plb1_besrh                (PLB_ARBITER_BASE+ 0x0B)
#define plb1_bearl                (PLB_ARBITER_BASE+ 0x0C)
#define plb1_bearh                (PLB_ARBITER_BASE+ 0x0D)

/* Pin Function Control Register 1 */
#define SDR0_PFC1                    0x4101
#define   SDR0_PFC1_U1ME_MASK         0x02000000    /* UART1 Mode Enable */
#define   SDR0_PFC1_U1ME_DSR_DTR      0x00000000      /* UART1 in DSR/DTR Mode */
#define   SDR0_PFC1_U1ME_CTS_RTS      0x02000000      /* UART1 in CTS/RTS Mode */
#define   SDR0_PFC1_U0ME_MASK         0x00080000    /* UART0 Mode Enable */
#define   SDR0_PFC1_U0ME_DSR_DTR      0x00000000      /* UART0 in DSR/DTR Mode */
#define   SDR0_PFC1_U0ME_CTS_RTS      0x00080000      /* UART0 in CTS/RTS Mode */
#define   SDR0_PFC1_U0IM_MASK         0x00040000    /* UART0 Interface Mode */
#define   SDR0_PFC1_U0IM_8PINS        0x00000000      /* UART0 Interface Mode 8 pins */
#define   SDR0_PFC1_U0IM_4PINS        0x00040000      /* UART0 Interface Mode 4 pins */
#define   SDR0_PFC1_SIS_MASK          0x00020000    /* SCP or IIC1 Selection */
#define   SDR0_PFC1_SIS_SCP_SEL       0x00000000      /* SCP Selected */
#define   SDR0_PFC1_SIS_IIC1_SEL      0x00020000      /* IIC1 Selected */
#define   SDR0_PFC1_UES_MASK          0x00010000    /* USB2D_RX_Active / EBC_Hold Req Selection */
#define   SDR0_PFC1_UES_USB2D_SEL     0x00000000      /* USB2D_RX_Active Selected */
#define   SDR0_PFC1_UES_EBCHR_SEL     0x00010000      /* EBC_Hold Req Selected */
#define   SDR0_PFC1_DIS_MASK          0x00008000    /* DMA_Req(1) / UIC_IRQ(5) Selection */
#define   SDR0_PFC1_DIS_DMAR_SEL      0x00000000      /* DMA_Req(1) Selected */
#define   SDR0_PFC1_DIS_UICIRQ5_SEL   0x00008000      /* UIC_IRQ(5) Selected */
#define   SDR0_PFC1_ERE_MASK          0x00004000    /* EBC Mast.Ext.Req.En./GPIO0(27) Selection */
#define   SDR0_PFC1_ERE_EXTR_SEL      0x00000000      /* EBC Mast.Ext.Req.En. Selected */
#define   SDR0_PFC1_ERE_GPIO0_27_SEL  0x00004000      /* GPIO0(27) Selected */
#define   SDR0_PFC1_UPR_MASK          0x00002000    /* USB2 Device Packet Reject Selection */
#define   SDR0_PFC1_UPR_DISABLE       0x00000000      /* USB2 Device Packet Reject Disable */
#define   SDR0_PFC1_UPR_ENABLE        0x00002000      /* USB2 Device Packet Reject Enable */

#define   SDR0_PFC1_PLB_PME_MASK      0x00001000    /* PLB3/PLB4 Perf. Monitor En. Selection */
#define   SDR0_PFC1_PLB_PME_PLB3_SEL  0x00000000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_PLB_PME_PLB4_SEL  0x00001000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_GFGGI_MASK        0x0000000F    /* GPT Frequency Generation Gated In */

/* USB Control Register */
#define SDR0_USB0                    0x0320
#define   SDR0_USB0_USB_DEVSEL_MASK   0x00000002    /* USB Device Selection */
#define   SDR0_USB0_USB20D_DEVSEL     0x00000000      /* USB2.0 Device Selected */
#define   SDR0_USB0_USB11D_DEVSEL     0x00000002      /* USB1.1 Device Selected */
#define   SDR0_USB0_LEEN_MASK         0x00000001    /* Little Endian selection */
#define   SDR0_USB0_LEEN_DISABLE      0x00000000      /* Little Endian Disable */
#define   SDR0_USB0_LEEN_ENABLE       0x00000001      /* Little Endian Enable */

/* Miscealleneaous Function Reg. */
#define SDR0_MFR                     0x4300
#define   SDR0_MFR_ETH0_CLK_SEL_MASK   0x08000000   /* Ethernet0 Clock Select */
#define   SDR0_MFR_ETH0_CLK_SEL_EXT    0x00000000
#define   SDR0_MFR_ETH1_CLK_SEL_MASK   0x04000000   /* Ethernet1 Clock Select */
#define   SDR0_MFR_ETH1_CLK_SEL_EXT    0x00000000
#define   SDR0_MFR_ZMII_MODE_MASK      0x03000000   /* ZMII Mode Mask */
#define   SDR0_MFR_ZMII_MODE_MII       0x00000000     /* ZMII Mode MII */
#define   SDR0_MFR_ZMII_MODE_SMII      0x01000000     /* ZMII Mode SMII */
#define   SDR0_MFR_ZMII_MODE_RMII_10M  0x02000000     /* ZMII Mode RMII - 10 Mbs */
#define   SDR0_MFR_ZMII_MODE_RMII_100M 0x03000000     /* ZMII Mode RMII - 100 Mbs */
#define   SDR0_MFR_ZMII_MODE_BIT0      0x02000000     /* ZMII Mode Bit0 */
#define   SDR0_MFR_ZMII_MODE_BIT1      0x01000000     /* ZMII Mode Bit1 */
#define   SDR0_MFR_ZM_ENCODE(n)        ((((unsigned long)(n))&0x3)<<24)
#define   SDR0_MFR_ZM_DECODE(n)        ((((unsigned long)(n))<<24)&0x3)

#define   SDR0_MFR_ERRATA3_EN0         0x00800000
#define   SDR0_MFR_ERRATA3_EN1         0x00400000
#define   SDR0_MFR_PKT_REJ_MASK        0x00180000   /* Pkt Rej. Enable Mask */
#define   SDR0_MFR_PKT_REJ_EN          0x00180000   /* Pkt Rej. Enable on both EMAC3 0-1 */
#define   SDR0_MFR_PKT_REJ_EN0         0x00100000   /* Pkt Rej. Enable on EMAC3(0) */
#define   SDR0_MFR_PKT_REJ_EN1         0x00080000   /* Pkt Rej. Enable on EMAC3(1) */
#define   SDR0_MFR_PKT_REJ_POL         0x00200000   /* Packet Reject Polarity */

#define GPT0_COMP6			0x00000098
#define GPT0_COMP5			0x00000094
#define GPT0_COMP4			0x00000090
#define GPT0_COMP3			0x0000008C
#define GPT0_COMP2			0x00000088
#define GPT0_COMP1			0x00000084

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_USB2D0CR                 0x0320
#define   SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK   0x00000004    /* USB 2.0 Device/EBC Master Selection */
#define   SDR0_USB2D0CR_USB2DEV_SELECTION      0x00000004    /* USB 2.0 Device Selection */
#define   SDR0_USB2D0CR_EBC_SELECTION          0x00000000    /* EBC Selection */

#define   SDR0_USB2D0CR_USB_DEV_INT_SEL_MASK   0x00000002    /* USB Device Interface Selection */
#define   SDR0_USB2D0CR_USB20D_DEVSEL          0x00000000      /* USB2.0 Device Selected */
#define   SDR0_USB2D0CR_USB11D_DEVSEL          0x00000002      /* USB1.1 Device Selected */

#define   SDR0_USB2D0CR_LEEN_MASK              0x00000001    /* Little Endian selection */
#define   SDR0_USB2D0CR_LEEN_DISABLE           0x00000000      /* Little Endian Disable */
#define   SDR0_USB2D0CR_LEEN_ENABLE            0x00000001      /* Little Endian Enable */

/* USB2 Host Control Register */
#define SDR0_USB2H0CR                0x0340
#define   SDR0_USB2H0CR_WDINT_MASK             0x00000001 /* Host UTMI Word Interface */
#define   SDR0_USB2H0CR_WDINT_8BIT_60MHZ       0x00000000  /* 8-bit/60MHz */
#define   SDR0_USB2H0CR_WDINT_16BIT_30MHZ      0x00000001  /* 16-bit/30MHz */
#define   SDR0_USB2H0CR_EFLADJ_MASK            0x0000007e /* EHCI Frame Length Adjustment */

/* Pin Function Control Register 1 */
#define SDR0_PFC1                    0x4101
#define   SDR0_PFC1_U1ME_MASK                  0x02000000    /* UART1 Mode Enable */
#define   SDR0_PFC1_U1ME_DSR_DTR               0x00000000      /* UART1 in DSR/DTR Mode */
#define   SDR0_PFC1_U1ME_CTS_RTS               0x02000000      /* UART1 in CTS/RTS Mode */

#define   SDR0_PFC1_SELECT_MASK                0x01C00000 /* Ethernet Pin Select EMAC 0 */
#define   SDR0_PFC1_SELECT_CONFIG_1_1          0x00C00000   /* 1xMII   using RGMII bridge */
#define   SDR0_PFC1_SELECT_CONFIG_1_2          0x00000000   /* 1xMII   using  ZMII bridge */
#define   SDR0_PFC1_SELECT_CONFIG_2            0x00C00000   /* 1xGMII  using RGMII bridge */
#define   SDR0_PFC1_SELECT_CONFIG_3            0x01000000   /* 1xTBI   using RGMII bridge */
#define   SDR0_PFC1_SELECT_CONFIG_4            0x01400000   /* 2xRGMII using RGMII bridge */
#define   SDR0_PFC1_SELECT_CONFIG_5            0x01800000   /* 2xRTBI  using RGMII bridge */
#define   SDR0_PFC1_SELECT_CONFIG_6            0x00800000   /* 2xSMII  using  ZMII bridge */

#define   SDR0_PFC1_U0ME_MASK                  0x00080000    /* UART0 Mode Enable */
#define   SDR0_PFC1_U0ME_DSR_DTR               0x00000000      /* UART0 in DSR/DTR Mode */
#define   SDR0_PFC1_U0ME_CTS_RTS               0x00080000      /* UART0 in CTS/RTS Mode */
#define   SDR0_PFC1_U0IM_MASK                  0x00040000    /* UART0 Interface Mode */
#define   SDR0_PFC1_U0IM_8PINS                 0x00000000      /* UART0 Interface Mode 8 pins */
#define   SDR0_PFC1_U0IM_4PINS                 0x00040000      /* UART0 Interface Mode 4 pins */
#define   SDR0_PFC1_SIS_MASK                   0x00020000    /* SCP or IIC1 Selection */
#define   SDR0_PFC1_SIS_SCP_SEL                0x00000000      /* SCP Selected */
#define   SDR0_PFC1_SIS_IIC1_SEL               0x00020000      /* IIC1 Selected */
#define   SDR0_PFC1_UES_MASK                   0x00010000    /* USB2D_RX_Active / EBC_Hold Req Selection */
#define   SDR0_PFC1_UES_USB2D_SEL              0x00000000      /* USB2D_RX_Active Selected */
#define   SDR0_PFC1_UES_EBCHR_SEL              0x00010000      /* EBC_Hold Req Selected */
#define   SDR0_PFC1_DIS_MASK                   0x00008000    /* DMA_Req(1) / UIC_IRQ(5) Selection */
#define   SDR0_PFC1_DIS_DMAR_SEL               0x00000000      /* DMA_Req(1) Selected */
#define   SDR0_PFC1_DIS_UICIRQ5_SEL            0x00008000      /* UIC_IRQ(5) Selected */
#define   SDR0_PFC1_ERE_MASK                   0x00004000    /* EBC Mast.Ext.Req.En./GPIO0(27) Selection */
#define   SDR0_PFC1_ERE_EXTR_SEL               0x00000000      /* EBC Mast.Ext.Req.En. Selected */
#define   SDR0_PFC1_ERE_GPIO0_27_SEL           0x00004000      /* GPIO0(27) Selected */
#define   SDR0_PFC1_UPR_MASK                   0x00002000    /* USB2 Device Packet Reject Selection */
#define   SDR0_PFC1_UPR_DISABLE                0x00000000      /* USB2 Device Packet Reject Disable */
#define   SDR0_PFC1_UPR_ENABLE                 0x00002000      /* USB2 Device Packet Reject Enable */

#define   SDR0_PFC1_PLB_PME_MASK               0x00001000    /* PLB3/PLB4 Perf. Monitor En. Selection */
#define   SDR0_PFC1_PLB_PME_PLB3_SEL           0x00000000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_PLB_PME_PLB4_SEL           0x00001000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_GFGGI_MASK                 0x0000000F    /* GPT Frequency Generation Gated In */

/* Ethernet PLL Configuration Register */
#define SDR0_PFC2                    0x4102
#define   SDR0_PFC2_TUNE_MASK                  0x01FF8000  /* Loop stability tuning bits */
#define   SDR0_PFC2_MULTI_MASK                 0x00007C00  /* Frequency multiplication selector */
#define   SDR0_PFC2_RANGEB_MASK                0x00000380  /* PLLOUTB/C frequency selector */
#define   SDR0_PFC2_RANGEA_MASK                0x00000071  /* PLLOUTA frequency selector */

#define   SDR0_PFC2_SELECT_MASK                0xE0000000  /* Ethernet Pin select EMAC1 */
#define   SDR0_PFC2_SELECT_CONFIG_1_1          0x60000000   /* 1xMII   using RGMII bridge */
#define   SDR0_PFC2_SELECT_CONFIG_1_2          0x00000000   /* 1xMII   using  ZMII bridge */
#define   SDR0_PFC2_SELECT_CONFIG_2            0x60000000   /* 1xGMII  using RGMII bridge */
#define   SDR0_PFC2_SELECT_CONFIG_3            0x80000000   /* 1xTBI   using RGMII bridge */
#define   SDR0_PFC2_SELECT_CONFIG_4            0xA0000000   /* 2xRGMII using RGMII bridge */
#define   SDR0_PFC2_SELECT_CONFIG_5            0xC0000000   /* 2xRTBI  using RGMII bridge */
#define   SDR0_PFC2_SELECT_CONFIG_6            0x40000000   /* 2xSMII  using  ZMII bridge */

#define SDR0_PFC4		0x4104

/* USB2PHY0 Control Register */
#define SDR0_USB2PHY0CR               0x4103
#define   SDR0_USB2PHY0CR_UTMICN_MASK          0x00100000 /*  PHY UTMI interface connection */
#define   SDR0_USB2PHY0CR_UTMICN_DEV           0x00000000  /* Device support */
#define   SDR0_USB2PHY0CR_UTMICN_HOST          0x00100000  /* Host support */

#define   SDR0_USB2PHY0CR_DWNSTR_MASK          0x00400000 /* Select downstream port mode */
#define   SDR0_USB2PHY0CR_DWNSTR_DEV           0x00000000  /* Device */
#define   SDR0_USB2PHY0CR_DWNSTR_HOST          0x00400000  /* Host   */

#define   SDR0_USB2PHY0CR_DVBUS_MASK           0x00800000 /* VBus detect (Device mode only)  */
#define   SDR0_USB2PHY0CR_DVBUS_PURDIS         0x00000000  /* Pull-up resistance on D+ is disabled */
#define   SDR0_USB2PHY0CR_DVBUS_PUREN          0x00800000  /* Pull-up resistance on D+ is enabled */

#define   SDR0_USB2PHY0CR_WDINT_MASK           0x01000000 /* PHY UTMI data width and clock select  */
#define   SDR0_USB2PHY0CR_WDINT_8BIT_60MHZ     0x00000000  /* 8-bit data/60MHz */
#define   SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ    0x01000000  /* 16-bit data/30MHz */

#define   SDR0_USB2PHY0CR_LOOPEN_MASK          0x02000000 /* Loop back test enable  */
#define   SDR0_USB2PHY0CR_LOOP_ENABLE          0x00000000  /* Loop back disabled */
#define   SDR0_USB2PHY0CR_LOOP_DISABLE         0x02000000  /* Loop back enabled (only test purposes) */

#define   SDR0_USB2PHY0CR_XOON_MASK            0x04000000 /* Force XO block on during a suspend  */
#define   SDR0_USB2PHY0CR_XO_ON                0x00000000  /* PHY XO block is powered-on */
#define   SDR0_USB2PHY0CR_XO_OFF               0x04000000  /* PHY XO block is powered-off when all ports are suspended */

#define   SDR0_USB2PHY0CR_PWRSAV_MASK          0x08000000 /* Select PHY power-save mode  */
#define   SDR0_USB2PHY0CR_PWRSAV_OFF           0x00000000  /* Non-power-save mode */
#define   SDR0_USB2PHY0CR_PWRSAV_ON            0x08000000  /* Power-save mode. Valid only for full-speed operation */

#define   SDR0_USB2PHY0CR_XOREF_MASK           0x10000000 /* Select reference clock source  */
#define   SDR0_USB2PHY0CR_XOREF_INTERNAL       0x00000000  /* PHY PLL uses chip internal 48M clock as a reference */
#define   SDR0_USB2PHY0CR_XOREF_XO             0x10000000  /* PHY PLL uses internal XO block output as a reference */

#define   SDR0_USB2PHY0CR_XOCLK_MASK           0x20000000 /* Select clock for XO block  */
#define   SDR0_USB2PHY0CR_XOCLK_EXTERNAL       0x00000000  /* PHY macro used an external clock */
#define   SDR0_USB2PHY0CR_XOCLK_CRYSTAL        0x20000000  /* PHY macro uses the clock from a crystal */

#define   SDR0_USB2PHY0CR_CLKSEL_MASK          0xc0000000 /* Select ref clk freq */
#define   SDR0_USB2PHY0CR_CLKSEL_12MHZ         0x00000000 /* Select ref clk freq = 12 MHz*/
#define   SDR0_USB2PHY0CR_CLKSEL_48MHZ         0x40000000 /* Select ref clk freq = 48 MHz*/
#define   SDR0_USB2PHY0CR_CLKSEL_24MHZ         0x80000000 /* Select ref clk freq = 24 MHz*/

/* Miscealleneaous Function Reg. */
#define SDR0_MFR                     0x4300
#define   SDR0_MFR_ETH0_CLK_SEL_MASK   0x08000000   /* Ethernet0 Clock Select */
#define   SDR0_MFR_ETH0_CLK_SEL_EXT    0x00000000
#define   SDR0_MFR_ETH1_CLK_SEL_MASK   0x04000000   /* Ethernet1 Clock Select */
#define   SDR0_MFR_ETH1_CLK_SEL_EXT    0x00000000
#define   SDR0_MFR_ZMII_MODE_MASK      0x03000000   /* ZMII Mode Mask */
#define   SDR0_MFR_ZMII_MODE_MII       0x00000000     /* ZMII Mode MII */
#define   SDR0_MFR_ZMII_MODE_SMII      0x01000000     /* ZMII Mode SMII */
#define   SDR0_MFR_ZMII_MODE_BIT0      0x02000000     /* ZMII Mode Bit0 */
#define   SDR0_MFR_ZMII_MODE_BIT1      0x01000000     /* ZMII Mode Bit1 */
#define   SDR0_MFR_ZM_ENCODE(n)        ((((unsigned long)(n))&0x3)<<24)
#define   SDR0_MFR_ZM_DECODE(n)        ((((unsigned long)(n))<<24)&0x3)

#define   SDR0_MFR_ERRATA3_EN0         0x00800000
#define   SDR0_MFR_ERRATA3_EN1         0x00400000
#define   SDR0_MFR_PKT_REJ_MASK        0x00180000   /* Pkt Rej. Enable Mask */
#define   SDR0_MFR_PKT_REJ_EN          0x00180000   /* Pkt Rej. Enable on both EMAC3 0-1 */
#define   SDR0_MFR_PKT_REJ_EN0         0x00100000   /* Pkt Rej. Enable on EMAC3(0) */
#define   SDR0_MFR_PKT_REJ_EN1         0x00080000   /* Pkt Rej. Enable on EMAC3(1) */
#define   SDR0_MFR_PKT_REJ_POL         0x00200000   /* Packet Reject Polarity */

#endif /* defined(CONFIG_440EPX) || defined(CONFIG_440GRX) */

/* CUST1 Customer Configuration Register1 */
#define   SDR0_CUST1                 0x4002
#define   SDR0_CUST1_NDRSC_MASK       0xFFFF0000     /* NDRSC Device Read Count */
#define   SDR0_CUST1_NDRSC_ENCODE(n) ((((unsigned long)(n))&0xFFFF)<<16)
#define   SDR0_CUST1_NDRSC_DECODE(n) ((((unsigned long)(n))>>16)&0xFFFF)

/* Pin Function Control Register 0 */
#define SDR0_PFC0                    0x4100
#define   SDR0_PFC0_CPU_TR_EN_MASK    0x00000100    /* CPU Trace Enable Mask */
#define   SDR0_PFC0_CPU_TRACE_EN      0x00000100      /* CPU Trace Enable */
#define   SDR0_PFC0_CPU_TRACE_DIS     0x00000100      /* CPU Trace Disable */
#define   SDR0_PFC0_CTE_ENCODE(n)    ((((unsigned long)(n))&0x01)<<8)
#define   SDR0_PFC0_CTE_DECODE(n)    ((((unsigned long)(n))>>8)&0x01)

/* Pin Function Control Register 1 */
#define SDR0_PFC1                    0x4101
#define   SDR0_PFC1_U1ME_MASK         0x02000000    /* UART1 Mode Enable */
#define   SDR0_PFC1_U1ME_DSR_DTR      0x00000000      /* UART1 in DSR/DTR Mode */
#define   SDR0_PFC1_U1ME_CTS_RTS      0x02000000      /* UART1 in CTS/RTS Mode */
#define   SDR0_PFC1_U0ME_MASK         0x00080000    /* UART0 Mode Enable */
#define   SDR0_PFC1_U0ME_DSR_DTR      0x00000000      /* UART0 in DSR/DTR Mode */
#define   SDR0_PFC1_U0ME_CTS_RTS      0x00080000      /* UART0 in CTS/RTS Mode */
#define   SDR0_PFC1_U0IM_MASK         0x00040000    /* UART0 Interface Mode */
#define   SDR0_PFC1_U0IM_8PINS        0x00000000      /* UART0 Interface Mode 8 pins */
#define   SDR0_PFC1_U0IM_4PINS        0x00040000      /* UART0 Interface Mode 4 pins */
#define   SDR0_PFC1_SIS_MASK          0x00020000    /* SCP or IIC1 Selection */
#define   SDR0_PFC1_SIS_SCP_SEL       0x00000000      /* SCP Selected */
#define   SDR0_PFC1_SIS_IIC1_SEL      0x00020000      /* IIC1 Selected */
#define   SDR0_PFC1_UES_MASK          0x00010000    /* USB2D_RX_Active / EBC_Hold Req Selection */
#define   SDR0_PFC1_UES_USB2D_SEL     0x00000000      /* USB2D_RX_Active Selected */
#define   SDR0_PFC1_UES_EBCHR_SEL     0x00010000      /* EBC_Hold Req Selected */
#define   SDR0_PFC1_DIS_MASK          0x00008000    /* DMA_Req(1) / UIC_IRQ(5) Selection */
#define   SDR0_PFC1_DIS_DMAR_SEL      0x00000000      /* DMA_Req(1) Selected */
#define   SDR0_PFC1_DIS_UICIRQ5_SEL   0x00008000      /* UIC_IRQ(5) Selected */
#define   SDR0_PFC1_ERE_MASK          0x00004000    /* EBC Mast.Ext.Req.En./GPIO0(27) Selection */
#define   SDR0_PFC1_ERE_EXTR_SEL      0x00000000      /* EBC Mast.Ext.Req.En. Selected */
#define   SDR0_PFC1_ERE_GPIO0_27_SEL  0x00004000      /* GPIO0(27) Selected */
#define   SDR0_PFC1_UPR_MASK          0x00002000    /* USB2 Device Packet Reject Selection */
#define   SDR0_PFC1_UPR_DISABLE       0x00000000      /* USB2 Device Packet Reject Disable */
#define   SDR0_PFC1_UPR_ENABLE        0x00002000      /* USB2 Device Packet Reject Enable */

#define   SDR0_PFC1_PLB_PME_MASK      0x00001000    /* PLB3/PLB4 Perf. Monitor En. Selection */
#define   SDR0_PFC1_PLB_PME_PLB3_SEL  0x00000000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_PLB_PME_PLB4_SEL  0x00001000      /* PLB3 Performance Monitor Enable */
#define   SDR0_PFC1_GFGGI_MASK        0x0000000F    /* GPT Frequency Generation Gated In */

#endif /* 440EP || 440GR || 440EPX || 440GRX */

/*-----------------------------------------------------------------------------
 | L2 Cache
 +----------------------------------------------------------------------------*/
#if defined (CONFIG_440GX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define L2_CACHE_BASE	0x030
#define l2_cache_cfg	(L2_CACHE_BASE+0x00)	/* L2 Cache Config	*/
#define l2_cache_cmd	(L2_CACHE_BASE+0x01)	/* L2 Cache Command	*/
#define l2_cache_addr	(L2_CACHE_BASE+0x02)	/* L2 Cache Address	*/
#define l2_cache_data	(L2_CACHE_BASE+0x03)	/* L2 Cache Data	*/
#define l2_cache_stat	(L2_CACHE_BASE+0x04)	/* L2 Cache Status	*/
#define l2_cache_cver	(L2_CACHE_BASE+0x05)	/* L2 Cache Revision ID */
#define l2_cache_snp0	(L2_CACHE_BASE+0x06)	/* L2 Cache Snoop reg 0 */
#define l2_cache_snp1	(L2_CACHE_BASE+0x07)	/* L2 Cache Snoop reg 1 */

#endif /* CONFIG_440GX */

/*-----------------------------------------------------------------------------
 | Internal SRAM
 +----------------------------------------------------------------------------*/
#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define ISRAM0_DCR_BASE 0x380
#else
#define ISRAM0_DCR_BASE 0x020
#endif
#define isram0_sb0cr	(ISRAM0_DCR_BASE+0x00)	/* SRAM bank config 0*/
#define isram0_sb1cr	(ISRAM0_DCR_BASE+0x01)	/* SRAM bank config 1*/
#define isram0_sb2cr	(ISRAM0_DCR_BASE+0x02)	/* SRAM bank config 2*/
#define isram0_sb3cr	(ISRAM0_DCR_BASE+0x03)	/* SRAM bank config 3*/
#define isram0_bear	(ISRAM0_DCR_BASE+0x04)	/* SRAM bus error addr reg */
#define isram0_besr0	(ISRAM0_DCR_BASE+0x05)	/* SRAM bus error status reg 0 */
#define isram0_besr1	(ISRAM0_DCR_BASE+0x06)	/* SRAM bus error status reg 1 */
#define isram0_pmeg	(ISRAM0_DCR_BASE+0x07)	/* SRAM power management */
#define isram0_cid	(ISRAM0_DCR_BASE+0x08)	/* SRAM bus core id reg */
#define isram0_revid	(ISRAM0_DCR_BASE+0x09)	/* SRAM bus revision id reg */
#define isram0_dpc	(ISRAM0_DCR_BASE+0x0a)	/* SRAM data parity check reg */

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
/* CUST0 Customer Configuration Register0 */
#define SDR0_CUST0                   0x4000
#define   SDR0_CUST0_MUX_E_N_G_MASK   0xC0000000     /* Mux_Emac_NDFC_GPIO */
#define   SDR0_CUST0_MUX_EMAC_SEL     0x40000000       /* Emac Selection */
#define   SDR0_CUST0_MUX_NDFC_SEL     0x80000000       /* NDFC Selection */
#define   SDR0_CUST0_MUX_GPIO_SEL     0xC0000000       /* GPIO Selection */

#define   SDR0_CUST0_NDFC_EN_MASK     0x20000000     /* NDFC Enable Mask */
#define   SDR0_CUST0_NDFC_ENABLE      0x20000000       /* NDFC Enable */
#define   SDR0_CUST0_NDFC_DISABLE     0x00000000       /* NDFC Disable */

#define   SDR0_CUST0_NDFC_BW_MASK     0x10000000     /* NDFC Boot Width */
#define   SDR0_CUST0_NDFC_BW_16_BIT   0x10000000       /* NDFC Boot Width = 16 Bit */
#define   SDR0_CUST0_NDFC_BW_8_BIT    0x00000000       /* NDFC Boot Width =  8 Bit */

#define   SDR0_CUST0_NDFC_BP_MASK     0x0F000000     /* NDFC Boot Page */
#define   SDR0_CUST0_NDFC_BP_ENCODE(n) ((((unsigned long)(n))&0xF)<<24)
#define   SDR0_CUST0_NDFC_BP_DECODE(n) ((((unsigned long)(n))>>24)&0x0F)

#define   SDR0_CUST0_NDFC_BAC_MASK    0x00C00000     /* NDFC Boot Address Cycle */
#define   SDR0_CUST0_NDFC_BAC_ENCODE(n) ((((unsigned long)(n))&0x3)<<22)
#define   SDR0_CUST0_NDFC_BAC_DECODE(n) ((((unsigned long)(n))>>22)&0x03)

#define   SDR0_CUST0_NDFC_ARE_MASK    0x00200000     /* NDFC Auto Read Enable */
#define   SDR0_CUST0_NDFC_ARE_ENABLE  0x00200000       /* NDFC Auto Read Enable */
#define   SDR0_CUST0_NDFC_ARE_DISABLE 0x00000000       /* NDFC Auto Read Disable */

#define   SDR0_CUST0_NRB_MASK         0x00100000     /* NDFC Ready / Busy */
#define   SDR0_CUST0_NRB_BUSY         0x00100000       /* Busy */
#define   SDR0_CUST0_NRB_READY        0x00000000       /* Ready */

#define   SDR0_CUST0_NDRSC_MASK       0x0000FFF0     /* NDFC Device Reset Count Mask */
#define   SDR0_CUST0_NDRSC_ENCODE(n) ((((unsigned long)(n))&0xFFF)<<4)
#define   SDR0_CUST0_NDRSC_DECODE(n) ((((unsigned long)(n))>>4)&0xFFF)

#define   SDR0_CUST0_CHIPSELGAT_MASK  0x0000000F     /* Chip Select Gating Mask */
#define   SDR0_CUST0_CHIPSELGAT_DIS   0x00000000       /* Chip Select Gating Disable */
#define   SDR0_CUST0_CHIPSELGAT_ENALL 0x0000000F       /* All Chip Select Gating Enable */
#define   SDR0_CUST0_CHIPSELGAT_EN0   0x00000008       /* Chip Select0 Gating Enable */
#define   SDR0_CUST0_CHIPSELGAT_EN1   0x00000004       /* Chip Select1 Gating Enable */
#define   SDR0_CUST0_CHIPSELGAT_EN2   0x00000002       /* Chip Select2 Gating Enable */
#define   SDR0_CUST0_CHIPSELGAT_EN3   0x00000001       /* Chip Select3 Gating Enable */
#endif

/*-----------------------------------------------------------------------------
 | On-Chip Buses
 +----------------------------------------------------------------------------*/
/* TODO: as needed */

/*-----------------------------------------------------------------------------
 | Clocking, Power Management and Chip Control
 +----------------------------------------------------------------------------*/
#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define CNTRL_DCR_BASE 0x160
#else
#define CNTRL_DCR_BASE 0x0b0
#endif

#define cpc0_er		(CNTRL_DCR_BASE+0x00)	/* CPM enable register		*/
#define cpc0_fr		(CNTRL_DCR_BASE+0x01)	/* CPM force register		*/
#define cpc0_sr		(CNTRL_DCR_BASE+0x02)	/* CPM status register		*/

#define cpc0_sys0	(CNTRL_DCR_BASE+0x30)	/* System configuration reg 0	*/
#define cpc0_sys1	(CNTRL_DCR_BASE+0x31)	/* System configuration reg 1	*/
#define cpc0_cust0	(CNTRL_DCR_BASE+0x32)	/* Customer configuration reg 0 */
#define cpc0_cust1	(CNTRL_DCR_BASE+0x33)	/* Customer configuration reg 1 */

#define cpc0_strp0	(CNTRL_DCR_BASE+0x34)	/* Power-on config reg 0 (RO)	*/
#define cpc0_strp1	(CNTRL_DCR_BASE+0x35)	/* Power-on config reg 1 (RO)	*/
#define cpc0_strp2	(CNTRL_DCR_BASE+0x36)	/* Power-on config reg 2 (RO)	*/
#define cpc0_strp3	(CNTRL_DCR_BASE+0x37)	/* Power-on config reg 3 (RO)	*/

#define cpc0_gpio	(CNTRL_DCR_BASE+0x38)	/* GPIO config reg (440GP)	*/

#define cntrl0		(CNTRL_DCR_BASE+0x3b)	/* Control 0 register		*/
#define cntrl1		(CNTRL_DCR_BASE+0x3a)	/* Control 1 register		*/

/*-----------------------------------------------------------------------------
 | Universal interrupt controller
 +----------------------------------------------------------------------------*/
#define UIC_SR	0x0			/* UIC status			   */
#define UIC_ER	0x2			/* UIC enable			   */
#define UIC_CR	0x3			/* UIC critical			   */
#define UIC_PR	0x4			/* UIC polarity			   */
#define UIC_TR	0x5			/* UIC triggering		   */
#define UIC_MSR 0x6			/* UIC masked status		   */
#define UIC_VR	0x7			/* UIC vector			   */
#define UIC_VCR 0x8			/* UIC vector configuration	   */

#define UIC0_DCR_BASE 0xc0
#define uic0sr	(UIC0_DCR_BASE+0x0)   /* UIC0 status			   */
#define uic0er	(UIC0_DCR_BASE+0x2)   /* UIC0 enable			   */
#define uic0cr	(UIC0_DCR_BASE+0x3)   /* UIC0 critical			   */
#define uic0pr	(UIC0_DCR_BASE+0x4)   /* UIC0 polarity			   */
#define uic0tr	(UIC0_DCR_BASE+0x5)   /* UIC0 triggering		   */
#define uic0msr (UIC0_DCR_BASE+0x6)   /* UIC0 masked status		   */
#define uic0vr	(UIC0_DCR_BASE+0x7)   /* UIC0 vector			   */
#define uic0vcr (UIC0_DCR_BASE+0x8)   /* UIC0 vector configuration	   */

#define UIC1_DCR_BASE 0xd0
#define uic1sr	(UIC1_DCR_BASE+0x0)   /* UIC1 status			   */
#define uic1er	(UIC1_DCR_BASE+0x2)   /* UIC1 enable			   */
#define uic1cr	(UIC1_DCR_BASE+0x3)   /* UIC1 critical			   */
#define uic1pr	(UIC1_DCR_BASE+0x4)   /* UIC1 polarity			   */
#define uic1tr	(UIC1_DCR_BASE+0x5)   /* UIC1 triggering		   */
#define uic1msr (UIC1_DCR_BASE+0x6)   /* UIC1 masked status		   */
#define uic1vr	(UIC1_DCR_BASE+0x7)   /* UIC1 vector			   */
#define uic1vcr (UIC1_DCR_BASE+0x8)   /* UIC1 vector configuration	   */

#if defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define UIC2_DCR_BASE 0xe0
#define uic2sr	(UIC2_DCR_BASE+0x0)   /* UIC2 status-Read Clear		*/
#define uic2srs	(UIC2_DCR_BASE+0x1)   /* UIC2 status-Read Set */
#define uic2er	(UIC2_DCR_BASE+0x2)   /* UIC2 enable			*/
#define uic2cr	(UIC2_DCR_BASE+0x3)   /* UIC2 critical			*/
#define uic2pr	(UIC2_DCR_BASE+0x4)   /* UIC2 polarity			*/
#define uic2tr	(UIC2_DCR_BASE+0x5)   /* UIC2 triggering		*/
#define uic2msr (UIC2_DCR_BASE+0x6)   /* UIC2 masked status		*/
#define uic2vr	(UIC2_DCR_BASE+0x7)   /* UIC2 vector			*/
#define uic2vcr (UIC2_DCR_BASE+0x8)   /* UIC2 vector configuration	*/

#define UIC3_DCR_BASE 0xf0
#define uic3sr	(UIC3_DCR_BASE+0x0)   /* UIC3 status-Read Clear		*/
#define uic3srs	(UIC3_DCR_BASE+0x1)   /* UIC3 status-Read Set */
#define uic3er	(UIC3_DCR_BASE+0x2)   /* UIC3 enable			*/
#define uic3cr	(UIC3_DCR_BASE+0x3)   /* UIC3 critical			*/
#define uic3pr	(UIC3_DCR_BASE+0x4)   /* UIC3 polarity			*/
#define uic3tr	(UIC3_DCR_BASE+0x5)   /* UIC3 triggering		*/
#define uic3msr (UIC3_DCR_BASE+0x6)   /* UIC3 masked status		*/
#define uic3vr	(UIC3_DCR_BASE+0x7)   /* UIC3 vector			*/
#define uic3vcr (UIC3_DCR_BASE+0x8)   /* UIC3 vector configuration	*/
#endif /* CONFIG_440SPE */

#if defined(CONFIG_440GX)
#define UIC2_DCR_BASE 0x210
#define uic2sr	(UIC2_DCR_BASE+0x0)   /* UIC2 status			   */
#define uic2er	(UIC2_DCR_BASE+0x2)   /* UIC2 enable			   */
#define uic2cr	(UIC2_DCR_BASE+0x3)   /* UIC2 critical			   */
#define uic2pr	(UIC2_DCR_BASE+0x4)   /* UIC2 polarity			   */
#define uic2tr	(UIC2_DCR_BASE+0x5)   /* UIC2 triggering		   */
#define uic2msr (UIC2_DCR_BASE+0x6)   /* UIC2 masked status		   */
#define uic2vr	(UIC2_DCR_BASE+0x7)   /* UIC2 vector			   */
#define uic2vcr (UIC2_DCR_BASE+0x8)   /* UIC2 vector configuration	   */


#define UIC_DCR_BASE 0x200
#define uicb0sr	 (UIC_DCR_BASE+0x0)   /* UIC Base Status Register	   */
#define uicb0er	 (UIC_DCR_BASE+0x2)   /* UIC Base enable		   */
#define uicb0cr	 (UIC_DCR_BASE+0x3)   /* UIC Base critical		   */
#define uicb0pr	 (UIC_DCR_BASE+0x4)   /* UIC Base polarity		   */
#define uicb0tr	 (UIC_DCR_BASE+0x5)   /* UIC Base triggering		   */
#define uicb0msr (UIC_DCR_BASE+0x6)   /* UIC Base masked status		   */
#define uicb0vr	 (UIC_DCR_BASE+0x7)   /* UIC Base vector		   */
#define uicb0vcr (UIC_DCR_BASE+0x8)   /* UIC Base vector configuration	   */
#endif /* CONFIG_440GX */

/* The following is for compatibility with 405 code */
#define uicsr  uic0sr
#define uicer  uic0er
#define uiccr  uic0cr
#define uicpr  uic0pr
#define uictr  uic0tr
#define uicmsr uic0msr
#define uicvr  uic0vr
#define uicvcr uic0vcr

#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX)
/*----------------------------------------------------------------------------+
| Clock / Power-on-reset DCR's.
+----------------------------------------------------------------------------*/
#define CPR0_CLKUPD			0x20
#define CPR0_CLKUPD_BSY_MASK		0x80000000
#define CPR0_CLKUPD_BSY_COMPLETED	0x00000000
#define CPR0_CLKUPD_BSY_BUSY		0x80000000
#define CPR0_CLKUPD_CUI_MASK		0x80000000
#define CPR0_CLKUPD_CUI_DISABLE		0x00000000
#define CPR0_CLKUPD_CUI_ENABLE		0x80000000
#define CPR0_CLKUPD_CUD_MASK		0x40000000
#define CPR0_CLKUPD_CUD_DISABLE		0x00000000
#define CPR0_CLKUPD_CUD_ENABLE		0x40000000

#define CPR0_PLLC			0x40
#define CPR0_PLLC_RST_MASK		0x80000000
#define CPR0_PLLC_RST_PLLLOCKED		0x00000000
#define CPR0_PLLC_RST_PLLRESET		0x80000000
#define CPR0_PLLC_ENG_MASK		0x40000000
#define CPR0_PLLC_ENG_DISABLE		0x00000000
#define CPR0_PLLC_ENG_ENABLE		0x40000000
#define CPR0_PLLC_ENG_ENCODE(n)		((((unsigned long)(n))&0x01)<<30)
#define CPR0_PLLC_ENG_DECODE(n)		((((unsigned long)(n))>>30)&0x01)
#define CPR0_PLLC_SRC_MASK		0x20000000
#define CPR0_PLLC_SRC_PLLOUTA		0x00000000
#define CPR0_PLLC_SRC_PLLOUTB		0x20000000
#define CPR0_PLLC_SRC_ENCODE(n)		((((unsigned long)(n))&0x01)<<29)
#define CPR0_PLLC_SRC_DECODE(n)		((((unsigned long)(n))>>29)&0x01)
#define CPR0_PLLC_SEL_MASK		0x07000000
#define CPR0_PLLC_SEL_PLLOUT		0x00000000
#define CPR0_PLLC_SEL_CPU		0x01000000
#define CPR0_PLLC_SEL_EBC		0x05000000
#define CPR0_PLLC_SEL_ENCODE(n)		((((unsigned long)(n))&0x07)<<24)
#define CPR0_PLLC_SEL_DECODE(n)		((((unsigned long)(n))>>24)&0x07)
#define CPR0_PLLC_TUNE_MASK		0x000003FF
#define CPR0_PLLC_TUNE_ENCODE(n)	((((unsigned long)(n))&0x3FF)<<0)
#define CPR0_PLLC_TUNE_DECODE(n)	((((unsigned long)(n))>>0)&0x3FF)

#define CPR0_PLLD			0x60
#define CPR0_PLLD_FBDV_MASK		0x1F000000
#define CPR0_PLLD_FBDV_ENCODE(n)	((((unsigned long)(n))&0x1F)<<24)
#define CPR0_PLLD_FBDV_DECODE(n)	((((((unsigned long)(n))>>24)-1)&0x1F)+1)
#define CPR0_PLLD_FWDVA_MASK		0x000F0000
#define CPR0_PLLD_FWDVA_ENCODE(n)	((((unsigned long)(n))&0x0F)<<16)
#define CPR0_PLLD_FWDVA_DECODE(n)	((((((unsigned long)(n))>>16)-1)&0x0F)+1)
#define CPR0_PLLD_FWDVB_MASK		0x00000700
#define CPR0_PLLD_FWDVB_ENCODE(n)	((((unsigned long)(n))&0x07)<<8)
#define CPR0_PLLD_FWDVB_DECODE(n)	((((((unsigned long)(n))>>8)-1)&0x07)+1)
#define CPR0_PLLD_LFBDV_MASK		0x0000003F
#define CPR0_PLLD_LFBDV_ENCODE(n)	((((unsigned long)(n))&0x3F)<<0)
#define CPR0_PLLD_LFBDV_DECODE(n)	((((((unsigned long)(n))>>0)-1)&0x3F)+1)

#define CPR0_PRIMAD			0x80
#define CPR0_PRIMAD_PRADV0_MASK		0x07000000
#define CPR0_PRIMAD_PRADV0_ENCODE(n)	((((unsigned long)(n))&0x07)<<24)
#define CPR0_PRIMAD_PRADV0_DECODE(n)	((((((unsigned long)(n))>>24)-1)&0x07)+1)

#define CPR0_PRIMBD			0xA0
#define CPR0_PRIMBD_PRBDV0_MASK		0x07000000
#define CPR0_PRIMBD_PRBDV0_ENCODE(n)	((((unsigned long)(n))&0x07)<<24)
#define CPR0_PRIMBD_PRBDV0_DECODE(n)	((((((unsigned long)(n))>>24)-1)&0x07)+1)

#define CPR0_OPBD			0xC0
#define CPR0_OPBD_OPBDV0_MASK		0x03000000
#define CPR0_OPBD_OPBDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<24)
#define CPR0_OPBD_OPBDV0_DECODE(n)	((((((unsigned long)(n))>>24)-1)&0x03)+1)

#define CPR0_PERD			0xE0
#if !defined(CONFIG_440EPX)
#define CPR0_PERD_PERDV0_MASK		0x03000000
#define CPR0_PERD_PERDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<24)
#define CPR0_PERD_PERDV0_DECODE(n)	((((((unsigned long)(n))>>24)-1)&0x03)+1)
#endif

#define CPR0_MALD			0x100
#define CPR0_MALD_MALDV0_MASK		0x03000000
#define CPR0_MALD_MALDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<24)
#define CPR0_MALD_MALDV0_DECODE(n)	((((((unsigned long)(n))>>24)-1)&0x03)+1)

#define CPR0_ICFG			0x140
#define CPR0_ICFG_RLI_MASK		0x80000000
#define CPR0_ICFG_RLI_RESETCPR		0x00000000
#define CPR0_ICFG_RLI_PRESERVECPR	0x80000000
#define CPR0_ICFG_ICS_MASK		0x00000007
#define CPR0_ICFG_ICS_ENCODE(n)		((((unsigned long)(n))&0x3F)<<0)
#define CPR0_ICFG_ICS_DECODE(n)		((((((unsigned long)(n))>>0)-1)&0x3F)+1)

/************************/
/* IIC defines          */
/************************/
#define IIC0_MMIO_BASE 0xA0000400
#define IIC1_MMIO_BASE 0xA0000500

#endif /* CONFIG_440SP */

/*-----------------------------------------------------------------------------
 | DMA
 +----------------------------------------------------------------------------*/
#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define DMA_DCR_BASE 0x200
#else
#define DMA_DCR_BASE 0x100
#endif
#define dmacr0	(DMA_DCR_BASE+0x00)  /* DMA channel control register 0	     */
#define dmact0	(DMA_DCR_BASE+0x01)  /* DMA count register 0		     */
#define dmasah0 (DMA_DCR_BASE+0x02)  /* DMA source address high 0	     */
#define dmasal0 (DMA_DCR_BASE+0x03)  /* DMA source address low 0	     */
#define dmadah0 (DMA_DCR_BASE+0x04)  /* DMA destination address high 0	     */
#define dmadal0 (DMA_DCR_BASE+0x05)  /* DMA destination address low 0	     */
#define dmasgh0 (DMA_DCR_BASE+0x06)  /* DMA scatter/gather desc addr high 0  */
#define dmasgl0 (DMA_DCR_BASE+0x07)  /* DMA scatter/gather desc addr low 0   */
#define dmacr1	(DMA_DCR_BASE+0x08)  /* DMA channel control register 1	     */
#define dmact1	(DMA_DCR_BASE+0x09)  /* DMA count register 1		     */
#define dmasah1 (DMA_DCR_BASE+0x0a)  /* DMA source address high 1	     */
#define dmasal1 (DMA_DCR_BASE+0x0b)  /* DMA source address low 1	     */
#define dmadah1 (DMA_DCR_BASE+0x0c)  /* DMA destination address high 1	     */
#define dmadal1 (DMA_DCR_BASE+0x0d)  /* DMA destination address low 1	     */
#define dmasgh1 (DMA_DCR_BASE+0x0e)  /* DMA scatter/gather desc addr high 1  */
#define dmasgl1 (DMA_DCR_BASE+0x0f)  /* DMA scatter/gather desc addr low 1   */
#define dmacr2	(DMA_DCR_BASE+0x10)  /* DMA channel control register 2	     */
#define dmact2	(DMA_DCR_BASE+0x11)  /* DMA count register 2		     */
#define dmasah2 (DMA_DCR_BASE+0x12)  /* DMA source address high 2	     */
#define dmasal2 (DMA_DCR_BASE+0x13)  /* DMA source address low 2	     */
#define dmadah2 (DMA_DCR_BASE+0x14)  /* DMA destination address high 2	     */
#define dmadal2 (DMA_DCR_BASE+0x15)  /* DMA destination address low 2	     */
#define dmasgh2 (DMA_DCR_BASE+0x16)  /* DMA scatter/gather desc addr high 2  */
#define dmasgl2 (DMA_DCR_BASE+0x17)  /* DMA scatter/gather desc addr low 2   */
#define dmacr3	(DMA_DCR_BASE+0x18)  /* DMA channel control register 2	     */
#define dmact3	(DMA_DCR_BASE+0x19)  /* DMA count register 2		     */
#define dmasah3 (DMA_DCR_BASE+0x1a)  /* DMA source address high 2	     */
#define dmasal3 (DMA_DCR_BASE+0x1b)  /* DMA source address low 2	     */
#define dmadah3 (DMA_DCR_BASE+0x1c)  /* DMA destination address high 2	     */
#define dmadal3 (DMA_DCR_BASE+0x1d)  /* DMA destination address low 2	     */
#define dmasgh3 (DMA_DCR_BASE+0x1e)  /* DMA scatter/gather desc addr high 2  */
#define dmasgl3 (DMA_DCR_BASE+0x1f)  /* DMA scatter/gather desc addr low 2   */
#define dmasr	(DMA_DCR_BASE+0x20)  /* DMA status register		     */
#define dmasgc	(DMA_DCR_BASE+0x23)  /* DMA scatter/gather command register  */
#define dmaslp	(DMA_DCR_BASE+0x25)  /* DMA sleep mode register		     */
#define dmapol	(DMA_DCR_BASE+0x26)  /* DMA polarity configuration register  */

/*-----------------------------------------------------------------------------
 | Memory Access Layer
 +----------------------------------------------------------------------------*/
#define MAL_DCR_BASE 0x180
#define malmcr	    (MAL_DCR_BASE+0x00) /* MAL Config reg		    */
#define malesr	    (MAL_DCR_BASE+0x01) /* Error Status reg (Read/Clear)    */
#define malier	    (MAL_DCR_BASE+0x02) /* Interrupt enable reg		    */
#define maldbr	    (MAL_DCR_BASE+0x03) /* Mal Debug reg (Read only)	    */
#define maltxcasr   (MAL_DCR_BASE+0x04) /* TX Channel active reg (set)	    */
#define maltxcarr   (MAL_DCR_BASE+0x05) /* TX Channel active reg (Reset)    */
#define maltxeobisr (MAL_DCR_BASE+0x06) /* TX End of buffer int status reg  */
#define maltxdeir   (MAL_DCR_BASE+0x07) /* TX Descr. Error Int reg	    */
#define maltxtattrr (MAL_DCR_BASE+0x08) /* TX PLB attribute reg		    */
#define maltxbattr  (MAL_DCR_BASE+0x09) /* TX descriptor base addr reg	    */
#define malrxcasr   (MAL_DCR_BASE+0x10) /* RX Channel active reg (set)	    */
#define malrxcarr   (MAL_DCR_BASE+0x11) /* RX Channel active reg (Reset)    */
#define malrxeobisr (MAL_DCR_BASE+0x12) /* RX End of buffer int status reg  */
#define malrxdeir   (MAL_DCR_BASE+0x13) /* RX Descr. Error Int reg	    */
#define malrxtattrr (MAL_DCR_BASE+0x14) /* RX PLB attribute reg		    */
#define malrxbattr  (MAL_DCR_BASE+0x15) /* RX descriptor base addr reg	    */
#define maltxctp0r  (MAL_DCR_BASE+0x20) /* TX 0 Channel table pointer reg   */
#define maltxctp1r  (MAL_DCR_BASE+0x21) /* TX 1 Channel table pointer reg   */
#define maltxctp2r  (MAL_DCR_BASE+0x22) /* TX 2 Channel table pointer reg   */
#define maltxctp3r  (MAL_DCR_BASE+0x23) /* TX 3 Channel table pointer reg   */
#define malrxctp0r  (MAL_DCR_BASE+0x40) /* RX 0 Channel table pointer reg   */
#define malrxctp1r  (MAL_DCR_BASE+0x41) /* RX 1 Channel table pointer reg   */
#define malrcbs0    (MAL_DCR_BASE+0x60) /* RX 0 Channel buffer size reg	    */
#define malrcbs1    (MAL_DCR_BASE+0x61) /* RX 1 Channel buffer size reg	    */
#if defined(CONFIG_440GX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define malrxctp2r  (MAL_DCR_BASE+0x42) /* RX 2 Channel table pointer reg   */
#define malrxctp3r  (MAL_DCR_BASE+0x43) /* RX 3 Channel table pointer reg   */
#define malrxctp8r  (MAL_DCR_BASE+0x48) /* RX 8 Channel table pointer reg   */
#define malrxctp16r (MAL_DCR_BASE+0x50) /* RX 16 Channel table pointer reg  */
#define malrxctp24r (MAL_DCR_BASE+0x58) /* RX 24 Channel table pointer reg  */
#define malrcbs2    (MAL_DCR_BASE+0x62) /* RX 2 Channel buffer size reg	    */
#define malrcbs3    (MAL_DCR_BASE+0x63) /* RX 3 Channel buffer size reg	    */
#define malrcbs8    (MAL_DCR_BASE+0x68) /* RX 8 Channel buffer size reg	    */
#define malrcbs16   (MAL_DCR_BASE+0x70) /* RX 16 Channel buffer size reg    */
#define malrcbs24   (MAL_DCR_BASE+0x78) /* RX 24 Channel buffer size reg    */
#endif /* CONFIG_440GX */


/*---------------------------------------------------------------------------+
|  Universal interrupt controller 0 interrupts (UIC0)
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440SP)
#define UIC_U0		0x80000000	/* UART 0			    */
#define UIC_U1		0x40000000	/* UART 1			    */
#define UIC_IIC0	0x20000000	/* IIC				    */
#define UIC_IIC1	0x10000000	/* IIC				    */
#define UIC_PIM		0x08000000	/* PCI0 inbound message		    */
#define UIC_PCRW	0x04000000	/* PCI0 command write register	    */
#define UIC_PPM		0x02000000	/* PCI0 power management	    */
#define UIC_PVPD	0x01000000	/* PCI0 VPD Access		    */
#define UIC_MSI0	0x00800000	/* PCI0 MSI level 0		    */
#define UIC_P1IM	0x00400000	/* PCI1 Inbound Message		    */
#define UIC_P1CRW	0x00200000	/* PCI1 command write register	    */
#define UIC_P1PM	0x00100000	/* PCI1 power management	    */
#define UIC_P1VPD	0x00080000	/* PCI1 VPD Access		    */
#define UIC_P1MSI0	0x00040000	/* PCI1 MSI level 0		    */
#define UIC_P2IM	0x00020000	/* PCI2 inbound message		    */
#define UIC_P2CRW	0x00010000	/* PCI2 command register write	    */
#define UIC_P2PM	0x00008000	/* PCI2 power management	    */
#define UIC_P2VPD	0x00004000	/* PCI2 VPD access		    */
#define UIC_P2MSI0	0x00002000	/* PCI2 MSI level 0		    */
#define UIC_D0CPF	0x00001000	/* DMA0 command pointer		    */
#define UIC_D0CSF	0x00000800	/* DMA0 command status		    */
#define UIC_D1CPF	0x00000400	/* DMA1 command pointer		    */
#define UIC_D1CSF	0x00000200	/* DMA1 command status		    */
#define UIC_I2OID	0x00000100	/* I2O inbound doorbell		    */
#define UIC_I2OPLF	0x00000080	/* I2O inbound post list	    */
#define UIC_I2O0LL	0x00000040	/* I2O0 low latency PLB write	    */
#define UIC_I2O1LL	0x00000020	/* I2O1 low latency PLB write	    */
#define UIC_I2O0HB	0x00000010	/* I2O0 high bandwidth PLB write    */
#define UIC_I2O1HB	0x00000008	/* I2O1 high bandwidth PLB write    */
#define UIC_GPTCT	0x00000004	/* GPT count timer		    */
#define UIC_UIC1NC	0x00000002	/* UIC1 non-critical interrupt	    */
#define UIC_UIC1C	0x00000001	/* UIC1 critical interrupt	    */
#elif defined(CONFIG_440GX) || defined(CONFIG_440EP)
#define UIC_U0		0x80000000	/* UART 0			    */
#define UIC_U1		0x40000000	/* UART 1			    */
#define UIC_IIC0	0x20000000	/* IIC				    */
#define UIC_IIC1	0x10000000	/* IIC				    */
#define UIC_PIM		0x08000000	/* PCI inbound message		    */
#define UIC_PCRW	0x04000000	/* PCI command register write	    */
#define UIC_PPM		0x02000000	/* PCI power management		    */
#define UIC_MSI0	0x01000000	/* PCI MSI level 0		    */
#define UIC_MSI1	0x00800000	/* PCI MSI level 1		    */
#define UIC_MSI2	0x00400000	/* PCI MSI level 2		    */
#define UIC_MTE		0x00200000	/* MAL TXEOB			    */
#define UIC_MRE		0x00100000	/* MAL RXEOB			    */
#define UIC_D0		0x00080000	/* DMA channel 0		    */
#define UIC_D1		0x00040000	/* DMA channel 1		    */
#define UIC_D2		0x00020000	/* DMA channel 2		    */
#define UIC_D3		0x00010000	/* DMA channel 3		    */
#define UIC_RSVD0	0x00008000	/* Reserved			    */
#define UIC_RSVD1	0x00004000	/* Reserved			    */
#define UIC_CT0		0x00002000	/* GPT compare timer 0		    */
#define UIC_CT1		0x00001000	/* GPT compare timer 1		    */
#define UIC_CT2		0x00000800	/* GPT compare timer 2		    */
#define UIC_CT3		0x00000400	/* GPT compare timer 3		    */
#define UIC_CT4		0x00000200	/* GPT compare timer 4		    */
#define UIC_EIR0	0x00000100	/* External interrupt 0		    */
#define UIC_EIR1	0x00000080	/* External interrupt 1		    */
#define UIC_EIR2	0x00000040	/* External interrupt 2		    */
#define UIC_EIR3	0x00000020	/* External interrupt 3		    */
#define UIC_EIR4	0x00000010	/* External interrupt 4		    */
#define UIC_EIR5	0x00000008	/* External interrupt 5		    */
#define UIC_EIR6	0x00000004	/* External interrupt 6		    */
#define UIC_UIC1NC	0x00000002	/* UIC1 non-critical interrupt	    */
#define UIC_UIC1C	0x00000001	/* UIC1 critical interrupt	    */

#elif defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

#define UIC_U0        0x80000000  /* UART 0                             */
#define UIC_U1        0x40000000  /* UART 1                             */
#define UIC_IIC0      0x20000000  /* IIC                                */
#define UIC_KRD       0x10000000  /* Kasumi Ready for data              */
#define UIC_KDA       0x08000000  /* Kasumi Data Available              */
#define UIC_PCRW      0x04000000  /* PCI command register write         */
#define UIC_PPM       0x02000000  /* PCI power management               */
#define UIC_IIC1      0x01000000  /* IIC                                */
#define UIC_SPI       0x00800000  /* SPI                                */
#define UIC_EPCISER   0x00400000  /* External PCI SERR                  */
#define UIC_MTE       0x00200000  /* MAL TXEOB                          */
#define UIC_MRE       0x00100000  /* MAL RXEOB                          */
#define UIC_D0        0x00080000  /* DMA channel 0                      */
#define UIC_D1        0x00040000  /* DMA channel 1                      */
#define UIC_D2        0x00020000  /* DMA channel 2                      */
#define UIC_D3        0x00010000  /* DMA channel 3                      */
#define UIC_UD0       0x00008000  /* UDMA irq 0                         */
#define UIC_UD1       0x00004000  /* UDMA irq 1                         */
#define UIC_UD2       0x00002000  /* UDMA irq 2                         */
#define UIC_UD3       0x00001000  /* UDMA irq 3                         */
#define UIC_HSB2D     0x00000800  /* USB2.0 Device                      */
#define UIC_OHCI1     0x00000400  /* USB2.0 Host OHCI irq 1             */
#define UIC_OHCI2     0x00000200  /* USB2.0 Host OHCI irq 2             */
#define UIC_EIP94     0x00000100  /* Security EIP94                     */
#define UIC_ETH0      0x00000080  /* Emac 0                             */
#define UIC_ETH1      0x00000040  /* Emac 1                             */
#define UIC_EHCI      0x00000020  /* USB2.0 Host EHCI                   */
#define UIC_EIR4      0x00000010  /* External interrupt 4               */
#define UIC_UIC2NC    0x00000008  /* UIC2 non-critical interrupt        */
#define UIC_UIC2C     0x00000004  /* UIC2 critical interrupt            */
#define UIC_UIC1NC    0x00000002  /* UIC1 non-critical interrupt        */
#define UIC_UIC1C     0x00000001  /* UIC1 critical interrupt            */

/* For compatibility with 405 code */
#define UIC_MAL_TXEOB	UIC_MTE
#define UIC_MAL_RXEOB	UIC_MRE

#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)

#define UIC_RSVD0	0x80000000	/* N/A - unused			    */
#define UIC_U1		0x40000000	/* UART 1			    */
#define UIC_IIC0	0x20000000	/* IIC				    */
#define UIC_IIC1	0x10000000	/* IIC				    */
#define UIC_PIM		0x08000000	/* PCI inbound message		    */
#define UIC_PCRW	0x04000000	/* PCI command register write	    */
#define UIC_PPM		0x02000000	/* PCI power management		    */
#define UIC_PCIVPD	0x01000000	/* PCI VPD			    */
#define UIC_MSI0	0x00800000	/* PCI MSI level 0		    */
#define UIC_EIR0	0x00400000	/* External interrupt 0		    */
#define UIC_UIC2NC	0x00200000	/* UIC2 non-critical interrupt	    */
#define UIC_UIC2C	0x00100000	/* UIC2 critical interrupt	    */
#define UIC_D0		0x00080000	/* DMA channel 0		    */
#define UIC_D1		0x00040000	/* DMA channel 1		    */
#define UIC_D2		0x00020000	/* DMA channel 2		    */
#define UIC_D3		0x00010000	/* DMA channel 3		    */
#define UIC_UIC3NC	0x00008000	/* UIC3 non-critical interrupt	    */
#define UIC_UIC3C	0x00004000	/* UIC3 critical interrupt	    */
#define UIC_EIR1	0x00002000	/* External interrupt 1		    */
#define UIC_TRNGDA	0x00001000	/* TRNG data available 		    */
#define UIC_PKAR1	0x00000800	/* PKA ready (PKA[1])		    */
#define UIC_D1CPFF	0x00000400	/* DMA1 cp fifo full		    */
#define UIC_D1CSNS	0x00000200	/* DMA1 cs fifo needs service	    */
#define UIC_I2OID	0x00000100	/* I2O inbound door bell	    */
#define UIC_I2OLNE	0x00000080	/* I2O Inbound Post List FIFO Not Empty */
#define UIC_I20R0LL	0x00000040	/* I2O Region 0 Low Latency PLB Write */
#define UIC_I2OR1LL	0x00000020	/* I2O Region 1 Low Latency PLB Write */
#define UIC_I20R0HB	0x00000010	/* I2O Region 0 High Bandwidth PLB Write */
#define UIC_I2OR1HB	0x00000008	/* I2O Region 1 High Bandwidth PLB Write */
#define UIC_EIP94	0x00000004	/* Security EIP94		    */
#define UIC_UIC1NC	0x00000002	/* UIC1 non-critical interrupt	    */
#define UIC_UIC1C	0x00000001	/* UIC1 critical interrupt	    */

#elif !defined(CONFIG_440SPE)
#define UIC_U0		0x80000000	/* UART 0			    */
#define UIC_U1		0x40000000	/* UART 1			    */
#define UIC_IIC0	0x20000000	/* IIC				    */
#define UIC_IIC1	0x10000000	/* IIC				    */
#define UIC_PIM		0x08000000	/* PCI inbound message		    */
#define UIC_PCRW	0x04000000	/* PCI command register write	    */
#define UIC_PPM		0x02000000	/* PCI power management		    */
#define UIC_MSI0	0x01000000	/* PCI MSI level 0		    */
#define UIC_MSI1	0x00800000	/* PCI MSI level 1		    */
#define UIC_MSI2	0x00400000	/* PCI MSI level 2		    */
#define UIC_MTE		0x00200000	/* MAL TXEOB			    */
#define UIC_MRE		0x00100000	/* MAL RXEOB			    */
#define UIC_D0		0x00080000	/* DMA channel 0		    */
#define UIC_D1		0x00040000	/* DMA channel 1		    */
#define UIC_D2		0x00020000	/* DMA channel 2		    */
#define UIC_D3		0x00010000	/* DMA channel 3		    */
#define UIC_RSVD0	0x00008000	/* Reserved			    */
#define UIC_RSVD1	0x00004000	/* Reserved			    */
#define UIC_CT0		0x00002000	/* GPT compare timer 0		    */
#define UIC_CT1		0x00001000	/* GPT compare timer 1		    */
#define UIC_CT2		0x00000800	/* GPT compare timer 2		    */
#define UIC_CT3		0x00000400	/* GPT compare timer 3		    */
#define UIC_CT4		0x00000200	/* GPT compare timer 4		    */
#define UIC_EIR0	0x00000100	/* External interrupt 0		    */
#define UIC_EIR1	0x00000080	/* External interrupt 1		    */
#define UIC_EIR2	0x00000040	/* External interrupt 2		    */
#define UIC_EIR3	0x00000020	/* External interrupt 3		    */
#define UIC_EIR4	0x00000010	/* External interrupt 4		    */
#define UIC_EIR5	0x00000008	/* External interrupt 5		    */
#define UIC_EIR6	0x00000004	/* External interrupt 6		    */
#define UIC_UIC1NC	0x00000002	/* UIC1 non-critical interrupt	    */
#define UIC_UIC1C	0x00000001	/* UIC1 critical interrupt	    */
#endif /* CONFIG_440GX */

/* For compatibility with 405 code */
#define UIC_MAL_TXEOB	UIC_MTE
#define UIC_MAL_RXEOB	UIC_MRE

/*---------------------------------------------------------------------------+
|  Universal interrupt controller 1 interrupts (UIC1)
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440SP)
#define UIC_EIR0	0x80000000	/* External interrupt 0		    */
#define UIC_MS		0x40000000	/* MAL SERR			    */
#define UIC_MTDE	0x20000000	/* MAL TXDE			    */
#define UIC_MRDE	0x10000000	/* MAL RXDE			    */
#define UIC_DECE	0x08000000	/* DDR SDRAM correctible error	    */
#define UIC_EBCO	0x04000000	/* EBCO interrupt status	    */
#define UIC_MTE		0x02000000	/* MAL TXEOB			    */
#define UIC_MRE		0x01000000	/* MAL RXEOB			    */
#define UIC_P0MSI1	0x00800000	/* PCI0 MSI level 1		    */
#define UIC_P1MSI1	0x00400000	/* PCI1 MSI level 1		    */
#define UIC_P2MSI1	0x00200000	/* PCI2 MSI level 1		    */
#define UIC_L2C		0x00100000	/* L2 cache			    */
#define UIC_CT0		0x00080000	/* GPT compare timer 0		    */
#define UIC_CT1		0x00040000	/* GPT compare timer 1		    */
#define UIC_CT2		0x00020000	/* GPT compare timer 2		    */
#define UIC_CT3		0x00010000	/* GPT compare timer 3		    */
#define UIC_CT4		0x00008000	/* GPT compare timer 4		    */
#define UIC_EIR1	0x00004000	/* External interrupt 1		    */
#define UIC_EIR2	0x00002000	/* External interrupt 2		    */
#define UIC_EIR3	0x00001000	/* External interrupt 3		    */
#define UIC_EIR4	0x00000800	/* External interrupt 4		    */
#define UIC_EIR5	0x00000400	/* External interrupt 5		    */
#define UIC_DMAE	0x00000200	/* DMA error			    */
#define UIC_I2OE	0x00000100	/* I2O error			    */
#define UIC_SRE		0x00000080	/* Serial ROM error		    */
#define UIC_P0AE	0x00000040	/* PCI0 asynchronous error	    */
#define UIC_P1AE	0x00000020	/* PCI1 asynchronous error	    */
#define UIC_P2AE	0x00000010	/* PCI2 asynchronous error	    */
#define UIC_ETH0	0x00000008	/* Ethernet 0			    */
#define UIC_EWU0	0x00000004	/* Ethernet 0 wakeup		    */
#define UIC_ETH1	0x00000002	/* Reserved			    */
#define UIC_XOR		0x00000001	/* XOR				    */
#elif defined(CONFIG_440GX) || defined(CONFIG_440EP)
#define UIC_MS		0x80000000	/* MAL SERR			    */
#define UIC_MTDE	0x40000000	/* MAL TXDE			    */
#define UIC_MRDE	0x20000000	/* MAL RXDE			    */
#define UIC_DEUE	0x10000000	/* DDR SDRAM ECC uncorrectible error*/
#define UIC_DECE	0x08000000	/* DDR SDRAM correctible error	    */
#define UIC_EBCO	0x04000000	/* EBCO interrupt status	    */
#define UIC_EBMI	0x02000000	/* EBMI interrupt status	    */
#define UIC_OPB		0x01000000	/* OPB to PLB bridge interrupt stat */
#define UIC_MSI3	0x00800000	/* PCI MSI level 3		    */
#define UIC_MSI4	0x00400000	/* PCI MSI level 4		    */
#define UIC_MSI5	0x00200000	/* PCI MSI level 5		    */
#define UIC_MSI6	0x00100000	/* PCI MSI level 6		    */
#define UIC_MSI7	0x00080000	/* PCI MSI level 7		    */
#define UIC_MSI8	0x00040000	/* PCI MSI level 8		    */
#define UIC_MSI9	0x00020000	/* PCI MSI level 9		    */
#define UIC_MSI10	0x00010000	/* PCI MSI level 10		    */
#define UIC_MSI11	0x00008000	/* PCI MSI level 11		    */
#define UIC_PPMI	0x00004000	/* PPM interrupt status		    */
#define UIC_EIR7	0x00002000	/* External interrupt 7		    */
#define UIC_EIR8	0x00001000	/* External interrupt 8		    */
#define UIC_EIR9	0x00000800	/* External interrupt 9		    */
#define UIC_EIR10	0x00000400	/* External interrupt 10	    */
#define UIC_EIR11	0x00000200	/* External interrupt 11	    */
#define UIC_EIR12	0x00000100	/* External interrupt 12	    */
#define UIC_SRE		0x00000080	/* Serial ROM error		    */
#define UIC_RSVD2	0x00000040	/* Reserved			    */
#define UIC_RSVD3	0x00000020	/* Reserved			    */
#define UIC_PAE		0x00000010	/* PCI asynchronous error	    */
#define UIC_ETH0	0x00000008	/* Ethernet 0			    */
#define UIC_EWU0	0x00000004	/* Ethernet 0 wakeup		    */
#define UIC_ETH1	0x00000002	/* Ethernet 1			    */
#define UIC_EWU1	0x00000001	/* Ethernet 1 wakeup		    */

#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)

#define UIC_EIR2	0x80000000	/* External interrupt 2		    */
#define UIC_U0		0x40000000	/* UART 0			    */
#define UIC_SPI		0x20000000	/* SPI				    */
#define UIC_TRNGAL	0x10000000	/* TRNG alarm			    */
#define UIC_DEUE	0x08000000	/* DDR SDRAM ECC correct/uncorrectable error */
#define UIC_EBCO	0x04000000	/* EBCO interrupt status	    */
#define UIC_NDFC	0x02000000	/* NDFC				    */
#define UIC_EIPPKPSE	0x01000000	/* EIPPKP slave error		    */
#define UIC_P0MSI1	0x00800000	/* PCI0 MSI level 1		    */
#define UIC_P0MSI2	0x00400000	/* PCI0 MSI level 2		    */
#define UIC_P0MSI3	0x00200000	/* PCI0 MSI level 3		    */
#define UIC_L2C		0x00100000	/* L2 cache			    */
#define UIC_CT0		0x00080000	/* GPT compare timer 0		    */
#define UIC_CT1		0x00040000	/* GPT compare timer 1		    */
#define UIC_CT2		0x00020000	/* GPT compare timer 2		    */
#define UIC_CT3		0x00010000	/* GPT compare timer 3		    */
#define UIC_CT4		0x00008000	/* GPT compare timer 4		    */
#define UIC_CT5		0x00004000	/* GPT compare timer 5		    */
#define UIC_CT6		0x00002000	/* GPT compare timer 6		    */
#define UIC_GPTDC	0x00001000	/* GPT decrementer pulse	    */
#define UIC_EIR3	0x00000800	/* External interrupt 3		    */
#define UIC_EIR4	0x00000400	/* External interrupt 4		    */
#define UIC_DMAE	0x00000200	/* DMA error			    */
#define UIC_I2OE	0x00000100	/* I2O error			    */
#define UIC_SRE		0x00000080	/* Serial ROM error		    */
#define UIC_P0AE	0x00000040	/* PCI0 asynchronous error	    */
#define UIC_EIR5	0x00000020	/* External interrupt 5		    */
#define UIC_EIR6	0x00000010	/* External interrupt 6		    */
#define UIC_U2		0x00000008	/* UART 2			    */
#define UIC_U3		0x00000004	/* UART 3			    */
#define UIC_EIR7	0x00000002	/* External interrupt 7		    */
#define UIC_EIR8	0x00000001	/* External interrupt 8		    */

#elif defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

#define UIC_MS        0x80000000  /* MAL SERR                           */
#define UIC_MTDE      0x40000000  /* MAL TXDE                           */
#define UIC_MRDE      0x20000000  /* MAL RXDE                           */
#define UIC_U2        0x10000000  /* UART 2                             */
#define UIC_U3        0x08000000  /* UART 3                             */
#define UIC_EBCO      0x04000000  /* EBCO interrupt status              */
#define UIC_NDFC      0x02000000  /* NDFC                               */
#define UIC_KSLE      0x01000000  /* KASUMI slave error                 */
#define UIC_CT5       0x00800000  /* GPT compare timer 5                */
#define UIC_CT6       0x00400000  /* GPT compare timer 6                */
#define UIC_PLB34I0   0x00200000  /* PLB3X4X MIRQ0                      */
#define UIC_PLB34I1   0x00100000  /* PLB3X4X MIRQ1                      */
#define UIC_PLB34I2   0x00080000  /* PLB3X4X MIRQ2                      */
#define UIC_PLB34I3   0x00040000  /* PLB3X4X MIRQ3                      */
#define UIC_PLB34I4   0x00020000  /* PLB3X4X MIRQ4                      */
#define UIC_PLB34I5   0x00010000  /* PLB3X4X MIRQ5                      */
#define UIC_CT0       0x00008000  /* GPT compare timer 0                */
#define UIC_CT1       0x00004000  /* GPT compare timer 1                */
#define UIC_EIR7      0x00002000  /* External interrupt 7               */
#define UIC_EIR8      0x00001000  /* External interrupt 8               */
#define UIC_EIR9      0x00000800  /* External interrupt 9               */
#define UIC_CT2       0x00000400  /* GPT compare timer 2                */
#define UIC_CT3       0x00000200  /* GPT compare timer 3                */
#define UIC_CT4       0x00000100  /* GPT compare timer 4                */
#define UIC_SRE       0x00000080  /* Serial ROM error                   */
#define UIC_GPTDC     0x00000040  /* GPT decrementer pulse              */
#define UIC_RSVD0     0x00000020  /* Reserved                           */
#define UIC_EPCIPER   0x00000010  /* External PCI PERR                  */
#define UIC_EIR0      0x00000008  /* External interrupt 0               */
#define UIC_EWU0      0x00000004  /* Ethernet 0 wakeup                  */
#define UIC_EIR1      0x00000002  /* External interrupt 1               */
#define UIC_EWU1      0x00000001  /* Ethernet 1 wakeup                  */

/* For compatibility with 405 code */
#define UIC_MAL_SERR	UIC_MS
#define UIC_MAL_TXDE	UIC_MTDE
#define UIC_MAL_RXDE	UIC_MRDE
#define UIC_ENET	UIC_ETH0

#elif !defined(CONFIG_440SPE)
#define UIC_MS		0x80000000	/* MAL SERR			    */
#define UIC_MTDE	0x40000000	/* MAL TXDE			    */
#define UIC_MRDE	0x20000000	/* MAL RXDE			    */
#define UIC_DEUE	0x10000000	/* DDR SDRAM ECC uncorrectible error*/
#define UIC_DECE	0x08000000	/* DDR SDRAM correctible error	    */
#define UIC_EBCO	0x04000000	/* EBCO interrupt status	    */
#define UIC_EBMI	0x02000000	/* EBMI interrupt status	    */
#define UIC_OPB		0x01000000	/* OPB to PLB bridge interrupt stat */
#define UIC_MSI3	0x00800000	/* PCI MSI level 3		    */
#define UIC_MSI4	0x00400000	/* PCI MSI level 4		    */
#define UIC_MSI5	0x00200000	/* PCI MSI level 5		    */
#define UIC_MSI6	0x00100000	/* PCI MSI level 6		    */
#define UIC_MSI7	0x00080000	/* PCI MSI level 7		    */
#define UIC_MSI8	0x00040000	/* PCI MSI level 8		    */
#define UIC_MSI9	0x00020000	/* PCI MSI level 9		    */
#define UIC_MSI10	0x00010000	/* PCI MSI level 10		    */
#define UIC_MSI11	0x00008000	/* PCI MSI level 11		    */
#define UIC_PPMI	0x00004000	/* PPM interrupt status		    */
#define UIC_EIR7	0x00002000	/* External interrupt 7		    */
#define UIC_EIR8	0x00001000	/* External interrupt 8		    */
#define UIC_EIR9	0x00000800	/* External interrupt 9		    */
#define UIC_EIR10	0x00000400	/* External interrupt 10	    */
#define UIC_EIR11	0x00000200	/* External interrupt 11	    */
#define UIC_EIR12	0x00000100	/* External interrupt 12	    */
#define UIC_SRE		0x00000080	/* Serial ROM error		    */
#define UIC_RSVD2	0x00000040	/* Reserved			    */
#define UIC_RSVD3	0x00000020	/* Reserved			    */
#define UIC_PAE		0x00000010	/* PCI asynchronous error	    */
#define UIC_ETH0	0x00000008	/* Ethernet 0			    */
#define UIC_EWU0	0x00000004	/* Ethernet 0 wakeup		    */
#define UIC_ETH1	0x00000002	/* Ethernet 1			    */
#define UIC_EWU1	0x00000001	/* Ethernet 1 wakeup		    */
#endif /* CONFIG_440SP */

/* For compatibility with 405 code */
#define UIC_MAL_SERR	UIC_MS
#define UIC_MAL_TXDE	UIC_MTDE
#define UIC_MAL_RXDE	UIC_MRDE
#define UIC_ENET	UIC_ETH0

/*---------------------------------------------------------------------------+
|  Universal interrupt controller 2 interrupts (UIC2)
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440GX)
#define UIC_ETH2	0x80000000	/* Ethernet 2			    */
#define UIC_EWU2	0x40000000	/* Ethernet 2 wakeup		    */
#define UIC_ETH3	0x20000000	/* Ethernet 3			    */
#define UIC_EWU3	0x10000000	/* Ethernet 3 wakeup		    */
#define UIC_TAH0	0x08000000	/* TAH 0			    */
#define UIC_TAH1	0x04000000	/* TAH 1			    */
#define UIC_IMUOBFQ	0x02000000	/* IMU outbound free queue	    */
#define UIC_IMUIBPQ	0x01000000	/* IMU inbound post queue	    */
#define UIC_IMUIRQDB	0x00800000	/* IMU irq doorbell		    */
#define UIC_IMUIBDB	0x00400000	/* IMU inbound doorbell		    */
#define UIC_IMUMSG0	0x00200000	/* IMU inbound message 0	    */
#define UIC_IMUMSG1	0x00100000	/* IMU inbound message 1	    */
#define UIC_IMUTO	0x00080000	/* IMU timeout			    */
#define UIC_MSI12	0x00040000	/* PCI MSI level 12		    */
#define UIC_MSI13	0x00020000	/* PCI MSI level 13		    */
#define UIC_MSI14	0x00010000	/* PCI MSI level 14		    */
#define UIC_MSI15	0x00008000	/* PCI MSI level 15		    */
#define UIC_EIR13	0x00004000	/* External interrupt 13	    */
#define UIC_EIR14	0x00002000	/* External interrupt 14	    */
#define UIC_EIR15	0x00001000	/* External interrupt 15	    */
#define UIC_EIR16	0x00000800	/* External interrupt 16	    */
#define UIC_EIR17	0x00000400	/* External interrupt 17	    */
#define UIC_PCIVPD	0x00000200	/* PCI VPD			    */
#define UIC_L2C		0x00000100	/* L2 Cache			    */
#define UIC_ETH2PCS	0x00000080	/* Ethernet 2 PCS		    */
#define UIC_ETH3PCS	0x00000040	/* Ethernet 3 PCS		    */
#define UIC_RSVD26	0x00000020	/* Reserved			    */
#define UIC_RSVD27	0x00000010	/* Reserved			    */
#define UIC_RSVD28	0x00000008	/* Reserved			    */
#define UIC_RSVD29	0x00000004	/* Reserved			    */
#define UIC_RSVD30	0x00000002	/* Reserved			    */
#define UIC_RSVD31	0x00000001	/* Reserved			    */

#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)

#define UIC_TAH0	0x80000000	/* TAHOE 0			    */
#define UIC_TAH1	0x40000000	/* TAHOE 1			    */
#define UIC_EIR9	0x20000000	/* External interrupt 9		    */
#define UIC_MS		0x10000000	/* MAL SERR			    */
#define UIC_MTDE	0x08000000	/* MAL TXDE			    */
#define UIC_MRDE	0x04000000	/* MAL RXDE			    */
#define UIC_MTE		0x02000000	/* MAL TXEOB			    */
#define UIC_MRE		0x01000000	/* MAL RXEOB			    */
#define UIC_MCTX0	0x00800000	/* MAL interrupt coalescence TX0    */
#define UIC_MCTX1	0x00400000	/* MAL interrupt coalescence TX1    */
#define UIC_MCTX2	0x00200000	/* MAL interrupt coalescence TX2    */
#define UIC_MCTX3	0x00100000	/* MAL interrupt coalescence TX3    */
#define UIC_MCTR0	0x00080000	/* MAL interrupt coalescence TR0    */
#define UIC_MCTR1	0x00040000	/* MAL interrupt coalescence TR1    */
#define UIC_MCTR2	0x00020000	/* MAL interrupt coalescence TR2    */
#define UIC_MCTR3	0x00010000	/* MAL interrupt coalescence TR3    */
#define UIC_ETH0	0x00008000	/* Ethernet 0			    */
#define UIC_ETH1	0x00004000	/* Ethernet 1			    */
#define UIC_ETH2	0x00002000	/* Ethernet 2			    */
#define UIC_ETH3	0x00001000	/* Ethernet 3			    */
#define UIC_EWU0	0x00000800	/* Ethernet 0 wakeup		    */
#define UIC_EWU1	0x00000400	/* Ethernet 1 wakeup		    */
#define UIC_EWU2	0x00000200	/* Ethernet 2 wakeup		    */
#define UIC_EWU3	0x00000100	/* Ethernet 3 wakeup		    */
#define UIC_EIR10	0x00000080	/* External interrupt 10	    */
#define UIC_EIR11	0x00000040	/* External interrupt 11	    */
#define UIC_RSVD2	0x00000020	/* Reserved			    */
#define UIC_PLB4XAHB	0x00000010	/* PLB4XAHB / AHBARB error	    */
#define UIC_OTG		0x00000008	/* USB2.0 OTG			    */
#define UIC_EHCI	0x00000004	/* USB2.0 Host EHCI		    */
#define UIC_OHCI	0x00000002	/* USB2.0 Host OHCI		    */
#define UIC_OHCISMI	0x00000001	/* USB2.0 Host OHCI SMI		    */

#elif defined(CONFIG_440EPX) || defined(CONFIG_440GRX) /* UIC2 */

#define UIC_EIR5    0x80000000  /* External interrupt 5                 */
#define UIC_EIR6    0x40000000  /* External interrupt 6                 */
#define UIC_OPB     0x20000000  /* OPB to PLB bridge interrupt stat     */
#define UIC_EIR2    0x10000000  /* External interrupt 2                 */
#define UIC_EIR3    0x08000000  /* External interrupt 3                 */
#define UIC_DDR2    0x04000000  /* DDR2 sdram                           */
#define UIC_MCTX0   0x02000000  /* MAl intp coalescence TX0             */
#define UIC_MCTX1   0x01000000  /* MAl intp coalescence TX1             */
#define UIC_MCTR0   0x00800000  /* MAl intp coalescence TR0             */
#define UIC_MCTR1   0x00400000  /* MAl intp coalescence TR1             */

#endif	/* CONFIG_440GX */

/*---------------------------------------------------------------------------+
|  Universal interrupt controller Base 0 interrupts (UICB0)
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440GX)
#define UICB0_UIC0CI	0x80000000	/* UIC0 Critical Interrupt	    */
#define UICB0_UIC0NCI	0x40000000	/* UIC0 Noncritical Interrupt	    */
#define UICB0_UIC1CI	0x20000000	/* UIC1 Critical Interrupt	    */
#define UICB0_UIC1NCI	0x10000000	/* UIC1 Noncritical Interrupt	    */
#define UICB0_UIC2CI	0x08000000	/* UIC2 Critical Interrupt	    */
#define UICB0_UIC2NCI	0x04000000	/* UIC2 Noncritical Interrupt	    */

#define UICB0_ALL	(UICB0_UIC0CI | UICB0_UIC0NCI | UICB0_UIC1CI | \
			 UICB0_UIC1NCI | UICB0_UIC2CI | UICB0_UIC2NCI)

#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)

#define UICB0_UIC1NCI	0x00000002	/* UIC1 Noncritical Interrupt	    */
#define UICB0_UIC1CI	0x00000001	/* UIC1 Critical Interrupt	    */
#define UICB0_UIC2NCI	0x00200000	/* UIC2 Noncritical Interrupt	    */
#define UICB0_UIC2CI	0x00100000	/* UIC2 Critical Interrupt	    */
#define UICB0_UIC3NCI	0x00008000	/* UIC3 Noncritical Interrupt	    */
#define UICB0_UIC3CI	0x00004000	/* UIC3 Critical Interrupt	    */

#define UICB0_ALL	(UICB0_UIC1CI | UICB0_UIC1NCI | UICB0_UIC2CI | \
			 UICB0_UIC2NCI | UICB0_UIC3CI | UICB0_UIC3NCI)

#elif defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

#define UICB0_UIC1CI	0x00000001	/* UIC1 Critical Interrupt	    */
#define UICB0_UIC1NCI	0x00000002	/* UIC1 Noncritical Interrupt	    */
#define UICB0_UIC2CI	0x00000004	/* UIC2 Critical Interrupt	    */
#define UICB0_UIC2NCI	0x00000008	/* UIC2 Noncritical Interrupt	    */

#define UICB0_ALL	(UICB0_UIC1CI | UICB0_UIC1NCI | \
			 UICB0_UIC1CI | UICB0_UIC2NCI)

#elif defined(CONFIG_440GP) || defined(CONFIG_440SP) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR)

#define UICB0_UIC1CI	0x00000001	/* UIC1 Critical Interrupt	    */
#define UICB0_UIC1NCI	0x00000002	/* UIC1 Noncritical Interrupt	    */

#define UICB0_ALL	(UICB0_UIC1CI | UICB0_UIC1NCI)

#endif /* CONFIG_440GX */
/*---------------------------------------------------------------------------+
|  Universal interrupt controller interrupts
+---------------------------------------------------------------------------*/
#if defined(CONFIG_440SPE)
/*#define UICB0_UIC0CI	0x80000000*/	/* UIC0 Critical Interrupt	    */
/*#define UICB0_UIC0NCI	0x40000000*/	/* UIC0 Noncritical Interrupt	    */
#define UICB0_UIC1CI	0x00000002	/* UIC1 Critical Interrupt	    */
#define UICB0_UIC1NCI	0x00000001	/* UIC1 Noncritical Interrupt	    */
#define UICB0_UIC2CI	0x00200000	/* UIC2 Critical Interrupt	    */
#define UICB0_UIC2NCI	0x00100000	/* UIC2 Noncritical Interrupt	    */
#define UICB0_UIC3CI	0x00008000	/* UIC3 Critical Interrupt	    */
#define UICB0_UIC3NCI	0x00004000	/* UIC3 Noncritical Interrupt	    */

#define UICB0_ALL		(UICB0_UIC1CI | UICB0_UIC1NCI | UICB0_UIC2CI | \
						 UICB0_UIC2NCI | UICB0_UIC3CI | UICB0_UIC3NCI)
/*---------------------------------------------------------------------------+
|  Universal interrupt controller 0 interrupts (UIC0)
+---------------------------------------------------------------------------*/
#define UIC_U0		0x80000000	/* UART 0			    */
#define UIC_U1		0x40000000	/* UART 1			    */
#define UIC_IIC0	0x20000000	/* IIC				    */
#define UIC_IIC1	0x10000000	/* IIC				    */
#define UIC_PIM		0x08000000	/* PCI inbound message		    */
#define UIC_PCRW	0x04000000	/* PCI command register write	    */
#define UIC_PPM		0x02000000	/* PCI power management		    */
#define UIC_PVPDA	0x01000000	/* PCIx 0 vpd access		    */
#define UIC_MSI0	0x00800000	/* PCIx MSI level 0		    */
#define UIC_EIR15	0x00400000	/* External intp 15		    */
#define UIC_PEMSI0	0x00080000	/* PCIe MSI level 0		    */
#define UIC_PEMSI1	0x00040000	/* PCIe MSI level 1		    */
#define UIC_PEMSI2	0x00020000	/* PCIe MSI level 2		    */
#define UIC_PEMSI3	0x00010000	/* PCIe MSI level 3		    */
#define UIC_EIR14	0x00002000	/* External interrupt 14	    */
#define UIC_D0CPFF	0x00001000	/* DMA0 cp fifo full		    */
#define UIC_D0CSNS	0x00000800	/* DMA0 cs fifo needs service	    */
#define UIC_D1CPFF	0x00000400	/* DMA1 cp fifo full		    */
#define UIC_D1CSNS	0x00000200	/* DMA1 cs fifo needs service	    */
#define UIC_I2OID	0x00000100	/* I2O inbound door bell	    */
#define UIC_I2OLNE	0x00000080	/* I2O Inbound Post List FIFO Not Empty */
#define UIC_I20R0LL	0x00000040	/* I2O Region 0 Low Latency PLB Write */
#define UIC_I2OR1LL	0x00000020	/* I2O Region 1 Low Latency PLB Write */
#define UIC_I20R0HB	0x00000010	/* I2O Region 0 High Bandwidth PLB Write */
#define UIC_I2OR1HB	0x00000008	/* I2O Region 1 High Bandwidth PLB Write */
#define UIC_CPTCNT	0x00000004	/* GPT Count Timer		    */
/*---------------------------------------------------------------------------+
|  Universal interrupt controller 1 interrupts (UIC1)
+---------------------------------------------------------------------------*/
#define UIC_EIR13	0x80000000	/* externei intp 13		    */
#define UIC_MS		0x40000000	/* MAL SERR			    */
#define UIC_MTDE	0x20000000	/* MAL TXDE			    */
#define UIC_MRDE	0x10000000	/* MAL RXDE			    */
#define UIC_DEUE	0x08000000	/* DDR SDRAM ECC correct/uncorrectable error */
#define UIC_EBCO	0x04000000	/* EBCO interrupt status	    */
#define UIC_MTE		0x02000000	/* MAL TXEOB			    */
#define UIC_MRE		0x01000000	/* MAL RXEOB			    */
#define UIC_MSI1	0x00800000	/* PCI MSI level 1		    */
#define UIC_MSI2	0x00400000	/* PCI MSI level 2		    */
#define UIC_MSI3	0x00200000	/* PCI MSI level 3		    */
#define UIC_L2C		0x00100000	/* L2 cache			    */
#define UIC_CT0		0x00080000	/* GPT compare timer 0		    */
#define UIC_CT1		0x00040000	/* GPT compare timer 1		    */
#define UIC_CT2		0x00020000	/* GPT compare timer 2		    */
#define UIC_CT3		0x00010000	/* GPT compare timer 3		    */
#define UIC_CT4		0x00008000	/* GPT compare timer 4		    */
#define UIC_EIR12	0x00004000	/* External interrupt 12	    */
#define UIC_EIR11	0x00002000	/* External interrupt 11	    */
#define UIC_EIR10	0x00001000	/* External interrupt 10	    */
#define UIC_EIR9	0x00000800	/* External interrupt 9		    */
#define UIC_EIR8	0x00000400	/* External interrupt 8		    */
#define UIC_DMAE	0x00000200	/* dma error			    */
#define UIC_I2OE	0x00000100	/* i2o error			    */
#define UIC_SRE		0x00000080	/* Serial ROM error		    */
#define UIC_PCIXAE	0x00000040	/* Pcix0 async error		    */
#define UIC_EIR7	0x00000020	/* External interrupt 7		    */
#define UIC_EIR6	0x00000010	/* External interrupt 6		    */
#define UIC_ETH0	0x00000008	/* Ethernet 0			    */
#define UIC_EWU0	0x00000004	/* Ethernet 0 wakeup		    */
#define UIC_ETH1	0x00000002	/* reserved			    */
#define UIC_XOR		0x00000001	/* xor				    */

/*---------------------------------------------------------------------------+
|  Universal interrupt controller 2 interrupts (UIC2)
+---------------------------------------------------------------------------*/
#define UIC_PEOAL	0x80000000	/* PE0  AL			    */
#define UIC_PEOVA	0x40000000	/* PE0  VPD access		    */
#define UIC_PEOHRR	0x20000000	/* PE0 Host reset request rising    */
#define UIC_PE0HRF	0x10000000	/* PE0 Host reset request falling   */
#define UIC_PE0TCR	0x08000000	/* PE0 TCR			    */
#define UIC_PE0BVCO	0x04000000	/* PE0 Busmaster VCO		    */
#define UIC_PE0DCRE	0x02000000	/* PE0 DCR error		    */
#define UIC_PE1AL	0x00800000	/* PE1  AL			    */
#define UIC_PE1VA	0x00400000	/* PE1  VPD access		    */
#define UIC_PE1HRR	0x00200000	/* PE1 Host reset request rising    */
#define UIC_PE1HRF	0x00100000	/* PE1 Host reset request falling   */
#define UIC_PE1TCR	0x00080000	/* PE1 TCR			    */
#define UIC_PE1BVCO	0x00040000	/* PE1 Busmaster VCO		    */
#define UIC_PE1DCRE	0x00020000	/* PE1 DCR error		    */
#define UIC_PE2AL	0x00008000	/* PE2  AL			    */
#define UIC_PE2VA	0x00004000	/* PE2  VPD access		    */
#define UIC_PE2HRR	0x00002000	/* PE2 Host reset request rising    */
#define UIC_PE2HRF	0x00001000	/* PE2 Host reset request falling   */
#define UIC_PE2TCR	0x00000800	/* PE2 TCR			    */
#define UIC_PE2BVCO	0x00000400	/* PE2 Busmaster VCO		    */
#define UIC_PE2DCRE	0x00000200	/* PE2 DCR error		    */
#define UIC_EIR5	0x00000080	/* External interrupt 5		    */
#define UIC_EIR4	0x00000040	/* External interrupt 4		    */
#define UIC_EIR3	0x00000020	/* External interrupt 3		    */
#define UIC_EIR2	0x00000010	/* External interrupt 2		    */
#define UIC_EIR1	0x00000008	/* External interrupt 1		    */
#define UIC_EIR0	0x00000004	/* External interrupt 0		    */
#endif /* CONFIG_440SPE */

/*-----------------------------------------------------------------------------+
|  External Bus Controller Bit Settings
+-----------------------------------------------------------------------------*/
#define EBC_CFGADDR_MASK		0x0000003F

#define EBC_BXCR_BAS_ENCODE(n)	((((unsigned long)(n))&0xFFF00000)<<0)
#define EBC_BXCR_BS_MASK		0x000E0000
#define EBC_BXCR_BS_1MB			0x00000000
#define EBC_BXCR_BS_2MB			0x00020000
#define EBC_BXCR_BS_4MB			0x00040000
#define EBC_BXCR_BS_8MB			0x00060000
#define EBC_BXCR_BS_16MB		0x00080000
#define EBC_BXCR_BS_32MB		0x000A0000
#define EBC_BXCR_BS_64MB		0x000C0000
#define EBC_BXCR_BS_128MB		0x000E0000
#define EBC_BXCR_BU_MASK		0x00018000
#define EBC_BXCR_BU_R			0x00008000
#define EBC_BXCR_BU_W			0x00010000
#define EBC_BXCR_BU_RW			0x00018000
#define EBC_BXCR_BW_MASK		0x00006000
#define EBC_BXCR_BW_8BIT		0x00000000
#define EBC_BXCR_BW_16BIT		0x00002000
#define EBC_BXCR_BW_32BIT		0x00006000
#define EBC_BXAP_BME_ENABLED		0x80000000
#define EBC_BXAP_BME_DISABLED		0x00000000
#define EBC_BXAP_TWT_ENCODE(n)		((((unsigned long)(n))&0xFF)<<23)
#define EBC_BXAP_BCE_DISABLE		0x00000000
#define EBC_BXAP_BCE_ENABLE		0x00400000
#define EBC_BXAP_BCT_MASK		0x00300000
#define EBC_BXAP_BCT_2TRANS		0x00000000
#define EBC_BXAP_BCT_4TRANS		0x00100000
#define EBC_BXAP_BCT_8TRANS		0x00200000
#define EBC_BXAP_BCT_16TRANS		0x00300000
#define EBC_BXAP_CSN_ENCODE(n)		((((unsigned long)(n))&0x3)<<18)
#define EBC_BXAP_OEN_ENCODE(n)		((((unsigned long)(n))&0x3)<<16)
#define EBC_BXAP_WBN_ENCODE(n)		((((unsigned long)(n))&0x3)<<14)
#define EBC_BXAP_WBF_ENCODE(n)		((((unsigned long)(n))&0x3)<<12)
#define EBC_BXAP_TH_ENCODE(n)		((((unsigned long)(n))&0x7)<<9)
#define EBC_BXAP_RE_ENABLED		0x00000100
#define EBC_BXAP_RE_DISABLED		0x00000000
#define EBC_BXAP_SOR_DELAYED		0x00000000
#define EBC_BXAP_SOR_NONDELAYED		0x00000080
#define EBC_BXAP_BEM_WRITEONLY		0x00000000
#define EBC_BXAP_BEM_RW			0x00000040
#define EBC_BXAP_PEN_DISABLED		0x00000000

#define EBC_CFG_LE_MASK			0x80000000
#define EBC_CFG_LE_UNLOCK		0x00000000
#define EBC_CFG_LE_LOCK			0x80000000
#define EBC_CFG_PTD_MASK		0x40000000
#define EBC_CFG_PTD_ENABLE		0x00000000
#define EBC_CFG_PTD_DISABLE		0x40000000
#define EBC_CFG_RTC_MASK		0x38000000
#define EBC_CFG_RTC_16PERCLK		0x00000000
#define EBC_CFG_RTC_32PERCLK		0x08000000
#define EBC_CFG_RTC_64PERCLK		0x10000000
#define EBC_CFG_RTC_128PERCLK		0x18000000
#define EBC_CFG_RTC_256PERCLK		0x20000000
#define EBC_CFG_RTC_512PERCLK		0x28000000
#define EBC_CFG_RTC_1024PERCLK		0x30000000
#define EBC_CFG_RTC_2048PERCLK		0x38000000
#define EBC_CFG_ATC_MASK		0x04000000
#define EBC_CFG_ATC_HI			0x00000000
#define EBC_CFG_ATC_PREVIOUS		0x04000000
#define EBC_CFG_DTC_MASK		0x02000000
#define EBC_CFG_DTC_HI			0x00000000
#define EBC_CFG_DTC_PREVIOUS		0x02000000
#define EBC_CFG_CTC_MASK		0x01000000
#define EBC_CFG_CTC_HI			0x00000000
#define EBC_CFG_CTC_PREVIOUS		0x01000000
#define EBC_CFG_OEO_MASK		0x00800000
#define EBC_CFG_OEO_HI			0x00000000
#define EBC_CFG_OEO_PREVIOUS		0x00800000
#define EBC_CFG_EMC_MASK		0x00400000
#define EBC_CFG_EMC_NONDEFAULT		0x00000000
#define EBC_CFG_EMC_DEFAULT		0x00400000
#define EBC_CFG_PME_MASK		0x00200000
#define EBC_CFG_PME_DISABLE		0x00000000
#define EBC_CFG_PME_ENABLE		0x00200000
#define EBC_CFG_PMT_MASK		0x001F0000
#define EBC_CFG_PMT_ENCODE(n)		((((unsigned long)(n))&0x1F)<<12)
#define EBC_CFG_PR_MASK			0x0000C000
#define EBC_CFG_PR_16			0x00000000
#define EBC_CFG_PR_32			0x00004000
#define EBC_CFG_PR_64			0x00008000
#define EBC_CFG_PR_128			0x0000C000

/*-----------------------------------------------------------------------------+
|  SDR0 Bit Settings
+-----------------------------------------------------------------------------*/
#if defined(CONFIG_440SP)
#define SDR0_SRST			0x0200

#define SDR0_DDR0			0x00E1
#define SDR0_DDR0_DPLLRST		0x80000000
#define SDR0_DDR0_DDRM_MASK		0x60000000
#define SDR0_DDR0_DDRM_DDR1		0x20000000
#define SDR0_DDR0_DDRM_DDR2		0x40000000
#define SDR0_DDR0_DDRM_ENCODE(n)	((((unsigned long)(n))&0x03)<<29)
#define SDR0_DDR0_DDRM_DECODE(n)	((((unsigned long)(n))>>29)&0x03)
#define SDR0_DDR0_TUNE_ENCODE(n)	((((unsigned long)(n))&0x2FF)<<0)
#define SDR0_DDR0_TUNE_DECODE(n)	((((unsigned long)(n))>>0)&0x2FF)
#endif

#if defined(CONFIG_440SPE)
#define SDR0_CP440			0x0180
#define SDR0_CP440_ERPN_MASK		0x30000000
#define SDR0_CP440_ERPN_MASK_HI		0x3000
#define SDR0_CP440_ERPN_MASK_LO		0x0000
#define SDR0_CP440_ERPN_EBC		0x10000000
#define SDR0_CP440_ERPN_EBC_HI		0x1000
#define SDR0_CP440_ERPN_EBC_LO		0x0000
#define SDR0_CP440_ERPN_PCI		0x20000000
#define SDR0_CP440_ERPN_PCI_HI		0x2000
#define SDR0_CP440_ERPN_PCI_LO		0x0000
#define SDR0_CP440_ERPN_ENCODE(n)	((((unsigned long)(n))&0x03)<<28)
#define SDR0_CP440_ERPN_DECODE(n)	((((unsigned long)(n))>>28)&0x03)
#define SDR0_CP440_NTO1_MASK		0x00000002
#define SDR0_CP440_NTO1_NTOP		0x00000000
#define SDR0_CP440_NTO1_NTO1		0x00000002
#define SDR0_CP440_NTO1_ENCODE(n)	((((unsigned long)(n))&0x01)<<1)
#define SDR0_CP440_NTO1_DECODE(n)	((((unsigned long)(n))>>1)&0x01)

#define SDR0_SDSTP0			0x0020
#define SDR0_SDSTP0_ENG_MASK		0x80000000
#define SDR0_SDSTP0_ENG_PLLDIS		0x00000000
#define SDR0_SDSTP0_ENG_PLLENAB		0x80000000
#define SDR0_SDSTP0_ENG_ENCODE(n)	((((unsigned long)(n))&0x01)<<31)
#define SDR0_SDSTP0_ENG_DECODE(n)	((((unsigned long)(n))>>31)&0x01)
#define SDR0_SDSTP0_SRC_MASK		0x40000000
#define SDR0_SDSTP0_SRC_PLLOUTA		0x00000000
#define SDR0_SDSTP0_SRC_PLLOUTB		0x40000000
#define SDR0_SDSTP0_SRC_ENCODE(n)	((((unsigned long)(n))&0x01)<<30)
#define SDR0_SDSTP0_SRC_DECODE(n)	((((unsigned long)(n))>>30)&0x01)
#define SDR0_SDSTP0_SEL_MASK		0x38000000
#define SDR0_SDSTP0_SEL_PLLOUT		0x00000000
#define SDR0_SDSTP0_SEL_CPU		0x08000000
#define SDR0_SDSTP0_SEL_EBC		0x28000000
#define SDR0_SDSTP0_SEL_ENCODE(n)	((((unsigned long)(n))&0x07)<<27)
#define SDR0_SDSTP0_SEL_DECODE(n)	((((unsigned long)(n))>>27)&0x07)
#define SDR0_SDSTP0_TUNE_MASK		0x07FE0000
#define SDR0_SDSTP0_TUNE_ENCODE(n)	((((unsigned long)(n))&0x3FF)<<17)
#define SDR0_SDSTP0_TUNE_DECODE(n)	((((unsigned long)(n))>>17)&0x3FF)
#define SDR0_SDSTP0_FBDV_MASK		0x0001F000
#define SDR0_SDSTP0_FBDV_ENCODE(n)	((((unsigned long)(n))&0x1F)<<12)
#define SDR0_SDSTP0_FBDV_DECODE(n)	((((((unsigned long)(n))>>12)-1)&0x1F)+1)
#define SDR0_SDSTP0_FWDVA_MASK		0x00000F00
#define SDR0_SDSTP0_FWDVA_ENCODE(n)	((((unsigned long)(n))&0x0F)<<8)
#define SDR0_SDSTP0_FWDVA_DECODE(n)	((((((unsigned long)(n))>>8)-1)&0x0F)+1)
#define SDR0_SDSTP0_FWDVB_MASK		0x000000E0
#define SDR0_SDSTP0_FWDVB_ENCODE(n)	((((unsigned long)(n))&0x07)<<5)
#define SDR0_SDSTP0_FWDVB_DECODE(n)	((((((unsigned long)(n))>>5)-1)&0x07)+1)
#define SDR0_SDSTP0_PRBDV0_MASK		0x0000001C
#define SDR0_SDSTP0_PRBDV0_ENCODE(n)	((((unsigned long)(n))&0x07)<<2)
#define SDR0_SDSTP0_PRBDV0_DECODE(n)	((((((unsigned long)(n))>>2)-1)&0x07)+1)
#define SDR0_SDSTP0_OPBDV0_MASK		0x00000003
#define SDR0_SDSTP0_OPBDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<0)
#define SDR0_SDSTP0_OPBDV0_DECODE(n)	((((((unsigned long)(n))>>0)-1)&0x03)+1)


#define SDR0_SDSTP1			0x0021
#define SDR0_SDSTP1_LFBDV_MASK		0xFC000000
#define SDR0_SDSTP1_LFBDV_ENCODE(n)	((((unsigned long)(n))&0x3F)<<26)
#define SDR0_SDSTP1_LFBDV_DECODE(n)	((((unsigned long)(n))>>26)&0x3F)
#define SDR0_SDSTP1_PERDV0_MASK		0x03000000
#define SDR0_SDSTP1_PERDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<24)
#define SDR0_SDSTP1_PERDV0_DECODE(n)	((((unsigned long)(n))>>24)&0x03)
#define SDR0_SDSTP1_MALDV0_MASK		0x00C00000
#define SDR0_SDSTP1_MALDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<22)
#define SDR0_SDSTP1_MALDV0_DECODE(n)	((((unsigned long)(n))>>22)&0x03)
#define SDR0_SDSTP1_DDR_MODE_MASK	0x00300000
#define SDR0_SDSTP1_DDR1_MODE		0x00100000
#define SDR0_SDSTP1_DDR2_MODE		0x00200000
#define SDR0_SDSTP1_DDR_ENCODE(n)	((((unsigned long)(n))&0x03)<<20)
#define SDR0_SDSTP1_DDR_DECODE(n)	((((unsigned long)(n))>>20)&0x03)
#define SDR0_SDSTP1_ERPN_MASK		0x00080000
#define SDR0_SDSTP1_ERPN_EBC		0x00000000
#define SDR0_SDSTP1_ERPN_PCI		0x00080000
#define SDR0_SDSTP1_PAE_MASK		0x00040000
#define SDR0_SDSTP1_PAE_DISABLE		0x00000000
#define SDR0_SDSTP1_PAE_ENABLE		0x00040000
#define SDR0_SDSTP1_PAE_ENCODE(n)	((((unsigned long)(n))&0x01)<<18)
#define SDR0_SDSTP1_PAE_DECODE(n)	((((unsigned long)(n))>>18)&0x01)
#define SDR0_SDSTP1_PHCE_MASK		0x00020000
#define SDR0_SDSTP1_PHCE_DISABLE	0x00000000
#define SDR0_SDSTP1_PHCE_ENABLE		0x00020000
#define SDR0_SDSTP1_PHCE_ENCODE(n)	((((unsigned long)(n))&0x01)<<17)
#define SDR0_SDSTP1_PHCE_DECODE(n)	((((unsigned long)(n))>>17)&0x01)
#define SDR0_SDSTP1_PISE_MASK		0x00010000
#define SDR0_SDSTP1_PISE_DISABLE	0x00000000
#define SDR0_SDSTP1_PISE_ENABLE		0x00001000
#define SDR0_SDSTP1_PISE_ENCODE(n)	((((unsigned long)(n))&0x01)<<16)
#define SDR0_SDSTP1_PISE_DECODE(n)	((((unsigned long)(n))>>16)&0x01)
#define SDR0_SDSTP1_PCWE_MASK		0x00008000
#define SDR0_SDSTP1_PCWE_DISABLE	0x00000000
#define SDR0_SDSTP1_PCWE_ENABLE		0x00008000
#define SDR0_SDSTP1_PCWE_ENCODE(n)	((((unsigned long)(n))&0x01)<<15)
#define SDR0_SDSTP1_PCWE_DECODE(n)	((((unsigned long)(n))>>15)&0x01)
#define SDR0_SDSTP1_PPIM_MASK		0x00007800
#define SDR0_SDSTP1_PPIM_ENCODE(n)	((((unsigned long)(n))&0x0F)<<11)
#define SDR0_SDSTP1_PPIM_DECODE(n)	((((unsigned long)(n))>>11)&0x0F)
#define SDR0_SDSTP1_PR64E_MASK		0x00000400
#define SDR0_SDSTP1_PR64E_DISABLE	0x00000000
#define SDR0_SDSTP1_PR64E_ENABLE	0x00000400
#define SDR0_SDSTP1_PR64E_ENCODE(n)	((((unsigned long)(n))&0x01)<<10)
#define SDR0_SDSTP1_PR64E_DECODE(n)	((((unsigned long)(n))>>10)&0x01)
#define SDR0_SDSTP1_PXFS_MASK		0x00000300
#define SDR0_SDSTP1_PXFS_100_133	0x00000000
#define SDR0_SDSTP1_PXFS_66_100		0x00000100
#define SDR0_SDSTP1_PXFS_50_66		0x00000200
#define SDR0_SDSTP1_PXFS_0_50		0x00000300
#define SDR0_SDSTP1_PXFS_ENCODE(n)	((((unsigned long)(n))&0x03)<<8)
#define SDR0_SDSTP1_PXFS_DECODE(n)	((((unsigned long)(n))>>8)&0x03)
#define SDR0_SDSTP1_EBCW_MASK		0x00000080 /* SOP */
#define SDR0_SDSTP1_EBCW_8_BITS		0x00000000 /* SOP */
#define SDR0_SDSTP1_EBCW_16_BITS	0x00000080 /* SOP */
#define SDR0_SDSTP1_DBGEN_MASK		0x00000030 /* $218C */
#define SDR0_SDSTP1_DBGEN_FUNC		0x00000000
#define SDR0_SDSTP1_DBGEN_TRACE		0x00000010
#define SDR0_SDSTP1_DBGEN_ENCODE(n)	((((unsigned long)(n))&0x03)<<4) /* $218C */
#define SDR0_SDSTP1_DBGEN_DECODE(n)	((((unsigned long)(n))>>4)&0x03) /* $218C */
#define SDR0_SDSTP1_ETH_MASK		0x00000004
#define SDR0_SDSTP1_ETH_10_100		0x00000000
#define SDR0_SDSTP1_ETH_GIGA		0x00000004
#define SDR0_SDSTP1_ETH_ENCODE(n)	((((unsigned long)(n))&0x01)<<2)
#define SDR0_SDSTP1_ETH_DECODE(n)	((((unsigned long)(n))>>2)&0x01)
#define SDR0_SDSTP1_NTO1_MASK		0x00000001
#define SDR0_SDSTP1_NTO1_DISABLE	0x00000000
#define SDR0_SDSTP1_NTO1_ENABLE		0x00000001
#define SDR0_SDSTP1_NTO1_ENCODE(n)	((((unsigned long)(n))&0x01)<<0)
#define SDR0_SDSTP1_NTO1_DECODE(n)	((((unsigned long)(n))>>0)&0x01)

#define SDR0_SDSTP2			0x0022
#define SDR0_SDSTP2_P1AE_MASK		0x80000000
#define SDR0_SDSTP2_P1AE_DISABLE	0x00000000
#define SDR0_SDSTP2_P1AE_ENABLE		0x80000000
#define SDR0_SDSTP2_P1AE_ENCODE(n)	((((unsigned long)(n))&0x01)<<31)
#define SDR0_SDSTP2_P1AE_DECODE(n)	((((unsigned long)(n))>>31)&0x01)
#define SDR0_SDSTP2_P1HCE_MASK		0x40000000
#define SDR0_SDSTP2_P1HCE_DISABLE	0x00000000
#define SDR0_SDSTP2_P1HCE_ENABLE	0x40000000
#define SDR0_SDSTP2_P1HCE_ENCODE(n)	((((unsigned long)(n))&0x01)<<30)
#define SDR0_SDSTP2_P1HCE_DECODE(n)	((((unsigned long)(n))>>30)&0x01)
#define SDR0_SDSTP2_P1ISE_MASK		0x20000000
#define SDR0_SDSTP2_P1ISE_DISABLE	0x00000000
#define SDR0_SDSTP2_P1ISE_ENABLE	0x20000000
#define SDR0_SDSTP2_P1ISE_ENCODE(n)	((((unsigned long)(n))&0x01)<<29)
#define SDR0_SDSTP2_P1ISE_DECODE(n)	((((unsigned long)(n))>>29)&0x01)
#define SDR0_SDSTP2_P1CWE_MASK		0x10000000
#define SDR0_SDSTP2_P1CWE_DISABLE	0x00000000
#define SDR0_SDSTP2_P1CWE_ENABLE	0x10000000
#define SDR0_SDSTP2_P1CWE_ENCODE(n)	((((unsigned long)(n))&0x01)<<28)
#define SDR0_SDSTP2_P1CWE_DECODE(n)	((((unsigned long)(n))>>28)&0x01)
#define SDR0_SDSTP2_P1PIM_MASK		0x0F000000
#define SDR0_SDSTP2_P1PIM_ENCODE(n)	((((unsigned long)(n))&0x0F)<<24)
#define SDR0_SDSTP2_P1PIM_DECODE(n)	((((unsigned long)(n))>>24)&0x0F)
#define SDR0_SDSTP2_P1R64E_MASK		0x00800000
#define SDR0_SDSTP2_P1R64E_DISABLE	0x00000000
#define SDR0_SDSTP2_P1R64E_ENABLE	0x00800000
#define SDR0_SDSTP2_P1R64E_ENCODE(n)	((((unsigned long)(n))&0x01)<<23)
#define SDR0_SDSTP2_P1R64E_DECODE(n)	((((unsigned long)(n))>>23)&0x01)
#define SDR0_SDSTP2_P1XFS_MASK		0x00600000
#define SDR0_SDSTP2_P1XFS_100_133	0x00000000
#define SDR0_SDSTP2_P1XFS_66_100	0x00200000
#define SDR0_SDSTP2_P1XFS_50_66		0x00400000
#define SDR0_SDSTP2_P1XFS_0_50		0x00600000
#define SDR0_SDSTP2_P1XFS_ENCODE(n)	((((unsigned long)(n))&0x03)<<21)
#define SDR0_SDSTP2_P1XFS_DECODE(n)	((((unsigned long)(n))>>21)&0x03)
#define SDR0_SDSTP2_P2AE_MASK		0x00040000
#define SDR0_SDSTP2_P2AE_DISABLE	0x00000000
#define SDR0_SDSTP2_P2AE_ENABLE		0x00040000
#define SDR0_SDSTP2_P2AE_ENCODE(n)	((((unsigned long)(n))&0x01)<<18)
#define SDR0_SDSTP2_P2AE_DECODE(n)	((((unsigned long)(n))>>18)&0x01)
#define SDR0_SDSTP2_P2HCE_MASK		0x00020000
#define SDR0_SDSTP2_P2HCE_DISABLE	0x00000000
#define SDR0_SDSTP2_P2HCE_ENABLE	0x00020000
#define SDR0_SDSTP2_P2HCE_ENCODE(n)	((((unsigned long)(n))&0x01)<<17)
#define SDR0_SDSTP2_P2HCE_DECODE(n)	((((unsigned long)(n))>>17)&0x01)
#define SDR0_SDSTP2_P2ISE_MASK		0x00010000
#define SDR0_SDSTP2_P2ISE_DISABLE	0x00000000
#define SDR0_SDSTP2_P2ISE_ENABLE	0x00010000
#define SDR0_SDSTP2_P2ISE_ENCODE(n)	((((unsigned long)(n))&0x01)<<16)
#define SDR0_SDSTP2_P2ISE_DECODE(n)	((((unsigned long)(n))>>16)&0x01)
#define SDR0_SDSTP2_P2CWE_MASK		0x00008000
#define SDR0_SDSTP2_P2CWE_DISABLE	0x00000000
#define SDR0_SDSTP2_P2CWE_ENABLE	0x00008000
#define SDR0_SDSTP2_P2CWE_ENCODE(n)	((((unsigned long)(n))&0x01)<<15)
#define SDR0_SDSTP2_P2CWE_DECODE(n)	((((unsigned long)(n))>>15)&0x01)
#define SDR0_SDSTP2_P2PIM_MASK		0x00007800
#define SDR0_SDSTP2_P2PIM_ENCODE(n)	((((unsigned long)(n))&0x0F)<<11)
#define SDR0_SDSTP2_P2PIM_DECODE(n)	((((unsigned long)(n))>>11)&0x0F)
#define SDR0_SDSTP2_P2XFS_MASK		0x00000300
#define SDR0_SDSTP2_P2XFS_100_133	0x00000000
#define SDR0_SDSTP2_P2XFS_66_100	0x00000100
#define SDR0_SDSTP2_P2XFS_50_66		0x00000200
#define SDR0_SDSTP2_P2XFS_0_50		0x00000100
#define SDR0_SDSTP2_P2XFS_ENCODE(n)	((((unsigned long)(n))&0x03)<<8)
#define SDR0_SDSTP2_P2XFS_DECODE(n)	((((unsigned long)(n))>>8)&0x03)

#define SDR0_SDSTP3			0x0023

#define SDR0_PINSTP			0x0040
#define SDR0_PINSTP_BOOTSTRAP_MASK	0xC0000000  /* Strap Bits */
#define SDR0_PINSTP_BOOTSTRAP_SETTINGS0	0x00000000  /* Default strap settings 0 (EBC boot) */
#define SDR0_PINSTP_BOOTSTRAP_SETTINGS1	0x40000000  /* Default strap settings 1 (PCI boot) */
#define SDR0_PINSTP_BOOTSTRAP_IIC_54_EN	0x80000000  /* Serial Device Enabled - Addr = 0x54 */
#define SDR0_PINSTP_BOOTSTRAP_IIC_50_EN	0xC0000000  /* Serial Device Enabled - Addr = 0x50 */
#define SDR0_SDCS			0x0060
#define SDR0_ECID0			0x0080
#define SDR0_ECID1			0x0081
#define SDR0_ECID2			0x0082
#define SDR0_JTAG			0x00C0

#define SDR0_DDR0			0x00E1
#define SDR0_DDR0_DPLLRST		0x80000000
#define SDR0_DDR0_DDRM_MASK		0x60000000
#define SDR0_DDR0_DDRM_DDR1		0x20000000
#define SDR0_DDR0_DDRM_DDR2		0x40000000
#define SDR0_DDR0_DDRM_ENCODE(n)	((((unsigned long)(n))&0x03)<<29)
#define SDR0_DDR0_DDRM_DECODE(n)	((((unsigned long)(n))>>29)&0x03)
#define SDR0_DDR0_TUNE_ENCODE(n)	((((unsigned long)(n))&0x2FF)<<0)
#define SDR0_DDR0_TUNE_DECODE(n)	((((unsigned long)(n))>>0)&0x2FF)

#define SDR0_UART0			0x0120
#define SDR0_UART1			0x0121
#define SDR0_UART2			0x0122
#define SDR0_UARTX_UXICS_MASK		0xF0000000
#define SDR0_UARTX_UXICS_PLB		0x20000000
#define SDR0_UARTX_UXEC_MASK		0x00800000
#define SDR0_UARTX_UXEC_INT		0x00000000
#define SDR0_UARTX_UXEC_EXT		0x00800000
#define SDR0_UARTX_UXDIV_MASK		0x000000FF
#define SDR0_UARTX_UXDIV_ENCODE(n)	((((unsigned long)(n))&0xFF)<<0)
#define SDR0_UARTX_UXDIV_DECODE(n)	((((((unsigned long)(n))>>0)-1)&0xFF)+1)

#define SDR0_CP440			0x0180
#define SDR0_CP440_ERPN_MASK		0x30000000
#define SDR0_CP440_ERPN_MASK_HI		0x3000
#define SDR0_CP440_ERPN_MASK_LO		0x0000
#define SDR0_CP440_ERPN_EBC		0x10000000
#define SDR0_CP440_ERPN_EBC_HI		0x1000
#define SDR0_CP440_ERPN_EBC_LO		0x0000
#define SDR0_CP440_ERPN_PCI		0x20000000
#define SDR0_CP440_ERPN_PCI_HI		0x2000
#define SDR0_CP440_ERPN_PCI_LO		0x0000
#define SDR0_CP440_ERPN_ENCODE(n)	((((unsigned long)(n))&0x03)<<28)
#define SDR0_CP440_ERPN_DECODE(n)	((((unsigned long)(n))>>28)&0x03)
#define SDR0_CP440_NTO1_MASK		0x00000002
#define SDR0_CP440_NTO1_NTOP		0x00000000
#define SDR0_CP440_NTO1_NTO1		0x00000002
#define SDR0_CP440_NTO1_ENCODE(n)	((((unsigned long)(n))&0x01)<<1)
#define SDR0_CP440_NTO1_DECODE(n)	((((unsigned long)(n))>>1)&0x01)

#define SDR0_XCR0			0x01C0
#define SDR0_XCR1			0x01C3
#define SDR0_XCR2			0x01C6
#define SDR0_XCRn_PAE_MASK		0x80000000
#define SDR0_XCRn_PAE_DISABLE		0x00000000
#define SDR0_XCRn_PAE_ENABLE		0x80000000
#define SDR0_XCRn_PAE_ENCODE(n)		((((unsigned long)(n))&0x01)<<31)
#define SDR0_XCRn_PAE_DECODE(n)		((((unsigned long)(n))>>31)&0x01)
#define SDR0_XCRn_PHCE_MASK		0x40000000
#define SDR0_XCRn_PHCE_DISABLE		0x00000000
#define SDR0_XCRn_PHCE_ENABLE		0x40000000
#define SDR0_XCRn_PHCE_ENCODE(n)	((((unsigned long)(n))&0x01)<<30)
#define SDR0_XCRn_PHCE_DECODE(n)	((((unsigned long)(n))>>30)&0x01)
#define SDR0_XCRn_PISE_MASK		0x20000000
#define SDR0_XCRn_PISE_DISABLE		0x00000000
#define SDR0_XCRn_PISE_ENABLE		0x20000000
#define SDR0_XCRn_PISE_ENCODE(n)	((((unsigned long)(n))&0x01)<<29)
#define SDR0_XCRn_PISE_DECODE(n)	((((unsigned long)(n))>>29)&0x01)
#define SDR0_XCRn_PCWE_MASK		0x10000000
#define SDR0_XCRn_PCWE_DISABLE		0x00000000
#define SDR0_XCRn_PCWE_ENABLE		0x10000000
#define SDR0_XCRn_PCWE_ENCODE(n)	((((unsigned long)(n))&0x01)<<28)
#define SDR0_XCRn_PCWE_DECODE(n)	((((unsigned long)(n))>>28)&0x01)
#define SDR0_XCRn_PPIM_MASK		0x0F000000
#define SDR0_XCRn_PPIM_ENCODE(n)	((((unsigned long)(n))&0x0F)<<24)
#define SDR0_XCRn_PPIM_DECODE(n)	((((unsigned long)(n))>>24)&0x0F)
#define SDR0_XCRn_PR64E_MASK		0x00800000
#define SDR0_XCRn_PR64E_DISABLE		0x00000000
#define SDR0_XCRn_PR64E_ENABLE		0x00800000
#define SDR0_XCRn_PR64E_ENCODE(n)	((((unsigned long)(n))&0x01)<<23)
#define SDR0_XCRn_PR64E_DECODE(n)	((((unsigned long)(n))>>23)&0x01)
#define SDR0_XCRn_PXFS_MASK		0x00600000
#define SDR0_XCRn_PXFS_100_133		0x00000000
#define SDR0_XCRn_PXFS_66_100		0x00200000
#define SDR0_XCRn_PXFS_50_66		0x00400000
#define SDR0_XCRn_PXFS_0_33		0x00600000
#define SDR0_XCRn_PXFS_ENCODE(n)	((((unsigned long)(n))&0x03)<<21)
#define SDR0_XCRn_PXFS_DECODE(n)	((((unsigned long)(n))>>21)&0x03)

#define SDR0_XPLLC0			0x01C1
#define SDR0_XPLLD0			0x01C2
#define SDR0_XPLLC1			0x01C4
#define SDR0_XPLLD1			0x01C5
#define SDR0_XPLLC2			0x01C7
#define SDR0_XPLLD2			0x01C8
#define SDR0_SRST			0x0200
#define SDR0_SLPIPE			0x0220

#define SDR0_AMP0			0x0240
#define SDR0_AMP0_PRIORITY		0xFFFF0000
#define SDR0_AMP0_ALTERNATE_PRIORITY	0x0000FF00
#define SDR0_AMP0_RESERVED_BITS_MASK	0x000000FF

#define SDR0_AMP1			0x0241
#define SDR0_AMP1_PRIORITY		0xFC000000
#define SDR0_AMP1_ALTERNATE_PRIORITY	0x0000E000
#define SDR0_AMP1_RESERVED_BITS_MASK	0x03FF1FFF

#define SDR0_MIRQ0			0x0260
#define SDR0_MIRQ1			0x0261
#define SDR0_MALTBL			0x0280
#define SDR0_MALRBL			0x02A0
#define SDR0_MALTBS			0x02C0
#define SDR0_MALRBS			0x02E0

/* Reserved for Customer Use */
#define SDR0_CUST0			0x4000
#define SDR0_CUST0_AUTONEG_MASK		0x8000000
#define SDR0_CUST0_NO_AUTONEG		0x0000000
#define SDR0_CUST0_AUTONEG		0x8000000
#define SDR0_CUST0_ETH_FORCE_MASK	0x6000000
#define SDR0_CUST0_ETH_FORCE_10MHZ	0x0000000
#define SDR0_CUST0_ETH_FORCE_100MHZ	0x2000000
#define SDR0_CUST0_ETH_FORCE_1000MHZ	0x4000000
#define SDR0_CUST0_ETH_DUPLEX_MASK	0x1000000
#define SDR0_CUST0_ETH_HALF_DUPLEX	0x0000000
#define SDR0_CUST0_ETH_FULL_DUPLEX	0x1000000

#define SDR0_SDSTP4			0x4001
#define SDR0_CUST1			0x4002
#define SDR0_SDSTP5			0x4003
#define SDR0_CUST2			0x4004
#define SDR0_SDSTP6			0x4005
#define SDR0_CUST3			0x4006
#define SDR0_SDSTP7			0x4007

#define SDR0_PFC0			0x4100
#define SDR0_PFC0_GPIO_0		0x80000000
#define SDR0_PFC0_PCIX0REQ2_N		0x00000000
#define SDR0_PFC0_GPIO_1		0x40000000
#define SDR0_PFC0_PCIX0REQ3_N		0x00000000
#define SDR0_PFC0_GPIO_2		0x20000000
#define SDR0_PFC0_PCIX0GNT2_N		0x00000000
#define SDR0_PFC0_GPIO_3		0x10000000
#define SDR0_PFC0_PCIX0GNT3_N		0x00000000
#define SDR0_PFC0_GPIO_4		0x08000000
#define SDR0_PFC0_PCIX1REQ2_N		0x00000000
#define SDR0_PFC0_GPIO_5		0x04000000
#define SDR0_PFC0_PCIX1REQ3_N		0x00000000
#define SDR0_PFC0_GPIO_6		0x02000000
#define SDR0_PFC0_PCIX1GNT2_N		0x00000000
#define SDR0_PFC0_GPIO_7		0x01000000
#define SDR0_PFC0_PCIX1GNT3_N		0x00000000
#define SDR0_PFC0_GPIO_8		0x00800000
#define SDR0_PFC0_PERREADY		0x00000000
#define SDR0_PFC0_GPIO_9		0x00400000
#define SDR0_PFC0_PERCS1_N		0x00000000
#define SDR0_PFC0_GPIO_10		0x00200000
#define SDR0_PFC0_PERCS2_N		0x00000000
#define SDR0_PFC0_GPIO_11		0x00100000
#define SDR0_PFC0_IRQ0			0x00000000
#define SDR0_PFC0_GPIO_12		0x00080000
#define SDR0_PFC0_IRQ1			0x00000000
#define SDR0_PFC0_GPIO_13		0x00040000
#define SDR0_PFC0_IRQ2			0x00000000
#define SDR0_PFC0_GPIO_14		0x00020000
#define SDR0_PFC0_IRQ3			0x00000000
#define SDR0_PFC0_GPIO_15		0x00010000
#define SDR0_PFC0_IRQ4			0x00000000
#define SDR0_PFC0_GPIO_16		0x00008000
#define SDR0_PFC0_IRQ5			0x00000000
#define SDR0_PFC0_GPIO_17		0x00004000
#define SDR0_PFC0_PERBE0_N		0x00000000
#define SDR0_PFC0_GPIO_18		0x00002000
#define SDR0_PFC0_PCI0GNT0_N		0x00000000
#define SDR0_PFC0_GPIO_19		0x00001000
#define SDR0_PFC0_PCI0GNT1_N		0x00000000
#define SDR0_PFC0_GPIO_20		0x00000800
#define SDR0_PFC0_PCI0REQ0_N		0x00000000
#define SDR0_PFC0_GPIO_21		0x00000400
#define SDR0_PFC0_PCI0REQ1_N		0x00000000
#define SDR0_PFC0_GPIO_22		0x00000200
#define SDR0_PFC0_PCI1GNT0_N		0x00000000
#define SDR0_PFC0_GPIO_23		0x00000100
#define SDR0_PFC0_PCI1GNT1_N		0x00000000
#define SDR0_PFC0_GPIO_24		0x00000080
#define SDR0_PFC0_PCI1REQ0_N		0x00000000
#define SDR0_PFC0_GPIO_25		0x00000040
#define SDR0_PFC0_PCI1REQ1_N		0x00000000
#define SDR0_PFC0_GPIO_26		0x00000020
#define SDR0_PFC0_PCI2GNT0_N		0x00000000
#define SDR0_PFC0_GPIO_27		0x00000010
#define SDR0_PFC0_PCI2GNT1_N		0x00000000
#define SDR0_PFC0_GPIO_28		0x00000008
#define SDR0_PFC0_PCI2REQ0_N		0x00000000
#define SDR0_PFC0_GPIO_29		0x00000004
#define SDR0_PFC0_PCI2REQ1_N		0x00000000
#define SDR0_PFC0_GPIO_30		0x00000002
#define SDR0_PFC0_UART1RX		0x00000000
#define SDR0_PFC0_GPIO_31		0x00000001
#define SDR0_PFC0_UART1TX		0x00000000

#define SDR0_PFC1			0x4101
#define SDR0_PFC1_UART1_CTS_RTS_MASK	0x02000000
#define SDR0_PFC1_UART1_DSR_DTR		0x00000000
#define SDR0_PFC1_UART1_CTS_RTS		0x02000000
#define SDR0_PFC1_UART2_IN_SERVICE_MASK	0x01000000
#define SDR0_PFC1_UART2_NOT_IN_SERVICE	0x00000000
#define SDR0_PFC1_UART2_IN_SERVICE	0x01000000
#define SDR0_PFC1_ETH_GIGA_MASK		0x00200000
#define SDR0_PFC1_ETH_10_100		0x00000000
#define SDR0_PFC1_ETH_GIGA		0x00200000
#define SDR0_PFC1_ETH_GIGA_ENCODE(n)	((((unsigned long)(n))&0x1)<<21)
#define SDR0_PFC1_ETH_GIGA_DECODE(n)	((((unsigned long)(n))>>21)&0x01)
#define SDR0_PFC1_CPU_TRACE_MASK	0x00180000   /* $218C */
#define SDR0_PFC1_CPU_NO_TRACE		0x00000000
#define SDR0_PFC1_CPU_TRACE		0x00080000
#define SDR0_PFC1_CPU_TRACE_ENCODE(n)	((((unsigned long)(n))&0x3)<<19)     /* $218C */
#define SDR0_PFC1_CPU_TRACE_DECODE(n)	((((unsigned long)(n))>>19)&0x03)    /* $218C */

#define SDR0_MFR			0x4300
#endif	/* CONFIG_440SPE	*/

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
/* Pin Function Control Register 0 (SDR0_PFC0) */
#define SDR0_PFC0		0x4100
#define SDR0_PFC0_DBG		0x00008000	/* debug enable */
#define SDR0_PFC0_G49E		0x00004000	/* GPIO 49 enable */
#define SDR0_PFC0_G50E		0x00002000	/* GPIO 50 enable */
#define SDR0_PFC0_G51E		0x00001000	/* GPIO 51 enable */
#define SDR0_PFC0_G52E		0x00000800	/* GPIO 52 enable */
#define SDR0_PFC0_G53E		0x00000400	/* GPIO 53 enable */
#define SDR0_PFC0_G54E		0x00000200	/* GPIO 54 enable */
#define SDR0_PFC0_G55E		0x00000100	/* GPIO 55 enable */
#define SDR0_PFC0_G56E		0x00000080	/* GPIO 56 enable */
#define SDR0_PFC0_G57E		0x00000040	/* GPIO 57 enable */
#define SDR0_PFC0_G58E		0x00000020	/* GPIO 58 enable */
#define SDR0_PFC0_G59E		0x00000010	/* GPIO 59 enable */
#define SDR0_PFC0_G60E		0x00000008	/* GPIO 60 enable */
#define SDR0_PFC0_G61E		0x00000004	/* GPIO 61 enable */
#define SDR0_PFC0_G62E		0x00000002	/* GPIO 62 enable */
#define SDR0_PFC0_G63E		0x00000001	/* GPIO 63 enable */

/* Pin Function Control Register 1 (SDR0_PFC1) */
#define SDR0_PFC1		0x4101
#define SDR0_PFC1_U1ME_MASK	0x02000000	/* UART1 Mode Enable */
#define SDR0_PFC1_U1ME_DSR_DTR	0x00000000	/* UART1 in DSR/DTR Mode */
#define SDR0_PFC1_U1ME_CTS_RTS	0x02000000	/* UART1 in CTS/RTS Mode */
#define SDR0_PFC1_U0ME_MASK	0x00080000	/* UART0 Mode Enable */
#define SDR0_PFC1_U0ME_DSR_DTR	0x00000000	/* UART0 in DSR/DTR Mode */
#define SDR0_PFC1_U0ME_CTS_RTS	0x00080000	/* UART0 in CTS/RTS Mode */
#define SDR0_PFC1_U0IM_MASK	0x00040000	/* UART0 Interface Mode */
#define SDR0_PFC1_U0IM_8PINS	0x00000000	/* UART0 Interface Mode 8 pins*/
#define SDR0_PFC1_U0IM_4PINS	0x00040000	/* UART0 Interface Mode 4 pins*/
#define SDR0_PFC1_SIS_MASK	0x00020000	/* SCP or IIC1 Selection */
#define SDR0_PFC1_SIS_SCP_SEL	0x00000000	/* SCP Selected */
#define SDR0_PFC1_SIS_IIC1_SEL	0x00020000	/* IIC1 Selected */

/* Ethernet PLL Configuration Register (SDR0_ETH_PLL) */
#define SDR0_ETH_PLL		0x4102
#define SDR0_ETH_PLL_PLLLOCK	 0x80000000	/*Ethernet PLL lock indication*/
#define SDR0_ETH_PLL_REF_CLK_SEL 0x10000000	/* Ethernet reference clock */
#define SDR0_ETH_PLL_BYPASS	 0x08000000	/* bypass mode enable */
#define SDR0_ETH_PLL_STOPCLK	 0x04000000	/* output clock disable */
#define SDR0_ETH_PLL_TUNE_MASK	 0x03FF0000	/* loop stability tuning bits */
#define SDR0_ETH_PLL_TUNE_ENCODE(n)	((((unsigned long)(n))&0x3ff)<<16)
#define SDR0_ETH_PLL_MULTI_MASK	 0x0000FF00	/* frequency multiplication */
#define SDR0_ETH_PLL_MULTI_ENCODE(n)	((((unsigned long)(n))&0xff)<<8)
#define SDR0_ETH_PLL_RANGEB_MASK 0x000000F0	/* PLLOUTB/C frequency */
#define SDR0_ETH_PLL_RANGEB_ENCODE(n)	((((unsigned long)(n))&0x0f)<<4)
#define SDR0_ETH_PLL_RANGEA_MASK 0x0000000F	/* PLLOUTA frequency */
#define SDR0_ETH_PLL_RANGEA_ENCODE(n)	(((unsigned long)(n))&0x0f)

/* Ethernet Configuration Register (SDR0_ETH_CFG) */
#define SDR0_ETH_CFG		0x4103
#define SDR0_ETH_CFG_SGMII3_LPBK	0x00800000	/* SGMII3 port loopback enable */
#define SDR0_ETH_CFG_SGMII2_LPBK	0x00400000	/* SGMII2 port loopback enable */
#define SDR0_ETH_CFG_SGMII1_LPBK	0x00200000	/* SGMII1 port loopback enable */
#define SDR0_ETH_CFG_SGMII0_LPBK	0x00100000	/* SGMII0 port loopback enable */
#define SDR0_ETH_CFG_SGMII_MASK		0x00070000	/* SGMII Mask */
#define SDR0_ETH_CFG_SGMII2_ENABLE	0x00040000	/* SGMII2 port enable */
#define SDR0_ETH_CFG_SGMII1_ENABLE	0x00020000	/* SGMII1 port enable */
#define SDR0_ETH_CFG_SGMII0_ENABLE	0x00010000	/* SGMII0 port enable */
#define SDR0_ETH_CFG_TAHOE1_BYPASS	0x00002000	/* TAHOE1 Bypass selector */
#define SDR0_ETH_CFG_TAHOE0_BYPASS	0x00001000	/* TAHOE0 Bypass selector */
#define SDR0_ETH_CFG_EMAC3_PHY_CLK_SEL	0x00000800	/* EMAC 3 PHY clock selector */
#define SDR0_ETH_CFG_EMAC2_PHY_CLK_SEL	0x00000400	/* EMAC 2 PHY clock selector */
#define SDR0_ETH_CFG_EMAC1_PHY_CLK_SEL	0x00000200	/* EMAC 1 PHY clock selector */
#define SDR0_ETH_CFG_EMAC0_PHY_CLK_SEL	0x00000100	/* EMAC 0 PHY clock selector */
#define SDR0_ETH_CFG_EMAC_2_1_SWAP	0x00000080	/* Swap EMAC2 with EMAC1 */
#define SDR0_ETH_CFG_EMAC_0_3_SWAP	0x00000040	/* Swap EMAC0 with EMAC3 */
#define SDR0_ETH_CFG_MDIO_SEL_MASK	0x00000030	/* MDIO source selector mask */
#define SDR0_ETH_CFG_MDIO_SEL_EMAC0	0x00000000	/* MDIO source - EMAC0 */
#define SDR0_ETH_CFG_MDIO_SEL_EMAC1	0x00000010	/* MDIO source - EMAC1 */
#define SDR0_ETH_CFG_MDIO_SEL_EMAC2	0x00000020	/* MDIO source - EMAC2 */
#define SDR0_ETH_CFG_MDIO_SEL_EMAC3	0x00000030	/* MDIO source - EMAC3 */
#define SDR0_ETH_CFG_ZMII_MODE_MASK	0x0000000C	/* ZMII bridge mode selector mask */
#define SDR0_ETH_CFG_ZMII_SEL_MII	0x00000000	/* ZMII bridge mode - MII */
#define SDR0_ETH_CFG_ZMII_SEL_SMII	0x00000004	/* ZMII bridge mode - SMII */
#define SDR0_ETH_CFG_ZMII_SEL_RMII_10	0x00000008	/* ZMII bridge mode - RMII (10 Mbps) */
#define SDR0_ETH_CFG_ZMII_SEL_RMII_100	0x0000000C	/* ZMII bridge mode - RMII (100 Mbps) */
#define SDR0_ETH_CFG_GMC1_BRIDGE_SEL	0x00000002	/* GMC Port 1 bridge selector */
#define SDR0_ETH_CFG_GMC0_BRIDGE_SEL	0x00000001	/* GMC Port 0 bridge selector */

#define SDR0_ETH_CFG_ZMII_MODE_SHIFT		4
#define SDR0_ETH_CFG_ZMII_MII_MODE		0x00
#define SDR0_ETH_CFG_ZMII_SMII_MODE		0x01
#define SDR0_ETH_CFG_ZMII_RMII_MODE_10M		0x10
#define SDR0_ETH_CFG_ZMII_RMII_MODE_100M	0x11

/* Miscealleneaous Function Reg. (SDR0_MFR) */
#define SDR0_MFR		0x4300
#define SDR0_MFR_T0TxFL		0x00800000	/* force parity error TAHOE0 Tx FIFO bits 0:63 */
#define SDR0_MFR_T0TxFH		0x00400000	/* force parity error TAHOE0 Tx FIFO bits 64:127 */
#define SDR0_MFR_T1TxFL		0x00200000	/* force parity error TAHOE1 Tx FIFO bits 0:63 */
#define SDR0_MFR_T1TxFH		0x00100000	/* force parity error TAHOE1 Tx FIFO bits 64:127 */
#define SDR0_MFR_E0TxFL		0x00008000	/* force parity error EMAC0 Tx FIFO bits 0:63 */
#define SDR0_MFR_E0TxFH		0x00004000	/* force parity error EMAC0 Tx FIFO bits 64:127 */
#define SDR0_MFR_E0RxFL		0x00002000	/* force parity error EMAC0 Rx FIFO bits 0:63 */
#define SDR0_MFR_E0RxFH		0x00001000	/* force parity error EMAC0 Rx FIFO bits 64:127 */
#define SDR0_MFR_E1TxFL		0x00000800	/* force parity error EMAC1 Tx FIFO bits 0:63 */
#define SDR0_MFR_E1TxFH		0x00000400	/* force parity error EMAC1 Tx FIFO bits 64:127 */
#define SDR0_MFR_E1RxFL		0x00000200	/* force parity error EMAC1 Rx FIFO bits 0:63 */
#define SDR0_MFR_E1RxFH		0x00000100	/* force parity error EMAC1 Rx FIFO bits 64:127 */
#define SDR0_MFR_E2TxFL		0x00000080	/* force parity error EMAC2 Tx FIFO bits 0:63 */
#define SDR0_MFR_E2TxFH		0x00000040	/* force parity error EMAC2 Tx FIFO bits 64:127 */
#define SDR0_MFR_E2RxFL		0x00000020	/* force parity error EMAC2 Rx FIFO bits 0:63 */
#define SDR0_MFR_E2RxFH		0x00000010	/* force parity error EMAC2 Rx FIFO bits 64:127 */
#define SDR0_MFR_E3TxFL		0x00000008	/* force parity error EMAC3 Tx FIFO bits 0:63 */
#define SDR0_MFR_E3TxFH		0x00000004	/* force parity error EMAC3 Tx FIFO bits 64:127 */
#define SDR0_MFR_E3RxFL		0x00000002	/* force parity error EMAC3 Rx FIFO bits 0:63 */
#define SDR0_MFR_E3RxFH		0x00000001	/* force parity error EMAC3 Rx FIFO bits 64:127 */

/* EMACx TX Status Register (SDR0_EMACxTXST)*/
#define SDR0_EMAC0TXST		0x4400
#define SDR0_EMAC1TXST		0x4401
#define SDR0_EMAC2TXST		0x4402
#define SDR0_EMAC3TXST		0x4403

#define SDR0_EMACxTXST_FUR	0x02000000	/* TX FIFO underrun */
#define SDR0_EMACxTXST_BC	0x01000000	/* broadcase address */
#define SDR0_EMACxTXST_MC	0x00800000	/* multicast address */
#define SDR0_EMACxTXST_UC	0x00400000	/* unicast address */
#define SDR0_EMACxTXST_FP	0x00200000	/* frame paused by control packet */
#define SDR0_EMACxTXST_BFCS	0x00100000	/* bad FCS in the transmitted frame */
#define SDR0_EMACxTXST_CPF	0x00080000	/* TX control pause frame */
#define SDR0_EMACxTXST_CF	0x00040000	/* TX control frame */
#define SDR0_EMACxTXST_MSIZ	0x00020000	/* 1024-maxsize bytes transmitted */
#define SDR0_EMACxTXST_1023	0x00010000	/* 512-1023 bytes transmitted */
#define SDR0_EMACxTXST_511	0x00008000	/* 256-511 bytes transmitted */
#define SDR0_EMACxTXST_255	0x00004000	/* 128-255 bytes transmitted */
#define SDR0_EMACxTXST_127	0x00002000	/* 65-127 bytes transmitted */
#define SDR0_EMACxTXST_64	0x00001000	/* 64 bytes transmitted */
#define SDR0_EMACxTXST_SQE	0x00000800	/* SQE indication */
#define SDR0_EMACxTXST_LOC	0x00000400	/* loss of carrier sense */
#define SDR0_EMACxTXST_IERR	0x00000080	/* EMAC internal error */
#define SDR0_EMACxTXST_EDF	0x00000040	/* excessive deferral */
#define SDR0_EMACxTXST_ECOL	0x00000020	/* excessive collisions */
#define SDR0_EMACxTXST_LCOL	0x00000010	/* late collision */
#define SDR0_EMACxTXST_DFFR	0x00000008	/* deferred frame */
#define SDR0_EMACxTXST_MCOL	0x00000004	/* multiple collision frame */
#define SDR0_EMACxTXST_SCOL	0x00000002	/* single collision frame */
#define SDR0_EMACxTXST_TXOK	0x00000001	/* transmit OK */

/* EMACx RX Status Register (SDR0_EMACxRXST)*/
#define SDR0_EMAC0RXST		0x4404
#define SDR0_EMAC1RXST		0x4405
#define SDR0_EMAC2RXST		0x4406
#define SDR0_EMAC3RXST		0x4407

#define SDR0_EMACxRXST_FOR	0x20000000	/* RX FIFO overrun */
#define SDR0_EMACxRXST_BC	0x10000000	/* broadcast address */
#define SDR0_EMACxRXST_MC	0x08000000	/* multicast address */
#define SDR0_EMACxRXST_UC	0x04000000	/* unicast address */
#define SDR0_EMACxRXST_UPR_MASK	0x03800000	/* user priority field */
#define SDR0_EMACxRXST_UPR_ENCODE(n)	((((unsigned long)(n))&0x07)<<23)
#define SDR0_EMACxRXST_VLAN	0x00400000	/* RX VLAN tagged frame */
#define SDR0_EMACxRXST_LOOP	0x00200000	/* received in loop-back mode */
#define SDR0_EMACxRXST_UOP	0x00100000	/* RX unsupported opcode */
#define SDR0_EMACxRXST_CPF	0x00080000	/* RX control pause frame */
#define SDR0_EMACxRXST_CF	0x00040000	/* RX control frame*/
#define SDR0_EMACxRXST_MSIZ	0x00020000	/* 1024-MaxSize bytes recieved*/
#define SDR0_EMACxRXST_1023	0x00010000	/* 512-1023 bytes received */
#define SDR0_EMACxRXST_511	0x00008000	/* 128-511 bytes received */
#define SDR0_EMACxRXST_255	0x00004000	/* 128-255 bytes received */
#define SDR0_EMACxRXST_127	0x00002000	/* 65-127 bytes received */
#define SDR0_EMACxRXST_64	0x00001000	/* 64 bytes received */
#define SDR0_EMACxRXST_RUNT	0x00000800	/* runt frame */
#define SDR0_EMACxRXST_SEVT	0x00000400	/* short event */
#define SDR0_EMACxRXST_AERR	0x00000200	/* alignment error */
#define SDR0_EMACxRXST_SERR	0x00000100	/* received with symbol error */
#define SDR0_EMACxRXST_BURST	0x00000040	/* received burst */
#define SDR0_EMACxRXST_F2L	0x00000020	/* frame is to long */
#define SDR0_EMACxRXST_OERR	0x00000010	/* out of range length error */
#define SDR0_EMACxRXST_IERR	0x00000008	/* in range length error */
#define SDR0_EMACxRXST_LOST	0x00000004	/* frame lost due to internal EMAC receive error */
#define SDR0_EMACxRXST_BFCS	0x00000002	/* bad FCS in the recieved frame */
#define SDR0_EMACxRXST_RXOK	0x00000001	/* Recieve OK */

/* EMACx TX Status Register (SDR0_EMACxREJCNT)*/
#define SDR0_EMAC0REJCNT	0x4408
#define SDR0_EMAC1REJCNT	0x4409
#define SDR0_EMAC2REJCNT	0x440A
#define SDR0_EMAC3REJCNT	0x440B

#define SDR0_DDR0			0x00E1
#define SDR0_DDR0_DPLLRST		0x80000000
#define SDR0_DDR0_DDRM_MASK		0x60000000
#define SDR0_DDR0_DDRM_DDR1		0x20000000
#define SDR0_DDR0_DDRM_DDR2		0x40000000
#define SDR0_DDR0_DDRM_ENCODE(n)	((((unsigned long)(n))&0x03)<<29)
#define SDR0_DDR0_DDRM_DECODE(n)	((((unsigned long)(n))>>29)&0x03)
#define SDR0_DDR0_TUNE_ENCODE(n)	((((unsigned long)(n))&0x2FF)<<0)
#define SDR0_DDR0_TUNE_DECODE(n)	((((unsigned long)(n))>>0)&0x2FF)

#define AHB_TOP			0xA4
#define AHB_BOT			0xA5
#endif /* CONFIG_460EX || CONFIG_460GT */

#define SDR0_SDCS_SDD			(0x80000000 >> 31)

#if defined(CONFIG_440GP)
#define CPC0_STRP1_PAE_MASK		(0x80000000 >> 11)
#define CPC0_STRP1_PISE_MASK		(0x80000000 >> 13)
#endif /* defined(CONFIG_440GP) */
#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 13)
#define SDR0_SDSTP1_PISE_MASK		(0x80000000 >> 15)
#endif /* defined(CONFIG_440GX) || defined(CONFIG_440SP) */
#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 21)
#define SDR0_SDSTP1_PAME_MASK		(0x80000000 >> 27)
#endif /* defined(CONFIG_440EP) || defined(CONFIG_440GR) */

#define SDR0_UARTX_UXICS_MASK		0xF0000000
#define SDR0_UARTX_UXICS_PLB		0x20000000
#define SDR0_UARTX_UXEC_MASK		0x00800000
#define SDR0_UARTX_UXEC_INT		0x00000000
#define SDR0_UARTX_UXEC_EXT		0x00800000
#define SDR0_UARTX_UXDTE_MASK		0x00400000
#define SDR0_UARTX_UXDTE_DISABLE	0x00000000
#define SDR0_UARTX_UXDTE_ENABLE		0x00400000
#define SDR0_UARTX_UXDRE_MASK		0x00200000
#define SDR0_UARTX_UXDRE_DISABLE	0x00000000
#define SDR0_UARTX_UXDRE_ENABLE		0x00200000
#define SDR0_UARTX_UXDC_MASK		0x00100000
#define SDR0_UARTX_UXDC_NOTCLEARED	0x00000000
#define SDR0_UARTX_UXDC_CLEARED		0x00100000
#define SDR0_UARTX_UXDIV_MASK		0x000000FF
#define SDR0_UARTX_UXDIV_ENCODE(n)	((((unsigned long)(n))&0xFF)<<0)
#define SDR0_UARTX_UXDIV_DECODE(n)	((((((unsigned long)(n))>>0)-1)&0xFF)+1)

#define SDR0_CPU440_EARV_MASK		0x30000000
#define SDR0_CPU440_EARV_EBC		0x10000000
#define SDR0_CPU440_EARV_PCI		0x20000000
#define SDR0_CPU440_EARV_ENCODE(n)	((((unsigned long)(n))&0x03)<<28)
#define SDR0_CPU440_EARV_DECODE(n)	((((unsigned long)(n))>>28)&0x03)
#define SDR0_CPU440_NTO1_MASK		0x00000002
#define SDR0_CPU440_NTO1_NTOP		0x00000000
#define SDR0_CPU440_NTO1_NTO1		0x00000002
#define SDR0_CPU440_NTO1_ENCODE(n)	((((unsigned long)(n))&0x01)<<1)
#define SDR0_CPU440_NTO1_DECODE(n)	((((unsigned long)(n))>>1)&0x01)

#define SDR0_XCR_PAE_MASK		0x80000000
#define SDR0_XCR_PAE_DISABLE		0x00000000
#define SDR0_XCR_PAE_ENABLE		0x80000000
#define SDR0_XCR_PAE_ENCODE(n)		((((unsigned long)(n))&0x01)<<31)
#define SDR0_XCR_PAE_DECODE(n)		((((unsigned long)(n))>>31)&0x01)
#define SDR0_XCR_PHCE_MASK		0x40000000
#define SDR0_XCR_PHCE_DISABLE		0x00000000
#define SDR0_XCR_PHCE_ENABLE		0x40000000
#define SDR0_XCR_PHCE_ENCODE(n)		((((unsigned long)(n))&0x01)<<30)
#define SDR0_XCR_PHCE_DECODE(n)		((((unsigned long)(n))>>30)&0x01)
#define SDR0_XCR_PISE_MASK		0x20000000
#define SDR0_XCR_PISE_DISABLE		0x00000000
#define SDR0_XCR_PISE_ENABLE		0x20000000
#define SDR0_XCR_PISE_ENCODE(n)		((((unsigned long)(n))&0x01)<<29)
#define SDR0_XCR_PISE_DECODE(n)		((((unsigned long)(n))>>29)&0x01)
#define SDR0_XCR_PCWE_MASK		0x10000000
#define SDR0_XCR_PCWE_DISABLE		0x00000000
#define SDR0_XCR_PCWE_ENABLE		0x10000000
#define SDR0_XCR_PCWE_ENCODE(n)		((((unsigned long)(n))&0x01)<<28)
#define SDR0_XCR_PCWE_DECODE(n)		((((unsigned long)(n))>>28)&0x01)
#define SDR0_XCR_PPIM_MASK		0x0F000000
#define SDR0_XCR_PPIM_ENCODE(n)		((((unsigned long)(n))&0x0F)<<24)
#define SDR0_XCR_PPIM_DECODE(n)		((((unsigned long)(n))>>24)&0x0F)
#define SDR0_XCR_PR64E_MASK		0x00800000
#define SDR0_XCR_PR64E_DISABLE		0x00000000
#define SDR0_XCR_PR64E_ENABLE		0x00800000
#define SDR0_XCR_PR64E_ENCODE(n)	((((unsigned long)(n))&0x01)<<23)
#define SDR0_XCR_PR64E_DECODE(n)	((((unsigned long)(n))>>23)&0x01)
#define SDR0_XCR_PXFS_MASK		0x00600000
#define SDR0_XCR_PXFS_HIGH		0x00000000
#define SDR0_XCR_PXFS_MED		0x00200000
#define SDR0_XCR_PXFS_LOW		0x00400000
#define SDR0_XCR_PXFS_ENCODE(n)		((((unsigned long)(n))&0x03)<<21)
#define SDR0_XCR_PXFS_DECODE(n)		((((unsigned long)(n))>>21)&0x03)
#define SDR0_XCR_PDM_MASK		0x00000040
#define SDR0_XCR_PDM_MULTIPOINT		0x00000000
#define SDR0_XCR_PDM_P2P		0x00000040
#define SDR0_XCR_PDM_ENCODE(n)		((((unsigned long)(n))&0x01)<<19)
#define SDR0_XCR_PDM_DECODE(n)		((((unsigned long)(n))>>19)&0x01)

#define SDR0_PFC0_UART1_DSR_CTS_EN_MASK 0x00030000
#define SDR0_PFC0_GEIE_MASK		0x00003E00
#define SDR0_PFC0_GEIE_TRE		0x00003E00
#define SDR0_PFC0_GEIE_NOTRE		0x00000000
#define SDR0_PFC0_TRE_MASK		0x00000100
#define SDR0_PFC0_TRE_DISABLE		0x00000000
#define SDR0_PFC0_TRE_ENABLE		0x00000100
#define SDR0_PFC0_TRE_ENCODE(n)		((((unsigned long)(n))&0x01)<<8)
#define SDR0_PFC0_TRE_DECODE(n)		((((unsigned long)(n))>>8)&0x01)

#define SDR0_PFC1_UART1_DSR_CTS_MASK	0x02000000
#define SDR0_PFC1_EPS_MASK		0x01C00000
#define SDR0_PFC1_EPS_GROUP0		0x00000000
#define SDR0_PFC1_EPS_GROUP1		0x00400000
#define SDR0_PFC1_EPS_GROUP2		0x00800000
#define SDR0_PFC1_EPS_GROUP3		0x00C00000
#define SDR0_PFC1_EPS_GROUP4		0x01000000
#define SDR0_PFC1_EPS_GROUP5		0x01400000
#define SDR0_PFC1_EPS_GROUP6		0x01800000
#define SDR0_PFC1_EPS_GROUP7		0x01C00000
#define SDR0_PFC1_EPS_ENCODE(n)		((((unsigned long)(n))&0x07)<<22)
#define SDR0_PFC1_EPS_DECODE(n)		((((unsigned long)(n))>>22)&0x07)
#define SDR0_PFC1_RMII_MASK		0x00200000
#define SDR0_PFC1_RMII_100MBIT		0x00000000
#define SDR0_PFC1_RMII_10MBIT		0x00200000
#define SDR0_PFC1_RMII_ENCODE(n)	((((unsigned long)(n))&0x01)<<21)
#define SDR0_PFC1_RMII_DECODE(n)	((((unsigned long)(n))>>21)&0x01)
#define SDR0_PFC1_CTEMS_MASK		0x00100000
#define SDR0_PFC1_CTEMS_EMS		0x00000000
#define SDR0_PFC1_CTEMS_CPUTRACE	0x00100000

#define SDR0_MFR_TAH0_MASK		0x80000000
#define SDR0_MFR_TAH0_ENABLE		0x00000000
#define SDR0_MFR_TAH0_DISABLE		0x80000000
#define SDR0_MFR_TAH1_MASK		0x40000000
#define SDR0_MFR_TAH1_ENABLE		0x00000000
#define SDR0_MFR_TAH1_DISABLE		0x40000000
#define SDR0_MFR_PCM_MASK		0x20000000
#define SDR0_MFR_PCM_PPC440GX		0x00000000
#define SDR0_MFR_PCM_PPC440GP		0x20000000
#define SDR0_MFR_ECS_MASK		0x10000000
#define SDR0_MFR_ECS_INTERNAL		0x10000000

#define SDR0_MFR_ETH0_CLK_SEL        0x08000000   /* Ethernet0 Clock Select */
#define SDR0_MFR_ETH1_CLK_SEL        0x04000000   /* Ethernet1 Clock Select */
#define SDR0_MFR_ZMII_MODE_MASK      0x03000000   /* ZMII Mode Mask   */
#define SDR0_MFR_ZMII_MODE_MII       0x00000000     /* ZMII Mode MII  */
#define SDR0_MFR_ZMII_MODE_SMII      0x01000000     /* ZMII Mode SMII */
#define SDR0_MFR_ZMII_MODE_RMII_10M  0x02000000     /* ZMII Mode RMII - 10 Mbs   */
#define SDR0_MFR_ZMII_MODE_RMII_100M 0x03000000     /* ZMII Mode RMII - 100 Mbs  */
#define SDR0_MFR_ZMII_MODE_BIT0      0x02000000     /* ZMII Mode Bit0 */
#define SDR0_MFR_ZMII_MODE_BIT1      0x01000000     /* ZMII Mode Bit1 */
#define SDR0_MFR_ERRATA3_EN0         0x00800000
#define SDR0_MFR_ERRATA3_EN1         0x00400000
#if defined(CONFIG_440GX) /* test-only: only 440GX or 440SPE??? */
#define SDR0_MFR_PKT_REJ_MASK        0x00300000   /* Pkt Rej. Enable Mask */
#define SDR0_MFR_PKT_REJ_EN          0x00300000   /* Pkt Rej. Enable on both EMAC3 0-1 */
#define SDR0_MFR_PKT_REJ_EN0         0x00200000   /* Pkt Rej. Enable on EMAC3(0) */
#define SDR0_MFR_PKT_REJ_EN1         0x00100000   /* Pkt Rej. Enable on EMAC3(1) */
#define SDR0_MFR_PKT_REJ_POL         0x00080000   /* Packet Reject Polarity      */
#endif

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_PFC1_EPS_ENCODE(n)		((((unsigned long)(n))&0x07)<<22)
#define SDR0_PFC1_EPS_DECODE(n)		((((unsigned long)(n))>>22)&0x07)
#define SDR0_PFC2_EPS_ENCODE(n)		((((unsigned long)(n))&0x07)<<29)
#define SDR0_PFC2_EPS_DECODE(n)		((((unsigned long)(n))>>29)&0x07)
#endif

#define SDR0_MFR_ECS_MASK		0x10000000
#define SDR0_MFR_ECS_INTERNAL		0x10000000

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_SRST0        0x200
#define SDR0_SRST0_BGO          0x80000000 /* PLB to OPB bridge */
#define SDR0_SRST0_PLB4         0x40000000 /* PLB4 arbiter */
#define SDR0_SRST0_EBC          0x20000000 /* External bus controller */
#define SDR0_SRST0_OPB          0x10000000 /* OPB arbiter */
#define SDR0_SRST0_UART0        0x08000000 /* Universal asynchronous receiver/transmitter 0 */
#define SDR0_SRST0_UART1        0x04000000 /* Universal asynchronous receiver/transmitter 1 */
#define SDR0_SRST0_IIC0         0x02000000 /* Inter integrated circuit 0 */
#define SDR0_SRST0_USB2H        0x01000000 /* USB2.0 Host */
#define SDR0_SRST0_GPIO         0x00800000 /* General purpose I/O */
#define SDR0_SRST0_GPT          0x00400000 /* General purpose timer */
#define SDR0_SRST0_DMC          0x00200000 /* DDR SDRAM memory controller */
#define SDR0_SRST0_PCI          0x00100000 /* PCI */
#define SDR0_SRST0_EMAC0        0x00080000 /* Ethernet media access controller 0 */
#define SDR0_SRST0_EMAC1        0x00040000 /* Ethernet media access controller 1 */
#define SDR0_SRST0_CPM0         0x00020000 /* Clock and power management */
#define SDR0_SRST0_ZMII         0x00010000 /* ZMII bridge */
#define SDR0_SRST0_UIC0         0x00008000 /* Universal interrupt controller 0 */
#define SDR0_SRST0_UIC1         0x00004000 /* Universal interrupt controller 1 */
#define SDR0_SRST0_IIC1         0x00002000 /* Inter integrated circuit 1 */
#define SDR0_SRST0_SCP          0x00001000 /* Serial communications port */
#define SDR0_SRST0_BGI          0x00000800 /* OPB to PLB bridge */
#define SDR0_SRST0_DMA          0x00000400 /* Direct memory access controller */
#define SDR0_SRST0_DMAC         0x00000200 /* DMA channel */
#define SDR0_SRST0_MAL          0x00000100 /* Media access layer */
#define SDR0_SRST0_USB2D        0x00000080 /* USB2.0 device */
#define SDR0_SRST0_GPTR         0x00000040 /* General purpose timer */
#define SDR0_SRST0_P4P3         0x00000010 /* PLB4 to PLB3 bridge */
#define SDR0_SRST0_P3P4         0x00000008 /* PLB3 to PLB4 bridge */
#define SDR0_SRST0_PLB3         0x00000004 /* PLB3 arbiter */
#define SDR0_SRST0_UART2        0x00000002 /* Universal asynchronous receiver/transmitter 2 */
#define SDR0_SRST0_UART3        0x00000001 /* Universal asynchronous receiver/transmitter 3 */

#define SDR0_SRST1        0x201
#define SDR0_SRST1_NDFC         0x80000000 /* Nand flash controller */
#define SDR0_SRST1_OPBA1        0x40000000 /* OPB Arbiter attached to PLB4 */
#define SDR0_SRST1_P4OPB0       0x20000000 /* PLB4 to OPB Bridge0 */
#define SDR0_SRST1_PLB42OPB0    SDR0_SRST1_P4OPB0
#define SDR0_SRST1_DMA4         0x10000000 /* DMA to PLB4 */
#define SDR0_SRST1_DMA4CH       0x08000000 /* DMA Channel to PLB4 */
#define SDR0_SRST1_OPBA2        0x04000000 /* OPB Arbiter attached to PLB4 USB 2.0 Host */
#define SDR0_SRST1_OPB2PLB40    0x02000000 /* OPB to PLB4 Bridge attached to USB 2.0 Host */
#define SDR0_SRST1_PLB42OPB1    0x01000000 /* PLB4 to OPB Bridge attached to USB 2.0 Host */
#define SDR0_SRST1_CPM1         0x00800000 /* Clock and Power management 1 */
#define SDR0_SRST1_UIC2         0x00400000 /* Universal Interrupt Controller 2 */
#define SDR0_SRST1_CRYP0        0x00200000 /* Security Engine */
#define SDR0_SRST1_USB20PHY     0x00100000 /* USB 2.0 Phy */
#define SDR0_SRST1_USB2HUTMI    0x00080000 /* USB 2.0 Host UTMI Interface */
#define SDR0_SRST1_USB2HPHY     0x00040000 /* USB 2.0 Host Phy Interface */
#define SDR0_SRST1_SRAM0        0x00020000 /* Internal SRAM Controller */
#define SDR0_SRST1_RGMII0       0x00010000 /* RGMII Bridge */
#define SDR0_SRST1_ETHPLL       0x00008000 /* Ethernet PLL */
#define SDR0_SRST1_FPU          0x00004000 /* Floating Point Unit */
#define SDR0_SRST1_KASU0        0x00002000 /* Kasumi Engine */

#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)

#define SDR0_SRST0		0x0200
#define SDR0_SRST		SDR0_SRST0 /* for compatability reasons */
#define SDR0_SRST0_BGO		0x80000000 /* PLB to OPB bridge */
#define SDR0_SRST0_PLB4		0x40000000 /* PLB4 arbiter */
#define SDR0_SRST0_EBC		0x20000000 /* External bus controller */
#define SDR0_SRST0_OPB		0x10000000 /* OPB arbiter */
#define SDR0_SRST0_UART0	0x08000000 /* Universal asynchronous receiver/transmitter 0 */
#define SDR0_SRST0_UART1	0x04000000 /* Universal asynchronous receiver/transmitter 1 */
#define SDR0_SRST0_IIC0		0x02000000 /* Inter integrated circuit 0 */
#define SDR0_SRST0_IIC1		0x01000000 /* Inter integrated circuit 1 */
#define SDR0_SRST0_GPIO0	0x00800000 /* General purpose I/O 0 */
#define SDR0_SRST0_GPT		0x00400000 /* General purpose timer */
#define SDR0_SRST0_DMC		0x00200000 /* DDR SDRAM memory controller */
#define SDR0_SRST0_PCI		0x00100000 /* PCI */
#define SDR0_SRST0_CPM0		0x00020000 /* Clock and power management */
#define SDR0_SRST0_IMU		0x00010000 /* I2O DMA */
#define SDR0_SRST0_UIC0		0x00008000 /* Universal interrupt controller 0*/
#define SDR0_SRST0_UIC1		0x00004000 /* Universal interrupt controller 1*/
#define SDR0_SRST0_SRAM		0x00002000 /* Universal interrupt controller 0*/
#define SDR0_SRST0_UIC2		0x00001000 /* Universal interrupt controller 2*/
#define SDR0_SRST0_UIC3		0x00000800 /* Universal interrupt controller 3*/
#define SDR0_SRST0_OCM		0x00000400 /* Universal interrupt controller 0*/
#define SDR0_SRST0_UART2	0x00000200 /* Universal asynchronous receiver/transmitter 2 */
#define SDR0_SRST0_MAL		0x00000100 /* Media access layer */
#define SDR0_SRST0_GPTR         0x00000040 /* General purpose timer */
#define SDR0_SRST0_L2CACHE	0x00000004 /* L2 Cache */
#define SDR0_SRST0_UART3	0x00000002 /* Universal asynchronous receiver/transmitter 3 */
#define SDR0_SRST0_GPIO1	0x00000001 /* General purpose I/O 1 */

#define SDR0_SRST1		0x201
#define SDR0_SRST1_RLL		0x80000000 /* SRIO RLL */
#define SDR0_SRST1_SCP		0x40000000 /* Serial communications port */
#define SDR0_SRST1_PLBARB	0x20000000 /* PLB Arbiter */
#define SDR0_SRST1_EIPPKP	0x10000000 /* EIPPPKP */
#define SDR0_SRST1_EIP94	0x08000000 /* EIP 94 */
#define SDR0_SRST1_EMAC0	0x04000000 /* Ethernet media access controller 0 */
#define SDR0_SRST1_EMAC1	0x02000000 /* Ethernet media access controller 1 */
#define SDR0_SRST1_EMAC2	0x01000000 /* Ethernet media access controller 2 */
#define SDR0_SRST1_EMAC3	0x00800000 /* Ethernet media access controller 3 */
#define SDR0_SRST1_ZMII		0x00400000 /* Ethernet ZMII/RMII/SMII */
#define SDR0_SRST1_RGMII0	0x00200000 /* Ethernet RGMII/RTBI 0 */
#define SDR0_SRST1_RGMII1	0x00100000 /* Ethernet RGMII/RTBI 1 */
#define SDR0_SRST1_DMA4		0x00080000 /* DMA to PLB4 */
#define SDR0_SRST1_DMA4CH	0x00040000 /* DMA Channel to PLB4 */
#define SDR0_SRST1_SATAPHY	0x00020000 /* Serial ATA PHY */
#define SDR0_SRST1_SRIODEV	0x00010000 /* Serial Rapid IO core, PCS, and serdes */
#define SDR0_SRST1_SRIOPCS	0x00008000 /* Serial Rapid IO core and PCS */
#define SDR0_SRST1_NDFC		0x00004000 /* Nand flash controller */
#define SDR0_SRST1_SRIOPLB	0x00002000 /* Serial Rapid IO PLB */
#define SDR0_SRST1_ETHPLL	0x00001000 /* Ethernet PLL */
#define SDR0_SRST1_TAHOE1	0x00000800 /* Ethernet Tahoe 1 */
#define SDR0_SRST1_TAHOE0	0x00000400 /* Ethernet Tahoe 0 */
#define SDR0_SRST1_SGMII0	0x00000200 /* Ethernet SGMII 0 */
#define SDR0_SRST1_SGMII1	0x00000100 /* Ethernet SGMII 1 */
#define SDR0_SRST1_SGMII2	0x00000080 /* Ethernet SGMII 2 */
#define SDR0_SRST1_AHB		0x00000040 /* PLB4XAHB bridge */
#define SDR0_SRST1_USBOTGPHY	0x00000020 /* USB 2.0 OTG PHY */
#define SDR0_SRST1_USBOTG	0x00000010 /* USB 2.0 OTG controller */
#define SDR0_SRST1_USBHOST	0x00000008 /* USB 2.0 Host controller */
#define SDR0_SRST1_AHBDMAC	0x00000004 /* AHB DMA controller */
#define SDR0_SRST1_AHBICM	0x00000002 /* AHB inter-connect matrix */
#define SDR0_SRST1_SATA		0x00000001 /* Serial ATA controller */

#define SDR0_PCI0		0x1c0		/* PCI Configuration Register */

#else

#define SDR0_SRST_BGO			0x80000000
#define SDR0_SRST_PLB			0x40000000
#define SDR0_SRST_EBC			0x20000000
#define SDR0_SRST_OPB			0x10000000
#define SDR0_SRST_UART0			0x08000000
#define SDR0_SRST_UART1			0x04000000
#define SDR0_SRST_IIC0			0x02000000
#define SDR0_SRST_IIC1			0x01000000
#define SDR0_SRST_GPIO			0x00800000
#define SDR0_SRST_GPT			0x00400000
#define SDR0_SRST_DMC			0x00200000
#define SDR0_SRST_PCI			0x00100000
#define SDR0_SRST_EMAC0			0x00080000
#define SDR0_SRST_EMAC1			0x00040000
#define SDR0_SRST_CPM			0x00020000
#define SDR0_SRST_IMU			0x00010000
#define SDR0_SRST_UIC01			0x00008000
#define SDR0_SRST_UICB2			0x00004000
#define SDR0_SRST_SRAM			0x00002000
#define SDR0_SRST_EBM			0x00001000
#define SDR0_SRST_BGI			0x00000800
#define SDR0_SRST_DMA			0x00000400
#define SDR0_SRST_DMAC			0x00000200
#define SDR0_SRST_MAL			0x00000100
#define SDR0_SRST_ZMII			0x00000080
#define SDR0_SRST_GPTR			0x00000040
#define SDR0_SRST_PPM			0x00000020
#define SDR0_SRST_EMAC2			0x00000010
#define SDR0_SRST_EMAC3			0x00000008
#define SDR0_SRST_RGMII			0x00000001

#endif

/*-----------------------------------------------------------------------------+
|  Clocking
+-----------------------------------------------------------------------------*/
#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define PLLSYS0_FWD_DIV_A_MASK	0x000000f0	/* Fwd Div A */
#define PLLSYS0_FWD_DIV_B_MASK	0x0000000f	/* Fwd Div B */
#define PLLSYS0_FB_DIV_MASK	0x0000ff00	/* Feedback divisor */
#define PLLSYS0_OPB_DIV_MASK	0x0c000000	/* OPB Divisor */
#define PLLSYS0_PLBEDV0_DIV_MASK 0xe0000000	/* PLB Early Clock Divisor */
#define PLLSYS0_PERCLK_DIV_MASK 0x03000000	/* Peripheral Clk Divisor */
#define PLLSYS0_SEL_MASK	0x18000000	/* 0 = PLL, 1 = PerClk */
#elif !defined (CONFIG_440GX) && \
    !defined(CONFIG_440EP) && !defined(CONFIG_440GR) && \
    !defined(CONFIG_440EPX) && !defined(CONFIG_440GRX) && \
    !defined(CONFIG_440SP) && !defined(CONFIG_440SPE)
#define PLLSYS0_TUNE_MASK	0xffc00000	/* PLL TUNE bits	    */
#define PLLSYS0_FB_DIV_MASK	0x003c0000	/* Feedback divisor	    */
#define PLLSYS0_FWD_DIV_A_MASK	0x00038000	/* Forward divisor A	    */
#define PLLSYS0_FWD_DIV_B_MASK	0x00007000	/* Forward divisor B	    */
#define PLLSYS0_OPB_DIV_MASK	0x00000c00	/* OPB divisor		    */
#define PLLSYS0_EPB_DIV_MASK	0x00000300	/* EPB divisor		    */
#define PLLSYS0_EXTSL_MASK	0x00000080	/* PerClk feedback path	    */
#define PLLSYS0_RW_MASK		0x00000060	/* ROM width		    */
#define PLLSYS0_RL_MASK		0x00000010	/* ROM location		    */
#define PLLSYS0_ZMII_SEL_MASK	0x0000000c	/* ZMII selection	    */
#define PLLSYS0_BYPASS_MASK	0x00000002	/* Bypass PLL		    */
#define PLLSYS0_NTO1_MASK	0x00000001	/* CPU:PLB N-to-1 ratio	    */

#define PLL_VCO_FREQ_MIN	500		/* Min VCO freq (MHz)	    */
#define PLL_VCO_FREQ_MAX	1000		/* Max VCO freq (MHz)	    */
#define PLL_CPU_FREQ_MAX	400		/* Max CPU freq (MHz)	    */
#define PLL_PLB_FREQ_MAX	133		/* Max PLB freq (MHz)	    */
#else /* !CONFIG_440GX or CONFIG_440EP or CONFIG_440GR */
#define PLLSYS0_ENG_MASK	0x80000000	/* 0 = SysClk, 1 = PLL VCO */
#define PLLSYS0_SRC_MASK	0x40000000	/* 0 = PLL A, 1 = PLL B */
#define PLLSYS0_SEL_MASK	0x38000000	/* 0 = PLL, 1 = CPU, 5 = PerClk */
#define PLLSYS0_TUNE_MASK	0x07fe0000	/* PLL Tune bits */
#define PLLSYS0_FB_DIV_MASK	0x0001f000	/* Feedback divisor */
#define PLLSYS0_FWD_DIV_A_MASK	0x00000f00	/* Fwd Div A */
#define PLLSYS0_FWD_DIV_B_MASK	0x000000e0	/* Fwd Div B */
#define PLLSYS0_PRI_DIV_B_MASK	0x0000001c	/* PLL Primary Divisor B */
#define PLLSYS0_OPB_DIV_MASK	0x00000003	/* OPB Divisor */

#define PLLC_ENG_MASK       0x20000000  /* PLL primary forward divisor source   */
#define PLLC_SRC_MASK       0x20000000  /* PLL feedback source   */
#define PLLD_FBDV_MASK      0x1f000000  /* PLL Feedback Divisor  */
#define PLLD_FWDVA_MASK     0x000f0000  /* PLL Forward Divisor A */
#define PLLD_FWDVB_MASK     0x00000700  /* PLL Forward Divisor B */
#define PLLD_LFBDV_MASK     0x0000003f  /* PLL Local Feedback Divisor */

#define OPBDDV_MASK         0x03000000  /* OPB Clock Divisor Register */
#define PERDV_MASK          0x07000000  /* Periferal Clock Divisor */
#define PRADV_MASK          0x07000000  /* Primary Divisor A */
#define PRBDV_MASK          0x07000000  /* Primary Divisor B */
#define SPCID_MASK          0x03000000  /* Sync PCI Divisor  */

#define PLL_VCO_FREQ_MIN	500		/* Min VCO freq (MHz)	    */
#define PLL_VCO_FREQ_MAX	1000		/* Max VCO freq (MHz)	    */
#define PLL_CPU_FREQ_MAX	400		/* Max CPU freq (MHz)	    */
#define PLL_PLB_FREQ_MAX	133		/* Max PLB freq (MHz)	    */

/* Strap 1 Register */
#define PLLSYS1_LF_DIV_MASK	0xfc000000	/* PLL Local Feedback Divisor */
#define PLLSYS1_PERCLK_DIV_MASK 0x03000000	/* Peripheral Clk Divisor */
#define PLLSYS1_MAL_DIV_MASK	0x00c00000	/* MAL Clk Divisor */
#define PLLSYS1_RW_MASK		0x00300000	/* ROM width */
#define PLLSYS1_EAR_MASK	0x00080000	/* ERAP Addres reset vector */
#define PLLSYS1_PAE_MASK	0x00040000	/* PCI arbitor enable */
#define PLLSYS1_PCHE_MASK	0x00020000	/* PCI host config enable */
#define PLLSYS1_PISE_MASK	0x00010000	/* PCI init seq. enable */
#define PLLSYS1_PCWE_MASK	0x00008000	/* PCI local cpu wait enable */
#define PLLSYS1_PPIM_MASK	0x00007800	/* PCI inbound map */
#define PLLSYS1_PR64E_MASK	0x00000400	/* PCI init Req64 enable */
#define PLLSYS1_PXFS_MASK	0x00000300	/* PCI-X Freq Sel */
#define PLLSYS1_RSVD_MASK	0x00000080	/* RSVD */
#define PLLSYS1_PDM_MASK	0x00000040	/* PCI-X Driver Mode */
#define PLLSYS1_EPS_MASK	0x00000038	/* Ethernet Pin Select */
#define PLLSYS1_RMII_MASK	0x00000004	/* RMII Mode */
#define PLLSYS1_TRE_MASK	0x00000002	/* GPIO Trace Enable */
#define PLLSYS1_NTO1_MASK	0x00000001	/* CPU:PLB N-to-1 ratio */
#endif /* CONFIG_440GX */

#if defined (CONFIG_440EPX) || defined (CONFIG_440GRX)
/*--------------------------------------*/
#define CPR0_PLLC                   0x40
#define   CPR0_PLLC_RST_MASK           0x80000000
#define   CPR0_PLLC_RST_PLLLOCKED      0x00000000
#define   CPR0_PLLC_RST_PLLRESET       0x80000000
#define   CPR0_PLLC_ENG_MASK           0x40000000
#define   CPR0_PLLC_ENG_DISABLE        0x00000000
#define   CPR0_PLLC_ENG_ENABLE         0x40000000
#define   CPR0_PLLC_ENG_ENCODE(n)      ((((unsigned long)(n))&0x01)<<30)
#define   CPR0_PLLC_ENG_DECODE(n)      ((((unsigned long)(n))>>30)&0x01)
#define   CPR0_PLLC_SRC_MASK           0x20000000
#define   CPR0_PLLC_SRC_PLLOUTA        0x00000000
#define   CPR0_PLLC_SRC_PLLOUTB        0x20000000
#define   CPR0_PLLC_SRC_ENCODE(n)      ((((unsigned long)(n))&0x01)<<29)
#define   CPR0_PLLC_SRC_DECODE(n)      ((((unsigned long)(n))>>29)&0x01)
#define   CPR0_PLLC_SEL_MASK           0x07000000
#define   CPR0_PLLC_SEL_PLL            0x00000000
#define   CPR0_PLLC_SEL_CPU            0x01000000
#define   CPR0_PLLC_SEL_PER            0x05000000
#define   CPR0_PLLC_SEL_ENCODE(n)      ((((unsigned long)(n))&0x07)<<24)
#define   CPR0_PLLC_SEL_DECODE(n)      ((((unsigned long)(n))>>24)&0x07)
#define   CPR0_PLLC_TUNE_MASK          0x000003FF
#define   CPR0_PLLC_TUNE_ENCODE(n)     ((((unsigned long)(n))&0x3FF)<<0)
#define   CPR0_PLLC_TUNE_DECODE(n)     ((((unsigned long)(n))>>0)&0x3FF)
/*--------------------------------------*/
#define CPR0_PLLD                   0x60
#define   CPR0_PLLD_FBDV_MASK          0x1F000000
#define   CPR0_PLLD_FBDV_ENCODE(n)     ((((unsigned long)(n))&0x1F)<<24)
#define   CPR0_PLLD_FBDV_DECODE(n)     ((((((unsigned long)(n))>>24)-1)&0x1F)+1)
#define   CPR0_PLLD_FWDVA_MASK         0x000F0000
#define   CPR0_PLLD_FWDVA_ENCODE(n)    ((((unsigned long)(n))&0x0F)<<16)
#define   CPR0_PLLD_FWDVA_DECODE(n)    ((((((unsigned long)(n))>>16)-1)&0x0F)+1)
#define   CPR0_PLLD_FWDVB_MASK         0x00000700
#define   CPR0_PLLD_FWDVB_ENCODE(n)    ((((unsigned long)(n))&0x07)<<8)
#define   CPR0_PLLD_FWDVB_DECODE(n)    ((((((unsigned long)(n))>>8)-1)&0x07)+1)
#define   CPR0_PLLD_LFBDV_MASK         0x0000003F
#define   CPR0_PLLD_LFBDV_ENCODE(n)    ((((unsigned long)(n))&0x3F)<<0)
#define   CPR0_PLLD_LFBDV_DECODE(n)    ((((((unsigned long)(n))>>0)-1)&0x3F)+1)
/*--------------------------------------*/
#define CPR0_PRIMAD                 0x80
#define   CPR0_PRIMAD_PRADV0_MASK      0x07000000
#define   CPR0_PRIMAD_PRADV0_ENCODE(n) ((((unsigned long)(n))&0x07)<<24)
#define   CPR0_PRIMAD_PRADV0_DECODE(n) ((((((unsigned long)(n))>>24)-1)&0x07)+1)
/*--------------------------------------*/
#define CPR0_PRIMBD                 0xA0
#define   CPR0_PRIMBD_PRBDV0_MASK      0x07000000
#define   CPR0_PRIMBD_PRBDV0_ENCODE(n) ((((unsigned long)(n))&0x07)<<24)
#define   CPR0_PRIMBD_PRBDV0_DECODE(n) ((((((unsigned long)(n))>>24)-1)&0x07)+1)
/*--------------------------------------*/
#if 0
#define CPR0_CPM0_ER                0xB0    /* CPM Enable Register */
#define CPR0_CPM0_FR                0xB1    /* CPM Force Register */
#define CPR0_CPM0_SR                0xB2    /* CPM Status Register */
#define CPR0_CPM0_IIC0               0x80000000    /* Inter-Intergrated Circuit0 */
#define CPR0_CPM0_IIC1               0x40000000    /* Inter-Intergrated Circuit1 */
#define CPR0_CPM0_PCI                0x20000000    /* Peripheral Component Interconnect */
#define CPR0_CPM0_USB1H              0x08000000    /* USB1.1 Host */
#define CPR0_CPM0_FPU                0x04000000    /* PPC440 FPU */
#define CPR0_CPM0_CPU                0x02000000    /* PPC440x5 Processor Core */
#define CPR0_CPM0_DMA                0x01000000    /* Direct Memory Access Controller */
#define CPR0_CPM0_BGO                0x00800000    /* PLB to OPB Bridge */
#define CPR0_CPM0_BGI                0x00400000    /* OPB to PLB Bridge */
#define CPR0_CPM0_EBC                0x00200000    /* External Bus Controller */
#define CPR0_CPM0_NDFC               0x00100000    /* Nand Flash Controller */
#define CPR0_CPM0_MADMAL             0x00080000    /* DDR SDRAM Controller or MADMAL ??? */
#define CPR0_CPM0_DMC                0x00080000    /* DDR SDRAM Controller or MADMAL ??? */
#define CPR0_CPM0_PLB4               0x00040000    /* PLB4 Arbiter */
#define CPR0_CPM0_PLB4x3x            0x00020000    /* PLB4 to PLB3 */
#define CPR0_CPM0_PLB3x4x            0x00010000    /* PLB3 to PLB4 */
#define CPR0_CPM0_PLB3               0x00008000    /* PLB3 Arbiter */
#define CPR0_CPM0_PPM                0x00002000    /* PLB Performance Monitor */
#define CPR0_CPM0_UIC1               0x00001000    /* Universal Interrupt Controller 1 */
#define CPR0_CPM0_GPIO               0x00000800    /* General Purpose IO */
#define CPR0_CPM0_GPT                0x00000400    /* General Purpose Timer */
#define CPR0_CPM0_UART0              0x00000200    /* Universal Asynchronous Rcver/Xmitter 0 */
#define CPR0_CPM0_UART1              0x00000100    /* Universal Asynchronous Rcver/Xmitter 1 */
#define CPR0_CPM0_UIC0               0x00000080    /* Universal Interrupt Controller 0 */
#define CPR0_CPM0_TMRCLK             0x00000040    /* CPU Timer */
#define CPR0_CPM0_EMC0               0x00000020    /* Ethernet 0 */
#define CPR0_CPM0_EMC1               0x00000010    /* Ethernet 1 */
#define CPR0_CPM0_UART2              0x00000008    /* Universal Asynchronous Rcver/Xmitter 2 */
#define CPR0_CPM0_UART3              0x00000004    /* Universal Asynchronous Rcver/Xmitter 3 */
#define CPR0_CPM0_USB2D              0x00000002    /* USB2.0 Device */
#define CPR0_CPM0_USB2H              0x00000001    /* USB2.0 Host */
#endif
/*--------------------------------------*/
#define CPR0_OPBD                   0xC0
#define   CPR0_OPBD_OPBDV0_MASK        0x03000000
#define   CPR0_OPBD_OPBDV0_ENCODE(n)   ((((unsigned long)(n))&0x03)<<24)
#define   CPR0_OPBD_OPBDV0_DECODE(n)   ((((((unsigned long)(n))>>24)-1)&0x03)+1)
/*--------------------------------------*/
#define CPR0_PERD                   0xE0
#define   CPR0_PERD_PERDV0_MASK        0x07000000
#define   CPR0_PERD_PERDV0_ENCODE(n)   ((((unsigned long)(n))&0x07)<<24)
#define   CPR0_PERD_PERDV0_DECODE(n)   ((((((unsigned long)(n))>>24)-1)&0x07)+1)
/*--------------------------------------*/
#define CPR0_MALD                  0x100
#define   CPR0_MALD_MALDV0_MASK        0x03000000
#define   CPR0_MALD_MALDV0_ENCODE(n)   ((((unsigned long)(n))&0x03)<<24)
#define   CPR0_MALD_MALDV0_DECODE(n)   ((((((unsigned long)(n))>>24)-1)&0x03)+1)
/*--------------------------------------*/
#define CPR0_SPCID                 0x120
#define   CPR0_SPCID_SPCIDV0_MASK      0x03000000
#define   CPR0_SPCID_SPCIDV0_ENCODE(n) ((((unsigned long)(n))&0x03)<<24)
#define   CPR0_SPCID_SPCIDV0_DECODE(n) ((((((unsigned long)(n))>>24)-1)&0x03)+1)
/*--------------------------------------*/
#define CPR0_ICFG                  0x140
#define   CPR0_ICFG_RLI_MASK           0x80000000
#define   CPR0_ICFG_RLI_RESETCPR       0x00000000
#define   CPR0_ICFG_RLI_PRESERVECPR    0x80000000
#define   CPR0_ICFG_ICS_MASK           0x00000007
#endif /* defined (CONFIG_440EPX) || defined (CONFIG_440GRX) */

/*-----------------------------------------------------------------------------
| IIC Register Offsets
'----------------------------------------------------------------------------*/
#define IICMDBUF		0x00
#define IICSDBUF		0x02
#define IICLMADR		0x04
#define IICHMADR		0x05
#define IICCNTL			0x06
#define IICMDCNTL		0x07
#define IICSTS			0x08
#define IICEXTSTS		0x09
#define IICLSADR		0x0A
#define IICHSADR		0x0B
#define IICCLKDIV		0x0C
#define IICINTRMSK		0x0D
#define IICXFRCNT		0x0E
#define IICXTCNTLSS		0x0F
#define IICDIRECTCNTL		0x10

/*-----------------------------------------------------------------------------
| UART Register Offsets
'----------------------------------------------------------------------------*/
#define DATA_REG		0x00
#define DL_LSB			0x00
#define DL_MSB			0x01
#define INT_ENABLE		0x01
#define FIFO_CONTROL		0x02
#define LINE_CONTROL		0x03
#define MODEM_CONTROL		0x04
#define LINE_STATUS		0x05
#define MODEM_STATUS		0x06
#define SCRATCH			0x07

/*-----------------------------------------------------------------------------
| PCI Internal Registers et. al. (accessed via plb)
+----------------------------------------------------------------------------*/
#define PCIX0_CFGADR		(CFG_PCI_BASE + 0x0ec00000)
#define PCIX0_CFGDATA		(CFG_PCI_BASE + 0x0ec00004)
#define PCIX0_CFGBASE		(CFG_PCI_BASE + 0x0ec80000)
#define PCIX0_IOBASE		(CFG_PCI_BASE + 0x08000000)

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

/* PCI Local Configuration Registers
   --------------------------------- */
#define PCI_MMIO_LCR_BASE (CFG_PCI_BASE + 0x0f400000)    /* Real => 0x0EF400000 */

/* PCI Master Local Configuration Registers */
#define PCIX0_PMM0LA         (PCI_MMIO_LCR_BASE + 0x00) /* PMM0 Local Address */
#define PCIX0_PMM0MA         (PCI_MMIO_LCR_BASE + 0x04) /* PMM0 Mask/Attribute */
#define PCIX0_PMM0PCILA      (PCI_MMIO_LCR_BASE + 0x08) /* PMM0 PCI Low Address */
#define PCIX0_PMM0PCIHA      (PCI_MMIO_LCR_BASE + 0x0C) /* PMM0 PCI High Address */
#define PCIX0_PMM1LA         (PCI_MMIO_LCR_BASE + 0x10) /* PMM1 Local Address */
#define PCIX0_PMM1MA         (PCI_MMIO_LCR_BASE + 0x14) /* PMM1 Mask/Attribute */
#define PCIX0_PMM1PCILA      (PCI_MMIO_LCR_BASE + 0x18) /* PMM1 PCI Low Address */
#define PCIX0_PMM1PCIHA      (PCI_MMIO_LCR_BASE + 0x1C) /* PMM1 PCI High Address */
#define PCIX0_PMM2LA         (PCI_MMIO_LCR_BASE + 0x20) /* PMM2 Local Address */
#define PCIX0_PMM2MA         (PCI_MMIO_LCR_BASE + 0x24) /* PMM2 Mask/Attribute */
#define PCIX0_PMM2PCILA      (PCI_MMIO_LCR_BASE + 0x28) /* PMM2 PCI Low Address */
#define PCIX0_PMM2PCIHA      (PCI_MMIO_LCR_BASE + 0x2C) /* PMM2 PCI High Address */

/* PCI Target Local Configuration Registers */
#define PCIX0_PTM1MS         (PCI_MMIO_LCR_BASE + 0x30) /* PTM1 Memory Size/Attribute */
#define PCIX0_PTM1LA         (PCI_MMIO_LCR_BASE + 0x34) /* PTM1 Local Addr. Reg */
#define PCIX0_PTM2MS         (PCI_MMIO_LCR_BASE + 0x38) /* PTM2 Memory Size/Attribute */
#define PCIX0_PTM2LA         (PCI_MMIO_LCR_BASE + 0x3C) /* PTM2 Local Addr. Reg */

#else

#define PCIX0_VENDID		(PCIX0_CFGBASE + PCI_VENDOR_ID )
#define PCIX0_DEVID		(PCIX0_CFGBASE + PCI_DEVICE_ID )
#define PCIX0_CMD		(PCIX0_CFGBASE + PCI_COMMAND )
#define PCIX0_STATUS		(PCIX0_CFGBASE + PCI_STATUS )
#define PCIX0_REVID		(PCIX0_CFGBASE + PCI_REVISION_ID )
#define PCIX0_CLS		(PCIX0_CFGBASE + PCI_CLASS_CODE)
#define PCIX0_CACHELS		(PCIX0_CFGBASE + PCI_CACHE_LINE_SIZE )
#define PCIX0_LATTIM		(PCIX0_CFGBASE + PCI_LATENCY_TIMER )
#define PCIX0_HDTYPE		(PCIX0_CFGBASE + PCI_HEADER_TYPE )
#define PCIX0_BIST		(PCIX0_CFGBASE + PCI_BIST )
#define PCIX0_BAR0		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_0 )
#define PCIX0_BAR1		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_1 )
#define PCIX0_BAR2		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_2 )
#define PCIX0_BAR3		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_3 )
#define PCIX0_BAR4		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_4 )
#define PCIX0_BAR5		(PCIX0_CFGBASE + PCI_BASE_ADDRESS_5 )
#define PCIX0_CISPTR		(PCIX0_CFGBASE + PCI_CARDBUS_CIS )
#define PCIX0_SBSYSVID		(PCIX0_CFGBASE + PCI_SUBSYSTEM_VENDOR_ID )
#define PCIX0_SBSYSID		(PCIX0_CFGBASE + PCI_SUBSYSTEM_ID )
#define PCIX0_EROMBA		(PCIX0_CFGBASE + PCI_ROM_ADDRESS )
#define PCIX0_CAP		(PCIX0_CFGBASE + PCI_CAPABILITY_LIST )
#define PCIX0_RES0		(PCIX0_CFGBASE + 0x0035 )
#define PCIX0_RES1		(PCIX0_CFGBASE + 0x0036 )
#define PCIX0_RES2		(PCIX0_CFGBASE + 0x0038 )
#define PCIX0_INTLN		(PCIX0_CFGBASE + PCI_INTERRUPT_LINE )
#define PCIX0_INTPN		(PCIX0_CFGBASE + PCI_INTERRUPT_PIN )
#define PCIX0_MINGNT		(PCIX0_CFGBASE + PCI_MIN_GNT )
#define PCIX0_MAXLTNCY		(PCIX0_CFGBASE + PCI_MAX_LAT )

#define PCIX0_BRDGOPT1		(PCIX0_CFGBASE + 0x0040)
#define PCIX0_BRDGOPT2		(PCIX0_CFGBASE + 0x0044)

#define PCIX0_POM0LAL		(PCIX0_CFGBASE + 0x0068)
#define PCIX0_POM0LAH		(PCIX0_CFGBASE + 0x006c)
#define PCIX0_POM0SA		(PCIX0_CFGBASE + 0x0070)
#define PCIX0_POM0PCIAL		(PCIX0_CFGBASE + 0x0074)
#define PCIX0_POM0PCIAH		(PCIX0_CFGBASE + 0x0078)
#define PCIX0_POM1LAL		(PCIX0_CFGBASE + 0x007c)
#define PCIX0_POM1LAH		(PCIX0_CFGBASE + 0x0080)
#define PCIX0_POM1SA		(PCIX0_CFGBASE + 0x0084)
#define PCIX0_POM1PCIAL		(PCIX0_CFGBASE + 0x0088)
#define PCIX0_POM1PCIAH		(PCIX0_CFGBASE + 0x008c)
#define PCIX0_POM2SA		(PCIX0_CFGBASE + 0x0090)

#define PCIX0_PIM0SA		(PCIX0_CFGBASE + 0x0098)
#define PCIX0_PIM0LAL		(PCIX0_CFGBASE + 0x009c)
#define PCIX0_PIM0LAH		(PCIX0_CFGBASE + 0x00a0)
#define PCIX0_PIM1SA		(PCIX0_CFGBASE + 0x00a4)
#define PCIX0_PIM1LAL		(PCIX0_CFGBASE + 0x00a8)
#define PCIX0_PIM1LAH		(PCIX0_CFGBASE + 0x00ac)
#define PCIX0_PIM2SA		(PCIX0_CFGBASE + 0x00b0)
#define PCIX0_PIM2LAL		(PCIX0_CFGBASE + 0x00b4)
#define PCIX0_PIM2LAH		(PCIX0_CFGBASE + 0x00b8)

#define PCIX0_STS		(PCIX0_CFGBASE + 0x00e0)

#endif /* !defined(CONFIG_440EP) !defined(CONFIG_440GR) */

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

/* USB2.0 Device */
#define USB2D0_BASE         CFG_USB2D0_BASE

#define USB2D0_INTRIN       (USB2D0_BASE + 0x00000000)

#define USB2D0_INTRIN       (USB2D0_BASE + 0x00000000) /* Interrupt register for Endpoint 0 plus IN Endpoints 1 to 3 */
#define USB2D0_POWER        (USB2D0_BASE + 0x00000000) /* Power management register */
#define USB2D0_FADDR        (USB2D0_BASE + 0x00000000) /* Function address register */
#define USB2D0_INTRINE      (USB2D0_BASE + 0x00000000) /* Interrupt enable register for USB2D0_INTRIN */
#define USB2D0_INTROUT      (USB2D0_BASE + 0x00000000) /* Interrupt register for OUT Endpoints 1 to 3 */
#define USB2D0_INTRUSBE     (USB2D0_BASE + 0x00000000) /* Interrupt enable register for USB2D0_INTRUSB */
#define USB2D0_INTRUSB      (USB2D0_BASE + 0x00000000) /* Interrupt register for common USB interrupts */
#define USB2D0_INTROUTE     (USB2D0_BASE + 0x00000000) /* Interrupt enable register for IntrOut */
#define USB2D0_TSTMODE      (USB2D0_BASE + 0x00000000) /* Enables the USB 2.0 test modes */
#define USB2D0_INDEX        (USB2D0_BASE + 0x00000000) /* Index register for selecting the Endpoint status/control registers */
#define USB2D0_FRAME        (USB2D0_BASE + 0x00000000) /* Frame number */
#define USB2D0_INCSR0       (USB2D0_BASE + 0x00000000) /* Control Status register for Endpoint 0. (Index register set to select Endpoint 0) */
#define USB2D0_INCSR        (USB2D0_BASE + 0x00000000) /* Control Status register for IN Endpoint. (Index register set to select Endpoints 13) */
#define USB2D0_INMAXP       (USB2D0_BASE + 0x00000000) /* Maximum packet size for IN Endpoint. (Index register set to select Endpoints 13) */
#define USB2D0_OUTCSR       (USB2D0_BASE + 0x00000000) /* Control Status register for OUT Endpoint. (Index register set to select Endpoints 13) */
#define USB2D0_OUTMAXP      (USB2D0_BASE + 0x00000000) /* Maximum packet size for OUT Endpoint. (Index register set to select Endpoints 13) */
#define USB2D0_OUTCOUNT0    (USB2D0_BASE + 0x00000000) /* Number of received bytes in Endpoint 0 FIFO. (Index register set to select Endpoint 0) */
#define USB2D0_OUTCOUNT     (USB2D0_BASE + 0x00000000) /* Number of bytes in OUT Endpoint FIFO. (Index register set to select Endpoints 13) */
#endif

/******************************************************************************
 * GPIO macro register defines
 ******************************************************************************/
#if defined(CONFIG_440GP) || defined(CONFIG_440GX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
#define GPIO0_BASE             (CFG_PERIPHERAL_BASE+0x00000700)

#define GPIO0_OR               (GPIO0_BASE+0x0)
#define GPIO0_TCR              (GPIO0_BASE+0x4)
#define GPIO0_ODR              (GPIO0_BASE+0x18)
#define GPIO0_IR               (GPIO0_BASE+0x1C)
#endif /* CONFIG_440GP */

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define GPIO0_BASE             (CFG_PERIPHERAL_BASE+0x00000B00)
#define GPIO1_BASE             (CFG_PERIPHERAL_BASE+0x00000C00)

#define GPIO0_OR               (GPIO0_BASE+0x0)
#define GPIO0_TCR              (GPIO0_BASE+0x4)
#define GPIO0_OSRL             (GPIO0_BASE+0x8)
#define GPIO0_OSRH             (GPIO0_BASE+0xC)
#define GPIO0_TSRL             (GPIO0_BASE+0x10)
#define GPIO0_TSRH             (GPIO0_BASE+0x14)
#define GPIO0_ODR              (GPIO0_BASE+0x18)
#define GPIO0_IR               (GPIO0_BASE+0x1C)
#define GPIO0_RR1              (GPIO0_BASE+0x20)
#define GPIO0_RR2              (GPIO0_BASE+0x24)
#define GPIO0_RR3	       (GPIO0_BASE+0x28)
#define GPIO0_ISR1L            (GPIO0_BASE+0x30)
#define GPIO0_ISR1H            (GPIO0_BASE+0x34)
#define GPIO0_ISR2L            (GPIO0_BASE+0x38)
#define GPIO0_ISR2H            (GPIO0_BASE+0x3C)
#define GPIO0_ISR3L            (GPIO0_BASE+0x40)
#define GPIO0_ISR3H            (GPIO0_BASE+0x44)

#define GPIO1_OR               (GPIO1_BASE+0x0)
#define GPIO1_TCR              (GPIO1_BASE+0x4)
#define GPIO1_OSRL             (GPIO1_BASE+0x8)
#define GPIO1_OSRH             (GPIO1_BASE+0xC)
#define GPIO1_TSRL             (GPIO1_BASE+0x10)
#define GPIO1_TSRH             (GPIO1_BASE+0x14)
#define GPIO1_ODR              (GPIO1_BASE+0x18)
#define GPIO1_IR               (GPIO1_BASE+0x1C)
#define GPIO1_RR1              (GPIO1_BASE+0x20)
#define GPIO1_RR2              (GPIO1_BASE+0x24)
#define GPIO1_RR3              (GPIO1_BASE+0x28)
#define GPIO1_ISR1L            (GPIO1_BASE+0x30)
#define GPIO1_ISR1H            (GPIO1_BASE+0x34)
#define GPIO1_ISR2L            (GPIO1_BASE+0x38)
#define GPIO1_ISR2H            (GPIO1_BASE+0x3C)
#define GPIO1_ISR3L            (GPIO1_BASE+0x40)
#define GPIO1_ISR3H            (GPIO1_BASE+0x44)
#endif

#ifndef __ASSEMBLY__

static inline u32 get_mcsr(void)
{
	u32 val;

	asm volatile("mfspr %0, 0x23c" : "=r" (val) :);
	return val;
}

static inline void set_mcsr(u32 val)
{
	asm volatile("mtspr 0x23c, %0" : "=r" (val) :);
}

#endif	/* _ASMLANGUAGE */

#endif	/* __PPC440_H__ */
