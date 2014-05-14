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
 * This file implements UBIFS initialization and VFS superblock operations. Some
 * initialization stuff which is rather large and complex is placed at
 * corresponding subsystems, but most of it is here.
 */

#include "ubifs.h"
#include <linux/math64.h>

#define INODE_LOCKED_MAX	64

struct super_block *ubifs_sb;
static struct inode *inodes_locked_down[INODE_LOCKED_MAX];

/* shrinker.c */

/* List of all UBIFS file-system instances */
struct list_head ubifs_infos;

/* linux/fs/super.c */

static int sb_set(struct super_block *sb, void *data)
{
	dev_t *dev = data;

	sb->s_dev = *dev;
	return 0;
}

/**
 *	sget	-	find or create a superblock
 *	@type:	filesystem type superblock should belong to
 *	@test:	comparison callback
 *	@set:	setup callback
 *	@data:	argument to each of them
 */
struct super_block *sget(struct file_system_type *type,
			int (*test)(struct super_block *,void *),
			int (*set)(struct super_block *,void *),
			void *data)
{
	struct super_block *s = NULL;
	int err;

	s = kzalloc(sizeof(struct super_block),  GFP_USER);
	if (!s) {
		err = -ENOMEM;
		return ERR_PTR(err);
	}

	INIT_LIST_HEAD(&s->s_instances);
	INIT_LIST_HEAD(&s->s_inodes);
	s->s_time_gran = 1000000000;

	err = set(s, data);
	if (err) {
		return ERR_PTR(err);
	}
	s->s_type = type;
	strncpy(s->s_id, type->name, sizeof(s->s_id));
	list_add(&s->s_instances, &type->fs_supers);
	return s;
}

/**
 * validate_inode - validate inode.
 * @c: UBIFS file-system description object
 * @inode: the inode to validate
 *
 * This is a helper function for 'ubifs_iget()' which validates various fields
 * of a newly built inode to make sure they contain sane values and prevent
 * possible vulnerabilities. Returns zero if the inode is all right and
 * a non-zero error code if not.
 */
static int validate_inode(struct ubifs_info *c, const struct inode *inode)
{
	int err;
	const struct ubifs_inode *ui = ubifs_inode(inode);

	if (inode->i_size > c->max_inode_sz) {
		ubifs_err("inode is too large (%lld)",
			  (long long)inode->i_size);
		return 1;
	}

	if (ui->compr_type < 0 || ui->compr_type >= UBIFS_COMPR_TYPES_CNT) {
		ubifs_err("unknown compression type %d", ui->compr_type);
		return 2;
	}

	if (ui->data_len < 0 || ui->data_len > UBIFS_MAX_INO_DATA)
		return 4;

	if (!ubifs_compr_present(ui->compr_type)) {
		ubifs_warn("inode %lu uses '%s' compression, but it was not "
			   "compiled in", inode->i_ino,
			   ubifs_compr_name(ui->compr_type));
	}

	err = dbg_check_dir_size(c, inode);
	return err;
}

struct inode *iget_locked(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;

	inode = (struct inode *)malloc(sizeof(struct ubifs_inode));
	if (inode) {
		inode->i_ino = ino;
		inode->i_sb = sb;
		list_add(&inode->i_sb_list, &sb->s_inodes);
		inode->i_state = I_LOCK | I_NEW;
	}

	return inode;
}

int ubifs_iput(struct inode *inode)
{
	list_del_init(&inode->i_sb_list);

	free(inode);
	return 0;
}

/*
 * Lock (save) inode in inode array for readback after recovery
 */
void iput(struct inode *inode)
{
	int i;
	struct inode *ino;

	/*
	 * Search end of list
	 */
	for (i = 0; i < INODE_LOCKED_MAX; i++) {
		if (inodes_locked_down[i] == NULL)
			break;
	}

	if (i >= INODE_LOCKED_MAX) {
		ubifs_err("Error, can't lock (save) more inodes while recovery!!!");
		return;
	}

	/*
	 * Allocate and use new inode
	 */
	ino = (struct inode *)malloc(sizeof(struct ubifs_inode));
	memcpy(ino, inode, sizeof(struct ubifs_inode));

	/*
	 * Finally save inode in array
	 */
	inodes_locked_down[i] = ino;
}

struct inode *ubifs_iget(struct super_block *sb, unsigned long inum)
{
	int err;
	union ubifs_key key;
	struct ubifs_ino_node *ino;
	struct ubifs_info *c = sb->s_fs_info;
	struct inode *inode;
	struct ubifs_inode *ui;
	int i;

	dbg_gen("inode %lu", inum);

	/*
	 * U-Boot special handling of locked down inodes via recovery
	 * e.g. ubifs_recover_size()
	 */
	for (i = 0; i < INODE_LOCKED_MAX; i++) {
		/*
		 * Exit on last entry (NULL), inode not found in list
		 */
		if (inodes_locked_down[i] == NULL)
			break;

		if (inodes_locked_down[i]->i_ino == inum) {
			/*
			 * We found the locked down inode in our array,
			 * so just return this pointer instead of creating
			 * a new one.
			 */
			return inodes_locked_down[i];
		}
	}

	inode = iget_locked(sb, inum);
	if (!inode)
		return ERR_PTR(-ENOMEM);
	if (!(inode->i_state & I_NEW))
		return inode;
	ui = ubifs_inode(inode);

	ino = kmalloc(UBIFS_MAX_INO_NODE_SZ, GFP_NOFS);
	if (!ino) {
		err = -ENOMEM;
		goto out;
	}

	ino_key_init(c, &key, inode->i_ino);

	err = ubifs_tnc_lookup(c, &key, ino);
	if (err)
		goto out_ino;

	inode->i_flags |= (S_NOCMTIME | S_NOATIME);
	inode->i_nlink = le32_to_cpu(ino->nlink);
	inode->i_uid   = le32_to_cpu(ino->uid);
	inode->i_gid   = le32_to_cpu(ino->gid);
	inode->i_atime.tv_sec  = (int64_t)le64_to_cpu(ino->atime_sec);
	inode->i_atime.tv_nsec = le32_to_cpu(ino->atime_nsec);
	inode->i_mtime.tv_sec  = (int64_t)le64_to_cpu(ino->mtime_sec);
	inode->i_mtime.tv_nsec = le32_to_cpu(ino->mtime_nsec);
	inode->i_ctime.tv_sec  = (int64_t)le64_to_cpu(ino->ctime_sec);
	inode->i_ctime.tv_nsec = le32_to_cpu(ino->ctime_nsec);
	inode->i_mode = le32_to_cpu(ino->mode);
	inode->i_size = le64_to_cpu(ino->size);

	ui->data_len    = le32_to_cpu(ino->data_len);
	ui->flags       = le32_to_cpu(ino->flags);
	ui->compr_type  = le16_to_cpu(ino->compr_type);
	ui->creat_sqnum = le64_to_cpu(ino->creat_sqnum);
	ui->synced_i_size = ui->ui_size = inode->i_size;

	err = validate_inode(c, inode);
	if (err)
		goto out_invalid;

	if ((inode->i_mode & S_IFMT) == S_IFLNK) {
		if (ui->data_len <= 0 || ui->data_len > UBIFS_MAX_INO_DATA) {
			err = 12;
			goto out_invalid;
		}
		ui->data = kmalloc(ui->data_len + 1, GFP_NOFS);
		if (!ui->data) {
			err = -ENOMEM;
			goto out_ino;
		}
		memcpy(ui->data, ino->data, ui->data_len);
		((char *)ui->data)[ui->data_len] = '\0';
	}

	kfree(ino);
	inode->i_state &= ~(I_LOCK | I_NEW);
	return inode;

out_invalid:
	ubifs_err("inode %lu validation failed, error %d", inode->i_ino, err);
	dbg_dump_node(c, ino);
	dbg_dump_inode(c, inode);
	err = -EINVAL;
out_ino:
	kfree(ino);
out:
	ubifs_err("failed to read inode %lu, error %d", inode->i_ino, err);
	return ERR_PTR(err);
}

/**
 * init_constants_early - initialize UBIFS constants.
 * @c: UBIFS file-system description object
 *
 * This function initialize UBIFS constants which do not need the superblock to
 * be read. It also checks that the UBI volume satisfies basic UBIFS
 * requirements. Returns zero in case of success and a negative error code in
 * case of failure.
 */
static int init_constants_early(struct ubifs_info *c)
{
	if (c->vi.corrupted) {
		ubifs_warn("UBI volume is corrupted - read-only mode");
		c->ro_media = 1;
	}

	if (c->di.ro_mode) {
		ubifs_msg("read-only UBI device");
		c->ro_media = 1;
	}

	if (c->vi.vol_type == UBI_STATIC_VOLUME) {
		ubifs_msg("static UBI volume - read-only mode");
		c->ro_media = 1;
	}

	c->leb_cnt = c->vi.size;
	c->leb_size = c->vi.usable_leb_size;
	c->half_leb_size = c->leb_size / 2;
	c->min_io_size = c->di.min_io_size;
	c->min_io_shift = fls(c->min_io_size) - 1;

	if (c->leb_size < UBIFS_MIN_LEB_SZ) {
		ubifs_err("too small LEBs (%d bytes), min. is %d bytes",
			  c->leb_size, UBIFS_MIN_LEB_SZ);
		return -EINVAL;
	}

	if (c->leb_cnt < UBIFS_MIN_LEB_CNT) {
		ubifs_err("too few LEBs (%d), min. is %d",
			  c->leb_cnt, UBIFS_MIN_LEB_CNT);
		return -EINVAL;
	}

	if (!is_power_of_2(c->min_io_size)) {
		ubifs_err("bad min. I/O size %d", c->min_io_size);
		return -EINVAL;
	}

	/*
	 * UBIFS aligns all node to 8-byte boundary, so to make function in
	 * io.c simpler, assume minimum I/O unit size to be 8 bytes if it is
	 * less than 8.
	 */
	if (c->min_io_size < 8) {
		c->min_io_size = 8;
		c->min_io_shift = 3;
	}

	c->ref_node_alsz = ALIGN(UBIFS_REF_NODE_SZ, c->min_io_size);
	c->mst_node_alsz = ALIGN(UBIFS_MST_NODE_SZ, c->min_io_size);

	/*
	 * Initialize node length ranges which are mostly needed for node
	 * length validation.
	 */
	c->ranges[UBIFS_PAD_NODE].len  = UBIFS_PAD_NODE_SZ;
	c->ranges[UBIFS_SB_NODE].len   = UBIFS_SB_NODE_SZ;
	c->ranges[UBIFS_MST_NODE].len  = UBIFS_MST_NODE_SZ;
	c->ranges[UBIFS_REF_NODE].len  = UBIFS_REF_NODE_SZ;
	c->ranges[UBIFS_TRUN_NODE].len = UBIFS_TRUN_NODE_SZ;
	c->ranges[UBIFS_CS_NODE].len   = UBIFS_CS_NODE_SZ;

	c->ranges[UBIFS_INO_NODE].min_len  = UBIFS_INO_NODE_SZ;
	c->ranges[UBIFS_INO_NODE].max_len  = UBIFS_MAX_INO_NODE_SZ;
	c->ranges[UBIFS_ORPH_NODE].min_len =
				UBIFS_ORPH_NODE_SZ + sizeof(__le64);
	c->ranges[UBIFS_ORPH_NODE].max_len = c->leb_size;
	c->ranges[UBIFS_DENT_NODE].min_len = UBIFS_DENT_NODE_SZ;
	c->ranges[UBIFS_DENT_NODE].max_len = UBIFS_MAX_DENT_NODE_SZ;
	c->ranges[UBIFS_XENT_NODE].min_len = UBIFS_XENT_NODE_SZ;
	c->ranges[UBIFS_XENT_NODE].max_len = UBIFS_MAX_XENT_NODE_SZ;
	c->ranges[UBIFS_DATA_NODE].min_len = UBIFS_DATA_NODE_SZ;
	c->ranges[UBIFS_DATA_NODE].max_len = UBIFS_MAX_DATA_NODE_SZ;
	/*
	 * Minimum indexing node size is amended later when superblock is
	 * read and the key length is known.
	 */
	c->ranges[UBIFS_IDX_NODE].min_len = UBIFS_IDX_NODE_SZ + UBIFS_BRANCH_SZ;
	/*
	 * Maximum indexing node size is amended later when superblock is
	 * read and the fanout is known.
	 */
	c->ranges[UBIFS_IDX_NODE].max_len = INT_MAX;

	/*
	 * Initialize dead and dark LEB space watermarks. See gc.c for comments
	 * about these values.
	 */
	c->dead_wm = ALIGN(MIN_WRITE_SZ, c->min_io_size);
	c->dark_wm = ALIGN(UBIFS_MAX_NODE_SZ, c->min_io_size);

	/*
	 * Calculate how many bytes would be wasted at the end of LEB if it was
	 * fully filled with data nodes of maximum size. This is used in
	 * calculations when reporting free space.
	 */
	c->leb_overhead = c->leb_size % UBIFS_MAX_DATA_NODE_SZ;

	return 0;
}

/*
 * init_constants_sb - initialize UBIFS constants.
 * @c: UBIFS file-system description object
 *
 * This is a helper function which initializes various UBIFS constants after
 * the superblock has been read. It also checks various UBIFS parameters and
 * makes sure they are all right. Returns zero in case of success and a
 * negative error code in case of failure.
 */
static int init_constants_sb(struct ubifs_info *c)
{
	int tmp, err;
	long long tmp64;

	c->main_bytes = (long long)c->main_lebs * c->leb_size;
	c->max_znode_sz = sizeof(struct ubifs_znode) +
				c->fanout * sizeof(struct ubifs_zbranch);

	tmp = ubifs_idx_node_sz(c, 1);
	c->ranges[UBIFS_IDX_NODE].min_len = tmp;
	c->min_idx_node_sz = ALIGN(tmp, 8);

	tmp = ubifs_idx_node_sz(c, c->fanout);
	c->ranges[UBIFS_IDX_NODE].max_len = tmp;
	c->max_idx_node_sz = ALIGN(tmp, 8);

	/* Make sure LEB size is large enough to fit full commit */
	tmp = UBIFS_CS_NODE_SZ + UBIFS_REF_NODE_SZ * c->jhead_cnt;
	tmp = ALIGN(tmp, c->min_io_size);
	if (tmp > c->leb_size) {
		dbg_err("too small LEB size %d, at least %d needed",
			c->leb_size, tmp);
		return -EINVAL;
	}

	/*
	 * Make sure that the log is large enough to fit reference nodes for
	 * all buds plus one reserved LEB.
	 */
	tmp64 = c->max_bud_bytes + c->leb_size - 1;
	c->max_bud_cnt = div_u64(tmp64, c->leb_size);
	tmp = (c->ref_node_alsz * c->max_bud_cnt + c->leb_size - 1);
	tmp /= c->leb_size;
	tmp += 1;
	if (c->log_lebs < tmp) {
		dbg_err("too small log %d LEBs, required min. %d LEBs",
			c->log_lebs, tmp);
		return -EINVAL;
	}

	/*
	 * When budgeting we assume worst-case scenarios when the pages are not
	 * be compressed and direntries are of the maximum size.
	 *
	 * Note, data, which may be stored in inodes is budgeted separately, so
	 * it is not included into 'c->inode_budget'.
	 */
	c->page_budget = UBIFS_MAX_DATA_NODE_SZ * UBIFS_BLOCKS_PER_PAGE;
	c->inode_budget = UBIFS_INO_NODE_SZ;
	c->dent_budget = UBIFS_MAX_DENT_NODE_SZ;

	/*
	 * When the amount of flash space used by buds becomes
	 * 'c->max_bud_bytes', UBIFS just blocks all writers and starts commit.
	 * The writers are unblocked when the commit is finished. To avoid
	 * writers to be blocked UBIFS initiates background commit in advance,
	 * when number of bud bytes becomes above the limit defined below.
	 */
	c->bg_bud_bytes = (c->max_bud_bytes * 13) >> 4;

	/*
	 * Ensure minimum journal size. All the bytes in the journal heads are
	 * considered to be used, when calculating the current journal usage.
	 * Consequently, if the journal is too small, UBIFS will treat it as
	 * always full.
	 */
	tmp64 = (long long)(c->jhead_cnt + 1) * c->leb_size + 1;
	if (c->bg_bud_bytes < tmp64)
		c->bg_bud_bytes = tmp64;
	if (c->max_bud_bytes < tmp64 + c->leb_size)
		c->max_bud_bytes = tmp64 + c->leb_size;

	err = ubifs_calc_lpt_geom(c);
	if (err)
		return err;

	return 0;
}

/*
 * init_constants_master - initialize UBIFS constants.
 * @c: UBIFS file-system description object
 *
 * This is a helper function which initializes various UBIFS constants after
 * the master node has been read. It also checks various UBIFS parameters and
 * makes sure they are all right.
 */
static void init_constants_master(struct ubifs_info *c)
{
	long long tmp64;

	c->min_idx_lebs = ubifs_calc_min_idx_lebs(c);

	/*
	 * Calculate total amount of FS blocks. This number is not used
	 * internally because it does not make much sense for UBIFS, but it is
	 * necessary to report something for the 'statfs()' call.
	 *
	 * Subtract the LEB reserved for GC, the LEB which is reserved for
	 * deletions, minimum LEBs for the index, and assume only one journal
	 * head is available.
	 */
	tmp64 = c->main_lebs - 1 - 1 - MIN_INDEX_LEBS - c->jhead_cnt + 1;
	tmp64 *= (long long)c->leb_size - c->leb_overhead;
	tmp64 = ubifs_reported_space(c, tmp64);
	c->block_cnt = tmp64 >> UBIFS_BLOCK_SHIFT;
}

/**
 * free_orphans - free orphans.
 * @c: UBIFS file-system description object
 */
static void free_orphans(struct ubifs_info *c)
{
	struct ubifs_orphan *orph;

	while (c->orph_dnext) {
		orph = c->orph_dnext;
		c->orph_dnext = orph->dnext;
		list_del(&orph->list);
		kfree(orph);
	}

	while (!list_empty(&c->orph_list)) {
		orph = list_entry(c->orph_list.next, struct ubifs_orphan, list);
		list_del(&orph->list);
		kfree(orph);
		dbg_err("orphan list not empty at unmount");
	}

	vfree(c->orph_buf);
	c->orph_buf = NULL;
}

/**
 * check_volume_empty - check if the UBI volume is empty.
 * @c: UBIFS file-system description object
 *
 * This function checks if the UBIFS volume is empty by looking if its LEBs are
 * mapped or not. The result of checking is stored in the @c->empty variable.
 * Returns zero in case of success and a negative error code in case of
 * failure.
 */
static int check_volume_empty(struct ubifs_info *c)
{
	int lnum, err;

	c->empty = 1;
	for (lnum = 0; lnum < c->leb_cnt; lnum++) {
		err = ubi_is_mapped(c->ubi, lnum);
		if (unlikely(err < 0))
			return err;
		if (err == 1) {
			c->empty = 0;
			break;
		}

		cond_resched();
	}

	return 0;
}

/**
 * mount_ubifs - mount UBIFS file-system.
 * @c: UBIFS file-system description object
 *
 * This function mounts UBIFS file system. Returns zero in case of success and
 * a negative error code in case of failure.
 *
 * Note, the function does not de-allocate resources it it fails half way
 * through, and the caller has to do this instead.
 */
static int mount_ubifs(struct ubifs_info *c)
{
	struct super_block *sb = c->vfs_sb;
	int err, mounted_read_only = (sb->s_flags & MS_RDONLY);
	long long x;
	size_t sz;

	err = init_constants_early(c);
	if (err)
		return err;

	err = ubifs_debugging_init(c);
	if (err)
		return err;

	err = check_volume_empty(c);
	if (err)
		goto out_free;

	if (c->empty && (mounted_read_only || c->ro_media)) {
		/*
		 * This UBI volume is empty, and read-only, or the file system
		 * is mounted read-only - we cannot format it.
		 */
		ubifs_err("can't format empty UBI volume: read-only %s",
			  c->ro_media ? "UBI volume" : "mount");
		err = -EROFS;
		goto out_free;
	}

	if (c->ro_media && !mounted_read_only) {
		ubifs_err("cannot mount read-write - read-only media");
		err = -EROFS;
		goto out_free;
	}

	/*
	 * The requirement for the buffer is that it should fit indexing B-tree
	 * height amount of integers. We assume the height if the TNC tree will
	 * never exceed 64.
	 */
	err = -ENOMEM;
	c->bottom_up_buf = kmalloc(BOTTOM_UP_HEIGHT * sizeof(int), GFP_KERNEL);
	if (!c->bottom_up_buf)
		goto out_free;

	c->sbuf = vmalloc(c->leb_size);
	if (!c->sbuf)
		goto out_free;

	/*
	 * We have to check all CRCs, even for data nodes, when we mount the FS
	 * (specifically, when we are replaying).
	 */
	c->always_chk_crc = 1;

	err = ubifs_read_superblock(c);
	if (err)
		goto out_free;

	/*
	 * Make sure the compressor which is set as default in the superblock
	 * or overridden by mount options is actually compiled in.
	 */
	if (!ubifs_compr_present(c->default_compr)) {
		ubifs_err("'compressor \"%s\" is not compiled in",
			  ubifs_compr_name(c->default_compr));
		goto out_free;
	}

	dbg_failure_mode_registration(c);

	err = init_constants_sb(c);
	if (err)
		goto out_free;

	sz = ALIGN(c->max_idx_node_sz, c->min_io_size);
	sz = ALIGN(sz + c->max_idx_node_sz, c->min_io_size);
	c->cbuf = kmalloc(sz, GFP_NOFS);
	if (!c->cbuf) {
		err = -ENOMEM;
		goto out_free;
	}

	sprintf(c->bgt_name, BGT_NAME_PATTERN, c->vi.ubi_num, c->vi.vol_id);

	err = ubifs_read_master(c);
	if (err)
		goto out_master;

	init_constants_master(c);

	if ((c->mst_node->flags & cpu_to_le32(UBIFS_MST_DIRTY)) != 0) {
		ubifs_msg("recovery needed");
		c->need_recovery = 1;
	}

	err = ubifs_lpt_init(c, 1, !mounted_read_only);
	if (err)
		goto out_lpt;

	err = dbg_check_idx_size(c, c->old_idx_sz);
	if (err)
		goto out_lpt;

	err = ubifs_replay_journal(c);
	if (err)
		goto out_journal;

	err = ubifs_mount_orphans(c, c->need_recovery, mounted_read_only);
	if (err)
		goto out_orphans;

	if (c->need_recovery) {
		err = ubifs_recover_size(c);
		if (err)
			goto out_orphans;
	}

	spin_lock(&ubifs_infos_lock);
	list_add_tail(&c->infos_list, &ubifs_infos);
	spin_unlock(&ubifs_infos_lock);

	if (c->need_recovery) {
		if (mounted_read_only)
			ubifs_msg("recovery deferred");
		else {
			c->need_recovery = 0;
			ubifs_msg("recovery completed");
		}
	}

	err = dbg_check_filesystem(c);
	if (err)
		goto out_infos;

	c->always_chk_crc = 0;

	ubifs_msg("mounted UBI device %d, volume %d, name \"%s\"",
		  c->vi.ubi_num, c->vi.vol_id, c->vi.name);
	if (mounted_read_only)
		ubifs_msg("mounted read-only");
	x = (long long)c->main_lebs * c->leb_size;
	ubifs_msg("file system size:   %lld bytes (%lld KiB, %lld MiB, %d "
		  "LEBs)", x, x >> 10, x >> 20, c->main_lebs);
	x = (long long)c->log_lebs * c->leb_size + c->max_bud_bytes;
	ubifs_msg("journal size:       %lld bytes (%lld KiB, %lld MiB, %d "
		  "LEBs)", x, x >> 10, x >> 20, c->log_lebs + c->max_bud_cnt);
	ubifs_msg("media format:       w%d/r%d (latest is w%d/r%d)",
		  c->fmt_version, c->ro_compat_version,
		  UBIFS_FORMAT_VERSION, UBIFS_RO_COMPAT_VERSION);
	ubifs_msg("default compressor: %s", ubifs_compr_name(c->default_compr));
	ubifs_msg("reserved for root:  %llu bytes (%llu KiB)",
		c->report_rp_size, c->report_rp_size >> 10);

	dbg_msg("min. I/O unit size:  %d bytes", c->min_io_size);
	dbg_msg("LEB size:            %d bytes (%d KiB)",
		c->leb_size, c->leb_size >> 10);
	dbg_msg("data journal heads:  %d",
		c->jhead_cnt - NONDATA_JHEADS_CNT);
	dbg_msg("UUID:                %02X%02X%02X%02X-%02X%02X"
	       "-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	       c->uuid[0], c->uuid[1], c->uuid[2], c->uuid[3],
	       c->uuid[4], c->uuid[5], c->uuid[6], c->uuid[7],
	       c->uuid[8], c->uuid[9], c->uuid[10], c->uuid[11],
	       c->uuid[12], c->uuid[13], c->uuid[14], c->uuid[15]);
	dbg_msg("big_lpt              %d", c->big_lpt);
	dbg_msg("log LEBs:            %d (%d - %d)",
		c->log_lebs, UBIFS_LOG_LNUM, c->log_last);
	dbg_msg("LPT area LEBs:       %d (%d - %d)",
		c->lpt_lebs, c->lpt_first, c->lpt_last);
	dbg_msg("orphan area LEBs:    %d (%d - %d)",
		c->orph_lebs, c->orph_first, c->orph_last);
	dbg_msg("main area LEBs:      %d (%d - %d)",
		c->main_lebs, c->main_first, c->leb_cnt - 1);
	dbg_msg("index LEBs:          %d", c->lst.idx_lebs);
	dbg_msg("total index bytes:   %lld (%lld KiB, %lld MiB)",
		c->old_idx_sz, c->old_idx_sz >> 10, c->old_idx_sz >> 20);
	dbg_msg("key hash type:       %d", c->key_hash_type);
	dbg_msg("tree fanout:         %d", c->fanout);
	dbg_msg("reserved GC LEB:     %d", c->gc_lnum);
	dbg_msg("first main LEB:      %d", c->main_first);
	dbg_msg("max. znode size      %d", c->max_znode_sz);
	dbg_msg("max. index node size %d", c->max_idx_node_sz);
	dbg_msg("node sizes:          data %zu, inode %zu, dentry %zu",
		UBIFS_DATA_NODE_SZ, UBIFS_INO_NODE_SZ, UBIFS_DENT_NODE_SZ);
	dbg_msg("node sizes:          trun %zu, sb %zu, master %zu",
		UBIFS_TRUN_NODE_SZ, UBIFS_SB_NODE_SZ, UBIFS_MST_NODE_SZ);
	dbg_msg("node sizes:          ref %zu, cmt. start %zu, orph %zu",
		UBIFS_REF_NODE_SZ, UBIFS_CS_NODE_SZ, UBIFS_ORPH_NODE_SZ);
	dbg_msg("max. node sizes:     data %zu, inode %zu dentry %zu",
		UBIFS_MAX_DATA_NODE_SZ, UBIFS_MAX_INO_NODE_SZ,
		UBIFS_MAX_DENT_NODE_SZ);
	dbg_msg("dead watermark:      %d", c->dead_wm);
	dbg_msg("dark watermark:      %d", c->dark_wm);
	dbg_msg("LEB overhead:        %d", c->leb_overhead);
	x = (long long)c->main_lebs * c->dark_wm;
	dbg_msg("max. dark space:     %lld (%lld KiB, %lld MiB)",
		x, x >> 10, x >> 20);
	dbg_msg("maximum bud bytes:   %lld (%lld KiB, %lld MiB)",
		c->max_bud_bytes, c->max_bud_bytes >> 10,
		c->max_bud_bytes >> 20);
	dbg_msg("BG commit bud bytes: %lld (%lld KiB, %lld MiB)",
		c->bg_bud_bytes, c->bg_bud_bytes >> 10,
		c->bg_bud_bytes >> 20);
	dbg_msg("current bud bytes    %lld (%lld KiB, %lld MiB)",
		c->bud_bytes, c->bud_bytes >> 10, c->bud_bytes >> 20);
	dbg_msg("max. seq. number:    %llu", c->max_sqnum);
	dbg_msg("commit number:       %llu", c->cmt_no);

	return 0;

out_infos:
	spin_lock(&ubifs_infos_lock);
	list_del(&c->infos_list);
	spin_unlock(&ubifs_infos_lock);
out_orphans:
	free_orphans(c);
out_journal:
out_lpt:
	ubifs_lpt_free(c, 0);
out_master:
	kfree(c->mst_node);
	kfree(c->rcvrd_mst_node);
	if (c->bgt)
		kthread_stop(c->bgt);
	kfree(c->cbuf);
out_free:
	vfree(c->ileb_buf);
	vfree(c->sbuf);
	kfree(c->bottom_up_buf);
	ubifs_debugging_exit(c);
	return err;
}

/**
 * ubifs_umount - un-mount UBIFS file-system.
 * @c: UBIFS file-system description object
 *
 * Note, this function is called to free allocated resourced when un-mounting,
 * as well as free resources when an error occurred while we were half way
 * through mounting (error path cleanup function). So it has to make sure the
 * resource was actually allocated before freeing it.
 */
void ubifs_umount(struct ubifs_info *c)
{
	dbg_gen("un-mounting UBI device %d, volume %d", c->vi.ubi_num,
		c->vi.vol_id);

	spin_lock(&ubifs_infos_lock);
	list_del(&c->infos_list);
	spin_unlock(&ubifs_infos_lock);

	if (c->bgt)
		kthread_stop(c->bgt);

	free_orphans(c);
	ubifs_lpt_free(c, 0);

	kfree(c->cbuf);
	kfree(c->rcvrd_mst_node);
	kfree(c->mst_node);
	vfree(c->ileb_buf);
	vfree(c->sbuf);
	kfree(c->bottom_up_buf);
	ubifs_debugging_exit(c);

	/* Finally free U-Boot's global copy of superblock */
	if (ubifs_sb != NULL) {
		free(ubifs_sb->s_fs_info);
		free(ubifs_sb);
	}
}

/**
 * open_ubi - parse UBI device name string and open the UBI device.
 * @name: UBI volume name
 * @mode: UBI volume open mode
 *
 * There are several ways to specify UBI volumes when mounting UBIFS:
 * o ubiX_Y    - UBI device number X, volume Y;
 * o ubiY      - UBI device number 0, volume Y;
 * o ubiX:NAME - mount UBI device X, volume with name NAME;
 * o ubi:NAME  - mount UBI device 0, volume with name NAME.
 *
 * Alternative '!' separator may be used instead of ':' (because some shells
 * like busybox may interpret ':' as an NFS host name separator). This function
 * returns ubi volume object in case of success and a negative error code in
 * case of failure.
 */
static struct ubi_volume_desc *open_ubi(const char *name, int mode)
{
	int dev, vol;
	char *endptr;

	if (name[0] != 'u' || name[1] != 'b' || name[2] != 'i')
		return ERR_PTR(-EINVAL);

	/* ubi:NAME method */
	if ((name[3] == ':' || name[3] == '!') && name[4] != '\0')
		return ubi_open_volume_nm(0, name + 4, mode);

	if (!isdigit(name[3]))
		return ERR_PTR(-EINVAL);

	dev = simple_strtoul(name + 3, &endptr, 0);

	/* ubiY method */
	if (*endptr == '\0')
		return ubi_open_volume(0, dev, mode);

	/* ubiX_Y method */
	if (*endptr == '_' && isdigit(endptr[1])) {
		vol = simple_strtoul(endptr + 1, &endptr, 0);
		if (*endptr != '\0')
			return ERR_PTR(-EINVAL);
		return ubi_open_volume(dev, vol, mode);
	}

	/* ubiX:NAME method */
	if ((*endptr == ':' || *endptr == '!') && endptr[1] != '\0')
		return ubi_open_volume_nm(dev, ++endptr, mode);

	return ERR_PTR(-EINVAL);
}

static int ubifs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct ubi_volume_desc *ubi = sb->s_fs_info;
	struct ubifs_info *c;
	struct inode *root;
	int err;

	c = kzalloc(sizeof(struct ubifs_info), GFP_KERNEL);
	if (!c)
		return -ENOMEM;

	spin_lock_init(&c->cnt_lock);
	spin_lock_init(&c->cs_lock);
	spin_lock_init(&c->buds_lock);
	spin_lock_init(&c->space_lock);
	spin_lock_init(&c->orphan_lock);
	init_rwsem(&c->commit_sem);
	mutex_init(&c->lp_mutex);
	mutex_init(&c->tnc_mutex);
	mutex_init(&c->log_mutex);
	mutex_init(&c->mst_mutex);
	mutex_init(&c->umount_mutex);
	init_waitqueue_head(&c->cmt_wq);
	c->buds = RB_ROOT;
	c->old_idx = RB_ROOT;
	c->size_tree = RB_ROOT;
	c->orph_tree = RB_ROOT;
	INIT_LIST_HEAD(&c->infos_list);
	INIT_LIST_HEAD(&c->idx_gc);
	INIT_LIST_HEAD(&c->replay_list);
	INIT_LIST_HEAD(&c->replay_buds);
	INIT_LIST_HEAD(&c->uncat_list);
	INIT_LIST_HEAD(&c->empty_list);
	INIT_LIST_HEAD(&c->freeable_list);
	INIT_LIST_HEAD(&c->frdi_idx_list);
	INIT_LIST_HEAD(&c->unclean_leb_list);
	INIT_LIST_HEAD(&c->old_buds);
	INIT_LIST_HEAD(&c->orph_list);
	INIT_LIST_HEAD(&c->orph_new);

	c->highest_inum = UBIFS_FIRST_INO;
	c->lhead_lnum = c->ltail_lnum = UBIFS_LOG_LNUM;

	ubi_get_volume_info(ubi, &c->vi);
	ubi_get_device_info(c->vi.ubi_num, &c->di);

	/* Re-open the UBI device in read-write mode */
	c->ubi = ubi_open_volume(c->vi.ubi_num, c->vi.vol_id, UBI_READONLY);
	if (IS_ERR(c->ubi)) {
		err = PTR_ERR(c->ubi);
		goto out_free;
	}

	c->vfs_sb = sb;

	sb->s_fs_info = c;
	sb->s_magic = UBIFS_SUPER_MAGIC;
	sb->s_blocksize = UBIFS_BLOCK_SIZE;
	sb->s_blocksize_bits = UBIFS_BLOCK_SHIFT;
	sb->s_dev = c->vi.cdev;
	sb->s_maxbytes = c->max_inode_sz = key_max_inode_size(c);
	if (c->max_inode_sz > MAX_LFS_FILESIZE)
		sb->s_maxbytes = c->max_inode_sz = MAX_LFS_FILESIZE;

	if (c->rw_incompat) {
		ubifs_err("the file-system is not R/W-compatible");
		ubifs_msg("on-flash format version is w%d/r%d, but software "
			  "only supports up to version w%d/r%d", c->fmt_version,
			  c->ro_compat_version, UBIFS_FORMAT_VERSION,
			  UBIFS_RO_COMPAT_VERSION);
		return -EROFS;
	}

	mutex_lock(&c->umount_mutex);
	err = mount_ubifs(c);
	if (err) {
		ubifs_assert(err < 0);
		goto out_unlock;
	}

	/* Read the root inode */
	root = ubifs_iget(sb, UBIFS_ROOT_INO);
	if (IS_ERR(root)) {
		err = PTR_ERR(root);
		goto out_umount;
	}

	sb->s_root = NULL;

	mutex_unlock(&c->umount_mutex);
	return 0;

out_umount:
	ubifs_umount(c);
out_unlock:
	mutex_unlock(&c->umount_mutex);
	ubi_close_volume(c->ubi);
out_free:
	kfree(c);
	return err;
}

static int sb_test(struct super_block *sb, void *data)
{
	dev_t *dev = data;

	return sb->s_dev == *dev;
}

static int ubifs_get_sb(struct file_system_type *fs_type, int flags,
			const char *name, void *data, struct vfsmount *mnt)
{
	struct ubi_volume_desc *ubi;
	struct ubi_volume_info vi;
	struct super_block *sb;
	int err;

	dbg_gen("name %s, flags %#x", name, flags);

	/*
	 * Get UBI device number and volume ID. Mount it read-only so far
	 * because this might be a new mount point, and UBI allows only one
	 * read-write user at a time.
	 */
	ubi = open_ubi(name, UBI_READONLY);
	if (IS_ERR(ubi)) {
		ubifs_err("cannot open \"%s\", error %d",
			  name, (int)PTR_ERR(ubi));
		return PTR_ERR(ubi);
	}
	ubi_get_volume_info(ubi, &vi);

	dbg_gen("opened ubi%d_%d", vi.ubi_num, vi.vol_id);

	sb = sget(fs_type, &sb_test, &sb_set, &vi.cdev);
	if (IS_ERR(sb)) {
		err = PTR_ERR(sb);
		goto out_close;
	}

	if (sb->s_root) {
		/* A new mount point for already mounted UBIFS */
		dbg_gen("this ubi volume is already mounted");
		if ((flags ^ sb->s_flags) & MS_RDONLY) {
			err = -EBUSY;
			goto out_deact;
		}
	} else {
		sb->s_flags = flags;
		/*
		 * Pass 'ubi' to 'fill_super()' in sb->s_fs_info where it is
		 * replaced by 'c'.
		 */
		sb->s_fs_info = ubi;
		err = ubifs_fill_super(sb, data, flags & MS_SILENT ? 1 : 0);
		if (err)
			goto out_deact;
		/* We do not support atime */
		sb->s_flags |= MS_ACTIVE | MS_NOATIME;
	}

	/* 'fill_super()' opens ubi again so we must close it here */
	ubi_close_volume(ubi);

	ubifs_sb = sb;
	return 0;

out_deact:
	up_write(&sb->s_umount);
out_close:
	ubi_close_volume(ubi);
	return err;
}

int __init ubifs_init(void)
{
	int err;

	BUILD_BUG_ON(sizeof(struct ubifs_ch) != 24);

	/* Make sure node sizes are 8-byte aligned */
	BUILD_BUG_ON(UBIFS_CH_SZ        & 7);
	BUILD_BUG_ON(UBIFS_INO_NODE_SZ  & 7);
	BUILD_BUG_ON(UBIFS_DENT_NODE_SZ & 7);
	BUILD_BUG_ON(UBIFS_XENT_NODE_SZ & 7);
	BUILD_BUG_ON(UBIFS_DATA_NODE_SZ & 7);
	BUILD_BUG_ON(UBIFS_TRUN_NODE_SZ & 7);
	BUILD_BUG_ON(UBIFS_SB_NODE_SZ   & 7);
	BUILD_BUG_ON(UBIFS_MST_NODE_SZ  & 7);
	BUILD_BUG_ON(UBIFS_REF_NODE_SZ  & 7);
	BUILD_BUG_ON(UBIFS_CS_NODE_SZ   & 7);
	BUILD_BUG_ON(UBIFS_ORPH_NODE_SZ & 7);

	BUILD_BUG_ON(UBIFS_MAX_DENT_NODE_SZ & 7);
	BUILD_BUG_ON(UBIFS_MAX_XENT_NODE_SZ & 7);
	BUILD_BUG_ON(UBIFS_MAX_DATA_NODE_SZ & 7);
	BUILD_BUG_ON(UBIFS_MAX_INO_NODE_SZ  & 7);
	BUILD_BUG_ON(UBIFS_MAX_NODE_SZ      & 7);
	BUILD_BUG_ON(MIN_WRITE_SZ           & 7);

	/* Check min. node size */
	BUILD_BUG_ON(UBIFS_INO_NODE_SZ  < MIN_WRITE_SZ);
	BUILD_BUG_ON(UBIFS_DENT_NODE_SZ < MIN_WRITE_SZ);
	BUILD_BUG_ON(UBIFS_XENT_NODE_SZ < MIN_WRITE_SZ);
	BUILD_BUG_ON(UBIFS_TRUN_NODE_SZ < MIN_WRITE_SZ);

	BUILD_BUG_ON(UBIFS_MAX_DENT_NODE_SZ > UBIFS_MAX_NODE_SZ);
	BUILD_BUG_ON(UBIFS_MAX_XENT_NODE_SZ > UBIFS_MAX_NODE_SZ);
	BUILD_BUG_ON(UBIFS_MAX_DATA_NODE_SZ > UBIFS_MAX_NODE_SZ);
	BUILD_BUG_ON(UBIFS_MAX_INO_NODE_SZ  > UBIFS_MAX_NODE_SZ);

	/* Defined node sizes */
	BUILD_BUG_ON(UBIFS_SB_NODE_SZ  != 4096);
	BUILD_BUG_ON(UBIFS_MST_NODE_SZ != 512);
	BUILD_BUG_ON(UBIFS_INO_NODE_SZ != 160);
	BUILD_BUG_ON(UBIFS_REF_NODE_SZ != 64);

	/*
	 * We use 2 bit wide bit-fields to store compression type, which should
	 * be amended if more compressors are added. The bit-fields are:
	 * @compr_type in 'struct ubifs_inode', @default_compr in
	 * 'struct ubifs_info' and @compr_type in 'struct ubifs_mount_opts'.
	 */
	BUILD_BUG_ON(UBIFS_COMPR_TYPES_CNT > 4);

	/*
	 * We require that PAGE_CACHE_SIZE is greater-than-or-equal-to
	 * UBIFS_BLOCK_SIZE. It is assumed that both are powers of 2.
	 */
	if (PAGE_CACHE_SIZE < UBIFS_BLOCK_SIZE) {
		ubifs_err("VFS page cache size is %u bytes, but UBIFS requires"
			  " at least 4096 bytes",
			  (unsigned int)PAGE_CACHE_SIZE);
		return -EINVAL;
	}

	err = -ENOMEM;

	err = ubifs_compressors_init();
	if (err)
		goto out_shrinker;

	return 0;

out_shrinker:
	return err;
}

/*
 * ubifsmount...
 */

static struct file_system_type ubifs_fs_type = {
	.name    = "ubifs",
	.owner   = THIS_MODULE,
	.get_sb  = ubifs_get_sb,
};

int ubifs_mount(char *name)
{
	int flags;
	void *data;
	struct vfsmount *mnt;
	int ret;
	struct ubifs_info *c;

	/*
	 * First unmount if allready mounted
	 */
	if (ubifs_sb)
		ubifs_umount(ubifs_sb->s_fs_info);

	INIT_LIST_HEAD(&ubifs_infos);
	INIT_LIST_HEAD(&ubifs_fs_type.fs_supers);

	/*
	 * Mount in read-only mode
	 */
	flags = MS_RDONLY;
	data = NULL;
	mnt = NULL;
	ret = ubifs_get_sb(&ubifs_fs_type, flags, name, data, mnt);
	if (ret) {
		ubifs_err("Error reading superblock on volume '%s' errno=%d!\n", name, ret);
		return -1;
	}

	c = ubifs_sb->s_fs_info;
	ubi_close_volume(c->ubi);

	return 0;
}
