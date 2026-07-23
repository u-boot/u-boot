// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <asm/io.h>
#include <asm/arch/scu_ast2700.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/lists.h>
#include <syscon.h>
#include <linux/bitfield.h>

#include <dt-bindings/clock/aspeed,ast2700-scu.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * RGMII clock source tree
 * HPLL -->|\
 *         | |---->| divider |---->RGMII 125M for MAC#0 & MAC#1
 * APLL -->|/
 */
#define RGMII_DEFAULT_CLK_SRC	SCU1_CLK_HPLL

struct mac_delay_config {
	u32 tx_delay_1000;
	u32 rx_delay_1000;
	u32 tx_delay_100;
	u32 rx_delay_100;
	u32 tx_delay_10;
	u32 rx_delay_10;
};

typedef int (*ast2700_clk_init_fn)(struct udevice *dev);

struct ast2700_clk_priv {
	void __iomem *reg;
	ast2700_clk_init_fn init;
};

static u32 ast2700_soc1_get_pll_rate(struct ast2700_scu1 *scu, int pll_idx)
{
	union ast2700_pll_reg pll_reg;
	u32 mul = 1, div = 1;

	switch (pll_idx) {
	case SCU1_CLK_HPLL:
		pll_reg.w = readl(&scu->hpll);
		break;
	case SCU1_CLK_APLL:
		pll_reg.w = readl(&scu->apll);
		break;
	case SCU1_CLK_DPLL:
		pll_reg.w = readl(&scu->dpll);
		break;
	}

	if (!pll_reg.b.bypass) {
		mul = (pll_reg.b.m + 1) / (pll_reg.b.n + 1);
		div = (pll_reg.b.p + 1);
	}

	return ((CLKIN_25M * mul) / div);
}

#define SCU_CLKSEL2_HCLK_DIV_MASK		GENMASK(22, 20)
#define SCU_CLKSEL2_HCLK_DIV_SHIFT		20

static u32 ast2700_soc1_get_hclk_rate(struct ast2700_scu1 *scu)
{
	u32 rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_HPLL);
	u32 clk_sel2 = readl(&scu->clk_sel2);
	u32 hclk_div = (clk_sel2 & SCU_CLKSEL2_HCLK_DIV_MASK) >>
			     SCU_CLKSEL2_HCLK_DIV_SHIFT;

	if (!hclk_div)
		hclk_div = 2;
	else
		hclk_div++;

	return (rate / hclk_div);
}

#define SCU1_CLKSEL1_PCLK_DIV_MASK		GENMASK(20, 18)
#define SCU1_CLKSEL1_PCLK_DIV_SHIFT		18

static u32 ast2700_soc1_get_pclk_rate(struct ast2700_scu1 *scu)
{
	u32 rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_HPLL);

	u32 clk_sel1 = readl(&scu->clk_sel1);
	u32 pclk_div = (clk_sel1 & SCU1_CLKSEL1_PCLK_DIV_MASK) >>
			     SCU1_CLKSEL1_PCLK_DIV_SHIFT;

	return (rate / ((pclk_div + 1) * 2));
}

#define SCU_UART_CLKGEN_N_MASK			GENMASK(17, 8)
#define SCU_UART_CLKGEN_N_SHIFT			8
#define SCU_UART_CLKGEN_R_MASK			GENMASK(7, 0)
#define SCU_UART_CLKGEN_R_SHIFT			0

static u32 ast2700_soc1_get_uart_uxclk_rate(struct ast2700_scu1 *scu)
{
	u32 uxclk_sel = readl(&scu->clk_sel2) & GENMASK(1, 0);
	u32 uxclk_ctrl = readl(&scu->uxclk_ctrl);
	u32 rate;

	switch (uxclk_sel) {
	case 0:
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_APLL) / 4;
		break;
	case 1:
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_APLL) / 2;
		break;
	case 2:
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_APLL);
		break;
	case 3:
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_HPLL);
		break;
	}

	u32 n = (uxclk_ctrl & SCU_UART_CLKGEN_N_MASK) >>
		      SCU_UART_CLKGEN_N_SHIFT;
	u32 r = (uxclk_ctrl & SCU_UART_CLKGEN_R_MASK) >>
		      SCU_UART_CLKGEN_R_SHIFT;

	return ((rate * r) / (n * 2));
}

#define SCU_HUART_CLKGEN_N_MASK			GENMASK(17, 8)
#define SCU_HUART_CLKGEN_N_SHIFT		8
#define SCU_HUART_CLKGEN_R_MASK			GENMASK(7, 0)
#define SCU_HUART_CLKGEN_R_SHIFT		0

static u32 ast2700_soc1_get_uart_huxclk_rate(struct ast2700_scu1 *scu)
{
	u32 huxclk_sel = (readl(&scu->clk_sel2) & GENMASK(4, 3)) >> 3;
	u32 huxclk_ctrl = readl(&scu->huxclk_ctrl);
	u32 n = (huxclk_ctrl & SCU_HUART_CLKGEN_N_MASK) >>
		      SCU_HUART_CLKGEN_N_SHIFT;
	u32 r = (huxclk_ctrl & SCU_HUART_CLKGEN_R_MASK) >>
		      SCU_HUART_CLKGEN_R_SHIFT;
	u32 rate;

	switch (huxclk_sel) {
	case 0:
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_APLL) / 4;
		break;
	case 1:
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_APLL) / 2;
		break;
	case 2:
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_APLL);
		break;
	case 3:
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_HPLL);
		break;
	}

	return ((rate * r) / (n * 2));
}

#define SCU_CLKSRC1_SDIO_DIV_MASK		GENMASK(16, 14)
#define SCU_CLKSRC1_SDIO_DIV_SHIFT		14
#define SCU_CLKSRC1_SDIO_SEL			BIT(13)
const int ast2700_sd_div_tbl[] = {
	2, 2, 3, 4, 5, 6, 7, 8
};

static u32 ast2700_soc1_get_sdio_clk_rate(struct ast2700_scu1 *scu)
{
	u32 rate = 0;
	u32 clk_sel1 = readl(&scu->clk_sel1);
	u32 div = (clk_sel1 & SCU_CLKSRC1_SDIO_DIV_MASK) >>
			     SCU_CLKSRC1_SDIO_DIV_SHIFT;

	if (clk_sel1 & SCU_CLKSRC1_SDIO_SEL)
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_APLL);
	else
		rate = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_HPLL);

	if (!div)
		div = 1;

	div++;

	return (rate / div);
}

static void ast2700_init_sdclk(struct ast2700_scu1 *scu)
{
	u32 src_clk = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_HPLL);
	u32 reg_280;
	int i;

	for (i = 0; i < 8; i++) {
		if (src_clk / ast2700_sd_div_tbl[i] <= 125000000)
			break;
	}

	reg_280 = readl(&scu->clk_sel1);
	reg_280 &= ~(SCU_CLKSRC1_SDIO_DIV_MASK | SCU_CLKSRC1_SDIO_SEL);
	reg_280 |= i << SCU_CLKSRC1_SDIO_DIV_SHIFT;
	writel(reg_280, &scu->clk_sel1);
}

static u32
ast2700_soc1_get_uart_clk_rate(struct ast2700_scu1 *scu, int uart_idx)
{
	u32 rate = 0;

	if (readl(&scu->clk_sel1) & BIT(uart_idx))
		rate = ast2700_soc1_get_uart_huxclk_rate(scu);
	else
		rate = ast2700_soc1_get_uart_uxclk_rate(scu);

	return rate;
}

static ulong ast2700_soc1_clk_get_rate(struct clk *clk)
{
	struct ast2700_clk_priv *priv = dev_get_priv(clk->dev);
	struct ast2700_scu1 *scu = (struct ast2700_scu1 *)priv->reg;
	ulong rate = 0;

	switch (clk->id) {
	case SCU1_CLK_HPLL:
	case SCU1_CLK_APLL:
	case SCU1_CLK_DPLL:
		rate = ast2700_soc1_get_pll_rate(scu, clk->id);
		break;
	case SCU1_CLK_AHB:
		rate = ast2700_soc1_get_hclk_rate(scu);
		break;
	case SCU1_CLK_APB:
		rate = ast2700_soc1_get_pclk_rate(scu);
		break;
	case SCU1_CLK_GATE_UART0CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 0);
		break;
	case SCU1_CLK_GATE_UART1CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 1);
		break;
	case SCU1_CLK_GATE_UART2CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 2);
		break;
	case SCU1_CLK_GATE_UART3CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 3);
		break;
	case SCU1_CLK_GATE_UART5CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 5);
		break;
	case SCU1_CLK_GATE_UART6CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 6);
		break;
	case SCU1_CLK_GATE_UART7CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 7);
		break;
	case SCU1_CLK_GATE_UART8CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 8);
		break;
	case SCU1_CLK_GATE_UART9CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 9);
		break;
	case SCU1_CLK_GATE_UART10CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 10);
		break;
	case SCU1_CLK_GATE_UART11CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 11);
		break;
	case SCU1_CLK_GATE_UART12CLK:
		rate = ast2700_soc1_get_uart_clk_rate(scu, 12);
		break;
	case SCU1_CLK_GATE_SDCLK:
		rate = ast2700_soc1_get_sdio_clk_rate(scu);
		break;
	case SCU1_CLK_UXCLK:
		rate = ast2700_soc1_get_uart_uxclk_rate(scu);
		break;
	case SCU1_CLK_HUXCLK:
		rate = ast2700_soc1_get_uart_huxclk_rate(scu);
		break;
	default:
		debug("%s: unknown clk %ld\n", __func__, clk->id);
		return -ENOENT;
	}

	return rate;
}

static int ast2700_soc1_clk_enable(struct clk *clk)
{
	struct ast2700_clk_priv *priv = dev_get_priv(clk->dev);
	struct ast2700_scu1 *scu = (struct ast2700_scu1 *)priv->reg;
	u32 clkgate_bit;

	if (clk->id >= 32)
		clkgate_bit = BIT(clk->id - 32);
	else
		clkgate_bit = BIT(clk->id);

	writel(clkgate_bit, &scu->clkgate_clr1);

	return 0;
}

static const struct clk_ops ast2700_soc1_clk_ops = {
	.get_rate = ast2700_soc1_clk_get_rate,
	.enable = ast2700_soc1_clk_enable,
};

#define SCU_HW_REVISION_ID		GENMASK(23, 16)
#define SCU_CPUCLK_MASK		GENMASK(4, 2)
#define SCU_CPUCLK_SHIFT	2
static u32 ast2700_soc0_get_hpll_rate(struct ast2700_scu0 *scu)
{
	u32 chip_id1 = readl(&scu->chip_id1);
	u32 hwstrap1 = readl(&scu->hwstrap1);
	union ast2700_pll_reg pll_reg;
	u32 mul = 1, div = 1;
	u32 rate;

	pll_reg.w = readl(&scu->hpll);

	if ((chip_id1 & SCU_HW_REVISION_ID) && (hwstrap1 & BIT(3))) {
		switch ((hwstrap1 & GENMASK(4, 2)) >> 2) {
		case 2:
			rate = 1800000000;
			break;
		case 3:
			rate = 1700000000;
			break;
		case 6:
			rate = 1200000000;
			break;
		case 7:
			rate = 800000000;
			break;
		default:
			rate = 1600000000;
		}
	} else if (hwstrap1 & GENMASK(3, 2)) {
		switch ((hwstrap1 & GENMASK(3, 2)) >> 2) {
		case 1U:
			rate = 1900000000;
			break;
		case 2U:
			rate = 1800000000;
			break;
		case 3U:
			rate = 1700000000;
			break;
		default:
			rate = 1600000000;
			break;
		}
	} else {
		if (pll_reg.b.bypass == 0U) {
			/* F = 25Mhz * [(M + 2) / 2 * (n + 1)] / (p + 1) */
			mul = (pll_reg.b.m + 1) / ((pll_reg.b.n + 1) * 2);
			div = (pll_reg.b.p + 1);
		}
		rate = ((CLKIN_25M * mul) / div);
	}

	return rate;
}

static u32 ast2700_soc0_get_pll_rate(struct ast2700_scu0 *scu, int pll_idx)
{
	union ast2700_pll_reg pll_reg;
	u32 mul = 1, div = 1;
	u32 rate;

	switch (pll_idx) {
	case SCU0_CLK_DPLL:
		pll_reg.w = readl(&scu->dpll);
		break;
	case SCU0_CLK_MPLL:
		pll_reg.w = readl(&scu->mpll);
		break;
	default:
		pr_err("%s: invalid PSP clock source (%d)\n", __func__, pll_idx);
		return 0;
	}

	if (pll_reg.b.bypass == 0U) {
		if (pll_idx == SCU0_CLK_MPLL) {
			/* F = 25Mhz * [M / (n + 1)] / (p + 1) */
			mul = (pll_reg.b.m) / ((pll_reg.b.n + 1));
			div = (pll_reg.b.p + 1);
		} else {
			/* F = 25Mhz * [(M + 2) / 2 * (n + 1)] / (p + 1) */
			mul = (pll_reg.b.m + 1) / ((pll_reg.b.n + 1) * 2);
			div = (pll_reg.b.p + 1);
		}
	}

	rate = ((CLKIN_25M * mul) / div);

	return rate;
}

/*
 * AST2700A1
 * SCU010[4:2]:
 * 000: CPUCLK=MPLL=1.6GHz (MPLL default setting with SCU310, SCU314)
 * 001: CPUCLK=HPLL=2.0GHz (HPLL default setting with SCU300, SCU304)
 * 010: CPUCLK=HPLL=1.8GHz (HPLL frequency is constance and is not controlled by SCU300, SCU304)
 * 011: CPUCLK=HPLL=1.7GHz (HPLL frequency is constance and is not controlled by SCU300, SCU304)
 * 100: CPUCLK=MPLL/2=800MHz (MPLL default setting with SCU310, SCU314)
 * 101: CPUCLK=HPLL/2=1.0GHz (HPLL default setting with SCU300, SCU304)
 * 110: CPUCLK=HPLL=1.2GHz (HPLL frequency is constance and is not controlled by SCU300, SCU304)
 * 111: CPUCLK=HPLL=800MHz (HPLL frequency is constance and is not controlled by SCU300, SCU304)
 */

static u32 ast2700_soc0_get_pspclk_rate(struct ast2700_scu0 *scu)
{
	u32 chip_id1 = readl(&scu->chip_id1);
	u32 hwstrap1 = readl(&scu->hwstrap1);
	u32 rate;
	int cpuclk_set;

	if (chip_id1 & SCU_HW_REVISION_ID) {
		cpuclk_set = (hwstrap1 & SCU_CPUCLK_MASK) >> SCU_CPUCLK_SHIFT;
		switch (cpuclk_set) {
		case 0:
			rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL);
			break;
		case 1:
		case 2:
		case 3:
		case 6:
		case 7:
			rate = ast2700_soc0_get_hpll_rate(scu);
			break;
		case 4:
			rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL) / 2;
			break;
		case 5:
			rate = ast2700_soc0_get_hpll_rate(scu) / 2;
			break;
		default:
			rate = ast2700_soc0_get_hpll_rate(scu);
			break;
		}
	} else {
		if (hwstrap1 & BIT(4))
			rate = ast2700_soc0_get_hpll_rate(scu);
		else
			rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL);
	}
	return rate;
}

static u32 ast2700_soc0_get_axi0clk_rate(struct ast2700_scu0 *scu)
{
	return ast2700_soc0_get_pspclk_rate(scu) / 2;
}

#define SCU_AHB_DIV_MASK		GENMASK(6, 5)
#define SCU_AHB_DIV_SHIFT		5
static u32 hclk_ast2700a1_div_table[] = {
	6, 5, 4, 7,
};

static u32 ast2700_soc0_get_hclk_rate(struct ast2700_scu0 *scu)
{
	u32 hwstrap1 = readl(&scu->hwstrap1);
	u32 chip_id1 = readl(&scu->chip_id1);
	u32 src_clk;
	int div;

	if (chip_id1 & SCU_HW_REVISION_ID) {
		if (hwstrap1 & BIT(7))
			src_clk = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL);
		else
			src_clk = ast2700_soc0_get_hpll_rate(scu);

		div = (hwstrap1 & SCU_AHB_DIV_MASK) >> SCU_AHB_DIV_SHIFT;
		div = hclk_ast2700a1_div_table[div];
	} else {
		if (hwstrap1 & BIT(7))
			src_clk = ast2700_soc0_get_hpll_rate(scu);
		else
			src_clk = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL);

		div = (hwstrap1 & SCU_AHB_DIV_MASK) >> SCU_AHB_DIV_SHIFT;

		if (!div)
			div = 4;
		else
			div = (div + 1) * 2;
	}
	return (src_clk / div);
}

static u32 ast2700_soc0_get_axi1clk_rate(struct ast2700_scu0 *scu)
{
	if (readl(&scu->chip_id1) & SCU_HW_REVISION_ID)
		return ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL) / 4;
	else
		return ast2700_soc0_get_hclk_rate(scu);
}

#define SCU0_CLKSEL1_PCLK_DIV_MASK		GENMASK(25, 23)
#define SCU0_CLKSEL1_PCLK_DIV_SHIFT		23

static u32 ast2700_soc0_get_pclk_rate(struct ast2700_scu0 *scu)
{
	u32 rate = ast2700_soc0_get_axi0clk_rate(scu);
	u32 clksel1 = readl(&scu->clk_sel1);
	int div;

	div = (clksel1 & SCU0_CLKSEL1_PCLK_DIV_MASK) >>
			    SCU0_CLKSEL1_PCLK_DIV_SHIFT;

	return (rate / ((div + 1) * 2));
}

#define SCU_CLKSEL1_MPHYCLK_SEL_MASK		GENMASK(19, 18)
#define SCU_CLKSEL1_MPHYCLK_SEL_SHIFT		18
#define SCU_CLKSEL1_MPHYCLK_DIV_MASK		GENMASK(7, 0)
static u32 ast2700_soc0_get_mphyclk_rate(struct ast2700_scu0 *scu)
{
	int div = readl(&scu->mphyclk_para) & SCU_CLKSEL1_MPHYCLK_DIV_MASK;
	u32 chip_id1 = readl(&scu->chip_id1);
	u32 clk_sel2;
	int clk_sel;
	u32 rate = 0;

	if (chip_id1 & SCU_HW_REVISION_ID) {
		clk_sel2 = readl(&scu->clk_sel2);
		clk_sel = (clk_sel2 & SCU_CLKSEL1_MPHYCLK_SEL_MASK)
			  >> SCU_CLKSEL1_MPHYCLK_SEL_SHIFT;
		switch (clk_sel) {
		case 0:
			rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL);
			break;
		case 1:
			rate = ast2700_soc0_get_hpll_rate(scu);
			break;
		case 2:
			rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_DPLL);
			break;
		case 3:
			rate = 26000000;
			break;
		}
	} else {
		rate = ast2700_soc0_get_hpll_rate(scu);
	}

	return (rate / (div + 1));
}

static void ast2700_mphy_clk_init(struct ast2700_scu0 *scu)
{
	u32 clksrc1, rate = 0;
	int i;

	/* set mphy clk */
	if (readl(&scu->chip_id1) & SCU_HW_REVISION_ID) {
		clksrc1 = (readl(&scu->clk_sel2) & SCU_CLKSEL1_MPHYCLK_SEL_MASK)
			  >> SCU_CLKSEL1_MPHYCLK_SEL_SHIFT;
		switch (clksrc1) {
		case 0:
			rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL);
			break;
		case 1:
			rate = ast2700_soc0_get_hpll_rate(scu);
			break;
		case 2:
			rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_DPLL);
			break;
		case 3:
			rate = 26000000;
			break;
		}
	} else {
		rate = ast2700_soc0_get_hpll_rate(scu);
	}

	for (i = 1; i < 256; i++) {
		if ((rate / i) <= 26000000)
			break;
	}

	/* register defined the value plus 1 is divider*/
	i--;
	writel(i, &scu->mphyclk_para);
}

#define SCU_CLKSRC1_EMMC_DIV_MASK		GENMASK(14, 12)
#define SCU_CLKSRC1_EMMC_DIV_SHIFT		12
#define SCU_CLKSRC1_EMMC_SEL			BIT(11)
static u32 ast2700_soc0_get_emmcclk_rate(struct ast2700_scu0 *scu)
{
	u32 clksel1 = readl(&scu->clk_sel1);
	u32 rate;
	int div;

	div = (clksel1 & SCU_CLKSRC1_EMMC_DIV_MASK) >> SCU_CLKSRC1_EMMC_DIV_SHIFT;

	if (clksel1 & SCU_CLKSRC1_EMMC_SEL)
		rate = ast2700_soc0_get_hpll_rate(scu) / 4;
	else
		rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL) / 4;

	return (rate / ((div + 1) * 2));
}

static void ast2700_emmc_init(struct ast2700_scu0 *scu)
{
	u32 clksrc1, rate, div;
	int i;

	/* set clk/cmd driving */
	writel(2, &scu->gpio18d0_ioctrl); /* clk driving */
	writel(1, &scu->gpio18d1_ioctrl); /* cmd driving */
	writel(1, &scu->gpio18d2_ioctrl); /* data0 driving */
	writel(1, &scu->gpio18d3_ioctrl); /* data1 driving */
	writel(1, &scu->gpio18d4_ioctrl); /* data2 driving */
	writel(1, &scu->gpio18d5_ioctrl); /* data2 driving */

	/* emmc clk: set clk src mpll/4:400Mhz */
	clksrc1 = readl(&scu->clk_sel1);
	rate = ast2700_soc0_get_pll_rate(scu, SCU0_CLK_MPLL) / 4;
	for (i = 0; i < 8; i++) {
		div = (i + 1) * 2;
		if ((rate / div) <= 200000000)
			break;
	}

	clksrc1 &= ~(SCU_CLKSRC1_EMMC_DIV_MASK | SCU_CLKSRC1_EMMC_SEL);
	clksrc1 |= (i << SCU_CLKSRC1_EMMC_DIV_SHIFT);
	writel(clksrc1, &scu->clk_sel1);
}

static void ast2700_vga_clk_init(struct ast2700_scu0 *scu)
{
	if ((readl(&scu->chip_id1) & SCU_HW_REVISION_ID) == 0)
		return;

	// Use d0clk/d1clk which generated from hpll for vga0/1 after A0
	// Use CRT1clk as soc display source
	setbits_le32(&scu->clk_sel3, BIT(14) | BIT(13) | BIT(12));
}

static u32 ast2700_soc0_get_uartclk_rate(struct ast2700_scu0 *scu)
{
	u32 clksel2 = readl(&scu->clk_sel2);
	u32 div = 1;
	u32 rate;

	if (clksel2 & BIT(15))
		rate = 192000000;
	else
		rate = 24000000;

	if (clksel2 & BIT(30))
		div = 13;
	return (rate / div);
}

static ulong ast2700_soc0_clk_get_rate(struct clk *clk)
{
	struct ast2700_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	switch (clk->id) {
	case SCU0_CLK_PSP:
		rate = ast2700_soc0_get_pspclk_rate((struct ast2700_scu0 *)priv->reg);
		break;
	case SCU0_CLK_HPLL:
		rate = ast2700_soc0_get_hpll_rate((struct ast2700_scu0 *)priv->reg);
		break;
	case SCU0_CLK_DPLL:
	case SCU0_CLK_MPLL:
		rate = ast2700_soc0_get_pll_rate((struct ast2700_scu0 *)priv->reg, clk->id);
		break;
	case SCU0_CLK_AXI0:
		rate = ast2700_soc0_get_axi0clk_rate((struct ast2700_scu0 *)priv->reg);
		break;
	case SCU0_CLK_AXI1:
		rate = ast2700_soc0_get_axi1clk_rate((struct ast2700_scu0 *)priv->reg);
		break;
	case SCU0_CLK_AHB:
		rate = ast2700_soc0_get_hclk_rate((struct ast2700_scu0 *)priv->reg);
		break;
	case SCU0_CLK_APB:
		rate = ast2700_soc0_get_pclk_rate((struct ast2700_scu0 *)priv->reg);
		break;
	case SCU0_CLK_GATE_EMMCCLK:
		rate = ast2700_soc0_get_emmcclk_rate((struct ast2700_scu0 *)priv->reg);
		break;
	case SCU0_CLK_GATE_UART4CLK:
		rate = ast2700_soc0_get_uartclk_rate((struct ast2700_scu0 *)priv->reg);
		break;
	case SCU0_CLK_MPHY:
		rate = ast2700_soc0_get_mphyclk_rate((struct ast2700_scu0 *)priv->reg);
		break;
	default:
		debug("%s: unknown clk %ld\n", __func__, clk->id);
		return -ENOENT;
	}

	return rate;
}

static int ast2700_soc0_clk_enable(struct clk *clk)
{
	struct ast2700_clk_priv *priv = dev_get_priv(clk->dev);
	struct ast2700_scu0 *scu = (struct ast2700_scu0 *)priv->reg;
	u32 clkgate_bit = BIT(clk->id);

	writel(clkgate_bit, &scu->clkgate_clr);

	return 0;
}

static const struct clk_ops ast2700_soc0_clk_ops = {
	.get_rate = ast2700_soc0_clk_get_rate,
	.enable = ast2700_soc0_clk_enable,
};

static void ast2700_init_mac_clk(struct ast2700_scu1 *scu)
{
	u32 src_clk = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_HPLL);
	u32 reg_280;
	u8 div_idx;

	/* The MAC source clock selects HPLL only, and the default clock
	 * setting is 200 Mhz.
	 * Calculate the corresponding divider:
	 * 1: div 2
	 * 2: div 3
	 * ...
	 * 7: div 8
	 */
	for (div_idx = 1; div_idx <= 7; div_idx++)
		if (DIV_ROUND_UP(src_clk, div_idx + 1) == 200000000)
			break;

	if (div_idx == 8) {
		pr_err("MAC clock cannot divide to 200 MHz\n");
		return;
	}

	/* set HPLL clock divider */
	reg_280 = readl(&scu->clk_sel1);
	reg_280 &= ~GENMASK(31, 29);
	reg_280 |= div_idx << 29;
	writel(reg_280, &scu->clk_sel1);
}

static void ast2700_init_rgmii_clk(struct ast2700_scu1 *scu)
{
	u32 reg_284 = readl(&scu->clk_sel2);
	u32 src_clk = ast2700_soc1_get_pll_rate(scu, RGMII_DEFAULT_CLK_SRC);

	if (RGMII_DEFAULT_CLK_SRC == SCU1_CLK_HPLL) {
		u32 reg_280;
		u8 div_idx;

		/* Calculate the corresponding divider:
		 * 1: div 4
		 * 2: div 6
		 * ...
		 * 7: div 16
		 */
		for (div_idx = 1; div_idx <= 7; div_idx++) {
			u8 div = 4 + 2 * (div_idx - 1);

			if (DIV_ROUND_UP(src_clk, div) == 125000000)
				break;
		}
		if (div_idx == 8) {
			pr_err("RGMII using HPLL cannot divide to 125 MHz\n");
			return;
		}

		/* set HPLL clock divider */
		reg_280 = readl(&scu->clk_sel1);
		reg_280 &= ~GENMASK(27, 25);
		reg_280 |= div_idx << 25;
		writel(reg_280, &scu->clk_sel1);

		/* select HPLL clock source */
		reg_284 &= ~BIT(18);
	} else {
		/* APLL clock divider is fixed to 8 */
		if (DIV_ROUND_UP(src_clk, 8) != 125000000) {
			pr_err("RGMII using APLL cannot divide to 125 MHz\n");
			return;
		}

		/* select APLL clock source */
		reg_284 |= BIT(18);
	}

	writel(reg_284, &scu->clk_sel2);
}

static void ast2700_init_rmii_clk(struct ast2700_scu1 *scu)
{
	u32 src_clk = ast2700_soc1_get_pll_rate(scu, SCU1_CLK_HPLL);
	u32 reg_280;
	u8 div_idx;

	/* The RMII source clock selects HPLL only.
	 * Calculate the corresponding divider:
	 * 1: div 8
	 * 2: div 12
	 * ...
	 * 7: div 32
	 */
	for (div_idx = 1; div_idx <= 7; div_idx++) {
		u8 div = 8 + 4 * (div_idx - 1);

		if (DIV_ROUND_UP(src_clk, div) == 50000000)
			break;
	}
	if (div_idx == 8) {
		pr_err("RMII using HPLL cannot divide to 50 MHz\n");
		return;
	}

	/* set RMII clock divider */
	reg_280 = readl(&scu->clk_sel1);
	reg_280 &= ~GENMASK(23, 21);
	reg_280 |= div_idx << 21;
	writel(reg_280, &scu->clk_sel1);
}

static void ast2700_init_spi(struct ast2700_scu1 *scu)
{
	writel(readl(&scu->io_driving8) | 0x0000aaaa, &scu->io_driving8);	/* fwspi driving */
	writel(readl(&scu->io_driving3) | 0x00000aaa, &scu->io_driving3);	/* spi0 driving */
	writel(readl(&scu->io_driving3) | 0x0aaa0000, &scu->io_driving3);	/* spi1 driving */
	writel(readl(&scu->io_driving4) | 0x00002aaa, &scu->io_driving4);	/* spi2 driving */
}

#define SCU1_CLK_I3C_DIV_MASK	GENMASK(25, 23)
#define SCU1_CLK_I3C_DIV(n)	((n) - 1)
static void ast2700_init_i3c_clk(struct ast2700_scu1 *scu)
{
	u32 reg_284;

	/* I3C 250MHz = HPLL/4 */
	reg_284 = readl(&scu->clk_sel2);
	reg_284 &= ~SCU1_CLK_I3C_DIV_MASK;
	reg_284 |= FIELD_PREP(SCU1_CLK_I3C_DIV_MASK, SCU1_CLK_I3C_DIV(4));
	writel(reg_284, &scu->clk_sel2);
}

static int ast2700_clk1_init(struct udevice *dev)
{
	struct ast2700_clk_priv *priv = dev_get_priv(dev);
	struct ast2700_scu1 *scu = (struct ast2700_scu1 *)priv->reg;

	ast2700_init_spi(scu);
	ast2700_init_mac_clk(scu);
	ast2700_init_rgmii_clk(scu);
	ast2700_init_rmii_clk(scu);
	ast2700_init_sdclk(scu);
	ast2700_init_i3c_clk(scu);

	return 0;
}

static int ast2700_clk0_init(struct udevice *dev)
{
	struct ast2700_clk_priv *priv = dev_get_priv(dev);
	struct ast2700_scu0 *scu = (struct ast2700_scu0 *)priv->reg;

	ast2700_emmc_init(scu);
	ast2700_mphy_clk_init(scu);
	ast2700_vga_clk_init(scu);

	return 0;
}

static int ast2700_clk_probe(struct udevice *dev)
{
	struct ast2700_clk_priv *priv = dev_get_priv(dev);

	priv->init = (ast2700_clk_init_fn)dev_get_driver_data(dev);
	priv->reg =  (void __iomem *)dev_read_addr_ptr(dev);

	if (priv->init)
		return priv->init(dev);

	return 0;
}

static int ast2700_clk_bind(struct udevice *dev)
{
	struct udevice *sysreset_dev, *rst_dev;
	int ret;

	/* The system reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "ast_sysreset", "reset", &sysreset_dev);
	if (ret)
		debug("Warning: No sysreset driver: ret = %d\n", ret);

	/* Bind the per-SCU reset controller to the same ofnode so that
	 * <&syscon0/1 RESET_X> phandle references resolve to a UCLASS_RESET
	 * device. This pairs with the airoha-style binding pattern.
	 */
	if (CONFIG_IS_ENABLED(RESET_AST2700)) {
		ret = device_bind_driver_to_node(dev, "ast2700_reset", "reset",
						 dev_ofnode(dev), &rst_dev);
		if (ret)
			debug("Warning: failed to bind reset controller: ret = %d\n", ret);
	}

	return 0;
}

static const struct udevice_id ast2700_soc1_clk_ids[] = {
	{ .compatible = "aspeed,ast2700-scu1", .data = (ulong)&ast2700_clk1_init },
	{ },
};

U_BOOT_DRIVER(aspeed_ast2700_soc1_clk) = {
	.name = "aspeed_ast2700_scu1",
	.id = UCLASS_CLK,
	.of_match = ast2700_soc1_clk_ids,
	.priv_auto = sizeof(struct ast2700_clk_priv),
	.ops = &ast2700_soc1_clk_ops,
	.probe = ast2700_clk_probe,
	.bind = ast2700_clk_bind,
};

static const struct udevice_id ast2700_soc0_clk_ids[] = {
	{ .compatible = "aspeed,ast2700-scu0", .data = (ulong)&ast2700_clk0_init },
	{ },
};

U_BOOT_DRIVER(aspeed_ast2700_soc0_clk) = {
	.name = "aspeed_ast2700_scu0",
	.id = UCLASS_CLK,
	.of_match = ast2700_soc0_clk_ids,
	.priv_auto = sizeof(struct ast2700_clk_priv),
	.ops = &ast2700_soc0_clk_ops,
	.probe = ast2700_clk_probe,
	.bind = ast2700_clk_bind,
};
