/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <mvebu/mvebu_chip_sar.h>
#include <fdtdec.h>
#include <dm.h>
#include <dm/device-internal.h>

struct sar_ops {
	int (*sar_init_func)(struct udevice *dev);
	int (*sar_dump_func)(struct udevice *dev);
	int (*sar_value_get_func)(struct udevice *dev, enum mvebu_sar_opts sar,
				  struct sar_val *val);
	int (*sar_bootsrc_get)(struct udevice *dev, u32 *idx);
};

struct dm_sar_pdata {
	void __iomem *sar_base;
	const char *sar_name;
};

int mvebu_sar_id_register(struct udevice *dev, u32 sar_id);
