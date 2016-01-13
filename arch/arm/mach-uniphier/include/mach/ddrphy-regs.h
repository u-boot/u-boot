/*
 * UniPhier DDR PHY registers
 *
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_DDRPHY_REGS_H
#define ARCH_DDRPHY_REGS_H

#include <linux/bitops.h>
#include <linux/compiler.h>

#ifndef __ASSEMBLY__

struct ddrphy {
	u32 ridr;		/* Revision Identification Register */
	u32 pir;		/* PHY Initialixation Register */
	u32 pgcr[2];		/* PHY General Configuration Register */
	u32 pgsr[2];		/* PHY General Status Register */
	u32 pllcr;		/* PLL Control Register */
	u32 ptr[5];		/* PHY Timing Register */
	u32 acmdlr;		/* AC Master Delay Line Register */
	u32 acbdlr;		/* AC Bit Delay Line Register */
	u32 aciocr;		/* AC I/O Configuration Register */
	u32 dxccr;		/* DATX8 Common Configuration Register */
	u32 dsgcr;		/* DDR System General Configuration Register */
	u32 dcr;		/* DRAM Configuration Register */
	u32 dtpr[3];		/* DRAM Timing Parameters Register */
	u32 mr0;		/* Mode Register 0 */
	u32 mr1;		/* Mode Register 1 */
	u32 mr2;		/* Mode Register 2 */
	u32 mr3;		/* Mode Register 3 */
	u32 odtcr;		/* ODT Configuration Register */
	u32 dtcr;		/* Data Training Configuration Register */
	u32 dtar[4];		/* Data Training Address Register */
	u32 dtdr[2];		/* Data Training Data Register */
	u32 dtedr[2];		/* Data Training Eye Data Register */
	u32 pgcr2;		/* PHY General Configuration Register 2 */
	u32 rsv0[8];		/* Reserved */
	u32 rdimmgcr[2];	/* RDIMM General Configuration Register */
	u32 rdimmcr0[2];	/* RDIMM Control Register */
	u32 dcuar;		/* DCU Address Register */
	u32 dcudr;		/* DCU Data Register */
	u32 dcurr;		/* DCU Run Register */
	u32 dculr;		/* DCU Loop Register */
	u32 dcugcr;		/* DCU General Configuration Register */
	u32 dcutpr;		/* DCU Timing Parameters Register */
	u32 dcusr[2];		/* DCU Status Register */
	u32 rsv1[8];		/* Reserved */
	u32 bistrr;		/* BIST Run Register */
	u32 bistwcr;		/* BIST Word Count Register */
	u32 bistmskr[3];	/* BIST Mask Register */
	u32 bistlsr;		/* BIST LFSR Sed Register */
	u32 bistar[3];		/* BIST Address Register */
	u32 bistudpr;		/* BIST User Data Pattern Register */
	u32 bistgsr;		/* BIST General Status Register */
	u32 bistwer;		/* BIST Word Error Register */
	u32 bistber[4];		/* BIST Bit Error Register */
	u32 bistwcsr;		/* BIST Word Count Status Register */
	u32 bistfwr[3];		/* BIST Fail Word Register */
	u32 rsv2[10];		/* Reserved */
	u32 gpr[2];		/* General Purpose Register */
	struct ddrphy_zq {	/* ZQ */
		u32 cr[2];	/* Impedance Control Register */
		u32 sr[2];	/* Impedance Status Register */
	} zq[4];
	struct ddrphy_datx8 {	/* DATX8 */
		u32 gcr;	/* General Configuration Register */
		u32 gsr[2];	/* General Status Register */
		u32 bdlr[5];	/* Bit Delay Line Register */
		u32 lcdlr[3];	/* Local Calibrated Delay Line Register */
		u32 mdlr;	/* Master Delay Line Register */
		u32 gtr;	/* General Timing Register */
		u32 gsr2;	/* General Status Register 2 */
		u32 rsv[2];	/* Reserved */
	} dx[9];
};

#endif /* __ASSEMBLY__ */

#define PIR_INIT		BIT(0)		/* Initialization Trigger */
#define PIR_ZCAL		BIT(1)		/* Impedance Calibration */
#define PIR_PLLINIT		BIT(4)		/* PLL Initialization */
#define PIR_DCAL		BIT(5)		/* DDL Calibration */
#define PIR_PHYRST		BIT(6)		/* PHY Reset */
#define PIR_DRAMRST		BIT(7)		/* DRAM Reset */
#define PIR_DRAMINIT		BIT(8)		/* DRAM Initialization */
#define PIR_WL			BIT(9)		/* Write Leveling */
#define PIR_QSGATE		BIT(10)		/* Read DQS Gate Training */
#define PIR_WLADJ		BIT(11)		/* Write Leveling Adjust */
#define PIR_RDDSKW		BIT(12)		/* Read Data Bit Deskew */
#define PIR_WRDSKW		BIT(13)		/* Write Data Bit Deskew */
#define PIR_RDEYE		BIT(14)		/* Read Data Eye Training */
#define PIR_WREYE		BIT(15)		/* Write Data Eye Training */
#define PIR_LOCKBYP		BIT(28)		/* PLL Lock Bypass */
#define PIR_DCALBYP		BIT(29)		/* DDL Calibration Bypass */
#define PIR_ZCALBYP		BIT(30)		/* Impedance Calib Bypass */
#define PIR_INITBYP		BIT(31)		/* Initialization Bypass */

#define PGSR0_IDONE		BIT(0)		/* Initialization Done */
#define PGSR0_PLDONE		BIT(1)		/* PLL Lock Done */
#define PGSR0_DCDONE		BIT(2)		/* DDL Calibration Done */
#define PGSR0_ZCDONE		BIT(3)		/* Impedance Calibration Done */
#define PGSR0_DIDONE		BIT(4)		/* DRAM Initialization Done */
#define PGSR0_WLDONE		BIT(5)		/* Write Leveling Done */
#define PGSR0_QSGDONE		BIT(6)		/* DQS Gate Training Done */
#define PGSR0_WLADONE		BIT(7)		/* Write Leveling Adjust Done */
#define PGSR0_RDDONE		BIT(8)		/* Read Bit Deskew Done */
#define PGSR0_WDDONE		BIT(9)		/* Write Bit Deskew Done */
#define PGSR0_REDONE		BIT(10)		/* Read Eye Training Done */
#define PGSR0_WEDONE		BIT(11)		/* Write Eye Training Done */
#define PGSR0_IERR		BIT(16)		/* Initialization Error */
#define PGSR0_PLERR		BIT(17)		/* PLL Lock Error */
#define PGSR0_DCERR		BIT(18)		/* DDL Calibration Error */
#define PGSR0_ZCERR		BIT(19)		/* Impedance Calib Error */
#define PGSR0_DIERR		BIT(20)		/* DRAM Initialization Error */
#define PGSR0_WLERR		BIT(21)		/* Write Leveling Error */
#define PGSR0_QSGERR		BIT(22)		/* DQS Gate Training Error */
#define PGSR0_WLAERR		BIT(23)		/* Write Leveling Adj Error */
#define PGSR0_RDERR		BIT(24)		/* Read Bit Deskew Error */
#define PGSR0_WDERR		BIT(25)		/* Write Bit Deskew Error */
#define PGSR0_REERR		BIT(26)		/* Read Eye Training Error */
#define PGSR0_WEERR		BIT(27)		/* Write Eye Training Error */
#define PGSR0_DTERR_SHIFT	28		/* Data Training Error Status*/
#define PGSR0_DTERR		(7 << (PGSR0_DTERR_SHIFT))
#define PGSR0_APLOCK		BIT(31)		/* AC PLL Lock */

#define DXCCR_DQSRES_OPEN	(0 << 5)
#define DXCCR_DQSRES_688_OHM	(1 << 5)
#define DXCCR_DQSRES_611_OHM	(2 << 5)
#define DXCCR_DQSRES_550_OHM	(3 << 5)
#define DXCCR_DQSRES_500_OHM	(4 << 5)
#define DXCCR_DQSRES_458_OHM	(5 << 5)
#define DXCCR_DQSRES_393_OHM	(6 << 5)
#define DXCCR_DQSRES_344_OHM	(7 << 5)

#define DXCCR_DQSNRES_OPEN	(0 << 9)
#define DXCCR_DQSNRES_688_OHM	(1 << 9)
#define DXCCR_DQSNRES_611_OHM	(2 << 9)
#define DXCCR_DQSNRES_550_OHM	(3 << 9)
#define DXCCR_DQSNRES_500_OHM	(4 << 9)
#define DXCCR_DQSNRES_458_OHM	(5 << 9)
#define DXCCR_DQSNRES_393_OHM	(6 << 9)
#define DXCCR_DQSNRES_344_OHM	(7 << 9)

#define DTCR_DTRANK_SHIFT	4		/* Data Training Rank */
#define DTCR_DTRANK_MASK	(0x3 << (DTCR_DTRANK_SHIFT))
#define DTCR_DTMPR		BIT(6)		/* Data Training using MPR */
#define DTCR_RANKEN_SHIFT	24		/* Rank Enable */
#define DTCR_RANKEN_MASK	(0xf << (DTCR_RANKEN_SHIFT))

#define DXGCR_WLRKEN_SHIFT	26		/* Write Level Rank Enable */
#define DXGCR_WLRKEN_MASK	(0xf << (DXGCR_WLRKEN_SHIFT))

/* SoC-specific parameters */
#define NR_DATX8_PER_DDRPHY	2

#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD4) || \
	defined(CONFIG_ARCH_UNIPHIER_PH1_SLD8)
#define NR_DDRPHY_PER_CH		1
#else
#define NR_DDRPHY_PER_CH		2
#endif

#define NR_DDRCH		2

#define DDRPHY_BASE(ch, phy)	(0x5bc01000 + 0x200000 * (ch) + 0x1000 * (phy))

#ifndef __ASSEMBLY__
int ph1_ld4_ddrphy_init(struct ddrphy __iomem *phy, int freq, int size);
int ph1_pro4_ddrphy_init(struct ddrphy __iomem *phy, int freq, int size);
int ph1_sld8_ddrphy_init(struct ddrphy __iomem *phy, int freq, int size);
void ddrphy_prepare_training(struct ddrphy __iomem *phy, int rank);
int ddrphy_training(struct ddrphy __iomem *phy);
#endif

#endif /* ARCH_DDRPHY_REGS_H */
