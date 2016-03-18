/*
 * UniPhier SC (System Control) block registers for ARMv8 SoCs
 *
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef SC64_REGS_H
#define SC64_REGS_H

#define SC_BASE_ADDR		0x61840000

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

#endif /* SC64_REGS_H */
