// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <sean.anderson@seco.com>
 */

#include <blk.h>
#include <dm.h>
#include <env.h>
#include <memalign.h>
#include <mmc.h>
#include <part.h>
#include <part_dos.h>
#include <part_efi.h>
#include <dm/test.h>
#include <test/ut.h>

static int do_test(struct unit_test_state *uts, int expected,
		   const char *part_str, bool whole)
{
	struct blk_desc *mmc_dev_desc;
	struct disk_partition part_info;

	int ret = part_get_info_by_dev_and_name_or_num("mmc", part_str,
						       &mmc_dev_desc,
						       &part_info, whole);

	ut_assertf(expected == ret, "test(%d, \"%s\", %d) == %d", expected,
		   part_str, whole, ret);
	return 0;
}

static int dm_test_part(struct unit_test_state *uts)
{
	char *oldbootdevice;
	char str_disk_guid[UUID_STR_LEN + 1];
	int ret;
	struct blk_desc *mmc_dev_desc;
	struct disk_partition parts[2] = {
		{
			.start = 48, /* GPT data takes up the first 34 blocks or so */
			.size = 1,
			.name = "test1",
		},
		{
			.start = 49,
			.size = 1,
			.name = "test2",
		},
	};

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", &mmc_dev_desc));
	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		gen_rand_uuid_str(parts[0].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(parts[1].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(str_disk_guid, UUID_STR_FORMAT_STD);
	}
	ut_assertok(gpt_restore(mmc_dev_desc, str_disk_guid, parts,
				ARRAY_SIZE(parts)));

	oldbootdevice = env_get("bootdevice");

#define test(expected, part_str, whole) \
	ut_assertok(do_test(uts, expected, part_str, whole))

	env_set("bootdevice", NULL);
	test(-ENODEV, NULL, true);
	test(-ENODEV, "", true);
	env_set("bootdevice", "0");
	test(0, NULL, true);
	test(0, "", true);
	env_set("bootdevice", "2");
	test(1, NULL, false);
	test(1, "", false);
	test(1, "-", false);
	env_set("bootdevice", "");
	test(-EPROTONOSUPPORT, "0", false);
	test(0, "0", true);
	test(0, ":0", true);
	test(0, ".0", true);
	test(0, ".0:0", true);
	test(-EINVAL, "#test1", true);
	test(1, "2", false);
	test(1, "2", true);
	test(-ENOENT, "2:0", false);
	test(0, "2:0", true);
	test(1, "2:1", false);
	test(2, "2:2", false);
	test(1, "2.0", false);
	test(0, "2.0:0", true);
	test(1, "2.0:1", false);
	test(2, "2.0:2", false);
	test(-EINVAL, "2#bogus", false);
	test(1, "2#test1", false);
	test(2, "2#test2", false);
	ret = 0;

	env_set("bootdevice", oldbootdevice);
	return ret;
}
DM_TEST(dm_test_part, UTF_SCAN_PDATA | UTF_SCAN_FDT);

static int dm_test_part_bootable(struct unit_test_state *uts)
{
	struct blk_desc *desc;
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_BLK, "mmc1.blk", &dev));
	desc = dev_get_uclass_plat(dev);
	ut_asserteq(1, part_get_bootable(desc));

	return 0;
}
DM_TEST(dm_test_part_bootable, UTF_SCAN_FDT);

static int do_get_info_test(struct unit_test_state *uts,
			    struct blk_desc *dev_desc, int part, int part_type,
			    struct disk_partition const *reference)
{
	struct disk_partition p;
	int ret;

	memset(&p, 0, sizeof(p));

	ret = part_get_info_by_type(dev_desc, part, part_type, &p);
	printf("part_get_info_by_type(%d, 0x%x) = %d\n", part, part_type, ret);
	if (ut_assertok(ret)) {
		return 0;
	}

	ut_asserteq(reference->start, p.start);
	ut_asserteq(reference->size, p.size);
	ut_asserteq(reference->sys_ind, p.sys_ind);

	return 0;
}

static int dm_test_part_get_info_by_type(struct unit_test_state *uts)
{
	char str_disk_guid[UUID_STR_LEN + 1];
	struct blk_desc *mmc_dev_desc;
	struct disk_partition gpt_parts[] = {
		{
			.start = 48, /* GPT data takes up the first 34 blocks or so */
			.size = 1,
			.name = "test1",
			.sys_ind = 0,
		},
		{
			.start = 49,
			.size = 1,
			.name = "test2",
			.sys_ind = 0,
		},
	};
	struct disk_partition mbr_parts[] = {
		{
			.start = 1,
			.size = 33,
			.name = "gpt",
			.sys_ind = EFI_PMBR_OSTYPE_EFI_GPT,
		},
		{
			.start = 48,
			.size = 1,
			.name = "test1",
			.sys_ind = 0x83,
		},
	};

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", &mmc_dev_desc));
	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		gen_rand_uuid_str(gpt_parts[0].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(gpt_parts[1].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(str_disk_guid, UUID_STR_FORMAT_STD);
	}
	ut_assertok(gpt_restore(mmc_dev_desc, str_disk_guid, gpt_parts,
				ARRAY_SIZE(gpt_parts)));

	ut_assertok(write_mbr_partitions(mmc_dev_desc, mbr_parts,
					 ARRAY_SIZE(mbr_parts), 0));

#define get_info_test(_part, _part_type, _reference) \
	ut_assertok(do_get_info_test(uts, mmc_dev_desc, _part, _part_type, \
				     _reference))

	for (int i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		get_info_test(i + 1, PART_TYPE_UNKNOWN, &gpt_parts[i]);
	}

	for (int i = 0; i < ARRAY_SIZE(mbr_parts); i++) {
		get_info_test(i + 1, PART_TYPE_DOS, &mbr_parts[i]);
	}

	for (int i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		get_info_test(i + 1, PART_TYPE_EFI, &gpt_parts[i]);
	}

	return 0;
}
DM_TEST(dm_test_part_get_info_by_type, UTF_SCAN_PDATA | UTF_SCAN_FDT);

static int dm_test_part_get_info_by_uuid(struct unit_test_state *uts)
{
	struct disk_partition parts[] = {
		{
			.start = 48,
			.size = 1,
			.name = "test1",
			.uuid = "c5bce7a2-03f0-4d03-9048-01ff23b9d527",
		},
		{
			.start = 49,
			.size = 1,
			.name = "test2",
			.uuid = "9df346e8-2c53-4cd8-b9ac-3af83f9a9b74",
		},
	};
	char disk_guid[UUID_STR_LEN + 1] =
		"8d60b397-1bb6-4d33-80ee-b1587d24c2f8";
	struct blk_desc *mmc_dev_desc;
	struct disk_partition info;
	int part, i;

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", &mmc_dev_desc));

	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		for (i = 0; i < ARRAY_SIZE(parts); i++)
			gen_rand_uuid_str(parts[i].uuid, UUID_STR_FORMAT_STD);

		gen_rand_uuid_str(disk_guid, UUID_STR_FORMAT_STD);
	}

	ut_assertok(gpt_restore(mmc_dev_desc, disk_guid, parts,
				ARRAY_SIZE(parts)));

	for (i = 0; i < ARRAY_SIZE(parts); i++) {
		part = part_get_info_by_uuid(mmc_dev_desc, parts[i].uuid,
					     &info);

		ut_asserteq(i + 1, part);
		ut_asserteq_str(parts[i].name, info.name);
		ut_asserteq(parts[i].start, info.start);
		ut_asserteq(parts[i].size, info.size);
	}

	part = part_get_info_by_uuid(mmc_dev_desc,
				     "00000000-0000-0000-0000-000000000000",
				     &info);
	ut_assert(part < 0);

	return 0;
}
DM_TEST(dm_test_part_get_info_by_uuid, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/**
 * dos_set_entry() - fill in one entry of a DOS partition table
 *
 * @pt:		entry to fill in
 * @sys_ind:	partition type
 * @start:	first sector, relative to what the table type says
 * @size:	number of sectors
 */
static void dos_set_entry(dos_partition_t *pt, u8 sys_ind, u32 start,
			  u32 size)
{
	memset(pt, '\0', sizeof(*pt));
	pt->sys_ind = sys_ind;
	pt->start_sect = cpu_to_le32(start);
	pt->nr_sects = cpu_to_le32(size);
}

/**
 * dos_write_table() - write a DOS partition table to a sector
 *
 * @desc:	block device to write to, with 512-byte blocks
 * @lba:	sector to write the table to
 * @pt:		the four entries of the table
 * Return: 0 if OK, -EIO if the sector could not be written
 */
static int dos_write_table(struct blk_desc *desc, lbaint_t lba,
			   const dos_partition_t pt[4])
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, buf, 512);

	memset(buf, '\0', 512);
	memcpy(buf + DOS_PART_TBL_OFFSET, pt, 4 * sizeof(*pt));
	buf[DOS_PART_MAGIC_OFFSET] = 0x55;
	buf[DOS_PART_MAGIC_OFFSET + 1] = 0xaa;

	return blk_dwrite(desc, lba, 1, buf) == 1 ? 0 : -EIO;
}

/*
 * A link from one extended partition table to the next adds no partition,
 * so a walk through tables that link to each other in a loop finds no more
 * partitions as it goes. Check that it ends anyway, both when looking for a
 * partition and when listing them, and that a table holding only a link is
 * still followed.
 */
static int dm_test_part_dos_ext_loop(struct unit_test_state *uts)
{
	struct disk_partition info;
	dos_partition_t pt[4];
	struct blk_desc *desc;

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", &desc));
	ut_asserteq(512, desc->blksz);

	/* an MBR with one extended partition, from sector 2 */
	memset(pt, '\0', sizeof(pt));
	dos_set_entry(&pt[0], DOS_PART_TYPE_EXTENDED, 2, 100);
	ut_assertok(dos_write_table(desc, 0, pt));

	/*
	 * logical partition 5 in a table of its own, then a table holding only
	 * a link, then logical partition 6; a link is relative to the extended
	 * partition, a partition to its own table
	 */
	dos_set_entry(&pt[0], 0x83, 1, 4);
	dos_set_entry(&pt[1], DOS_PART_TYPE_EXTENDED, 10, 10);
	ut_assertok(dos_write_table(desc, 2, pt));
	memset(pt, '\0', sizeof(pt));
	dos_set_entry(&pt[0], DOS_PART_TYPE_EXTENDED, 20, 10);
	ut_assertok(dos_write_table(desc, 12, pt));
	dos_set_entry(&pt[0], 0x83, 1, 4);
	dos_set_entry(&pt[1], DOS_PART_TYPE_EXTENDED, 30, 10);
	ut_assertok(dos_write_table(desc, 22, pt));

	/* then a table with no partition, only a link back to itself */
	memset(pt, '\0', sizeof(pt));
	dos_set_entry(&pt[0], DOS_PART_TYPE_EXTENDED, 30, 10);
	ut_assertok(dos_write_table(desc, 32, pt));

	ut_assertok(part_get_info_by_type(desc, 5, PART_TYPE_DOS, &info));
	ut_asserteq(3, info.start);
	ut_asserteq(4, info.size);
	ut_assertok(part_get_info_by_type(desc, 6, PART_TYPE_DOS, &info));
	ut_asserteq(23, info.start);
	ut_asserteq(4, info.size);

	/* there is no partition 7, and looking for it has to end */
	ut_asserteq(-ENOENT,
		    part_get_info_by_type(desc, 7, PART_TYPE_DOS, &info));
	ut_assert_nextline("** Nested DOS partitions detected, stopping **");
	ut_assert_console_end();

	/* and so does listing them, once the table is detected as DOS */
	part_init(desc);
	ut_asserteq(PART_TYPE_DOS, desc->part_type);
	part_print(desc);
	ut_assert_skip_to_line("%3d\t%-10d\t%-10d\t%08x-%02x\t%02x", 6, 23, 4,
			       0, 6, 0x83);
	ut_assert_nextline("** Nested DOS partitions detected, stopping **");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_part_dos_ext_loop, UTF_SCAN_PDATA | UTF_SCAN_FDT | UTF_CONSOLE);

/**
 * dos_write_chain() - write a chain of extended partition tables
 *
 * The MBR holds an extended partition from sector 2, whose first table
 * holds logical partition 5 and a link, then come @empty tables holding only
 * a link, and then a table holding logical partition 6.
 *
 * @desc:	block device to write to, with 512-byte blocks
 * @empty:	number of tables holding only a link
 * Return: sector where partition 6 starts, or -EIO on error
 */
static int dos_write_chain(struct blk_desc *desc, int empty)
{
	dos_partition_t pt[4];
	int i;

	memset(pt, '\0', sizeof(pt));
	dos_set_entry(&pt[0], DOS_PART_TYPE_EXTENDED, 2, 1000);
	if (dos_write_table(desc, 0, pt))
		return -EIO;

	/* table n is at sector 2 + 10 * n, its link 10 * (n + 1) */
	for (i = 0; i <= empty + 1; i++) {
		memset(pt, '\0', sizeof(pt));
		if (i == 0 || i == empty + 1)
			dos_set_entry(&pt[0], 0x83, 1, 4);
		if (i <= empty)
			dos_set_entry(&pt[1], DOS_PART_TYPE_EXTENDED,
				      10 * (i + 1), 10);
		if (dos_write_table(desc, 2 + 10 * i, pt))
			return -EIO;
	}

	return 2 + 10 * (empty + 1) + 1;
}

/*
 * Check that a walk through the extended partition tables follows up to 15
 * tables in a row holding no partition, as a damaged but valid chain may
 * have, but gives up at the 16th
 */
static int dm_test_part_dos_ext_empty(struct unit_test_state *uts)
{
	struct disk_partition info;
	struct blk_desc *desc;
	int start;

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", &desc));
	ut_asserteq(512, desc->blksz);

	start = dos_write_chain(desc, 15);
	ut_assert(start > 0);
	ut_assertok(part_get_info_by_type(desc, 6, PART_TYPE_DOS, &info));
	ut_asserteq(start, info.start);
	ut_assert_console_end();

	ut_assert(dos_write_chain(desc, 16) > 0);
	ut_assertok(part_get_info_by_type(desc, 5, PART_TYPE_DOS, &info));
	ut_asserteq(-ENOENT,
		    part_get_info_by_type(desc, 6, PART_TYPE_DOS, &info));
	ut_assert_nextline("** Nested DOS partitions detected, stopping **");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_part_dos_ext_empty, UTF_SCAN_PDATA | UTF_SCAN_FDT | UTF_CONSOLE);

/*
 * Partition numbers above 255 take three digits in the UUID made up for a
 * DOS partition, from the disk signature and the partition number
 */
static int dm_test_part_dos_uuid(struct unit_test_state *uts)
{
	struct disk_partition info;
	dos_partition_t pt[4];
	struct blk_desc *desc;
	int part;

	if (!CONFIG_IS_ENABLED(PARTITION_UUIDS))
		return -EAGAIN;

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", &desc));
	ut_asserteq(512, desc->blksz);

	/* an extended partition from sector 2 */
	memset(pt, '\0', sizeof(pt));
	dos_set_entry(&pt[0], DOS_PART_TYPE_EXTENDED, 2, 512);
	ut_assertok(dos_write_table(desc, 0, pt));

	/*
	 * holding logical partitions 5 to 256, each of one sector after its
	 * table, with the tables two sectors apart from sector 2 on
	 */
	for (part = 5; part <= 256; part++) {
		memset(pt, '\0', sizeof(pt));
		dos_set_entry(&pt[0], 0x83, 1, 1);
		if (part < 256)
			dos_set_entry(&pt[1], DOS_PART_TYPE_EXTENDED,
				      (part - 4) * 2, 2);
		ut_assertok(dos_write_table(desc, 2 + (part - 5) * 2, pt));
	}

	ut_assertok(part_get_info_by_type(desc, 255, PART_TYPE_DOS, &info));
	ut_asserteq(2 + 250 * 2 + 1, info.start);
	ut_asserteq_str("00000000-ff", disk_partition_uuid(&info));
	ut_assertok(part_get_info_by_type(desc, 256, PART_TYPE_DOS, &info));
	ut_asserteq(2 + 251 * 2 + 1, info.start);
	ut_asserteq_str("00000000-100", disk_partition_uuid(&info));

	return 0;
}
DM_TEST(dm_test_part_dos_uuid, UTF_SCAN_PDATA | UTF_SCAN_FDT);
