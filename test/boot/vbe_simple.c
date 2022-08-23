// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for vbe-simple bootmeth. All start with 'vbe_simple'
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootmeth.h>
#include <dm.h>
#include <image.h>
#include <memalign.h>
#include <mmc.h>
#include <of_live.h>
#include <vbe.h>
#include <version_string.h>
#include <linux/log2.h>
#include <test/suites.h>
#include <test/ut.h>
#include <u-boot/crc.h>
#include "bootstd_common.h"

#define NVDATA_START_BLK	((0x400 + 0x400) / MMC_MAX_BLOCK_LEN)
#define VERSION_START_BLK	((0x400 + 0x800) / MMC_MAX_BLOCK_LEN)
#define TEST_VERSION		"U-Boot v2022.04-local2"
#define TEST_VERNUM		0x00010002

/* Basic test of reading nvdata and updating a fwupd node in the device tree */
static int vbe_simple_test_base(struct unit_test_state *uts)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, buf, MMC_MAX_BLOCK_LEN);
	const char *version, *bl_version;
	struct event_ft_fixup fixup;
	struct udevice *dev, *mmc;
	struct device_node *np;
	struct blk_desc *desc;
	char fdt_buf[0x400];
	char info[100];
	int node_ofs;
	ofnode node;
	u32 vernum;

	/* Set up the version string */
	ut_assertok(uclass_get_device(UCLASS_MMC, 1, &mmc));
	desc = blk_get_by_device(mmc);
	ut_assertnonnull(desc);

	memset(buf, '\0', MMC_MAX_BLOCK_LEN);
	strcpy(buf, TEST_VERSION);
	if (blk_dwrite(desc, VERSION_START_BLK, 1, buf) != 1)
		return log_msg_ret("write", -EIO);

	/* Set up the nvdata */
	memset(buf, '\0', MMC_MAX_BLOCK_LEN);
	buf[1] = ilog2(0x40) << 4 | 1;
	*(u32 *)(buf + 4) = TEST_VERNUM;
	buf[0] = crc8(0, buf + 1, 0x3f);
	if (blk_dwrite(desc, NVDATA_START_BLK, 1, buf) != 1)
		return log_msg_ret("write", -EIO);

	/* Read the version back */
	ut_assertok(vbe_find_by_any("firmware0", &dev));
	ut_assertok(bootmeth_get_state_desc(dev, info, sizeof(info)));
	ut_asserteq_str("Version: " TEST_VERSION "\nVernum: 1/2", info);

	ut_assertok(fdt_create_empty_tree(fdt_buf, sizeof(fdt_buf)));
	node_ofs = fdt_add_subnode(fdt_buf, 0, "chosen");
	ut_assert(node_ofs > 0);

	node_ofs = fdt_add_subnode(fdt_buf, node_ofs, "fwupd");
	ut_assert(node_ofs > 0);

	node_ofs = fdt_add_subnode(fdt_buf, node_ofs, "firmware0");
	ut_assert(node_ofs > 0);

	/*
	 * This can only work on the live tree, since the ofnode interface for
	 * flat tree assumes that ofnode points to the control FDT
	 */
	ut_assertok(unflatten_device_tree(fdt_buf, &np));

	/*
	 * It would be better to call image_setup_libfdt() here, but that
	 * function does not allow passing an ofnode. We can pass fdt_buf but
	 * when it comes to send the evenr, it creates an ofnode that uses the
	 * control FDT, since it has no way of accessing the live tree created
	 * here.
	 *
	 * Two fix this we need:
	 * - image_setup_libfdt() is updated to use ofnode
	 * - ofnode updated to support access to an FDT other than the control
	 *   FDT. This is partially implemented with live tree, but not with
	 *   flat tree
	 */
	fixup.tree.np = np;
	ut_assertok(event_notify(EVT_FT_FIXUP, &fixup, sizeof(fixup)));

	node = ofnode_path_root(fixup.tree, "/chosen/fwupd/firmware0");

	version = ofnode_read_string(node, "cur-version");
	ut_assertnonnull(version);
	ut_asserteq_str(TEST_VERSION, version);

	ut_assertok(ofnode_read_u32(node, "cur-vernum", &vernum));
	ut_asserteq(TEST_VERNUM, vernum);

	bl_version = ofnode_read_string(node, "bootloader-version");
	ut_assertnonnull(bl_version);
	ut_asserteq_str(version_string, bl_version);

	return 0;
}
BOOTSTD_TEST(vbe_simple_test_base, UT_TESTF_DM | UT_TESTF_SCAN_FDT |
	     UT_TESTF_LIVE_TREE);
