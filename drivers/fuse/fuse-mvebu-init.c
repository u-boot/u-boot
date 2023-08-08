// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Copyright (C) 2015-2016 Reinhard Pfau <reinhard.pfau@gdsys.cc>
 *
 * https://spdx.org/licenses
 */

#include <config.h>
#include <common.h>
#include <asm/arch/cpu.h>
#include <linux/mbus.h>
#include <dm.h>
#include <asm/io.h>
#include <errno.h>
#include <mvebu/fuse-mvebu.h>

#define MBUS_EFUSE_SIZE BIT(20)
#define EFUSE_ADDR_MASK	0xff000000

int mvebu_efuse_init_hw(struct udevice *dev)
{
	struct mvebu_fuse_block_data *priv = dev_get_priv(dev);
	int ret = 0;
	phys_addr_t efuse_base;

	efuse_base = (phys_addr_t)priv->target_otp_mem & EFUSE_ADDR_MASK;

	ret = mvebu_mbus_add_window_by_id(CPU_TARGET_SATA23_DFX,
					  0xA, efuse_base, MBUS_EFUSE_SIZE);

	return ret;
}
