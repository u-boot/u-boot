/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2025 Toradex - https://www.toradex.com/
 */

#ifndef __AQUILA_AM69_DDRS_PATCH_H
#define __AQUILA_AM69_DDRS_PATCH_H

struct ddr_reg_patch {
	u32 off;
	u32 val;
};

struct ddrss_patch {
	u32 ddr_fhs_cnt;
	struct ddr_reg_patch *ctl_patch;
	u32 ctl_patch_num;
	struct ddr_reg_patch *pi_patch;
	u32 pi_patch_num;
	struct ddr_reg_patch *phy_patch;
	u32 phy_patch_num;
};

/**
 * aquila_am69_fdt_apply_ddr_patch - Apply DDRSS patches to the R5 U-Boot device tree
 * @fdt:         Pointer to the flattened device tree (FDT) blob in memory
 * @ddrss_patch: Array of pointers to DDRSS patch structures (one per controller)
 * @intrlv_size: Value to set for the "intrlv-size" property
 *
 * This function updates the MSMC node in the device tree by setting the
 * "intrlv-size" property and applying DDRSS patches to each of the four
 * memory controller nodes.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int aquila_am69_fdt_apply_ddr_patch(void *fdt, struct ddrss_patch *ddrss_patch[4], u32 intrlv_size);

#endif // __AQUILA_AM69_DDRS_PATCH_H
