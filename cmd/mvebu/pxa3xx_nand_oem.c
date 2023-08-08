/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <linux/mtd/mtd.h>
#include <command.h>
#include <nand.h>
#include <pxa3xx_nand.h>

#define NDCR_SPARE_EN		(0x1 << 31)

static char nand_oem_help_text[] =
	"use \"nand_oem prepare m-offs m-size p-offs p-size\" to prepare oem image\n"
	"use \"nand_oem restore m-offs m-size\" to restore original data\n"
	"\tm-offs - meta-data partition offset in bytes\n"
	"\tm-size - meta-data partition size in bytes\n"
	"\tp-offs - device data partition offset in bytes\n"
	"\tp-size - device data partition size in bytes\n"
	"";

	/*
	 * Consider to add optional parameter to get a type of non-empty/-blank
	 * block to skip such a block during "prepare" stage.
	 */

#define AUX_TBL_MAGIC_LEN	8
struct aux_tbl_header {
	char magic[AUX_TBL_MAGIC_LEN];
	loff_t off;
	loff_t sz;
	size_t blk_cnt;
};

enum blank_block_type {
	NAND_EMPTY_BLOCK,
	NAND_UBI_BLANK_BLOCK
};

static int is_block_blank(const uchar *nand_blk, const uchar *blank_blk,
			  size_t blk_sz, enum blank_block_type type)
{
	int rval = 0;

	switch (type) {
	case NAND_EMPTY_BLOCK:
		if (!memcmp(nand_blk, blank_blk, blk_sz))
			rval = 1;
		break;
	case NAND_UBI_BLANK_BLOCK:
		/* add ubi blank block skipping logic here */
		break;
	default:
		printf("found unsupported blank block type\n");
	}

	return rval;
}

static int nand_prepare(struct mtd_info *nand, char * const argv[],
			loff_t meta_off, loff_t meta_sz, size_t bb_pos,
			loff_t bb_pg_off)
{
	struct aux_tbl_header *aux_tbl_hdr_rd, aux_tbl_hdr = {
		.magic = {'l', 'l', 'e', 'v', 'r', 'a', 'm', '@'},
		.off = 0,
		.sz = 0,
		.blk_cnt = 0
	};
	size_t rw_sz;
	size_t pg_sz = nand->writesize;
	size_t blk_sz = nand->erasesize;
	size_t aux_tbl_idx = 0;
	size_t aux_tbl_delta = 0;
	size_t aux_tbl_pg = 0;
	loff_t off, part_off = 0;
	loff_t part_sz = 0;
	loff_t aux_tbl_off = -1;
	u_char *aux_tbl, *rw_blk, *r_pg, *e_blk;
	nand_erase_options_t opts;
	int ret = 1;

	/* allocate memory for an empty block */
	e_blk = memalign(ARCH_DMA_MINALIGN, blk_sz);
	/* initialize the empty block */
	memset(e_blk, 0xff, blk_sz);
	/* allocate memory for page handling */
	r_pg = memalign(ARCH_DMA_MINALIGN, pg_sz);
	/* allocate memory for block handling */
	rw_blk = memalign(ARCH_DMA_MINALIGN, blk_sz);
	/* allocate memory for oem image meta-data handling */
	aux_tbl = memalign(ARCH_DMA_MINALIGN, blk_sz);
	/* initialize the meta-data */
	memset(aux_tbl, 0xff, blk_sz);

	/* device data partition size in bytes */
	part_sz = simple_strtoul(argv[5], NULL, 0);
	if (part_sz >= nand->size || part_sz < blk_sz) {
		printf("%s: incorrect device data partition size: 0x%llx\n",
		       __func__, part_sz);
		goto oem_prepare_fail;
	}
	aux_tbl_hdr.sz = part_sz;

	/* device data partition offset in bytes */
	part_off = simple_strtoul(argv[4], NULL, 0);
	if (part_off + part_sz >= nand->size) {
		printf("%s: partition exceeding nand size: 0x%llx\n", __func__,
		       part_off + part_sz);
		goto oem_prepare_fail;
	}
	if ((u64)part_off % blk_sz) {
		printf("%s: partition offset should be block-aligned\n",
		       __func__);
		goto oem_prepare_fail;
	}
	aux_tbl_hdr.off = part_off;

	printf("device data partition offset = %llu bytes\n", part_off);
	printf("device data partition size = %llu bytes\n", part_sz);
	printf("device data partition blocks number = %llu blocks\n",
	       ((u64)part_sz - 1) / blk_sz + 1);

	/*
	 * Consider to add optional parameter to get a type of non-empty/-blank
	 * block to skip such a block during "prepare" stage.
	 */

	/* pass through meta-data partition in nand */
	for (off = meta_off; off < meta_off + meta_sz; off += blk_sz) {
		/* skip bad blocks */
		if (nand_block_isbad(nand, off))
			continue;

		/* read the first page */
		rw_sz = pg_sz;
		if (nand_read_skip_bad(nand, off, &rw_sz, NULL, pg_sz, r_pg)) {
			printf("failed to read page at offset 0x%llx\n", off);
			goto oem_prepare_fail;
		}

		/* cast the meta-data header */
		aux_tbl_hdr_rd = (struct aux_tbl_header *)r_pg;

		/* look for the meta-data signature */
		if (!strncmp(aux_tbl_hdr_rd->magic, aux_tbl_hdr.magic,
			     sizeof(aux_tbl_hdr.magic))) {
			/* found allocated meta-data block */
			/* check for overlapping partitions */
			if ((part_off < aux_tbl_hdr_rd->off &&
			     part_off + part_sz <= aux_tbl_hdr_rd->off) ||
			    (part_off >= aux_tbl_hdr_rd->off +
					 aux_tbl_hdr_rd->sz &&
			     part_off + part_sz > aux_tbl_hdr_rd->off +
						  aux_tbl_hdr_rd->sz)) {
				/* no overlapping; continue looking for free
				 * meta-data block
				 */
				continue;
			} else { /* overlapping */
				printf("%s: partition overlaps at off 0x%llx\n",
				       __func__, part_off);
				goto oem_prepare_fail;
			}
		} else { /* found free meta-data block */
			aux_tbl_off = off;
			break;
		} /* free or allocated block? */
	}  /* partition's passthrough */

	if (aux_tbl_off == -1) {
		printf("free meta-data block wasn't found\n");
		goto oem_prepare_fail;
	}

	/* pass through device data partition in nand */
	for (off = part_off; off < part_off + part_sz; off += blk_sz) {
		/* skip bad blocks */
		if (nand_block_isbad(nand, off))
			continue;

		/* found a good block; read it */
		rw_sz = blk_sz;
		if (nand_read_skip_bad(nand, off, &rw_sz, NULL, blk_sz,
				       rw_blk)) {
			printf("failed to read block at offset 0x%llx\n", off);
			goto oem_prepare_fail;
		}

		/* skip empty block */
		if (is_block_blank(rw_blk, e_blk, blk_sz, NAND_EMPTY_BLOCK))
			continue;

		/* skip ubi blank block */
		if (is_block_blank(rw_blk, e_blk, blk_sz, NAND_UBI_BLANK_BLOCK))
			continue;

		/* check two bytes at bbm locations in the bbm page */
		if (rw_blk[bb_pg_off + bb_pos] != 0xff ||
		    rw_blk[bb_pg_off + bb_pos + 1] != 0xff) {
			/* found the bbm to mask in a good block */
			printf("found falsy bad block marks (0x%x), (0x%x) ",
			       rw_blk[bb_pg_off + bb_pos],
			       rw_blk[bb_pg_off + bb_pos + 1]);
			printf("in block[%llu]\n", ((u64)off / blk_sz));

			/* Save the two bbm bytes in the buffer with the
			 * meta-data
			 */
			aux_tbl_idx = aux_tbl_hdr.blk_cnt  * 2 +
				      sizeof(aux_tbl_hdr);
			aux_tbl_pg = (aux_tbl_idx + aux_tbl_delta) / pg_sz;
			/* calculate delta to skip bbm location in meta-data */
			if (aux_tbl_idx && !((aux_tbl_idx + aux_tbl_delta) %
					     (aux_tbl_pg * pg_sz + bb_pos)))
				aux_tbl_delta += 2;
			aux_tbl[aux_tbl_idx + aux_tbl_delta] =
						     rw_blk[bb_pg_off + bb_pos];
			aux_tbl[aux_tbl_idx + aux_tbl_delta + 1] =
						 rw_blk[bb_pg_off + bb_pos + 1];

			/* mask the data indicating the bad block mark */
			rw_blk[bb_pg_off + bb_pos] = 0xff;
			rw_blk[bb_pg_off + bb_pos + 1] = 0xff;

			/* erase the block to write its modified version */
			memset(&opts, 0, sizeof(opts));
			opts.offset = off;
			opts.length = blk_sz;
			opts.quiet  = 1;
			if (nand_erase_opts(nand, &opts)) {
				printf("failed to erase block at off 0x%llx\n",
				       off);
				goto oem_prepare_fail;
			}

			/* write the modified block to nand */
			rw_sz = blk_sz;
			if (nand_write_skip_bad(nand, off, &rw_sz, NULL,
						blk_sz, rw_blk, 0)) {
				printf("failed to write block to off 0x%llx\n",
				       off);
				goto oem_prepare_fail;
			}
		} /* falsy bb mark? */

		/*
		 * update the meta-data header for the found good
		 * non-empty/-blank block
		 */
		aux_tbl_hdr.blk_cnt++;
	} /* partition's passthrough */

	/* erase the block to write the meta-data */
	memset(&opts, 0, sizeof(opts));
	opts.offset = aux_tbl_off;
	opts.length = blk_sz;
	opts.quiet  = 1;
	if (nand_erase_opts(nand, &opts)) {
		printf("failed to erase block at offset 0x%llx\n", aux_tbl_off);
		goto oem_prepare_fail;
	}

	/* copy the meta-data signature to the buffer */
	memcpy(aux_tbl, &aux_tbl_hdr, sizeof(aux_tbl_hdr));

	/* write the buffer with the meta-data to nand */
	rw_sz = blk_sz;
	if (nand_write_skip_bad(nand, aux_tbl_off, &rw_sz, NULL, blk_sz,
				aux_tbl, 0)) {
		printf("failed to write block to offset 0x%llx\n", aux_tbl_off);
		goto oem_prepare_fail;
	}

	ret = 0;

oem_prepare_fail:
	free(e_blk);
	free(aux_tbl);
	free(r_pg);
	free(rw_blk);

	if (ret == 0)
		printf("\"nand oem\" command completed successfully\n");
	else
		printf("\"nand oem\" command failed\n");

	return ret;
}

static int nand_restore(struct mtd_info *nand, char * const argv[],
			loff_t meta_off, loff_t meta_sz, size_t bb_pos,
			loff_t bb_pg_off)
{
	struct aux_tbl_header *aux_tbl_hdr_rd, aux_tbl_hdr = {
		.magic = {'l', 'l', 'e', 'v', 'r', 'a', 'm', '@'},
		.off = 0,
		.sz = 0,
		.blk_cnt = 0
	};
	size_t rw_sz;
	size_t pg_sz = nand->writesize;
	size_t blk_sz = nand->erasesize;
	size_t gd_blk_cnt = 0;
	size_t aux_tbl_idx = 0;
	size_t aux_tbl_delta = 0;
	size_t aux_tbl_pg = 0;
	loff_t off, aux_tbl_off = -1;
	u_char *aux_tbl, *rw_blk, *r_pg;
	nand_erase_options_t opts;
	int ret = 1;

	/* allocate memory for page handling */
	r_pg = memalign(ARCH_DMA_MINALIGN, pg_sz);
	/* allocate memory for block handling */
	rw_blk = memalign(ARCH_DMA_MINALIGN, blk_sz);
	/* allocate memory for oem image meta-data handling */
	aux_tbl = memalign(ARCH_DMA_MINALIGN, blk_sz);
	/* initialize the meta-data */
	memset(aux_tbl, 0xff, blk_sz);

	/* pass through meta-data partition in nand */
	for (off = meta_off; off < meta_off + meta_sz; off += blk_sz) {
		/* skip bad blocks */
		if (nand_block_isbad(nand, off))
			continue;
		/* read the first page */
		rw_sz = pg_sz;
		if (nand_read_skip_bad(nand, off, &rw_sz, NULL, pg_sz, r_pg)) {
			printf("failed to read page at offset 0x%llx\n", off);
			goto oem_restore_fail;
		}

		/* cast the meta-data header */
		aux_tbl_hdr_rd = (struct aux_tbl_header *)r_pg;

		/* look for the meta-data signature */
		if (!strncmp(aux_tbl_hdr_rd->magic, aux_tbl_hdr.magic,
			     sizeof(aux_tbl_hdr.magic))) {
			aux_tbl_off = off;
			break;
		}
		/* continue looking for allocated meta-data block */
	}  /* partition's passthrough */

	if (aux_tbl_off == -1) {
		printf("allocated meta-data block wasn't found\n");
		goto oem_restore_fail;
	}

	/* the meta-data block is found; read it */
	rw_sz = blk_sz;
	if (nand_read_skip_bad(nand, aux_tbl_off, &rw_sz, NULL, blk_sz,
			       rw_blk)) {
		printf("failed to read block at offset 0x%llx\n", off);
		goto oem_restore_fail;
	}

	/* copy the meta-data to the buffer */
	memcpy(aux_tbl, rw_blk, blk_sz);

	/* cast the meta-data header */
	aux_tbl_hdr_rd = (struct aux_tbl_header *)aux_tbl;

	/* pass through device data partition in nand */
	for (off = aux_tbl_hdr_rd->off;
	     off < aux_tbl_hdr_rd->off + aux_tbl_hdr_rd->sz; off += blk_sz) {
		/* skip bad blocks */
		if (nand_block_isbad(nand, off))
			continue;

		/* check if restore is completed */
		if (gd_blk_cnt >= aux_tbl_hdr_rd->blk_cnt) {
			printf("scanned %zu partition blocks",
			       aux_tbl_hdr_rd->blk_cnt);
			break;
		}
		/* a good block is found; check if to restore */
		aux_tbl_idx = gd_blk_cnt * 2 + sizeof(struct aux_tbl_header);
		aux_tbl_pg = (aux_tbl_idx + aux_tbl_delta) / pg_sz;
		/* calculate delta to skip bbm location in meta-data */
		if (aux_tbl_idx && !((aux_tbl_idx + aux_tbl_delta) %
				     (aux_tbl_pg * pg_sz + bb_pos)))
			aux_tbl_delta += 2;
		if (aux_tbl[aux_tbl_idx + aux_tbl_delta] != 0xff ||
		    aux_tbl[aux_tbl_idx + aux_tbl_delta + 1] != 0xff) {
			/* read the entire block to be restored */
			rw_sz = blk_sz;
			if (nand_read_skip_bad(nand, off, &rw_sz, NULL, blk_sz,
					       rw_blk)) {
				printf("failed to read block at off 0x%llx\n",
				       off);
				goto oem_restore_fail;
			}

			/* simple check prior to restore */
			if (rw_blk[bb_pg_off + bb_pos] != 0xff ||
			    rw_blk[bb_pg_off + bb_pos + 1] != 0xff) {
				printf("bb marks: expected 0xff, 0xff, ");
				printf("got 0x%x, 0x%x at offset 0x%llx\n",
				       rw_blk[bb_pg_off + bb_pos],
				       rw_blk[bb_pg_off + bb_pos + 1],
				       off + bb_pg_off + bb_pos);
				goto oem_restore_fail;
			}

			/* restore the original data at the bbm position */
			rw_blk[bb_pg_off + bb_pos] =
					   aux_tbl[aux_tbl_idx + aux_tbl_delta];
			rw_blk[bb_pg_off + bb_pos + 1] =
				       aux_tbl[aux_tbl_idx + 1 + aux_tbl_delta];

			printf("restored data %x at pos %llu in block[%llu]\n",
			       rw_blk[bb_pg_off + bb_pos],
			       bb_pg_off + bb_pos,
			       ((u64)off / blk_sz));
			printf("restored data %x at pos %llu in block[%llu]\n",
			       rw_blk[bb_pg_off + bb_pos + 1],
			       bb_pg_off + bb_pos + 1,
			       ((u64)off / blk_sz));

			/* erase the block to write its original version */
			memset(&opts, 0, sizeof(opts));
			opts.offset = off;
			opts.length = blk_sz;
			opts.quiet  = 1;
			if (nand_erase_opts(nand, &opts)) {
				printf("failed to erase block at off 0x%llx\n",
				       off);
				goto oem_restore_fail;
			}

			/* write the restored block to nand */
			rw_sz = blk_sz;
			if (nand_write_skip_bad(nand, off, &rw_sz, NULL, blk_sz,
						rw_blk, 0)) {
				printf("failed to write block to off 0x%llx\n",
				       off);
				goto oem_restore_fail;
			}
		} /* to restore? */
		gd_blk_cnt++;
	} /* partition's passthrough */

	/* erase the block with the meta-data */
	memset(&opts, 0, sizeof(opts));
	opts.offset = aux_tbl_off;
	opts.length = blk_sz;
	opts.quiet  = 1;
	if (nand_erase_opts(nand, &opts)) {
		printf("failed to erase block at offset 0x%llx\n", aux_tbl_off);
		goto oem_restore_fail;
	}

	ret = 0;

oem_restore_fail:
	free(aux_tbl);
	free(r_pg);
	free(rw_blk);

	if (ret == 0)
		printf("\"nand oem\" command completed successfully\n");
	else
		printf("\"nand oem\" command failed\n");

	return ret;
}

static int do_nand_oem(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	int dev = nand_curr_device;
	struct mtd_info *nand = get_nand_dev_by_index(dev);
	char *cmd = argv[1];
	struct nand_chip *chip = mtd_to_nand(nand);
	struct pxa3xx_nand_host *host = nand_get_controller_data(chip);
	struct pxa3xx_nand_info *info = host->info_data;
	size_t pg_sz = nand->writesize;
	size_t blk_sz = nand->erasesize;
	size_t bb_pos;
	loff_t bb_pg_off, meta_off, meta_sz;
	unsigned int spare_size = 0;

	/*
	 * Since the pxa nfc uses different page layout (logical layout) than
	 * chip has (physical layout), the bad block marker logical offset is
	 * different than physical one and need to be calculated. It is
	 * different when spare area (NDCR_SPARE_EN) is enabled.
	 */
	if (info->reg_ndcr & (NDCR_SPARE_EN))
		spare_size = info->spare_size;

	bb_pos = pg_sz - ((pg_sz / info->chunk_size - 1) * (30 + spare_size));

	/*
	 * Bad block marking page can be placed on first, second or last
	 * page, depending on the manufacturer. Calculate the offset
	 * based on proper flag set by generic nand_decode_bbm_options.
	 *
	 * It seems that for some nand chip id. u-boot may have
	 * incorrect information since different patterns are used by
	 * one vendor.  E.g. in Toshiba chips it can be stored in first,
	 * second or last (0x98DE948276560420 it is "first or last"),
	 * while currently u-boot never set NAND_BBT_SCANLASTPAGE flag
	 * for THOSIBA maf_id. More info can be found in:
	 * http://www.linux-mtd.infradead.org/nand-data/nanddata.html
	 *
	 * In case of any problems the chip->badblockpos should be
	 * fixed. Other solution is to align with new Linux framework
	 * which registers nand_manufacturer_ops for each manufacturer,
	 * where bbt_options (_SCANLASTPAGE, _SCAN2NDPAGE) is determined
	 * in per chip .init
	 */
	if (chip->bbt_options & NAND_BBT_SCANLASTPAGE)
		bb_pg_off = blk_sz - pg_sz;
	else if (chip->options & NAND_BBT_SCAN2NDPAGE)
		bb_pg_off = 2 * pg_sz;
	else /* first page */
		bb_pg_off = 0;

	if (!bb_pos) {
		printf("bbm location can't be zero\n");
		return 1;
	}

	if (!pg_sz) {
		printf("page size can't be zero\n");
		return 1;
	}

	if (!blk_sz) {
		printf("block size can't be zero\n");
		return 1;
	}

	/* meta-data partition size in bytes */
	meta_sz = simple_strtoul(argv[3], NULL, 0);
	if (meta_sz >= nand->size || meta_sz < blk_sz) {
		printf("found incorrect meta-data partition size: 0x%llx\n",
		       meta_sz);
		return 1;
	}

	/* meta-data partition offset in bytes */
	meta_off = simple_strtoul(argv[2], NULL, 0);
	if (meta_off + meta_sz >= nand->size) {
		printf("meta-data partition exceeding nand size: 0x%llx\n",
		       meta_off + meta_sz);
		return 1;
	}
	if ((u64)meta_off % blk_sz) {
		printf("meta-data partition offset should be block-aligned\n");
		return 0;
	}

	printf("meta-data partition offset = %llu bytes\n", meta_off);
	printf("meta-data partition size = %llu bytes\n", meta_sz);
	printf("meta-data partition blocks number = %llu blocks\n",
	       ((u64)meta_sz - 1) / blk_sz + 1);

	/* read suffix of "oem" command */
	if (!strncmp(cmd, "prepare", 7)) {  /* prepare oem image */
		if (argc < 6) {
			printf("%s", nand_oem_help_text);
			return 1;
		}
		nand_prepare(nand, argv, meta_off, meta_sz, bb_pos, bb_pg_off);
	} else if (!strncmp(cmd, "restore", 7)) { /* restore original data */
		if (argc < 4) {
			printf("%s", nand_oem_help_text);
			return 1;
		}
		nand_restore(nand, argv, meta_off, meta_sz, bb_pos, bb_pg_off);
	} else { /* nand oem cmd provided with incorrect or without suffix */
		printf("%s", nand_oem_help_text);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(nand_oem, 6, 0, do_nand_oem,
	   "NAND oem cmd", nand_oem_help_text);
