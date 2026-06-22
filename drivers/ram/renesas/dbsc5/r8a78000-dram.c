// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Renesas Electronics Corp.
 *
 * Portions Copyright (C) 2026 Synopsys, Inc. Used with permission. All rights reserved.
 */

#include <asm/io.h>
#include <r8a78000-dbsc5.h>
#include <dm.h>
#include <errno.h>
#include <hang.h>
#include <ram.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include "r8a78000-dbsc5.h"
#include "r8a78000-dram.h"

#define DBSC5_PHYNUM_CNT		8
#define DBSC5_DBSC_CNT			8
#define DBSC5_PLL3_CNT			4

/* Minimum value table for JS1 configuration table that can be taken */
#define JS1_USABLEC_SPEC_LO		5
/* Maximum value table for JS1 configuration table that can be taken */
#define JS1_USABLEC_SPEC_HI		14
/* The number of JS1 setting table */
#define JS1_FREQ_TBL_NUM		15
/* Macro to set the value of MR1 */
#define JS1_MR1(f)			((f) << 4) /* CK mode = 0B */
/* Macro to set the value of MR2 */
#define JS1_MR2(f)			(((f) << 4) | (f))

#define JS2_tSR		0	/* Element for self refresh */
#define JS2_tXP		1	/* Exit power-down mode to first valid command */
#define JS2_tRCD	2	/* Active to read or write delay */
#define JS2_tRPpb	3	/* Minimum Row Precharge Delay Time */
#define JS2_tRPab	4	/* Minimum Row Precharge Delay Time */
#define JS2_tRAS	5	/* ACTIVE-to-PRECHARGE command */
#define JS2_tWTR_S	6	/* Internal WRITE-to-READ command delay */
#define JS2_tWTR_L	7	/* Internal WRITE-to-READ command delay */
#define JS2_tRRD	8	/* Active bank a to active bank b command */
#define JS2_tPPD	9	/* Precharge Power Down */
#define JS2_tFAW	10	/* Four bank ACT window */
#define JS2_tMRR	11	/* Mode Register Read */
#define JS2_tMRW	12	/* Mode Register Write */
#define JS2_tMRD	13	/* LOAD MODE REGISTER command cycle time */
#define JS2_tZQCALns	14	/* ZQ Calibration */
#define JS2_tZQLAT	15	/* ZQ Latency */
#define JS2_tODTon_min	16	/* Minimum time on die termination */
#define JS2_tPDN_DSM	17	/* Recommended minimum time for Deep Sleep Mode duration */
#define JS2_tXSR_DSM	18	/* Required time to be fully re-powered up from Deep Sleep Mode */
#define JS2_tXDSM_XP	19	/* Delay from Deep Sleep Mode Exit to Power-Down Exit */
#define JS2_tWCK2DQI_HF	20	/* Setting value of DQ to WCK input offset */
#define JS2_tWCK2DQO_HF	21	/* Setting value of WCK to DQ output offset */
#define JS2_tWCK2DQI_LF	22	/* Setting value of DQ to WCK input offset */
#define JS2_tWCK2DQO_LF	23	/* Setting value of WCK to DQ output offset */
#define JS2_tOSCODQI	24	/* Delay time from Stop WCK2DQI Interval Oscillator command to Mode Register Readout */
#define JS2_tOSCODQO	25
#define JS2_tDQ72DQns	26	/* Reception time to change the value fof REF(CA) for Command Bus Training Mode2 */
#define JS2_tCAENTns	27	/* Reception time to change the value fof REF(CA) for Command Bus Training Mode1 */
#define JS2_tCSCAL	28	/* Minimum CA Low Duration time */
#define JS2_tWCKSTOP	29
#define JS2_tpbR2act	30
#define JS2_TBLCNT	31	/* The number of table */

#define JS2_tRCpb	JS2_TBLCNT		/* ACTIVATE-to-ACTIVATE command period with per bank precharge */
#define JS2_tRCab	(JS2_TBLCNT + 1)	/* ACTIVATE-to-ACTIVATE command period with all bank precharge */
#define JS2_tRFCab	(JS2_TBLCNT + 2)	/* Refresh Cycle Time with All Banks */
#define JS2_tRBTP	(JS2_TBLCNT + 3)	/* READ Burst end to PRECHARGE command delay */
#define JS2_tXSR	(JS2_TBLCNT + 4)	/* Exit Self Refresh to Valid commands */
#define JS2_tPDN	(JS2_TBLCNT + 5)
#define JS2_tWLWCKOFF	(JS2_TBLCNT + 6)
#define JS2_tRFCpb	(JS2_TBLCNT + 7)
#define JS2_tpbR2pbR	(JS2_TBLCNT + 8)
#define JS2_tRFMab	(JS2_TBLCNT + 9)
#define JS2_tRFMpb	(JS2_TBLCNT + 10)
#define JS2_CNT		(JS2_TBLCNT + 11)

#define JS2_DERATE			0
#define DBSC_REFINT			1920
#define PERIODIC_TRAINING_INTERVAL	20000

#define DBMEMCONF_REG(d3, row, BG, bank, col, dw)	\
	(((d3) << 30) | ((row) << 24) | ((BG) << 20) |	\
	((bank) << 16) | ((col) << 8) | (dw))

#define DBMEMCONF_REGD(density)								\
	(DBMEMCONF_REG(((density) % 2), (((density) + 1) / 2) + (28 - 2 - 2 - 10 - 1),	\
	2, 2, 10, 1))

struct jedec_spec1 {
	u32 fx3;
	u32 RLset0;
	u32 RLset1;
	u32 RLset2;
	u32 WLsetA;
	u32 WLsetB;
	u32 nWR;
	u32 nRBTP;
	u32 ODTLon;
	u32 MR1;
	u32 MR2;
	u32 WCKENLR0;
	u32 WCKENLR1;
	u32 WCKENLW;
	u32 WCKENLF;
	u32 WCKPRESTA;
	u32 WCKPRETGLR;
	u32 tRRD;
	u32 tFAW;
};

static const struct jedec_spec1 js1[JS1_FREQ_TBL_NUM] = {
	/* fx3,RL0,RL1,RL2,WLA.WLB.nWR.nRBTP,ODTLon */
	{   800,  3,  3,  3,  2,  2,  3, 0, 1, JS1_MR1(0),  JS1_MR2(0),  0,  0, 0, 0, 1,  3, 3750, 15000 }, /*  533.333Mbps */
	{  1600,  4,  4,  4,  2,  3,  5, 0, 1, JS1_MR1(1),  JS1_MR2(1),  0,  0, 0, 0, 1,  4, 3750, 15000 }, /* 1066.666Mbps */
	{  2400,  5,  5,  6,  3,  4,  7, 0, 2, JS1_MR1(2),  JS1_MR2(2),  1,  1, 1, 1, 1,  4, 3750, 15000 }, /* 1600.000Mbps */
	{  3200,  6,  7,  7,  4,  5, 10, 0, 2, JS1_MR1(3),  JS1_MR2(3),  1,  2, 1, 1, 2,  4, 3750, 15000 }, /* 2133.333Mbps */
	{  4000,  8,  8,  9,  4,  7, 12, 1, 2, JS1_MR1(4),  JS1_MR2(4),  2,  2, 1, 1, 2,  5, 3750, 15000 }, /* 2666.666Mbps */
	{  4800,  9, 10, 10,  5,  8, 14, 1, 3, JS1_MR1(5),  JS1_MR2(5),  3,  4, 2, 1, 2,  5, 3750, 15000 }, /* 3200.000Mbps */
	{  5600, 10, 11, 12,  6,  9, 16, 2, 4, JS1_MR1(6),  JS1_MR2(6),  3,  4, 2, 1, 3,  5, 3750, 15000 }, /* 3733.333Mbps */
	{  6400, 12, 13, 14,  6, 11, 19, 2, 3, JS1_MR1(7),  JS1_MR2(7),  4,  5, 2, 1, 3,  6, 3750, 15000 }, /* 4266.666Mbps */
	{  7200, 13, 14, 15,  7, 12, 21, 3, 4, JS1_MR1(8),  JS1_MR2(8),  5,  6, 3, 2, 3,  6, 3750, 15000 }, /* 4800.000Mbps */
	{  8250, 15, 16, 17,  8, 14, 24, 4, 5, JS1_MR1(9),  JS1_MR2(9),  6,  7, 3, 2, 4,  6, 3750, 15000 }, /* 5500.000Mbps */
	{  9000, 16, 17, 19,  9, 15, 26, 4, 6, JS1_MR1(10), JS1_MR2(10), 6,  7, 4, 2, 4,  7, 3750, 15000 }, /* 6000.000Mbps */
	{  9600, 17, 18, 20,  9, 16, 28, 4, 6, JS1_MR1(11), JS1_MR2(11), 7,  8, 4, 2, 4,  7, 3750, 15000 }, /* 6400.000Mbps */
	{ 11250, 20, 22, 24, 11, 19, 32, 6, 7, JS1_MR1(12), JS1_MR2(12), 7,  9, 5, 3, 5,  9, 3750, 15000 }, /* 7500.000Mbps */
	{ 12800, 23, 25, 26, 12, 22, 37, 6, 8, JS1_MR1(13), JS1_MR2(13), 8, 10, 5, 3, 6, 10, 3750, 15000 }, /* 8533.333Mbps */
	{ 14400, 25, 28, 29, 14, 24, 41, 7, 9, JS1_MR1(14), JS1_MR2(14), 8, 11, 6, 3, 7, 11, 3330, 13320 }  /* 9600.000Mbps */
};

struct jedec_spec2 {
	u32 ps;
	u32 cyc;
};

static const struct jedec_spec2 jedec_spec2[2][JS2_TBLCNT] = {
	{
		{ 15000, 2 },	/* tSR */
		{ 7000, 3 },	/* tXP */
		{ 18000, 2 },	/* tRCD */
		{ 18000, 2 },	/* tRPpb */
		{ 21000, 2 },	/* tRPab */
		{ 42000, 3 },	/* tRAS */
		{ 6250, 4 },	/* tWTR_S */
		{ 12000, 4 },	/* tWTR_L */
		{ 0, 2 },	/* tRRD */
		{ 0, 2 },	/* tPPD */
		{ 0, 0 },	/* tFAW */
		{ 0, 4 },	/* tMRR */
		{ 10000, 5 },	/* tMRW */
		{ 14000, 5 },	/* tMRD */
		{ 1500, 0 },	/* tZQCALns */
		{ 30000, 4 },	/* tZQLAT */
		{ 1500, 0 },	/* tODTon_min */
		{ 4000, 0 },	/* tPDN_DSMus */
		{ 200, 0 },	/* tXSR_DSMus */
		{ 190, 0 },	/* tXDSM_XPus */
		{ 700, 0 },	/* tWCK2DQI_HF */
		{ 1600, 0 },	/* tWCK2DQO_HF */
		{ 900, 0 },	/* tWCK2DQI_LF */
		{ 1900, 0 },	/* tWCK2DQO_LF */
		{ 40000, 8 },	/* tOSCODQI */
		{ 40000, 8 },	/* tOSCODQO */
		{ 125, 0 },	/* tDQ72DQns */
		{ 250, 0 },	/* tCAENTns */
		{ 1750, 0 },	/* tCSCAL */
		{ 6000, 2 },	/* tWCKSTOP */
		{ 7500, 0 },	/* tpbR2act */
	}, {
		{ 15000, 2 },	/* tSR */
		{ 7000, 3 },	/* tXP */
		{ 19875, 2 },	/* tRCD */
		{ 19875, 2 },	/* tRPpb */
		{ 22875, 2 },	/* tRPab */
		{ 43875, 3 },	/* tRAS */
		{ 6250, 4 },	/* tWTR_S */
		{ 12000, 4 },	/* tWTR_L */
		{ 0, 2 },	/* tRRD */
		{ 0, 2 },	/* tPPD */
		{ 0, 0 },	/* tFAW */
		{ 0, 4 },	/* tMRR */
		{ 10000, 5 },	/* tMRW */
		{ 14000, 5 },	/* tMRD */
		{ 1500, 0 },	/* tZQCALns */
		{ 30000, 4 },	/* tZQLAT */
		{ 1500, 0 },	/* tODTon_min */
		{ 4000, 0 },	/* tPDN_DSMus */
		{ 200, 0 },	/* tXSR_DSMus */
		{ 190, 0 },	/* tXDSM_XPus */
		{ 715, 0 },	/* tWCK2DQI_HF */
		{ 1635, 0 },	/* tWCK2DQO_HF */
		{ 920, 0 },	/* tWCK2DQI_LF */
		{ 1940, 0 },	/* tWCK2DQO_LF */
		{ 40000, 8 },	/* tOSCODQI */
		{ 40000, 8 },	/* tOSCODQO */
		{ 125, 0 },	/* tDQ72DQns */
		{ 250, 0 },	/* tCAENTns */
		{ 1750, 0 },	/* tCSCAL */
		{ 6000, 2 },	/* tWCKSTOP */
		{ 7500, 0 },	/* tpbR2act */
	}
};

static const u32 jedec_spec2_tRFC_ab[9] = {
	/* 2Gb, 3Gb, 4Gb, 6Gb, 8Gb, 12Gb, 16Gb, 24Gb, 32Gb */
	 130, 180, 180, 210, 210, 280, 280, 380, 380
};

static const u32 jedec_spec2_tRFC_pb[9] = {
	/* 2Gb, 3Gb, 4Gb, 6Gb, 8Gb, 12Gb, 16Gb, 24Gb, 32Gb */
	60, 90, 90, 120, 120, 140, 140, 190, 190
};

static const u32 jedec_spec2_tpbR2pbR[9] = {
	/* 2Gb, 3Gb, 4Gb, 6Gb, 8Gb, 12Gb, 16Gb, 24Gb, 32Gb */
	60, 90, 90, 90, 90, 90, 90, 90, 90
};

static const u32 jedec_spec2_tRFM_ab[9] = {
	/* 2Gb, 3Gb, 4Gb, 6Gb, 8Gb, 12Gb, 16Gb, 24Gb, 32Gb */
	0, 0, 0, 0, 210, 280, 280, 380, 380
};

static const u32 jedec_spec2_tRFM_pb[9] = {
	/* 2Gb, 3Gb, 4Gb, 6Gb, 8Gb, 12Gb, 16Gb, 24Gb, 32Gb */
	0, 0, 0, 0, 170, 190, 190, 260, 260
};

/* System registers */
#define SYSSS_TOP_BASE				0xc6480000
#define SYSSS_ZB3CKCR0				(SYSSS_TOP_BASE + 0x10a0)
#define SYSSS_ZB3CKCR1				(SYSSS_TOP_BASE + 0x10a4)
#define SYSSS_ZB3CKCR_KICK			BIT(31)
#define SYSSS_ZB3CKCR_PHYx(ch)			((0x7 * ((ch) & 0x1)) + (0x8 * ((ch) & 0x2)))
#define SYSSS_PLL1_1_CR0			(SYSSS_TOP_BASE + 0x1114)
#define SYSSS_PLL1_1DCR				(SYSSS_TOP_BASE + 0x127c)
#define SYSSS_PLL3_x_CR0(ch)			(SYSSS_TOP_BASE + 0x1180 + ((ch) * 0xc))
#define SYSSS_PLL3_x_CR1(ch)			(SYSSS_TOP_BASE + 0x1184 + ((ch) * 0xc))
#define SYSSS_PLL3_x_CR2(ch)			(SYSSS_TOP_BASE + 0x1188 + ((ch) * 0xc))
#define SYSSS_PLL3_x_DCR(ch)			(SYSSS_TOP_BASE + 0x12c4 + ((ch) * 0x8))
#define SYSSS_PLL6_CR0				(SYSSS_TOP_BASE + 0x11c8)
#define SYSSS_PLL3_xSCR(ch)			(SYSSS_TOP_BASE + 0x12c0 + ((ch) * 0x8))
#define SYSSS_CLKTOPPKCPROT0			(SYSSS_TOP_BASE + 0x1370)
#define CLK_DIV(a, diva, b, divb)		(((a) * (divb)) / ((b) * (diva)))

/* MDLC registers */
#define MODULE_CONTROL_DDRx_BASE(ch)		(0xe8000000 + ((ch) * 0x80000))
#define MODULE_CONTROL_DDRFI_BASE		0xc6480000
#define MODULE_CONTROL_MDLC12PKCPROT1		(MODULE_CONTROL_DDRFI_BASE + 0xCF4)
#define MODULE_CONTROL_MDLC0xPKCPROT1(ch)	(MODULE_CONTROL_DDRx_BASE(ch) + 0xCF4)
#define MODULE_CONTROL_MDLC15MSRES8		(MODULE_CONTROL_DDRFI_BASE + 0x920)
#define MODULE_CONTROL_MDLC0xMSRES1(ch)		(MODULE_CONTROL_DDRx_BASE(ch) + 0x904)
#define MODULE_CONTROL_MDLC0xMSRES3(ch)		(MODULE_CONTROL_DDRx_BASE(ch) + 0x90C)
#define MODULE_CONTROL_MDLC15MSRESS8		(MODULE_CONTROL_DDRFI_BASE + 0x980)
#define MODULE_CONTROL_MDLC0xMSRESS1(ch)	(MODULE_CONTROL_DDRx_BASE(ch) + 0x964)
#define MODULE_CONTROL_MDLC0xMSRESS3(ch)	(MODULE_CONTROL_DDRx_BASE(ch) + 0x96C)
#define MODULE_STANDBY				0
#define MODULE_RUN				3

/* DBSC5 registers */
#define DBSC_A_BASE				0xE9800000
#define DBSC_D_BASE				0xE9900000

#define DBSC_DBSYSCONF0				(DBSC_A_BASE + 0x0)
#define DBSC_DBSYSCONF1				(DBSC_D_BASE + 0x0)
#define DBSC_DBSYSCONF1A			(DBSC_A_BASE + 0x4)
#define DBSC_DBSYSCONF2				(DBSC_D_BASE + 0x4)
#define DBSC_DBPHYCONF0				(DBSC_D_BASE + 0x8)
#define DBSC_DBSYSCONF2A			(DBSC_A_BASE + 0x8)
#define DBSC_DBMEMKIND				(DBSC_D_BASE + 0x20)
#define DBSC_DBMEMKINDA				(DBSC_A_BASE + 0x20)
#define DBSC_DBMEMCONF(ch, cs)			\
	(DBSC_D_BASE + 0x30 + (0x2000 * ((ch) & 0xE)) + (0x10 * ((ch) & 0x1)) + (0x4 * (cs)))
#define DBSC_DBMEMCONFA(ch, cs)			\
	(DBSC_A_BASE + 0x30 + (0x4000 * ((ch) & 0xE)) + (0x10 * ((ch) & 0x1)) + (0x4 * (cs)))
#define DBSC_DBSYSCNT0				(DBSC_D_BASE + 0x100)
#define DBSC_DBSYSCNT0A				(DBSC_A_BASE + 0x100)
#define DBSC_DBSYSCNT1A				(DBSC_A_BASE + 0x104)
#define DBSC_DBACEN				(DBSC_A_BASE + 0x200)
#define DBSC_DBRFEN				(DBSC_D_BASE + 0x204)
#define DBSC_DBCMD				(DBSC_D_BASE + 0x208)
#define DBSC_DBWAIT				(DBSC_D_BASE + 0x210)
#define DBSC_DBTR(x)				(DBSC_D_BASE + 0x300 + (0x4 * (x)))
#define DBSC_DBBL				(DBSC_D_BASE + 0x400)
#define DBSC_DBBLA				(DBSC_A_BASE + 0x400)
#define DBSC_DBRFCNF1				(DBSC_D_BASE + 0x414)
#define DBSC_DBRFCNF2				(DBSC_D_BASE + 0x418)
#define DBSC_DBCALCNF				(DBSC_D_BASE + 0x424)
#define DBSC_DBSNOOPC				(DBSC_D_BASE + 0x42C)
#define DBSC_DBRNK(x)				(DBSC_D_BASE + 0x430 + (0x4 * (x)))
#define DBSC_DBDBICNT				(DBSC_D_BASE + 0x518)
#define DBSC_DBDFIPMSTRCNF			(DBSC_D_BASE + 0x520)
#define DBSC_DBDFICUPDCNF			(DBSC_D_BASE + 0x540)
#define DBSC_DBDFISTAT(ch)			\
	(DBSC_D_BASE + 0x600 + (0x2000 * ((ch) & 0xE)) + (0x40 * ((ch) & 0x1)))
#define DBSC_DBDFICNT(ch)			\
	(DBSC_D_BASE + 0x604 + (0x2000 * ((ch) & 0xE)) + (0x40 * ((ch) & 0x1)))
#define DBSC_DBPDSTAT01(ch)			(DBSC_D_BASE + 0x634 + (0x4000 * (ch)))
#define DBSC_DBBUS0CNF1				(DBSC_A_BASE + 0x804)
#define DBSC_DBBCAMDIS				(DBSC_A_BASE + 0x9FC)
#define DBSC_DBSCHRW(x)				(DBSC_A_BASE + 0x1020 + (0x4 * (x)))
#define DBSC_DBSCHTR0				(DBSC_A_BASE + 0x1030)
#define DBSC_DBSCHFCTST(x)			(DBSC_A_BASE + 0x1040 + (0x4 * (x)))

/* DDR PHY registers */
#define DDR_PHY_BASE(ch)			(0xA0000000 + (0x1000000 * (ch)))
#define DDR_PHY_DMA_TRANS_BASE(ch)		(0xE0000000 + (0x1000000 * (ch)))

#define PHY_HMAC_BASE(n)			((n) * 0x1000)
#define PHY_HMAC_TXSLEWAC(n)			(PHY_HMAC_BASE(n) + 0x6d)
#define PHY_HMAC_TXIMPEDANCEAC(n)		(PHY_HMAC_BASE(n) + 0x70)
#define PHY_HMAC_ODTIMPEDANCEAC(n)		(PHY_HMAC_BASE(n) + 0x79)

#define PHY_DBYTE_BASE(n)			(0x10000 + ((n) * 0x1000))
#define PHY_DBYTE_ENABLEWRITELINKECC(n)		(PHY_DBYTE_BASE(n) + 0x01)
#define PHY_DBYTE_DQ0LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x80)
#define PHY_DBYTE_DQ1LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x81)
#define PHY_DBYTE_DQ2LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x82)
#define PHY_DBYTE_DQ3LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x83)
#define PHY_DBYTE_DQ4LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x84)
#define PHY_DBYTE_DQ5LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x85)
#define PHY_DBYTE_DQ6LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x86)
#define PHY_DBYTE_DQ7LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x87)
#define PHY_DBYTE_DQ8LNSEL(n)			(PHY_DBYTE_BASE(n) + 0x88)
#define PHY_DBYTE_PPTCTLSTATIC(n)		(PHY_DBYTE_BASE(n) + 0xa3)

#define PHY_AC_BASE(n)				(0x30000 + ((n) * 0x1000))
#define PHY_AC_MAPCA0TODFI(n)			(PHY_AC_BASE(n) + 0x90)
#define PHY_AC_MAPCA1TODFI(n)			(PHY_AC_BASE(n) + 0x91)
#define PHY_AC_MAPCA2TODFI(n)			(PHY_AC_BASE(n) + 0x92)
#define PHY_AC_MAPCA3TODFI(n)			(PHY_AC_BASE(n) + 0x93)
#define PHY_AC_MAPCA4TODFI(n)			(PHY_AC_BASE(n) + 0x94)
#define PHY_AC_MAPCA5TODFI(n)			(PHY_AC_BASE(n) + 0x95)
#define PHY_AC_MAPCA6TODFI(n)			(PHY_AC_BASE(n) + 0x96)
#define PHY_AC_ACLNDISABLE(n)			(PHY_AC_BASE(n) + 0xac)
#define PHY_AC_DFICLKACLNDIS(n)			(PHY_AC_BASE(n) + 0xad)
#define PHY_AC_PCLKACLNDIS(n)			(PHY_AC_BASE(n) + 0xae)

#define PHY_APB_BASE				0x58000
#define PHY_APB_DRAMFREQ			(PHY_APB_BASE + 0x01)
#define PHY_APB_DCAOPTS				(PHY_APB_BASE + 0x02)
#define PHY_APB_SEQUENCECTRL			(PHY_APB_BASE + 0x04)
#define PHY_APB_HDTCTRL				(PHY_APB_BASE + 0x04)
#define PHY_APB_RXDFEOPT			(PHY_APB_BASE + 0x07)
#define PHY_APB_CSPRESENTCHA			(PHY_APB_BASE + 0x09)
#define PHY_APB_CSPRESENTCHB			(PHY_APB_BASE + 0x10)
#define PHY_APB_MR1_A0				(PHY_APB_BASE + 0x16)
#define PHY_APB_MR1_A1				(PHY_APB_BASE + 0x16)
#define PHY_APB_MR1_B0				(PHY_APB_BASE + 0x17)
#define PHY_APB_MR1_B1				(PHY_APB_BASE + 0x17)
#define PHY_APB_MR2_A0				(PHY_APB_BASE + 0x17)
#define PHY_APB_MR2_A1				(PHY_APB_BASE + 0x17)
#define PHY_APB_MR2_B0				(PHY_APB_BASE + 0x18)
#define PHY_APB_MR2_B1				(PHY_APB_BASE + 0x18)
#define PHY_APB_MR3_A0				(PHY_APB_BASE + 0x18)
#define PHY_APB_MR3_A1				(PHY_APB_BASE + 0x18)
#define PHY_APB_MR3_B0				(PHY_APB_BASE + 0x19)
#define PHY_APB_MR3_B1				(PHY_APB_BASE + 0x19)
#define PHY_APB_MR11_A0				(PHY_APB_BASE + 0x1A)
#define PHY_APB_MR11_A1				(PHY_APB_BASE + 0x1A)
#define PHY_APB_MR11_B0				(PHY_APB_BASE + 0x1B)
#define PHY_APB_MR11_B1				(PHY_APB_BASE + 0x1B)
#define PHY_APB_MR12_A0				(PHY_APB_BASE + 0x1B)
#define PHY_APB_MR12_A1				(PHY_APB_BASE + 0x1B)
#define PHY_APB_MR12_B0				(PHY_APB_BASE + 0x1C)
#define PHY_APB_MR12_B1				(PHY_APB_BASE + 0x1C)
#define PHY_APB_MR22_A0				(PHY_APB_BASE + 0x25)
#define PHY_APB_MR22_A1				(PHY_APB_BASE + 0x25)
#define PHY_APB_MR22_B0				(PHY_APB_BASE + 0x26)
#define PHY_APB_MR22_B1				(PHY_APB_BASE + 0x26)
#define PHY_APB_MR24_A0				(PHY_APB_BASE + 0x26)
#define PHY_APB_MR24_A1				(PHY_APB_BASE + 0x26)
#define PHY_APB_MR24_B0				(PHY_APB_BASE + 0x27)
#define PHY_APB_MR24_B1				(PHY_APB_BASE + 0x27)
#define PHY_APB_MR41_A0				(PHY_APB_BASE + 0x32)
#define PHY_APB_MR41_A1				(PHY_APB_BASE + 0x32)
#define PHY_APB_MR41_B0				(PHY_APB_BASE + 0x33)
#define PHY_APB_MR41_B1				(PHY_APB_BASE + 0x33)
#define PHY_APB_MR58_A0				(PHY_APB_BASE + 0x34)
#define PHY_APB_MR58_A1				(PHY_APB_BASE + 0x34)
#define PHY_APB_MR58_B0				(PHY_APB_BASE + 0x35)
#define PHY_APB_MR58_B1				(PHY_APB_BASE + 0x35)
#define PHY_APB_TXDFETRAINOPT			(PHY_APB_BASE + 0x3E)
#define PHY_APB_UPPERLOWERBYTE			(PHY_APB_BASE + 0x48)
#define PHY_APB_ALT_RL				(PHY_APB_BASE + 0x48)
#define PHY_APB_MAIN_RL				(PHY_APB_BASE + 0x49)
#define PHY_APB_RXVREFSTARTPAT			(PHY_APB_BASE + 0x4E)
#define PHY_APB_RXVREFSTARTPRBS			(PHY_APB_BASE + 0x4E)
#define PHY_APB_RXVREFENDPAT			(PHY_APB_BASE + 0x4F)
#define PHY_APB_RXVREFENDPRBS			(PHY_APB_BASE + 0x4F)
#define PHY_APB_RXVREFSTEPPAT			(PHY_APB_BASE + 0x50)
#define PHY_APB_TXVREFSTART			(PHY_APB_BASE + 0x50)
#define PHY_APB_TXVREFEND			(PHY_APB_BASE + 0x50)
#define PHY_APB_TXVREFSTEP			(PHY_APB_BASE + 0x51)
#define PHY_APB_RXVREFSTEPPRBS			(PHY_APB_BASE + 0x51)
#define PHY_APB_RXDFEBITTIMECONTROL		(PHY_APB_BASE + 0x59)

#define PHY_HMMAS_BASE				0x60000
#define PHY_HMMAS_CPLLCTRL1			(PHY_HMMAS_BASE + 0x05)
#define PHY_HMMAS_CPLLCTRL4			(PHY_HMMAS_BASE + 0x07)
#define PHY_HMMAS_CPLLCTRL5			(PHY_HMMAS_BASE + 0x08)
#define PHY_HMMAS_CPLLUPLLPROG0			(PHY_HMMAS_BASE + 0x0d)
#define PHY_HMMAS_CPLLUPLLPROG1			(PHY_HMMAS_BASE + 0x0e)
#define PHY_HMMAS_CPLLUPLLPROG2			(PHY_HMMAS_BASE + 0x0f)
#define PHY_HMMAS_CPLLUPLLPROG3			(PHY_HMMAS_BASE + 0x10)

#define PHY_PPGC_BASE				0x70000
#define PHY_PPGC_ACSMDONE			(PHY_PPGC_BASE + 0x121)
#define PHY_PPGC_ACSMSTARTADDRXLATVAL0		(PHY_PPGC_BASE + 0x324)
#define PHY_PPGC_ACSMSTOPADDRXLATVAL0		(PHY_PPGC_BASE + 0x38b)
#define PHY_PPGC_HWTLPCSENA			(PHY_PPGC_BASE + 0x072)
#define PHY_PPGC_HWTLPCSENB			(PHY_PPGC_BASE + 0x073)
#define PHY_PPGC_HWTCONTROLVAL			(PHY_PPGC_BASE + 0x07e)
#define PHY_PPGC_ACSMRPTCNTOVERRIDE		(PHY_PPGC_BASE + 0x145)
#define PHY_PPGC_ACSMNOPADDR			(PHY_PPGC_BASE + 0x18a)

#define PHY_INITENG_BASE			0x90000
#define PHY_INITENG_SEQ0BDISABLEFLAG0		(PHY_INITENG_BASE + 0x70c)
#define PHY_INITENG_SEQ0BGPR1			(PHY_INITENG_BASE + 0x801)
#define PHY_INITENG_SEQ0BDLY0			(PHY_INITENG_BASE + 0x8e0)
#define PHY_INITENG_SEQ0BGPR14			(PHY_INITENG_BASE + 0x80e)
#define PHY_INITENG_SEQ0BGPR15			(PHY_INITENG_BASE + 0x80f)

#define PHY_DRTUB_BASE				0xc0000
#define PHY_DRTUB_PIEINITVECSEL			(PHY_DRTUB_BASE + 0x01)
#define PHY_DRTUB_UCCLKHCLKENABLES		(PHY_DRTUB_BASE + 0x80)
#define PHY_DRTUB_STARTDCCMCLEAR		(PHY_DRTUB_BASE + 0x88)

#define PHY_APBONLY_BASE			0xd0000
#define PHY_APBONLY_MICROCONTMUXSEL		(PHY_APBONLY_BASE + 0x00)
#define PHY_APBONLY_UCTSHADOWREGS		(PHY_APBONLY_BASE + 0x04)
#define PHY_APBONLY_DCTWRITEPROT		(PHY_APBONLY_BASE + 0x31)
#define PHY_APBONLY_UCTWRITEONLYSHADOW		(PHY_APBONLY_BASE + 0x32)
#define PHY_APBONLY_UCTDATWRITEONLYSHADOW	(PHY_APBONLY_BASE + 0x34)
#define PHY_APBONLY_MICRORESET			(PHY_APBONLY_BASE + 0x99)
#define PHY_APBONLY_SEQUENCEROVERRIDE		(PHY_APBONLY_BASE + 0xe7)

#define PHY_HMDBYTE_BASE(n)			(0xe0000 + ((n) * 0x1000))
#define PHY_HMDBYTE_VREGCTRL1DQ(n)		(PHY_HMDBYTE_BASE(n) + 0x00)
#define PHY_HMDBYTE_TXDQSLEW(n)			(PHY_HMDBYTE_BASE(n) + 0x1c)
#define PHY_HMDBYTE_TXIMPEDANCEDQ(n)		(PHY_HMDBYTE_BASE(n) + 0x2c)
#define PHY_HMDBYTE_TXIMPEDANCEDQS(n)		(PHY_HMDBYTE_BASE(n) + 0x2d)
#define PHY_HMDBYTE_ODTIMPEDANCEDQ(n)		(PHY_HMDBYTE_BASE(n) + 0x2e)
#define PHY_HMDBYTE_ODTIMPEDANCEDQS(n)		(PHY_HMDBYTE_BASE(n) + 0x2f)

/* RENESAS X5H Board (64Gb 1rank x 4pcs) */
static const struct renesas_dbsc5_board_config renesas_x5h_dbsc5_board_config = {
	.bdcfg_phyvalid		= 0xffff,
	.bdcfg_tx_drv		= 0x77777,
	.bdcfg_tx_ffc		= 0x30000000,
	.bdcfg_rx_odt		= 0x33333,
	.bdcfg_rx_dfe		= 0x0,
	.bdcfg_tx_odt		= 0x43,
	.bdcfg_tx_ntodt		= 0x3,
	.bdcfg_tx_dfe		= 0x0,
	.bdcfg_rx_dca		= 0x1,
	.bdcfg_rx_drv		= 0x6,
	.bdcfg_rx_emphasis	= 0x0,
	.bdcfg_tx_dca		= 0x1,
	.bdcfg_ca_vref		= 0x2e,
	.bdcfg_rx_vref		= 0xff001e,
	.bdcfg_rx_vref_step	= 0xf,
	.bdcfg_tx_vref		= 0x56e0a,
	.bdcfg_rfm_chk		= 0x1,
	.ch = {
		[0] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x2641350,
			.bdcfg_dqs_swap		= 0x1,
			.bdcfg_dq_swap		= { 0x47583016, 0x85013247 },
			.bdcfg_dm_swap		= { 0x2, 0x6 },
		},
		[1] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x3265410,
			.bdcfg_dqs_swap		= 0x10,
			.bdcfg_dq_swap		= { 0x54263701, 0x84650312 },
			.bdcfg_dm_swap		= { 0x8, 0x7 },
		},
		[2] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x1240635,
			.bdcfg_dqs_swap		= 0x1,
			.bdcfg_dq_swap		= { 0x56420713, 0x57802134 },
			.bdcfg_dm_swap		= { 0x8, 0x6 },
		},
		[3] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x5462301,
			.bdcfg_dqs_swap		= 0x10,
			.bdcfg_dq_swap		= { 0x74560128, 0x58760312 },
			.bdcfg_dm_swap		= { 0x3, 0x4 },
		},
		[4] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x4215306,
			.bdcfg_dqs_swap		= 0x1,
			.bdcfg_dq_swap		= { 0x45760138, 0x75803124 },
			.bdcfg_dm_swap		= { 0x2, 0x6 },
		},
		[5] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x4065321,
			.bdcfg_dqs_swap		= 0x10,
			.bdcfg_dq_swap		= { 0x65420371, 0x78560213 },
			.bdcfg_dm_swap		= { 0x8, 0x4 },
		},
		[6] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x4612035,
			.bdcfg_dqs_swap		= 0x1,
			.bdcfg_dq_swap		= { 0x25481730, 0x56703128 },
			.bdcfg_dm_swap		= { 0x6, 0x4 },
		},
		[7] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x5023416,
			.bdcfg_dqs_swap		= 0x10,
			.bdcfg_dq_swap		= { 0x54783026, 0x41650238 },
			.bdcfg_dm_swap		= { 0x1, 0x7 },
		},
		[8] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x521463,
			.bdcfg_dqs_swap		= 0x1,
			.bdcfg_dq_swap		= { 0x76453012, 0x85430127 },
			.bdcfg_dm_swap		= { 0x8, 0x6 },
		},
		[9] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x5326041,
			.bdcfg_dqs_swap		= 0x10,
			.bdcfg_dq_swap		= { 0x43620781, 0x81653420 },
			.bdcfg_dm_swap		= { 0x5, 0x7 },
		},
		[10] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x2104635,
			.bdcfg_dqs_swap		= 0x1,
			.bdcfg_dq_swap		= { 0x54763210, 0x75831062 },
			.bdcfg_dm_swap		= { 0x8, 0x4 },
		},
		[11] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x3654021,
			.bdcfg_dqs_swap		= 0x10,
			.bdcfg_dq_swap		= { 0x74561082, 0x56473012 },
			.bdcfg_dm_swap		= { 0x3, 0x8 },
		},
		[12] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x514362,
			.bdcfg_dqs_swap		= 0x1,
			.bdcfg_dq_swap		= { 0x45761032, 0x74830126 },
			.bdcfg_dm_swap		= { 0x8, 0x5 },
		},
		[13] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x4523016,
			.bdcfg_dqs_swap		= 0x10,
			.bdcfg_dq_swap		= { 0x54763021, 0x58470312 },
			.bdcfg_dm_swap		= { 0x8, 0x6 },
		},
		[14] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x2315064,
			.bdcfg_dqs_swap		= 0x1,
			.bdcfg_dq_swap		= { 0x43620718, 0x68134207 },
			.bdcfg_dm_swap		= { 0x5, 0x5 },
		},
		[15] = {
			.bdcfg_ddr_density	= { 0x6, 0xff },
			.bdcfg_ca_swap		= 0x6315042,
			.bdcfg_dqs_swap		= 0x10,
			.bdcfg_dq_swap		= { 0x47653082, 0x64583012 },
			.bdcfg_dm_swap		= { 0x1, 0x7 },
		}
	}
};

struct renesas_dbsc5_dram_priv {
	void __iomem	*regs;

	/* The board parameter structure of the board */
	const struct renesas_dbsc5_board_config *dbsc5_board_config;

	/* The board clock frequency */
	u32		brd_clk;
	u32		brd_clkdiv;
	u32		brd_clkdiva;
	/* The Mbps of DDR */
	u32		ddr_mbps;
	u32		ddr_mbpsdiv;
	/* Value indicating the enabled channel */
	u32		ddr_phyvalid;
	u32		ddr_dramvalid;
	/* Channels used for each memory rank */
	u32		ch_have_this_cs[CS_CNT];
	u32		upper_lower_byte[DBSC5_PHYNUM_CNT];
	u32		link_ecc_en;
	/* The maximum memory capacity */
	u32		max_density;
	/* Index of jedec spec1 setting table you use */
	u32		js1_ind;
};

/**
 * vch_nxt() - Macro for channel selection loop
 * @dev: DBSC5 device
 * @pos: Iterator position
 *
 * Return the ID of the channel to be used. Check for valid channels
 * between the value of posn and the maximum number of CHs. If a valid
 * channel is found, returns the value of that channel.
 */
static u32 vch_nxt(struct udevice *dev, u32 pos)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 posn;

	for (posn = pos; posn < DRAM_CH_CNT; posn++)
		if (priv->ddr_dramvalid & BIT(posn))
			break;

	return posn;
}

/**
 * vphych_nxt() - Macro for PHY channel selection loop
 * @dev: DBSC5 device
 * @pos: Iterator position
 *
 * Return the ID of the channel to be used. Check for valid channels
 * between the value of posn and the maximum number of CHs. If a valid
 * channel is found, returns the value of that channel.
 */
static u32 vphych_nxt(struct udevice *dev, u32 pos)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 posn;

	for (posn = pos; posn < DBSC5_PHYNUM_CNT; posn++)
		if (priv->ddr_phyvalid & BIT(posn))
			break;

	return posn;
}

#define foreach_vch(dev, ch) \
	for ((ch) = vch_nxt((dev), 0); (ch) < DRAM_CH_CNT; (ch) = vch_nxt((dev), (ch) + 1))

#define foreach_vphych(dev, ch) \
	for ((ch) = vphych_nxt((dev), 0); (ch) < DBSC5_PHYNUM_CNT; (ch) = vphych_nxt((dev), (ch) + 1))

#define foreach_ech(ch) \
	for (ch = 0; ch < DRAM_CH_CNT; ch++)

/**
 * dbsc5_soft_delay() - Wait loop number of CPU cycles
 * @loop: Number of cycles to wait
 *
 * This is a cycle counting approximate delay loop, used in very
 * early code to wait for near future events.
 */
static void dbsc5_soft_delay(const int loop)
{
	int i;

	for (i = 0; i < loop; i++)
		asm volatile("nop");
}

/**
 * dbsc5_init_pll3() - Set PLL3
 * @dev: DBSC5 device
 */
static void dbsc5_init_pll3(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 data_cr0, data_cr1, data_div, data_dcr_1st, data_dcr_2nd, data_dcr_3rd;
	u32 ddr_mul, ddr_mul_div, ddr_mul_ni, ddr_mul_nf;
	const int pll3_frac_mode_threshold = 9000;
	u32 ssmode, pll3vco, pll3_valid = 0;
	u32 ddr_zb3ckcr[DBSC5_PHYNUM_CNT];
	u32 zb3ckcr0, zb3ckcr1;
	u32 fvv, ch;

	/* Calculate valid PLL3 channel */
	for (ch = 0; ch < DBSC5_PHYNUM_CNT; ch++) {
		pll3_valid |= (((priv->ddr_phyvalid >> (ch * 2)) |
			       (priv->ddr_phyvalid >> ((ch * 2) + 1))) & 0x1) << ch;
	}

	/* Calculate PLL3 settings */
	if (priv->ddr_mbps < (pll3_frac_mode_threshold * priv->ddr_mbpsdiv))
		ssmode = 4;
	else
		ssmode = 0;

	ddr_mul = CLK_DIV(priv->ddr_mbps, priv->ddr_mbpsdiv * 2, priv->brd_clk,
			  priv->brd_clkdiv * (priv->brd_clkdiva + 1));

	/* PLL3VCO = EXTAL * ddr_mul */
	pll3vco = ddr_mul * priv->brd_clk / (priv->brd_clkdiv * (priv->brd_clkdiva + 1));

	if (pll3vco < 3000) {	/* div = 4 */
		data_div = 2;
		pll3vco = pll3vco * 2;
		ddr_mul = ddr_mul * 2;
		ddr_mul_div = 2;
	} else {		/* div = 2 */
		data_div = 0;
		ddr_mul_div = 1;
	}

	/*
	 *   PLL3VCO    FVV
	 * 3000 - 4000   0
	 * 3750 - 4900   1
	 * 4600 - 5800   2
	 * 5450 - 6200   3
	 */
	if (pll3vco > 5800)
		fvv = 3;
	else if (pll3vco > 4900)
		fvv = 2;
	else if (pll3vco > 4000)
		fvv = 1;
	else
		fvv = 0;

	ddr_mul_ni = (ddr_mul / 2) - 1;
	if (priv->ddr_mbps < (pll3_frac_mode_threshold * priv->ddr_mbpsdiv)) {
		ddr_mul_nf = ((8 * priv->ddr_mbps * priv->brd_clkdiv * (priv->brd_clkdiva + 1) * ddr_mul_div) /
			      (priv->ddr_mbpsdiv * 2 * priv->brd_clk)) - (8 * (ddr_mul_ni + 1) * 2);
	} else {
		ddr_mul_nf = 0;
	}

	data_cr0 = (ddr_mul_ni << 20) | (ssmode << 16);
	data_cr1 = (fvv  << 26) | ddr_mul_nf;

	/* Gradually frequency change settings */
	data_dcr_1st = 0x18;
	data_dcr_2nd = 0x10;
	data_dcr_3rd = 0;

	/* Disable write protection */
	writel(0xa5a5a501, SYSSS_CLKTOPPKCPROT0);
	while (!((readl(SYSSS_CLKTOPPKCPROT0)) & BIT(0)))
		;

	/* Clock change & setting PLL3_x_CR0 and PLL3_x_CR1 */
	for (ch = 0; ch < DBSC5_PLL3_CNT; ch++) {
		if (!(pll3_valid & BIT(ch)))
			continue;

		/* 1. Confirm that the PLL3_x are stable with PLLCLKSTAB */
		while (!(readl(SYSSS_PLL3_x_CR2(ch)) & BIT(31)))
			;

		/* 2-1. Select PLL3_0SELID to CLK_IOSC1 */
		writel(0x1, SYSSS_PLL3_xSCR(ch));
		/* 2-2. Confirm PLL3_0SELACT register to ID equals CLK_IOSC1 */
		while (((readl(SYSSS_PLL3_xSCR(ch))) & 0x10001) != 0x10001)
			;

		/* 3. Stop the PLL3_x (PLLDISTRG = 1) */
		writel(0x20000000, SYSSS_PLL3_x_CR2(ch));
		/* 4. Confirm that the PLL3_0 has been stopped (PLLCLKSTAB = 0) */
		while (readl(SYSSS_PLL3_x_CR2(ch)) & BIT(31))
			;

		/* set PLL3_x_CR0 */
		clrsetbits_le32(SYSSS_PLL3_x_CR0(ch), 0x1ff70000, data_cr0);
		/* set PLL3_x_CR1 */
		clrsetbits_le32(SYSSS_PLL3_x_CR1(ch), 0x0dffffff, data_cr1);
		/* set PLL3_x_DCR */
		writel(data_dcr_1st, SYSSS_PLL3_x_DCR(ch));
		/* set PLL3_x_CR2 (PLLENTRG = 1) */
		writel(BIT(28), SYSSS_PLL3_x_CR2(ch));
	}

	/* Clock change */
	for (ch = 0; ch < DBSC5_PLL3_CNT; ch++) {
		if (!(pll3_valid & BIT(ch)))
			continue;

		/* 1. Confirm that the PLL3_0 have been stable (PLLCLKSTAB = 1) */
		while (!(readl(SYSSS_PLL3_x_CR2(ch)) & BIT(31)))
			;

		/* 2-1. Select PLL input clock by PLLSELID register */
		writel(0, SYSSS_PLL3_xSCR(ch));
		/* 2-2. Confirm that PLL3_0SELACT register becomes same value with PLL3_0SELID */
		while (readl(SYSSS_PLL3_xSCR(ch)) & 0x10001)
			;
	}

	/* Set PLL3_DCR gradually */
	for (ch = 0; ch < DBSC5_PLL3_CNT; ch++) {
		if (!(pll3_valid & BIT(ch)))
			continue;

		/* Set PLL3_x_DCR */
		writel(data_dcr_2nd, SYSSS_PLL3_x_DCR(ch));
		/* Wait PLL3DIVSYNC is changed from 0 to 1 */
		while (!(readl(SYSSS_PLL3_x_DCR(ch)) & BIT(16)))
			;
	}

	/* Wait until the current stabilizes */
	dbsc5_soft_delay(1000);

	for (ch = 0; ch < DBSC5_PLL3_CNT; ch++) {
		if (!(pll3_valid & BIT(ch)))
			continue;

		/* Set PLL3_x_DCR */
		writel(data_dcr_3rd, SYSSS_PLL3_x_DCR(ch));
		/* Wait PLL3DIVSYNC is changed from 0 to 1 */
		while (((readl(SYSSS_PLL3_x_DCR(ch))) & 0x10000) != 0x10000)
			;
	}

	/* Wait until the current stabilizes */
	dbsc5_soft_delay(1000);

	/* Set PLL3 divider */
	zb3ckcr0 = readl(SYSSS_ZB3CKCR0);
	zb3ckcr1 = readl(SYSSS_ZB3CKCR1);
	for (ch = 0; ch < DBSC5_PHYNUM_CNT; ch++) {
		if (priv->ddr_phyvalid & BIT(ch)) {
			/* set FC[3:0] = data_div */
			ddr_zb3ckcr[ch] = data_div;
		} else {
			if (ch < 4)
				ddr_zb3ckcr[ch] = (zb3ckcr0 >> SYSSS_ZB3CKCR_PHYx(ch)) & 0x1f;
			else
				ddr_zb3ckcr[ch] = (zb3ckcr1 >> SYSSS_ZB3CKCR_PHYx(ch)) & 0x1f;
		}
	}

	zb3ckcr0 = (ddr_zb3ckcr[3] << 23) | (ddr_zb3ckcr[2] << 16) |
		   (ddr_zb3ckcr[1] << 7) | ddr_zb3ckcr[0];
	zb3ckcr1 = (ddr_zb3ckcr[7] << 23) | (ddr_zb3ckcr[6] << 16) |
		   (ddr_zb3ckcr[5] << 7) | ddr_zb3ckcr[4];

	/* Check ZB3CKCR0/1 KICK bit = 0 */
	while (readl(SYSSS_ZB3CKCR0) & SYSSS_ZB3CKCR_KICK)
		;
	while (readl(SYSSS_ZB3CKCR1) & SYSSS_ZB3CKCR_KICK)
		;

	/* Set ZB3CKCR0/1 */
	writel(zb3ckcr0, SYSSS_ZB3CKCR0);
	writel(zb3ckcr1, SYSSS_ZB3CKCR1);

	/* Set kickbit = 1 */
	setbits_le32(SYSSS_ZB3CKCR0, SYSSS_ZB3CKCR_KICK);
	setbits_le32(SYSSS_ZB3CKCR1, SYSSS_ZB3CKCR_KICK);

	/* Wait ZB3CKCR0/1 KICK bit neg */
	while (readl(SYSSS_ZB3CKCR0) & SYSSS_ZB3CKCR_KICK)
		;
	while (readl(SYSSS_ZB3CKCR1) & SYSSS_ZB3CKCR_KICK)
		;
}

/**
 * dbsc5_msres_sync_wait() - Wait for MSRES and MSRESS to synchronize
 * @msres: MSRES register
 * @msress: MSRESS register
 */
static void dbsc5_msres_sync_wait(u32 msres, u32 msress)
{
	while (readl(msres) != readl(msress))
		;
}

/**
 * dbsc5_msres_status_set - Set DBSC5 into RUN or STANDBY mode
 * @dev: DBSC5 device
 * @status: RUN or STANDBY mode
 */
static void dbsc5_msres_status_set(struct udevice *dev, u32 status)
{
	u32 ch;

	foreach_vphych(dev, ch) {
		/*
		 * Enable write access of protected registers
		 * dbq_reset / ddr_reset_apb / ddr_reset_hard / ddr_reset
		 */
		writel(0xa5a5a501, MODULE_CONTROL_MDLC0xPKCPROT1(ch));
		/* ddrfi */
		writel(0xa5a5a501, MODULE_CONTROL_MDLC12PKCPROT1);

		/* Check MDLCnMSRESS[i] = MDLCnMSRES[i] */
		/* dbq_reset */
		dbsc5_msres_sync_wait(MODULE_CONTROL_MDLC0xMSRES1(ch),
				      MODULE_CONTROL_MDLC0xMSRESS1(ch));

		/* ddr_reset_apb / ddr_reset_hard / ddr_reset */
		dbsc5_msres_sync_wait(MODULE_CONTROL_MDLC0xMSRES3(ch),
				      MODULE_CONTROL_MDLC0xMSRESS3(ch));

		/* ddrfi */
		dbsc5_msres_sync_wait(MODULE_CONTROL_MDLC15MSRES8,
				      MODULE_CONTROL_MDLC15MSRESS8);

		/* Set MDLCnMSRESx */
		/* dbq_reset */
		clrsetbits_le32(MODULE_CONTROL_MDLC0xMSRES1(ch), 0x3, status);

		/* ddr_reset_apb / ddr_reset_hard / ddr_reset */
		clrsetbits_le32(MODULE_CONTROL_MDLC0xMSRES3(ch), 0x3f,
				(status << 4) | (status << 2) | status);

		/* ddrfi */
		clrsetbits_le32(MODULE_CONTROL_MDLC15MSRES8, 0x3 << (ch * 2),
				status << (ch * 2));

		/* Check MDLCnMSRESS[i] = MDLCnMSRES[i] */
		/* dbq_reset */
		dbsc5_msres_sync_wait(MODULE_CONTROL_MDLC0xMSRES1(ch),
				      MODULE_CONTROL_MDLC0xMSRESS1(ch));

		/* ddr_reset_apb / ddr_reset_hard / ddr_reset */
		dbsc5_msres_sync_wait(MODULE_CONTROL_MDLC0xMSRES3(ch),
				      MODULE_CONTROL_MDLC0xMSRESS3(ch));

		/* ddrfi */
		dbsc5_msres_sync_wait(MODULE_CONTROL_MDLC15MSRES8,
				      MODULE_CONTROL_MDLC15MSRESS8);

		/* Disables write access of protected registers */
		/* dbq_reset / ddr_reset_apb / ddr_reset_hard / ddr_reset */
		writel(0xa5a5a500, MODULE_CONTROL_MDLC0xPKCPROT1(ch));
		/* ddrfi */
		writel(0xa5a5a500, MODULE_CONTROL_MDLC12PKCPROT1);
	}
}

/**
 * dbsc5_reg_write() - Write DBSC register
 * @dev: DBSC5 device
 * @addr: Destination address
 * @data: Setting value to be written
 *
 * Write 32bit value @data to register at @addr .
 */
static void dbsc5_reg_write(struct udevice *dev, uintptr_t addr, u32 data)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < DBSC5_DBSC_CNT; i++) {
		if (!(priv->ddr_phyvalid & BIT(i)))
			continue;

		if ((addr & 0xFFF00000) == 0xE9900000)	/* clk_dbsc region */
			writel(data, addr + (0x4000 * i));
		else /* clk_axim region */
			writel(data, addr + (0x8000 * i));
	}
}

/**
 * dbsc5_regset_unlock() - Unlock DBSC access
 * @dev: DBSC5 device
 */
static void dbsc5_regset_unlock(struct udevice *dev)
{
	dbsc5_reg_write(dev, DBSC_DBSYSCNT0, 0x1234);
	dbsc5_reg_write(dev, DBSC_DBSYSCNT0A, 0x1234);
}

/**
 * dbsc5_regset_lock() - Lock DBSC access
 * @dev: DBSC5 device
 */
static void dbsc5_regset_lock(struct udevice *dev)
{
	dbsc5_reg_write(dev, DBSC_DBSYSCNT0, 0);
	dbsc5_reg_write(dev, DBSC_DBSYSCNT0A, 0);
}

/**
 * dbsc5_send_dbcmd() - DRAM Command Write
 * @dev: DBSC5 device
 * @cmd: Command
 */
static void dbsc5_send_dbcmd(struct udevice *dev, u32 cmd)
{
	readl(DBSC_DBCMD);	/* Dummy read */

	while (readl(DBSC_DBWAIT) & BIT(0))	/* Wait for not busy */
		;

	dbsc5_reg_write(dev, DBSC_DBCMD, cmd);
}

/**
 * dbsc5_send_dbcmd_ch() - DRAM Command Write on channel
 * @ch: DBSC5 channel
 * @cmd: Command
 */
static void dbsc5_send_dbcmd_ch(u32 ch, u32 cmd)
{
	readl(DBSC_DBCMD);	/* Dummy read */

	while (readl(DBSC_DBWAIT) & BIT(0))	/* Wait for not busy */
		;

	writel(cmd, DBSC_DBCMD + (0x4000 * ch));
}

/**
 * dbsc5_f_scale_rate() - Calculate the best value for DBSC timing setting
 * @priv: Driver private data
 * @frac: Perform fractional rounding
 * @ddr_mbps: DRAM Mbps
 * @ddr_mbpsdiv: DRAM Mbps divider
 * @ps: Optimal setting value in pico second
 * @cyc: Optimal setting value in cycle count
 *
 * Convert the optimal value in pico second to in cycle count. Optionally, if @frac is true,
 * perform fractional rounding. Compare the value of the result of the conversion with the
 * value of the argument @cyc and return the larger value.
 */
static u32 dbsc5_f_scale_rate(struct renesas_dbsc5_dram_priv *priv, const bool frac,
			      u32 ddr_mbps, u32 ddr_mbpsdiv, u32 ps, u32 cyc)
{
	const u32 mul = frac ? 8 : 800000;
	const u32 tmp = DIV_ROUND_UP(ps, 10UL) * ddr_mbps;
	const u32 f_scale_div = DIV_ROUND_UP(tmp, mul * ddr_mbpsdiv);

	return (f_scale_div > cyc) ? f_scale_div : cyc;
}

/**
 * dbsc5_f_scale() - Calculate the best value for DBSC timing setting
 * @priv: Driver private data
 * @frac: Perform fractional rounding
 * @ps: Optimal setting value in pico second
 * @cyc: Optimal setting value in cycle count
 *
 * Convert the optimal value in pico second to in cycle count. Optionally, if @frac is true,
 * perform fractional rounding. Compare the value of the result of the conversion with the
 * value of the argument @cyc and return the larger value.
 */
static u32 dbsc5_f_scale(struct renesas_dbsc5_dram_priv *priv, const bool frac, u32 ps, u32 cyc)
{
	return dbsc5_f_scale_rate(priv, frac, priv->ddr_mbps, priv->ddr_mbpsdiv, ps, cyc);
}

/**
 * dbsc5_f_scale_js2() - Select optimal settings based on jedec_spec2
 * @priv: Driver private data
 * @js2: Array of jedec spec2 setting table
 *
 * Calculate and assign each setting value of jedec_spec2 by "dbsc5_f_scale" function.
 * Only the following array elements are calculated using different formulas from those
 * described above -- JS2_tRRD/JS2_tFAW/JS2_tZQCALns/JS2_tRCpb/JS2_tRCab.
 */
static void dbsc5_f_scale_js2(struct renesas_dbsc5_dram_priv *priv, u32 *f_js2)
{
	int i;

	for (i = 0; i < JS2_TBLCNT; i++) {
		f_js2[i] = dbsc5_f_scale(priv, false,
					 jedec_spec2[JS2_DERATE][i].ps,
					 jedec_spec2[JS2_DERATE][i].cyc);
	}

	f_js2[JS2_tRRD] = dbsc5_f_scale(priv, false,
					js1[priv->js1_ind].tRRD + jedec_spec2[JS2_DERATE][JS2_tRRD].ps,
					jedec_spec2[JS2_DERATE][JS2_tRRD].cyc);
	f_js2[JS2_tFAW] = dbsc5_f_scale(priv, false,
					js1[priv->js1_ind].tFAW + jedec_spec2[JS2_DERATE][JS2_tFAW].ps,
					jedec_spec2[JS2_DERATE][JS2_tFAW].cyc);
	f_js2[JS2_tZQCALns] = dbsc5_f_scale(priv, false,
					    jedec_spec2[JS2_DERATE][JS2_tZQCALns].ps * 1000, 0);
	f_js2[JS2_tDQ72DQns] = dbsc5_f_scale(priv, false,
					     jedec_spec2[JS2_DERATE][JS2_tDQ72DQns].ps * 1000, 0);
	f_js2[JS2_tCAENTns] = dbsc5_f_scale(priv, false,
					    jedec_spec2[JS2_DERATE][JS2_tCAENTns].ps * 1000, 0);
	f_js2[JS2_tRCpb] = f_js2[JS2_tRAS] + f_js2[JS2_tRPpb];
	f_js2[JS2_tRCab] = f_js2[JS2_tRAS] + f_js2[JS2_tRPab];
	f_js2[JS2_tRFCab] = dbsc5_f_scale(priv, false,
					  jedec_spec2_tRFC_ab[priv->max_density] * 1000, 0);
	f_js2[JS2_tRFCpb] = dbsc5_f_scale(priv, false,
					  jedec_spec2_tRFC_pb[priv->max_density] * 1000, 0);
	f_js2[JS2_tpbR2pbR] = dbsc5_f_scale(priv, false,
					    jedec_spec2_tpbR2pbR[priv->max_density] * 1000, 0);
	f_js2[JS2_tRFMab] = dbsc5_f_scale(priv, false,
					  jedec_spec2_tRFM_ab[priv->max_density] * 1000, 0);
	f_js2[JS2_tRFMpb] = dbsc5_f_scale(priv, false,
					  jedec_spec2_tRFM_pb[priv->max_density] * 1000, 0);

	f_js2[JS2_tRBTP] = dbsc5_f_scale(priv, false, 7500, 2) - 2;
	f_js2[JS2_tXSR] = f_js2[JS2_tRFCab] + dbsc5_f_scale(priv, false, 7500, 2);
	f_js2[JS2_tPDN] = dbsc5_f_scale(priv, false, 10000, 0) + 1;
	f_js2[JS2_tPDN_DSM] = dbsc5_f_scale(priv, true,
					    jedec_spec2[JS2_DERATE][JS2_tPDN_DSM].ps * 10, 0);
	f_js2[JS2_tXSR_DSM] = dbsc5_f_scale(priv, true,
					    jedec_spec2[JS2_DERATE][JS2_tXSR_DSM].ps * 10, 0);
	f_js2[JS2_tXDSM_XP] = dbsc5_f_scale(priv, true,
					    jedec_spec2[JS2_DERATE][JS2_tXDSM_XP].ps * 10, 0);
	f_js2[JS2_tWLWCKOFF] = dbsc5_f_scale(priv, false, 14000, 5);
}

/**
 * dbsc5_dbsc_regset_pre() - Configure primary DDR registers
 * @dev: DBSC5 device
 *
 * Set SDRAM type, Burst length, and PHY type. Frequency mode setting.
 * Write SDRAM configuration contents to registers.
 */
static void dbsc5_dbsc_regset_pre(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ddr_density[DRAM_CH_CNT][CS_CNT];
	u32 RL, WL, WCKENLR;
	u32 ch, cs, i, val;
	u32 js2[JS2_CNT];
	u32 param_trdwr;
	u32 tmp[4];

	/* Determine DBSC clock frequency (in MHz) */
	const u32 ni = (readl(SYSSS_PLL1_1_CR0) >> 20) & 0x1FF;
	const u32 cksel = (readl(SYSSS_PLL1_1_CR0) >> 7) & 0x1;
	const u32 div = 0x20 - ((readl(SYSSS_PLL1_1DCR)) & 0x1F);
	const u32 bus_clk = ((priv->brd_clk * (ni + 1) * 2) * div) / (cksel + 1) / 32 / 4;

	/* Determine board density */
	priv->max_density = 0;
	for (ch = 0; ch < DRAM_CH_CNT; ch++) {
		for (cs = 0; cs < CS_CNT; cs++) {
			ddr_density[ch][cs] = priv->dbsc5_board_config->ch[ch].bdcfg_ddr_density[cs];
			if (priv->dbsc5_board_config->ch[ch].bdcfg_ddr_density[cs] == 0xFF)
				continue;

			if (priv->dbsc5_board_config->ch[ch].bdcfg_ddr_density[cs] > priv->max_density)
				priv->max_density = priv->dbsc5_board_config->ch[ch].bdcfg_ddr_density[cs];
		}
	}

	/* Search jedec_spec1 index */
	for (i = JS1_USABLEC_SPEC_LO; i < (JS1_FREQ_TBL_NUM - 1); i++)
		if ((js1[i].fx3 * 2 * priv->ddr_mbpsdiv) >= (priv->ddr_mbps * 3))
			break;

	if (i > JS1_USABLEC_SPEC_HI)
		priv->js1_ind = JS1_USABLEC_SPEC_HI;
	else
		priv->js1_ind = i;

	RL = js1[priv->js1_ind].RLset0;
	WL = js1[priv->js1_ind].WLsetA;
	WCKENLR = js1[priv->js1_ind].WCKENLR0;

	/* Calculate jedec_spec2 */
	dbsc5_f_scale_js2(priv, js2);

	/* LPDDR5, BL=16, DFI interface */
	dbsc5_reg_write(dev, DBSC_DBMEMKIND, 0xC);
	dbsc5_reg_write(dev, DBSC_DBMEMKINDA, 0xC);
	dbsc5_reg_write(dev, DBSC_DBBL, 0x2);
	dbsc5_reg_write(dev, DBSC_DBBLA, 0x2);

	/* dcmpmd = 2 : bypass mode */
	dbsc5_reg_write(dev, DBSC_DBSYSCNT1A, 0x2);
	dbsc5_reg_write(dev, DBSC_DBPHYCONF0, 0x1);
	dbsc5_reg_write(dev, DBSC_DBSYSCONF0, 0x1);

	/* FREQRATIO=2 */
	dbsc5_reg_write(dev, DBSC_DBSYSCONF1, 0x20000);
	dbsc5_reg_write(dev, DBSC_DBSYSCONF1A, 0);
	dbsc5_reg_write(dev, DBSC_DBSYSCONF2, 0x1);
	dbsc5_reg_write(dev, DBSC_DBSYSCONF2A, 0x241);

	for (ch = 0; ch < DRAM_CH_CNT; ch++) {
		for (cs = 0; cs < CS_CNT; cs++) {
			if (ddr_density[ch][cs] == 0xFF) {
				writel(0, DBSC_DBMEMCONF(ch, cs));
				writel(0, DBSC_DBMEMCONFA(ch, cs));
			} else {
				writel(DBMEMCONF_REGD(ddr_density[ch][cs]),
				       DBSC_DBMEMCONF(ch, cs));
				writel(DBMEMCONF_REGD(ddr_density[ch][cs]),
				       DBSC_DBMEMCONFA(ch, cs));
			}
		}
	}

	/* DBTR0.cl : RL */
	dbsc5_reg_write(dev, DBSC_DBTR(0), RL);

	/* DBTR1.cwl : WL */
	dbsc5_reg_write(dev, DBSC_DBTR(1), WL);

	/* DBTR2.al = 0 */
	dbsc5_reg_write(dev, DBSC_DBTR(2), 0);

	/* DBTR3.trcd : tRCD */
	dbsc5_reg_write(dev, DBSC_DBTR(3), js2[JS2_tRCD]);

	/* DBTR4.trpa,trp : tRPab,tRPpb */
	dbsc5_reg_write(dev, DBSC_DBTR(4), (js2[JS2_tRPab] << 16) | js2[JS2_tRPpb]);

	/* DBTR5.trc : tRCpb */
	dbsc5_reg_write(dev, DBSC_DBTR(5), js2[JS2_tRCpb]);

	/* DBTR6.tras : tRAS */
	dbsc5_reg_write(dev, DBSC_DBTR(6), js2[JS2_tRAS]);

	/* DBTR7.trrd : tRRD */
	dbsc5_reg_write(dev, DBSC_DBTR(7), (js2[JS2_tRRD] << 16) | js2[JS2_tRRD]);

	/* DBTR8.tfaw : tFAW */
	dbsc5_reg_write(dev, DBSC_DBTR(8), js2[JS2_tFAW]);

	/* DBTR9.trdpr : nRBTP */
	dbsc5_reg_write(dev, DBSC_DBTR(9), js1[priv->js1_ind].nRBTP);

	/* DBTR10.twr : nWR */
	dbsc5_reg_write(dev, DBSC_DBTR(10), js1[priv->js1_ind].nWR);

	/*
	 * DBTR11.trdwr : RL + BL/n_max + RU(tWCK2DQO(max)/tCK) + RD(tRPST/tCK) -
	 *                ODTLon - RD(tODTon(min)/tCK) + 1 + feature
	 */
	param_trdwr = RL + 4 + js2[JS2_tWCK2DQO_HF] + 0 - js1[priv->js1_ind].ODTLon -
		      (js2[JS2_tODTon_min] - 1) + 1 + 0;
	dbsc5_reg_write(dev, DBSC_DBTR(11), param_trdwr);

	/* DBTR12.twrrd_s,twrrd : WL + BL/2 + tWTR_S,WL + BL + tWTR_L */
	dbsc5_reg_write(dev, DBSC_DBTR(12), ((WL + 2 + js2[JS2_tWTR_S]) << 16) |
					    (WL + 4 + js2[JS2_tWTR_L]));

	/* DBTR13.trfcpb,trfc : tRFCpb,tRFCab */
	dbsc5_reg_write(dev, DBSC_DBTR(13), (js2[JS2_tRFCpb] << 16) | (js2[JS2_tRFCab]));

	/* DBTR14.tcscal,tckehdll,tckeh : tCSCAL,tXP,tXP */
	dbsc5_reg_write(dev, DBSC_DBTR(14), (js2[JS2_tCSCAL] << 24) | (js2[JS2_tXP] << 16) |
					    js2[JS2_tXP]);

	/* DBTR15 */
	/* tckel : tSR */
	tmp[0] = js2[JS2_tSR];
	/* tckesr : tSR */
	tmp[1] = js2[JS2_tSR];
	/* tespd : tESPD = 2 */
	tmp[2] = 0x2;
	dbsc5_reg_write(dev, DBSC_DBTR(15), (tmp[2] << 24) | (tmp[1] << 16) | tmp[0]);

	/* DBTR16 */
	/* wdql(tphy_wrlat + tphy_wrdata) */
	tmp[0] = (WL * 4) - 5 + 2;
	/* dqenltcy(tphy_wrlat) */
	tmp[1] = (WL * 4) - 5;
	/* dql(tphy_rdlat + trddata_en) : (6 + csrDFIMRL) * 4 + trddata_en */
	tmp[2] = ((6 + 7) * 4) + ((RL * 4) - 13);
	/* dqienltncy(trddata_en) : RL * 4 - 13 */
	tmp[3] = (RL * 4) - 13;
	dbsc5_reg_write(dev, DBSC_DBTR(16), (tmp[3] << 25) | (tmp[2] << 16) | (tmp[1] << 8) | tmp[0]);

	/* DBTR17.tmodrd,tmod : tMRR,tMRW */
	dbsc5_reg_write(dev, DBSC_DBTR(17), (js2[JS2_tMRR] << 24) | (js2[JS2_tMRW] << 16));

	/* DBTR18.rodtl,rodta = 0 */
	dbsc5_reg_write(dev, DBSC_DBTR(18), 0);

	/* DBTR19.tzqcl,tzqcs = 0 */
	dbsc5_reg_write(dev, DBSC_DBTR(19), 0);

	/* DBTR20.txsdll,txs : tXSR,tXSR */
	dbsc5_reg_write(dev, DBSC_DBTR(20), (js2[JS2_tXSR] << 16) | js2[JS2_tXSR]);

	/* DBTR21.tccdmw, tccd */
	/* tccd(BL/2_max) */
	tmp[0] = 0x4;
	/* tccd_s(BL/2_min) */
	tmp[1] = 0x2;
	/* tccdmw */
	if ((priv->ddr_mbps * 3) > (25600 * priv->ddr_mbpsdiv))
		tmp[2] = 0x14;
	else
		tmp[2] = 0x10;
	dbsc5_reg_write(dev, DBSC_DBTR(21), (tmp[2] << 24) | (tmp[1] << 16) | tmp[0]);

	/* DBTR22.tzqcal,tzqlat : tZQCAL,tZQLAT */
	dbsc5_reg_write(dev, DBSC_DBTR(22), (js2[JS2_tZQCALns] << 16) | js2[JS2_tZQLAT]);

	/* DBTR23.rrspc = 0 */
	dbsc5_reg_write(dev, DBSC_DBTR(23), 0);

	/* DBTR24 */
	/* wrcslat(tphy_wrcslat) */
	tmp[0] = (WL * 4) - 5;
	/* wrcsgap(tphy_wrcsgap) */
	tmp[1] = 0x4;
	/* rdcslat(tphy_rdcslat) */
	tmp[2] = (RL * 4) - 13;
	/* rdcsgap(tphy_rdcsgap) */
	tmp[3] = 0x4;
	dbsc5_reg_write(dev, DBSC_DBTR(24), (tmp[3] << 24) | (tmp[2] << 16) |
					    (tmp[1] << 8) | tmp[0]);

	/* DBTR25 */
	dbsc5_reg_write(dev, DBSC_DBTR(25), 0);

	/* DBTR26 */
	dbsc5_reg_write(dev, DBSC_DBTR(26), 0);

	/* DBTR27.tpdn : tPDN */
	dbsc5_reg_write(dev, DBSC_DBTR(27), js2[JS2_tPDN]);

	/* DBTR28.txsrdsm : tXSR_DSM */
	dbsc5_reg_write(dev, DBSC_DBTR(28), js2[JS2_tXSR_DSM]);

	/* DBTR29.tdsmxp : tXDSM_XP */
	dbsc5_reg_write(dev, DBSC_DBTR(29), js2[JS2_tXDSM_XP]);

	/* DBTR30.tcmdpd : tCMDPD = 3 */
	dbsc5_reg_write(dev, DBSC_DBTR(30), 0x3);

	/* DBTR31.twck2dqomax,twck2dqimax : tWCK2DQO_HF/LF,tWCK2DQI_HF/LF */
	dbsc5_reg_write(dev, DBSC_DBTR(31), (js2[JS2_tWCK2DQO_HF] << 4) | js2[JS2_tWCK2DQI_HF]);

	/* DBTR32 */
	/* twckenr */
	tmp[0] = (WCKENLR * 4) - 4 - 1;
	/* twckenw */
	tmp[1] = (js1[priv->js1_ind].WCKENLW * 4) - 4 - 1;
	/* twckenlf */
	tmp[2] = (js1[priv->js1_ind].WCKENLF * 4) - 4;
	/* twckpresta */
	tmp[3] = js1[priv->js1_ind].WCKPRESTA * 4;
	dbsc5_reg_write(dev, DBSC_DBTR(32), (tmp[3] << 24) | (tmp[2] << 16) |
					    (tmp[1] << 8) | tmp[0]);

	/* DBTR33 */
	/* twcktgl */
	tmp[0] = 4;
	/* twckdis : (RL+ bl/n_max) * 4 + RU(tWCKPST/tWCK),tWCKPST = MR10[3:2] = 2.5 */
	tmp[1] = ((RL + 4) * 4) + 3;
	dbsc5_reg_write(dev, DBSC_DBTR(33), (tmp[1] << 8) | tmp[0]);

	/* DBTR34 */
	/* twcksus = 4 */
	tmp[0] = 4;
	/* twckpst : RU(tWCKPST/tCK),tWCKPST = MR10[3:2] = 2.5 */
	tmp[1] = 1;
	dbsc5_reg_write(dev, DBSC_DBTR(34), (tmp[1] << 8) | tmp[0]);

	/* DBTR35 */
	/* trd2wckoff : RL + BL/n_max + RD(tWCKPST/tCK) + 1 */
	tmp[0] = RL + 4 + 0 + 1;
	/* twr2wckoff : WL + BL/n_max + RD(tWCKPST/tCK) + 1 */
	tmp[1] = WL + 4 + 0 + 1;
	dbsc5_reg_write(dev, DBSC_DBTR(35), (tmp[1] << 16) | tmp[0]);

	/* DBTR36 */
	/* twssuswrx : CAS(WCKSUS)WRX */
	tmp[0] = 3;
	/* twsoffwrx : CAS(WS_OFF)WRX */
	tmp[1] = 3;
	/* twsfswrx : CAS(WS_FS)WRX */
	tmp[2] = 2;
	dbsc5_reg_write(dev, DBSC_DBTR(36), (tmp[2] << 16) | (tmp[1] << 8) | tmp[0]);

	/* DBTR37 */
	/* tosco : tOSCODQI */
	tmp[0] = js2[JS2_tOSCODQI];
	/* toscodqi : tOSCODQI */
	tmp[1] = js2[JS2_tOSCODQI];
	/* toscodqo : tOSCODQO */
	tmp[2] = js2[JS2_tOSCODQO];
	dbsc5_reg_write(dev, DBSC_DBTR(37), (tmp[2] << 24) | (tmp[1] << 16) | tmp[0]);

	/* DBTR38.tpbr2act,tpbr2pbr : tpbR2act,tpbR2pbR */
	dbsc5_reg_write(dev, DBSC_DBTR(38), (js2[JS2_tpbR2act] << 8) | js2[JS2_tpbR2pbR]);

	/* DBTR39.trfmpb,trmbab : tRFMpb,tRFMab */
	dbsc5_reg_write(dev, DBSC_DBTR(39), (js2[JS2_tRFMpb] << 16) | js2[JS2_tRFMab]);

	/* DBRNK2 */
	/* rnkrr = tphy_wckcsgap + tWCK2DQO_rank2rank_HF/tWCK */
	val = dbsc5_f_scale_rate(priv, false, priv->ddr_mbps * 4, priv->ddr_mbpsdiv, 650, 0);
	tmp[0] = (4 + val) / 4;
	if ((tmp[0] * 4) == (4 + val))
		val = tmp[0];
	else
		val = tmp[0] + 1;
	dbsc5_reg_write(dev, DBSC_DBRNK(2), (val << 4) | val);

	/* DBRNK3 */
	/* rnkrw = tphy_wckcsgap + tWCK2DQO_rank2rank_HF/tWCK */
	val = dbsc5_f_scale_rate(priv, false, priv->ddr_mbps * 4, priv->ddr_mbpsdiv, 650, 0);
	tmp[0] = (4 + val) / 4;
	if ((tmp[0] * 4) == (4 + val))
		val = tmp[0];
	else
		val = (tmp[0] + 1);

	/* trd2wckoff - trdwr */
	tmp[0] = (RL + 4 + 0 + 1) - param_trdwr;
	if (tmp[0] > val)
		val = tmp[0];
	dbsc5_reg_write(dev, DBSC_DBRNK(3), (val << 4) | val);

	/* DBRNK4 */
	/* rnkwr = tphy_wckcsgap */
	dbsc5_reg_write(dev, DBSC_DBRNK(4), 0x11);

	/* DBRNK5 */
	/* rnkww = tphy_wckcsgap */
	dbsc5_reg_write(dev, DBSC_DBRNK(5), 0x11);

	/* DBRNK6 */
	/* refmode = tphy_wckcsgap */
	/* per_bank_refresh */
	dbsc5_reg_write(dev, DBSC_DBRNK(6), 0x1);

	/* SCFCTST0 */
	/* scpreact */
	tmp[0] = 1UL * js2[JS2_tRPpb] * bus_clk * priv->ddr_mbpsdiv * 8 /
		 priv->ddr_mbps / priv->brd_clkdiv;
	/* scactrdwr */
	tmp[1] = 1UL * (WL + 2 + 1 + js1[priv->js1_ind].nWR + js2[JS2_tRPpb]) *
		 bus_clk * priv->ddr_mbpsdiv * 8 / priv->ddr_mbps / priv->brd_clkdiv;
	/* scrdacrt */
	tmp[2] = 1UL * ((js1[priv->js1_ind].nRBTP + 2) + js2[JS2_tRPpb]) *
		 bus_clk * priv->ddr_mbpsdiv * 8 / priv->ddr_mbps / priv->brd_clkdiv;
	/* scactact */
	tmp[3] = 1UL * js2[JS2_tRCpb] * bus_clk * priv->ddr_mbpsdiv * 8 /
		 priv->ddr_mbps / priv->brd_clkdiv;
	dbsc5_reg_write(dev, DBSC_DBSCHFCTST(0), (tmp[3] << 24) | (tmp[2] << 16) |
						 (tmp[1] << 8) | tmp[0]);

	/* SCFCTST1 */
	/* scasyncofs */
	tmp[0] = 12;
	/* scactrdwr */
	tmp[1] = 1UL * js2[JS2_tRCD] * bus_clk * priv->ddr_mbpsdiv * 8 /
		 priv->ddr_mbps / priv->brd_clkdiv;
	/* scwrrd */
	tmp[2] = 1UL * (WL + 4 + js2[JS2_tWTR_L]) * bus_clk * priv->ddr_mbpsdiv * 8 /
		 priv->ddr_mbps / priv->brd_clkdiv;
	/* scrdwr */
	tmp[3] = 1UL * param_trdwr * bus_clk * priv->ddr_mbpsdiv * 8 /
		 priv->ddr_mbps / priv->brd_clkdiv;
	dbsc5_reg_write(dev, DBSC_DBSCHFCTST(1), (tmp[3] << 24) | (tmp[2] << 16) |
						 (tmp[1] << 8) | tmp[0]);

	/* DBSCHRW1 */
	/* sctrfcab */
	val = 1UL * (js2[JS2_tRFCab] + js2[JS2_tZQLAT]) * bus_clk * priv->ddr_mbpsdiv * 8 /
		priv->ddr_mbps / priv->brd_clkdiv;
	/* sctrfcab[7:0] = DBSCHRW1[7:0] */
	tmp[0] = val & 0xFF;
	/* sctrfcab[8] = DBSCHRW1[24] */
	tmp[3] = val & 0x100;
	/* sctrfcpb */
	tmp[1] = 1UL * (js2[JS2_tRFCpb]) * bus_clk * priv->ddr_mbpsdiv * 8 /
		 priv->ddr_mbps / priv->brd_clkdiv;
	/* sctrfcpb */
	tmp[2] = 1UL * (js2[JS2_tpbR2pbR]) * bus_clk * priv->ddr_mbpsdiv * 8 /
		 priv->ddr_mbps / priv->brd_clkdiv;
	dbsc5_reg_write(dev, DBSC_DBSCHRW(1), (tmp[3] << 24) | (tmp[2] << 16) |
					      (tmp[1] << 8) | tmp[0]);

	/* DBSCHRW2 */
	/* scrfpben */
	tmp[0] = 1;
	/* screfipb : tREFIpb */
	tmp[1] = DBSC_REFINT * bus_clk / priv->brd_clkdiv / 1000 / 8;
	dbsc5_reg_write(dev, DBSC_DBSCHRW(2), (tmp[1] << 16) | tmp[0]);

	/* DBSCHTR0 */
	/* scdt0 */
	tmp[0] = (1UL * 4 * bus_clk * priv->ddr_mbpsdiv * 8 / priv->ddr_mbps / priv->brd_clkdiv) - 1;
	/* scdt1 */
	tmp[1] = (1UL * 8 * bus_clk * priv->ddr_mbpsdiv * 8 / priv->ddr_mbps / priv->brd_clkdiv) - 1;
	/* scdt2 */
	tmp[2] = (1UL * 12 * bus_clk * priv->ddr_mbpsdiv * 8 / priv->ddr_mbps / priv->brd_clkdiv) - 1;
	/* scdt3 */
	tmp[3] = (1UL * 16 * bus_clk * priv->ddr_mbpsdiv * 8 / priv->ddr_mbps / priv->brd_clkdiv) - 1;
	dbsc5_reg_write(dev, DBSC_DBSCHTR0, (tmp[3] << 24) | (tmp[2] << 16) |
					    (tmp[1] << 8) | tmp[0]);

	dbsc5_reg_write(dev, DBSC_DBBCAMDIS, 0x10);

	/* Dummy PDE */
	dbsc5_send_dbcmd(dev, 0x8840000);
	/* Dummy PDX */
	dbsc5_send_dbcmd(dev, 0x8840001);
}

/**
 * dbsc5_dbsc_regset_post() - Set DBSC registers
 * @dev: DBSC5 device
 */
static void dbsc5_dbsc_regset_post(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 val;
	u32 tmp;

	dbsc5_reg_write(dev, DBSC_DBBUS0CNF1, 0);

	/* SRX */
	dbsc5_send_dbcmd(dev, 0xA840001);

	/* Write DBI ON */
	dbsc5_reg_write(dev, DBSC_DBDBICNT, 0x1);

	/* set REFCYCLE */
	val = DBSC_REFINT * priv->ddr_mbps / 8000 / priv->ddr_mbpsdiv;
	/* refpmax=8 */
	dbsc5_reg_write(dev, DBSC_DBRFCNF1, 0x80000 | (val & 0xFFFF));
	/* refpmin=1 */
	dbsc5_reg_write(dev, DBSC_DBRFCNF2, 0x10000);

	/* periodic dram zqcal enable */
	dbsc5_reg_write(dev, DBSC_DBCALCNF, 0x1000010);

	/* wtmode = 1, pmstrmd = 2(WA for WS1), pmstren = 1 */
	dbsc5_reg_write(dev, DBSC_DBDFIPMSTRCNF, 0x15);

	/* periodic phy ctrl update enable */
	/* max = 80, min = 76, en = 1 */
	dbsc5_reg_write(dev, DBSC_DBDFICUPDCNF, 0x504C0001);

	/* set Auto Refresh */
	dbsc5_reg_write(dev, DBSC_DBRFEN, 0x1);

	/* dram access enable */
	dbsc5_reg_write(dev, DBSC_DBACEN, 0x1);

	tmp = (PERIODIC_TRAINING_INTERVAL * 1000) / DBSC_REFINT;
	/* (ZQCalibration_interval + 1) / ZQCalibration_interval */
	val = (tmp * (0x10 + 1)) / 0x10;
	/* snoopen=1 */
	dbsc5_reg_write(dev, DBSC_DBSNOOPC, BIT(24) | val);
}

#define RTMMAC_BASE		0xb8900000
#define RTDMAC_RDMSAR_0(ch)	(RTMMAC_BASE + 0x0 + ((ch) * 0x1000))
#define RTDMAC_RDMDAR_0(ch)	(RTMMAC_BASE + 0x4 + ((ch) * 0x1000))
#define RTDMAC_RDMTCR_0(ch)	(RTMMAC_BASE + 0x8 + ((ch) * 0x1000))
#define RTDMAC_RDMCHCR_0(ch)	(RTMMAC_BASE + 0xc + ((ch) * 0x1000))
#define RTDMAC_RDMCHCLR(ch)	(RTMMAC_BASE + 0x100 + ((ch) * 0x1000))
#define RTDMAC_RDMOR		0xb9438060

/**
 * dbsc5_rtdmac_init() - Configure RTDMAC DMA controller
 * @ch: DMA Channel
 * @dst: Destination address
 * @src: Source address
 * @cnt: Byte count
 */
static void dbsc5_rtdmac_init(u32 ch, u32 dst, u32 src, u32 cnt)
{
	/* DMA Setting - 64Byte/AutoRequest mode */
	writel(src, RTDMAC_RDMSAR_0(ch));
	writel(dst, RTDMAC_RDMDAR_0(ch));
	writel(cnt, RTDMAC_RDMTCR_0(ch));
	writel(0x105409, RTDMAC_RDMCHCR_0(ch));
}

/**
 * dbsc5_rtdmac_start() - Start RTDMAC DMA controller
 */
static void dbsc5_rtdmac_start(void)
{
	/* Start DMA */
	writew(0x1, RTDMAC_RDMOR);
}

/**
 * dbsc5_rtdmac_wait() - Wait for RTDMAC DMA completion
 * @ch: DMA Channel
 */
static void dbsc5_rtdmac_wait(int ch)
{
	u32 val;

	/* Wait for DMA completion */
	for (;;) {
		val = readl(RTDMAC_RDMCHCR_0(ch));
		if (val & BIT(1)) { /* Clear TE */
			writel(val & ~BIT(1), RTDMAC_RDMCHCR_0(ch));
			break;
		}

		if (val & BIT(31)) { /* Clear CAE */
			writel(val & ~BIT(31), RTDMAC_RDMCHCR_0(ch));
			break;
		}
	}
}

/**
 * dbsc5_rtdmac_stop() - Stop RTDMAC DMA controller
 */
static void dbsc5_rtdmac_stop(void)
{
	u32 ch;

	/* Disable DMA */
	writew(0, RTDMAC_RDMOR);
	for (ch = 0; ch < 16; ch++)
		writel(readl(RTDMAC_RDMCHCR_0(ch)) | BIT(0), RTDMAC_RDMCHCLR(ch));
}

/**
 * dbsc5_rtdmac_phyinit_trans_ach() - Load PHY using RTDMAC
 * @dev: DBSC5 device
 * @dst: Destination address
 * @src: Source address
 * @size: Transfer size
 */
static void dbsc5_rtdmac_phyinit_trans_ach(struct udevice *dev, u32 dst, u32 src, u32 size)
{
	u32 ch;

	foreach_vphych(dev, ch) {
		dbsc5_rtdmac_init(ch, DDR_PHY_DMA_TRANS_BASE(ch) + (dst << 2),
				  src, (size + 0x3f) >> 6);
		dbsc5_rtdmac_start();
	}

	foreach_vphych(dev, ch)
		dbsc5_rtdmac_wait(ch);

	dbsc5_rtdmac_stop();
}

/**
 * dbsc5_phy_apb_wr() - Write PHY register
 * @ch: PHY channel
 * @addr: Register address
 * @data: Register value
 */
static void dbsc5_phy_apb_wr(u32 ch, u32 addr, u32 data)
{
	writel(data, DDR_PHY_BASE(ch) + (addr << 2));
}

/**
 * dbsc5_phy_apb_wr_ach() - Write PHY register on all channels
 * @addr: Register address
 * @data: Register value
 */
static void dbsc5_phy_apb_wr_ach(struct udevice *dev, u32 addr, u32 data)
{
	u32 ch;

	foreach_vphych(dev, ch)
		dbsc5_phy_apb_wr(ch, addr, data);
}

/**
 * dbsc5_phy_apb_rd() - Read PHY register
 * @ch: PHY channel
 * @addr: Register address
 */
static u32 dbsc5_phy_apb_rd(u32 ch, u32 addr)
{
	return readl(DDR_PHY_BASE(ch) + (addr << 2));
}

/**
 * ddrphy_apb_rd_mod_wr() - Read-modify-Write PHY register
 * @ch: PHY channel
 * @addr: Register address
 * @len: Clearing bitmask length
 * @lsb: Clearing bitmask shift
 * @data: Register value
 */
static void ddrphy_apb_rd_mod_wr(u32 ch, u32 addr, u32 len, u32 lsb, u32 data)
{
	u32 msk = 0xffffffff;
	u32 val;

	if (len != 32)
		msk = (BIT(len) - 1) << lsb;

	val = dbsc5_phy_apb_rd(ch, addr);
	val &= ~msk;
	val |= data << lsb;
	dbsc5_phy_apb_wr(ch, addr, val);
}

/**
 * dbsc5_phy_apb_rd() - Read PHY message block
 * @ch: PHY channel
 */
static u32 dbsc5_phy_apb_msg_rd(u32 ch)
{
	u32 ret;

	while (dbsc5_phy_apb_rd(ch, PHY_APBONLY_UCTSHADOWREGS) & BIT(0))
		;

	ret = dbsc5_phy_apb_rd(ch, PHY_APBONLY_UCTWRITEONLYSHADOW);
	dbsc5_phy_apb_wr(ch, PHY_APBONLY_DCTWRITEPROT, 0);

	while (!(dbsc5_phy_apb_rd(ch, PHY_APBONLY_UCTSHADOWREGS) & BIT(0)))
		;

	dbsc5_phy_apb_wr(ch, PHY_APBONLY_DCTWRITEPROT, BIT(0));

	return ret;
}

/**
 * dbsc5_ddr_config_post() - Set DDR user configuration
 * @dev: DBSC5 device
 */
static void dbsc5_ddr_config_post(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ch, tmp;

	foreach_vphych(dev, ch) {
		/* 2-Rank settings */
		if (priv->ch_have_this_cs[1] & (0x3 << (ch * 2)))
			dbsc5_phy_apb_wr(ch, PHY_PPGC_HWTCONTROLVAL, 0xc3);

		/* Configure IO parameters */
		/* DRAM Tx ODT for DQ */
		ddrphy_apb_rd_mod_wr(ch, 0x41036, 3, 7,
				     (priv->dbsc5_board_config->bdcfg_tx_odt >> 4) & 0x7);

		/* DRAM Tx ODT for CA */
		ddrphy_apb_rd_mod_wr(ch, 0x41036, 3, 11,
				     (priv->dbsc5_board_config->bdcfg_tx_odt) & 0x7);

		/* DRAM Tx NT DQ ODT */
		tmp = (priv->dbsc5_board_config->bdcfg_tx_ntodt) & 0x7;
		if (tmp < 0x4) {
			ddrphy_apb_rd_mod_wr(ch, 0x4105A, 1, 6, 0x0);
			tmp += 4;
		}
		ddrphy_apb_rd_mod_wr(ch, 0x4105A, 3, 12, tmp);

		/* DRAM Tx NT ODT */
		ddrphy_apb_rd_mod_wr(ch, 0x41036, 1, 10,
				     ((priv->dbsc5_board_config->bdcfg_tx_ntodt) >> 4) & 0x1);

		/* DRAM Tx Per-pin DFE  */
		ddrphy_apb_rd_mod_wr(ch, 0x4105A, 1, 7,
				     (priv->dbsc5_board_config->bdcfg_tx_dfe) & 0x1);

		/* DRAM Rx DRV */
		ddrphy_apb_rd_mod_wr(ch, 0x41026, 3, 7,
				     (priv->dbsc5_board_config->bdcfg_rx_drv) & 0x7);

		/* Link ECC setting */
		if (priv->link_ecc_en) {
			ddrphy_apb_rd_mod_wr(ch, 0x41052, 1, 11, 0x1);
			ddrphy_apb_rd_mod_wr(ch, 0x41052, 1, 13, 0x1);
		}
	}
}

/**
 * dbsc5_ddr_smb_config() - Set user configuration
 * @dev: DBSC5 device
 */
static void dbsc5_ddr_smb_config(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ch, val;

	foreach_vphych(dev, ch) {
		/* Frequency settings */
		if (priv->ddr_mbps < (6401 * priv->ddr_mbpsdiv)) {
			/* 5501 - 6400 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_DRAMFREQ, 16, 16, 0x1900);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_A0, 8, 16, 0xB0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_A1, 8, 24, 0xB0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_B0, 8, 0, 0xB0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_B1, 8, 8, 0xB0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_A0, 8, 16, 0xBB);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_A1, 8, 24, 0xBB);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_B0, 8, 0, 0xBB);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_B1, 8, 8, 0xBB);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_ALT_RL, 8, 24, 0x3B);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MAIN_RL, 8, 0, 0x37);
		} else if (priv->ddr_mbps < (7501 * priv->ddr_mbpsdiv)) {
			/* 6401 - 7500 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_DRAMFREQ, 16, 16, 0x1D48);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_A0, 8, 16, 0xC0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_A1, 8, 24, 0xC0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_B0, 8, 0, 0xC0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_B1, 8, 8, 0xC0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_A0, 8, 16, 0xCC);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_A1, 8, 24, 0xCC);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_B0, 8, 0, 0xCC);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_B1, 8, 8, 0xCC);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_ALT_RL, 8, 24, 0x4B);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MAIN_RL, 8, 0, 0x43);
		} else if (priv->ddr_mbps < (8534 * priv->ddr_mbpsdiv)) {
			/* 7501 - 8533 -- None */
		} else {
			/* 8534 - 9600 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_DRAMFREQ, 16, 16, 0x2580);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_DCAOPTS,  8, 8, 0x44);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_A0, 8, 16, 0xE0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_A1, 8, 24, 0xE0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_B0, 8, 0, 0xE0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR1_B1, 8, 8, 0xE0);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_A0, 8, 16, 0xEE);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_A1, 8, 24, 0xEE);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_B0, 8, 0, 0xEE);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR2_B1, 8, 8, 0xEE);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_ALT_RL, 8, 24, 0x63);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MAIN_RL, 8, 0, 0x57);
		}

		/* 2Rank settings */
		if (priv->ch_have_this_cs[1] & (0x3 << (ch * 2))) {
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_CSPRESENTCHA, 2, 8, 0x3);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_CSPRESENTCHB, 2, 0, 0x3);
		}

		/* Board settings, data byte swap */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_UPPERLOWERBYTE, 4, 8, priv->upper_lower_byte[ch]);

		/* Configure Training settings */
		/* SOC Rx DFE function */
		if (!(priv->dbsc5_board_config->bdcfg_rx_dfe & BIT(0))) {
			/* Rx DFE Disable */
			/* RxDFEBitTimeControl[4] */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXDFEBITTIMECONTROL, 1, 4, 0x1);
		} else {
			/* Rx DFE DQ 1tap */
			/* RxDFEOpt[6] */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXDFEOPT, 1, 30, 0);
			/* RxClk scan skip */
			/* RxDFEBitTimeControl[6] */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXDFEBITTIMECONTROL, 1, 6, 1);
		}

		/* DRAM Tx DFE function */
		val = priv->dbsc5_board_config->bdcfg_tx_dfe & 0x7;
		if (val == 0x0) {
			/* Tx DFE Disable */
			/* PHY_APB_SEQUENCECTRL[10] */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_SEQUENCECTRL, 1, 10, 0x0);
		} else {
			/* Select Tx DFE step */
			/* DRAM Tx DFE for Upper */
			/* PHY_APB_MR24_A0, PHY_APB_MR24_A1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR24_A0, 3, 20, val);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR24_A1, 3, 28, val);
			/* PHY_APB_MR24_B0, PHY_APB_MR24_B1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR24_B0, 3, 4, val);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR24_B1, 3, 12, val);

			/* DRAM Tx DFE for Lower */
			/* PHY_APB_MR24_A0, PHY_APB_MR24_A1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR24_A0, 3, 16, val);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR24_A1, 3, 24, val);
			/* PHY_APB_MR24_B0, PHY_APB_MR24_B1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR24_B0, 3, 0, val);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR24_B1, 3, 8, val);
		}

		/* DRAM Tx Per-pin DFE  */
		val = (priv->dbsc5_board_config->bdcfg_tx_dfe >> 4) & 0x1;
		/* PHY_APB_MR41_A0, PHY_APB_MR41_A1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR41_A0, 1, 16, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR41_A1, 1, 24, val);
		/* PHY_APB_MR41_B0, PHY_APB_MR41_B1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR41_B0, 1, 0, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR41_B1, 1, 8, val);
		/* TxDFETrainOpt[0] */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_TXDFETRAINOPT, val, 1, 0);

		/* DRAM Rx DCA function */
		/* PHY_APB_SEQUENCECTRL[7] */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_SEQUENCECTRL, 1, 7,
				     priv->dbsc5_board_config->bdcfg_rx_dca & 0x1);

		/* DRAM Tx DCA function */
		/* PHY_APB_SEQUENCECTRL[6] */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_SEQUENCECTRL, 1, 6,
				     priv->dbsc5_board_config->bdcfg_tx_dca & 0x1);

		/* CA Vref training initial value */
		val = priv->dbsc5_board_config->bdcfg_ca_vref & 0x7F;
		/* PHY_APB_MR12_A0, PHY_APB_MR12_A1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR12_A0, 7, 16, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR12_A1, 7, 24, val);
		/* PHY_APB_MR12_B0, PHY_APB_MR12_B1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR12_B0, 7, 0, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR12_B1, 7, 8, val);

		/* Rx Vref training range */
		/* RxVrefStartPat, RxVrefStartPrbs */
		val = priv->dbsc5_board_config->bdcfg_rx_vref & 0xFFFF;
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXVREFSTARTPAT, 16, 0, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXVREFSTARTPRBS, 16, 16, val);
		/* RxVrefEndPat, RxVrefEndPrbs */
		val = (priv->dbsc5_board_config->bdcfg_rx_vref >> 16) & 0xFFFF;
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXVREFENDPAT, 16, 0, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXVREFENDPRBS, 16, 16, val);
		/* RxVrefStepPat, RxVrefStepPrbs */
		val = priv->dbsc5_board_config->bdcfg_rx_vref_step & 0xFFFF;
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXVREFSTEPPAT, 16, 16, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_RXVREFSTEPPRBS, 16, 0, val);

		/* Tx Vref training range */
		/* TxVrefStart */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_TXVREFSTART, 8, 0,
				     priv->dbsc5_board_config->bdcfg_tx_vref & 0xFF);
		/* TxVrefEnd */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_TXVREFEND, 8, 8,
				     (priv->dbsc5_board_config->bdcfg_tx_vref >> 8) & 0xFF);
		/* TxVrefStep */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_TXVREFSTEP, 8, 16,
				     (priv->dbsc5_board_config->bdcfg_tx_vref >> 16) & 0xFF);

		/* FW message setting */
		/* HdtCtrl */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_HDTCTRL, 8, 16, 0xff);

		/* Configure IO parameters */
		/* DRAM Tx ODT for DQ */
		val = (priv->dbsc5_board_config->bdcfg_tx_odt >> 4) & 0x7;
		/* PHY_APB_MR11_A0, PHY_APB_MR11_B0 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_A0, 3, 16, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_A1, 3, 24, val);
		/* PHY_APB_MR11_A1, PHY_APB_MR11_B1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_B0, 3, 0, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_B1, 3, 8, val);

		/* DRAM Tx ODT for CA */
		val = priv->dbsc5_board_config->bdcfg_tx_odt & 0x7;
		/* PHY_APB_MR11_A0, PHY_APB_MR11_A1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_A0, 3, 20, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_A1, 3, 28, val);
		/* PHY_APB_MR11_B0, PHY_APB_MR11_B1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_B0, 3, 4, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_B1, 3, 12, val);

		/* DRAM Tx NT DQ ODT */
		val = priv->dbsc5_board_config->bdcfg_tx_ntodt & 0x7;
		/* PHY_APB_MR41_A0, PHY_APB_MR41_A1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR41_A0, 3, 21, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR41_A1, 3, 29, val);
		/* PHY_APB_MR41_B0, PHY_APB_MR41_B1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR41_B0, 3, 5, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR41_B1, 3, 13, val);

		/* DRAM Tx NT ODT */
		val = (priv->dbsc5_board_config->bdcfg_tx_ntodt >> 4) & 0x1;
		/* PHY_APB_MR11_A0, PHY_APB_MR11_A1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_A0, 1, 19, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_A1, 1, 27, val);
		/* PHY_APB_MR11_B0, PHY_APB_MR11_B1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_B0, 1, 3, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR11_B1, 1, 11, val);

		/* DRAM Rx DRV */
		val = priv->dbsc5_board_config->bdcfg_rx_drv & 0x7;
		/* PHY_APB_MR3_A0, PHY_APB_MR3_A1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR3_A0, 3, 16, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR3_A1, 3, 24, val);
		/* PHY_APB_MR3_B0, PHY_APB_MR3_B1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR3_B0, 3, 0, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR3_B1, 3, 8, val);

		/* DRAM Rx Pre-emphasis */
		val = ((priv->dbsc5_board_config->bdcfg_rx_emphasis >> 12) & 0x3);
		val |= ((priv->dbsc5_board_config->bdcfg_rx_emphasis >> 8) & 0x3) << 2;
		val |= ((priv->dbsc5_board_config->bdcfg_rx_emphasis >> 4) & 0x3) << 4;
		val |= (priv->dbsc5_board_config->bdcfg_rx_emphasis & 0x3) << 6;
		/* PHY_APB_MR58_A0, PHY_APB_MR58_A1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR58_A0, 8, 16, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR58_A1, 8, 24, val);
		/* PHY_APB_MR58_B0, PHY_APB_MR58_B1 */
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR58_B0, 8, 0, val);
		ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR58_B1, 8, 8, val);

		/* Link ECC setting */
		if (priv->link_ecc_en) {
			/* PHY_APB_MR22_A0, PHY_APB_MR22_A1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR22_A0, 1, 20, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR22_A1, 1, 28, 0x1);
			/* PHY_APB_MR22_B0, PHY_APB_MR22_B1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR22_B0, 1, 4, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR22_B1, 1, 12, 0x1);
			/* PHY_APB_MR22_A0, PHY_APB_MR22_A1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR22_A0, 1, 22, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR22_A1, 1, 30, 0x1);
			/* PHY_APB_MR22_B0, PHY_APB_MR22_B1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR22_B0, 1, 6, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MR22_B1, 1, 14, 0x1);
			/* PHY_APB_ALT_RL, PHY_APB_MAIN_RL */
			if (priv->ddr_mbps < (7501 * priv->ddr_mbpsdiv)) {
				/* 6401 - 7500 */
				val = 0x5B;
			} else {
				/* 7501 - 8533 */
				val = 0x4F;
			}
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_ALT_RL, 8, 24, val);
			ddrphy_apb_rd_mod_wr(ch, PHY_APB_MAIN_RL, 8, 0, val);
		}
	}
}

/**
 * dbsc5_phy_data_load() - Load PHY data memory
 * @dev: DBSC5 device
 */
static void dbsc5_phy_data_load(struct udevice *dev)
{
	dbsc5_phy_apb_wr_ach(dev, PHY_DRTUB_STARTDCCMCLEAR, 0x0);
	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICROCONTMUXSEL, 0x0);
	dbsc5_rtdmac_phyinit_trans_ach(dev, DDR_PHY_DCCM_OFS,
				       (uintptr_t)DMAC_TRANS_DCCM,
				       ARRAY_SIZE(DMAC_TRANS_DCCM) * 4);
	dbsc5_ddr_smb_config(dev);
	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICROCONTMUXSEL, 0x1);
}

/**
 * dbsc5_phy_init_load() - Load PHY init memory
 * @dev: DBSC5 device
 */
static void dbsc5_phy_init_load(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ch, i;

	foreach_vphych(dev, ch)
		for (i = 0; i < ARRAY_SIZE(cpu_trans_tab4); i++)
			dbsc5_phy_apb_wr(ch, cpu_trans_tab4[i].addr, cpu_trans_tab4[i].data);

	if (priv->ddr_mbps < (6401 * priv->ddr_mbpsdiv)) /* 5501 - 6400 */
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL5, 0x2d56);

	foreach_vphych(dev, ch) {
		if ((priv->ch_have_this_cs[1] & (0x3 << (ch * 2))) != 0x0) {
			dbsc5_phy_apb_wr(ch, PHY_PPGC_HWTLPCSENA, 0x3);
			dbsc5_phy_apb_wr(ch, PHY_INITENG_SEQ0BGPR14, 0x3);
			dbsc5_phy_apb_wr(ch, PHY_PPGC_HWTLPCSENB, 0x3);
			dbsc5_phy_apb_wr(ch, PHY_INITENG_SEQ0BGPR15, 0x3);
		}
	}

	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICROCONTMUXSEL, 0);
	dbsc5_phy_apb_wr_ach(dev, 0x41000, 0);
	dbsc5_phy_apb_wr_ach(dev, 0x41001, 0);
	dbsc5_phy_apb_wr_ach(dev, 0x41002, 0);
	dbsc5_phy_apb_wr_ach(dev, 0x41003, 0);

	dbsc5_rtdmac_phyinit_trans_ach(dev, DDR_PHY_ACSM_OFS,
				       (uintptr_t)DMAC_TRANS_ACSM,
				       ARRAY_SIZE(DMAC_TRANS_ACSM) * 4);

	dbsc5_rtdmac_phyinit_trans_ach(dev, DDR_PHY_PIE_OFS,
				       (uintptr_t)DMAC_TRANS_PIE,
				       ARRAY_SIZE(DMAC_TRANS_PIE) * 4);

	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_SEQUENCEROVERRIDE, 0x600);
	dbsc5_phy_apb_wr_ach(dev, PHY_PPGC_ACSMNOPADDR, 0x0);

	dbsc5_rtdmac_phyinit_trans_ach(dev, PHY_PPGC_ACSMSTARTADDRXLATVAL0,
				       (uintptr_t)DATA_A, 64);

	foreach_vphych(dev, ch)
		for (i = 0; i < ARRAY_SIZE(cpu_trans_tab5); i++)
			dbsc5_phy_apb_wr(ch, cpu_trans_tab5[i].addr, cpu_trans_tab5[i].data);

	dbsc5_rtdmac_phyinit_trans_ach(dev, PHY_PPGC_ACSMSTOPADDRXLATVAL0,
				       (uintptr_t)DATA_B, 64);

	foreach_vphych(dev, ch)
		for (i = 0; i < ARRAY_SIZE(cpu_trans_tab6); i++)
			dbsc5_phy_apb_wr(ch, cpu_trans_tab6[i].addr, cpu_trans_tab6[i].data);

	if (priv->ddr_mbps < (6401 * priv->ddr_mbpsdiv))	/* 5501 - 6400 */
		dbsc5_phy_apb_wr_ach(dev, PHY_PPGC_ACSMRPTCNTOVERRIDE, 0x2);
	else if (priv->ddr_mbps < (7501 * priv->ddr_mbpsdiv))	/* 6401 - 7500 */
		dbsc5_phy_apb_wr_ach(dev, PHY_PPGC_ACSMRPTCNTOVERRIDE, 0x3);
	else if (priv->ddr_mbps < (8534 * priv->ddr_mbpsdiv)) {	/* 7501 - 8533 */
	} else							/* 8534 - 9600 */
		dbsc5_phy_apb_wr_ach(dev, PHY_PPGC_ACSMRPTCNTOVERRIDE, 0x5);

	dbsc5_rtdmac_phyinit_trans_ach(dev, 0x41004, (uintptr_t)ACSM_0, 384);

	if (priv->ddr_mbps < (6401 * priv->ddr_mbpsdiv)) {	/* 5501 - 6400 */
		dbsc5_phy_apb_wr_ach(dev, 0x41009, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41011, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41016, 0xD848);
		dbsc5_phy_apb_wr_ach(dev, 0x41019, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x4101E, 0xDDC8);
		dbsc5_phy_apb_wr_ach(dev, 0x41021, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41029, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41031, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41039, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41045, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x4104D, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41055, 0x4B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x4105D, 0x4B000000);
	} else if (priv->ddr_mbps < (7501 * priv->ddr_mbpsdiv)) {	/* 6401 - 7500 */
		dbsc5_phy_apb_wr_ach(dev, 0x41016, 0xE048);
		dbsc5_phy_apb_wr_ach(dev, 0x4101E, 0xE648);
	} else if (priv->ddr_mbps < (8534 * priv->ddr_mbpsdiv)) {	/* 7501 - 8533 */
		/* Nothing */
	} else {							/* 8534 - 9600 */
		dbsc5_phy_apb_wr_ach(dev, 0x41009, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41011, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41016, 0xF048);
		dbsc5_phy_apb_wr_ach(dev, 0x41019, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x4101E, 0xF748);
		dbsc5_phy_apb_wr_ach(dev, 0x41021, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41029, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41031, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41039, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41045, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x4104D, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x41055, 0x6B000000);
		dbsc5_phy_apb_wr_ach(dev, 0x4105D, 0x6B000000);
	}

	dbsc5_rtdmac_phyinit_trans_ach(dev, 0x4110C, (uintptr_t)ACSM_123, 384);
	dbsc5_rtdmac_phyinit_trans_ach(dev, 0x41214, (uintptr_t)ACSM_123, 384);
	dbsc5_rtdmac_phyinit_trans_ach(dev, 0x4131C, (uintptr_t)ACSM_123, 384);

	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_SEQUENCEROVERRIDE, 0x400);
	dbsc5_phy_apb_wr_ach(dev, PHY_DRTUB_PIEINITVECSEL, 0x5821);

	dbsc5_rtdmac_phyinit_trans_ach(dev, PHY_INITENG_SEQ0BDISABLEFLAG0,
				       (uintptr_t)DATA_C, 64);

	foreach_vphych(dev, ch)
		for (i = 0; i < ARRAY_SIZE(cpu_trans_tab7); i++)
			dbsc5_phy_apb_wr(ch, cpu_trans_tab7[i].addr, cpu_trans_tab7[i].data);

	dbsc5_ddr_config_post(dev);

	dbsc5_phy_apb_wr_ach(dev, PHY_DRTUB_UCCLKHCLKENABLES, 0x6);

	if (priv->ddr_mbps < (6481 * priv->ddr_mbpsdiv)) {		/* 5401 - 6480 */
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL1, 0x1C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL4, 0x1C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL5, 0x2D56);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG0, 0x1009);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG1, 0x5060);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG2, 0x6B01);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG3, 0x423);
	} else if (priv->ddr_mbps < (8537 * priv->ddr_mbpsdiv)) {	/* 6481 - 8536 */
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL1, 0x2C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL4, 0x2C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL5, 0x1956);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG0, 0x1009);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG1, 0x5060);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG2, 0x6B01);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG3, 0x423);
	} else {							/* 8537 - 9600 */
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL1, 0x2C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL4, 0x2C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL5, 0x1956);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG0, 0x1009);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG1, 0x5060);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG2, 0x6B01);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG3, 0x423);
	}

	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICROCONTMUXSEL, 0x1);
}

/**
 * dbsc5_phy_training() - Perform PHY training and read message block back
 * @dev: DBSC5 device
 */
static u32 dbsc5_phy_training(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 phytrainingok = 0;
	u32 fail_flag = 0;
	u32 ch, val;

	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICROCONTMUXSEL, 0x1);
	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICRORESET, 0x9);
	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICRORESET, 0x1);
	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICRORESET, 0x0);

	while ((phytrainingok != priv->ddr_phyvalid) && !fail_flag) {
		foreach_vphych(dev, ch) {
			val = dbsc5_phy_apb_msg_rd(ch);
			if (val == 0x7)		/* Training complete */
				phytrainingok |= BIT(ch);
			else if (val == 0xFF)	/* Training FAIL */
				fail_flag = 1;
		}
	}

	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICRORESET, 1);
	dbsc5_phy_apb_wr_ach(dev, PHY_APBONLY_MICROCONTMUXSEL, 0);

	return phytrainingok;
}

/**
 * dbsc5_ddr_config() - Configure DBSC5 registers
 * @dev: DBSC5 device
 */
static void dbsc5_ddr_config(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ch, ddr_ch, i, tmp, val;

	foreach_vphych(dev, ch) {
		/* Frequency settings */
		if (priv->ddr_mbps < (6401 * priv->ddr_mbpsdiv)) {		/* 5501 - 6400 */
			for (i = 0; i < ARRAY_SIZE(cpu_trans_tab3_6400); i++)
				dbsc5_phy_apb_wr(ch, cpu_trans_tab3_6400[i].addr, cpu_trans_tab3_6400[i].data);
		} else if (priv->ddr_mbps < (7501 * priv->ddr_mbpsdiv)) {	/* 6401 - 7500 */
			for (i = 0; i < ARRAY_SIZE(cpu_trans_tab3_7500); i++)
				dbsc5_phy_apb_wr(ch, cpu_trans_tab3_7500[i].addr, cpu_trans_tab3_7500[i].data);
		} else if (priv->ddr_mbps < (8534 * priv->ddr_mbpsdiv)) {	/* 7501 - 8533 */
			/* Nothing */
		} else {							/* 8534 - 9600 */
			for (i = 0; i < ARRAY_SIZE(cpu_trans_tab3_9600); i++)
				dbsc5_phy_apb_wr(ch, cpu_trans_tab3_9600[i].addr, cpu_trans_tab3_9600[i].data);
		}

		/* 2-Rank settings */
		if (priv->ch_have_this_cs[1] & (0x3 << (ch * 2))) {
			/* PHY_ACx_P0_PCLKACLNDIS[9] : Tx SEC lane1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_AC_PCLKACLNDIS(0), 1, 9, 0x0);
			ddrphy_apb_rd_mod_wr(ch, PHY_AC_PCLKACLNDIS(1), 1, 9, 0x0);
			/* PHY_ACx_P0_DFICLKACLNDIS[9] : Tx SEC lane1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_AC_DFICLKACLNDIS(0), 1, 9, 0x0);
			ddrphy_apb_rd_mod_wr(ch, PHY_AC_DFICLKACLNDIS(1), 1, 9, 0x0);
			/* PHY_ACx_P0_ACLNDISABLE[9] : Tx SEC lane1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_AC_ACLNDISABLE(0), 1, 9, 0x0);
			ddrphy_apb_rd_mod_wr(ch, PHY_AC_ACLNDISABLE(1), 1, 9, 0x0);
			/* PHY_DBYTEx_P0_PPTCTLSTATIC[1] : PptEnDqs2DqTg1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(0), 1, 1, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(1), 1, 1, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(2), 1, 1, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(3), 1, 1, 0x1);
			/* PHY_DBYTEx_P0_PPTCTLSTATIC[5] : PptEnWck2DqoTg1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(0), 1, 5, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(1), 1, 5, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(2), 1, 5, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(3), 1, 5, 0x1);
			/* PHY_DBYTEx_P0_PPTCTLSTATIC[9] : PptEnRxEnDlyTg1 */
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(0), 1, 9, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(1), 1, 9, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(2), 1, 9, 0x1);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(3), 1, 9, 0x1);
		}

		/* BOARD SETTINGS (CA, DQ, DMI) */
		/* CA_SWAP_CHA */
		ddr_ch = (ch * 2);
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_ca_swap & 0xfffffff;
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA0TODFI(0), val & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA1TODFI(0), (val >> 4) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA2TODFI(0), (val >> 8) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA3TODFI(0), (val >> 12) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA4TODFI(0), (val >> 16) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA5TODFI(0), (val >> 20) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA6TODFI(0), (val >> 24) & 0xf);

		/* DQ_SWAP_CHA_SLICE0 */
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dq_swap[0] & 0xffffffff;
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ0LNSEL(0), val & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ1LNSEL(0), (val >> 4) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ2LNSEL(0), (val >> 8) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ3LNSEL(0), (val >> 12) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ4LNSEL(0), (val >> 16) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ5LNSEL(0), (val >> 20) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ6LNSEL(0), (val >> 24) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ7LNSEL(0), (val >> 28) & 0xf);

		/* DM_SWAP_CHA_SLICE0 */
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dm_swap[0] & 0xf;
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ8LNSEL(0), val & 0xf);

		/* DQ_SWAP_CHA_SLICE1 */
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dq_swap[1] & 0xffffffff;
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ0LNSEL(1), val & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ1LNSEL(1), (val >> 4) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ2LNSEL(1), (val >> 8) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ3LNSEL(1), (val >> 12) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ4LNSEL(1), (val >> 16) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ5LNSEL(1), (val >> 20) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ6LNSEL(1), (val >> 24) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ7LNSEL(1), (val >> 28) & 0xf);

		/* DM_SWAP_CHA_SLICE1 */
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dm_swap[1] & 0xf;
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ8LNSEL(1), val & 0xf);

		/* CA_SWAP_CHB */
		ddr_ch = ((ch * 2) + 1);
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_ca_swap & 0xfffffff;
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA0TODFI(1), val & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA1TODFI(1), (val >> 4) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA2TODFI(1), (val >> 8) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA3TODFI(1), (val >> 12) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA4TODFI(1), (val >> 16) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA5TODFI(1), (val >> 20) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_AC_MAPCA6TODFI(1), (val >> 24) & 0xf);

		/* DQ_SWAP_CHB_SLICE0 */
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dq_swap[0] & 0xffffffff;
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ0LNSEL(2), val & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ1LNSEL(2), (val >> 4) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ2LNSEL(2), (val >> 8) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ3LNSEL(2), (val >> 12) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ4LNSEL(2), (val >> 16) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ5LNSEL(2), (val >> 20) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ6LNSEL(2), (val >> 24) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ7LNSEL(2), (val >> 28) & 0xf);

		/* DM_SWAP_CHB_SLICE0 */
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dm_swap[0] & 0xf;
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ8LNSEL(2), val & 0xf);

		/* DQ_SWAP_CHB_SLICE1 */
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dq_swap[1] & 0xffffffff;
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ0LNSEL(3), val & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ1LNSEL(3), (val >> 4) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ2LNSEL(3), (val >> 8) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ3LNSEL(3), (val >> 12) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ4LNSEL(3), (val >> 16) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ5LNSEL(3), (val >> 20) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ6LNSEL(3), (val >> 24) & 0xf);
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ7LNSEL(3), (val >> 28) & 0xf);

		/* DM_SWAP_CHB_SLICE1 */
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dm_swap[1] & 0xf;
		dbsc5_phy_apb_wr(ch, PHY_DBYTE_DQ8LNSEL(3), val & 0xf);

		/* BOARD SETTINGS (DATA_BYTE_SWAP) */
		priv->upper_lower_byte[ch] = 0x0;
		ddr_ch = ch * 2;
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dqs_swap;
		if ((val & 0xff) == 0x1) {
			/* PHY_DBYTEx_P0_PPTCTLSTATIC[3][2] : DOCByteSelTg1/0 */
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(0), 2, 2, 0x3);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(1), 2, 2, 0x0);
			priv->upper_lower_byte[ch] |= 0x3;
		}

		ddr_ch = (ch * 2) + 1;
		val = priv->dbsc5_board_config->ch[ddr_ch].bdcfg_dqs_swap;
		if ((val & 0xFF) == 0x1) {
			/* PHY_DBYTEx_P0_PPTCTLSTATIC[3][2] : DOCByteSelTg1/0 */
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(2), 2, 2, 0x3);
			ddrphy_apb_rd_mod_wr(ch, PHY_DBYTE_PPTCTLSTATIC(3), 2, 2, 0x0);
			priv->upper_lower_byte[ch] |= 0xc;
		}
	}

	/* Configure IO parameters */
	/* SOC Tx DRV for DQ */
	val = priv->dbsc5_board_config->bdcfg_tx_drv & 0xf;
	tmp = (val << 4) | val;
	for (i = 0; i < 8; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMDBYTE_TXIMPEDANCEDQ(i), tmp);

	/* SOC Tx DRV for DQS */
	val = (priv->dbsc5_board_config->bdcfg_tx_drv >> 4) & 0xF;
	tmp = (val << 8) | val;
	for (i = 0; i < 8; i += 2)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMDBYTE_TXIMPEDANCEDQS(i), tmp);

	/* SOC Tx DRV for WCK */
	val = (priv->dbsc5_board_config->bdcfg_tx_drv >> 8) & 0xF;
	tmp = (val << 12) | (val << 8) | (val << 4) | val;
	for (i = 0; i < 8; i += 2)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMDBYTE_TXIMPEDANCEDQS(i + 1), tmp);

	/* SOC Tx DRV for CK */
	val = (priv->dbsc5_board_config->bdcfg_tx_drv >> 12) & 0xF;
	tmp = (val << 4) | val;
	dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_TXIMPEDANCEAC(5),  tmp);
	dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_TXIMPEDANCEAC(12), tmp);

	/* SOC Tx DRV for AC */
	val = (priv->dbsc5_board_config->bdcfg_tx_drv >> 16) & 0xF;
	tmp = (val << 4) | val;
	for (i = 0; i < 4; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_TXIMPEDANCEAC(i),  tmp);
	for (i = 0; i < 4; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_TXIMPEDANCEAC(i + 7),  tmp);

	/* SOC Tx FFC for DQ */
	/* Rise */
	val = (priv->dbsc5_board_config->bdcfg_tx_ffc >> 28) & 0xF;
	tmp = val;
	/* Fall */
	val = (priv->dbsc5_board_config->bdcfg_tx_ffc >> 24) & 0xF;
	tmp |= (val << 4);
	for (i = 0; i < 8; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMDBYTE_TXDQSLEW(i), tmp);

	/* SOC Tx FFC for CA */
	/* Rise */
	val = (priv->dbsc5_board_config->bdcfg_tx_ffc >> 20) & 0xF;
	tmp = val;
	/* Fall */
	val = (priv->dbsc5_board_config->bdcfg_tx_ffc >> 16) & 0xF;
	tmp |= (val << 4);
	for (i = 0; i < 4; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_TXSLEWAC(i), tmp);
	for (i = 0; i < 4; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_TXSLEWAC(i + 7), tmp);

	/* SOC Tx FFC for CK */
	/* Rise */
	val = (priv->dbsc5_board_config->bdcfg_tx_ffc >> 12) & 0xF;
	tmp = val;
	/* Fall */
	val = (priv->dbsc5_board_config->bdcfg_tx_ffc >> 8)  & 0xF;
	tmp |= (val << 4);
	dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_TXSLEWAC(5),  tmp);
	dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_TXSLEWAC(12), tmp);

	/* SOC Rx ODT for DQ */
	val = priv->dbsc5_board_config->bdcfg_rx_odt & 0xF;
	tmp = val << 4;
	for (i = 0; i < 8; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMDBYTE_ODTIMPEDANCEDQ(i), tmp);

	/* SOC Rx ODT for DQS */
	val = (priv->dbsc5_board_config->bdcfg_rx_odt >> 4) & 0xF;
	tmp = (val << 12) | (val << 8);
	for (i = 0; i < 8; i += 2)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMDBYTE_ODTIMPEDANCEDQS(i), tmp);

	/* SOC Rx ODT for WCK */
	val = (priv->dbsc5_board_config->bdcfg_rx_odt >> 8) & 0xF;
	tmp = (val << 12) | (val << 8);
	for (i = 0; i < 8; i += 2)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMDBYTE_ODTIMPEDANCEDQS(i + 1), tmp);

	/* SOC Rx ODT for CK */
	val = (priv->dbsc5_board_config->bdcfg_rx_odt >> 12) & 0xF;
	tmp = val << 4;
	dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_ODTIMPEDANCEAC(5),  tmp);
	dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_ODTIMPEDANCEAC(12), tmp);

	/* SOC Rx ODT for AC */
	val = (priv->dbsc5_board_config->bdcfg_rx_odt >> 16) & 0xF;
	tmp = val << 4;
	for (i = 0; i < 5; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_ODTIMPEDANCEAC(i),  tmp);
	for (i = 0; i < 5; i++)
		dbsc5_phy_apb_wr_ach(dev, PHY_HMAC_ODTIMPEDANCEAC(i + 7),  tmp);

	/* Link ECC setting */
	if (priv->link_ecc_en)
		for (i = 0; i < 4; i++)
			dbsc5_phy_apb_wr_ach(dev, PHY_DBYTE_ENABLEWRITELINKECC(i), 0x1);
}

/**
 * dbsc5_phy_init() - Initialize DBSC5 PHY
 * @dev: DBSC5 device
 */
static void dbsc5_phy_init(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ch, i;

	/* Initialize PHY Configuration */
	foreach_vphych(dev, ch)
		for (i = 0; i < ARRAY_SIZE(cpu_trans_tab1); i++)
			dbsc5_phy_apb_wr(ch, cpu_trans_tab1[i].addr, cpu_trans_tab1[i].data);
	foreach_vphych(dev, ch)
		for (i = 0; i < ARRAY_SIZE(cpu_trans_tab2); i++)
			dbsc5_phy_apb_wr(ch, cpu_trans_tab2[i].addr, cpu_trans_tab2[i].data);

	if (priv->ddr_mbps < (6481 * priv->ddr_mbpsdiv)) {	/* 5401 - 6480 */
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL1, 0x808);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL4, 0x1C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL5, 0x2E9A);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG0, 0x1009);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG1, 0x5060);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG2, 0x6B01);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG3, 0x423);
	} else if (priv->ddr_mbps < (8537 * priv->ddr_mbpsdiv)) { /* 6481 - 8536 */
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL1, 0x1008);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL4, 0x2C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL5, 0x1A9A);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG0, 0x1009);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG1, 0x5060);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG2, 0x6B01);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG3, 0x423);
	} else {
		/* 8537 - 9600 */
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL1, 0x1008);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL4, 0x2C7F);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLCTRL5, 0x1A9A);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG0, 0x1009);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG1, 0x5060);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG2, 0x6B01);
		dbsc5_phy_apb_wr_ach(dev, PHY_HMMAS_CPLLUPLLPROG3, 0x423);
	}

	dbsc5_phy_apb_wr_ach(dev, PHY_DRTUB_UCCLKHCLKENABLES, 0x5);

	dbsc5_rtdmac_phyinit_trans_ach(dev, DDR_PHY_ICCM_OFS,
				       (uintptr_t)DMAC_TRANS_ICCM,
				       ARRAY_SIZE(DMAC_TRANS_ICCM) * 4);

	dbsc5_phy_apb_wr_ach(dev, PHY_DRTUB_UCCLKHCLKENABLES, 0x7);

	foreach_vphych(dev, ch)
		for (i = 0; i < ARRAY_SIZE(cpu_trans_tab3); i++)
			dbsc5_phy_apb_wr(ch, cpu_trans_tab3[i].addr, cpu_trans_tab3[i].data);

	dbsc5_rtdmac_phyinit_trans_ach(dev, PHY_PPGC_ACSMDONE, ((uintptr_t)DATA_3_1), 128);
	dbsc5_rtdmac_phyinit_trans_ach(dev, PHY_INITENG_SEQ0BGPR1, ((uintptr_t)DATA_3_2), 128);
	dbsc5_rtdmac_phyinit_trans_ach(dev, PHY_INITENG_SEQ0BDLY0, ((uintptr_t)DATA_3_3), 64);
	for (i = 0; i < 8; i += 2) {
		dbsc5_rtdmac_phyinit_trans_ach(dev, PHY_HMDBYTE_VREGCTRL1DQ(i),
					       (uintptr_t)DATA_3_4, 256);
		dbsc5_rtdmac_phyinit_trans_ach(dev, PHY_HMDBYTE_VREGCTRL1DQ(i + i),
					       (uintptr_t)DATA_3_5, 256);
	}

	dbsc5_ddr_config(dev);

	dbsc5_phy_apb_wr_ach(dev, PHY_DRTUB_STARTDCCMCLEAR, 0x1);
}

/**
 * dbsc5_dfi_wait_init_complete() - Wait DFI init completion
 * @dev: DBSC5 device
 */
static void dbsc5_dfi_wait_init_complete(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 retry = 0, phytrainingok = 0;
	const u32 RETRY_MAX = 0x10000;
	u32 ch, val;

	while (retry < RETRY_MAX) {
		foreach_vch(dev, ch) {
			val = readl(DBSC_DBDFISTAT(ch));
			if (val & 0x1)
				phytrainingok |= BIT(ch);
		}

		if (phytrainingok == priv->ddr_dramvalid)
			break;

		retry++;
	}
}

/**
 * dbsc5_dfi_init_start() - Start DFI initialization
 * @dev: DBSC5 device
 */
static void dbsc5_dfi_init_start(struct udevice *dev)
{
	u32 ch;

	foreach_vch(dev, ch)
		writel(0x21, DBSC_DBDFICNT(ch));

	dbsc5_dfi_wait_init_complete(dev);

	foreach_vch(dev, ch)
		writel(0x20, DBSC_DBDFICNT(ch));
}

/**
 * dbsc5_ddr_mode_register_set() - Set DDR mode registers
 * @dev: DBSC5 device
 */
static void dbsc5_ddr_mode_register_set(struct udevice *dev)
{
	dbsc5_send_dbcmd(dev, 0xE841C24);	/* MR28 */
	dbsc5_send_dbcmd(dev, 0xE842540);
	dbsc5_send_dbcmd(dev, 0xE842840);
}

/**
 * dbsc5_ddr_mode_register_read() - Read DDR mode registers
 * @dev: DBSC5 device
 */
static void dbsc5_ddr_mode_register_read(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ch, val;

	val = priv->dbsc5_board_config->bdcfg_rfm_chk;
	if (!val)
		return;

	dbsc5_send_dbcmd(dev, 0xF801B00);	/* MR27:0 */
	dbsc5_send_dbcmd(dev, 0xF803900);	/* MR57:0 */

	foreach_vphych(dev, ch) {
		if ((priv->ch_have_this_cs[1] & (0x3 << (ch * 2)))) {
			dbsc5_send_dbcmd_ch(ch, 0xF811B00);	/* MR27:1 */
			dbsc5_send_dbcmd_ch(ch, 0xF813900);	/* MR57:1 */
		}
	}
}

/**
 * dbsc5_phy_pll_lock_status_read() - Read PHY PLL lock status
 * @dev: DBSC5 device
 */
static u32 dbsc5_phy_pll_lock_status_read(struct udevice *dev)
{
	u32 ch, ret = 0;

	foreach_vphych(dev, ch)
		ret |= (readl(DBSC_DBPDSTAT01(ch)) & BIT(0)) << ch;

	return ret;
}

/**
 * dbsc5_ddr_initialize() - Initialize DDR
 * @dev: DBSC5 device
 */
static int dbsc5_ddr_initialize(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ddrphy_pll_lock_ok, phytrainingok;

	dbsc5_regset_unlock(dev);
	dbsc5_msres_status_set(dev, MODULE_STANDBY);
	dbsc5_init_pll3(dev);
	dbsc5_msres_status_set(dev, MODULE_RUN);
	dbsc5_phy_init(dev);
	dbsc5_dbsc_regset_pre(dev);
	dbsc5_phy_data_load(dev);
	phytrainingok = dbsc5_phy_training(dev);
	if (phytrainingok != priv->ddr_phyvalid)
		return -EINVAL;

	dbsc5_phy_init_load(dev);
	dbsc5_dfi_init_start(dev);
	ddrphy_pll_lock_ok = dbsc5_phy_pll_lock_status_read(dev);
	if (ddrphy_pll_lock_ok != priv->ddr_phyvalid)
		return -EINVAL;

	dbsc5_ddr_mode_register_set(dev);
	dbsc5_ddr_mode_register_read(dev);

	dbsc5_dbsc_regset_post(dev);
	dbsc5_regset_lock(dev);

	return 0;
}

/**
 * dbsc5_get_board_data() - Obtain board specific DRAM configuration
 * @dev: DBSC5 device
 *
 * Return board specific DRAM configuration structure pointer.
 */
__weak const struct renesas_dbsc5_board_config *
dbsc5_get_board_data(struct udevice *dev)
{
	return &renesas_x5h_dbsc5_board_config;
}

/**
 * renesas_dbsc5_dram_probe() - DDR Initialize entry
 * @dev: DBSC5 device
 *
 * Remove write protection on DBSC register. Read DDR configuration
 * information from driver data. Calculate board clock frequency and
 * operating frequency from DDR configuration information. Call the
 * main function of DDR initialization. Perform DBSC write protection
 * after initialization is complete.
 */
static int renesas_dbsc5_dram_probe(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);
	u32 ch, cs, i;

	priv->dbsc5_board_config = dbsc5_get_board_data(dev);

	/* Link ECC only on SoC > 1.0 */
	if (renesas_get_cpu_rev_integer() == 1 && renesas_get_cpu_rev_fraction() == 0)
		priv->link_ecc_en = 0;
	else
		priv->link_ecc_en = 1;

	priv->ddr_dramvalid = (u32)(priv->dbsc5_board_config->bdcfg_phyvalid);
	priv->ddr_phyvalid = 0x0;
	for (i = 0; i < DBSC5_PHYNUM_CNT; i++) {
		priv->ddr_phyvalid |= ((((priv->ddr_dramvalid >> (i * 2)) |
				       (priv->ddr_dramvalid >> ((i * 2) + 1))) & 0x1) << i);
	}

	for (cs = 0; cs < CS_CNT; cs++)
		priv->ch_have_this_cs[cs] = 0;

	foreach_vch(dev, ch)
		for (cs = 0; cs < CS_CNT; cs++)
			if (priv->dbsc5_board_config->ch[ch].bdcfg_ddr_density[cs] != 0xff)
				priv->ch_have_this_cs[cs] |= BIT(ch);

	/* Determine board clock frequency (in MHz) */
	priv->brd_clkdiv = 3;
	switch (readl(SYSSS_PLL6_CR0)) {
	case 0xa700000:	/* 50 / 3 = 16.66MHz */
		priv->brd_clk = 50;
		break;
	case 0x8b00000:	/* 60 / 3 = 20.00MHz */
		priv->brd_clk = 60;
		break;
	case 0xa700080:	/* 100 / 3 = 33.33MHz */
		priv->brd_clk = 100;
		break;
	default:	/* 50 / 3 = 16.66MHz */
		priv->brd_clk = 50;
		break;
	}

	priv->brd_clkdiva = !!(readl(SYSSS_PLL6_CR0) & BIT(7));

	/* Determine DDR operating frequency clock (in Mbps) */
	priv->ddr_mbps = 6400;
	priv->ddr_mbpsdiv = 1;

	/* Initialize DDR */
	return dbsc5_ddr_initialize(dev);
}

/**
 * renesas_dbsc5_dram_of_to_plat() - Convert OF data to plat data
 * @dev: DBSC5 device
 *
 * Extract DBSC5 address from DT and store it in driver data.
 */
static int renesas_dbsc5_dram_of_to_plat(struct udevice *dev)
{
	struct renesas_dbsc5_dram_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

/**
 * renesas_dbsc5_dram_get_info() - Return RAM size
 * @dev: DBSC5 device
 * @info: Output RAM info
 *
 * Return size of the RAM managed by this RAM driver.
 */
static int renesas_dbsc5_dram_get_info(struct udevice *dev,
				       struct ram_info *info)
{
	info->base = 0x40000000;
	info->size = 0;

	return 0;
}

static const struct ram_ops renesas_dbsc5_dram_ops = {
	.get_info = renesas_dbsc5_dram_get_info,
};

U_BOOT_DRIVER(renesas_dbsc5_dram) = {
	.name		= "dbsc5_dram",
	.id		= UCLASS_RAM,
	.of_to_plat	= renesas_dbsc5_dram_of_to_plat,
	.ops		= &renesas_dbsc5_dram_ops,
	.probe		= renesas_dbsc5_dram_probe,
	.priv_auto	= sizeof(struct renesas_dbsc5_dram_priv),
};
