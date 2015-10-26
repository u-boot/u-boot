/*
 * Copyright 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <fsl_csu.h>
#include <asm/arch/ns_access.h>

static void enable_devices_ns_access(struct csu_ns_dev *ns_dev, uint32_t num)
{
	u32 *base = (u32 *)CONFIG_SYS_FSL_CSU_ADDR;
	u32 *reg;
	uint32_t val;
	int i;

	for (i = 0; i < num; i++) {
		reg = base + ns_dev[i].ind / 2;
		val = in_be32(reg);
		if (ns_dev[i].ind % 2 == 0) {
			val &= 0x0000ffff;
			val |= ns_dev[i].val << 16;
		} else {
			val &= 0xffff0000;
			val |= ns_dev[i].val;
		}
		out_be32(reg, val);
	}
}

void enable_layerscape_ns_access(void)
{
	enable_devices_ns_access(ns_dev, ARRAY_SIZE(ns_dev));
}
