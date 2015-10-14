/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <mach/init.h>
#include <mach/sg-regs.h>

int proxstream2_memconf_init(const struct uniphier_board_data *bd)
{
	u32 tmp;
	unsigned long size_per_word;

	tmp = readl(SG_MEMCONF);

	tmp &= ~(SG_MEMCONF_CH2_SZ_MASK | SG_MEMCONF_CH2_NUM_MASK);

	switch (bd->dram_ch2_width) {
	case 16:
		tmp |= SG_MEMCONF_CH2_NUM_1;
		size_per_word = bd->dram_ch2_size;
		break;
	case 32:
		tmp |= SG_MEMCONF_CH2_NUM_2;
		size_per_word = bd->dram_ch2_size >> 1;
		break;
	default:
		pr_err("error: unsupported DRAM Ch2 width\n");
		return -EINVAL;
	}

	/* Set DDR size */
	switch (size_per_word) {
	case SZ_64M:
		tmp |= SG_MEMCONF_CH2_SZ_64M;
		break;
	case SZ_128M:
		tmp |= SG_MEMCONF_CH2_SZ_128M;
		break;
	case SZ_256M:
		tmp |= SG_MEMCONF_CH2_SZ_256M;
		break;
	case SZ_512M:
		tmp |= SG_MEMCONF_CH2_SZ_512M;
		break;
	default:
		pr_err("error: unsupported DRAM Ch2 size\n");
		return -EINVAL;
	}

	if (size_per_word)
		tmp &= ~SG_MEMCONF_CH2_DISABLE;
	else
		tmp |= SG_MEMCONF_CH2_DISABLE;

	writel(tmp, SG_MEMCONF);

	return 0;
}
