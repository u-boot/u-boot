/*
 * UniPhier DDR MultiPHY registers
 *
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_DDRMPHY_REGS_H
#define ARCH_DDRMPHY_REGS_H

#include <linux/bitops.h>

#define DMPHY_SHIFT			2

#define DMPHY_RIDR		(0x000 << DMPHY_SHIFT)
#define DMPHY_PIR		(0x001 << DMPHY_SHIFT)
#define   DMPHY_PIR_INIT		BIT(0)	/* Initialization Trigger */
#define   DMPHY_PIR_ZCAL		BIT(1)	/* Impedance Calibration */
#define   DMPHY_PIR_PLLINIT		BIT(4)	/* PLL Initialization */
#define   DMPHY_PIR_DCAL		BIT(5)	/* DDL Calibration */
#define   DMPHY_PIR_PHYRST		BIT(6)	/* PHY Reset */
#define   DMPHY_PIR_DRAMRST		BIT(7)	/* DRAM Reset */
#define   DMPHY_PIR_DRAMINIT		BIT(8)	/* DRAM Initialization */
#define   DMPHY_PIR_WL			BIT(9)	/* Write Leveling */
#define   DMPHY_PIR_QSGATE		BIT(10)	/* Read DQS Gate Training */
#define   DMPHY_PIR_WLADJ		BIT(11)	/* Write Leveling Adjust */
#define   DMPHY_PIR_RDDSKW		BIT(12)	/* Read Data Bit Deskew */
#define   DMPHY_PIR_WRDSKW		BIT(13)	/* Write Data Bit Deskew */
#define   DMPHY_PIR_RDEYE		BIT(14)	/* Read Data Eye Training */
#define   DMPHY_PIR_WREYE		BIT(15)	/* Write Data Eye Training */
#define   DMPHY_PIR_ZCALBYP		BIT(30)	/* Impedance Calib Bypass */
#define   DMPHY_PIR_INITBYP		BIT(31)	/* Initialization Bypass */
#define DMPHY_PGCR0		(0x002 << DMPHY_SHIFT)
#define   DMPHY_PGCR0_PHYFRST		BIT(26)	/* PHY FIFO Reset */
#define DMPHY_PGCR1		(0x003 << DMPHY_SHIFT)
#define   DMPHY_PGCR1_INHVT		BIT(26)	/* VT Calculation Inhibit */
#define DMPHY_PGCR2		(0x004 << DMPHY_SHIFT)
#define   DMPHY_PGCR2_DUALCHN		BIT(28)	/* Dual Channel Configuration*/
#define   DMPHY_PGCR2_ACPDDC		BIT(29)	/* AC Power-Down with Dual Ch*/
#define DMPHY_PGCR3		(0x005 << DMPHY_SHIFT)
#define DMPHY_PGSR0		(0x006 << DMPHY_SHIFT)
#define   DMPHY_PGSR0_IDONE		BIT(0)	/* Initialization Done */
#define   DMPHY_PGSR0_PLDONE		BIT(1)	/* PLL Lock Done */
#define   DMPHY_PGSR0_DCDONE		BIT(2)	/* DDL Calibration Done */
#define   DMPHY_PGSR0_ZCDONE		BIT(3)	/* Impedance Calibration Done */
#define   DMPHY_PGSR0_DIDONE		BIT(4)	/* DRAM Initialization Done */
#define   DMPHY_PGSR0_WLDONE		BIT(5)	/* Write Leveling Done */
#define   DMPHY_PGSR0_QSGDONE		BIT(6)	/* DQS Gate Training Done */
#define   DMPHY_PGSR0_WLADONE		BIT(7)	/* Write Leveling Adjust Done */
#define   DMPHY_PGSR0_RDDONE		BIT(8)	/* Read Bit Deskew Done */
#define   DMPHY_PGSR0_WDDONE		BIT(9)	/* Write Bit Deskew Done */
#define   DMPHY_PGSR0_REDONE		BIT(10)	/* Read Eye Training Done */
#define   DMPHY_PGSR0_WEDONE		BIT(11)	/* Write Eye Training Done */
#define   DMPHY_PGSR0_ZCERR		BIT(20)	/* Impedance Calib Error */
#define   DMPHY_PGSR0_WLERR		BIT(21)	/* Write Leveling Error */
#define   DMPHY_PGSR0_QSGERR		BIT(22)	/* DQS Gate Training Error */
#define   DMPHY_PGSR0_WLAERR		BIT(23)	/* Write Leveling Adj Error */
#define   DMPHY_PGSR0_RDERR		BIT(24)	/* Read Bit Deskew Error */
#define   DMPHY_PGSR0_WDERR		BIT(25)	/* Write Bit Deskew Error */
#define   DMPHY_PGSR0_REERR		BIT(26)	/* Read Eye Training Error */
#define   DMPHY_PGSR0_WEERR		BIT(27)	/* Write Eye Training Error */
#define DMPHY_PGSR1		(0x007 << DMPHY_SHIFT)
#define   DMPHY_PGSR1_VTSTOP		BIT(30)	/* VT Stop */
#define DMPHY_PLLCR		(0x008 << DMPHY_SHIFT)
#define DMPHY_PTR0		(0x009 << DMPHY_SHIFT)
#define DMPHY_PTR1		(0x00A << DMPHY_SHIFT)
#define DMPHY_PTR2		(0x00B << DMPHY_SHIFT)
#define DMPHY_PTR3		(0x00C << DMPHY_SHIFT)
#define DMPHY_PTR4		(0x00D << DMPHY_SHIFT)
#define DMPHY_ACMDLR		(0x00E << DMPHY_SHIFT)
#define DMPHY_ACLCDLR		(0x00F << DMPHY_SHIFT)
#define DMPHY_ACBDLR0		(0x010 << DMPHY_SHIFT)
#define DMPHY_ACBDLR1		(0x011 << DMPHY_SHIFT)
#define DMPHY_ACBDLR2		(0x012 << DMPHY_SHIFT)
#define DMPHY_ACBDLR3		(0x013 << DMPHY_SHIFT)
#define DMPHY_ACBDLR4		(0x014 << DMPHY_SHIFT)
#define DMPHY_ACBDLR5		(0x015 << DMPHY_SHIFT)
#define DMPHY_ACBDLR6		(0x016 << DMPHY_SHIFT)
#define DMPHY_ACBDLR7		(0x017 << DMPHY_SHIFT)
#define DMPHY_ACBDLR8		(0x018 << DMPHY_SHIFT)
#define DMPHY_ACBDLR9		(0x019 << DMPHY_SHIFT)
#define DMPHY_ACIOCR0		(0x01A << DMPHY_SHIFT)
#define DMPHY_ACIOCR1		(0x01B << DMPHY_SHIFT)
#define DMPHY_ACIOCR2		(0x01C << DMPHY_SHIFT)
#define DMPHY_ACIOCR3		(0x01D << DMPHY_SHIFT)
#define DMPHY_ACIOCR4		(0x01E << DMPHY_SHIFT)
#define DMPHY_ACIOCR5		(0x01F << DMPHY_SHIFT)
#define DMPHY_DXCCR		(0x020 << DMPHY_SHIFT)
#define DMPHY_DSGCR		(0x021 << DMPHY_SHIFT)
#define DMPHY_DCR		(0x022 << DMPHY_SHIFT)
#define DMPHY_DTPR0		(0x023 << DMPHY_SHIFT)
#define DMPHY_DTPR1		(0x024 << DMPHY_SHIFT)
#define DMPHY_DTPR2		(0x025 << DMPHY_SHIFT)
#define DMPHY_DTPR3		(0x026 << DMPHY_SHIFT)
#define DMPHY_MR0		(0x027 << DMPHY_SHIFT)
#define DMPHY_MR1		(0x028 << DMPHY_SHIFT)
#define DMPHY_MR2		(0x029 << DMPHY_SHIFT)
#define DMPHY_MR3		(0x02A << DMPHY_SHIFT)
#define DMPHY_ODTCR		(0x02B << DMPHY_SHIFT)
#define DMPHY_DTCR		(0x02C << DMPHY_SHIFT)
#define   DMPHY_DTCR_RANKEN_SHIFT	24	/* Rank Enable */
#define   DMPHY_DTCR_RANKEN_MASK	(0xf << (DMPHY_DTCR_RANKEN_SHIFT))
#define DMPHY_DTAR0		(0x02D << DMPHY_SHIFT)
#define DMPHY_DTAR1		(0x02E << DMPHY_SHIFT)
#define DMPHY_DTAR2		(0x02F << DMPHY_SHIFT)
#define DMPHY_DTAR3		(0x030 << DMPHY_SHIFT)
#define DMPHY_DTDR0		(0x031 << DMPHY_SHIFT)
#define DMPHY_DTDR1		(0x032 << DMPHY_SHIFT)
#define DMPHY_DTEDR0		(0x033 << DMPHY_SHIFT)
#define DMPHY_DTEDR1		(0x034 << DMPHY_SHIFT)
#define DMPHY_ZQCR		(0x090 << DMPHY_SHIFT)
#define   DMPHY_ZQCR_AVGEN			BIT(16)	/* Average Algorithm */
#define   DMPHY_ZQCR_FORCE_ZCAL_VT_UPDATE	BIT(27)	/* force VT update */
/* ZQ */
#define DMPHY_ZQ_BASE		(0x091 << DMPHY_SHIFT)
#define DMPHY_ZQ_STRIDE		(0x004 << DMPHY_SHIFT)
#define DMPHY_ZQ_PR		(0x000 << DMPHY_SHIFT)
#define DMPHY_ZQ_DR		(0x001 << DMPHY_SHIFT)
#define DMPHY_ZQ_SR		(0x002 << DMPHY_SHIFT)
/* DATX8 */
#define DMPHY_DX_BASE		(0x0A0 << DMPHY_SHIFT)
#define DMPHY_DX_STRIDE		(0x020 << DMPHY_SHIFT)
#define DMPHY_DX_GCR0		(0x000 << DMPHY_SHIFT)
#define   DMPHY_DX_GCR0_WLRKEN_SHIFT	26	/* Write Level Rank Enable */
#define   DMPHY_DX_GCR0_WLRKEN_MASK	(0xf << (DMPHY_DX_GCR0_WLRKEN_SHIFT))
#define DMPHY_DX_GCR1		(0x001 << DMPHY_SHIFT)
#define DMPHY_DX_GCR2		(0x002 << DMPHY_SHIFT)
#define DMPHY_DX_GCR3		(0x003 << DMPHY_SHIFT)
#define DMPHY_DX_GSR0		(0x004 << DMPHY_SHIFT)
#define DMPHY_DX_GSR1		(0x005 << DMPHY_SHIFT)
#define DMPHY_DX_GSR2		(0x006 << DMPHY_SHIFT)
#define DMPHY_DX_BDLR0		(0x007 << DMPHY_SHIFT)
#define DMPHY_DX_BDLR1		(0x008 << DMPHY_SHIFT)
#define DMPHY_DX_BDLR2		(0x009 << DMPHY_SHIFT)
#define DMPHY_DX_BDLR3		(0x00A << DMPHY_SHIFT)
#define DMPHY_DX_BDLR4		(0x00B << DMPHY_SHIFT)
#define DMPHY_DX_BDLR5		(0x00C << DMPHY_SHIFT)
#define DMPHY_DX_BDLR6		(0x00D << DMPHY_SHIFT)
#define DMPHY_DX_LCDLR0		(0x00E << DMPHY_SHIFT)
#define DMPHY_DX_LCDLR1		(0x00F << DMPHY_SHIFT)
#define DMPHY_DX_LCDLR2		(0x010 << DMPHY_SHIFT)
#define DMPHY_DX_MDLR		(0x011 << DMPHY_SHIFT)
#define DMPHY_DX_GTR		(0x012 << DMPHY_SHIFT)

#endif /* ARCH_DDRMPHY_REGS_H */
