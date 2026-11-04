// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for MTD virtual concatenation
 *
 * Copyright (C) 2026 Advanced Micro Devices, Inc.
 */

#include <dm.h>
#include <env.h>
#include <malloc.h>
#include <mtd.h>
#include <nand.h>
#include <os.h>
#include <rand.h>
#include <spi_flash.h>

#include <dm/device-internal.h>
#include <dm/test.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>
#include <test/test.h>
#include <test/ut.h>

/*
 * In test.dts, "nor-spare" (spi.bin@0, 0x100000-0x200000) is concatenated
 * with "nor1-spare" (spi.bin@1, 0x0-0x100000), and "nor1-b1" with "nor1-b2"
 * (spi.bin@1, 0x100000-0x180000 and 0x180000-0x200000). All the flashes use
 * spi.bin as their backing file, so the second group overlaps the first, but
 * only the first is used to access data.
 */
#define CONCAT_NAME	"nor-spare-nor1-spare-concat"
#define CONCAT2_NAME	"nor1-b1-nor1-b2-concat"
#define FLASH_SIZE	SZ_2M
#define PART_SIZE	SZ_1M
#define ERASE_SIZE	SZ_64K
#define BUF_SIZE	SZ_8K

/* Scanning again must not use more memory than this, in bytes, in total */
#define LEAK_LIMIT	128

/* Number of times to scan again, to see that nothing is lost each time */
#define RESCANS		8

/* Set in a concat device, to tell it from a new one in the same place */
#define MARK		0x1234

#define MTDIDS0		"nor0=nor0"
#define MTDIDS1		"nor1=nor1"
#define MTDPARTS_NOR0	"mtdparts=nor0:1m(nor0-mtdparts)"

/* Create the backing file and add the MTD devices and DT partitions */
static int setup_mtd(struct unit_test_state *uts)
{
	void *buf;

	buf = calloc(1, FLASH_SIZE);
	ut_assertnonnull(buf);
	ut_assertok(os_write_file("spi.bin", buf, FLASH_SIZE));
	free(buf);

	/*
	 * mtd_probe_devices() only scans the partitions again when mtdids or
	 * mtdparts has changed
	 */
	ut_assertok(env_set("mtdids", MTDIDS0));
	ut_assertok(mtd_probe_devices());

	return 0;
}

/*
 * Change mtdids, so that the next mtd_probe_devices() scans the partitions
 * again. Both values have the same length, so that the copy which
 * mtd_probe_devices() keeps does not change the amount of free memory.
 */
static int change_mtdids(struct unit_test_state *uts)
{
	const char *mtdids = env_get("mtdids");

	ut_assertnonnull(mtdids);
	if (!strcmp(mtdids, MTDIDS0))
		ut_assertok(env_set("mtdids", MTDIDS1));
	else
		ut_assertok(env_set("mtdids", MTDIDS0));

	return 0;
}

/* Undo setup_mtd() */
static int cleanup_mtd(struct unit_test_state *uts)
{
	ut_assertok(env_set("mtdparts", NULL));
	ut_assertok(env_set("mtdids", NULL));

	/*
	 * Scan once more, so that mtd_probe_devices() forgets the old values
	 * and later callers do not scan the partitions again
	 */
	ut_assertok(mtd_probe_devices());

	return 0;
}

/* Check whether an MTD device is registered */
static int check_mtd(struct unit_test_state *uts, const char *name,
		     bool expect)
{
	struct mtd_info *mtd;

	mtd = get_mtd_device_nm(name);
	if (!expect) {
		ut_asserteq_ptr(ERR_PTR(-ENODEV), mtd);
		return 0;
	}
	ut_assertok_ptr(mtd);
	put_mtd_device(mtd);

	return 0;
}

/* Get the MTD device of a SPI flash */
static struct mtd_info *flash_mtd(struct udevice *dev)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return &flash->mtd;
}

/*
 * These tests need the live tree, since the sandbox NAND devices, which are
 * registered at start-up and also scanned for partitions, still refer to the
 * live tree.
 *
 * A device which a test holds must be let go before the test ends, even if a
 * check fails, since it must not be in use when the driver model is removed.
 * So such a test makes its checks in a function, and lets go afterwards.
 */

/* Test that the partitions are joined and can be accessed */
static int dm_test_mtd_virt_concat(struct unit_test_state *uts)
{
	struct udevice *dev0, *dev1;
	struct erase_info erase;
	struct mtd_info *mtd;
	u8 *wbuf, *rbuf;
	size_t retlen;
	loff_t offset;
	int i;

	ut_assertok(setup_mtd(uts));

	mtd = get_mtd_device_nm(CONCAT_NAME);
	ut_assertok_ptr(mtd);
	ut_asserteq(MTD_NORFLASH, mtd->type);
	ut_asserteq(2 * PART_SIZE, mtd->size);
	ut_asserteq(ERASE_SIZE, mtd->erasesize);

	/* The partitions in the groups are not registered on their own... */
	ut_assertok(check_mtd(uts, "nor-spare", false));
	ut_assertok(check_mtd(uts, "nor1-spare", false));
	ut_assertok(check_mtd(uts, "nor1-b1", false));
	ut_assertok(check_mtd(uts, "nor1-b2", false));
	ut_assertok(check_mtd(uts, CONCAT2_NAME, true));

	/* ...but other partitions are */
	ut_assertok(check_mtd(uts, "nor-fit", true));

	wbuf = malloc(BUF_SIZE);
	ut_assertnonnull(wbuf);
	rbuf = malloc(BUF_SIZE);
	ut_assertnonnull(rbuf);
	for (i = 0; i < BUF_SIZE; i++)
		wbuf[i] = i * 7 + 3;

	/* Erase, write and read back across the boundary between flashes */
	offset = PART_SIZE - BUF_SIZE / 2;
	memset(&erase, '\0', sizeof(erase));
	erase.mtd = mtd;
	erase.addr = PART_SIZE - ERASE_SIZE;
	erase.len = 2 * ERASE_SIZE;
	ut_assertok(mtd_erase(mtd, &erase));
	ut_assertok(mtd_write(mtd, offset, BUF_SIZE, &retlen, wbuf));
	ut_asserteq(BUF_SIZE, retlen);
	ut_assertok(mtd_read(mtd, offset, BUF_SIZE, &retlen, rbuf));
	ut_asserteq(BUF_SIZE, retlen);
	ut_asserteq_mem(wbuf, rbuf, BUF_SIZE);

	/* Each half must have gone to the right place on its own flash */
	ut_assertok(uclass_get_device_by_name(UCLASS_SPI_FLASH, "spi.bin@0",
					      &dev0));
	ut_assertok(uclass_get_device_by_name(UCLASS_SPI_FLASH, "spi.bin@1",
					      &dev1));
	ut_assertok(spi_flash_read_dm(dev0, FLASH_SIZE - BUF_SIZE / 2,
				      BUF_SIZE / 2, rbuf));
	ut_asserteq_mem(wbuf, rbuf, BUF_SIZE / 2);
	ut_assertok(spi_flash_read_dm(dev1, 0, BUF_SIZE / 2, rbuf));
	ut_asserteq_mem(wbuf + BUF_SIZE / 2, rbuf, BUF_SIZE / 2);

	free(rbuf);
	free(wbuf);
	put_mtd_device(mtd);
	ut_assertok(cleanup_mtd(uts));

	return 0;
}
DM_TEST(dm_test_mtd_virt_concat, UTF_SCAN_FDT | UTF_LIVE_TREE);

/* Test that scanning the partitions again keeps the concatenated devices */
static int dm_test_mtd_virt_concat_rescan(struct unit_test_state *uts)
{
	struct mtd_info *mtd;
	ulong mem;
	int i;

	ut_assertok(setup_mtd(uts));

	/*
	 * Mark the device, so that a new one can be told apart from it, even
	 * if it was allocated at the same address
	 */
	mtd = get_mtd_device_nm(CONCAT_NAME);
	ut_assertok_ptr(mtd);
	mtd->ecc_stats.corrected = MARK;
	put_mtd_device(mtd);

	/*
	 * Scan several times. Each scan creates and frees the other
	 * partitions, but must not leave anything behind, so check the total
	 * rather than the exact amount of free memory, which depends on how
	 * the heap happens to be laid out.
	 */
	ut_assertok(change_mtdids(uts));
	ut_assertok(mtd_probe_devices());
	mem = ut_check_free();
	for (i = 0; i < RESCANS; i++) {
		ut_assertok(change_mtdids(uts));
		ut_assertok(mtd_probe_devices());
	}
	ut_assert(ut_check_delta(mem) < LEAK_LIMIT);

	mtd = get_mtd_device_nm(CONCAT_NAME);
	ut_assertok_ptr(mtd);
	ut_asserteq(MARK, mtd->ecc_stats.corrected);
	put_mtd_device(mtd);
	ut_assertok(check_mtd(uts, "nor-spare", false));
	ut_assertok(check_mtd(uts, "nor1-spare", false));
	ut_assertok(check_mtd(uts, "nor-fit", true));

	ut_assertok(cleanup_mtd(uts));

	return 0;
}
DM_TEST(dm_test_mtd_virt_concat_rescan, UTF_SCAN_FDT | UTF_LIVE_TREE);

/* Check that a concat device survives a scan while another partition is in use */
static int check_busy(struct unit_test_state *uts, struct mtd_info *concat)
{
	struct mtd_info *mtd;

	ut_assertok(change_mtdids(uts));
	ut_assertok(mtd_probe_devices());
	mtd = get_mtd_device_nm(CONCAT_NAME);
	ut_asserteq_ptr(concat, mtd);
	put_mtd_device(mtd);
	ut_assertok(check_mtd(uts, "nor-spare", false));
	ut_assertok(check_mtd(uts, "nor1-spare", false));
	ut_assertok(check_mtd(uts, "nor-fit", true));

	return 0;
}

/* Check that mtdparts leaves a flash alone while its concat device is in use */
static int check_busy_mtdparts(struct unit_test_state *uts,
			       struct mtd_info *concat)
{
	struct mtd_info *mtd;

	ut_assertok(env_set("mtdparts", MTDPARTS_NOR0));
	ut_assertok(mtd_probe_devices());
	mtd = get_mtd_device_nm(CONCAT_NAME);
	ut_asserteq_ptr(concat, mtd);
	put_mtd_device(mtd);
	ut_assertok(check_mtd(uts, "nor0-mtdparts", false));
	ut_assertok(check_mtd(uts, "nor-fit", true));

	return 0;
}

/* Test a concat device which is in use, and a partition which is in use */
static int dm_test_mtd_virt_concat_busy(struct unit_test_state *uts)
{
	struct mtd_info *concat, *fit;
	int ret;

	ut_assertok(setup_mtd(uts));

	concat = get_mtd_device_nm(CONCAT_NAME);
	ut_assertok_ptr(concat);
	fit = get_mtd_device_nm("nor-fit");
	if (IS_ERR(fit)) {
		put_mtd_device(concat);
		ut_assertok_ptr(fit);
	}

	/* A partition of the same flash in use does not affect the concat */
	ret = check_busy(uts, concat);
	put_mtd_device(fit);

	/* With the concat in use, the flash keeps its device tree partitions */
	if (!ret)
		ret = check_busy_mtdparts(uts, concat);
	put_mtd_device(concat);
	ut_assertok(ret);

	/*
	 * Once the concat device is free, mtdparts is used, and it replaces
	 * the concat device. The partition left over from the group is then
	 * registered on its own.
	 */
	ut_assertok(mtd_probe_devices());
	ut_assertok(check_mtd(uts, CONCAT_NAME, false));
	ut_assertok(check_mtd(uts, "nor0-mtdparts", true));
	ut_assertok(check_mtd(uts, "nor1-spare", true));

	/* Without mtdparts, the group is joined again */
	ut_assertok(cleanup_mtd(uts));
	ut_assertok(check_mtd(uts, CONCAT_NAME, true));

	return 0;
}
DM_TEST(dm_test_mtd_virt_concat_busy, UTF_SCAN_FDT | UTF_LIVE_TREE);

/* Check that the concat devices of a flash go, except one which is in use */
static int check_destroy(struct unit_test_state *uts, struct udevice *dev)
{
	ut_asserteq(-EBUSY, mtd_virt_concat_destroy(flash_mtd(dev)));
	ut_assertok(check_mtd(uts, CONCAT_NAME, true));
	ut_assertok(check_mtd(uts, CONCAT2_NAME, false));

	return 0;
}

/* Test destroying the concat devices of a flash, one of which is in use */
static int dm_test_mtd_virt_concat_destroy(struct unit_test_state *uts)
{
	struct mtd_info *concat;
	struct udevice *dev;
	int ret;

	ut_assertok(setup_mtd(uts));
	ut_assertok(uclass_get_device_by_name(UCLASS_SPI_FLASH, "spi.bin@1",
					      &dev));

	concat = get_mtd_device_nm(CONCAT_NAME);
	ut_assertok_ptr(concat);
	ret = check_destroy(uts, dev);
	put_mtd_device(concat);
	ut_assertok(ret);

	/* The one which was destroyed is created again by the next scan */
	ut_assertok(mtd_probe_devices());
	ut_assertok(check_mtd(uts, CONCAT_NAME, true));
	ut_assertok(check_mtd(uts, CONCAT2_NAME, true));

	ut_assertok(cleanup_mtd(uts));

	return 0;
}
DM_TEST(dm_test_mtd_virt_concat_destroy, UTF_SCAN_FDT | UTF_LIVE_TREE);

/* Test removing a flash that concatenated devices use */
static int dm_test_mtd_virt_concat_remove(struct unit_test_state *uts)
{
	struct mtd_info *mtd;
	struct udevice *dev;
	int idx;

	ut_assertok(setup_mtd(uts));

	/* Both concat devices are above the flash, so find the lowest */
	mtd = get_mtd_device_nm(CONCAT_NAME);
	ut_assertok_ptr(mtd);
	idx = mtd->index;
	put_mtd_device(mtd);
	mtd = get_mtd_device_nm(CONCAT2_NAME);
	ut_assertok_ptr(mtd);
	idx = min(idx, mtd->index);
	put_mtd_device(mtd);

	/*
	 * Removing a flash destroys the concat devices. Check their indexes
	 * rather than their names, since the list of MTD devices stops at the
	 * first unused index, which the flash has just freed.
	 */
	ut_assertok(uclass_get_device_by_name(UCLASS_SPI_FLASH, "spi.bin@1",
					      &dev));
	ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));
	ut_assertnull(__mtd_next_device(idx));

	/* and the next scan, which probes the flash again, joins them again */
	ut_assertok(mtd_probe_devices());
	ut_assertok(check_mtd(uts, CONCAT_NAME, true));
	ut_assertok(check_mtd(uts, CONCAT2_NAME, true));

	ut_assertok(cleanup_mtd(uts));

	return 0;
}
DM_TEST(dm_test_mtd_virt_concat_remove, UTF_SCAN_FDT | UTF_LIVE_TREE);

/* Test that the partitions of an incomplete group are registered */
static int dm_test_mtd_virt_concat_incomplete(struct unit_test_state *uts)
{
	ut_assertok(setup_mtd(uts));

	/* Use mtdparts for nor0, so that its DT partitions are not added */
	ut_assertok(env_set("mtdparts", MTDPARTS_NOR0));
	ut_assertok(mtd_probe_devices());

	ut_assertok(check_mtd(uts, CONCAT_NAME, false));
	ut_assertok(check_mtd(uts, "nor0-mtdparts", true));
	ut_assertok(check_mtd(uts, "nor1-spare", true));

	ut_assertok(cleanup_mtd(uts));

	return 0;
}
DM_TEST(dm_test_mtd_virt_concat_incomplete, UTF_SCAN_FDT | UTF_LIVE_TREE);

/* Test that a partition which is out of reach is not concatenated */
static int dm_test_mtd_virt_concat_bad_part(struct unit_test_state *uts)
{
	struct udevice *dev;
	__be32 reg[2];
	ofnode node;

	ut_assertok(setup_mtd(uts));
	ut_assertok(uclass_get_device_by_name(UCLASS_SPI_FLASH, "spi.bin@1",
					      &dev));
	node = ofnode_path("/spi@0/spi.bin@1/partitions/partition@0");
	ut_assert(ofnode_valid(node));

	/* Move "nor1-spare" beyond the end of the flash, and form the group */
	reg[0] = cpu_to_be32(SZ_2M + SZ_1M);
	reg[1] = cpu_to_be32(PART_SIZE);
	ut_assertok(ofnode_write_prop(node, "reg", reg, sizeof(reg), true));
	ut_assertok(mtd_virt_concat_destroy(flash_mtd(dev)));
	ut_assertok(mtd_probe_devices());

	/* Its group is left incomplete, so "nor-spare" is registered alone */
	ut_assertok(check_mtd(uts, CONCAT_NAME, false));
	ut_assertok(check_mtd(uts, "nor-spare", true));

	/* Put it back, and the group is joined again */
	reg[0] = cpu_to_be32(0);
	ut_assertok(ofnode_write_prop(node, "reg", reg, sizeof(reg), true));
	ut_assertok(change_mtdids(uts));
	ut_assertok(mtd_probe_devices());
	ut_assertok(check_mtd(uts, CONCAT_NAME, true));

	ut_assertok(cleanup_mtd(uts));

	return 0;
}
DM_TEST(dm_test_mtd_virt_concat_bad_part, UTF_SCAN_FDT | UTF_LIVE_TREE);

/* Test that a page can be written to a concat device made of NAND devices */
static int dm_test_mtd_concat_write_oob(struct unit_test_state *uts)
{
	struct mtd_info *subdev[2], *concat;
	struct mtd_oob_ops ops = {};
	struct erase_info erase;
	struct udevice *ctrl;
	size_t retlen, size;
	u8 *wbuf, *rbuf;
	int i, ret;

	/* Seed RNG for bit errors, so that the test does the same each time */
	srand(~0);

	/* Probe the controller, which registers the NAND devices */
	ut_assertok(uclass_get_device_by_driver(UCLASS_MTD,
						DM_DRIVER_GET(sand_nand),
						&ctrl));
	subdev[0] = get_nand_dev_by_index(0);
	subdev[1] = get_nand_dev_by_index(2);
	ut_assertnonnull(subdev[0]);
	ut_assertnonnull(subdev[1]);
	ut_asserteq(subdev[0]->writesize, subdev[1]->writesize);
	size = subdev[0]->writesize;

	wbuf = malloc(size);
	ut_assertnonnull(wbuf);
	rbuf = malloc(size);
	ut_assertnonnull(rbuf);
	for (i = 0; i < size; i++)
		wbuf[i] = i * 5 + 1;

	/* Erase the first block of each, so that we can see what is written */
	for (i = 0; i < 2; i++) {
		/*
		 * The first check for a bad block scans for them, and leaves
		 * the chip deselected, which makes an erase fail if it is
		 * what does the scan
		 */
		ut_assertok(mtd_block_isbad(subdev[i], 0));
		memset(&erase, '\0', sizeof(erase));
		erase.mtd = subdev[i];
		erase.len = subdev[i]->erasesize;
		ut_assertok(mtd_erase(subdev[i], &erase));
	}

	concat = mtd_concat_create(subdev, 2, "nand-test-concat");
	ut_assertnonnull(concat);

	/* Write the first page, which is in the first device */
	ops.mode = MTD_OPS_PLACE_OOB;
	ops.len = size;
	ops.datbuf = wbuf;
	ut_assertok(mtd_write_oob(concat, 0, &ops));
	ut_asserteq(size, ops.retlen);

	/* The sandbox NAND adds bit errors, which are corrected on reading */
	ret = mtd_read(subdev[0], 0, size, &retlen, rbuf);
	ut_assert(!ret || mtd_is_bitflip(ret));
	ut_asserteq_mem(wbuf, rbuf, size);

	/* Nothing must have been written to the second device */
	memset(wbuf, 0xff, size);
	ret = mtd_read(subdev[1], 0, size, &retlen, rbuf);
	ut_assert(!ret || mtd_is_bitflip(ret));
	ut_asserteq_mem(wbuf, rbuf, size);

	mtd_concat_destroy(concat);
	free(rbuf);
	free(wbuf);

	return 0;
}
DM_TEST(dm_test_mtd_concat_write_oob, UTF_SCAN_FDT);
