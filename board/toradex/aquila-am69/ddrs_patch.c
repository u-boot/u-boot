// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Toradex - https://www.toradex.com/
 */

#include <fdt_support.h>
#include <linux/errno.h>
#include <stdint.h>

#include "ddrs_patch.h"

static int ftd_apply_ddrss_prop_patch(void *fdt, int mem_offset, const char *prop_name,
				      const struct ddr_reg_patch *patch_set, int patch_num)
{
	int ret;
	int i;
	u32 val;

	for (i = 0; i < patch_num; i++, patch_set++) {
		val = cpu_to_be32(patch_set->val);
		ret = fdt_setprop_inplace_namelen_partial(fdt, mem_offset,
							  prop_name, strlen(prop_name),
							  patch_set->off * sizeof(u32),
							  &val, sizeof(u32));
		if (ret)
			return ret;
	}

	return 0;
}

static int fdt_apply_ddrss_patch(void *fdt, int msmc_offset, const char *ddrs_ctrl,
				 const struct ddrss_patch *ddrss_patch)
{
	int ret;
	int mem_offset;

	mem_offset = fdt_subnode_offset(fdt, msmc_offset, ddrs_ctrl);
	if (mem_offset < 0)
		return -ENODEV;

	ret = fdt_setprop_inplace_u32(fdt, mem_offset, "ti,ddr-fhs-cnt", ddrss_patch->ddr_fhs_cnt);
	if (ret)
		return ret;

	ret = ftd_apply_ddrss_prop_patch(fdt, mem_offset, "ti,ctl-data",
					 ddrss_patch->ctl_patch, ddrss_patch->ctl_patch_num);
	if (ret)
		return ret;

	ret = ftd_apply_ddrss_prop_patch(fdt, mem_offset, "ti,pi-data",
					 ddrss_patch->pi_patch, ddrss_patch->pi_patch_num);
	if (ret)
		return ret;

	ret = ftd_apply_ddrss_prop_patch(fdt, mem_offset, "ti,phy-data",
					 ddrss_patch->phy_patch, ddrss_patch->phy_patch_num);
	if (ret)
		return ret;

	return 0;
}

int aquila_am69_fdt_apply_ddr_patch(void *fdt, struct ddrss_patch *ddrss_patch[4], u32 intrlv_size)
{
	int ret;
	int msmc_offset;

	msmc_offset = fdt_path_offset(fdt, "/bus@100000/bus@30000000/msmc");
	if (msmc_offset < 0)
		return -ENODEV;

	ret = fdt_setprop_inplace_u32(fdt, msmc_offset, "intrlv-size", intrlv_size);
	if (ret)
		return ret;

	ret = fdt_apply_ddrss_patch(fdt, msmc_offset, "memorycontroller@2990000", ddrss_patch[0]);
	if (ret)
		return ret;

	ret = fdt_apply_ddrss_patch(fdt, msmc_offset, "memorycontroller@29b0000", ddrss_patch[1]);
	if (ret)
		return ret;

	ret = fdt_apply_ddrss_patch(fdt, msmc_offset, "memorycontroller@29d0000", ddrss_patch[2]);
	if (ret)
		return ret;

	ret = fdt_apply_ddrss_patch(fdt, msmc_offset, "memorycontroller@29f0000", ddrss_patch[3]);
	if (ret)
		return ret;

	return 0;
}
