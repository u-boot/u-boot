// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <config.h>

#include <fdt_support.h>
#include <image.h>
#include <lmb.h>
#include <malloc.h>

#include <asm/global_data.h>

#include <test/test.h>
#include <test/ut.h>

#define IMAGE_FDT_TEST(_name, _flags) UNIT_TEST(_name, _flags, image_fdt)

DECLARE_GLOBAL_DATA_PTR;

/**
 * test_boot_fdt_add_mem_rsv_regions - Make sure dt reservations are created and
 *                                     destroyed correctly
 * @uts: Test state
 *
 * This test depends on the UT_DM device tree and ensures the following
 * statements hold true: The default reservation in test.dtb exists.
 * Re-reserving that region will result in an error. Loading a new device tree
 * will remove old reservations.
 */
static int test_boot_fdt_add_mem_rsv_regions(struct unit_test_state *uts)
{
	phys_addr_t start = CFG_SYS_SDRAM_BASE + 0x100000;
	const void *old_blob = gd->fdt_blob;
	int ret = CMD_RET_FAILURE;
	ulong fdt_sz;
	int nodeoffset;
	void *new_blob;

	/* Default reservation should exist */
	ut_asserteq(1, lmb_is_reserved_flags(start, LMB_NOMAP));

	/* Attempting to re-reserve should warn the user */
	boot_fdt_add_mem_rsv_regions(gd->fdt_blob);
	ut_assert_nextlinen("ERROR: reserving");
	ut_assert_console_end();

	/* Loading a new_blob device tree should be allowed */
	fdt_sz = fdt_totalsize(gd->fdt_blob);
	new_blob = malloc(fdt_sz);
	ut_assertnonnull(new_blob);
	memcpy(new_blob, gd->fdt_blob, fdt_sz);

	nodeoffset = fdt_path_offset(new_blob, "/reserved-memory");
	if (nodeoffset < 0)
		goto free_blob;

	if (fdt_del_node(new_blob, nodeoffset))
		goto free_blob;

	boot_fdt_add_mem_rsv_regions(new_blob);
	gd->fdt_blob = new_blob;

	if (ut_check_console_end(uts)) {
		ut_failf(uts, __FILE__, __LINE__, __func__, "console",
			 "Expected no more output, got '%s'", uts->actual_str);
		goto switch_fdt;
	}

	/* Reservation should not exist now */
	if (!lmb_is_reserved_flags(start, LMB_NOMAP))
		ret = 0;

	/* Cleanup */
switch_fdt:
	boot_fdt_add_mem_rsv_regions(old_blob);
	gd->fdt_blob = old_blob;
free_blob:
	free(new_blob);
	return ret;
}
IMAGE_FDT_TEST(test_boot_fdt_add_mem_rsv_regions, UTF_CONSOLE);
