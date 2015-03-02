/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <mach/sg-regs.h>

static inline u32 sg_memconf_val_ch0(unsigned long size, int num)
{
	int size_mb = size / num;
	u32 ret;

	switch (size_mb) {
	case SZ_64M:
		ret = SG_MEMCONF_CH0_SZ_64M;
		break;
	case SZ_128M:
		ret = SG_MEMCONF_CH0_SZ_128M;
		break;
	case SZ_256M:
		ret = SG_MEMCONF_CH0_SZ_256M;
		break;
	case SZ_512M:
		ret = SG_MEMCONF_CH0_SZ_512M;
		break;
	case SZ_1G:
		ret = SG_MEMCONF_CH0_SZ_1G;
		break;
	default:
		BUG();
		break;
	}

	switch (num) {
	case 1:
		ret |= SG_MEMCONF_CH0_NUM_1;
		break;
	case 2:
		ret |= SG_MEMCONF_CH0_NUM_2;
		break;
	default:
		BUG();
		break;
	}
	return ret;
}

static inline u32 sg_memconf_val_ch1(unsigned long size, int num)
{
	int size_mb = size / num;
	u32 ret;

	switch (size_mb) {
	case SZ_64M:
		ret = SG_MEMCONF_CH1_SZ_64M;
		break;
	case SZ_128M:
		ret = SG_MEMCONF_CH1_SZ_128M;
		break;
	case SZ_256M:
		ret = SG_MEMCONF_CH1_SZ_256M;
		break;
	case SZ_512M:
		ret = SG_MEMCONF_CH1_SZ_512M;
		break;
	case SZ_1G:
		ret = SG_MEMCONF_CH1_SZ_1G;
		break;
	default:
		BUG();
		break;
	}

	switch (num) {
	case 1:
		ret |= SG_MEMCONF_CH1_NUM_1;
		break;
	case 2:
		ret |= SG_MEMCONF_CH1_NUM_2;
		break;
	default:
		BUG();
		break;
	}
	return ret;
}

void memconf_init(void)
{
	u32 tmp;

	/* Set DDR size */
	tmp = sg_memconf_val_ch0(CONFIG_SDRAM0_SIZE, CONFIG_DDR_NUM_CH0);
	tmp |= sg_memconf_val_ch1(CONFIG_SDRAM1_SIZE, CONFIG_DDR_NUM_CH1);
#if CONFIG_SDRAM0_BASE + CONFIG_SDRAM0_SIZE < CONFIG_SDRAM1_BASE
	tmp |= SG_MEMCONF_SPARSEMEM;
#endif
	writel(tmp, SG_MEMCONF);
}
