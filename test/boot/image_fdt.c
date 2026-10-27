// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <config.h>

#include <fdt_support.h>
#include <fdtdec.h>
#include <image.h>
#include <lmb.h>
#include <malloc.h>

#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>

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

/**
 * test_boot_fdt_rsv_above_ram_top - Make sure dt reservations above ram_top
 *                                   are handled quietly
 * @uts: Test state
 *
 * With LMB_LIMIT_DMA_BELOW_RAM_TOP, LMB reserves the memory above ram_top
 * itself. A reservation above ram_top must then be skipped without an error,
 * and a reservation that crosses ram_top must still reserve its part below
 * ram_top.
 */
static int test_boot_fdt_rsv_above_ram_top(struct unit_test_state *uts)
{
	phys_addr_t ram_top = gd->ram_base + gd->ram_size / 2;
	phys_addr_t old_ram_top = gd->ram_top;
	phys_size_t size = gd->ram_base + gd->ram_size - ram_top;
	struct fdt_memory above = {
		.start = ram_top + SZ_4K,
		.end = ram_top + 2 * SZ_4K - 1,
	};
	struct fdt_memory across = {
		.start = ram_top - SZ_4K,
		.end = ram_top + SZ_4K - 1,
	};
	phys_addr_t addr = ram_top;
	int nomap_start, nomap_end, nomap_above, noov_above;
	struct lmb store;
	void *blob;
	int ret;

	if (!IS_ENABLED(CONFIG_LMB_LIMIT_DMA_BELOW_RAM_TOP))
		return -EAGAIN;

	blob = malloc(SZ_4K);
	ut_assertnonnull(blob);
	ut_assertok(fdt_create_empty_tree(blob, SZ_4K));
	ut_assertok(fdtdec_add_reserved_memory(blob, "above", &above, NULL, 0,
					       NULL,
					       FDTDEC_RESERVED_MEMORY_NO_MAP));
	ut_assertok(fdtdec_add_reserved_memory(blob, "across", &across, NULL,
					       0, NULL,
					       FDTDEC_RESERVED_MEMORY_NO_MAP));

	/* Set up LMB as lmb_add_memory() does for a lower ram_top */
	ut_assertok(lmb_push(&store));
	ut_assertok(lmb_add(gd->ram_base, gd->ram_size));
	ut_assertok(lmb_alloc_mem(LMB_MEM_ALLOC_ADDR, 0, &addr, size,
				  LMB_NOOVERWRITE));
	gd->ram_top = ram_top;

	boot_fdt_add_mem_rsv_regions(blob);
	ret = ut_check_console_end(uts);
	nomap_start = lmb_is_reserved_flags(across.start, LMB_NOMAP);
	nomap_end = lmb_is_reserved_flags(ram_top - 1, LMB_NOMAP);
	nomap_above = lmb_is_reserved_flags(ram_top, LMB_NOMAP);
	noov_above = lmb_is_reserved_flags(ram_top, LMB_NOOVERWRITE);

	gd->ram_top = old_ram_top;
	lmb_pop(&store);
	free(blob);

	ut_assertok(ret);
	/* The part of the region below ram_top is reserved up to ram_top */
	ut_asserteq(1, nomap_start);
	ut_asserteq(1, nomap_end);
	/* The reservation of LMB above ram_top is left alone */
	ut_asserteq(0, nomap_above);
	ut_asserteq(1, noov_above);

	return 0;
}
IMAGE_FDT_TEST(test_boot_fdt_rsv_above_ram_top, UTF_CONSOLE);
