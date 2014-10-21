/*
 * UniPhier SC (System Control) block registers
 *
 * Copyright (C) 2011-2014 Panasonic Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_SC_REGS_H
#define ARCH_SC_REGS_H

#define SC_BASE_ADDR			0x61840000

#define SC_MPLLOSCCTL                   (SC_BASE_ADDR | 0x1184)
#define SC_MPLLOSCCTL_MPLLEN		(0x1 << 0)
#define SC_MPLLOSCCTL_MPLLST		(0x1 << 1)

#define SC_DPLLCTRL			(SC_BASE_ADDR | 0x1200)
#define SC_DPLLCTRL_SSC_EN		(0x1 << 31)
#define SC_DPLLCTRL_FOUTMODE_MASK        (0xf << 16)
#define SC_DPLLCTRL_SSC_RATE		(0x1 << 15)

#define SC_DPLLCTRL2			(SC_BASE_ADDR | 0x1204)
#define SC_DPLLCTRL2_NRSTDS		(0x1 << 28)

#define SC_DPLLCTRL3			(SC_BASE_ADDR | 0x1208)
#define SC_DPLLCTRL3_LPFSEL_COEF2	(0x0 << 31)
#define SC_DPLLCTRL3_LPFSEL_COEF3	(0x1 << 31)

#define SC_UPLLCTRL			(SC_BASE_ADDR | 0x1210)

#define SC_VPLL27ACTRL			(SC_BASE_ADDR | 0x1270)
#define SC_VPLL27ACTRL2			(SC_BASE_ADDR | 0x1274)
#define SC_VPLL27ACTRL3			(SC_BASE_ADDR | 0x1278)

#define SC_VPLL27BCTRL			(SC_BASE_ADDR | 0x1290)
#define SC_VPLL27BCTRL2			(SC_BASE_ADDR | 0x1294)
#define SC_VPLL27BCTRL3			(SC_BASE_ADDR | 0x1298)

#define SC_RSTCTRL			(SC_BASE_ADDR | 0x2000)
#define SC_RSTCTRL_NRST_ETHER		(0x1 << 12)
#define SC_RSTCTRL_NRST_UMC1		(0x1 <<  5)
#define SC_RSTCTRL_NRST_UMC0		(0x1 <<  4)
#define SC_RSTCTRL_NRST_NAND		(0x1 <<  2)

#define SC_RSTCTRL2			(SC_BASE_ADDR | 0x2004)
#define SC_RSTCTRL3			(SC_BASE_ADDR | 0x2008)

#define SC_CLKCTRL			(SC_BASE_ADDR | 0x2104)
#define SC_CLKCTRL_CLK_ETHER		(0x1 << 12)
#define SC_CLKCTRL_CLK_MIO		(0x1 << 11)
#define SC_CLKCTRL_CLK_UMC		(0x1 <<  4)
#define SC_CLKCTRL_CLK_NAND		(0x1 <<  2)
#define SC_CLKCTRL_CLK_SBC		(0x1 <<  1)
#define SC_CLKCTRL_CLK_PERI		(0x1 <<  0)

/* System reset control register */
#define SC_IRQTIMSET			(SC_BASE_ADDR | 0x3000)
#define SC_SLFRSTSEL			(SC_BASE_ADDR | 0x3010)
#define SC_SLFRSTCTL			(SC_BASE_ADDR | 0x3014)

#endif /* ARCH_SC_REGS_H */
