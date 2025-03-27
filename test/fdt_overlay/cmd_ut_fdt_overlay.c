// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 NextThing Co
 * Copyright (c) 2016 Free Electrons
 */

#include <command.h>
#include <errno.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <image.h>
#include <log.h>
#include <malloc.h>

#include <linux/sizes.h>

#include <test/fdt_overlay.h>
#include <test/ut.h>

/* 4k ought to be enough for anybody */
#define FDT_COPY_SIZE	(4 * SZ_1K)

extern u32 __dtb_test_fdt_base_begin;
extern u32 __dtbo_test_fdt_overlay_begin;
extern u32 __dtbo_test_fdt_overlay_stacked_begin;

static void *fdt;

static int fdt_overlay_init(struct unit_test_state *uts)
{
	void *fdt_base = &__dtb_test_fdt_base_begin;
	void *fdt_overlay = &__dtbo_test_fdt_overlay_begin;
	void *fdt_overlay_stacked = &__dtbo_test_fdt_overlay_stacked_begin;
	void *fdt_overlay_copy, *fdt_overlay_stacked_copy;

	ut_assertok(fdt_check_header(fdt_base));
	ut_assertok(fdt_check_header(fdt_overlay));

	fdt = malloc(FDT_COPY_SIZE);
	fdt_overlay_copy = malloc(FDT_COPY_SIZE);
	fdt_overlay_stacked_copy = malloc(FDT_COPY_SIZE);
	ut_assertnonnull(fdt);
	ut_assertnonnull(fdt_overlay_copy);
	ut_assertnonnull(fdt_overlay_stacked_copy);

	/*
	 * Resize the FDT to 4k so that we have room to operate on
	 *
	 * (and relocate it since the memory might be mapped
	 * read-only)
	 */
	ut_assertok(fdt_open_into(fdt_base, fdt, FDT_COPY_SIZE));

	/*
	 * Resize the overlay to 4k so that we have room to operate on
	 *
	 * (and relocate it since the memory might be mapped
	 * read-only)
	 */
	ut_assertok(fdt_open_into(fdt_overlay, fdt_overlay_copy,
				  FDT_COPY_SIZE));

	/*
	 * Resize the stacked overlay to 4k so that we have room to operate on
	 *
	 * (and relocate it since the memory might be mapped
	 * read-only)
	 */
	ut_assertok(fdt_open_into(fdt_overlay_stacked, fdt_overlay_stacked_copy,
				  FDT_COPY_SIZE));

	/* Apply the overlay */
	ut_assertok(fdt_overlay_apply(fdt, fdt_overlay_copy));

	/* Apply the stacked overlay */
	ut_assertok(fdt_overlay_apply(fdt, fdt_overlay_stacked_copy));

	free(fdt_overlay_stacked_copy);
	free(fdt_overlay_copy);

	return 0;
}
FDT_OVERLAY_TEST_INIT(fdt_overlay_init, 0);

static int fdt_getprop_str(void *fdt, const char *path, const char *name,
			   const char **out)
{
	int node_off;
	int len;

	node_off = fdt_path_offset(fdt, path);
	if (node_off < 0)
		return node_off;

	*out = fdt_stringlist_get(fdt, node_off, name, 0, &len);

	return len < 0 ? len : 0;
}

static int fdt_overlay_test_change_int_property(struct unit_test_state *uts)
{
	int off;

	off = fdt_path_offset(fdt, "/test-node");
	ut_assert(off >= 0);

	ut_asserteq(43, fdtdec_get_uint(fdt, off, "test-int-property", 0));

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_change_int_property, 0);

static int fdt_overlay_test_change_str_property(struct unit_test_state *uts)
{
	const char *val = NULL;

	ut_assertok(fdt_getprop_str(fdt, "/test-node", "test-str-property",
				    &val));
	ut_asserteq_str("foobar", val);

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_change_str_property, 0);

static int fdt_overlay_test_add_str_property(struct unit_test_state *uts)
{
	const char *val = NULL;

	ut_assertok(fdt_getprop_str(fdt, "/test-node", "test-str-property-2",
				    &val));
	ut_asserteq_str("foobar2", val);

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_add_str_property, 0);

static int fdt_overlay_test_add_node_by_phandle(struct unit_test_state *uts)
{
	int off;

	off = fdt_path_offset(fdt, "/test-node/new-node");
	ut_assert(off >= 0);

	ut_assertnonnull(fdt_getprop(fdt, off, "new-property", NULL));

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_add_node_by_phandle, 0);

static int fdt_overlay_test_add_node_by_path(struct unit_test_state *uts)
{
	int off;

	off = fdt_path_offset(fdt, "/new-node");
	ut_assert(off >= 0);

	ut_assertnonnull(fdt_getprop(fdt, off, "new-property", NULL));

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_add_node_by_path, 0);

static int fdt_overlay_test_add_subnode_property(struct unit_test_state *uts)
{
	int off;

	off = fdt_path_offset(fdt, "/test-node/sub-test-node");
	ut_assert(off >= 0);

	ut_assertnonnull(fdt_getprop(fdt, off, "sub-test-property", NULL));
	ut_assertnonnull(fdt_getprop(fdt, off, "new-sub-test-property", NULL));

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_add_subnode_property, 0);

static int fdt_overlay_test_local_phandle(struct unit_test_state *uts)
{
	uint32_t local_phandle;
	u32 val[2];
	int off;

	off = fdt_path_offset(fdt, "/new-local-node");
	ut_assert(off >= 0);

	local_phandle = fdt_get_phandle(fdt, off);
	ut_assert(local_phandle);

	ut_assertok(fdtdec_get_int_array(fdt, 0, "test-several-phandle", val,
					 ARRAY_SIZE(val)));
	ut_asserteq(local_phandle, val[0]);
	ut_asserteq(local_phandle, val[1]);

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_local_phandle, 0);

static int fdt_overlay_test_local_phandles(struct unit_test_state *uts)
{
	uint32_t local_phandle, test_phandle;
	u32 val[2];
	int off;

	off = fdt_path_offset(fdt, "/new-local-node");
	ut_assert(off >= 0);

	local_phandle = fdt_get_phandle(fdt, off);
	ut_assert(local_phandle);

	off = fdt_path_offset(fdt, "/test-node");
	ut_assert(off >= 0);

	test_phandle = fdt_get_phandle(fdt, off);
	ut_assert(test_phandle);

	ut_assertok(fdtdec_get_int_array(fdt, 0, "test-phandle", val,
					 ARRAY_SIZE(val)));
	ut_asserteq(test_phandle, val[0]);
	ut_asserteq(local_phandle, val[1]);

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_local_phandles, 0);

static int fdt_overlay_test_stacked(struct unit_test_state *uts)
{
	int off;

	off = fdt_path_offset(fdt, "/new-local-node");
	ut_assert(off > 0);

	ut_asserteq(43,
		    fdtdec_get_uint(fdt, off, "stacked-test-int-property", 0));

	return CMD_RET_SUCCESS;
}
FDT_OVERLAY_TEST(fdt_overlay_test_stacked, 0);
