/*
 * Copyright (c) 2010, Applied Micro Circuits Corporation
 * Author: Tirumala R Marri <tmarri@apm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _APM821XX_H_
#define _APM821XX_H_

#define CONFIG_SDRAM_PPC4xx_IBM_DDR2	/* IBM DDR(2) controller */

/* Memory mapped registers */
#define CONFIG_SYS_PERIPHERAL_BASE	0xEF600000
#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0200)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0700)

#define SDR0_SRST0_DMC		0x00200000
#define SDR0_SRST1_AHB		0x00000040      /* PLB4XAHB bridge */

/* AHB config. */
#define AHB_TOP			0xA4
#define AHB_BOT			0xA5

/* clk divisors */
#define PLLSYS0_FWD_DIV_A_MASK	0x000000f0	/* Fwd Div A */
#define PLLSYS0_FWD_DIV_B_MASK	0x0000000f	/* Fwd Div B */
#define PLLSYS0_FB_DIV_MASK	0x0000ff00	/* Feedback divisor */
#define PLLSYS0_OPB_DIV_MASK	0x0c000000	/* OPB Divisor */
#define PLLSYS0_EPB_DIV_MASK	0x00000300      /* EPB divisor */
#define PLLSYS0_EXTSL_MASK	0x00000080      /* PerClk feedback path */
#define PLLSYS0_PLBEDV0_DIV_MASK	0xe0000000/* PLB Early Clk Div*/
#define PLLSYS0_PERCLK_DIV_MASK	0x03000000	/* Peripheral Clk Divisor */
#define PLLSYS0_SEL_MASK	0x18000000	/* 0 = PLL, 1 = PerClk */

/*
   + * Clocking Controller
   + */
#define CPR0_CLKUPD	0x0020
#define CPR0_PLLC	0x0040
#define CPR0_PLLC_SEL(pllc)		(((pllc) & 0x01000000) >> 24)
#define CPR0_PLLD	0x0060
#define CPR0_PLLD_FDV(plld)		(((plld) & 0xff000000) >> 24)
#define CPR0_PLLD_FWDVA(plld)		(((plld) & 0x000f0000) >> 16)
#define CPR0_CPUD	0x0080
#define CPR0_CPUD_CPUDV(cpud)		(((cpud) & 0x07000000) >> 24)
#define CPR0_PLB2D	0x00a0
#define CPR0_PLB2D_PLB2DV(plb2d)	(((plb2d) & 0x06000000) >> 25)
#define CPR0_OPBD	0x00c0
#define CPR0_OPBD_OPBDV(opbd)		(((opbd) & 0x03000000) >> 24)
#define CPR0_PERD	0x00e0
#define CPR0_PERD_PERDV(perd)		(((perd) & 0x03000000) >> 24)
#define CPR0_DDR2D	0x0100
#define CPR0_DDR2D_DDR2DV(ddr2d) 	(((ddr2d) & 0x06000000) >> 25)
#define CLK_ICFG	0x0140

#endif /* _APM821XX_H_ */
