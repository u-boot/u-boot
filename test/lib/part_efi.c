// SPDX-License-Identifier: GPL-2.0+
// SPDX-FileCopyrightText: 2026 Flipper FZCO
/*
 * Tests for the GPT layout helpers in disk/part_efi.c
 */

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
