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
 * Author: Adrian Hunter
 */

#include "ubifs.h"

/*
 * An orphan is an inode number whose inode node has been committed to the index
 * with a link count of zero. That happens when an open file is deleted
 * (unlinked) and then a commit is run. In the normal course of events the inode
 * would be deleted when the file is closed. However in the case of an unclean
 * unmount, orphans need to be accounted for. After an unclean unmount, the
 * orphans' inodes must be deleted which means either scanning the entire index
 * looking for them, or keeping a list on flash somewhere. This unit implements
 * the latter approach.
 *
 * The orphan area is a fixed number of LEBs situated between the LPT area and
 * the main area. The number of orphan area LEBs is specified when the file
 * system is created. The minimum number is 1. The size of the orphan area
 * should be so that it can hold the maximum number of orphans that are expected
 * to ever exist at one time.
 *
 * The number of orphans that can fit in a LEB is:
 *
 *         (c->leb_size - UBIFS_ORPH_NODE_SZ) / sizeof(__le64)
 *
 * For example: a 15872 byte LEB can fit 1980 orphans so 1 LEB may be enough.
 *
 * Orphans are accumulated in a rb-tree. When an inode's link count drops to
 * zero, the inode number is added to the rb-tree. It is removed from the tree
 * when the inode is deleted.  Any new orphans that are in the orphan tree when
 * the commit is run, are written to the orphan area in 1 or more orphan nodes.
 * If the orphan area is full, it is consolidated to make space.  There is
 * always enough space because validation prevents the user from creating more
 * than the maximum number of orphans allowed.
 */

/**
 * tot_avail_orphs - calculate total space.
 * @c: UBIFS file-system description object
 *
 * This function returns the number of orphans that can be written in half
 * the total space. That leaves half the space for adding new orphans.
 */
static int tot_avail_orphs(struct ubifs_info *c)
{
	int avail_lebs, avail;

	avail_lebs = c->orph_lebs;
	avail = avail_lebs *
	       ((c->leb_size - UBIFS_ORPH_NODE_SZ) / sizeof(__le64));
	return avail / 2;
}

/**
 * ubifs_clear_orphans - erase all LEBs used for orphans.
 * @c: UBIFS file-system description object
 *
 * If recovery is not required, then the orphans from the previous session
 * are not needed. This function locates the LEBs used to record
 * orphans, and un-maps them.
 */
int ubifs_clear_orphans(struct ubifs_info *c)
{
	int lnum, err;

	for (lnum = c->orph_first; lnum <= c->orph_last; lnum++) {
		err = ubifs_leb_unmap(c, lnum);
		if (err)
			return err;
	}
	c->ohead_lnum = c->orph_first;
	c->ohead_offs = 0;
	return 0;
}

/**
 * insert_dead_orphan - insert an orphan.
 * @c: UBIFS file-system description object
 * @inum: orphan inode number
 *
 * This function is a helper to the 'do_kill_orphans()' function. The orphan
 * must be kept until the next commit, so it is added to the rb-tree and the
 * deletion list.
 */
static int insert_dead_orphan(struct ubifs_info *c, ino_t inum)
{
	struct ubifs_orphan *orphan, *o;
	struct rb_node **p, *parent = NULL;

	orphan = kzalloc(sizeof(struct ubifs_orphan), GFP_KERNEL);
	if (!orphan)
		return -ENOMEM;
	orphan->inum = inum;

	p = &c->orph_tree.rb_node;
	while (*p) {
		parent = *p;
		o = rb_entry(parent, struct ubifs_orphan, rb);
		if (inum < o->inum)
			p = &(*p)->rb_left;
		else if (inum > o->inum)
			p = &(*p)->rb_right;
		else {
			/* Already added - no problem */
			kfree(orphan);
			return 0;
		}
	}
	c->tot_orphans += 1;
	rb_link_node(&orphan->rb, parent, p);
	rb_insert_color(&orphan->rb, &c->orph_tree);
	list_add_tail(&orphan->list, &c->orph_list);
	orphan->dnext = c->orph_dnext;
	c->orph_dnext = orphan;
	dbg_mnt("ino %lu, new %d, tot %d", (unsigned long)inum,
		c->new_orphans, c->tot_orphans);
	return 0;
}

/**
 * do_kill_orphans - remove orphan inodes from the index.
 * @c: UBIFS file-system description object
 * @sleb: scanned LEB
 * @last_cmt_no: cmt_no of last orphan node read is passed and returned here
 * @outofdate: whether the LEB is out of date is returned here
 * @last_flagged: whether the end orphan node is encountered
 *
 * This function is a helper to the 'kill_orphans()' function. It goes through
 * every orphan node in a LEB and for every inode number recorded, removes
 * all keys for that inode from the TNC.
 */
static int do_kill_orphans(struct ubifs_info *c, struct ubifs_scan_leb *sleb,
			   unsigned long long *last_cmt_no, int *outofdate,
			   int *last_flagged)
{
	struct ubifs_scan_node *snod;
	struct ubifs_orph_node *orph;
	unsigned long long cmt_no;
	ino_t inum;
	int i, n, err, first = 1;

	list_for_each_entry(snod, &sleb->nodes, list) {
		if (snod->type != UBIFS_ORPH_NODE) {
			ubifs_err("invalid node type %d in orphan area at "
				  "%d:%d", snod->type, sleb->lnum, snod->offs);
			dbg_dump_node(c, snod->node);
			return -EINVAL;
		}

		orph = snod->node;

		/* Check commit number */
		cmt_no = le64_to_cpu(orph->cmt_no) & LLONG_MAX;
		/*
		 * The commit number on the master node may be less, because
		 * of a failed commit. If there are several failed commits in a
		 * row, the commit number written on orphan nodes will continue
		 * to increase (because the commit number is adjusted here) even
		 * though the commit number on the master node stays the same
		 * because the master node has not been re-written.
		 */
		if (cmt_no > c->cmt_no)
			c->cmt_no = cmt_no;
		if (cmt_no < *last_cmt_no && *last_flagged) {
			/*
			 * The last orphan node had a higher commit number and
			 * was flagged as the last written for that commit
			 * number. That makes this orphan node, out of date.
			 */
			if (!first) {
				ubifs_err("out of order commit number %llu in "
					  "orphan node at %d:%d",
					  cmt_no, sleb->lnum, snod->offs);
				dbg_dump_node(c, snod->node);
				return -EINVAL;
			}
			dbg_rcvry("out of date LEB %d", sleb->lnum);
			*outofdate = 1;
			return 0;
		}

		if (first)
			first = 0;

		n = (le32_to_cpu(orph->ch.len) - UBIFS_ORPH_NODE_SZ) >> 3;
		for (i = 0; i < n; i++) {
			inum = le64_to_cpu(orph->inos[i]);
			dbg_rcvry("deleting orphaned inode %lu",
				  (unsigned long)inum);
			err = ubifs_tnc_remove_ino(c, inum);
			if (err)
				return err;
			err = insert_dead_orphan(c, inum);
			if (err)
				return err;
		}

		*last_cmt_no = cmt_no;
		if (le64_to_cpu(orph->cmt_no) & (1ULL << 63)) {
			dbg_rcvry("last orph node for commit %llu at %d:%d",
				  cmt_no, sleb->lnum, snod->offs);
			*last_flagged = 1;
		} else
			*last_flagged = 0;
	}

	return 0;
}

/**
 * kill_orphans - remove all orphan inodes from the index.
 * @c: UBIFS file-system description object
 *
 * If recovery is required, then orphan inodes recorded during the previous
 * session (which ended with an unclean unmount) must be deleted from the index.
 * This is done by updating the TNC, but since the index is not updated until
 * the next commit, the LEBs where the orphan information is recorded are not
 * erased until the next commit.
 */
static int kill_orphans(struct ubifs_info *c)
{
	unsigned long long last_cmt_no = 0;
	int lnum, err = 0, outofdate = 0, last_flagged = 0;

	c->ohead_lnum = c->orph_first;
	c->ohead_offs = 0;
	/* Check no-orphans flag and skip this if no orphans */
	if (c->no_orphs) {
		dbg_rcvry("no orphans");
		return 0;
	}
	/*
	 * Orph nodes always start at c->orph_first and are written to each
	 * successive LEB in turn. Generally unused LEBs will have been unmapped
	 * but may contain out of date orphan nodes if the unmap didn't go
	 * through. In addition, the last orphan node written for each commit is
	 * marked (top bit of orph->cmt_no is set to 1). It is possible that
	 * there are orphan nodes from the next commit (i.e. the commit did not
	 * complete successfully). In that case, no orphans will have been lost
	 * due to the way that orphans are written, and any orphans added will
	 * be valid orphans anyway and so can be deleted.
	 */
	for (lnum = c->orph_first; lnum <= c->orph_last; lnum++) {
		struct ubifs_scan_leb *sleb;

		dbg_rcvry("LEB %d", lnum);
		sleb = ubifs_scan(c, lnum, 0, c->sbuf);
		if (IS_ERR(sleb)) {
			sleb = ubifs_recover_leb(c, lnum, 0, c->sbuf, 0);
			if (IS_ERR(sleb)) {
				err = PTR_ERR(sleb);
				break;
			}
		}
		err = do_kill_orphans(c, sleb, &last_cmt_no, &outofdate,
				      &last_flagged);
		if (err || outofdate) {
			ubifs_scan_destroy(sleb);
			break;
		}
		if (sleb->endpt) {
			c->ohead_lnum = lnum;
			c->ohead_offs = sleb->endpt;
		}
		ubifs_scan_destroy(sleb);
	}
	return err;
}

/**
 * ubifs_mount_orphans - delete orphan inodes and erase LEBs that recorded them.
 * @c: UBIFS file-system description object
 * @unclean: indicates recovery from unclean unmount
 * @read_only: indicates read only mount
 *
 * This function is called when mounting to erase orphans from the previous
 * session. If UBIFS was not unmounted cleanly, then the inodes recorded as
 * orphans are deleted.
 */
int ubifs_mount_orphans(struct ubifs_info *c, int unclean, int read_only)
{
	int err = 0;

	c->max_orphans = tot_avail_orphs(c);

	if (!read_only) {
		c->orph_buf = vmalloc(c->leb_size);
		if (!c->orph_buf)
			return -ENOMEM;
	}

	if (unclean)
		err = kill_orphans(c);
	else if (!read_only)
		err = ubifs_clear_orphans(c);

	return err;
}
