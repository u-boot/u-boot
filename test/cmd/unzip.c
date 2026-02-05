// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Tests for zip/unzip/gzwrite commands
 *
 * Copyright 2026, Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <command.h>
#include <env.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/test.h>
#include <linux/sizes.h>
#include <mapmem.h>
#include <part.h>
#include <test/cmd.h>
#include <test/test.h>
#include <test/ut.h>
#include <u-boot/crc.h>

/* sys/random.h is not accessible */
extern ssize_t getrandom(void *buf, size_t size, unsigned int flags);

static const ssize_t sizes[] = { 32, SZ_1K, SZ_4K, SZ_1M, SZ_16M, SZ_1M - 1, SZ_1M + 1, 6758401 };

static int do_test_cmd_zip_unzip(struct unit_test_state *uts, ssize_t size,
				 const bool gzwrite)
{
	unsigned long loadaddr = env_get_ulong("loadaddr", 16, 0);
	unsigned long encaddr = loadaddr + size + 0x10000;
	unsigned long decaddr = encaddr + size + 0x10000;
	unsigned char *loadmap = map_sysmem(loadaddr, size);
	unsigned char *decmap = map_sysmem(decaddr, size);
	unsigned char *encmap = map_sysmem(encaddr, size);

	/*
	 * Prepare three buffers, $loadadd, $encaddr, $decaddr, and
	 * fill them all with random data. Add slight space between
	 * the compressed buffer 'encaddr' and uncompressed buffer
	 * 'decaddr', because the compressed data with gzip header
	 * might be longer than uncompressed source data 'loadaddr',
	 * and if the uncompressed data buffer 'decaddr' followed
	 * 'encaddr', the decompression could corrupt end of 'encaddr'
	 * buffer.
	 */

	ut_assert(getrandom(loadmap, size, 0) == size);
	ut_assert(getrandom(decmap, size, 0) == size);
	ut_assert(getrandom(encmap, size, 0) == size);

	/* Compress data in $loadaddr into $encaddr */
	ut_assertok(run_commandf("zip $loadaddr %zx %zx", size, encaddr));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_assert(strstr(uts->actual_str, "Compressed size: "));

	if (gzwrite) {
		unsigned int sectsize = DIV_ROUND_UP(size, 512);
		u32 crc = crc32(0, loadmap, size);
		struct blk_desc *mmc_dev_desc;

		ut_assertok(run_commandf("gzwrite mmc 9 %zx $filesize", encaddr));
		ut_assert_skip_to_line("\t%zu bytes, crc 0x%08x", size, crc);

		ut_asserteq(9, blk_get_device_by_str("mmc", "9", &mmc_dev_desc));
		ut_assertok(run_commandf("mmc dev 9"));
		ut_assert_nextline("switch to partitions #0, OK");
		ut_assert_nextline("mmc9 is current device");

		ut_assertok(run_commandf("mmc read %zx 0 %x", decaddr, sectsize));
		ut_assert_nextline("MMC read: dev # 9, block # 0, count %u ... %u blocks read: OK",
				   sectsize, sectsize);
	} else {
		/* Decompress data in $encaddr into $decaddr */
		ut_assertok(run_commandf("unzip %zx %zx $filesize", encaddr, decaddr));
		ut_assert_nextline("Uncompressed size: %zu = 0x%zX", size, size);
	}

	/* Input data and compressed-decompressed data */
	ut_asserteq_mem(loadmap, decmap, size);

	ut_assert_console_end();

	unmap_sysmem(loadmap);
	unmap_sysmem(decmap);
	unmap_sysmem(encmap);

	return 0;
}

static int dm_test_cmd_zip_unzip(struct unit_test_state *uts)
{
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(sizes); i++) {
		ret = do_test_cmd_zip_unzip(uts, sizes[i], false);
		if (ret)
			return ret;
	}

	return 0;
}
DM_TEST(dm_test_cmd_zip_unzip, UTF_CONSOLE);

static int dm_test_cmd_zip_gzwrite(struct unit_test_state *uts)
{
	struct udevice *dev;
	ofnode root, node;
	int i, ret;

	/* Enable the mmc9 node for this test */
	root = oftree_root(oftree_default());
	node = ofnode_find_subnode(root, "mmc9");
	ut_assert(ofnode_valid(node));
	ut_assertok(lists_bind_fdt(gd->dm_root, node, &dev, NULL, false));

	for (i = 0; i < ARRAY_SIZE(sizes); i++) {
		ret = do_test_cmd_zip_unzip(uts, sizes[i], true);
		if (ret)
			return ret;
	}

	return 0;
}
DM_TEST(dm_test_cmd_zip_gzwrite, UTF_CONSOLE);
