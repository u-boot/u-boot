/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "pll.h"

/* PLL type: SSC */
#define SC_PLLCTRL_SSC_DK_MASK		GENMASK(14, 0)
#define SC_PLLCTRL_SSC_EN		BIT(31)
#define SC_PLLCTRL2_NRSTDS		BIT(28)
#define SC_PLLCTRL2_SSC_JK_MASK		GENMASK(26, 0)

/* PLL type: VPLL27 */
#define SC_VPLL27CTRL_WP		BIT(0)
#define SC_VPLL27CTRL3_K_LD		BIT(28)

/* PLL type: DSPLL */
#define SC_DSPLLCTRL2_K_LD		BIT(28)

int uniphier_ld20_sscpll_init(unsigned long reg_base, unsigned int freq,
			      unsigned int ssc_rate, unsigned int divn)
{
	void __iomem *base;
	u32 tmp;

	base = ioremap(reg_base, SZ_16);
	if (!base)
		return -ENOMEM;

	if (freq != UNIPHIER_PLL_FREQ_DEFAULT) {
		tmp = readl(base);	/* SSCPLLCTRL */
		tmp &= ~SC_PLLCTRL_SSC_DK_MASK;
		tmp |= (487 * freq * ssc_rate / divn / 512) &
							SC_PLLCTRL_SSC_DK_MASK;
		writel(tmp, base);

		tmp = readl(base + 4);
		tmp &= ~SC_PLLCTRL2_SSC_JK_MASK;
		tmp |= (41859 * freq / divn) & SC_PLLCTRL2_SSC_JK_MASK;

		udelay(50);
	}

	tmp = readl(base + 4);		/* SSCPLLCTRL2 */
	tmp |= SC_PLLCTRL2_NRSTDS;
	writel(tmp, base + 4);

	iounmap(base);

	return 0;
}

int uniphier_ld20_sscpll_ssc_en(unsigned long reg_base)
{
	void __iomem *base;
	u32 tmp;

	base = ioremap(reg_base, SZ_16);
	if (!base)
		return -ENOMEM;

	tmp = readl(base);	/* SSCPLLCTRL */
	tmp |= SC_PLLCTRL_SSC_EN;
	writel(tmp, base);

	iounmap(base);

	return 0;
}

int uniphier_ld20_vpll27_init(unsigned long reg_base)
{
	void __iomem *base;
	u32 tmp;

	base = ioremap(reg_base, SZ_16);
	if (!base)
		return -ENOMEM;

	tmp = readl(base);		/* VPLL27CTRL */
	tmp |= SC_VPLL27CTRL_WP;	/* write protect off */
	writel(tmp, base);

	tmp = readl(base + 8);		/* VPLL27CTRL3 */
	tmp |= SC_VPLL27CTRL3_K_LD;
	writel(tmp, base + 8);

	tmp = readl(base);		/* VPLL27CTRL */
	tmp &= ~SC_VPLL27CTRL_WP;	/* write protect on */
	writel(tmp, base);

	iounmap(base);

	return 0;
}

int uniphier_ld20_dspll_init(unsigned long reg_base)
{
	void __iomem *base;
	u32 tmp;

	base = ioremap(reg_base, SZ_16);
	if (!base)
		return -ENOMEM;

	tmp = readl(base + 8);		/* DSPLLCTRL2 */
	tmp |= SC_DSPLLCTRL2_K_LD;
	writel(tmp, base + 8);

	iounmap(base);

	return 0;
}
