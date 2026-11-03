// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for attaching UBI to the sandbox NAND
 */

#include <malloc.h>
#include <mtd.h>
#include <ubi_uboot.h>
#include <dm/test.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <linux/string.h>
#include <test/test.h>
#include <test/ut.h>

#define UBI_TEST_MTD	"nand2"

static int ubi_test_erase_peb(struct unit_test_state *uts,
			      struct mtd_info *mtd, u64 off)
{
	struct erase_info ei;

	memset(&ei, 0, sizeof(ei));
	ei.mtd = mtd;
	ei.addr = off;
	ei.len = mtd->erasesize;
	ut_assertok(mtd_erase(mtd, &ei));

	return 0;
}

/*
 * Set data_size in the VID header of LEB 0 of volume 0, with a valid header
 * CRC, by rewriting the PEB that holds it. @peb holds one eraseblock.
 */
static int ubi_test_set_data_size(struct unit_test_state *uts,
				  struct mtd_info *mtd, u8 *peb, u32 data_size)
{
	struct ubi_ec_hdr *ec_hdr = (struct ubi_ec_hdr *)peb;
	struct ubi_vid_hdr *vid_hdr;
	size_t retlen;
	u64 off;

	for (off = 0; off < mtd->size; off += mtd->erasesize) {
		if (mtd_block_isbad(mtd, off))
			continue;
		ut_assertok(mtd_read(mtd, off, mtd->erasesize, &retlen, peb));
		if (be32_to_cpu(ec_hdr->magic) != UBI_EC_HDR_MAGIC)
			continue;
		vid_hdr = (struct ubi_vid_hdr *)
			(peb + be32_to_cpu(ec_hdr->vid_hdr_offset));
		if (be32_to_cpu(vid_hdr->magic) == UBI_VID_HDR_MAGIC &&
		    be32_to_cpu(vid_hdr->vol_id) == 0 &&
		    be32_to_cpu(vid_hdr->lnum) == 0)
			break;
	}
	ut_assert(off < mtd->size);

	vid_hdr->data_size = cpu_to_be32(data_size);
	vid_hdr->hdr_crc = cpu_to_be32(crc32(UBI_CRC32_INIT, vid_hdr,
					     UBI_VID_HDR_SIZE_CRC));

	ut_assertok(ubi_test_erase_peb(uts, mtd, off));
	ut_assertok(mtd_write(mtd, off, mtd->erasesize, &retlen, peb));

	return 0;
}

/* Find the test MTD device and erase it all */
static int ubi_test_erase_mtd(struct unit_test_state *uts,
			      struct mtd_info **mtdp)
{
	struct mtd_info *mtd;
	u64 off;

	mtd_probe_devices();
	mtd = get_mtd_device_nm(UBI_TEST_MTD);
	ut_assert(!IS_ERR_OR_NULL(mtd));
	put_mtd_device(mtd);

	for (off = 0; off < mtd->size; off += mtd->erasesize) {
		if (mtd_block_isbad(mtd, off))
			continue;
		ut_assertok(ubi_test_erase_peb(uts, mtd, off));
	}
	*mtdp = mtd;

	return 0;
}

static int ubi_test_vid_hdr_data_size(struct unit_test_state *uts)
{
	struct mtd_info *mtd;
	int leb_size;
	u8 *buf;

	ut_assertok(ubi_test_erase_mtd(uts, &mtd));

	buf = malloc(mtd->erasesize);
	ut_assertnonnull(buf);

	/* A static volume whose only LEB is full: data_size is the LEB size */
	ut_assertok(ubi_part(UBI_TEST_MTD, NULL));
	leb_size = ubi_devices[0]->leb_size;
	ut_assertok(ubi_create_vol("vol0", leb_size, false, 0, false));
	memset(buf, 0xa5, leb_size);
	ut_assertok(ubi_volume_write("vol0", buf, 0, leb_size));
	ubi_detach();

	/* It attaches again and reads back */
	ut_assertok(ubi_part(UBI_TEST_MTD, NULL));
	memset(buf, 0, leb_size);
	ut_assertok(ubi_volume_read("vol0", buf, 0, leb_size));
	ut_assertnull(memchr_inv(buf, 0xa5, leb_size));
	ubi_detach();

	/* One byte more than the LEB holds is a corrupt header */
	ut_assertok(ubi_test_set_data_size(uts, mtd, buf, leb_size + 1));
	ut_asserteq(-EINVAL, ubi_part(UBI_TEST_MTD, NULL));

	free(buf);

	return 0;
}

static int dm_test_ubi_vid_hdr_data_size(struct unit_test_state *uts)
{
	int ret;

	/* UBI holds the MTD device: release it before and after the test */
	ubi_detach();
	ret = ubi_test_vid_hdr_data_size(uts);
	ubi_detach();

	return ret;
}
DM_TEST(dm_test_ubi_vid_hdr_data_size, UTF_SCAN_FDT | UTF_LIVE_TREE);

/*
 * A static volume with data padding holds less than a LEB in each LEB, so the
 * size of its last LEB is bounded by its usable LEB size, not the LEB size.
 */
static int ubi_test_static_last_leb_size(struct unit_test_state *uts)
{
	struct ubi_mkvol_req req;
	struct ubi_device *ubi;
	struct ubi_volume *vol;
	struct mtd_info *mtd;
	int usable;
	u8 *buf;

	ut_assertok(ubi_test_erase_mtd(uts, &mtd));

	buf = malloc(mtd->erasesize);
	ut_assertnonnull(buf);

	/* One LEB, padded by one min. I/O unit, filled to its usable size */
	ut_assertok(ubi_part(UBI_TEST_MTD, NULL));
	ubi = ubi_devices[0];
	memset(&req, 0, sizeof(req));
	req.vol_id = 0;
	req.vol_type = UBI_STATIC_VOLUME;
	req.alignment = ubi->leb_size - ubi->min_io_size;
	req.bytes = ubi->leb_size - ubi->min_io_size;
	strcpy(req.name, "vol0");
	req.name_len = strlen(req.name);
	ut_assertok(ubi_create_volume(ubi, &req));
	vol = ubi_find_volume("vol0");
	ut_assertnonnull(vol);
	ut_asserteq(ubi->min_io_size, vol->data_pad);
	usable = vol->usable_leb_size;
	ut_asserteq(req.bytes, usable);
	memset(buf, 0xa5, usable);
	ut_assertok(ubi_volume_write("vol0", buf, 0, usable));
	ubi_detach();

	/* It attaches again and reads back */
	ut_assertok(ubi_part(UBI_TEST_MTD, NULL));
	memset(buf, 0, usable);
	ut_assertok(ubi_volume_read("vol0", buf, 0, usable));
	ut_assertnull(memchr_inv(buf, 0xa5, usable));
	ubi_detach();

	/*
	 * One byte more than the usable LEB size still fits in the LEB, so
	 * the VID header is valid, but the volume is not.
	 */
	ut_assertok(ubi_test_set_data_size(uts, mtd, buf, usable + 1));
	ut_asserteq(-EINVAL, ubi_part(UBI_TEST_MTD, NULL));

	free(buf);

	return 0;
}

static int dm_test_ubi_static_last_leb_size(struct unit_test_state *uts)
{
	int ret;

	/* UBI holds the MTD device: release it before and after the test */
	ubi_detach();
	ret = ubi_test_static_last_leb_size(uts);
	ubi_detach();

	return ret;
}
DM_TEST(dm_test_ubi_static_last_leb_size, UTF_SCAN_FDT | UTF_LIVE_TREE);
