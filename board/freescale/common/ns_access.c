/*
 * Copyright 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <fsl_csu.h>
#include <asm/arch/ns_access.h>

void set_devices_ns_access(struct csu_ns_dev *ns_dev, u16 val)
{
	u32 *base = (u32 *)CONFIG_SYS_FSL_CSU_ADDR;
	u32 *reg;
	uint32_t tmp;

	reg = base + ns_dev->ind / 2;
	tmp = in_be32(reg);
	if (ns_dev->ind % 2 == 0) {
		tmp &= 0x0000ffff;
		tmp |= val << 16;
	} else {
		tmp &= 0xffff0000;
		tmp |= val;
	}

	out_be32(reg, tmp);
}

static void enable_devices_ns_access(struct csu_ns_dev *ns_dev, uint32_t num)
{
	int i;

	for (i = 0; i < num; i++)
		set_devices_ns_access(ns_dev + i, ns_dev[i].val);
}

void enable_layerscape_ns_access(void)
{
	enable_devices_ns_access(ns_dev, ARRAY_SIZE(ns_dev));
}
