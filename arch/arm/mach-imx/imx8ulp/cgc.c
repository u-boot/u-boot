// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/cgc.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

static struct cgc1_regs *cgc1_regs = (struct cgc1_regs *)0x292C0000UL;
static struct cgc2_regs *cgc2_regs = (struct cgc2_regs *)0x2da60000UL;

void cgc1_soscdiv_init(void)
{
	/* Configure SOSC/FRO DIV1 ~ DIV3 */
	clrbits_le32(&cgc1_regs->soscdiv, BIT(7));
	clrbits_le32(&cgc1_regs->soscdiv, BIT(15));
	clrbits_le32(&cgc1_regs->soscdiv, BIT(23));
	clrbits_le32(&cgc1_regs->soscdiv, BIT(31));

	clrbits_le32(&cgc1_regs->frodiv, BIT(7));
}

void cgc1_pll2_init(void)
{
	u32 reg;

	if (readl(&cgc1_regs->pll2csr) & BIT(23))
		clrbits_le32(&cgc1_regs->pll2csr, BIT(23));

	/* Disable PLL2 */
	clrbits_le32(&cgc1_regs->pll2csr, BIT(0));
	mdelay(1);

	/* wait valid bit false */
	while ((readl(&cgc1_regs->pll2csr) & BIT(24)))
		;

	/* Select SOSC as source, freq = 31 * 24 =744mhz */
	reg = 31 << 16;
	writel(reg, &cgc1_regs->pll2cfg);

	/* Enable PLL2 */
	setbits_le32(&cgc1_regs->pll2csr, BIT(0));

	/* Wait for PLL2 clock ready */
	while (!(readl(&cgc1_regs->pll2csr) & BIT(24)))
		;
}

static void cgc1_set_a35_clk(u32 clk_src, u32 div_core)
{
	u32 reg;

	/* ulock */
	if (readl(&cgc1_regs->ca35clk) & BIT(31))
		clrbits_le32(&cgc1_regs->ca35clk, BIT(31));

	reg = readl(&cgc1_regs->ca35clk);
	reg &= ~GENMASK(29, 21);
	reg |= ((clk_src & 0x3) << 28);
	reg |= (((div_core - 1) & 0x3f) << 21);
	writel(reg, &cgc1_regs->ca35clk);

	while (!(readl(&cgc1_regs->ca35clk) & BIT(27)))
		;
}

void cgc1_init_core_clk(void)
{
	u32 reg = readl(&cgc1_regs->ca35clk);

	/* if already selected to PLL2, switch to FRO firstly */
	if (((reg >> 28) & 0x3) == 0x1)
		cgc1_set_a35_clk(0, 1);

	/* Set pll2 to 750Mhz for 1V  */
	cgc1_pll2_init();

	/* Set A35 clock to pll2 */
	cgc1_set_a35_clk(1, 1);
}

void cgc1_enet_stamp_sel(u32 clk_src)
{
	writel((clk_src & 0x7) << 24, &cgc1_regs->enetstamp);
}

void cgc1_pll3_init(void)
{
	/* Gate off VCO */
	setbits_le32(&cgc1_regs->pll3div_vco, BIT(7));

	/* Disable PLL3 */
	clrbits_le32(&cgc1_regs->pll3csr, BIT(0));

	/* Gate off PFDxDIV */
	setbits_le32(&cgc1_regs->pll3div_pfd0, BIT(7) | BIT(15) | BIT(23) | BIT(31));
	setbits_le32(&cgc1_regs->pll3div_pfd1, BIT(7) | BIT(15) | BIT(23) | BIT(31));

	/* Gate off PFDx */
	setbits_le32(&cgc1_regs->pll3pfdcfg, BIT(7));
	setbits_le32(&cgc1_regs->pll3pfdcfg, BIT(15));
	setbits_le32(&cgc1_regs->pll3pfdcfg, BIT(23));
	setbits_le32(&cgc1_regs->pll3pfdcfg, BIT(31));

	/* Select SOSC as source */
	clrbits_le32(&cgc1_regs->pll3cfg, BIT(0));

	//setbits_le32(&cgc1_regs->pll3cfg, 22 << 16);
	writel(22 << 16, &cgc1_regs->pll3cfg);

	writel(578, &cgc1_regs->pll3num);
	writel(1000, &cgc1_regs->pll3denom);

	/* Enable PLL3 */
	setbits_le32(&cgc1_regs->pll3csr, BIT(0));

	/* Wait for PLL3 clock ready */
	while (!(readl(&cgc1_regs->pll3csr) & BIT(24)))
		;
	/* Gate on VCO */
	clrbits_le32(&cgc1_regs->pll3div_vco, BIT(7));

	/*
	 * PFD0: 380MHz/396/396/328
	 */
	clrbits_le32(&cgc1_regs->pll3pfdcfg, 0x3F);
	setbits_le32(&cgc1_regs->pll3pfdcfg, 25 << 0);
	clrbits_le32(&cgc1_regs->pll3pfdcfg, BIT(7));
	while (!(readl(&cgc1_regs->pll3pfdcfg) & BIT(6)))
		;

	clrbits_le32(&cgc1_regs->pll3pfdcfg, 0x3F << 8);
	setbits_le32(&cgc1_regs->pll3pfdcfg, 24 << 8);
	clrbits_le32(&cgc1_regs->pll3pfdcfg, BIT(15));
	while (!(readl(&cgc1_regs->pll3pfdcfg) & BIT(14)))
		;

	clrbits_le32(&cgc1_regs->pll3pfdcfg, 0x3F << 16);
	setbits_le32(&cgc1_regs->pll3pfdcfg, 24 << 16);
	clrbits_le32(&cgc1_regs->pll3pfdcfg, BIT(23));
	while (!(readl(&cgc1_regs->pll3pfdcfg) & BIT(22)))
		;

	clrbits_le32(&cgc1_regs->pll3pfdcfg, 0x3F << 24);
	setbits_le32(&cgc1_regs->pll3pfdcfg, 29 << 24);
	clrbits_le32(&cgc1_regs->pll3pfdcfg, BIT(31));
	while (!(readl(&cgc1_regs->pll3pfdcfg) & BIT(30)))
		;

	clrbits_le32(&cgc1_regs->pll3div_pfd0, BIT(7));
	clrbits_le32(&cgc1_regs->pll3div_pfd0, BIT(15));
	clrbits_le32(&cgc1_regs->pll3div_pfd0, BIT(23));
	clrbits_le32(&cgc1_regs->pll3div_pfd0, BIT(31));

	clrbits_le32(&cgc1_regs->pll3div_pfd1, BIT(7));
	clrbits_le32(&cgc1_regs->pll3div_pfd1, BIT(15));
	clrbits_le32(&cgc1_regs->pll3div_pfd1, BIT(23));
	clrbits_le32(&cgc1_regs->pll3div_pfd1, BIT(31));
}

void cgc2_pll4_init(void)
{
	/* Disable PFD DIV and clear DIV */
	writel(0x80808080, &cgc2_regs->pll4div_pfd0);
	writel(0x80808080, &cgc2_regs->pll4div_pfd1);

	/* Gate off and clear PFD  */
	writel(0x80808080, &cgc2_regs->pll4pfdcfg);

	/* Disable PLL4 */
	writel(0x0, &cgc2_regs->pll4csr);

	/* Configure PLL4 to 528Mhz and clock source from SOSC */
	writel(22 << 16, &cgc2_regs->pll4cfg);
	writel(0x1, &cgc2_regs->pll4csr);

	/* wait for PLL4 output valid */
	while (!(readl(&cgc2_regs->pll4csr) & BIT(24)))
		;

	/* Enable all 4 PFDs */
	setbits_le32(&cgc2_regs->pll4pfdcfg, 30 << 0); /* 316.8Mhz for NIC_LPAV */
	setbits_le32(&cgc2_regs->pll4pfdcfg, 18 << 8);
	setbits_le32(&cgc2_regs->pll4pfdcfg, 12 << 16);
	setbits_le32(&cgc2_regs->pll4pfdcfg, 24 << 24);

	clrbits_le32(&cgc2_regs->pll4pfdcfg, BIT(7) | BIT(15) | BIT(23) | BIT(31));

	while ((readl(&cgc2_regs->pll4pfdcfg) & (BIT(30) | BIT(22) | BIT(14) | BIT(6)))
		!= (BIT(30) | BIT(22) | BIT(14) | BIT(6)))
		;

	/* Enable PFD DIV */
	clrbits_le32(&cgc2_regs->pll4div_pfd0, BIT(7) | BIT(15) | BIT(23) | BIT(31));
	clrbits_le32(&cgc2_regs->pll4div_pfd1, BIT(7) | BIT(15) | BIT(23) | BIT(31));
}

void cgc2_ddrclk_config(u32 src, u32 div)
{
	writel((src << 28) | (div << 21), &cgc2_regs->ddrclk);
	/* wait for DDRCLK switching done */
	while (!(readl(&cgc2_regs->ddrclk) & BIT(27)))
		;
}

u32 decode_pll(enum cgc1_clk pll)
{
	u32 reg, infreq, mult;
	u32 num, denom;

	infreq = 24000000U;
	/*
	 * Alought there are four choices for the bypass src,
	 * we choose SOSC 24M which is the default set in ROM.
	 * TODO: check more the comments
	 */
	switch (pll) {
	case PLL2:
		reg = readl(&cgc1_regs->pll2csr);
		if (!(reg & BIT(24)))
			return 0;

		reg = readl(&cgc1_regs->pll2cfg);
		mult = (reg >> 16) & 0x7F;
		denom = readl(&cgc1_regs->pll2denom) & 0x3FFFFFFF;
		num = readl(&cgc1_regs->pll2num) & 0x3FFFFFFF;

		return (u64)infreq * mult + (u64)infreq * num / denom;
	case PLL3:
		reg = readl(&cgc1_regs->pll3csr);
		if (!(reg & BIT(24)))
			return 0;

		reg = readl(&cgc1_regs->pll3cfg);
		mult = (reg >> 16) & 0x7F;
		denom = readl(&cgc1_regs->pll3denom) & 0x3FFFFFFF;
		num = readl(&cgc1_regs->pll3num) & 0x3FFFFFFF;

		return (u64)infreq * mult + (u64)infreq * num / denom;
	default:
		printf("Unsupported pll clocks %d\n", pll);
		break;
	}

	return 0;
}

u32 cgc1_pll3_vcodiv_rate(void)
{
	u32 reg, gate, div;

	reg = readl(&cgc1_regs->pll3div_vco);
	gate = BIT(7) & reg;
	div = reg & 0x3F;

	return gate ? 0 : decode_pll(PLL3) / (div + 1);
}

u32 cgc1_pll3_pfd_rate(enum cgc1_clk clk)
{
	u32 index, gate, vld, reg;

	switch (clk) {
	case PLL3_PFD0:
		index = 0;
		break;
	case PLL3_PFD1:
		index = 1;
		break;
	case PLL3_PFD2:
		index = 2;
		break;
	case PLL3_PFD3:
		index = 3;
		break;
	default:
		return 0;
	}

	reg = readl(&cgc1_regs->pll3pfdcfg);
	gate = reg & (BIT(7) << (index * 8));
	vld = reg & (BIT(6) << (index * 8));

	if (gate || !vld)
		return 0;

	return (u64)decode_pll(PLL3) * 18 / ((reg >> (index * 8)) & 0x3F);
}

u32 cgc1_pll3_pfd_div(enum cgc1_clk clk)
{
	void __iomem *base;
	u32 pfd, index, gate, reg;

	switch (clk) {
	case PLL3_PFD0_DIV1:
		base = &cgc1_regs->pll3div_pfd0;
		pfd = PLL3_PFD0;
		index = 0;
		break;
	case PLL3_PFD0_DIV2:
		base = &cgc1_regs->pll3div_pfd0;
		pfd = PLL3_PFD0;
		index = 1;
		break;
	case PLL3_PFD1_DIV1:
		base = &cgc1_regs->pll3div_pfd0;
		pfd = PLL3_PFD1;
		index = 2;
		break;
	case PLL3_PFD1_DIV2:
		base = &cgc1_regs->pll3div_pfd0;
		pfd = PLL3_PFD1;
		index = 3;
		break;
	case PLL3_PFD2_DIV1:
		base = &cgc1_regs->pll3div_pfd1;
		pfd = PLL3_PFD2;
		index = 0;
		break;
	case PLL3_PFD2_DIV2:
		base = &cgc1_regs->pll3div_pfd1;
		pfd = PLL3_PFD2;
		index = 1;
		break;
	case PLL3_PFD3_DIV1:
		base = &cgc1_regs->pll3div_pfd1;
		pfd = PLL3_PFD3;
		index = 2;
		break;
	case PLL3_PFD3_DIV2:
		base = &cgc1_regs->pll3div_pfd1;
		pfd = PLL3_PFD3;
		index = 3;
		break;
	default:
		return 0;
	}

	reg = readl(base);
	gate = reg & (BIT(7) << (index * 8));

	if (gate)
		return 0;

	return cgc1_pll3_pfd_rate(pfd) / (((reg >> (index * 8)) & 0x3F) + 1);
}

u32 cgc1_sosc_div(enum cgc1_clk clk)
{
	u32 reg, gate, index;

	switch (clk) {
	case SOSC:
		return 24000000;
	case SOSC_DIV1:
		index = 0;
		break;
	case SOSC_DIV2:
		index = 1;
		break;
	case SOSC_DIV3:
		index = 2;
		break;
	default:
		return 0;
	}

	reg = readl(&cgc1_regs->soscdiv);
	gate = reg & (BIT(7) << (index * 8));

	if (gate)
		return 0;

	return 24000000 / (((reg >> (index * 8)) & 0x3F) + 1);
}

u32 cgc1_fro_div(enum cgc1_clk clk)
{
	u32 reg, gate, vld, index;

	switch (clk) {
	case FRO:
		return 192000000;
	case FRO_DIV1:
		index = 0;
		break;
	case FRO_DIV2:
		index = 1;
		break;
	case FRO_DIV3:
		index = 2;
		break;
	default:
		return 0;
	}

	reg = readl(&cgc1_regs->frodiv);
	gate = reg & (BIT(7) << (index * 8));
	vld = reg & (BIT(6) << (index * 8));

	if (gate || !vld)
		return 0;

	return 24000000 / (((reg >> (index * 8)) & 0x3F) + 1);
}

u32 cgc1_clk_get_rate(enum cgc1_clk clk)
{
	switch (clk) {
	case SOSC:
	case SOSC_DIV1:
	case SOSC_DIV2:
	case SOSC_DIV3:
		return cgc1_sosc_div(clk);
	case FRO:
	case FRO_DIV1:
	case FRO_DIV2:
	case FRO_DIV3:
		return cgc1_fro_div(clk);
	case PLL2:
		return decode_pll(PLL2);
	case PLL3:
		return decode_pll(PLL3);
	case PLL3_VCODIV:
		return cgc1_pll3_vcodiv_rate();
	case PLL3_PFD0:
	case PLL3_PFD1:
	case PLL3_PFD2:
	case PLL3_PFD3:
		return cgc1_pll3_pfd_rate(clk);
	case PLL3_PFD0_DIV1:
	case PLL3_PFD0_DIV2:
	case PLL3_PFD1_DIV1:
	case PLL3_PFD1_DIV2:
	case PLL3_PFD2_DIV1:
	case PLL3_PFD2_DIV2:
	case PLL3_PFD3_DIV1:
	case PLL3_PFD3_DIV2:
		return cgc1_pll3_pfd_div(clk);
	default:
		printf("Unsupported cgc1 clock: %d\n", clk);
		return 0;
	}
}
