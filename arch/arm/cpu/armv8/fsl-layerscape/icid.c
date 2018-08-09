// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>

static void set_icid(struct icid_id_table *tbl, int size)
{
	int i;

	for (i = 0; i < size; i++)
		out_be32((u32 *)(tbl[i].reg_addr), tbl[i].reg);
}

void set_icids(void)
{
	/* setup general icid offsets */
	set_icid(icid_tbl, icid_tbl_sz);
}

int fdt_set_iommu_prop(void *blob, int off, int smmu_ph, u32 *ids, int num_ids)
{
	int i, ret;
	u32 prop[8];

	/*
	 * Note: The "iommus" property definition mentions Stream IDs while
	 * this code handles ICIDs. The current implementation assumes that
	 * ICIDs and Stream IDs are equal.
	 */
	for (i = 0; i < num_ids; i++) {
		prop[i * 2] = cpu_to_fdt32(smmu_ph);
		prop[i * 2 + 1] = cpu_to_fdt32(ids[i]);
	}
	ret = fdt_setprop(blob, off, "iommus",
			  prop, sizeof(u32) * num_ids * 2);
	if (ret) {
		printf("WARNING unable to set iommus: %s\n", fdt_strerror(ret));
		return ret;
	}

	return 0;
}

int fdt_fixup_icid_tbl(void *blob, int smmu_ph,
		       struct icid_id_table *tbl, int size)
{
	int i, err, off;

	for (i = 0; i < size; i++) {
		if (!tbl[i].compat)
			continue;

		off = fdt_node_offset_by_compat_reg(blob,
						    tbl[i].compat,
						    tbl[i].compat_addr);
		if (off > 0) {
			err = fdt_set_iommu_prop(blob, off, smmu_ph,
						 &tbl[i].id, 1);
			if (err)
				return err;
		} else {
			printf("WARNING could not find node %s: %s.\n",
			       tbl[i].compat, fdt_strerror(off));
		}
	}

	return 0;
}

int fdt_get_smmu_phandle(void *blob)
{
	int noff, smmu_ph;

	noff = fdt_node_offset_by_compatible(blob, -1, "arm,mmu-500");
	if (noff < 0) {
		printf("WARNING failed to get smmu node: %s\n",
		       fdt_strerror(noff));
		return noff;
	}

	smmu_ph = fdt_get_phandle(blob, noff);
	if (!smmu_ph) {
		smmu_ph = fdt_create_phandle(blob, noff);
		if (!smmu_ph) {
			printf("WARNING failed to get smmu phandle\n");
			return -1;
		}
	}

	return smmu_ph;
}

void fdt_fixup_icid(void *blob)
{
	int smmu_ph;

	smmu_ph = fdt_get_smmu_phandle(blob);
	if (smmu_ph < 0)
		return;

	fdt_fixup_icid_tbl(blob, smmu_ph, icid_tbl, icid_tbl_sz);
}
