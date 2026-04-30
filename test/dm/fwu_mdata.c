// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 * Copyright (c) 2022, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <blk.h>
#include <dm.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <part.h>

#include <dm/test.h>
#include <test/ut.h>

#include "fwu_mdata_disk_image.h"

/* Block size of compressed disk image */
#define COMPRESSED_DISK_IMAGE_BLOCK_SIZE 8

static struct udevice *mmc_dev;
static struct blk_desc *dev_desc;

/* One 8 byte block of the compressed disk image */
struct line {
	size_t addr;
	char *line;
};

/* Compressed disk image */
struct compressed_disk_image {
	size_t length;
	struct line lines[];
};

static const struct compressed_disk_image img = FWU_MDATA_DISK_IMG;

/* Decompressed disk image */
static u8 *image;

static int setup_blk_device(struct unit_test_state *uts)
{
	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &mmc_dev));
	ut_assertok(blk_get_device_by_str("mmc", "0", &dev_desc));

	return 0;
}

static int populate_mmc_disk_image(struct unit_test_state *uts)
{
	u8 *buf;
	size_t i;
	size_t addr;
	size_t len;

	buf = malloc(img.length);
	if (!buf)
		return -ENOMEM;

	memset(buf, 0, img.length);

	for (i = 0; ; i++) {
		if (!img.lines[i].line)
			break;
		addr = img.lines[i].addr;
		len = COMPRESSED_DISK_IMAGE_BLOCK_SIZE;
		if (addr + len > img.length)
			len = img.length - addr;
		memcpy(buf + addr, img.lines[i].line, len);
	}
	image = buf;

	return 0;
}

static int write_mmc_blk_device(struct unit_test_state *uts)
{
	lbaint_t blkcnt;

	blkcnt = BLOCK_CNT(img.length, dev_desc);

	ut_asserteq(blkcnt, blk_dwrite(dev_desc, 0, blkcnt, image));

	return 0;
}

static int dm_test_fwu_mdata_read(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };

	ut_assertok(setup_blk_device(uts));
	ut_assertok(populate_mmc_disk_image(uts));
	ut_assertok(write_mmc_blk_device(uts));

	/*
	 * Trigger lib/fwu_updates/fwu.c fwu_boottime_checks()
	 * to populate g_dev global pointer in that library.
	 */
	ut_assertok(event_notify_null(EVT_POST_PREBOOT));

	ut_assertok(uclass_first_device_err(UCLASS_FWU_MDATA, &dev));
	ut_assertok(fwu_init());

	ut_assertok(fwu_get_mdata(&mdata));

	ut_asserteq(mdata.version, 0x1);

	return 0;
}
DM_TEST(dm_test_fwu_mdata_read, UTF_SCAN_FDT);

static int dm_test_fwu_mdata_write(struct unit_test_state *uts)
{
	u32 active_idx;
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };

	ut_assertok(setup_blk_device(uts));
	ut_assertok(populate_mmc_disk_image(uts));
	ut_assertok(write_mmc_blk_device(uts));

	/*
	 * Trigger lib/fwu_updates/fwu.c fwu_boottime_checks()
	 * to populate g_dev global pointer in that library.
	 */
	ut_assertok(event_notify_null(EVT_POST_PREBOOT));

	ut_assertok(uclass_first_device_err(UCLASS_FWU_MDATA, &dev));

	ut_assertok(fwu_init());
	ut_assertok(fwu_get_mdata(&mdata));

	active_idx = (mdata.active_index + 1) % CONFIG_FWU_NUM_BANKS;
	ut_assertok(fwu_set_active_index(active_idx));

	ut_assertok(fwu_get_mdata(&mdata));
	ut_asserteq(mdata.active_index, active_idx);

	return 0;
}
DM_TEST(dm_test_fwu_mdata_write, UTF_SCAN_FDT);

static int dm_test_fwu_mdata_get_image_guid(struct unit_test_state *uts)
{
	efi_guid_t image_type_guid =
		EFI_GUID(0x09d7cf52, 0x0720, 0x4710, \
			 0x91, 0xd1, 0x08, 0x46, 0x9b, 0x7f, 0xe9, 0xc8);
	efi_guid_t bank_0_image_guid =
		EFI_GUID(0x10057a86, 0xdaf1, 0x4f93, \
			 0xba, 0x7f, 0xb1, 0x95, 0xf7, 0xfa, 0x41, 0x70);
	efi_guid_t bank_1_image_guid =
		EFI_GUID(0xdb62ed3e, 0x6237, 0x4fb4, \
			 0x80, 0xc4, 0x1b, 0x74, 0xd8, 0x46, 0xa8, 0xe7);
	efi_guid_t wrong_image_type_guid =
		EFI_GUID(0x12345678, 0x1302, 0x133f, \
			 0x18, 0x0a, 0x14, 0x05, 0x18, 0x05, 0x14, 0x0b);
	struct udevice *dev;
	efi_guid_t image_guid;

	ut_assertok(setup_blk_device(uts));
	ut_assertok(populate_mmc_disk_image(uts));
	ut_assertok(write_mmc_blk_device(uts));

	/*
	 * Trigger lib/fwu_updates/fwu.c fwu_boottime_checks()
	 * to populate g_dev global pointer in that library.
	 */
	ut_assertok(event_notify_null(EVT_POST_PREBOOT));

	ut_assertok(uclass_first_device_err(UCLASS_FWU_MDATA, &dev));

	ut_assertok(fwu_init());

	ut_assertok(fwu_mdata_get_image_guid(&image_guid, &image_type_guid, 0));
	ut_assertok(guidcmp(&image_guid, &bank_0_image_guid));

	ut_assertok(fwu_mdata_get_image_guid(&image_guid, &image_type_guid, 1));
	ut_assertok(guidcmp(&image_guid, &bank_1_image_guid));

	ut_asserteq(-EINVAL, fwu_mdata_get_image_guid(&image_guid,
						      &image_type_guid, 2));

	ut_asserteq(-ENOENT, fwu_mdata_get_image_guid(&image_guid,
						      &wrong_image_type_guid,
						      0));

	return 0;
}
DM_TEST(dm_test_fwu_mdata_get_image_guid, UTF_SCAN_FDT);
