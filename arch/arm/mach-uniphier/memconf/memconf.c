/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 * Copyright (C) 2016      Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "../init.h"
#include "../sg-regs.h"

int memconf_init(const struct uniphier_board_data *bd)
{
	u32 tmp;
	unsigned long size_per_word;

	tmp = readl(SG_MEMCONF);

	tmp &= ~(SG_MEMCONF_CH0_SZ_MASK | SG_MEMCONF_CH0_NUM_MASK);

	switch (bd->dram_ch[0].width) {
	case 16:
		tmp |= SG_MEMCONF_CH0_NUM_1;
		size_per_word = bd->dram_ch[0].size;
		break;
	case 32:
		tmp |= SG_MEMCONF_CH0_NUM_2;
		size_per_word = bd->dram_ch[0].size >> 1;
		break;
	default:
		pr_err("error: unsupported DRAM Ch0 width\n");
		return -EINVAL;
	}

	/* Set DDR size */
	switch (size_per_word) {
	case SZ_64M:
		tmp |= SG_MEMCONF_CH0_SZ_64M;
		break;
	case SZ_128M:
		tmp |= SG_MEMCONF_CH0_SZ_128M;
		break;
	case SZ_256M:
		tmp |= SG_MEMCONF_CH0_SZ_256M;
		break;
	case SZ_512M:
		tmp |= SG_MEMCONF_CH0_SZ_512M;
		break;
	case SZ_1G:
		tmp |= SG_MEMCONF_CH0_SZ_1G;
		break;
	default:
		pr_err("error: unsupported DRAM Ch0 size\n");
		return -EINVAL;
	}

	tmp &= ~(SG_MEMCONF_CH1_SZ_MASK | SG_MEMCONF_CH1_NUM_MASK);

	switch (bd->dram_ch[1].width) {
	case 16:
		tmp |= SG_MEMCONF_CH1_NUM_1;
		size_per_word = bd->dram_ch[1].size;
		break;
	case 32:
		tmp |= SG_MEMCONF_CH1_NUM_2;
		size_per_word = bd->dram_ch[1].size >> 1;
		break;
	default:
		pr_err("error: unsupported DRAM Ch1 width\n");
		return -EINVAL;
	}

	switch (size_per_word) {
	case SZ_64M:
		tmp |= SG_MEMCONF_CH1_SZ_64M;
		break;
	case SZ_128M:
		tmp |= SG_MEMCONF_CH1_SZ_128M;
		break;
	case SZ_256M:
		tmp |= SG_MEMCONF_CH1_SZ_256M;
		break;
	case SZ_512M:
		tmp |= SG_MEMCONF_CH1_SZ_512M;
		break;
	case SZ_1G:
		tmp |= SG_MEMCONF_CH1_SZ_1G;
		break;
	default:
		pr_err("error: unsupported DRAM Ch1 size\n");
		return -EINVAL;
	}

	if (bd->dram_ch[0].base + bd->dram_ch[0].size < bd->dram_ch[1].base)
		tmp |= SG_MEMCONF_SPARSEMEM;
	else
		tmp &= ~SG_MEMCONF_SPARSEMEM;

	writel(tmp, SG_MEMCONF);

	return 0;
}
