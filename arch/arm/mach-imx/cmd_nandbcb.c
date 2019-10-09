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
#include <nand.h>

#include <asm/io.h>
#include <jffs2/jffs2.h>
#include <linux/mtd/mtd.h>

#include <asm/mach-imx/imx-nandbcb.h>
#include <asm/mach-imx/imximage.cfg>
#include <mxs_nand.h>
#include <linux/mtd/mtd.h>
#include <nand.h>

#define BF_VAL(v, bf)		(((v) & bf##_MASK) >> bf##_OFFSET)
#define GETBIT(v, n)		(((v) >> (n)) & 0x1)

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

static u32 calc_chksum(void *buf, size_t size)
{
	u32 chksum = 0;
	u8 *bp = buf;
	size_t i;

	for (i = 0; i < size; i++)
		chksum += bp[i];

	return ~chksum;
}

static void fill_fcb(struct fcb_block *fcb, struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);

	fcb->fingerprint = FCB_FINGERPRINT;
	fcb->version = FCB_VERSION_1;
	fcb->pagesize = mtd->writesize;
	fcb->oob_pagesize = mtd->writesize + mtd->oobsize;
	fcb->sectors = mtd->erasesize / mtd->writesize;

	/* Divide ECC strength by two and save the value into FCB structure. */
	fcb->ecc_level = nand_info->bch_geometry.ecc_strength >> 1;

	fcb->ecc_type = fcb->ecc_level;

	/* Also hardcoded in kobs-ng */
	fcb->ecc_nr = 0x00000200;
	fcb->ecc_size = 0x00000200;
	fcb->datasetup = 80;
	fcb->datahold = 60;
	fcb->addr_setup = 25;
	fcb->dsample_time = 6;
	fcb->meta_size = 10;

	/* DBBT search area starts at second page on first block */
	fcb->dbbt_start = 1;

	fcb->bb_byte = nand_info->bch_geometry.block_mark_byte_offset;
	fcb->bb_start_bit = nand_info->bch_geometry.block_mark_bit_offset;

	fcb->phy_offset = mtd->writesize;

	fcb->nr_blocks = mtd->writesize / fcb->ecc_nr - 1;

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

static int nandbcb_update(struct mtd_info *mtd, loff_t off, size_t size,
			  size_t maxsize, const u_char *buf)
{
	nand_erase_options_t opts;
	struct fcb_block *fcb;
	struct dbbt_block *dbbt;
	loff_t fw1_off;
	void *fwbuf, *fcb_raw_page, *dbbt_page, *dbbt_data_page;
	int nr_blks, nr_blks_fcb, fw1_blk;
	size_t fwsize, dummy;
	int i, ret;

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
	nr_blks_fcb = 2;
	nr_blks = maxsize / mtd->erasesize;
	fw1_blk = nr_blks_fcb;

	/* write fw */
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
	printf("NAND fw write: 0x%llx offset, 0x%x bytes written: %s\n",
	       fw1_off, fwsize, ret ? "ERROR" : "OK");
	if (ret)
		goto fwbuf_err;

	/* fill fcb */
	fcb = kzalloc(sizeof(*fcb), GFP_KERNEL);
	if (!fcb) {
		debug("failed to allocate fcb\n");
		ret = -ENOMEM;
		goto fwbuf_err;
	}

	fcb->fw1_start = (fw1_blk * mtd->erasesize) / mtd->writesize;
	fcb->fw1_pages = size / mtd->writesize + 1;
	fill_fcb(fcb, mtd);

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

	/* write fcb/dbbt */
	fcb_raw_page = kzalloc(mtd->writesize + mtd->oobsize, GFP_KERNEL);
	if (!fcb_raw_page) {
		debug("failed to allocate fcb_raw_page\n");
		ret = -ENOMEM;
		goto dbbt_data_page_err;
	}

	memcpy(fcb_raw_page + 12, fcb, sizeof(struct fcb_block));
	encode_hamming_13_8(fcb_raw_page + 12, fcb_raw_page + 12 + 512, 512);
	/*
	 * Set the first and second byte of OOB data to 0xFF, not 0x00. These
	 * bytes are used as the Manufacturers Bad Block Marker (MBBM). Since
	 * the FCB is mostly written to the first page in a block, a scan for
	 * factory bad blocks will detect these blocks as bad, e.g. when
	 * function nand_scan_bbt() is executed to build a new bad block table.
	 */
	memset(fcb_raw_page + mtd->writesize, 0xFF, 2);

	for (i = 0; i < nr_blks_fcb; i++) {
		if (mtd_block_isbad(mtd, off)) {
			printf("Block %d is bad, skipped\n", i);
			continue;
		}

		/* raw write */
		mtd_oob_ops_t ops = {
			.datbuf = (u8 *)fcb_raw_page,
			.oobbuf = ((u8 *)fcb_raw_page) + mtd->writesize,
			.len = mtd->writesize,
			.ooblen = mtd->oobsize,
			.mode = MTD_OPS_RAW
		};

		ret = mtd_write_oob(mtd, mtd->erasesize * i, &ops);
		if (ret)
			goto fcb_raw_page_err;
		debug("NAND fcb write: 0x%x offset, 0x%x bytes written: %s\n",
		      mtd->erasesize * i, ops.len, ret ? "ERROR" : "OK");

		ret = mtd_write(mtd, mtd->erasesize * i + mtd->writesize,
				mtd->writesize, &dummy, dbbt_page);
		if (ret)
			goto fcb_raw_page_err;
		debug("NAND dbbt write: 0x%x offset, 0x%x bytes written: %s\n",
		      mtd->erasesize * i + mtd->writesize, dummy,
		      ret ? "ERROR" : "OK");

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
	kfree(fcb_raw_page);
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
		printf("failed to get nand_curr_device, run nand device");
		return CMD_RET_FAILURE;
	}

	addr = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return CMD_RET_FAILURE;

	mtd = get_nand_dev_by_index(dev);
	if (mtd_arg_off_size(argc - 2, argv + 2, &dev, &offset, &size,
			     &maxsize, MTD_DEV_TYPE_NAND, mtd->size))
		return CMD_RET_FAILURE;

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

	if (argc < 5)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "update") == 0) {
		ret = do_nandbcb_update(argc, argv);
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
	"	'off|part' to memory address 'addr', skipping  bad blocks";
#endif

U_BOOT_CMD(nandbcb, 5, 1, do_nandbcb,
	   "i.MX6 Nand BCB",
	   nandbcb_help_text
);
