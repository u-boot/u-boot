// SPDX-License-Identifier: GPL-2.0+
/*
 * Bounds-check tests for vbe_read_fit()
 *
 * vbe_read_fit() pulls a firmware-phase FIT from a trusted firmware area
 * on a block device. The external-data location and size carried in the
 * FIT image node are attacker-controllable when the firmware area is on
 * mutable boot media, so vbe_read_fit() must reject FITs whose external
 * data extends past @area_size before issuing the follow-up blk_read().
 *
 * These tests build small synthetic FITs with deliberately out-of-range
 * values and confirm vbe_read_fit() returns -E2BIG for each.
 *
 * Copyright 2026 Canonical Ltd.
 * Written by Aristo Chen <aristo.chen@canonical.com>
 */

#include <blk.h>
#include <dm.h>
#include <image.h>
#include <memalign.h>
#include <mmc.h>
#include <test/test.h>
#include <test/ut.h>
#include <linux/libfdt.h>
#include "bootstd_common.h"
#include "../../boot/vbe_common.h"

/*
 * The synthetic FIT is written to mmc1 starting at block TEST_FIT_BLK.
 * bootstd_setup_for_tests() uses blocks 4 and 6 (see bootstd_common.h);
 * block 16 leaves a comfortable gap.
 */
#define TEST_FIT_BLK	16
#define TEST_FIT_OFF	((ulong)TEST_FIT_BLK * MMC_MAX_BLOCK_LEN)
#define TEST_AREA_SIZE	0x1000

/**
 * build_fit() - Build a minimal external-data FIT for vbe_read_fit()
 *
 * The FIT advertises a single firmware image whose @data-position and
 * @data-size are passed in directly. Both values are attacker-controlled
 * in the real threat model.
 *
 * @buf:		Destination buffer (must be at least 512 bytes)
 * @buf_size:		Size of @buf
 * @data_position:	Value written to the image's data-position property
 * @data_size:		Value written to the image's data-size property
 * Returns: 0 on success, libfdt error otherwise
 */
static int build_fit(void *buf, size_t buf_size, u32 data_position,
		     u32 data_size)
{
	int ret;

	ret = fdt_create(buf, buf_size);
	if (ret)
		return ret;
	ret = fdt_finish_reservemap(buf);
	if (ret)
		return ret;

	ret = fdt_begin_node(buf, "");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_DESC_PROP, "vbe-read-fit test");
	if (ret)
		return ret;
	ret = fdt_property_u32(buf, FIT_TIMESTAMP_PROP, 0);
	if (ret)
		return ret;

	ret = fdt_begin_node(buf, "images");
	if (ret)
		return ret;
	ret = fdt_begin_node(buf, "u-boot");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_DESC_PROP, "U-Boot");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_TYPE_PROP, "firmware");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_ARCH_PROP, "sandbox");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_OS_PROP, "u-boot");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_PHASE_PROP, "u-boot");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_COMP_PROP, "none");
	if (ret)
		return ret;
	ret = fdt_property_u32(buf, FIT_DATA_POSITION_PROP, data_position);
	if (ret)
		return ret;
	ret = fdt_property_u32(buf, FIT_DATA_SIZE_PROP, data_size);
	if (ret)
		return ret;
	ret = fdt_end_node(buf);	/* u-boot */
	if (ret)
		return ret;
	ret = fdt_end_node(buf);	/* images */
	if (ret)
		return ret;

	ret = fdt_begin_node(buf, "configurations");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_DEFAULT_PROP, "conf-1");
	if (ret)
		return ret;
	ret = fdt_begin_node(buf, "conf-1");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, "compatible", "sandbox");
	if (ret)
		return ret;
	ret = fdt_property_string(buf, FIT_FIRMWARE_PROP, "u-boot");
	if (ret)
		return ret;
	ret = fdt_end_node(buf);	/* conf-1 */
	if (ret)
		return ret;
	ret = fdt_end_node(buf);	/* configurations */
	if (ret)
		return ret;

	ret = fdt_end_node(buf);	/* root */
	if (ret)
		return ret;

	return fdt_finish(buf);
}

/**
 * place_fit_on_mmc() - Write a synthetic FIT to mmc1 and return its blk dev
 *
 * @uts: Unit test state
 * @fit: FIT image to write
 * @blkp: On success, receives the block udevice for mmc1
 * Returns: 0 on success, -ve on error
 */
static int place_fit_on_mmc(struct unit_test_state *uts, const void *fit,
			    struct udevice **blkp)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, blkbuf, MMC_MAX_BLOCK_LEN);
	struct udevice *mmc;
	struct blk_desc *desc;
	size_t fit_size = fdt_totalsize(fit);
	size_t pos;
	int blknum = TEST_FIT_BLK;

	ut_assertok(uclass_get_device(UCLASS_MMC, 1, &mmc));
	desc = blk_get_by_device(mmc);
	if (!desc)
		return log_msg_ret("desc", -ENODEV);

	for (pos = 0; pos < fit_size; pos += MMC_MAX_BLOCK_LEN, blknum++) {
		size_t this_blk = min(fit_size - pos,
				      (size_t)MMC_MAX_BLOCK_LEN);

		memset(blkbuf, '\0', MMC_MAX_BLOCK_LEN);
		memcpy(blkbuf, (const u8 *)fit + pos, this_blk);
		if (blk_dwrite(desc, blknum, 1, blkbuf) != 1)
			return log_msg_ret("wr", -EIO);
	}
	*blkp = desc->bdev;

	return 0;
}

/*
 * data-position points past area_size: vbe_read_fit() must reject the
 * FIT with -E2BIG before issuing the external-data blk_read().
 */
static int vbe_read_fit_oob_position(struct unit_test_state *uts)
{
	u8 fit[1024] __aligned(8);
	struct udevice *blk;
	ulong load_addr = 0, len = 0;
	char *name = NULL;
	int ret;

	ut_assertok(build_fit(fit, sizeof(fit),
			      TEST_AREA_SIZE + 0x10, 0x40));
	ut_assertok(place_fit_on_mmc(uts, fit, &blk));

	ret = vbe_read_fit(blk, TEST_FIT_OFF, TEST_AREA_SIZE,
			   NULL, &load_addr, &len, &name);
	ut_asserteq(-E2BIG, ret);

	return 0;
}

BOOTSTD_TEST(vbe_read_fit_oob_position, UTF_DM | UTF_SCAN_FDT);

/*
 * data-position is inside the area but data-size pushes the end past
 * area_size: vbe_read_fit() must reject the FIT with -E2BIG.
 */
static int vbe_read_fit_oversize_data(struct unit_test_state *uts)
{
	u8 fit[1024] __aligned(8);
	struct udevice *blk;
	ulong load_addr = 0, len = 0;
	char *name = NULL;
	int ret;

	ut_assertok(build_fit(fit, sizeof(fit),
			      0x400, TEST_AREA_SIZE));
	ut_assertok(place_fit_on_mmc(uts, fit, &blk));

	ret = vbe_read_fit(blk, TEST_FIT_OFF, TEST_AREA_SIZE,
			   NULL, &load_addr, &len, &name);
	ut_asserteq(-E2BIG, ret);

	return 0;
}

BOOTSTD_TEST(vbe_read_fit_oversize_data, UTF_DM | UTF_SCAN_FDT);
