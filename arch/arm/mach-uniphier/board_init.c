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
	const u32 *val;
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

static void uniphier_nand_pin_init(bool cs2)
{
#ifdef CONFIG_NAND_DENALI
	if (uniphier_pin_init(cs2 ? "nand2cs_grp" : "nand_grp"))
		pr_err("failed to init NAND pins\n");
#endif
}

int board_init(void)
{
	const struct uniphier_board_data *bd;

	led_puts("U0");

	bd = uniphier_get_board_param();
	if (!bd)
		return -ENODEV;

	switch (uniphier_get_soc_type()) {
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	case SOC_UNIPHIER_SLD3:
		uniphier_nand_pin_init(true);
		led_puts("U1");
		uniphier_sld3_pll_init();
		uniphier_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	case SOC_UNIPHIER_LD4:
		uniphier_nand_pin_init(true);
		led_puts("U1");
		uniphier_ld4_pll_init();
		uniphier_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	case SOC_UNIPHIER_PRO4:
		uniphier_nand_pin_init(false);
		led_puts("U1");
		uniphier_pro4_pll_init();
		uniphier_pro4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case SOC_UNIPHIER_SLD8:
		uniphier_nand_pin_init(true);
		led_puts("U1");
		uniphier_ld4_pll_init();
		uniphier_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case SOC_UNIPHIER_PRO5:
		uniphier_nand_pin_init(true);
		led_puts("U1");
		uniphier_pro5_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	case SOC_UNIPHIER_PXS2:
		uniphier_nand_pin_init(true);
		led_puts("U1");
		uniphier_pxs2_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case SOC_UNIPHIER_LD6B:
		uniphier_nand_pin_init(true);
		led_puts("U1");
		uniphier_pxs2_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11)
	case SOC_UNIPHIER_LD11:
		uniphier_nand_pin_init(false);
		sg_set_pinsel(149, 14, 8, 4);	/* XIRQ0    -> XIRQ0 */
		sg_set_iectrl(149);
		sg_set_pinsel(153, 14, 8, 4);	/* XIRQ4    -> XIRQ4 */
		sg_set_iectrl(153);
		led_puts("U1");
		uniphier_ld11_pll_init();
		uniphier_ld11_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	case SOC_UNIPHIER_LD20:
		/* ES1 errata: increase VDD09 supply to suppress VBO noise */
		if (uniphier_get_soc_revision() == 1) {
			writel(0x00000003, 0x6184e004);
			writel(0x00000100, 0x6184e040);
			writel(0x0000b500, 0x6184e024);
			writel(0x00000001, 0x6184e000);
		}
		uniphier_nand_pin_init(false);
		sg_set_pinsel(149, 14, 8, 4);	/* XIRQ0    -> XIRQ0 */
		sg_set_iectrl(149);
		sg_set_pinsel(153, 14, 8, 4);	/* XIRQ4    -> XIRQ4 */
		sg_set_iectrl(153);
		led_puts("U1");
		uniphier_ld20_pll_init(bd);
		uniphier_ld20_clk_init();
		cci500_init(2);
		break;
#endif
	default:
		break;
	}

	uniphier_setup_xirq();

	led_puts("U2");

	support_card_late_init();

	led_puts("U3");

#ifdef CONFIG_ARM64
	uniphier_smp_kick_all_cpus();
#endif

	led_puts("Uboo");

	return 0;
}
