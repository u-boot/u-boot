// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor
 */

#include <common.h>
#include <asm/io.h>
#include <fsl_csu.h>
#include <asm/arch/ns_access.h>
#include <asm/arch/fsl_serdes.h>

void set_devices_ns_access(unsigned long index, u16 val)
{
	u32 *base = (u32 *)CONFIG_SYS_FSL_CSU_ADDR;
	u32 *reg;
	uint32_t tmp;

	reg = base + index / 2;
	tmp = in_be32(reg);
	if (index % 2 == 0) {
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
		set_devices_ns_access(ns_dev[i].ind, ns_dev[i].val);
}

void enable_layerscape_ns_access(void)
{
#ifdef CONFIG_ARM64
	if (current_el() == 3)
#endif
		enable_devices_ns_access(ns_dev, ARRAY_SIZE(ns_dev));
}

void set_pcie_ns_access(int pcie, u16 val)
{
	switch (pcie) {
#ifdef CONFIG_PCIE1
	case PCIE1:
		set_devices_ns_access(CSU_CSLX_PCIE1, val);
		set_devices_ns_access(CSU_CSLX_PCIE1_IO, val);
		return;
#endif
#ifdef CONFIG_PCIE2
	case PCIE2:
		set_devices_ns_access(CSU_CSLX_PCIE2, val);
		set_devices_ns_access(CSU_CSLX_PCIE2_IO, val);
		return;
#endif
#ifdef CONFIG_PCIE3
	case PCIE3:
		set_devices_ns_access(CSU_CSLX_PCIE3, val);
		set_devices_ns_access(CSU_CSLX_PCIE3_IO, val);
		return;
#endif
	default:
		debug("The PCIE%d doesn't exist!\n", pcie);
		return;
	}
}
