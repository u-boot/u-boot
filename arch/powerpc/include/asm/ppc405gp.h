/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PPC405GP_H_
#define _PPC405GP_H_

#define CONFIG_SDRAM_PPC4xx_IBM_SDRAM	/* IBM SDRAM controller */

/* Memory mapped register */
#define CONFIG_SYS_PERIPHERAL_BASE	0xef600000 /* Internal Peripherals */

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0400)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0700)

/* DCR's */
#define DCP0_CFGADDR	0x0014		/* Decompression controller addr reg */
#define DCP0_CFGDATA	0x0015		/* Decompression controller data reg */
#define OCM0_ISCNTL	0x0019	/* OCM I-side control reg */
#define OCM0_DSARC	0x001a	/* OCM D-side address compare */
#define OCM0_DSCNTL	0x001b	/* OCM D-side control */
#define CPC0_PLLMR	0x00b0		/* PLL mode  register */
#define CPC0_CR0	0x00b1		/* chip control register 0 */
#define CPC0_CR1	0x00b2		/* chip control register 1 */
#define CPC0_PSR	0x00b4		/* chip pin strapping reg */
#define CPC0_EIRR	0x00b6		/* ext interrupt routing reg */
#define CPC0_SR		0x00b8		/* Power management status */
#define CPC0_ER		0x00b9		/* Power management enable */
#define CPC0_FR		0x00ba		/* Power management force */
#define CPC0_ECR	0x00aa		/* edge conditioner register */

/* values for kiar register - indirect addressing of these regs */
#define KCONF		0x40		/* decompression core config register */

#define PLLMR_FWD_DIV_MASK	0xE0000000	/* Forward Divisor */
#define PLLMR_FWD_DIV_BYPASS	0xE0000000
#define PLLMR_FWD_DIV_3		0xA0000000
#define PLLMR_FWD_DIV_4		0x80000000
#define PLLMR_FWD_DIV_6		0x40000000

#define PLLMR_FB_DIV_MASK	0x1E000000	/* Feedback Divisor */
#define PLLMR_FB_DIV_1		0x02000000
#define PLLMR_FB_DIV_2		0x04000000
#define PLLMR_FB_DIV_3		0x06000000
#define PLLMR_FB_DIV_4		0x08000000

#define PLLMR_TUNING_MASK	0x01F80000

#define PLLMR_CPU_TO_PLB_MASK	0x00060000	/* CPU:PLB Frequency Divisor */
#define PLLMR_CPU_PLB_DIV_1	0x00000000
#define PLLMR_CPU_PLB_DIV_2	0x00020000
#define PLLMR_CPU_PLB_DIV_3	0x00040000
#define PLLMR_CPU_PLB_DIV_4	0x00060000

#define PLLMR_OPB_TO_PLB_MASK	0x00018000	/* OPB:PLB Frequency Divisor */
#define PLLMR_OPB_PLB_DIV_1	0x00000000
#define PLLMR_OPB_PLB_DIV_2	0x00008000
#define PLLMR_OPB_PLB_DIV_3	0x00010000
#define PLLMR_OPB_PLB_DIV_4	0x00018000

#define PLLMR_PCI_TO_PLB_MASK	0x00006000	/* PCI:PLB Frequency Divisor */
#define PLLMR_PCI_PLB_DIV_1	0x00000000
#define PLLMR_PCI_PLB_DIV_2	0x00002000
#define PLLMR_PCI_PLB_DIV_3	0x00004000
#define PLLMR_PCI_PLB_DIV_4	0x00006000

#define PLLMR_EXB_TO_PLB_MASK	0x00001800	/* External Bus:PLB Divisor */
#define PLLMR_EXB_PLB_DIV_2	0x00000000
#define PLLMR_EXB_PLB_DIV_3	0x00000800
#define PLLMR_EXB_PLB_DIV_4	0x00001000
#define PLLMR_EXB_PLB_DIV_5	0x00001800

/* definitions for PPC405GPr (new mode strapping) */
#define PLLMR_FWDB_DIV_MASK	0x00000007	/* Forward Divisor B */

#define PSR_PLL_FWD_MASK	0xC0000000
#define PSR_PLL_FDBACK_MASK	0x30000000
#define PSR_PLL_TUNING_MASK	0x0E000000
#define PSR_PLB_CPU_MASK	0x01800000
#define PSR_OPB_PLB_MASK	0x00600000
#define PSR_PCI_PLB_MASK	0x00180000
#define PSR_EB_PLB_MASK		0x00060000
#define PSR_ROM_WIDTH_MASK	0x00018000
#define PSR_ROM_LOC		0x00004000
#define PSR_PCI_ASYNC_EN	0x00001000
#define PSR_PERCLK_SYNC_MODE_EN 0x00000800	/* PPC405GPr only */
#define PSR_PCI_ARBIT_EN	0x00000400
#define PSR_NEW_MODE_EN		0x00000020	/* PPC405GPr only */

#endif /* _PPC405GP_H_ */
