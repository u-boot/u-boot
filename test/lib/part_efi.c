// SPDX-License-Identifier: GPL-2.0+
// SPDX-FileCopyrightText: 2026 Flipper FZCO
/*
 * Tests for the GPT layout helpers in disk/part_efi.c
 */

#include <blk.h>
#include <blkmap.h>
#include <dm.h>
#include <malloc.h>
#include <memalign.h>
#include <part.h>
#include <part_efi.h>
#include <asm/byteorder.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

/*
 * Check that the GPT layout adapts to the block size of the device. Neither
 * gpt_fill_header() nor gpt_fill_pte() does any block I/O, so a synthetic
 * descriptor is enough here, and no block device is needed. The only outside
 * dependency is partition_entries_offset(), which reads the device tree
 * '/config' node, and that is available without scanning for devices.
 */
static int lib_test_gpt_blksz(struct unit_test_state *uts)
{
	static const struct {
		unsigned long blksz;
		lbaint_t lba;
	} cases[] = {
		{   512, 0x2000 },	/* 4MB, the traditional 34-block layout */
		{  1024, 0x1000 },
		{  2048,  0x800 },
		{  4096,  0x400 },	/* 4K native media, e.g. UFS */
		{  8192,  0x200 },
		{ 16384,  0x100 },	/* the array fits in a single block... */
		{ 32768,   0x40 },	/* ...on a device smaller than 34 blocks */
	};

	char str_disk_guid[UUID_STR_LEN + 1] =
		"8d60b397-1bb6-4d33-80ee-b1587d24c2f8";
	struct disk_partition part;
	struct blk_desc desc;
	gpt_header gpt_h;
	gpt_entry *gpt_e;
	int i;

	gpt_e = calloc(GPT_ENTRY_NUMBERS, sizeof(gpt_entry));
	ut_assertnonnull(gpt_e);

	for (i = 0; i < ARRAY_SIZE(cases); i++) {
		u64 entry_lba, first_lba, last_lba;
		u32 pte_blks;

		memset(&desc, '\0', sizeof(desc));
		memset(&gpt_h, '\0', sizeof(gpt_h));
		desc.blksz = cases[i].blksz;
		desc.log2blksz = LOG2(desc.blksz);
		desc.lba = cases[i].lba;

		ut_assertok(gpt_fill_header(&desc, &gpt_h, str_disk_guid, 1));

		/*
		 * Size the array with BLOCK_CNT() rather than the
		 * DIV_ROUND_UP() that gpt_pte_blocks() uses, so that this
		 * checks the layout instead of restating the implementation.
		 */
		pte_blks = BLOCK_CNT(le32_to_cpu(gpt_h.num_partition_entries) *
				     le32_to_cpu(gpt_h.sizeof_partition_entry),
				     (&desc));
		entry_lba = le64_to_cpu(gpt_h.partition_entry_lba);
		first_lba = le64_to_cpu(gpt_h.first_usable_lba);
		last_lba = le64_to_cpu(gpt_h.last_usable_lba);

		ut_asserteq_64(1, le64_to_cpu(gpt_h.my_lba));
		ut_asserteq_64(desc.lba - 1, le64_to_cpu(gpt_h.alternate_lba));

		/* the primary array ends where the usable area begins */
		ut_assert(entry_lba >= 2);
		ut_asserteq_64(entry_lba + pte_blks, first_lba);
		ut_assert(first_lba < last_lba);

		/*
		 * write_gpt_table() puts the backup array at
		 * last_usable_lba + 1, so it has to end exactly where the
		 * backup header begins
		 */
		ut_asserteq_64(le64_to_cpu(gpt_h.alternate_lba),
			       last_lba + 1 + pte_blks);

		/* a partition with no start and no size fills the disk */
		memset(&part, '\0', sizeof(part));
		disk_partition_set_uuid(&part, str_disk_guid);
		ut_assertok(gpt_fill_pte(&desc, &gpt_h, gpt_e, &part, 1));
		ut_asserteq_64(first_lba, le64_to_cpu(gpt_e[0].starting_lba));
		ut_asserteq_64(last_lba, le64_to_cpu(gpt_e[0].ending_lba));

		/* a partition overlapping the entry array is rejected */
		part.start = entry_lba;
		part.size = 1;
		ut_asserteq(-ENOSPC,
			    gpt_fill_pte(&desc, &gpt_h, gpt_e, &part, 1));

		/* so is one running past the end of the usable area */
		part.start = first_lba;
		part.size = last_lba - first_lba + 2;
		ut_asserteq(-E2BIG,
			    gpt_fill_pte(&desc, &gpt_h, gpt_e, &part, 1));
	}

	free(gpt_e);

	return 0;
}
LIB_TEST(lib_test_gpt_blksz, 0);

#if CONFIG_IS_ENABLED(BLKMAP)

#define GPT_TEST_LABEL	"gpttest"

/**
 * gpt_blkmap_check() - write a GPT to a blkmap device and read it back
 *
 * @uts:	test state
 * @dev:	blkmap device with nothing mapped yet
 * @blksz:	block size the device has to report
 * @lba:	size of the device in blocks
 * @buf:	memory to back the device with, @lba blocks of @blksz
 * @gpt_head:	buffer for one GPT header, padded to @blksz
 * Return: 0 if OK, other value on error
 */
static int gpt_blkmap_check(struct unit_test_state *uts, struct udevice *dev,
			    unsigned long blksz, lbaint_t lba, void *buf,
			    gpt_header *gpt_head)
{
	char str_disk_guid[UUID_STR_LEN + 1] =
		"8d60b397-1bb6-4d33-80ee-b1587d24c2f8";
	struct disk_partition parts[2], info;
	struct blk_desc *desc;
	struct udevice *blk;
	gpt_entry *gpt_pte;
	u64 first_lba;

	ut_assertok(blkmap_map_mem(dev, 0, lba, buf));
	ut_assertok(blk_get_from_parent(dev, &blk));
	desc = dev_get_uclass_plat(blk);

	ut_asserteq(blksz, desc->blksz);
	ut_asserteq_64(lba, desc->lba);

	/* two partitions of a fixed size, neither with a start */
	memset(parts, '\0', sizeof(parts));
	strcpy((char *)parts[0].name, "first");
	strcpy((char *)parts[1].name, "second");
	parts[0].size = 0x100;
	parts[1].size = 0x100;
	disk_partition_set_uuid(&parts[0],
				"a2a0d0eb-e5b9-3344-87c0-68b6b72699c7");
	disk_partition_set_uuid(&parts[1],
				"a2a0d0eb-e5b9-3344-87c0-68b6b72699c8");

	ut_assertok(gpt_restore(desc, str_disk_guid, parts, 2));

	/*
	 * Read the table back and compare it against the layout it was asked
	 * for. This walks gpt_verify_headers() and is_gpt_valid() on the way,
	 * so it covers the header fields those two convert.
	 */
	gpt_pte = NULL;
	ut_assertok(gpt_verify_partitions(desc, parts, 2, gpt_head, &gpt_pte));
	free(gpt_pte);

	/*
	 * And parse it the way every other user of a disk does. Clear the
	 * existing partition type to force part_init() to guess it anew,
	 * otherwise whatever stale state remains sticks across the part_init()
	 * call
	 */
	desc->part_type = PART_TYPE_UNKNOWN;
	part_init(desc);
	ut_asserteq(PART_TYPE_EFI, desc->part_type);

	/*
	 * gpt_verify_partitions() skips the start of a partition that was not
	 * given one, and reads names through a different helper than
	 * part_get_info() does, so check both here
	 */
	first_lba = le64_to_cpu(gpt_head->first_usable_lba);
	ut_assertok(part_get_info(desc, 1, &info));
	ut_asserteq_str((char *)parts[0].name, (char *)info.name);
	ut_asserteq_64(first_lba, info.start);
	ut_asserteq_64(parts[0].size, info.size);
	ut_assertok(part_get_info(desc, 2, &info));
	ut_asserteq_str((char *)parts[1].name, (char *)info.name);
	ut_asserteq_64(first_lba + parts[0].size, info.start);
	ut_asserteq_64(parts[1].size, info.size);

	/* the backup header has to be in the very last block */
	ut_asserteq(1, blk_dread(desc, lba - 1, 1, gpt_head));
	ut_asserteq_64(GPT_HEADER_SIGNATURE_UBOOT,
		       le64_to_cpu(gpt_head->signature));
	ut_asserteq_64(lba - 1, le64_to_cpu(gpt_head->my_lba));
	ut_asserteq_64(1, le64_to_cpu(gpt_head->alternate_lba));

	return 0;
}

/*
 * Write a GPT to a real block device, then read it back the way the rest of
 * U-Boot does. gpt_verify_partitions() and part_get_info() both start by
 * reading blocks, so neither is reachable from lib_test_gpt_blksz(), which has
 * only a synthetic descriptor. Backing the device with memory through blkmap
 * keeps this runnable anywhere rather than on sandbox alone.
 */
static int lib_test_gpt_blkmap(struct unit_test_state *uts)
{
	static const struct {
		unsigned long blksz;
		lbaint_t lba;
	} cases[] = {
		/*
		 * Keep the disk small: the backing memory comes out of the
		 * malloc pool, which is only a few MB on boards like malta.
		 * Add a 4096-byte case here once blkmap can be created with a
		 * block size.
		 */
		{ 512, 0x400 },
	};

	int i;

	for (i = 0; i < ARRAY_SIZE(cases); i++) {
		unsigned long blksz = cases[i].blksz;
		lbaint_t lba = cases[i].lba;
		gpt_header *gpt_head;
		struct udevice *dev;
		int ret, err;
		void *buf;

		/*
		 * blkmap devices are fixed at DEFAULT_BLKSZ, so this is the
		 * only size that can be asked for today
		 */
		ut_asserteq(DEFAULT_BLKSZ, blksz);

		/*
		 * Start from a dirty disk rather than a blank one, since
		 * gpt_restore() has to write everything that is read back
		 */
		buf = memalign(ARCH_DMA_MINALIGN, lba * blksz);
		ut_assertnonnull(buf);
		memset(buf, 0xa5, lba * blksz);

		gpt_head = malloc_cache_aligned(roundup(sizeof(gpt_header),
							blksz));
		ret = gpt_head ? blkmap_create(GPT_TEST_LABEL, &dev) : -ENOMEM;
		if (ret) {
			free(gpt_head);
			free(buf);
			ut_assertok(ret);
		}

		/*
		 * A failed check returns early, so clean up here rather than
		 * in the helper. The device lives under the real dm_root, and
		 * one left bound would fail every later run in blkmap_create()
		 * with -EBUSY, hiding whatever went wrong first.
		 */
		ret = gpt_blkmap_check(uts, dev, blksz, lba, buf, gpt_head);
		err = blkmap_destroy(dev);
		free(gpt_head);
		free(buf);
		if (ret)
			return ret;
		ut_assertok(err);
	}

	return 0;
}
LIB_TEST(lib_test_gpt_blkmap, 0);

#endif /* BLKMAP */
