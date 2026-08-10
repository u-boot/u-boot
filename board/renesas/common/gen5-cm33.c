// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 Renesas Electronics Corp.
 */

#include <asm/arch/renesas.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <dm/uclass.h>
#include <hang.h>
#include <linux/bitfield.h>
#include <linux/errno.h>
#include <linux/iopoll.h>
#include <lmb.h>

#include "gen5-cm33.h"

DECLARE_GLOBAL_DATA_PTR;

#define PKC_PROT_LOCK				0xa5a5a500
#define PKC_PROT_UNLOCK				0xa5a5a501

#define RSIP_BASE				0xe0000000
#define RSIP_NVIC_ISER_00			(RSIP_BASE + 0xe100)
#define RSIP_NVIC_ISER_00_SETENA_INTIWDTA0	BIT(16)
#define RSIP_NVIC_ICER_00			(RSIP_BASE + 0xe180)
#define RSIP_NVIC_ICER_01			(RSIP_BASE + 0xe184)
#define RSIP_NVIC_ICER_02			(RSIP_BASE + 0xe188)
#define RSIP_NVIC_ICER_03			(RSIP_BASE + 0xe18c)
#define RSIP_NVIC_ICER_04			(RSIP_BASE + 0xe190)
#define RSIP_NVIC_ICER_05			(RSIP_BASE + 0xe194)
#define RSIP_NVIC_ICER_06			(RSIP_BASE + 0xe198)
#define RSIP_NVIC_ICER_07			(RSIP_BASE + 0xe19c)
#define RSIP_NVIC_ICER_08			(RSIP_BASE + 0xe1a0)
#define RSIP_NVIC_ICER_09			(RSIP_BASE + 0xe1a4)
#define RSIP_NVIC_ICER_10			(RSIP_BASE + 0xe1a8)
#define RSIP_NVIC_ICER_11			(RSIP_BASE + 0xe1ac)
#define RSIP_NVIC_ICER_12			(RSIP_BASE + 0xe1b0)
#define RSIP_NVIC_ICER_13			(RSIP_BASE + 0xe1b4)
#define RSIP_NVIC_ICER_14			(RSIP_BASE + 0xe1b8)
#define RSIP_NVIC_ICER_15			(RSIP_BASE + 0xe1bc)
#define RSIP_SHCSR				(RSIP_BASE + 0xed24)
#define RSIP_SHCSR_USGFAULTENA			BIT(18)
#define RSIP_SHCSR_BUSFAULTENA			BIT(17)
#define RSIP_SHCSR_MEMFAULTENA			BIT(16)

#define RSIP_CTL_BASE				0x5fffd000
#define RSIP_CTL_CFG4				(RSIP_CTL_BASE + 0xb0)
#define RSIP_CTL_CFG4_OPWDEN			BIT(3)
#define RSIP_CTL_CFG4_OPWDVAC			BIT(5)
#define RSIP_CTL_ESICREMAP0			(RSIP_CTL_BASE + 0x70)
#define RSIP_CTL_PROT0PCMD			(RSIP_CTL_BASE + 0x840)
#define RSIP_CTL_PROT0PCMD_WREN			0xa5
#define RSIP_CTL_PROT0PS			(RSIP_CTL_BASE + 0x844)
#define RSIP_CTL_PROT0PS_ERR			BIT(0)

#define SYSSS_BASE				0xc1320000
#define SYSSS_MODE1				(SYSSS_BASE + 0x1010)
#define SYSSS_MODE1_MASK			GENMASK(11, 0)
#define SYSSS_MODE1_BAUDRATE_MASK		GENMASK(10, 9)
#define SYSSS_MODE1_BAUDRATE_921600		1
#define SYSSS_MODE1_BAUDRATE_1843200		2
#define SYSSS_MODE1_BAUDRATE_3250000		3
#define SYSSS_MODE1_BOOTMODE_MASK		GENMASK(20, 18)
#define SYSSS_MODE1_BOOTMODE_HF_DMA		2
#define SYSSS_MODE1_BOOTMODE_QSPI		4
#define SYSSS_MODE1_BOOTMODE_UFS		6

#define CLK_CONTROL_TOP_BASE			0xc6480000
#define CLK_CONTROL_SCP_BASE			0xc1330000
#define CLK_CONTROL_PERE_BASE			0xc08f0000

#define CLK_CONTROL_PLL01_0_CR0			(CLK_CONTROL_TOP_BASE + 0x1108)
#define CLK_CONTROL_PLL01_1_CR0			(CLK_CONTROL_TOP_BASE + 0x1114)
#define CLK_CONTROL_PLL02_0_CR0			(CLK_CONTROL_TOP_BASE + 0x1120)
#define CLK_CONTROL_PLL02_1_CR0			(CLK_CONTROL_TOP_BASE + 0x112c)
#define CLK_CONTROL_PLL02_2_CR0			(CLK_CONTROL_TOP_BASE + 0x1138)
#define CLK_CONTROL_PLL02_3_CR0			(CLK_CONTROL_TOP_BASE + 0x1144)
#define CLK_CONTROL_PLL02_4_CR0			(CLK_CONTROL_TOP_BASE + 0x1150)
#define CLK_CONTROL_PLL02_5_CR0			(CLK_CONTROL_TOP_BASE + 0x115c)
#define CLK_CONTROL_PLL02_6_CR0			(CLK_CONTROL_TOP_BASE + 0x1168)
#define CLK_CONTROL_PLL02_7_CR0			(CLK_CONTROL_TOP_BASE + 0x1174)
#define CLK_CONTROL_PLL03_0_CR0			(CLK_CONTROL_TOP_BASE + 0x1180)
#define CLK_CONTROL_PLL03_1_CR0			(CLK_CONTROL_TOP_BASE + 0x118c)
#define CLK_CONTROL_PLL03_2_CR0			(CLK_CONTROL_TOP_BASE + 0x1198)
#define CLK_CONTROL_PLL03_3_CR0			(CLK_CONTROL_TOP_BASE + 0x11a4)
#define CLK_CONTROL_PLL06_CR0			(CLK_CONTROL_TOP_BASE + 0x11c8)
#define CLK_CONTROL_PLL07_CR0			(CLK_CONTROL_TOP_BASE + 0x11d4)
#define CLK_CONTROL_PLL12_CR0			(CLK_CONTROL_TOP_BASE + 0x121c)
#define CLK_CONTROL_PLL13_CR0			(CLK_CONTROL_SCP_BASE + 0x1228)
#define CLK_CONTROL_PLL14_CR0			(CLK_CONTROL_TOP_BASE + 0x1234)
#define CLK_CONTROL_PLL15_0_CR0			(CLK_CONTROL_TOP_BASE + 0x1240)
#define CLK_CONTROL_PLL15_1_CR0			(CLK_CONTROL_TOP_BASE + 0x124c)
#define CLK_CONTROL_PLL15_2_CR0			(CLK_CONTROL_TOP_BASE + 0x1258)
#define CLK_CONTROL_PLL15_3_CR0			(CLK_CONTROL_TOP_BASE + 0x1264)
#define CLK_CONTROL_PLL01_0_CR1			(CLK_CONTROL_TOP_BASE + 0x110c)
#define CLK_CONTROL_PLL01_1_CR1			(CLK_CONTROL_TOP_BASE + 0x1118)
#define CLK_CONTROL_PLL02_0_CR1			(CLK_CONTROL_TOP_BASE + 0x1124)
#define CLK_CONTROL_PLL02_1_CR1			(CLK_CONTROL_TOP_BASE + 0x1130)
#define CLK_CONTROL_PLL02_2_CR1			(CLK_CONTROL_TOP_BASE + 0x113c)
#define CLK_CONTROL_PLL02_3_CR1			(CLK_CONTROL_TOP_BASE + 0x1148)
#define CLK_CONTROL_PLL02_4_CR1			(CLK_CONTROL_TOP_BASE + 0x1154)
#define CLK_CONTROL_PLL02_5_CR1			(CLK_CONTROL_TOP_BASE + 0x1160)
#define CLK_CONTROL_PLL02_6_CR1			(CLK_CONTROL_TOP_BASE + 0x116c)
#define CLK_CONTROL_PLL02_7_CR1			(CLK_CONTROL_TOP_BASE + 0x1178)
#define CLK_CONTROL_PLL03_0_CR1			(CLK_CONTROL_TOP_BASE + 0x1184)
#define CLK_CONTROL_PLL03_1_CR1			(CLK_CONTROL_TOP_BASE + 0x1190)
#define CLK_CONTROL_PLL03_2_CR1			(CLK_CONTROL_TOP_BASE + 0x119c)
#define CLK_CONTROL_PLL03_3_CR1			(CLK_CONTROL_TOP_BASE + 0x11a8)
#define CLK_CONTROL_PLL06_CR1			(CLK_CONTROL_TOP_BASE + 0x11cc)
#define CLK_CONTROL_PLL07_CR1			(CLK_CONTROL_TOP_BASE + 0x11d8)
#define CLK_CONTROL_PLL12_CR1			(CLK_CONTROL_TOP_BASE + 0x1220)
#define CLK_CONTROL_PLL13_CR1			(CLK_CONTROL_SCP_BASE + 0x122c)
#define CLK_CONTROL_PLL14_CR1			(CLK_CONTROL_TOP_BASE + 0x1238)
#define CLK_CONTROL_PLL15_0_CR1			(CLK_CONTROL_TOP_BASE + 0x1244)
#define CLK_CONTROL_PLL15_1_CR1			(CLK_CONTROL_TOP_BASE + 0x1250)
#define CLK_CONTROL_PLL15_2_CR1			(CLK_CONTROL_TOP_BASE + 0x125c)
#define CLK_CONTROL_PLL15_3_CR1			(CLK_CONTROL_TOP_BASE + 0x1268)
#define CLK_CONTROL_PLL01_0_CR2			(CLK_CONTROL_TOP_BASE + 0x1110)
#define CLK_CONTROL_PLL01_1_CR2			(CLK_CONTROL_TOP_BASE + 0x111c)
#define CLK_CONTROL_PLL02_0_CR2			(CLK_CONTROL_TOP_BASE + 0x1128)
#define CLK_CONTROL_PLL02_1_CR2			(CLK_CONTROL_TOP_BASE + 0x1134)
#define CLK_CONTROL_PLL02_2_CR2			(CLK_CONTROL_TOP_BASE + 0x1140)
#define CLK_CONTROL_PLL02_3_CR2			(CLK_CONTROL_TOP_BASE + 0x114c)
#define CLK_CONTROL_PLL02_4_CR2			(CLK_CONTROL_TOP_BASE + 0x1158)
#define CLK_CONTROL_PLL02_5_CR2			(CLK_CONTROL_TOP_BASE + 0x1164)
#define CLK_CONTROL_PLL02_6_CR2			(CLK_CONTROL_TOP_BASE + 0x1170)
#define CLK_CONTROL_PLL02_7_CR2			(CLK_CONTROL_TOP_BASE + 0x117c)
#define CLK_CONTROL_PLL03_0_CR2			(CLK_CONTROL_TOP_BASE + 0x1188)
#define CLK_CONTROL_PLL03_1_CR2			(CLK_CONTROL_TOP_BASE + 0x1194)
#define CLK_CONTROL_PLL03_2_CR2			(CLK_CONTROL_TOP_BASE + 0x11a0)
#define CLK_CONTROL_PLL03_3_CR2			(CLK_CONTROL_TOP_BASE + 0x11ac)
#define CLK_CONTROL_PLL06_CR2			(CLK_CONTROL_TOP_BASE + 0x11d0)
#define CLK_CONTROL_PLL07_CR2			(CLK_CONTROL_TOP_BASE + 0x11dc)
#define CLK_CONTROL_PLL12_CR2			(CLK_CONTROL_TOP_BASE + 0x1224)
#define CLK_CONTROL_PLL13_CR2			(CLK_CONTROL_SCP_BASE + 0x1230)
#define CLK_CONTROL_PLL14_CR2			(CLK_CONTROL_TOP_BASE + 0x123c)
#define CLK_CONTROL_PLL15_0_CR2			(CLK_CONTROL_TOP_BASE + 0x1248)
#define CLK_CONTROL_PLL15_1_CR2			(CLK_CONTROL_TOP_BASE + 0x1254)
#define CLK_CONTROL_PLL15_2_CR2			(CLK_CONTROL_TOP_BASE + 0x1260)
#define CLK_CONTROL_PLL15_3_CR2			(CLK_CONTROL_TOP_BASE + 0x126c)
#define CLK_CONTROL_PLL01_0_SCR			(CLK_CONTROL_TOP_BASE + 0x1270)
#define CLK_CONTROL_PLL01_1_SCR			(CLK_CONTROL_TOP_BASE + 0x1278)
#define CLK_CONTROL_PLL02_0_SCR			(CLK_CONTROL_TOP_BASE + 0x1280)
#define CLK_CONTROL_PLL02_1_SCR			(CLK_CONTROL_TOP_BASE + 0x1288)
#define CLK_CONTROL_PLL02_2_SCR			(CLK_CONTROL_TOP_BASE + 0x1290)
#define CLK_CONTROL_PLL02_3_SCR			(CLK_CONTROL_TOP_BASE + 0x1298)
#define CLK_CONTROL_PLL02_4_SCR			(CLK_CONTROL_TOP_BASE + 0x12a0)
#define CLK_CONTROL_PLL02_5_SCR			(CLK_CONTROL_TOP_BASE + 0x12a8)
#define CLK_CONTROL_PLL02_6_SCR			(CLK_CONTROL_TOP_BASE + 0x12b0)
#define CLK_CONTROL_PLL02_7_SCR			(CLK_CONTROL_TOP_BASE + 0x12b8)
#define CLK_CONTROL_PLL03_0_SCR			(CLK_CONTROL_TOP_BASE + 0x12c0)
#define CLK_CONTROL_PLL03_1_SCR			(CLK_CONTROL_TOP_BASE + 0x12c8)
#define CLK_CONTROL_PLL03_2_SCR			(CLK_CONTROL_TOP_BASE + 0x12d0)
#define CLK_CONTROL_PLL03_3_SCR			(CLK_CONTROL_TOP_BASE + 0x12d8)
#define CLK_CONTROL_PLL06_SCR			(CLK_CONTROL_TOP_BASE + 0x12f0)
#define CLK_CONTROL_PLL07_SCR			(CLK_CONTROL_TOP_BASE + 0x12f8)
#define CLK_CONTROL_PLL12_SCR			(CLK_CONTROL_TOP_BASE + 0x1328)
#define CLK_CONTROL_PLL13_SCR			(CLK_CONTROL_SCP_BASE + 0x1330)
#define CLK_CONTROL_PLL14_SCR			(CLK_CONTROL_TOP_BASE + 0x1338)
#define CLK_CONTROL_PLL15_0_SCR			(CLK_CONTROL_TOP_BASE + 0x1340)
#define CLK_CONTROL_PLL15_1_SCR			(CLK_CONTROL_TOP_BASE + 0x1348)
#define CLK_CONTROL_PLL15_2_SCR			(CLK_CONTROL_TOP_BASE + 0x1350)
#define CLK_CONTROL_PLL15_3_SCR			(CLK_CONTROL_TOP_BASE + 0x1358)
#define CLK_CONTROL_PLL01_0_DCR			(CLK_CONTROL_TOP_BASE + 0x1274)
#define CLK_CONTROL_PLL01_1_DCR			(CLK_CONTROL_TOP_BASE + 0x127c)
#define CLK_CONTROL_PLL02_0_DCR			(CLK_CONTROL_TOP_BASE + 0x1284)
#define CLK_CONTROL_PLL02_1_DCR			(CLK_CONTROL_TOP_BASE + 0x128c)
#define CLK_CONTROL_PLL02_2_DCR			(CLK_CONTROL_TOP_BASE + 0x1294)
#define CLK_CONTROL_PLL02_3_DCR			(CLK_CONTROL_TOP_BASE + 0x129c)
#define CLK_CONTROL_PLL02_4_DCR			(CLK_CONTROL_TOP_BASE + 0x12a4)
#define CLK_CONTROL_PLL02_5_DCR			(CLK_CONTROL_TOP_BASE + 0x12ac)
#define CLK_CONTROL_PLL02_6_DCR			(CLK_CONTROL_TOP_BASE + 0x12b4)
#define CLK_CONTROL_PLL02_7_DCR			(CLK_CONTROL_TOP_BASE + 0x12bc)
#define CLK_CONTROL_PLL03_0_DCR			(CLK_CONTROL_TOP_BASE + 0x12c4)
#define CLK_CONTROL_PLL03_1_DCR			(CLK_CONTROL_TOP_BASE + 0x12cc)
#define CLK_CONTROL_PLL03_2_DCR			(CLK_CONTROL_TOP_BASE + 0x12d4)
#define CLK_CONTROL_PLL03_3_DCR			(CLK_CONTROL_TOP_BASE + 0x12dc)
#define CLK_CONTROL_PLL06_DCR			(CLK_CONTROL_TOP_BASE + 0x12f4)
#define CLK_CONTROL_PLL07_DCR			(CLK_CONTROL_TOP_BASE + 0x12fc)
#define CLK_CONTROL_PLL12_DCR			(CLK_CONTROL_TOP_BASE + 0x132c)
#define CLK_CONTROL_PLL13_DCR			(CLK_CONTROL_SCP_BASE + 0x1334)
#define CLK_CONTROL_PLL14_DCR			(CLK_CONTROL_TOP_BASE + 0x133c)
#define CLK_CONTROL_PLL15_0_DCR			(CLK_CONTROL_TOP_BASE + 0x1344)
#define CLK_CONTROL_PLL15_1_DCR			(CLK_CONTROL_TOP_BASE + 0x134c)
#define CLK_CONTROL_PLL15_2_DCR			(CLK_CONTROL_TOP_BASE + 0x1354)
#define CLK_CONTROL_PLL15_3_DCR			(CLK_CONTROL_TOP_BASE + 0x135c)
#define CLK_CONTROL_RPCCKCR			(CLK_CONTROL_PERE_BASE + 0x1030)
#define CLK_CONTROL_RPCCKCR_CKSTP1		BIT(8)
#define CLK_CONTROL_RPCCKCR_RPCFC_MASK		GENMASK(4, 0)
#define CLK_CONTROL_PROT_REG_MASK		0xffff
#define CLK_CONTROL_PROT_REG			0x1370

#define CLK_CONTROL_PLL_CR2_PLLCLKSTAB		BIT(31)
#define CLK_CONTROL_PLL_CR2_PLLDISTRG		BIT(29)
#define CLK_CONTROL_PLL_CR2_PLLENTRG		BIT(28)
#define CLK_CONTROL_PLL_SCR_SELID_CLK_IOSC	BIT(0)
#define CLK_CONTROL_PLL_SCR_SELACT_MASK		BIT(16)
#define CLK_CONTROL_PLL_DCR_PLLDIVSYNC		BIT(16)

/**
 * sysss_read_modemr() - Read MODE Register 1
 * @return: MD[11:0] pin state
 */
static u32 sysss_read_modemr(void)
{
	return readl(SYSSS_MODE1) & SYSSS_MODE1_MASK;
}

/**
 * clk_control_poll_cr2() - Poll CR2 until PLL is stable
 * @cr2: PLL CR2 register address
 */
static void clk_control_poll_cr2(const u32 cr2)
{
	u32 val;

	/* This can not use readl_poll_timeout(), timer is not available yet. */
	for (;;) {
		val = readl(cr2);
		if (val & CLK_CONTROL_PLL_CR2_PLLCLKSTAB)
			break;
	}
}

/**
 * clk_control_write_cr() - Write protected clock controller register
 * @reg: Register address
 * @val: Value to be written
 */
static void clk_control_write_cr(const u32 reg, const u32 val)
{
	const u32 protreg = (reg & ~CLK_CONTROL_PROT_REG_MASK) | CLK_CONTROL_PROT_REG;

	writel(PKC_PROT_UNLOCK, protreg);
	writel(val, reg);
	writel(PKC_PROT_LOCK, protreg);
}

/**
 * clk_control_write_and_poll_dcr() - Write and poll DCR
 * @dcr: DCR register address
 * @val: Value to be written
 */
static void clk_control_write_and_poll_dcr(const u32 dcr, const u32 val)
{
	u32 tmp;

	clk_control_write_cr(dcr, val);

	/* This can not use readl_poll_timeout(), timer is not available yet. */
	for (;;) {
		tmp = readl(dcr);
		if (tmp & CLK_CONTROL_PLL_DCR_PLLDIVSYNC)
			break;
	}
}

/**
 * clk_control_switch_from_iosc_to_pll() - Switch clock from internal oscillator to PLL
 * @cr2: PLL CR2 register address
 * @scr: PLL SCR register address
 */
static void clk_control_switch_from_iosc_to_pll(const u32 cr2, const u32 scr)
{
	u32 val;

	clk_control_poll_cr2(cr2);

	/* Switch from internal oscillator to PLL. */
	val = readl(scr);
	val &= ~CLK_CONTROL_PLL_SCR_SELID_CLK_IOSC;
	clk_control_write_cr(scr, val);

	/* This can not use readl_poll_timeout(), timer is not available yet. */
	for (;;) {
		val = readl(scr);
		if (!(val & CLK_CONTROL_PLL_SCR_SELACT_MASK))
			break;
	}
}

/**
 * clk_control_switch_from_pll_to_iosc() - Switch clock from PLL to internal oscillator
 * @cr2: PLL CR2 register address
 * @scr: PLL SCR register address
 */
static void clk_control_switch_from_pll_to_iosc(const u32 cr2, const u32 scr)
{
	u32 val;

	clk_control_poll_cr2(cr2);

	/* Switch from PLL to internal oscillator. */
	clk_control_write_cr(scr, CLK_CONTROL_PLL_SCR_SELID_CLK_IOSC);

	/* This can not use readl_poll_timeout(), timer is not available yet. */
	for (;;) {
		val = readl(scr);
		if (val & CLK_CONTROL_PLL_SCR_SELACT_MASK)
			break;
	}
}

/**
 * clk_control_set_pll_freq() - Set PLL frequency
 * @cr0: PLL CR0 register address
 * @cr1: PLL CR1 register address
 * @cr2: PLL CR2 register address
 * @scr: PLL SCR register address
 * @dcr: PLL DCR register address
 * @cr0_val: PLL CR0 register value
 * @cr1_val: PLL CR1 register value
 * @dcr_val: PLL DCR register value
 */
static void clk_control_set_pll_freq(const u32 cr0, const u32 cr1, const u32 cr2,
				     const u32 scr, const u32 dcr, const u32 cr0_val,
				     const u32 cr1_val, const u32 dcr_val)
{
	u32 val;

	clk_control_switch_from_pll_to_iosc(cr2, scr);

	/* Disable PLL trigger and wait until it unlocks */
	clk_control_write_cr(cr2, CLK_CONTROL_PLL_CR2_PLLDISTRG);
	/* This can not use readl_poll_timeout(), timer is not available yet. */
	for (;;) {
		val = readl(cr2);
		if (!(val & CLK_CONTROL_PLL_CR2_PLLCLKSTAB))
			break;
	}

	clk_control_write_cr(cr0, cr0_val);
	clk_control_write_cr(cr1, cr1_val);
	clk_control_write_cr(dcr, dcr_val);

	/* Enable PLL */
	clk_control_write_cr(cr2, CLK_CONTROL_PLL_CR2_PLLENTRG);
}

/**
 * clk_control_set_pll() - Load configuration into PLLs
 */
static void clk_control_set_pll(void)
{
	/* Switch PLLs to internal oscillator and configure dividers. */
	clk_control_set_pll_freq(CLK_CONTROL_PLL01_0_CR0, CLK_CONTROL_PLL01_0_CR1,
				 CLK_CONTROL_PLL01_0_CR2, CLK_CONTROL_PLL01_0_SCR,
				 CLK_CONTROL_PLL01_0_DCR, 0x06500000, 0x00000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL01_1_CR0, CLK_CONTROL_PLL01_1_CR1,
				 CLK_CONTROL_PLL01_1_CR2, CLK_CONTROL_PLL01_1_SCR,
				 CLK_CONTROL_PLL01_1_DCR, 0x04c00000, 0x00000000, 0x18);

	clk_control_set_pll_freq(CLK_CONTROL_PLL02_0_CR0, CLK_CONTROL_PLL02_0_CR1,
				 CLK_CONTROL_PLL02_0_CR2, CLK_CONTROL_PLL02_0_SCR,
				 CLK_CONTROL_PLL02_0_DCR, 0x08900000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL02_1_CR0, CLK_CONTROL_PLL02_1_CR1,
				 CLK_CONTROL_PLL02_1_CR2, CLK_CONTROL_PLL02_1_SCR,
				 CLK_CONTROL_PLL02_1_DCR, 0x08900000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL02_2_CR0, CLK_CONTROL_PLL02_2_CR1,
				 CLK_CONTROL_PLL02_2_CR2, CLK_CONTROL_PLL02_2_SCR,
				 CLK_CONTROL_PLL02_2_DCR, 0x08900000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL02_3_CR0, CLK_CONTROL_PLL02_3_CR1,
				 CLK_CONTROL_PLL02_3_CR2, CLK_CONTROL_PLL02_3_SCR,
				 CLK_CONTROL_PLL02_3_DCR, 0x08900000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL02_4_CR0, CLK_CONTROL_PLL02_4_CR1,
				 CLK_CONTROL_PLL02_4_CR2, CLK_CONTROL_PLL02_4_SCR,
				 CLK_CONTROL_PLL02_4_DCR, 0x08900000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL02_5_CR0, CLK_CONTROL_PLL02_5_CR1,
				 CLK_CONTROL_PLL02_5_CR2, CLK_CONTROL_PLL02_5_SCR,
				 CLK_CONTROL_PLL02_5_DCR, 0x08900000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL02_6_CR0, CLK_CONTROL_PLL02_6_CR1,
				 CLK_CONTROL_PLL02_6_CR2, CLK_CONTROL_PLL02_6_SCR,
				 CLK_CONTROL_PLL02_6_DCR, 0x08900000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL02_7_CR0, CLK_CONTROL_PLL02_7_CR1,
				 CLK_CONTROL_PLL02_7_CR2, CLK_CONTROL_PLL02_7_SCR,
				 CLK_CONTROL_PLL02_7_DCR, 0x08900000, 0x04000000, 0x18);

	clk_control_set_pll_freq(CLK_CONTROL_PLL03_0_CR0, CLK_CONTROL_PLL03_0_CR1,
				 CLK_CONTROL_PLL03_0_CR2, CLK_CONTROL_PLL03_0_SCR,
				 CLK_CONTROL_PLL03_0_DCR, 0x05f00000, 0x00000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL03_1_CR0, CLK_CONTROL_PLL03_1_CR1,
				 CLK_CONTROL_PLL03_1_CR2, CLK_CONTROL_PLL03_1_SCR,
				 CLK_CONTROL_PLL03_1_DCR, 0x05f00000, 0x00000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL03_2_CR0, CLK_CONTROL_PLL03_2_CR1,
				 CLK_CONTROL_PLL03_2_CR2, CLK_CONTROL_PLL03_2_SCR,
				 CLK_CONTROL_PLL03_2_DCR, 0x05f00000, 0x00000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL03_3_CR0, CLK_CONTROL_PLL03_3_CR1,
				 CLK_CONTROL_PLL03_3_CR2, CLK_CONTROL_PLL03_3_SCR,
				 CLK_CONTROL_PLL03_3_DCR, 0x05f00000, 0x00000000, 0x18);

	clk_control_set_pll_freq(CLK_CONTROL_PLL06_CR0, CLK_CONTROL_PLL06_CR1,
				 CLK_CONTROL_PLL06_CR2, CLK_CONTROL_PLL06_SCR,
				 CLK_CONTROL_PLL06_DCR, 0x0a700000, 0x0c000000, 0x18);

	clk_control_set_pll_freq(CLK_CONTROL_PLL07_CR0, CLK_CONTROL_PLL07_CR1,
				 CLK_CONTROL_PLL07_CR2, CLK_CONTROL_PLL07_SCR,
				 CLK_CONTROL_PLL07_DCR, 0x09500000, 0x08000000, 0x18);

	clk_control_set_pll_freq(CLK_CONTROL_PLL12_CR0, CLK_CONTROL_PLL12_CR1,
				 CLK_CONTROL_PLL12_CR2, CLK_CONTROL_PLL12_SCR,
				 CLK_CONTROL_PLL12_DCR, 0x0a400000, 0x0c000000, 0x18);

	clk_control_set_pll_freq(CLK_CONTROL_PLL13_CR0, CLK_CONTROL_PLL13_CR1,
				 CLK_CONTROL_PLL13_CR2, CLK_CONTROL_PLL13_SCR,
				 CLK_CONTROL_PLL13_DCR, 0x08f00000, 0x08000000, 0x18);

	clk_control_set_pll_freq(CLK_CONTROL_PLL14_CR0, CLK_CONTROL_PLL14_CR1,
				 CLK_CONTROL_PLL14_CR2, CLK_CONTROL_PLL14_SCR,
				 CLK_CONTROL_PLL14_DCR, 0x04700000, 0x00000000, 0x18);

	clk_control_set_pll_freq(CLK_CONTROL_PLL15_0_CR0, CLK_CONTROL_PLL15_0_CR1,
				 CLK_CONTROL_PLL15_0_CR2, CLK_CONTROL_PLL15_0_SCR,
				 CLK_CONTROL_PLL15_0_DCR, 0x07700000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL15_1_CR0, CLK_CONTROL_PLL15_1_CR1,
				 CLK_CONTROL_PLL15_1_CR2, CLK_CONTROL_PLL15_1_SCR,
				 CLK_CONTROL_PLL15_1_DCR, 0x07700000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL15_2_CR0, CLK_CONTROL_PLL15_2_CR1,
				 CLK_CONTROL_PLL15_2_CR2, CLK_CONTROL_PLL15_2_SCR,
				 CLK_CONTROL_PLL15_2_DCR, 0x07700000, 0x04000000, 0x18);
	clk_control_set_pll_freq(CLK_CONTROL_PLL15_3_CR0, CLK_CONTROL_PLL15_3_CR1,
				 CLK_CONTROL_PLL15_3_CR2, CLK_CONTROL_PLL15_3_SCR,
				 CLK_CONTROL_PLL15_3_DCR, 0x07700000, 0x04000000, 0x18);

	/* Switch PLLs back and wait for them to stabilize. */
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL01_0_CR2, CLK_CONTROL_PLL01_0_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL01_1_CR2, CLK_CONTROL_PLL01_1_SCR);

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL02_0_CR2, CLK_CONTROL_PLL02_0_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL02_1_CR2, CLK_CONTROL_PLL02_1_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL02_2_CR2, CLK_CONTROL_PLL02_2_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL02_3_CR2, CLK_CONTROL_PLL02_3_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL02_4_CR2, CLK_CONTROL_PLL02_4_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL02_5_CR2, CLK_CONTROL_PLL02_5_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL02_6_CR2, CLK_CONTROL_PLL02_6_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL02_7_CR2, CLK_CONTROL_PLL02_7_SCR);

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL03_0_CR2, CLK_CONTROL_PLL03_0_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL03_1_CR2, CLK_CONTROL_PLL03_1_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL03_2_CR2, CLK_CONTROL_PLL03_2_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL03_3_CR2, CLK_CONTROL_PLL03_3_SCR);

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL06_CR2, CLK_CONTROL_PLL06_SCR);

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL07_CR2, CLK_CONTROL_PLL07_SCR);

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL12_CR2, CLK_CONTROL_PLL12_SCR);

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL13_CR2, CLK_CONTROL_PLL13_SCR);

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL14_CR2, CLK_CONTROL_PLL14_SCR);

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL15_0_CR2, CLK_CONTROL_PLL15_0_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL15_1_CR2, CLK_CONTROL_PLL15_1_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL15_2_CR2, CLK_CONTROL_PLL15_2_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL15_3_CR2, CLK_CONTROL_PLL15_3_SCR);

	/* Write second-stage DCR */
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL01_0_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL01_1_DCR, 0x10);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_0_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_1_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_2_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_3_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_4_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_5_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_6_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_7_DCR, 0x10);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL03_0_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL03_1_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL03_2_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL03_3_DCR, 0x10);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL06_DCR, 0x10);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL07_DCR, 0x10);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL12_DCR, 0x10);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL13_DCR, 0x10);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL14_DCR, 0x10);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL15_0_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL15_1_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL15_2_DCR, 0x10);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL15_3_DCR, 0x10);

	udelay(25);

	/* Write third-stage DCR */
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL01_0_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL01_1_DCR, 0x00);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_0_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_1_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_2_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_3_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_4_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_5_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_6_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL02_7_DCR, 0x00);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL03_0_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL03_1_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL03_2_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL03_3_DCR, 0x00);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL06_DCR, 0x10);	/* 1/2 */

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL07_DCR, 0x00);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL12_DCR, 0x00);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL13_DCR, 0x00);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL14_DCR, 0x00);

	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL15_0_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL15_1_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL15_2_DCR, 0x00);
	clk_control_write_and_poll_dcr(CLK_CONTROL_PLL15_3_DCR, 0x00);

	udelay(25);
}

#define MDLC_PERW_BASE				0xc05d0000
#define MDLC_PERE_BASE				0xc08f0000
#define MDLC_HSCN_BASE				0xc9c90000
/* The addresses in range 0x08000000..0x1fffffff are incremented by 0xa0000000 */
#define MDLC_RT_BASE				0xb9440000
#define MDLC_SCP_BASE				0xc1330000

#define MDLC_SCP_MSRES02			(MDLC_SCP_BASE + 0x908)
#define MDLC_SCP_MSRESS02			(MDLC_SCP_BASE + 0x968)
#define MDLC_SCP_MSRES02_SCP_MASK		GENMASK(1, 0)
#define MDLC_SCP_PKCPROT1			(MDLC_SCP_BASE + 0xcf4)

#define MDLC_PERW_MSRES05			(MDLC_PERW_BASE + 0x914)
#define MDLC_PERW_MSRESS05			(MDLC_PERW_BASE + 0x974)
#define MDLC_PERW_MSRES05_HSCIF0_MASK		GENMASK(9, 8)
#define MDLC_PERW_MSRES05_HSCIF1_MASK		GENMASK(11, 10)
#define MDLC_MSRESS_STANDBY			0
#define MDLC_MSRESS_RESET			1
#define MDLC_MSRESS_STOP			2
#define MDLC_MSRESS_RUN				3
#define MDLC_PERW_PKCPROT1			(MDLC_PERW_BASE + 0xcf4)

#define MDLC_MPG_GATING				0
#define MDLC_MPG_RESET				1
#define MDLC_MPG_RUN				3

#define MDLC_PERE_MPIER				(MDLC_PERE_BASE + 0x110)
#define MDLC_PERE_MPIMR				(MDLC_PERE_BASE + 0x120)
#define MDLC_PERE_MPDG00			(MDLC_PERE_BASE + 0x200)
#define MDLC_PERE_MPDG01			(MDLC_PERE_BASE + 0x204)
#define MDLC_PERE_MPDGS00			(MDLC_PERE_BASE + 0x300)
#define MDLC_PERE_MPDGS01			(MDLC_PERE_BASE + 0x304)
#define MDLC_PERE_MSRES06			(MDLC_PERE_BASE + 0x918)
#define MDLC_PERE_MSRESS06			(MDLC_PERE_BASE + 0x978)
#define MDLC_PERE_PKCPROT0			(MDLC_PERE_BASE + 0xcf0)
#define MDLC_PERE_PKCPROT1			(MDLC_PERE_BASE + 0xcf4)
#define MDLC_PERE_MSRES06_UFS0_MASK		GENMASK(1, 0)
#define MDLC_PERE_MSRES06_UFS1_MASK		GENMASK(3, 2)

#define MDLC_HSCN_MPIER				(MDLC_HSCN_BASE + 0x110)
#define MDLC_HSCN_MPIMR				(MDLC_HSCN_BASE + 0x120)
#define MDLC_HSCN_MPDG00			(MDLC_HSCN_BASE + 0x200)
#define MDLC_HSCN_MPDG01			(MDLC_HSCN_BASE + 0x204)
#define MDLC_HSCN_MPDG02			(MDLC_HSCN_BASE + 0x208)
#define MDLC_HSCN_MPDG03			(MDLC_HSCN_BASE + 0x20c)
#define MDLC_HSCN_MPDG04			(MDLC_HSCN_BASE + 0x210)
#define MDLC_HSCN_MPDG05			(MDLC_HSCN_BASE + 0x214)
#define MDLC_HSCN_MPDG06			(MDLC_HSCN_BASE + 0x218)
#define MDLC_HSCN_MPDGS00			(MDLC_HSCN_BASE + 0x300)
#define MDLC_HSCN_MPDGS01			(MDLC_HSCN_BASE + 0x304)
#define MDLC_HSCN_MPDGS02			(MDLC_HSCN_BASE + 0x308)
#define MDLC_HSCN_MPDGS03			(MDLC_HSCN_BASE + 0x30c)
#define MDLC_HSCN_MPDGS04			(MDLC_HSCN_BASE + 0x310)
#define MDLC_HSCN_MPDGS05			(MDLC_HSCN_BASE + 0x314)
#define MDLC_HSCN_MPDGS06			(MDLC_HSCN_BASE + 0x318)
#define MDLC_HSCN_PKCPROT0			(MDLC_HSCN_BASE + 0xcf0)

#define MDLC_RT_MPIER				(MDLC_RT_BASE + 0x110)
#define MDLC_RT_MPIMR				(MDLC_RT_BASE + 0x120)
#define MDLC_RT_MPDG00				(MDLC_RT_BASE + 0x200)
#define MDLC_RT_MPDG01				(MDLC_RT_BASE + 0x204)
#define MDLC_RT_MPDG02				(MDLC_RT_BASE + 0x208)
#define MDLC_RT_MPDG03				(MDLC_RT_BASE + 0x20c)
#define MDLC_RT_MPDG04				(MDLC_RT_BASE + 0x210)
#define MDLC_RT_MPDG05				(MDLC_RT_BASE + 0x214)
#define MDLC_RT_MPDG06				(MDLC_RT_BASE + 0x218)
#define MDLC_RT_MPDG07				(MDLC_RT_BASE + 0x21c)
#define MDLC_RT_MPDG08				(MDLC_RT_BASE + 0x220)
#define MDLC_RT_MPDG09				(MDLC_RT_BASE + 0x224)
#define MDLC_RT_MPDG10				(MDLC_RT_BASE + 0x228)
#define MDLC_RT_MPDG11				(MDLC_RT_BASE + 0x22c)
#define MDLC_RT_MPDGS00				(MDLC_RT_BASE + 0x300)
#define MDLC_RT_MPDGS01				(MDLC_RT_BASE + 0x304)
#define MDLC_RT_MPDGS02				(MDLC_RT_BASE + 0x308)
#define MDLC_RT_MPDGS03				(MDLC_RT_BASE + 0x30c)
#define MDLC_RT_MPDGS04				(MDLC_RT_BASE + 0x310)
#define MDLC_RT_MPDGS05				(MDLC_RT_BASE + 0x314)
#define MDLC_RT_MPDGS06				(MDLC_RT_BASE + 0x318)
#define MDLC_RT_MPDGS07				(MDLC_RT_BASE + 0x31c)
#define MDLC_RT_MPDGS08				(MDLC_RT_BASE + 0x320)
#define MDLC_RT_MPDGS09				(MDLC_RT_BASE + 0x324)
#define MDLC_RT_MPDGS10				(MDLC_RT_BASE + 0x328)
#define MDLC_RT_MPDGS11				(MDLC_RT_BASE + 0x32c)
#define MDLC_RT_MSRES02				(MDLC_RT_BASE + 0x908)
#define MDLC_RT_MSRESS02			(MDLC_RT_BASE + 0x968)
#define MDLC_RT_MSRES03				(MDLC_RT_BASE + 0x90c)
#define MDLC_RT_MSRESS03			(MDLC_RT_BASE + 0x96c)
#define MDLC_RT_MSRES15				(MDLC_RT_BASE + 0x93c)
#define MDLC_RT_MSRESS15			(MDLC_RT_BASE + 0x99c)
#define MDLC_RT_MSRES15_INTAP0_MASK		GENMASK(9, 8)
#define MDLC_RT_MSRES15_INTTP_MASK		GENMASK(11, 10)
#define MDLC_RT_MSRES15_INTAP1_MASK		GENMASK(13, 12)
#define MDLC_RT_PKCPROT0			(MDLC_RT_BASE + 0xcf0)
#define MDLC_RT_PKCPROT1			(MDLC_RT_BASE + 0xcf4)

/*
 * mdlc_wait_for_reset() - Wait for MDLC reset register and reset status register to align
 * @res: Reset register
 * @stat: Reset status register
 */
static void mdlc_wait_for_reset(const u32 res, const u32 stat)
{
	/* This can not use readl_poll_timeout(), timer is not available yet. */
	for (; readl(res) != readl(stat);)
		;
}

/**
 * mdlc_write_msres() - Write MSRES register to release IP from reset
 * @prot: Protect register
 * @res: Reset register
 * @val: Value to set in the masked out bits
 */
static void mdlc_write_msres(const u32 prot, const u32 res, const int val)
{
	writel(PKC_PROT_UNLOCK, prot);
	writel(val, res);
	writel(PKC_PROT_LOCK, prot);
}

/**
 * mdlc_rmw_msres() - Read-modify-write MSRES register to release IP from reset
 * @prot: Protect register
 * @res: Reset register
 * @mask: Mask in the register to clear
 * @val: Value to set in the masked out bits
 */
static void mdlc_rmw_msres(const u32 prot, const u32 res, const u32 mask, const int val)
{
	u32 reg;

	reg = readl(res);
	reg &= ~mask;
	reg |= field_prep(mask, val);

	mdlc_write_msres(prot, res, reg);
}

/**
 * mdlc_set_reset() - Set IP into reset
 * @prot: Protect register
 * @res: Reset register
 * @stat: Reset register
 * @mask: Mask in the register to clear
 */
static void mdlc_set_reset(const u32 prot, const u32 res, const u32 stat, const u32 mask)
{
	u32 status;

	mdlc_wait_for_reset(res, stat);

	status = field_get(mask, readl(stat));
	if (status == MDLC_MSRESS_STOP) {
		mdlc_rmw_msres(prot, res, mask, MDLC_MSRESS_STANDBY);
		mdlc_wait_for_reset(res, stat);
		status = field_get(mask, readl(stat));
	}

	if (status == MDLC_MSRESS_STANDBY || status == MDLC_MSRESS_RUN) {
		mdlc_rmw_msres(prot, res, mask, MDLC_MSRESS_RESET);
		mdlc_wait_for_reset(res, stat);
	}
}

/**
 * mdlc_release_reset() - Release IP from reset
 * @prot: Protect register
 * @res: Reset register
 * @stat: Reset register
 * @mask: Mask in the register to clear
 */
static void mdlc_release_reset(const u32 prot, const u32 res, const u32 stat, const u32 mask)
{
	u32 status;

	mdlc_wait_for_reset(res, stat);

	status = field_get(mask, readl(stat));
	if (status == MDLC_MSRESS_STANDBY) {
		mdlc_rmw_msres(prot, res, mask, MDLC_MSRESS_RESET);
		mdlc_wait_for_reset(res, stat);
		status = field_get(mask, readl(stat));
	}

	if (status == MDLC_MSRESS_RESET || status == MDLC_MSRESS_STOP) {
		mdlc_rmw_msres(prot, res, mask, MDLC_MSRESS_RUN);
		mdlc_wait_for_reset(res, stat);
	}
}

/**
 * mldc_mpg_module_run() - Release MPG module from gating
 * @prot: Protect register
 * @res: Reset register
 * @stat: Reset status register
 * @ier: Interrupt enable register
 * @imr: Interrupt mask register
 */
static void mldc_mpg_module_run(const u32 prot, const u32 res, const u32 stat,
				const u32 ier, const u32 imr)
{
	u32 val;

	mdlc_wait_for_reset(res, stat);

	val = readl(stat);
	if (val == MDLC_MPG_GATING) {
		writel(0, ier);
		writel(0, imr);
		mdlc_write_msres(prot, res, MDLC_MPG_RESET);
		mdlc_wait_for_reset(res, stat);
		val = readl(stat);
	}

	if (val == MDLC_MPG_RESET || val == MDLC_MPG_RUN) {
		writel(0, ier);
		writel(0, imr);
		mdlc_write_msres(prot, res, MDLC_MPG_RUN);
		mdlc_wait_for_reset(res, stat);
		val = readl(stat);
	}
}

/**
 * mdlc_mpg_start() - Configure MPG module state
 */
static void mdlc_mpg_start(void)
{
	mldc_mpg_module_run(MDLC_PERE_PKCPROT0, MDLC_PERE_MPDG01, MDLC_PERE_MPDGS01,
			    MDLC_PERE_MPIER, MDLC_PERE_MPIMR);
	mldc_mpg_module_run(MDLC_PERE_PKCPROT0, MDLC_PERE_MPDG00, MDLC_PERE_MPDGS00,
			    MDLC_PERE_MPIER, MDLC_PERE_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG11, MDLC_RT_MPDGS11,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG10, MDLC_RT_MPDGS10,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG09, MDLC_RT_MPDGS09,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG08, MDLC_RT_MPDGS08,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG07, MDLC_RT_MPDGS07,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG06, MDLC_RT_MPDGS06,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG05, MDLC_RT_MPDGS05,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG04, MDLC_RT_MPDGS04,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG03, MDLC_RT_MPDGS03,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG02, MDLC_RT_MPDGS02,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG01, MDLC_RT_MPDGS01,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	mldc_mpg_module_run(MDLC_RT_PKCPROT0, MDLC_RT_MPDG00, MDLC_RT_MPDGS00,
			    MDLC_RT_MPIER, MDLC_RT_MPIMR);
	/* Power on RSwitch */
	mldc_mpg_module_run(MDLC_HSCN_PKCPROT0, MDLC_HSCN_MPDG06, MDLC_HSCN_MPDGS06,
			    MDLC_HSCN_MPIER, MDLC_HSCN_MPIMR);
	mldc_mpg_module_run(MDLC_HSCN_PKCPROT0, MDLC_HSCN_MPDG05, MDLC_HSCN_MPDGS05,
			    MDLC_HSCN_MPIER, MDLC_HSCN_MPIMR);
	mldc_mpg_module_run(MDLC_HSCN_PKCPROT0, MDLC_HSCN_MPDG04, MDLC_HSCN_MPDGS04,
			    MDLC_HSCN_MPIER, MDLC_HSCN_MPIMR);
	mldc_mpg_module_run(MDLC_HSCN_PKCPROT0, MDLC_HSCN_MPDG03, MDLC_HSCN_MPDGS03,
			    MDLC_HSCN_MPIER, MDLC_HSCN_MPIMR);
	mldc_mpg_module_run(MDLC_HSCN_PKCPROT0, MDLC_HSCN_MPDG02, MDLC_HSCN_MPDGS02,
			    MDLC_HSCN_MPIER, MDLC_HSCN_MPIMR);
	mldc_mpg_module_run(MDLC_HSCN_PKCPROT0, MDLC_HSCN_MPDG01, MDLC_HSCN_MPDGS01,
			    MDLC_HSCN_MPIER, MDLC_HSCN_MPIMR);
	mldc_mpg_module_run(MDLC_HSCN_PKCPROT0, MDLC_HSCN_MPDG00, MDLC_HSCN_MPDGS00,
			    MDLC_HSCN_MPIER, MDLC_HSCN_MPIMR);
}

#define PFC_GP2_BASE			0xc1081000
#define PFC_GP2_GPSR			(PFC_GP2_BASE + 0x40)
#define PFC_GP2_IOINTSEL		(PFC_GP2_BASE + 0x110)
#define PFC_GP2_INOUTSEL		(PFC_GP2_BASE + 0x114)
#define PFC_GP2_OUTDT			(PFC_GP2_BASE + 0x118)
#define PFC_GP2_POSNEG			(PFC_GP2_BASE + 0x1a0)
#define PFC_GP2_PIN_AVS0		BIT(21)
#define PFC_GP2_PIN_AVS1		BIT(22)
#define PFC_GP2_PIN_AVS			\
	(PFC_GP2_PIN_AVS0 | PFC_GP2_PIN_AVS1)

#define PFC_GP3_BASE			0xc0800000
#define PFC_GP3_PULLEN			(PFC_GP3_BASE + 0xc0)
#define PFC_GP3_PUDSEL			(PFC_GP3_BASE + 0xc4)

#define PFC_GP5_BASE			0xc0400000
#define PFC_GROUP_ADDR_MASK		GENMASK(31, 11)
#define PFC_GP5_GPSR			(PFC_GP5_BASE + 0x40)
#define PFC_GP5_ALTSEL0			(PFC_GP5_BASE + 0x60)
#define PFC_GP5_ALTSEL1			(PFC_GP5_BASE + 0x64)
#define PFC_GP5_ALTSEL2			(PFC_GP5_BASE + 0x68)
#define PFC_GP5_ALTSEL3			(PFC_GP5_BASE + 0x6c)
#define PFC_GP5_PIN_HTX0		BIT(0)
#define PFC_GP5_PIN_HRX0		BIT(1)
#define PFC_GP5_PIN_SCIF_CLK		BIT(5)
#define PFC_GP5_PIN_HTX1		BIT(6)
#define PFC_GP5_PIN_HRX1		BIT(7)
#define PFC_GP5_PIN_HSCIF0_HSCIF1	\
	(PFC_GP5_PIN_HTX0 | PFC_GP5_PIN_HRX0 | \
	 PFC_GP5_PIN_HTX1 | PFC_GP5_PIN_HRX1)
#define PFC_GP5_ALTSEL_HSCIF0_HSCIF1	\
	(PFC_GP5_PIN_HSCIF0_HSCIF1 | PFC_GP5_PIN_SCIF_CLK)

/**
 * pfc_rmw_reg() - Read-modify-write PFC register
 * @reg: Register to write
 * @mask: Mask in the register to clear
 * @val: Value to set in the masked out bits
 */
static void pfc_rmw_reg(const u32 reg, const u32 mask, const u32 val)
{
	u32 pmmr = reg & PFC_GROUP_ADDR_MASK;
	u32 tmp;

	tmp = readl(reg);
	tmp &= ~mask;
	tmp |= val;

	writel(~tmp, pmmr);
	writel(tmp, reg);
}

/**
 * pfc_set_hscif0_hscif1_pinmux() - Set HSCIF0 and HSCIF1 pinmux
 * @bd3250k: Set to TRUE if HSCIF configured for 3.25 MBdps
 *
 * This function configures both HSCIF0 and HSCIF1 pin multiplexing,
 * HSCIF0 is used for follow up stages, HSCIF1 is used for IPL console.
 */
static void pfc_set_hscif0_hscif1_pinmux(bool bd3250k)
{
	u32 gpsr_mask = PFC_GP5_PIN_HSCIF0_HSCIF1;

	if (bd3250k)
		gpsr_mask |= PFC_GP5_PIN_SCIF_CLK;

	pfc_rmw_reg(PFC_GP5_ALTSEL0, PFC_GP5_ALTSEL_HSCIF0_HSCIF1, 0);
	pfc_rmw_reg(PFC_GP5_ALTSEL1, PFC_GP5_ALTSEL_HSCIF0_HSCIF1, 0);
	pfc_rmw_reg(PFC_GP5_ALTSEL2, PFC_GP5_ALTSEL_HSCIF0_HSCIF1, 0);
	pfc_rmw_reg(PFC_GP5_ALTSEL3, PFC_GP5_ALTSEL_HSCIF0_HSCIF1, 0);
	pfc_rmw_reg(PFC_GP5_GPSR, gpsr_mask, gpsr_mask);
}

/**
 * pfc_set_avs_pinmux() - Set AVS pinmux
 */
static void pfc_set_avs_pinmux(void)
{
	clrbits_le32(PFC_GP2_POSNEG, PFC_GP2_PIN_AVS);
	clrbits_le32(PFC_GP2_IOINTSEL, PFC_GP2_PIN_AVS);
	setbits_le32(PFC_GP2_OUTDT, PFC_GP2_PIN_AVS);
	setbits_le32(PFC_GP2_INOUTSEL, PFC_GP2_PIN_AVS);
	pfc_rmw_reg(PFC_GP2_GPSR, PFC_GP2_PIN_AVS, 0);
}

/* The addresses in range 0x08000000..0x1fffffff are incremented by 0xa0000000 */
#define MFIS_BASE		0xb89e0000
#define MFIS_WPCNTR		(MFIS_BASE + 0x900)
#define MFIS_CODEVALUE		0xacce0000

/**
 * mfis_unprotect() - Remove MFIS register write protection
 */
static void mfis_unprotect(void)
{
	writel(MFIS_CODEVALUE, MFIS_WPCNTR);
}

/**
 * rsip_write_reg() - Write RSIP control register
 * @reg: Register to write
 * @val: Value to set in the register
 */
static void rsip_write_reg(const u32 reg, const int val)
{
	for (;;) {
		writel(RSIP_CTL_PROT0PCMD_WREN, RSIP_CTL_PROT0PCMD);
		writel(val, reg);
		writel(~val, reg);
		writel(val, reg);

		if (readl(RSIP_CTL_PROT0PS) != RSIP_CTL_PROT0PS_ERR)
			break;
	}
}

/**
 * rsip_irq_setup() - Configure RSIP interrupts
 */
static void rsip_irq_setup(void)
{
	asm volatile("cpsid i");

	setbits_le32(RSIP_SHCSR, RSIP_SHCSR_USGFAULTENA | RSIP_SHCSR_BUSFAULTENA |
				 RSIP_SHCSR_MEMFAULTENA);

	writel(0xffffffff, RSIP_NVIC_ICER_00);
	writel(0xffffffff, RSIP_NVIC_ICER_01);
	writel(0xffffffff, RSIP_NVIC_ICER_02);
	writel(0xffffffff, RSIP_NVIC_ICER_03);
	writel(0xffffffff, RSIP_NVIC_ICER_04);
	writel(0xffffffff, RSIP_NVIC_ICER_05);
	writel(0xffffffff, RSIP_NVIC_ICER_06);
	writel(0xffffffff, RSIP_NVIC_ICER_07);
	writel(0xffffffff, RSIP_NVIC_ICER_08);
	writel(0xffffffff, RSIP_NVIC_ICER_09);
	writel(0xffffffff, RSIP_NVIC_ICER_10);
	writel(0xffffffff, RSIP_NVIC_ICER_11);
	writel(0xffffffff, RSIP_NVIC_ICER_12);
	writel(0xffffffff, RSIP_NVIC_ICER_13);
	writel(0xffffffff, RSIP_NVIC_ICER_14);
	writel(0xffffffff, RSIP_NVIC_ICER_15);

	/* WDT IRQ */
	writel(RSIP_NVIC_ISER_00_SETENA_INTIWDTA0, RSIP_NVIC_ISER_00);

	asm volatile("cpsid i");
}

#define RPC_BASE				0xc08c0000
#define RPC_CMNSR				(RPC_BASE + 0x48)
#define RPC_PHYCNT				(RPC_BASE + 0x7c)
#define RPC_CMNCR				(RPC_BASE + 0x00)
#define RPC_SSLDR				(RPC_BASE + 0x04)
#define RPC_DRCR				(RPC_BASE + 0x0c)
#define RPC_DRCMR				(RPC_BASE + 0x10)
#define RPC_DREAR				(RPC_BASE + 0x14)
#define RPC_DROPR				(RPC_BASE + 0x18)
#define RPC_DRENR				(RPC_BASE + 0x1c)
#define RPC_SMCR				(RPC_BASE + 0x20)
#define RPC_SMCMR				(RPC_BASE + 0x24)
#define RPC_SMADR				(RPC_BASE + 0x28)
#define RPC_SMENR				(RPC_BASE + 0x30)
#define RPC_SMWDR0				(RPC_BASE + 0x40)
#define RPC_DRDMCR				(RPC_BASE + 0x58)
#define RPC_DRDRENR				(RPC_BASE + 0x5c)
#define RPC_PHYOFFSET1				(RPC_BASE + 0x80)
#define RPC_PHYINT				(RPC_BASE + 0x88)
#define RPC_SEC_CONF				(RPC_BASE + 0xb8)

#define RPC_CMNSR_TEND				BIT(0)

#define RPC_CMNCR_MD				BIT(31)
#define RPC_CMNCR_MOIIO3(n)			FIELD_PREP(GENMASK(23, 22), (n))
#define RPC_CMNCR_MOIIO2(n)			FIELD_PREP(GENMASK(21, 20), (n))
#define RPC_CMNCR_MOIIO1(n)			FIELD_PREP(GENMASK(19, 18), (n))
#define RPC_CMNCR_MOIIO0(n)			FIELD_PREP(GENMASK(17, 16), (n))
#define RPC_CMNCR_IO0FV(n)			FIELD_PREP(GENMASK(9, 8), (n))
#define RPC_CMNCR_BSZ(n)			FIELD_PREP(GENMASK(1, 0), (n))

#define RPC_SSLDR_SPNDL_MASK			GENMASK(18, 16)
#define RPC_SSLDR_SPNDL_SPCLK_2_CYCLES		FIELD_PREP(RPC_SSLDR_SPNDL_MASK, 1)
#define RPC_SSLDR_SLNDL_MASK			GENMASK(10, 8)
#define RPC_SSLDR_SLNDL_SPCLK_5_5_CYCLES	FIELD_PREP(RPC_SSLDR_SLNDL_MASK, 4)
#define RPC_SSLDR_SCKDL_MASK			GENMASK(2, 0)
#define RPC_SSLDR_SCKDL_SPCLK_2_CYCLES		FIELD_PREP(RPC_SSLDR_SCKDL_MASK, 1)

#define RPC_DRCR_SSLN				BIT(24)
#define RPC_DRCR_RBURST_MASK			GENMASK(20, 16)
#define RPC_DRCR_RCF_READ_CACHE_CLEARE		BIT(9)
#define RPC_DRCR_RBE				BIT(8)

#define RPC_DRCMR_CMD_MASK			GENMASK(23, 16)
#define RPC_DRCMR_CMD_HYPERFLASH_READ		FIELD_PREP(RPC_DRCMR_CMD_MASK, 0xA0)

#define RPC_DREAR_EAV_MASK			GENMASK(23, 16)
#define RPC_DREAR_EAC_MASK			GENMASK(2, 0)

#define RPC_DRENR_DME				BIT(15)
#define RPC_DRENR_CDE				BIT(14)
#define RPC_DRENR_OCDE				BIT(12)
#define RPC_DRENR_ADE_MASK			GENMASK(11, 8)
#define RPC_DRENR_ADE_HYPERFLASH		FIELD_PREP(RPC_DRENR_ADE_MASK, 4)
#define RPC_DRENR_OPDE_MASK			GENMASK(7, 4)

#define RPC_DRDMCR_DMCYC_MASK			GENMASK(4, 0)
#define RPC_DRDMCR_DMCYC_15_CYCLE		FIELD_PREP(RPC_DRDMCR_DMCYC_MASK, 0xe)

#define RPC_DRDRENR_HYPE_MASK			GENMASK(14, 12)
#define RPC_DRDRENR_HYPE_HYPERFLASH		FIELD_PREP(RPC_DRDRENR_HYPE_MASK, 5)
#define RPC_DRDRENR_ADDRE			BIT(8)
#define RPC_DRDRENR_OPDRE			BIT(4)
#define RPC_DRDRENR_DRDRE			BIT(0)

#define RPC_PHYCNT_STRTIM_BIT27BIT17_15_MASK	0x08038000

#define RPC_PHYCNT_HS				BIT(18)
#define RPC_PHYCNT_PHYMEM_MASK			GENMASK(1, 0)
#define RPC_PHYCNT_PHYMEM_HYPERFLASH		FIELD_PREP(RPC_PHYCNT_PHYMEM_MASK, 3)

/**
 * rpc_safe_setup() - Configure RPC with safe static settings
 */
static void rpc_safe_setup(void)
{
	writel(RPC_CMNCR_MOIIO3(2) | RPC_CMNCR_MOIIO2(2) | RPC_CMNCR_MOIIO1(2) |
	       RPC_CMNCR_MOIIO0(2) | RPC_CMNCR_IO0FV(3) | RPC_CMNCR_BSZ(1) |
	       0x01007000, RPC_CMNCR);
	writel(RPC_SSLDR_SPNDL_SPCLK_2_CYCLES | RPC_SSLDR_SLNDL_SPCLK_5_5_CYCLES |
	       RPC_SSLDR_SCKDL_SPCLK_2_CYCLES, RPC_SSLDR);
	writel(RPC_DRCR_RBURST_MASK | RPC_DRCR_RBE, RPC_DRCR);
	writel(RPC_DRCMR_CMD_HYPERFLASH_READ, RPC_DRCMR);
	writel(0, RPC_DREAR);
	writel(RPC_DRENR_ADE_HYPERFLASH | 0xa222d000, RPC_DRENR);
	writel(0, RPC_SMCR);
	writel(0, RPC_SMCMR);
	writel(0, RPC_SMADR);
	writel(BIT(14), RPC_SMENR);
	writel(0, RPC_SMWDR0);
	writel(RPC_CMNSR_TEND, RPC_CMNSR);
	writel(RPC_DRDMCR_DMCYC_15_CYCLE, RPC_DRDMCR);
	writel(RPC_DRDRENR_HYPE_HYPERFLASH | RPC_DRDRENR_ADDRE | RPC_DRDRENR_DRDRE,
	       RPC_DRDRENR);
	writel(0x08078263, RPC_PHYCNT);
	writel(0x21511144, RPC_PHYOFFSET1);
	writel(0x07070002, RPC_PHYINT);
	writel(0x00000155, RPC_SEC_CONF);
	writel(0x11, CLK_CONTROL_RPCCKCR);
	writel(0x10100, PFC_GP3_PULLEN);
	writel(0x10100, PFC_GP3_PUDSEL);
}

/**
 * rpc_boot_setup() - Configure RPC after boot from HF
 */
static void rpc_boot_setup(void)
{
	clrsetbits_le32(RPC_PHYCNT,
			RPC_PHYCNT_STRTIM_BIT27BIT17_15_MASK | RPC_PHYCNT_HS |
			RPC_PHYCNT_PHYMEM_MASK,
			RPC_PHYCNT_STRTIM_BIT27BIT17_15_MASK |
			RPC_PHYCNT_HS | RPC_PHYCNT_PHYMEM_HYPERFLASH);

	clrsetbits_le32(RPC_CMNCR,
			RPC_CMNCR_MD |
			RPC_CMNCR_MOIIO3(3) | RPC_CMNCR_MOIIO2(3) | RPC_CMNCR_MOIIO1(3) |
			RPC_CMNCR_MOIIO0(3) | RPC_CMNCR_IO0FV(3) | RPC_CMNCR_BSZ(3),
			RPC_CMNCR_MOIIO3(2) | RPC_CMNCR_MOIIO2(2) | RPC_CMNCR_MOIIO1(2) |
			RPC_CMNCR_MOIIO0(2) | RPC_CMNCR_IO0FV(3) | RPC_CMNCR_BSZ(1));

	clrsetbits_le32(RPC_SSLDR,
			RPC_SSLDR_SPNDL_MASK | RPC_SSLDR_SLNDL_MASK | RPC_SSLDR_SCKDL_MASK,
			RPC_SSLDR_SPNDL_SPCLK_2_CYCLES | RPC_SSLDR_SLNDL_SPCLK_5_5_CYCLES |
			RPC_SSLDR_SCKDL_SPCLK_2_CYCLES);

	clrsetbits_le32(RPC_DRCR,
			RPC_DRCR_SSLN | RPC_DRCR_RBURST_MASK | RPC_DRCR_RBE,
			RPC_DRCR_SSLN | RPC_DRCR_RBURST_MASK |
			RPC_DRCR_RCF_READ_CACHE_CLEARE | RPC_DRCR_RBE);
	readl(RPC_DRCR);	/* Dummy readback */

	clrsetbits_le32(RPC_DRCMR, RPC_DRCMR_CMD_MASK, RPC_DRCMR_CMD_HYPERFLASH_READ);

	clrbits_le32(RPC_DREAR, RPC_DREAR_EAV_MASK | RPC_DREAR_EAC_MASK);

	writel(0, RPC_DROPR);

	clrsetbits_le32(RPC_DRENR,
			RPC_DRENR_DME | RPC_DRENR_CDE | RPC_DRENR_OCDE |
			RPC_DRENR_ADE_MASK | RPC_DRENR_OPDE_MASK,
			RPC_DRENR_DME | RPC_DRENR_CDE | RPC_DRENR_OCDE |
			RPC_DRENR_ADE_HYPERFLASH);

	clrsetbits_le32(RPC_DRDMCR, RPC_DRDMCR_DMCYC_MASK, RPC_DRDMCR_DMCYC_15_CYCLE);

	clrsetbits_le32(RPC_DRDRENR,
			RPC_DRDRENR_HYPE_MASK | RPC_DRDRENR_ADDRE |
			RPC_DRDRENR_OPDRE | RPC_DRDRENR_DRDRE,
			RPC_DRDRENR_HYPE_HYPERFLASH | RPC_DRDRENR_ADDRE |
			RPC_DRDRENR_DRDRE);
}

/**
 * rpc_setup() - Configure RPC
 */
static void rpc_setup(void)
{
	const u32 boot = field_get(SYSSS_MODE1_BOOTMODE_MASK, sysss_read_modemr());

	if (boot != SYSSS_MODE1_BOOTMODE_HF_DMA) {
		/* Not booted from HF, this may be SCIF loader, use safe setup. */
		rpc_safe_setup();
		return;
	}

	/* Wait for any outstanding transfer to end. */
	/* This can not use readl_poll_timeout(), timer is not available yet. */
	while (!(readl(RPC_CMNSR) & RPC_CMNSR_TEND))
		;

	/* Tristate IO */
	setbits_le32(RPC_CMNCR, RPC_CMNCR_MOIIO3(3) | RPC_CMNCR_MOIIO2(3) |
				RPC_CMNCR_MOIIO1(3) | RPC_CMNCR_MOIIO0(3));

	/* Set 160 MHz RPC HF clock */
	clrsetbits_le32(CLK_CONTROL_RPCCKCR,
			CLK_CONTROL_RPCCKCR_CKSTP1 | CLK_CONTROL_RPCCKCR_RPCFC_MASK,
			0x11);

	rpc_boot_setup();
}

#define AXMM10_ADSPLCR0_CTRL				0xe9a07100
#define AXMM10_ADSPLCR1_CTRL				0xe9a07104
#define AXMM11_ADSPLCR0_CTRL				0xe9a07150
#define AXMM11_ADSPLCR1_CTRL				0xe9a07154

/**
 * axi_qos_init() - Configure AXI bus QoS
 */
static void axi_qos_init(void)
{
	writel(0x00011d0c, AXMM10_ADSPLCR0_CTRL);
	writel(0x0000ffff, AXMM10_ADSPLCR1_CTRL);
	writel(0x00011d0c, AXMM11_ADSPLCR0_CTRL);
	writel(0x0000ffff, AXMM11_ADSPLCR1_CTRL);
	dsb();
}

/**
 * load_perm_table() - Load shared table into SRAM and hardware
 * @dstaddr: SRAM destination address
 * @dstsize: SRAM destination area size
 * @listtable: Table of registers to write at offset 2i
 * @listsize: Size of table of registers
 * @listval: Value to write at offset 2i+1
 */
static void load_perm_table(const u32 dstaddr, const u32 dstsize,
			    const u32 *listtable, const u32 listsize,
			    const u32 listval)
{
	/*
	 * The addresses in range 0x08000000..0x1fffffff are
	 * incremented by 0xa0000000 .
	 */
	u32 *dmabuf = (u32 *)(dstaddr + 0xa0000000);
	u32 reg, rv;
	int i;

	/* Place shared bus access permissions configuration table into SRAM */
	for (i = 0; i < dstsize / (2 * sizeof(u32)); i++) {
		if (i >= listsize) {
			dmabuf[2 * i] = 0;
			dmabuf[(2 * i) + 1] = 0;
			continue;
		}

		dmabuf[2 * i] = listtable[i];
		dmabuf[(2 * i) + 1] = listval;

		/*
		 * The addresses in range 0x08000000..0x1fffffff are
		 * incremented by 0xa0000000 .
		 */
		reg = listtable[i];
		if (reg > 0x08000000 && reg < 0x20000000)
			reg += 0xa0000000;
		writel(listval, reg);
	}

	/* Validate hardware write. */
	for (i = 0; i < listsize; i++) {
		reg = listtable[i];
		if (reg > 0x08000000 && reg < 0x20000000)
			reg += 0xa0000000;
		rv = readl(reg);
		if (rv == listval)
			continue;
		printf("RG[%d] = 0x%x / expected 0x%x\n", i, rv, listval);
	}
}

/**
 * load_perm_tables() - Load shared tables into SRAM and hardware
 */
static void load_perm_tables(void)
{
	load_perm_table(RGIDM_SHARED_ADDR, RGIDM_SHARED_SIZE,
			rgidm_register_list, ARRAY_SIZE(rgidm_register_list), 0);
	load_perm_table(SEC_MODID_SHARED_ADDR, SEC_MODID_SHARED_SIZE,
			sec_modid_register_list, ARRAY_SIZE(sec_modid_register_list), 0xffff);
	load_perm_table(RGIDR_SHARED_ADDR, RGIDR_SHARED_SIZE,
			rgidr_register_list, ARRAY_SIZE(rgidr_register_list), 1);
	load_perm_table(RGIDW_SHARED_ADDR, RGIDW_SHARED_SIZE,
			rgidw_register_list, ARRAY_SIZE(rgidw_register_list), 1);

	/* Override Region ID secure group settings */
	writel(0xffffffff, 0xc0983820);
	writel(0xffffffff, 0xc0983828);

	writel(0xffffffff, 0xec603828);
	writel(0xffffffff, 0xec60382c);
	writel(0xffffffff, 0xec603830);
	writel(0xffffffff, 0xec603834);
	writel(0xffffffff, 0xec603838);
	writel(0xffffffff, 0xec60383c);

	writel(0xffffffff, 0xc9d03880);
	writel(0xffffffff, 0xc9d03884);

	/*
	 * The addresses in range 0x08000000..0x1fffffff
	 * are incremented by 0xa0000000
	 */
	writel(0xffffffff, 0xba80383c);
	writel(0xffffffff, 0xba803840);
	writel(0xffffffff, 0xba803848);
	writel(0xffffffff, 0xba80384c);
	writel(0xffffffff, 0xba803974);

	writel(0xffffffff, 0xde803804);
	writel(0xffffffff, 0xde803808);
	writel(0xffffffff, 0xde803810);
	writel(0xffffffff, 0xde803814);
	writel(0xffffffff, 0xde803838);
	writel(0xffffffff, 0xde80383c);
	writel(0xffffffff, 0xde803840);
	writel(0xffffffff, 0xde803844);

	writel(0xffffffff, 0xc1283808);

	writel(0xffffffff, 0xe9a08000);
	writel(0xffffffff, 0xe9a081fc);
	writel(0xffffffff, 0xe9a08200);
	writel(0xffffffff, 0xe9a081fc);
}

#define SCP_STCM_BASE				0xc1000000
#define SCP_BASE				0xc1340000
#define SCP_CFGVECTABLE				(SCP_BASE + 0x0)
#define SCP_CFGNSSTCALIB			(SCP_BASE + 0x10)
#define SCP_CPUWAIT				(SCP_BASE + 0x30)
#define SCP_CFGNSSTCALIB_13_3MHZ		0x010040f0
#define SCP_CPUWAIT_WAIT			BIT(0)

/**
 * scp_initialize() - Initialize SCP
 *
 * Put SCP into reset, configure SCP entry point address and systick timer,
 * release SCP from reset, and zero out SCP STCM regions.
 */
static void scp_initialize(void)
{
	u32 addr;

	mdlc_set_reset(MDLC_SCP_PKCPROT1, MDLC_SCP_MSRES02, MDLC_SCP_MSRESS02,
		       MDLC_SCP_MSRES02_SCP_MASK);

	writel(SCP_STCM_BASE, SCP_CFGVECTABLE);
	writel(SCP_CFGNSSTCALIB_13_3MHZ, SCP_CFGNSSTCALIB);
	setbits_le32(SCP_CPUWAIT, SCP_CPUWAIT_WAIT);

	mdlc_release_reset(MDLC_SCP_PKCPROT1, MDLC_SCP_MSRES02, MDLC_SCP_MSRESS02,
			   MDLC_SCP_MSRES02_SCP_MASK);

	/* Fill zero to SCP STCM regions 0 ... 27 */
	for (addr = SCP_STCM_BASE; addr < 0xc1061b00; addr += 8)
		writeq(0, addr);

	asm volatile("dsb sy");
}

#define GIC720AE_GICR_PWRR(cpu)					\
	(GICR_BASE + 0x24 + ((cpu) * 0x40000))

#define GIC720AE_GICD_IVIEWR(num)				\
	(GICD_BASE + 0xf600 + ((num) * 0x4))
#define GIC720AE_GICD_IVIEWRE(num)				\
	(GICD_BASE + 0xf800 + ((num) * 0x4))
#define GIC720AE_GICR_TYPER(cpu)				\
	(GICD_BASE + 0x80000 + ((cpu) * 0x40000) + 0x8)
#define GIC720AE_GICR_VIEWR(cpu)				\
	(GICD_BASE + 0x80000 + ((cpu) * 0x40000) + 0x2c)
#define GIC720AE_GICR_MPIDR(cpu)				\
	(GICD_BASE + 0x80000 + ((cpu) * 0x40000) + 0x100)
#define GIC720AE_GICR_TYPER_AFF_MASK	GENMASK_ULL(63, 32)

#define CA_CORE_MAX			32

/**
 * gic720ae_init() - GIC720AE initialization
 */
static void gic720ae_init(void)
{
	u64 val;
	int i;

	mdlc_release_reset(MDLC_RT_PKCPROT1, MDLC_RT_MSRES15, MDLC_RT_MSRESS15,
			   MDLC_RT_MSRES15_INTAP0_MASK);
	mdlc_release_reset(MDLC_RT_PKCPROT1, MDLC_RT_MSRES15, MDLC_RT_MSRESS15,
			   MDLC_RT_MSRES15_INTTP_MASK);
	mdlc_release_reset(MDLC_RT_PKCPROT1, MDLC_RT_MSRES15, MDLC_RT_MSRESS15,
			   MDLC_RT_MSRES15_INTAP1_MASK);

	for (i = 0; i < CA_CORE_MAX; i++)
		writel(BIT(1), GIC720AE_GICR_PWRR(i));

	for (i = 0; i < CA_CORE_MAX; i++) {
		val = readq(GIC720AE_GICR_TYPER(i)) & GIC720AE_GICR_TYPER_AFF_MASK;
		writel((u32)(val >> 32U), GIC720AE_GICR_MPIDR(i));
	}

	writel(0x3f, GICD_BASE);	/* CTRL register */

	for (i = 2; i <= 61; i++)	/* From IVIEWR min..max */
		writel(0x55555555, GIC720AE_GICD_IVIEWR(i));

	for (i = 0; i <= 63; i++)	/* From IVIEWRE min..max */
		writel(0x55555555, GIC720AE_GICD_IVIEWRE(i));

	for (i = 0; i < CA_CORE_MAX; i++)
		writel(1, GIC720AE_GICR_VIEWR(i));	/* View 1 */
}

/**
 * mach_cpu_init() - Initialize hardware and start other cores
 */
int mach_cpu_init(void)
{
	mfis_unprotect();
	pfc_set_avs_pinmux();
	mdlc_mpg_start();
	clk_control_set_pll();
	rsip_irq_setup();
	rpc_setup();
	axi_qos_init();
	load_perm_tables();
	gic720ae_init();

	/* Release SCP from reset */
	scp_initialize();

	return 0;
}

int board_early_init_r(void)
{
	u32 remaptmp = readl(RSIP_CTL_ESICREMAP0);
	struct udevice *dev;
	int ret;

	/* Remap DDR PHY during DRAM init. */
	rsip_write_reg(RSIP_CTL_ESICREMAP0, 0xe0000000);

	/* Start DBSC5 */
	ret = uclass_get_device_by_name(UCLASS_NOP, "ram@e9800000", &dev);
	if (ret)
		printf("DBSC5 init failed: %d\n", ret);

	/* Restore remapping. */
	rsip_write_reg(RSIP_CTL_ESICREMAP0, remaptmp);

	return 0;
}

/**
 * board_debug_uart_init() - Initialize all HSCIF
 */
void board_debug_uart_init(void)
{
	const u32 baud = field_get(SYSSS_MODE1_BAUDRATE_MASK, sysss_read_modemr());

	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL01_0_CR2,
					    CLK_CONTROL_PLL01_0_SCR);
	clk_control_switch_from_iosc_to_pll(CLK_CONTROL_PLL01_1_CR2,
					    CLK_CONTROL_PLL01_1_SCR);

	mdlc_release_reset(MDLC_PERW_PKCPROT1, MDLC_PERW_MSRES05, MDLC_PERW_MSRESS05,
			   MDLC_PERW_MSRES05_HSCIF0_MASK);
	mdlc_release_reset(MDLC_PERW_PKCPROT1, MDLC_PERW_MSRES05, MDLC_PERW_MSRESS05,
			   MDLC_PERW_MSRES05_HSCIF1_MASK);

	pfc_set_hscif0_hscif1_pinmux(baud == SYSSS_MODE1_BAUDRATE_3250000);
}

/**
 * board_init() - Board specific initialization
 */
int board_init(void)
{
	/* Allow WDT reset */
	writel(RST_KCPROT_DIS, RST_RESKCPROT0);
	clrbits_le32(RST_WDTRSTCR, RST_WWDT_RSTMSK | RST_RWDT_RSTMSK);

	return 0;
}

/**
 * arm_reserve_mmu() - Reserve space for MMU tables
 */
int arm_reserve_mmu(void)
{
	/* Reserve space for MMU tables just above stack in STCM */
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->arch.tlb_addr = CONFIG_CUSTOM_SYS_INIT_SP_ADDR;
	debug("TLB table from %08lx to %08lx\n", gd->arch.tlb_addr,
	      gd->arch.tlb_addr + gd->arch.tlb_size);

	return 0;
}

/**
 * reset_cpu() - Reset this CPU core
 */
void __weak reset_cpu(void)
{
	writel(RST_KCPROT_DIS, RST_RESKCPROT0);
	writel(0x1, RST_SWSRES1A);
}

/**
 * lmb_arch_add_memory() - Add memory to LMB
 *
 * Add the window to a subset of 32bit DRAM are into LMB,
 * to make it possible to TFTP into it.
 */
void lmb_arch_add_memory(void)
{
	lmb_add(0x60000000, 0x40000000);
}
