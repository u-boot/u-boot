/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <libfdt.h>
#include <linux/kernel.h>
#include <mach/init.h>

#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD3)
static const struct uniphier_board_data ph1_sld3_data = {
	.dram_ch0_base	= 0x80000000,
	.dram_ch0_size	= 0x20000000,
	.dram_ch0_width	= 32,
	.dram_ch1_base	= 0xc0000000,
	.dram_ch1_size	= 0x20000000,
	.dram_ch1_width	= 16,
	.dram_ch2_base	= 0xc0000000,
	.dram_ch2_size	= 0x10000000,
	.dram_ch2_width	= 16,
	.dram_freq	= 1600,
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD4)
static const struct uniphier_board_data ph1_ld4_data = {
	.dram_ch0_base	= 0x80000000,
	.dram_ch0_size	= 0x10000000,
	.dram_ch0_width	= 16,
	.dram_ch1_base	= 0x90000000,
	.dram_ch1_size	= 0x10000000,
	.dram_ch1_width	= 16,
	.dram_freq	= 1600,
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO4)
static const struct uniphier_board_data ph1_pro4_data = {
	.dram_ch0_base	= 0x80000000,
	.dram_ch0_size	= 0x20000000,
	.dram_ch0_width	= 32,
	.dram_ch1_base	= 0xa0000000,
	.dram_ch1_size	= 0x20000000,
	.dram_ch1_width	= 32,
	.dram_freq	= 1600,
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD8)
static const struct uniphier_board_data ph1_sld8_data = {
	.dram_ch0_base	= 0x80000000,
	.dram_ch0_size	= 0x10000000,
	.dram_ch0_width	= 16,
	.dram_ch1_base	= 0x90000000,
	.dram_ch1_size	= 0x10000000,
	.dram_ch1_width	= 16,
	.dram_freq	= 1333,
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO5)
static const struct uniphier_board_data ph1_pro5_data = {
	.dram_ch0_base  = 0x80000000,
	.dram_ch0_size  = 0x20000000,
	.dram_ch0_width = 32,
	.dram_ch1_base  = 0xa0000000,
	.dram_ch1_size  = 0x20000000,
	.dram_ch1_width = 32,
	.dram_freq      = 1866,
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_PROXSTREAM2) || \
	defined(CONFIG_ARCH_UNIPHIER_PH1_LD6B)
static const struct uniphier_board_data proxstream2_data = {
	.dram_ch0_base  = 0x80000000,
	.dram_ch0_size  = 0x40000000,
	.dram_ch0_width = 32,
	.dram_ch1_base  = 0xc0000000,
	.dram_ch1_size  = 0x20000000,
	.dram_ch1_width = 32,
	.dram_ch2_base  = 0xe0000000,
	.dram_ch2_size  = 0x20000000,
	.dram_ch2_width = 16,
	.dram_freq      = 1866,
};
#endif

struct uniphier_board_id {
	const char *compatible;
	const struct uniphier_board_data *param;
};

static const struct uniphier_board_id uniphier_boards[] = {
#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD3)
	{ "socionext,ph1-sld3", &ph1_sld3_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD4)
	{ "socionext,ph1-ld4", &ph1_ld4_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO4)
	{ "socionext,ph1-pro4", &ph1_pro4_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD8)
	{ "socionext,ph1-sld8", &ph1_sld8_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO5)
	{ "socionext,ph1-pro5", &ph1_pro5_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PROXSTREAM2)
	{ "socionext,proxstream2", &proxstream2_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD6B)
	{ "socionext,ph1-ld6b", &proxstream2_data, },
#endif
};

const struct uniphier_board_data *uniphier_get_board_param(const void *fdt)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uniphier_boards); i++) {
		if (!fdt_node_check_compatible(fdt, 0,
					       uniphier_boards[i].compatible))
			return uniphier_boards[i].param;
	}

	return NULL;
}
