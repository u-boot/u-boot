/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"
#include "../sg-regs.h"

#undef DPLL_SSC_RATE_1PER

static int dpll_init(unsigned int dram_freq)
{
	u32 tmp;

	/*
	 * Set Frequency
	 * Set 0xc(1600MHz)/0xd(1333MHz)/0xe(1066MHz)
	 * to FOUT (DPLLCTRL.bit[29:20])
	 */
	tmp = readl(SC_DPLLCTRL);
	tmp &= ~0x000f0000;
	switch (dram_freq) {
	case 1333:
		tmp |= 0x000d0000;
		break;
	case 1600:
		tmp |= 0x000c0000;
		break;
	default:
		pr_err("Unsupported frequency");
		return -EINVAL;
	}

#if defined(DPLL_SSC_RATE_1PER)
	tmp &= ~SC_DPLLCTRL_SSC_RATE;
#else
	tmp |= SC_DPLLCTRL_SSC_RATE;
#endif
	writel(tmp, SC_DPLLCTRL);

	tmp = readl(SC_DPLLCTRL2);
	tmp |= SC_DPLLCTRL2_NRSTDS;
	writel(tmp, SC_DPLLCTRL2);

	return 0;
}

static void upll_init(void)
{
	u32 tmp, clk_mode_upll, clk_mode_axosel;

	tmp = readl(SG_PINMON0);
	clk_mode_upll   = tmp & SG_PINMON0_CLK_MODE_UPLLSRC_MASK;
	clk_mode_axosel = tmp & SG_PINMON0_CLK_MODE_AXOSEL_MASK;

	/* set 0 to SNRT(UPLLCTRL.bit28) and K_LD(UPLLCTRL.bit[27]) */
	tmp = readl(SC_UPLLCTRL);
	tmp &= ~0x18000000;
	writel(tmp, SC_UPLLCTRL);

	if (clk_mode_upll == SG_PINMON0_CLK_MODE_UPLLSRC_DEFAULT) {
		if (clk_mode_axosel == SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ_U ||
		    clk_mode_axosel == SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ_A) {
			/* AXO: 25MHz */
			tmp &= ~0x07ffffff;
			tmp |= 0x0228f5c0;
		} else {
			/* AXO: default 24.576MHz */
			tmp &= ~0x07ffffff;
			tmp |= 0x02328000;
		}
	}

	writel(tmp, SC_UPLLCTRL);

	/* set 1 to K_LD(UPLLCTRL.bit[27]) */
	tmp |= 0x08000000;
	writel(tmp, SC_UPLLCTRL);

	/* wait 10 usec */
	udelay(10);

	/* set 1 to SNRT(UPLLCTRL.bit[28]) */
	tmp |= 0x10000000;
	writel(tmp, SC_UPLLCTRL);
}

static void vpll_init(void)
{
	u32 tmp, clk_mode_axosel;

	tmp = readl(SG_PINMON0);
	clk_mode_axosel = tmp & SG_PINMON0_CLK_MODE_AXOSEL_MASK;

	/* set 1 to VPLA27WP and VPLA27WP */
	tmp = readl(SC_VPLL27ACTRL);
	tmp |= 0x00000001;
	writel(tmp, SC_VPLL27ACTRL);
	tmp = readl(SC_VPLL27BCTRL);
	tmp |= 0x00000001;
	writel(tmp, SC_VPLL27BCTRL);

	/* Set 0 to VPLA_K_LD and VPLB_K_LD */
	tmp = readl(SC_VPLL27ACTRL3);
	tmp &= ~0x10000000;
	writel(tmp, SC_VPLL27ACTRL3);
	tmp = readl(SC_VPLL27BCTRL3);
	tmp &= ~0x10000000;
	writel(tmp, SC_VPLL27BCTRL3);

	/* Set 0 to VPLA_SNRST and VPLB_SNRST */
	tmp = readl(SC_VPLL27ACTRL2);
	tmp &= ~0x10000000;
	writel(tmp, SC_VPLL27ACTRL2);
	tmp = readl(SC_VPLL27BCTRL2);
	tmp &= ~0x10000000;
	writel(tmp, SC_VPLL27BCTRL2);

	/* Set 0x20 to VPLA_SNRST and VPLB_SNRST */
	tmp = readl(SC_VPLL27ACTRL2);
	tmp &= ~0x0000007f;
	tmp |= 0x00000020;
	writel(tmp, SC_VPLL27ACTRL2);
	tmp = readl(SC_VPLL27BCTRL2);
	tmp &= ~0x0000007f;
	tmp |= 0x00000020;
	writel(tmp, SC_VPLL27BCTRL2);

	if (clk_mode_axosel == SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ_U ||
	    clk_mode_axosel == SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ_A) {
		/* AXO: 25MHz */
		tmp = readl(SC_VPLL27ACTRL3);
		tmp &= ~0x000fffff;
		tmp |= 0x00066664;
		writel(tmp, SC_VPLL27ACTRL3);
		tmp = readl(SC_VPLL27BCTRL3);
		tmp &= ~0x000fffff;
		tmp |= 0x00066664;
		writel(tmp, SC_VPLL27BCTRL3);
	} else {
		/* AXO: default 24.576MHz */
		tmp = readl(SC_VPLL27ACTRL3);
		tmp &= ~0x000fffff;
		tmp |= 0x000f5800;
		writel(tmp, SC_VPLL27ACTRL3);
		tmp = readl(SC_VPLL27BCTRL3);
		tmp &= ~0x000fffff;
		tmp |= 0x000f5800;
		writel(tmp, SC_VPLL27BCTRL3);
	}

	/* Set 1 to VPLA_K_LD and VPLB_K_LD */
	tmp = readl(SC_VPLL27ACTRL3);
	tmp |= 0x10000000;
	writel(tmp, SC_VPLL27ACTRL3);
	tmp = readl(SC_VPLL27BCTRL3);
	tmp |= 0x10000000;
	writel(tmp, SC_VPLL27BCTRL3);

	/* wait 10 usec */
	udelay(10);

	/* Set 0 to VPLA_SNRST and VPLB_SNRST */
	tmp = readl(SC_VPLL27ACTRL2);
	tmp |= 0x10000000;
	writel(tmp, SC_VPLL27ACTRL2);
	tmp = readl(SC_VPLL27BCTRL2);
	tmp |= 0x10000000;
	writel(tmp, SC_VPLL27BCTRL2);

	/* set 0 to VPLA27WP and VPLA27WP */
	tmp = readl(SC_VPLL27ACTRL);
	tmp &= ~0x00000001;
	writel(tmp, SC_VPLL27ACTRL);
	tmp = readl(SC_VPLL27BCTRL);
	tmp |= ~0x00000001;
	writel(tmp, SC_VPLL27BCTRL);
}

int uniphier_ld4_pll_init(const struct uniphier_board_data *bd)
{
	int ret;

	ret = dpll_init(bd->dram_freq);
	if (ret)
		return ret;
	upll_init();
	vpll_init();

	/*
	 * Wait 500 usec until dpll get stable
	 * We wait 10 usec in upll_init() and vpll_init()
	 * so 20 usec can be saved here.
	 */
	udelay(480);

	return 0;
}
