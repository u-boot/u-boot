/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/types.h>
#include <linux/sizes.h>
#include <mach/sg-regs.h>

static inline u32 sg_memconf_val_ch2(unsigned long size, int num)
{
	int size_mb = size / num;
	u32 ret;

	switch (size_mb) {
	case SZ_64M:
		ret = SG_MEMCONF_CH2_SZ_64M;
		break;
	case SZ_128M:
		ret = SG_MEMCONF_CH2_SZ_128M;
		break;
	case SZ_256M:
		ret = SG_MEMCONF_CH2_SZ_256M;
		break;
	case SZ_512M:
		ret = SG_MEMCONF_CH2_SZ_512M;
		break;
	default:
		BUG();
		break;
	}

	switch (num) {
	case 1:
		ret |= SG_MEMCONF_CH2_NUM_1;
		break;
	case 2:
		ret |= SG_MEMCONF_CH2_NUM_2;
		break;
	default:
		BUG();
		break;
	}
	return ret;
}

u32 memconf_additional_val(void)
{
	return sg_memconf_val_ch2(CONFIG_SDRAM2_SIZE, CONFIG_DDR_NUM_CH2);
}
