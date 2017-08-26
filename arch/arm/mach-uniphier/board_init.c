/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <linux/io.h>

#include "init.h"
#include "micro-support-card.h"
#include "sg-regs.h"
#include "soc-info.h"

DECLARE_GLOBAL_DATA_PTR;

static void uniphier_setup_xirq(void)
{
	const void *fdt = gd->fdt_blob;
	int soc_node, aidet_node;
	const fdt32_t *val;
	unsigned long aidet_base;
	u32 tmp;

	soc_node = fdt_path_offset(fdt, "/soc");
	if (soc_node < 0)
		return;

	aidet_node = fdt_subnode_offset_namelen(fdt, soc_node, "aidet", 5);
	if (aidet_node < 0)
		return;

	val = fdt_getprop(fdt, aidet_node, "reg", NULL);
	if (!val)
		return;

	aidet_base = fdt32_to_cpu(*val);

	tmp = readl(aidet_base + 8);	/* AIDET DETCONFR2 */
	tmp |= 0x00ff0000;		/* Set XIRQ0-7 low active */
	writel(tmp, aidet_base + 8);

	tmp = readl(0x55000090);	/* IRQCTL */
	tmp |= 0x000000ff;
	writel(tmp, 0x55000090);
}

#ifdef CONFIG_ARCH_UNIPHIER_LD11
static void uniphier_ld11_misc_init(void)
{
	sg_set_pinsel(149, 14, 8, 4);	/* XIRQ0    -> XIRQ0 */
	sg_set_iectrl(149);
	sg_set_pinsel(153, 14, 8, 4);	/* XIRQ4    -> XIRQ4 */
	sg_set_iectrl(153);
}
#endif

#ifdef CONFIG_ARCH_UNIPHIER_LD20
static void uniphier_ld20_misc_init(void)
{
	sg_set_pinsel(149, 14, 8, 4);	/* XIRQ0    -> XIRQ0 */
	sg_set_iectrl(149);
	sg_set_pinsel(153, 14, 8, 4);	/* XIRQ4    -> XIRQ4 */
	sg_set_iectrl(153);

	/* ES1 errata: increase VDD09 supply to suppress VBO noise */
	if (uniphier_get_soc_revision() == 1) {
		writel(0x00000003, 0x6184e004);
		writel(0x00000100, 0x6184e040);
		writel(0x0000b500, 0x6184e024);
		writel(0x00000001, 0x6184e000);
	}
}
#endif

struct uniphier_initdata {
	unsigned int soc_id;
	void (*sbc_init)(void);
	void (*pll_init)(void);
	void (*clk_init)(void);
	void (*misc_init)(void);
};

static const struct uniphier_initdata uniphier_initdata[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	{
		.soc_id = UNIPHIER_LD4_ID,
		.sbc_init = uniphier_ld4_sbc_init,
		.pll_init = uniphier_ld4_pll_init,
		.clk_init = uniphier_ld4_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	{
		.soc_id = UNIPHIER_PRO4_ID,
		.sbc_init = uniphier_sbc_init_savepin,
		.pll_init = uniphier_pro4_pll_init,
		.clk_init = uniphier_pro4_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	{
		.soc_id = UNIPHIER_SLD8_ID,
		.sbc_init = uniphier_ld4_sbc_init,
		.pll_init = uniphier_ld4_pll_init,
		.clk_init = uniphier_ld4_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	{
		.soc_id = UNIPHIER_PRO5_ID,
		.sbc_init = uniphier_sbc_init_savepin,
		.clk_init = uniphier_pro5_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	{
		.soc_id = UNIPHIER_PXS2_ID,
		.sbc_init = uniphier_pxs2_sbc_init,
		.clk_init = uniphier_pxs2_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	{
		.soc_id = UNIPHIER_LD6B_ID,
		.sbc_init = uniphier_pxs2_sbc_init,
		.clk_init = uniphier_pxs2_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11)
	{
		.soc_id = UNIPHIER_LD11_ID,
		.sbc_init = uniphier_ld11_sbc_init,
		.pll_init = uniphier_ld11_pll_init,
		.clk_init = uniphier_ld11_clk_init,
		.misc_init = uniphier_ld11_misc_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	{
		.soc_id = UNIPHIER_LD20_ID,
		.sbc_init = uniphier_ld11_sbc_init,
		.pll_init = uniphier_ld20_pll_init,
		.clk_init = uniphier_ld20_clk_init,
		.misc_init = uniphier_ld20_misc_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS3)
	{
		.soc_id = UNIPHIER_PXS3_ID,
		.sbc_init = uniphier_pxs2_sbc_init,
		.pll_init = uniphier_pxs3_pll_init,
		.clk_init = uniphier_pxs3_clk_init,
	},
#endif
};
UNIPHIER_DEFINE_SOCDATA_FUNC(uniphier_get_initdata, uniphier_initdata)

int board_init(void)
{
	const struct uniphier_initdata *initdata;

	led_puts("U0");

	initdata = uniphier_get_initdata();
	if (!initdata) {
		pr_err("unsupported SoC\n");
		return -EINVAL;
	}

	initdata->sbc_init();

	support_card_init();

	led_puts("U0");

	if (initdata->pll_init)
		initdata->pll_init();

	led_puts("U1");

	if (initdata->clk_init)
		initdata->clk_init();

	led_puts("U2");

	if (initdata->misc_init)
		initdata->misc_init();

	led_puts("U3");

	uniphier_setup_xirq();

	led_puts("U4");

	support_card_late_init();

	led_puts("Uboo");

	return 0;
}
