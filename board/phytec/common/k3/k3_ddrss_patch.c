/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

#include "k3_ddrss_patch.h"

#include <fdt_support.h>
#include <linux/errno.h>

#ifdef CONFIG_K3_AM64_DDRSS
#define LPDDR4_INTR_CTL_REG_COUNT (423U)
#define LPDDR4_INTR_PHY_INDEP_REG_COUNT (345U)
#define LPDDR4_INTR_PHY_REG_COUNT (1406U)
#endif

static int fdt_setprop_inplace_idx_u32(void *fdt, int nodeoffset,
				       const char *name, uint32_t idx, u32 val)
{
	val = cpu_to_be32(val);
	return fdt_setprop_inplace_namelen_partial(fdt, nodeoffset, name,
						   strlen(name),
						   idx * sizeof(val), &val,
						   sizeof(val));
}

int fdt_apply_ddrss_timings_patch(void *fdt, struct ddrss *ddrss)
{
	int i, j;
	int ret;
	int mem_offset;

	mem_offset = fdt_path_offset(fdt, "/memorycontroller@f300000");
	if (mem_offset < 0)
		return -ENODEV;

	for (i = 0; i < LPDDR4_INTR_CTL_REG_COUNT; i++)
		for (j = 0; j < ddrss->ctl_regs_num; j++)
			if (i == ddrss->ctl_regs[j].off) {
				ret = fdt_setprop_inplace_idx_u32(fdt,
						mem_offset, "ti,ctl-data", i,
						ddrss->ctl_regs[j].val);
				if (ret)
					return ret;
			}

	for (i = 0; i < LPDDR4_INTR_PHY_INDEP_REG_COUNT; i++)
		for (j = 0; j < ddrss->pi_regs_num; j++)
			if (i == ddrss->pi_regs[j].off) {
				ret = fdt_setprop_inplace_idx_u32(fdt,
						mem_offset, "ti,pi-data", i,
						ddrss->pi_regs[j].val);
				if (ret)
					return ret;
			}

	for (i = 0; i < LPDDR4_INTR_PHY_REG_COUNT; i++)
		for (j = 0; j < ddrss->phy_regs_num; j++)
			if (i == ddrss->phy_regs[j].off) {
				ret = fdt_setprop_inplace_idx_u32(fdt,
						mem_offset, "ti,phy-data", i,
						ddrss->phy_regs[j].val);
				if (ret)
					return ret;
			}

	return 0;
}
