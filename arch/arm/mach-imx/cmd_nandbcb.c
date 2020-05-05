/*
 * i.MX6 nand boot control block(bcb).
 *
 * Based on the common/imx-bbu-nand-fcb.c from barebox and imx kobs-ng
 *
 * Copyright (C) 2017 Jagan Teki <jagan@amarulasolutions.com>
 * Copyright (C) 2016 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <nand.h>
#include <dm/devres.h>

#include <asm/io.h>
#include <jffs2/jffs2.h>
#include <linux/bch.h>
#include <linux/mtd/mtd.h>

#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/imx-nandbcb.h>
#include <asm/mach-imx/imximage.cfg>
#include <mxs_nand.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <div64.h>

#include "../../../cmd/legacy-mtd-utils.h"

#define BF_VAL(v, bf)		(((v) & bf##_MASK) >> bf##_OFFSET)
#define GETBIT(v, n)		(((v) >> (n)) & 0x1)
#define IMX8MQ_SPL_SZ 0x3e000
#define IMX8MQ_HDMI_FW_SZ 0x19c00
#define BOOT_SEARCH_COUNT 2

struct mtd_info *dump_mtd;
static loff_t dump_nandboot_size;
static struct fcb_block dump_fill_fcb;
static struct dbbt_block dump_fill_dbbt;
static struct fcb_block dump_nand_fcb[BOOT_SEARCH_COUNT];
static struct dbbt_block dump_nand_dbbt[BOOT_SEARCH_COUNT];
static u32 dump_fcb_off[BOOT_SEARCH_COUNT];
static u32 dump_dbbt_off[BOOT_SEARCH_COUNT];

#if defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
static uint8_t reverse_bit(uint8_t b)
{
	b = (b & 0xf0) >> 4 | (b & 0x0f) << 4;
	b = (b & 0xcc) >> 2 | (b & 0x33) << 2;
	b = (b & 0xaa) >> 1 | (b & 0x55) << 1;

	return b;
}

static void encode_bch_ecc(void *buf, struct fcb_block *fcb, int eccbits)
{
	int i, j, m = 13;
	int blocksize = 128;
	int numblocks = 8;
	int ecc_buf_size = (m * eccbits + 7) / 8;
	struct bch_control *bch = init_bch(m, eccbits, 0);
	u8 *ecc_buf = kzalloc(ecc_buf_size, GFP_KERNEL);
	u8 *tmp_buf = kzalloc(blocksize * numblocks, GFP_KERNEL);
	u8 *psrc, *pdst;

	/*
	 * The blocks here are bit aligned. If eccbits is a multiple of 8,
	 * we just can copy bytes. Otherwiese we must move the blocks to
	 * the next free bit position.
	 */
	WARN_ON(eccbits % 8);

	memcpy(tmp_buf, fcb, sizeof(*fcb));

	for (i = 0; i < numblocks; i++) {
		memset(ecc_buf, 0, ecc_buf_size);
		psrc = tmp_buf + i * blocksize;
		pdst = buf + i * (blocksize + ecc_buf_size);

		/* copy data byte aligned to destination buf */
		memcpy(pdst, psrc, blocksize);

		/*
		 * imx-kobs use a modified encode_bch which reverse the
		 * bit order of the data before calculating bch.
		 * Do this in the buffer and use the bch lib here.
		 */
		for (j = 0; j < blocksize; j++)
			psrc[j] = reverse_bit(psrc[j]);

		encode_bch(bch, psrc, blocksize, ecc_buf);

		/* reverse ecc bit */
		for (j = 0; j < ecc_buf_size; j++)
			ecc_buf[j] = reverse_bit(ecc_buf[j]);

		/* Here eccbuf is byte aligned and we can just copy it */
		memcpy(pdst + blocksize, ecc_buf, ecc_buf_size);
	}

	kfree(ecc_buf);
	kfree(tmp_buf);
	free_bch(bch);
}
#else

static u8 calculate_parity_13_8(u8 d)
{
	u8 p = 0;

	p |= (GETBIT(d, 6) ^ GETBIT(d, 5) ^ GETBIT(d, 3) ^ GETBIT(d, 2)) << 0;
	p |= (GETBIT(d, 7) ^ GETBIT(d, 5) ^ GETBIT(d, 4) ^ GETBIT(d, 2) ^
	      GETBIT(d, 1)) << 1;
	p |= (GETBIT(d, 7) ^ GETBIT(d, 6) ^ GETBIT(d, 5) ^ GETBIT(d, 1) ^
	      GETBIT(d, 0)) << 2;
	p |= (GETBIT(d, 7) ^ GETBIT(d, 4) ^ GETBIT(d, 3) ^ GETBIT(d, 0)) << 3;
	p |= (GETBIT(d, 6) ^ GETBIT(d, 4) ^ GETBIT(d, 3) ^ GETBIT(d, 2) ^
	      GETBIT(d, 1) ^ GETBIT(d, 0)) << 4;

	return p;
}

static void encode_hamming_13_8(void *_src, void *_ecc, size_t size)
{
	int i;
	u8 *src = _src;
	u8 *ecc = _ecc;

	for (i = 0; i < size; i++)
		ecc[i] = calculate_parity_13_8(src[i]);
}
#endif

static u32 calc_chksum(void *buf, size_t size)
{
	u32 chksum = 0;
	u8 *bp = buf;
	size_t i;

	for (i = 0; i < size; i++)
		chksum += bp[i];

	return ~chksum;
}

static void fill_fcb(struct fcb_block *fcb, struct mtd_info *mtd,
		     u32 fw1_start, u32 fw2_start, u32 fw_pages)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	struct mxs_nand_layout l;

	mxs_nand_get_layout(mtd, &l);

	fcb->fingerprint = FCB_FINGERPRINT;
	fcb->version = FCB_VERSION_1;

	fcb->pagesize = mtd->writesize;
	fcb->oob_pagesize = mtd->writesize + mtd->oobsize;
	fcb->sectors = mtd->erasesize / mtd->writesize;

	fcb->meta_size = l.meta_size;
	fcb->nr_blocks = l.nblocks;
	fcb->ecc_nr = l.data0_size;
	fcb->ecc_level = l.ecc0;
	fcb->ecc_size = l.datan_size;
	fcb->ecc_type = l.eccn;
	fcb->bchtype = l.gf_len;

	/* Also hardcoded in kobs-ng */
	if (is_mx6() || is_imx8m()) {
		fcb->datasetup = 80;
		fcb->datahold = 60;
		fcb->addr_setup = 25;
		fcb->dsample_time = 6;
	} else if (is_mx7()) {
		fcb->datasetup = 10;
		fcb->datahold = 7;
		fcb->addr_setup = 15;
		fcb->dsample_time = 6;
	}

	/* DBBT search area starts at second page on first block */
	fcb->dbbt_start = 1;

	fcb->bb_byte = nand_info->bch_geometry.block_mark_byte_offset;
	fcb->bb_start_bit = nand_info->bch_geometry.block_mark_bit_offset;

	fcb->phy_offset = mtd->writesize;

	fcb->nr_blocks = mtd->writesize / fcb->ecc_nr - 1;

	fcb->disbbm = 0;
	fcb->disbbm_search = 0;

	fcb->fw1_start = fw1_start; /* Firmware image starts on this sector */
	fcb->fw2_start = fw2_start; /* Secondary FW Image starting Sector */
	fcb->fw1_pages = fw_pages; /* Number of sectors in firmware image */
	fcb->fw2_pages = fw_pages; /* Number of sector in secondary FW image */

	fcb->checksum = calc_chksum((void *)fcb + 4, sizeof(*fcb) - 4);
}

static int dbbt_fill_data(struct mtd_info *mtd, void *buf, int num_blocks)
{
	int n, n_bad_blocks = 0;
	u32 *bb = buf + 0x8;
	u32 *n_bad_blocksp = buf + 0x4;

	for (n = 0; n < num_blocks; n++) {
		loff_t offset = n * mtd->erasesize;
			if (mtd_block_isbad(mtd, offset)) {
				n_bad_blocks++;
				*bb = n;
				bb++;
		}
	}

	*n_bad_blocksp = n_bad_blocks;

	return n_bad_blocks;
}

static int write_fcb_dbbt_and_readback(struct mtd_info *mtd,
				       struct fcb_block *fcb,
				       struct dbbt_block *dbbt,
				       void *dbbt_data_page, loff_t off)
{
	void *fcb_raw_page = 0;
	int i, ret;
	size_t dummy;

	/*
	 * We prepare raw page only for i.MX6, for i.MX7 we
	 * leverage BCH hw module instead
	 */
	if (is_mx6()) {
		/* write fcb/dbbt */
		fcb_raw_page = kzalloc(mtd->writesize + mtd->oobsize,
				       GFP_KERNEL);
		if (!fcb_raw_page) {
			debug("failed to allocate fcb_raw_page\n");
			ret = -ENOMEM;
			return ret;
		}

#if defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
		/* 40 bit BCH, for i.MX6UL(L) */
		encode_bch_ecc(fcb_raw_page + 32, fcb, 40);
#else
		memcpy(fcb_raw_page + 12, fcb, sizeof(struct fcb_block));
		encode_hamming_13_8(fcb_raw_page + 12,
				    fcb_raw_page + 12 + 512, 512);
#endif
		/*
		 * Set the first and second byte of OOB data to 0xFF,
		 * not 0x00. These bytes are used as the Manufacturers Bad
		 * Block Marker (MBBM). Since the FCB is mostly written to
		 * the first page in a block, a scan for
		 * factory bad blocks will detect these blocks as bad, e.g.
		 * when function nand_scan_bbt() is executed to build a new
		 * bad block table.
		 */
		memset(fcb_raw_page + mtd->writesize, 0xFF, 2);
	}
	for (i = 0; i < 2; i++) {
		if (mtd_block_isbad(mtd, off)) {
			printf("Block %d is bad, skipped\n", i);
			continue;
		}

		/*
		 * User BCH ECC hardware module for i.MX7
		 */
		if (is_mx7() || is_imx8m()) {
			u32 off = i * mtd->erasesize;
			size_t rwsize = sizeof(*fcb);

			printf("Writing %zd bytes to 0x%x: ", rwsize, off);

			/* switch nand BCH to FCB compatible settings */
			mxs_nand_mode_fcb(mtd);
			ret = nand_write(mtd, off, &rwsize,
					 (unsigned char *)fcb);

			dump_fcb_off[i] = off;
			nand_read(mtd, off, &rwsize,
				  (unsigned char *)(dump_nand_fcb + i));

			mxs_nand_mode_normal(mtd);

			printf("%s\n", ret ? "ERROR" : "OK");
		} else if (is_mx6()) {
			/* raw write */
			mtd_oob_ops_t ops = {
				.datbuf = (u8 *)fcb_raw_page,
				.oobbuf = ((u8 *)fcb_raw_page) +
					  mtd->writesize,
				.len = mtd->writesize,
				.ooblen = mtd->oobsize,
				.mode = MTD_OPS_RAW
			};

			ret = mtd_write_oob(mtd, mtd->erasesize * i, &ops);
			if (ret)
				goto fcb_raw_page_err;
			debug("NAND fcb write: 0x%x offset 0x%zx written: %s\n",
			      mtd->erasesize * i, ops.len, ret ?
			      "ERROR" : "OK");

			ops.datbuf = (u8 *)(dump_nand_fcb + i);
			ops.oobbuf = ((u8 *)(dump_nand_fcb + i)) + mtd->writesize;
			mtd_read_oob(mtd, mtd->erasesize * i, &ops);
		}

		ret = mtd_write(mtd, mtd->erasesize * i + mtd->writesize,
				mtd->writesize, &dummy, (void *)dbbt);
		if (ret)
			goto fcb_raw_page_err;
		debug("NAND dbbt write: 0x%x offset, 0x%zx bytes written: %s\n",
		      mtd->erasesize * i + mtd->writesize, dummy,
		      ret ? "ERROR" : "OK");

		dump_dbbt_off[i] = mtd->erasesize * i + mtd->writesize;
		size_t rwsize = sizeof(*dbbt);

		nand_read(mtd, dump_dbbt_off[i], &rwsize,
			  (unsigned char *)(dump_nand_dbbt + i));

		/* dbbtpages == 0 if no bad blocks */
		if (dbbt->dbbtpages > 0) {
			loff_t to = (mtd->erasesize * i + mtd->writesize * 5);

			ret = mtd_write(mtd, to, mtd->writesize, &dummy,
					dbbt_data_page);
			if (ret)
				goto fcb_raw_page_err;
		}
	}

fcb_raw_page_err:
	if (is_mx6())
		kfree(fcb_raw_page);

	return ret;
}

static int nandbcb_update(struct mtd_info *mtd, loff_t off, size_t size,
			  size_t maxsize, const u_char *buf)
{
	nand_erase_options_t opts;
	struct fcb_block *fcb;
	struct dbbt_block *dbbt;
	loff_t fw1_off;
	void *fwbuf, *dbbt_page, *dbbt_data_page;
	u32 fw1_start, fw1_pages;
	int nr_blks, nr_blks_fcb, fw1_blk;
	size_t fwsize;
	int ret;
	size_t extra_fwsize;
	void *extra_fwbuf;
	loff_t extra_fw1_off;

	/* erase */
	memset(&opts, 0, sizeof(opts));
	opts.offset = off;
	opts.length = maxsize - 1;
	ret = nand_erase_opts(mtd, &opts);
	if (ret) {
		printf("%s: erase failed (ret = %d)\n", __func__, ret);
		return ret;
	}

	/*
	 * Reference documentation from i.MX6DQRM section 8.5.2.2
	 *
	 * Nand Boot Control Block(BCB) contains two data structures,
	 * - Firmware Configuration Block(FCB)
	 * - Discovered Bad Block Table(DBBT)
	 *
	 * FCB contains,
	 * - nand timings
	 * - DBBT search page address,
	 * - start page address of primary firmware
	 * - start page address of secondary firmware
	 *
	 * setup fcb:
	 * - number of blocks = mtd partition size / mtd erasesize
	 * - two firmware blocks, primary and secondary
	 * - first 4 block for FCB/DBBT
	 * - rest split in half for primary and secondary firmware
	 * - same firmware will write two times
	 */
	nr_blks_fcb = BOOT_SEARCH_COUNT;
	nr_blks = maxsize / mtd->erasesize;
	fw1_blk = nr_blks_fcb;

	/* write fw */
	fwbuf = NULL;
	if (is_mx6() || is_mx7()) {
		fwsize = ALIGN(size + FLASH_OFFSET_STANDARD + mtd->writesize,
			       mtd->writesize);
		fwbuf = kzalloc(fwsize, GFP_KERNEL);
		if (!fwbuf) {
			debug("failed to allocate fwbuf\n");
			ret = -ENOMEM;
			goto err;
		}

		memcpy(fwbuf + FLASH_OFFSET_STANDARD, buf, size);
		fw1_off = fw1_blk * mtd->erasesize;
		ret = nand_write_skip_bad(mtd, fw1_off, &fwsize, NULL, maxsize,
					  (u_char *)fwbuf, WITH_WR_VERIFY);
		printf("NAND fw write: 0x%llx offset, 0x%zx bytes written: %s\n",
		       fw1_off, fwsize, ret ? "ERROR" : "OK");
		if (ret)
			goto fwbuf_err;
	} else if (is_imx8m()) {
		fwsize = ALIGN(IMX8MQ_SPL_SZ + FLASH_OFFSET_STANDARD + mtd->writesize, mtd->writesize);
		fwbuf = kzalloc(fwsize, GFP_KERNEL);
		if (!fwbuf) {
			printf("failed to allocate fwbuf\n");
			ret = -ENOMEM;
			goto err;
		}

		memcpy(fwbuf + FLASH_OFFSET_STANDARD, buf, IMX8MQ_SPL_SZ);
		fw1_off = fw1_blk * mtd->erasesize;
		ret = nand_write_skip_bad(mtd, fw1_off, &fwsize, NULL, maxsize,
					  (u_char *)fwbuf, WITH_WR_VERIFY);
		printf("NAND fw write: 0x%llx offset, 0x%zx bytes written: %s\n",
		       fw1_off, fwsize, ret ? "ERROR" : "OK");
		if (ret)
			goto fwbuf_err;

		extra_fwsize = ALIGN(IMX8MQ_SPL_SZ + mtd->writesize, mtd->writesize);
		extra_fwbuf = kzalloc(extra_fwsize, GFP_KERNEL);
		extra_fw1_off = fw1_off + mtd->erasesize * ((IMX8MQ_SPL_SZ + mtd->erasesize - 1) / mtd->erasesize);
		if (!extra_fwbuf) {
			printf("failed to allocate fwbuf\n");
			ret = -ENOMEM;
			goto fwbuf_err;
		}

		memcpy(extra_fwbuf, buf + IMX8MQ_HDMI_FW_SZ, IMX8MQ_SPL_SZ);
		ret = nand_write_skip_bad(mtd, extra_fw1_off, &extra_fwsize, NULL, maxsize,
					  (u_char *)extra_fwbuf, WITH_WR_VERIFY);
		printf("NAND extra_fw write: 0x%llx offset, 0x%zx bytes written: %s\n",
		       extra_fw1_off, extra_fwsize, ret ? "ERROR" : "OK");
		if (ret) {
			kfree(extra_fwbuf);
			goto fwbuf_err;
		}
	}

	/* fill fcb */
	fcb = kzalloc(sizeof(*fcb), GFP_KERNEL);
	if (!fcb) {
		debug("failed to allocate fcb\n");
		ret = -ENOMEM;
		goto fwbuf_err;
	}

	fw1_start = (fw1_blk * mtd->erasesize) / mtd->writesize;
	fw1_pages = size / mtd->writesize + 1;
	if (is_imx8m())
		fw1_pages = (IMX8MQ_SPL_SZ + (mtd->writesize - 1)) / mtd->writesize;
	fill_fcb(fcb, mtd, fw1_start, 0, fw1_pages);

	dump_fill_fcb = *fcb;

	/* fill dbbt */
	dbbt_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_page) {
		debug("failed to allocate dbbt_page\n");
		ret = -ENOMEM;
		goto fcb_err;
	}

	dbbt_data_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_data_page) {
		debug("failed to allocate dbbt_data_page\n");
		ret = -ENOMEM;
		goto dbbt_page_err;
	}

	dbbt = dbbt_page;
	dbbt->checksum = 0;
	dbbt->fingerprint = DBBT_FINGERPRINT2;
	dbbt->version = DBBT_VERSION_1;
	ret = dbbt_fill_data(mtd, dbbt_data_page, nr_blks);
	if (ret < 0)
		goto dbbt_data_page_err;
	else if (ret > 0)
		dbbt->dbbtpages = 1;

	dump_fill_dbbt = *dbbt;

	/* write fcb and dbbt to nand */
	ret = write_fcb_dbbt_and_readback(mtd, fcb, dbbt, dbbt_data_page, off);
	if (ret < 0)
		printf("failed to write FCB/DBBT\n");

dbbt_data_page_err:
	kfree(dbbt_data_page);
dbbt_page_err:
	kfree(dbbt_page);
fcb_err:
	kfree(fcb);
fwbuf_err:
	kfree(fwbuf);
err:
	return ret;
}

static int do_nandbcb_bcbonly(int argc, char * const argv[])
{
	struct fcb_block *fcb;
	struct dbbt_block *dbbt;
	u32 fw_len, fw1_off, fw2_off;
	struct mtd_info *mtd;
	void *dbbt_page, *dbbt_data_page;
	int dev, ret;

	dev = nand_curr_device;
	if ((dev < 0) || (dev >= CONFIG_SYS_MAX_NAND_DEVICE) ||
	    (!get_nand_dev_by_index(dev))) {
		puts("No devices available\n");
		return CMD_RET_FAILURE;
	}

	mtd = get_nand_dev_by_index(dev);

	if (argc < 3)
		return CMD_RET_FAILURE;

	fw_len = simple_strtoul(argv[1], NULL, 16);
	fw1_off = simple_strtoul(argv[2], NULL, 16);

	if (argc > 3)
		fw2_off = simple_strtoul(argv[3], NULL, 16);
	else
		fw2_off = fw1_off;

	/* fill fcb */
	fcb = kzalloc(sizeof(*fcb), GFP_KERNEL);
	if (!fcb) {
		debug("failed to allocate fcb\n");
		ret = -ENOMEM;
		return CMD_RET_FAILURE;
	}

	fill_fcb(fcb, mtd, fw1_off / mtd->writesize,
		 fw2_off / mtd->writesize, fw_len / mtd->writesize);

	/* fill dbbt */
	dbbt_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_page) {
		debug("failed to allocate dbbt_page\n");
		ret = -ENOMEM;
		goto fcb_err;
	}

	dbbt_data_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_data_page) {
		debug("failed to allocate dbbt_data_page\n");
		ret = -ENOMEM;
		goto dbbt_page_err;
	}

	dbbt = dbbt_page;
	dbbt->checksum = 0;
	dbbt->fingerprint = DBBT_FINGERPRINT2;
	dbbt->version = DBBT_VERSION_1;
	ret = dbbt_fill_data(mtd, dbbt_data_page, 0);
	if (ret < 0)
		goto dbbt_data_page_err;
	else if (ret > 0)
		dbbt->dbbtpages = 1;

	/* write fcb and dbbt to nand */
	ret = write_fcb_dbbt_and_readback(mtd, fcb, dbbt, dbbt_data_page, 0);
dbbt_data_page_err:
	kfree(dbbt_data_page);
dbbt_page_err:
	kfree(dbbt_page);
fcb_err:
	kfree(fcb);

	if (ret < 0) {
		printf("failed to write FCB/DBBT\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

/* dump data which is planned to be encoded and written to NAND chip */
void mtd_cfg_dump(void)
{
	u64 blocks;

	printf("MTD CONFIG:\n");
	printf("  %s = %d\n", "data_setup_time", dump_fill_fcb.datasetup);
	printf("  %s = %d\n", "data_hold_time", dump_fill_fcb.datahold);
	printf("  %s = %d\n", "address_setup_time", dump_fill_fcb.addr_setup);
	printf("  %s = %d\n", "data_sample_time", dump_fill_fcb.dsample_time);

	printf("NFC geometry :\n");
	printf("\tECC Strength       : %d\n", dump_mtd->ecc_strength);
	printf("\tPage Size in Bytes : %d\n", dump_fill_fcb.oob_pagesize);
	printf("\tMetadata size      : %d\n", dump_fill_fcb.meta_size);
	printf("\tECC Chunk Size in byte : %d\n", dump_fill_fcb.ecc_size);
	printf("\tECC Chunk count        : %d\n", dump_fill_fcb.nr_blocks + 1);
	printf("\tBlock Mark Byte Offset : %d\n", dump_fill_fcb.bb_byte);
	printf("\tBlock Mark Bit Offset  : %d\n", dump_fill_fcb.bb_start_bit);
	printf("====================================================\n");

	printf("mtd: partition #0\n");
	printf("  %s = %d\n", "type", dump_mtd->type);
	printf("  %s = %d\n", "flags", dump_mtd->flags);
	printf("  %s = %llu\n", "size", dump_nandboot_size);
	printf("  %s = %d\n", "erasesize", dump_mtd->erasesize);
	printf("  %s = %d\n", "writesize", dump_mtd->writesize);
	printf("  %s = %d\n", "oobsize", dump_mtd->oobsize);
	blocks = dump_nandboot_size;
	do_div(blocks, dump_mtd->erasesize);
	printf("  %s = %llu\n", "blocks", blocks);
}

/* dump data which is read from NAND chip */
void mtd_dump_structure(int i)
{
	#define P1(x) printf("  %s = 0x%08x\n", #x, dump_nand_fcb[i].x)
		printf("FCB %d:\n", i);
		P1(checksum);
		P1(fingerprint);
		P1(version);
	#undef P1
	#define P1(x)	printf("  %s = %d\n", #x, dump_nand_fcb[i].x)
		P1(datasetup);
		P1(datahold);
		P1(addr_setup);
		P1(dsample_time);
		P1(pagesize);
		P1(oob_pagesize);
		P1(sectors);
		P1(nr_nand);
		P1(nr_die);
		P1(celltype);
		P1(ecc_type);
		P1(ecc_nr);
		P1(ecc_size);
		P1(ecc_level);
		P1(meta_size);
		P1(nr_blocks);
		P1(ecc_type_sdk);
		P1(ecc_nr_sdk);
		P1(ecc_size_sdk);
		P1(ecc_level_sdk);
		P1(nr_blocks_sdk);
		P1(meta_size_sdk);
		P1(erase_th);
		P1(bootpatch);
		P1(patch_size);
		P1(fw1_start);
		P1(fw2_start);
		P1(fw1_pages);
		P1(fw2_pages);
		P1(dbbt_start);
		P1(bb_byte);
		P1(bb_start_bit);
		P1(phy_offset);
		P1(bchtype);
		P1(readlatency);
		P1(predelay);
		P1(cedelay);
		P1(postdelay);
		P1(cmdaddpause);
		P1(datapause);
		P1(tmspeed);
		P1(busytimeout);
		P1(disbbm);
		P1(spare_offset);
		P1(onfi_sync_enable);
		P1(onfi_sync_speed);
		P1(onfi_sync_nand_data);
		P1(disbbm_search);
		P1(disbbm_search_limit);
		P1(read_retry_enable);
	#undef P1
	#define P1(x)	printf("  %s = 0x%08x\n", #x, dump_nand_dbbt[i].x)
		printf("DBBT %d:\n", i);
		P1(checksum);
		P1(fingerprint);
		P1(version);
	#undef P1
	#define P1(x)	printf("  %s = %d\n", #x, dump_nand_dbbt[i].x)
		P1(numberbb);
	#undef P1

	printf("Firmware: image #0 @ 0x%x size 0x%x - available 0x%llx\n",
	       dump_nand_fcb[i].fw1_start * dump_nand_fcb[i].pagesize,
	       dump_nand_fcb[i].fw1_pages * dump_nand_fcb[i].pagesize,
	       dump_nandboot_size - dump_nand_fcb[i].fw1_start *
	       dump_nand_fcb[i].pagesize);
	if (is_imx8m()) {
		printf("Extra Firmware: image #0 @ 0x%x size 0x%x - available 0x%llx\n",
		       dump_nand_fcb[i].fw1_start *
		       dump_nand_fcb[i].pagesize + dump_mtd->erasesize *
		       ((IMX8MQ_SPL_SZ + dump_mtd->erasesize - 1) /
			dump_mtd->erasesize),
		       dump_nand_fcb[i].fw1_pages * dump_nand_fcb[i].pagesize,
		       dump_nandboot_size -
		       (dump_nand_fcb[i].fw1_start *
			dump_nand_fcb[i].pagesize + dump_mtd->erasesize *
			((IMX8MQ_SPL_SZ + dump_mtd->erasesize - 1) /
			 dump_mtd->erasesize)));
	}
}

static int do_nandbcb_dump(int argc, char * const argv[])
{
	int num;
	int stride;
	int search_area_sz;
	bool bab_block_table[BOOT_SEARCH_COUNT];
	int bab_block_flag;

	if (argc != 2)
		return CMD_RET_USAGE;

	switch (argv[1][0]) {
	case '0':
		num = 0;
		break;
	case '1':
		num = 1;
		break;
	default:
		return CMD_RET_USAGE;
	}

	/* dump data which is planned to be encoded and written to NAND chip */
	mtd_cfg_dump();

	stride = dump_mtd->erasesize;
	search_area_sz = BOOT_SEARCH_COUNT * stride;
	printf("stride: %x, search_area_sz: %x\n", stride, search_area_sz);

	bab_block_flag = 0;
	for (int i = 0; i < BOOT_SEARCH_COUNT; i++) {
		if (mtd_block_isbad(dump_mtd,
				    (loff_t)(dump_mtd->erasesize * i))) {
			bab_block_table[i] = 1;
			bab_block_flag = 1;
			continue;
		}
		bab_block_table[i] = 0;
		if (!memcmp(dump_nand_fcb + i, &dump_fill_fcb,
			    sizeof(dump_fill_fcb))) {
			printf("mtd: found FCB%d candidate version %08x @%d:0x%x\n",
			       i, dump_nand_fcb[i].version, i, dump_fcb_off[i]);
		} else {
			printf("mtd: FCB%d not found\n", i);
		}
	}

	for (int i = 0; i < BOOT_SEARCH_COUNT; i++) {
		if (mtd_block_isbad(dump_mtd,
				    (loff_t)(dump_mtd->erasesize * i)))
			continue;

		if (!memcmp(dump_nand_dbbt + i, &dump_fill_dbbt,
			    sizeof(dump_fill_dbbt))) {
			printf("mtd: DBBT%d found\n", i);
			printf("mtd: Valid DBBT%d found @%d:0x%x\n",
			       i, i, dump_dbbt_off[i]);

		} else {
			printf("mtd: DBBT%d not found\n", i);
		}
	}
	if (bab_block_flag == 0) {
		printf("no bad block found, dbbt: %08x\n",
		       dump_fill_dbbt.fingerprint);
	} else {
		for (int i = 0; i < BOOT_SEARCH_COUNT; i++) {
			if (bab_block_table[i] == 1)
				printf("mtd: bad block @ 0x%llx\n",
				       (loff_t)(dump_mtd->erasesize * i));
		}
	}

	/* dump data which is read from NAND chip */
	if (num > (BOOT_SEARCH_COUNT - 1))
		return CMD_RET_USAGE;

	if (bab_block_table[num] == 1) {
		printf("mtd: bad block @ 0x%llx (FCB - DBBT)\n",
		       (loff_t)(dump_mtd->erasesize * num));
		return CMD_RET_USAGE;
	}

	mtd_dump_structure(num);

	return 0;
}

static int do_nandbcb_update(int argc, char * const argv[])
{
	struct mtd_info *mtd;
	loff_t addr, offset, size, maxsize;
	char *endp;
	u_char *buf;
	int dev;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	dev = nand_curr_device;
	if (dev < 0) {
		printf("failed to get nand_curr_device, run nand device\n");
		return CMD_RET_FAILURE;
	}

	addr = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return CMD_RET_FAILURE;

	mtd = get_nand_dev_by_index(dev);
	if (mtd_arg_off_size(argc - 2, argv + 2, &dev, &offset, &size,
			     &maxsize, MTD_DEV_TYPE_NAND, mtd->size))
		return CMD_RET_FAILURE;

	/* dump_mtd and dump_nandboot_size are used for "nandbcb dump [-v]" */
	dump_mtd = mtd;
	dump_nandboot_size = maxsize;

	buf = map_physmem(addr, size, MAP_WRBACK);
	if (!buf) {
		puts("failed to map physical memory\n");
		return CMD_RET_FAILURE;
	}

	ret = nandbcb_update(mtd, offset, size, maxsize, buf);

	return ret == 0 ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

static int do_nandbcb(cmd_tbl_t *cmdtp, int flag, int argc,
		      char * const argv[])
{
	const char *cmd;
	int ret = 0;

	if (argc < 3)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "update") == 0) {
		ret = do_nandbcb_update(argc, argv);
		goto done;
	}

	if (strcmp(cmd, "dump") == 0) {
		ret = do_nandbcb_dump(argc, argv);
		goto done;
	}

	if (strcmp(cmd, "bcbonly") == 0) {
		ret = do_nandbcb_bcbonly(argc, argv);
		goto done;
	}

done:
	if (ret != -1)
		return ret;
usage:
	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char nandbcb_help_text[] =
	"update addr off|partition len	- update 'len' bytes starting at\n"
	"       'off|part' to memory address 'addr', skipping  bad blocks\n"
	"bcbonly fw-size fw1-off [fw2-off] - write only BCB (FCB and DBBT)\n"
	"       where `fw-size` is fw sizes in bytes, `fw1-off`\n"
	"       and `fw2-off` - firmware offsets\n"
	"       FIY, BCB isn't erased automatically, so mtd erase should\n"
	"       be called in advance before writing new BCB:\n"
	"           > mtd erase mx7-bcb\n"
	"nandbcb dump num - verify/dump boot structures\n"
	"	'num' can be set to 0 and 1";
#endif

U_BOOT_CMD(nandbcb, 5, 1, do_nandbcb,
	   "i.MX6/i.MX7 NAND Boot Control Blocks write",
	   nandbcb_help_text
);
