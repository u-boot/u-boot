/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef _PPC405EZ_H_
#define _PPC405EZ_H_

#define CONFIG_NAND_NDFC

/* Memory mapped register */
#define CONFIG_SYS_PERIPHERAL_BASE	0xef600000 /* Internal Peripherals */

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0400)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0700)
#define GPIO1_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0800)

/* DCR register */
#define OCM0_PLBCR1	0x0020	/* OCM PLB3 Bank 1 Config */
#define OCM0_PLBCR2	0x0021	/* OCM PLB3 Bank 2 Config */
#define OCM0_PLBBEAR	0x0022	/* OCM PLB3 Bus Error Add */
#define OCM0_DSRC1	0x0028	/* OCM D-side Bank 1 Config */
#define OCM0_DSRC2	0x0029	/* OCM D-side Bank 2 Config */
#define OCM0_ISRC1	0x002A	/* OCM I-side Bank 1Config */
#define OCM0_ISRC2	0x002B	/* OCM I-side Bank 2 Config */
#define OCM0_DISDPC	0x002C	/* OCM D-/I-side Data Par Chk */

/* SDR register */
#define SDR0_NAND0	0x4000
#define SDR0_ULTRA0	0x4040
#define SDR0_ULTRA1	0x4050
#define SDR0_ICINTSTAT	0x4510

/* CPR register */
#define CPR0_PRIMAD	0x0080
#define CPR0_PERD0	0x00e0
#define CPR0_PERD1	0x00e1
#define CPR0_PERC0	0x0180

#define	MAL_DCR_BASE	0x380

#define SDR_NAND0_NDEN		0x80000000
#define SDR_NAND0_NDBTEN	0x40000000
#define SDR_NAND0_NDBADR_MASK	0x30000000
#define SDR_NAND0_NDBPG_MASK	0x0f000000
#define SDR_NAND0_NDAREN	0x00800000
#define SDR_NAND0_NDRBEN	0x00400000

#define SDR_ULTRA0_NDGPIOBP	0x80000000
#define SDR_ULTRA0_CSN_MASK	0x78000000
#define SDR_ULTRA0_CSNSEL0	0x40000000
#define SDR_ULTRA0_CSNSEL1	0x20000000
#define SDR_ULTRA0_CSNSEL2	0x10000000
#define SDR_ULTRA0_CSNSEL3	0x08000000
#define SDR_ULTRA0_EBCRDYEN	0x04000000
#define SDR_ULTRA0_SPISSINEN	0x02000000
#define SDR_ULTRA0_NFSRSTEN	0x01000000

#define SDR_ULTRA1_LEDNENABLE	0x40000000

#define SDR_ICRX_STAT		0x80000000
#define SDR_ICTX0_STAT		0x40000000
#define SDR_ICTX1_STAT		0x20000000

#define CPR_CLKUPD_ENPLLCH_EN	0x40000000 /* Enable CPR PLL Changes */
#define CPR_CLKUPD_ENDVCH_EN	0x20000000 /* Enable CPR Sys. Div. Changes */
#define CPR_PERD0_SPIDV_MASK	0x000F0000 /* SPI Clock Divider */

#define PLLC_SRC_MASK		0x20000000 /* PLL feedback source */

#define PLLD_FBDV_MASK		0x1F000000 /* PLL feedback divider value */
#define PLLD_FWDVA_MASK		0x000F0000 /* PLL forward divider A value */
#define PLLD_FWDVB_MASK		0x00000700 /* PLL forward divider B value */

#define PRIMAD_CPUDV_MASK	0x0F000000 /* CPU Clock Divisor Mask */
#define PRIMAD_PLBDV_MASK	0x000F0000 /* PLB Clock Divisor Mask */
#define PRIMAD_OPBDV_MASK	0x00000F00 /* OPB Clock Divisor Mask */
#define PRIMAD_EBCDV_MASK	0x0000000F /* EBC Clock Divisor Mask */

#define PERD0_PWMDV_MASK	0xFF000000 /* PWM Divider Mask */
#define PERD0_SPIDV_MASK	0x000F0000 /* SPI Divider Mask */
#define PERD0_U0DV_MASK		0x0000FF00 /* UART 0 Divider Mask */
#define PERD0_U1DV_MASK		0x000000FF /* UART 1 Divider Mask */

#endif /* _PPC405EZ_H_ */
