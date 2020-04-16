// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <dm.h>
#include <dm/of_extra.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_fdtdec_set_carveout(struct unit_test_state *uts)
{
	struct fdt_memory resv;
	void *blob;
	const fdt32_t *prop;
	int blob_sz, len, offset;

	blob_sz = fdt_totalsize(gd->fdt_blob) + 4096;
	blob = malloc(blob_sz);
	ut_assertnonnull(blob);

	/* Make a writtable copy of the fdt blob */
	ut_assertok(fdt_open_into(gd->fdt_blob, blob, blob_sz));

	resv.start = 0x1000;
	resv.end = 0x2000;
	ut_assertok(fdtdec_set_carveout(blob, "/a-test",
					"memory-region", 2, "test_resv1",
					&resv));

	resv.start = 0x10000;
	resv.end = 0x20000;
	ut_assertok(fdtdec_set_carveout(blob, "/a-test",
					"memory-region", 1, "test_resv2",
					&resv));

	resv.start = 0x100000;
	resv.end = 0x200000;
	ut_assertok(fdtdec_set_carveout(blob, "/a-test",
					"memory-region", 0, "test_resv3",
					&resv));

	offset = fdt_path_offset(blob, "/a-test");
	ut_assert(offset > 0);
	prop = fdt_getprop(blob, offset, "memory-region", &len);
	ut_assertnonnull(prop);

	ut_asserteq(len, 12);
	ut_assert(fdt_node_offset_by_phandle(blob, fdt32_to_cpu(prop[0])) > 0);
	ut_assert(fdt_node_offset_by_phandle(blob, fdt32_to_cpu(prop[1])) > 0);
	ut_assert(fdt_node_offset_by_phandle(blob, fdt32_to_cpu(prop[2])) > 0);

	free(blob);

	return 0;
}
DM_TEST(dm_test_fdtdec_set_carveout,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT | DM_TESTF_FLAT_TREE);
