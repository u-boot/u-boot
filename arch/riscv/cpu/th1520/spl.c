// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 */
#include <dm.h>
#include <linux/sizes.h>
#include <log.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_dram_init(void)
{
	int ret;
	struct udevice *dev;

	ret = fdtdec_setup_mem_size_base();
	if (ret) {
		printf("failed to setup memory size and base: %d\n", ret);
		return ret;
	}

	/* DDR init */
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		printf("DRAM init failed: %d\n", ret);
		return ret;
	}

	return 0;
}
