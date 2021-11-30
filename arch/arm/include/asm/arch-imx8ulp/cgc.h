/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef _ASM_ARCH_CGC_H
#define _ASM_ARCH_CGC_H

enum cgc1_clk {
	DUMMY0_CLK,
	DUMMY1_CLK,
	LPOSC,
	XBAR_BUSCLK,
	SOSC,
	SOSC_DIV1,
	SOSC_DIV2,
	SOSC_DIV3,
	FRO,
	FRO_DIV1,
	FRO_DIV2,
	FRO_DIV3,
	PLL2,
	PLL3,
	PLL3_VCODIV,
	PLL3_PFD0,
	PLL3_PFD1,
	PLL3_PFD2,
	PLL3_PFD3,
	PLL3_PFD0_DIV1,
	PLL3_PFD0_DIV2,
	PLL3_PFD1_DIV1,
	PLL3_PFD1_DIV2,
	PLL3_PFD2_DIV1,
	PLL3_PFD2_DIV2,
	PLL3_PFD3_DIV1,
	PLL3_PFD3_DIV2,
};

struct cgc1_regs {
	u32 verid;
	u32 rsvd1[4];
	u32 ca35clk;
	u32 rsvd2[2];
	u32 clkoutcfg;
	u32 rsvd3[4];
	u32 nicclk;
	u32 xbarclk;
	u32 rsvd4[21];
	u32 clkdivrst;
	u32 rsvd5[29];
	u32 soscdiv;
	u32 rsvd6[63];
	u32 frodiv;
	u32 rsvd7[189];
	u32 pll2csr;
	u32 rsvd8[3];
	u32 pll2cfg;
	u32 rsvd9;
	u32 pll2denom;
	u32 pll2num;
	u32 pll2ss;
	u32 rsvd10[55];
	u32 pll3csr;
	u32 pll3div_vco;
	u32 pll3div_pfd0;
	u32 pll3div_pfd1;
	u32 pll3cfg;
	u32 pll3pfdcfg;
	u32 pll3denom;
	u32 pll3num;
	u32 pll3ss;
	u32 pll3lock;
	u32 rsvd11[54];
	u32 enetstamp;
	u32 rsvd12[67];
	u32 pllusbcfg;
	u32 rsvd13[59];
	u32 aud_clk1;
	u32 sai5_4_clk;
	u32 tpm6_7clk;
	u32 mqs1clk;
	u32 rsvd14[60];
	u32 lvdscfg;
};

struct cgc2_regs {
	u32 verid;
	u32 rsvd1[4];
	u32 hificlk;
	u32 rsvd2[2];
	u32 clkoutcfg;
	u32 rsvd3[6];
	u32 niclpavclk;
	u32 ddrclk;
	u32 rsvd4[19];
	u32 clkdivrst;
	u32 rsvd5[29];
	u32 soscdiv;
	u32 rsvd6[63];
	u32 frodiv;
	u32 rsvd7[253];
	u32 pll4csr;
	u32 pll4div_vco;
	u32 pll4div_pfd0;
	u32 pll4div_pfd1;
	u32 pll4cfg;
	u32 pll4pfdcfg;
	u32 pll4denom;
	u32 pll4num;
	u32 pll4ss;
	u32 pll4lock;
	u32 rsvd8[128];
	u32 aud_clk2;
	u32 sai7_6_clk;
	u32 tpm8clk;
	u32 rsvd9[1];
	u32 spdifclk;
	u32 rsvd10[59];
	u32 lvdscfg;
};

u32 cgc1_clk_get_rate(enum cgc1_clk clk);
void cgc1_pll3_init(void);
void cgc1_pll2_init(void);
void cgc1_soscdiv_init(void);
void cgc1_init_core_clk(void);
void cgc2_pll4_init(void);
void cgc2_ddrclk_config(u32 src, u32 div);
u32 cgc1_sosc_div(enum cgc1_clk clk);
#endif
