/*
 * UniPhier SC (System Control) block registers for ARMv8 SoCs
 *
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef SC64_REGS_H
#define SC64_REGS_H

#define SC_BASE_ADDR		0x61840000

/* PLL type: SSC */
#define SC_CPLLCTRL	(SC_BASE_ADDR | 0x1400)	/* LD11/20: CPU/ARM */
#define SC_SPLLCTRL	(SC_BASE_ADDR | 0x1410)	/* LD11/20: misc */
#define SC_SPLL2CTRL	(SC_BASE_ADDR | 0x1420)	/* LD20: IPP */
#define SC_MPLLCTRL	(SC_BASE_ADDR | 0x1430)	/* LD11/20: Video codec */
#define SC_VSPLLCTRL	(SC_BASE_ADDR | 0x1440)	/* LD11 */
#define SC_VPPLLCTRL	(SC_BASE_ADDR | 0x1440)	/* LD20: VPE etc. */
#define SC_GPPLLCTRL	(SC_BASE_ADDR | 0x1450)	/* LD20: GPU/Mali */
#define SC_DPLLCTRL	(SC_BASE_ADDR | 0x1460)	/* LD11: DDR memory */
#define SC_DPLL0CTRL	(SC_BASE_ADDR | 0x1460)	/* LD20: DDR memory 0 */
#define SC_DPLL1CTRL	(SC_BASE_ADDR | 0x1470)	/* LD20: DDR memory 1 */
#define SC_DPLL2CTRL	(SC_BASE_ADDR | 0x1480)	/* LD20: DDR memory 2 */

/* PLL type: VPLL27 */
#define SC_VPLL27FCTRL	(SC_BASE_ADDR | 0x1500)
#define SC_VPLL27ACTRL	(SC_BASE_ADDR | 0x1520)

/* PLL type: DSPLL */
#define SC_VPLL8KCTRL	(SC_BASE_ADDR | 0x1540)
#define SC_A2PLLCTRL	(SC_BASE_ADDR | 0x15C0)

#define SC_RSTCTRL		(SC_BASE_ADDR | 0x2000)
#define SC_RSTCTRL3		(SC_BASE_ADDR | 0x2008)
#define SC_RSTCTRL4		(SC_BASE_ADDR | 0x200c)
#define   SC_RSTCTRL4_ETHER		(1 << 6)
#define   SC_RSTCTRL4_NAND		(1 << 0)
#define SC_RSTCTRL5		(SC_BASE_ADDR | 0x2010)
#define SC_RSTCTRL6		(SC_BASE_ADDR | 0x2014)
#define SC_RSTCTRL7		(SC_BASE_ADDR | 0x2018)
#define   SC_RSTCTRL7_UMCSB		(1 << 16)
#define   SC_RSTCTRL7_UMCA2		(1 << 10)
#define   SC_RSTCTRL7_UMCA1		(1 << 9)
#define   SC_RSTCTRL7_UMCA0		(1 << 8)
#define   SC_RSTCTRL7_UMC32		(1 << 2)
#define   SC_RSTCTRL7_UMC31		(1 << 1)
#define   SC_RSTCTRL7_UMC30		(1 << 0)

#define SC_CLKCTRL		(SC_BASE_ADDR | 0x2100)
#define SC_CLKCTRL3		(SC_BASE_ADDR | 0x2108)
#define SC_CLKCTRL4		(SC_BASE_ADDR | 0x210c)
#define   SC_CLKCTRL4_MIO		(1 << 10)
#define   SC_CLKCTRL4_STDMAC		(1 << 8)
#define   SC_CLKCTRL4_PERI		(1 << 7)
#define   SC_CLKCTRL4_ETHER		(1 << 6)
#define   SC_CLKCTRL4_NAND		(1 << 0)
#define SC_CLKCTRL5		(SC_BASE_ADDR | 0x2110)
#define SC_CLKCTRL6		(SC_BASE_ADDR | 0x2114)
#define SC_CLKCTRL7		(SC_BASE_ADDR | 0x2118)
#define   SC_CLKCTRL7_UMCSB		(1 << 16)
#define   SC_CLKCTRL7_UMC32		(1 << 2)
#define   SC_CLKCTRL7_UMC31		(1 << 1)
#define   SC_CLKCTRL7_UMC30		(1 << 0)

#define SC_CA72_GEARST		(SC_BASE_ADDR | 0x8000)
#define SC_CA72_GEARSET		(SC_BASE_ADDR | 0x8004)
#define SC_CA72_GEARUPD		(SC_BASE_ADDR | 0x8008)
#define SC_CA53_GEARST		(SC_BASE_ADDR | 0x8080)
#define SC_CA53_GEARSET		(SC_BASE_ADDR | 0x8084)
#define SC_CA53_GEARUPD		(SC_BASE_ADDR | 0x8088)
#define   SC_CA_GEARUPD			(1 << 0)

#endif /* SC64_REGS_H */
