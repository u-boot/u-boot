/*
 * This file is part of UBIFS.
 *
 * Copyright (C) 2006-2008 Nokia Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Authors: Artem Bityutskiy (Битюцкий Артём)
 *          Adrian Hunter
 */

/*
 * This file implements UBIFS superblock. The superblock is stored at the first
 * LEB of the volume and is never changed by UBIFS. Only user-space tools may
 * change it. The superblock node mostly contains geometry information.
 */

#include "ubifs.h"

/*
 * Default journal size in logical eraseblocks as a percent of total
 * flash size.
 */
#define DEFAULT_JNL_PERCENT 5

/* Default maximum journal size in bytes */
#define DEFAULT_MAX_JNL (32*1024*1024)

/* Default indexing tree fanout */
#define DEFAULT_FANOUT 8

/* Default number of data journal heads */
#define DEFAULT_JHEADS_CNT 1

/* Default positions of different LEBs in the main area */
#define DEFAULT_IDX_LEB  0
#define DEFAULT_DATA_LEB 1
#define DEFAULT_GC_LEB   2

/* Default number of LEB numbers in LPT's save table */
#define DEFAULT_LSAVE_CNT 256

/* Default reserved pool size as a percent of maximum free space */
#define DEFAULT_RP_PERCENT 5

/* The default maximum size of reserved pool in bytes */
#define DEFAULT_MAX_RP_SIZE (5*1024*1024)

/* Default time granularity in nanoseconds */
#define DEFAULT_TIME_GRAN 1000000000

/**
 * validate_sb - validate superblock node.
 * @c: UBIFS file-system description object
 * @sup: superblock node
 *
 * This function validates superblock node @sup. Since most of data was read
 * from the superblock and stored in @c, the function validates fields in @c
 * instead. Returns zero in case of success and %-EINVAL in case of validation
 * failure.
 */
static int validate_sb(struct ubifs_info *c, struct ubifs_sb_node *sup)
{
	long long max_bytes;
	int err = 1, min_leb_cnt;

	if (!c->key_hash) {
		err = 2;
		goto failed;
	}

	if (sup->key_fmt != UBIFS_SIMPLE_KEY_FMT) {
		err = 3;
		goto failed;
	}

	if (le32_to_cpu(sup->min_io_size) != c->min_io_size) {
		ubifs_err("min. I/O unit mismatch: %d in superblock, %d real",
			  le32_to_cpu(sup->min_io_size), c->min_io_size);
		goto failed;
	}

	if (le32_to_cpu(sup->leb_size) != c->leb_size) {
		ubifs_err("LEB size mismatch: %d in superblock, %d real",
			  le32_to_cpu(sup->leb_size), c->leb_size);
		goto failed;
	}

	if (c->log_lebs < UBIFS_MIN_LOG_LEBS ||
	    c->lpt_lebs < UBIFS_MIN_LPT_LEBS ||
	    c->orph_lebs < UBIFS_MIN_ORPH_LEBS ||
	    c->main_lebs < UBIFS_MIN_MAIN_LEBS) {
		err = 4;
		goto failed;
	}

	/*
	 * Calculate minimum allowed amount of main area LEBs. This is very
	 * similar to %UBIFS_MIN_LEB_CNT, but we take into account real what we
	 * have just read from the superblock.
	 */
	min_leb_cnt = UBIFS_SB_LEBS + UBIFS_MST_LEBS + c->log_lebs;
	min_leb_cnt += c->lpt_lebs + c->orph_lebs + c->jhead_cnt + 6;

	if (c->leb_cnt < min_leb_cnt || c->leb_cnt > c->vi.size) {
		ubifs_err("bad LEB count: %d in superblock, %d on UBI volume, "
			  "%d minimum required", c->leb_cnt, c->vi.size,
			  min_leb_cnt);
		goto failed;
	}

	if (c->max_leb_cnt < c->leb_cnt) {
		ubifs_err("max. LEB count %d less than LEB count %d",
			  c->max_leb_cnt, c->leb_cnt);
		goto failed;
	}

	if (c->main_lebs < UBIFS_MIN_MAIN_LEBS) {
		err = 7;
		goto failed;
	}

	if (c->max_bud_bytes < (long long)c->leb_size * UBIFS_MIN_BUD_LEBS ||
	    c->max_bud_bytes > (long long)c->leb_size * c->main_lebs) {
		err = 8;
		goto failed;
	}

	if (c->jhead_cnt < NONDATA_JHEADS_CNT + 1 ||
	    c->jhead_cnt > NONDATA_JHEADS_CNT + UBIFS_MAX_JHEADS) {
		err = 9;
		goto failed;
	}

	if (c->fanout < UBIFS_MIN_FANOUT ||
	    ubifs_idx_node_sz(c, c->fanout) > c->leb_size) {
		err = 10;
		goto failed;
	}

	if (c->lsave_cnt < 0 || (c->lsave_cnt > DEFAULT_LSAVE_CNT &&
	    c->lsave_cnt > c->max_leb_cnt - UBIFS_SB_LEBS - UBIFS_MST_LEBS -
	    c->log_lebs - c->lpt_lebs - c->orph_lebs)) {
		err = 11;
		goto failed;
	}

	if (UBIFS_SB_LEBS + UBIFS_MST_LEBS + c->log_lebs + c->lpt_lebs +
	    c->orph_lebs + c->main_lebs != c->leb_cnt) {
		err = 12;
		goto failed;
	}

	if (c->default_compr < 0 || c->default_compr >= UBIFS_COMPR_TYPES_CNT) {
		err = 13;
		goto failed;
	}

	max_bytes = c->main_lebs * (long long)c->leb_size;
	if (c->rp_size < 0 || max_bytes < c->rp_size) {
		err = 14;
		goto failed;
	}

	if (le32_to_cpu(sup->time_gran) > 1000000000 ||
	    le32_to_cpu(sup->time_gran) < 1) {
		err = 15;
		goto failed;
	}

	return 0;

failed:
	ubifs_err("bad superblock, error %d", err);
	dbg_dump_node(c, sup);
	return -EINVAL;
}

/**
 * ubifs_read_sb_node - read superblock node.
 * @c: UBIFS file-system description object
 *
 * This function returns a pointer to the superblock node or a negative error
 * code.
 */
struct ubifs_sb_node *ubifs_read_sb_node(struct ubifs_info *c)
{
	struct ubifs_sb_node *sup;
	int err;

	sup = kmalloc(ALIGN(UBIFS_SB_NODE_SZ, c->min_io_size), GFP_NOFS);
	if (!sup)
		return ERR_PTR(-ENOMEM);

	err = ubifs_read_node(c, sup, UBIFS_SB_NODE, UBIFS_SB_NODE_SZ,
			      UBIFS_SB_LNUM, 0);
	if (err) {
		kfree(sup);
		return ERR_PTR(err);
	}

	return sup;
}

/**
 * ubifs_read_superblock - read superblock.
 * @c: UBIFS file-system description object
 *
 * This function finds, reads and checks the superblock. If an empty UBI volume
 * is being mounted, this function creates default superblock. Returns zero in
 * case of success, and a negative error code in case of failure.
 */
int ubifs_read_superblock(struct ubifs_info *c)
{
	int err, sup_flags;
	struct ubifs_sb_node *sup;

	if (c->empty) {
		printf("No UBIFS filesystem found!\n");
		return -1;
	}

	sup = ubifs_read_sb_node(c);
	if (IS_ERR(sup))
		return PTR_ERR(sup);

	c->fmt_version = le32_to_cpu(sup->fmt_version);
	c->ro_compat_version = le32_to_cpu(sup->ro_compat_version);

	/*
	 * The software supports all previous versions but not future versions,
	 * due to the unavailability of time-travelling equipment.
	 */
	if (c->fmt_version > UBIFS_FORMAT_VERSION) {
		struct super_block *sb = c->vfs_sb;
		int mounting_ro = sb->s_flags & MS_RDONLY;

		ubifs_assert(!c->ro_media || mounting_ro);
		if (!mounting_ro ||
		    c->ro_compat_version > UBIFS_RO_COMPAT_VERSION) {
			ubifs_err("on-flash format version is w%d/r%d, but "
				  "software only supports up to version "
				  "w%d/r%d", c->fmt_version,
				  c->ro_compat_version, UBIFS_FORMAT_VERSION,
				  UBIFS_RO_COMPAT_VERSION);
			if (c->ro_compat_version <= UBIFS_RO_COMPAT_VERSION) {
				ubifs_msg("only R/O mounting is possible");
				err = -EROFS;
			} else
				err = -EINVAL;
			goto out;
		}

		/*
		 * The FS is mounted R/O, and the media format is
		 * R/O-compatible with the UBIFS implementation, so we can
		 * mount.
		 */
		c->rw_incompat = 1;
	}

	if (c->fmt_version < 3) {
		ubifs_err("on-flash format version %d is not supported",
			  c->fmt_version);
		err = -EINVAL;
		goto out;
	}

	switch (sup->key_hash) {
	case UBIFS_KEY_HASH_R5:
		c->key_hash = key_r5_hash;
		c->key_hash_type = UBIFS_KEY_HASH_R5;
		break;

	case UBIFS_KEY_HASH_TEST:
		c->key_hash = key_test_hash;
		c->key_hash_type = UBIFS_KEY_HASH_TEST;
		break;
	};

	c->key_fmt = sup->key_fmt;

	switch (c->key_fmt) {
	case UBIFS_SIMPLE_KEY_FMT:
		c->key_len = UBIFS_SK_LEN;
		break;
	default:
		ubifs_err("unsupported key format");
		err = -EINVAL;
		goto out;
	}

	c->leb_cnt       = le32_to_cpu(sup->leb_cnt);
	c->max_leb_cnt   = le32_to_cpu(sup->max_leb_cnt);
	c->max_bud_bytes = le64_to_cpu(sup->max_bud_bytes);
	c->log_lebs      = le32_to_cpu(sup->log_lebs);
	c->lpt_lebs      = le32_to_cpu(sup->lpt_lebs);
	c->orph_lebs     = le32_to_cpu(sup->orph_lebs);
	c->jhead_cnt     = le32_to_cpu(sup->jhead_cnt) + NONDATA_JHEADS_CNT;
	c->fanout        = le32_to_cpu(sup->fanout);
	c->lsave_cnt     = le32_to_cpu(sup->lsave_cnt);
	c->default_compr = le16_to_cpu(sup->default_compr);
	c->rp_size       = le64_to_cpu(sup->rp_size);
	c->rp_uid        = le32_to_cpu(sup->rp_uid);
	c->rp_gid        = le32_to_cpu(sup->rp_gid);
	sup_flags        = le32_to_cpu(sup->flags);

	c->vfs_sb->s_time_gran = le32_to_cpu(sup->time_gran);
	memcpy(&c->uuid, &sup->uuid, 16);
	c->big_lpt = !!(sup_flags & UBIFS_FLG_BIGLPT);

	/* Automatically increase file system size to the maximum size */
	c->old_leb_cnt = c->leb_cnt;
	if (c->leb_cnt < c->vi.size && c->leb_cnt < c->max_leb_cnt) {
		c->leb_cnt = min_t(int, c->max_leb_cnt, c->vi.size);
		dbg_mnt("Auto resizing (ro) from %d LEBs to %d LEBs",
			c->old_leb_cnt,	c->leb_cnt);
	}

	c->log_bytes = (long long)c->log_lebs * c->leb_size;
	c->log_last = UBIFS_LOG_LNUM + c->log_lebs - 1;
	c->lpt_first = UBIFS_LOG_LNUM + c->log_lebs;
	c->lpt_last = c->lpt_first + c->lpt_lebs - 1;
	c->orph_first = c->lpt_last + 1;
	c->orph_last = c->orph_first + c->orph_lebs - 1;
	c->main_lebs = c->leb_cnt - UBIFS_SB_LEBS - UBIFS_MST_LEBS;
	c->main_lebs -= c->log_lebs + c->lpt_lebs + c->orph_lebs;
	c->main_first = c->leb_cnt - c->main_lebs;
	c->report_rp_size = ubifs_reported_space(c, c->rp_size);

	err = validate_sb(c, sup);
out:
	kfree(sup);
	return err;
}
