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
 * Authors: Adrian Hunter
 *          Artem Bityutskiy (Битюцкий Артём)
 */

/*
 * This file implements the LEB properties tree (LPT) area. The LPT area
 * contains the LEB properties tree, a table of LPT area eraseblocks (ltab), and
 * (for the "big" model) a table of saved LEB numbers (lsave). The LPT area sits
 * between the log and the orphan area.
 *
 * The LPT area is like a miniature self-contained file system. It is required
 * that it never runs out of space, is fast to access and update, and scales
 * logarithmically. The LEB properties tree is implemented as a wandering tree
 * much like the TNC, and the LPT area has its own garbage collection.
 *
 * The LPT has two slightly different forms called the "small model" and the
 * "big model". The small model is used when the entire LEB properties table
 * can be written into a single eraseblock. In that case, garbage collection
 * consists of just writing the whole table, which therefore makes all other
 * eraseblocks reusable. In the case of the big model, dirty eraseblocks are
 * selected for garbage collection, which consists of marking the clean nodes in
 * that LEB as dirty, and then only the dirty nodes are written out. Also, in
 * the case of the big model, a table of LEB numbers is saved so that the entire
 * LPT does not to be scanned looking for empty eraseblocks when UBIFS is first
 * mounted.
 */

#include "ubifs.h"
#include "crc16.h"
#include <linux/math64.h>

/**
 * do_calc_lpt_geom - calculate sizes for the LPT area.
 * @c: the UBIFS file-system description object
 *
 * Calculate the sizes of LPT bit fields, nodes, and tree, based on the
 * properties of the flash and whether LPT is "big" (c->big_lpt).
 */
static void do_calc_lpt_geom(struct ubifs_info *c)
{
	int i, n, bits, per_leb_wastage, max_pnode_cnt;
	long long sz, tot_wastage;

	n = c->main_lebs + c->max_leb_cnt - c->leb_cnt;
	max_pnode_cnt = DIV_ROUND_UP(n, UBIFS_LPT_FANOUT);

	c->lpt_hght = 1;
	n = UBIFS_LPT_FANOUT;
	while (n < max_pnode_cnt) {
		c->lpt_hght += 1;
		n <<= UBIFS_LPT_FANOUT_SHIFT;
	}

	c->pnode_cnt = DIV_ROUND_UP(c->main_lebs, UBIFS_LPT_FANOUT);

	n = DIV_ROUND_UP(c->pnode_cnt, UBIFS_LPT_FANOUT);
	c->nnode_cnt = n;
	for (i = 1; i < c->lpt_hght; i++) {
		n = DIV_ROUND_UP(n, UBIFS_LPT_FANOUT);
		c->nnode_cnt += n;
	}

	c->space_bits = fls(c->leb_size) - 3;
	c->lpt_lnum_bits = fls(c->lpt_lebs);
	c->lpt_offs_bits = fls(c->leb_size - 1);
	c->lpt_spc_bits = fls(c->leb_size);

	n = DIV_ROUND_UP(c->max_leb_cnt, UBIFS_LPT_FANOUT);
	c->pcnt_bits = fls(n - 1);

	c->lnum_bits = fls(c->max_leb_cnt - 1);

	bits = UBIFS_LPT_CRC_BITS + UBIFS_LPT_TYPE_BITS +
	       (c->big_lpt ? c->pcnt_bits : 0) +
	       (c->space_bits * 2 + 1) * UBIFS_LPT_FANOUT;
	c->pnode_sz = (bits + 7) / 8;

	bits = UBIFS_LPT_CRC_BITS + UBIFS_LPT_TYPE_BITS +
	       (c->big_lpt ? c->pcnt_bits : 0) +
	       (c->lpt_lnum_bits + c->lpt_offs_bits) * UBIFS_LPT_FANOUT;
	c->nnode_sz = (bits + 7) / 8;

	bits = UBIFS_LPT_CRC_BITS + UBIFS_LPT_TYPE_BITS +
	       c->lpt_lebs * c->lpt_spc_bits * 2;
	c->ltab_sz = (bits + 7) / 8;

	bits = UBIFS_LPT_CRC_BITS + UBIFS_LPT_TYPE_BITS +
	       c->lnum_bits * c->lsave_cnt;
	c->lsave_sz = (bits + 7) / 8;

	/* Calculate the minimum LPT size */
	c->lpt_sz = (long long)c->pnode_cnt * c->pnode_sz;
	c->lpt_sz += (long long)c->nnode_cnt * c->nnode_sz;
	c->lpt_sz += c->ltab_sz;
	if (c->big_lpt)
		c->lpt_sz += c->lsave_sz;

	/* Add wastage */
	sz = c->lpt_sz;
	per_leb_wastage = max_t(int, c->pnode_sz, c->nnode_sz);
	sz += per_leb_wastage;
	tot_wastage = per_leb_wastage;
	while (sz > c->leb_size) {
		sz += per_leb_wastage;
		sz -= c->leb_size;
		tot_wastage += per_leb_wastage;
	}
	tot_wastage += ALIGN(sz, c->min_io_size) - sz;
	c->lpt_sz += tot_wastage;
}

/**
 * ubifs_calc_lpt_geom - calculate and check sizes for the LPT area.
 * @c: the UBIFS file-system description object
 *
 * This function returns %0 on success and a negative error code on failure.
 */
int ubifs_calc_lpt_geom(struct ubifs_info *c)
{
	int lebs_needed;
	long long sz;

	do_calc_lpt_geom(c);

	/* Verify that lpt_lebs is big enough */
	sz = c->lpt_sz * 2; /* Must have at least 2 times the size */
	lebs_needed = div_u64(sz + c->leb_size - 1, c->leb_size);
	if (lebs_needed > c->lpt_lebs) {
		ubifs_err("too few LPT LEBs");
		return -EINVAL;
	}

	/* Verify that ltab fits in a single LEB (since ltab is a single node */
	if (c->ltab_sz > c->leb_size) {
		ubifs_err("LPT ltab too big");
		return -EINVAL;
	}

	c->check_lpt_free = c->big_lpt;
	return 0;
}

/**
 * ubifs_unpack_bits - unpack bit fields.
 * @addr: address at which to unpack (passed and next address returned)
 * @pos: bit position at which to unpack (passed and next position returned)
 * @nrbits: number of bits of value to unpack (1-32)
 *
 * This functions returns the value unpacked.
 */
uint32_t ubifs_unpack_bits(uint8_t **addr, int *pos, int nrbits)
{
	const int k = 32 - nrbits;
	uint8_t *p = *addr;
	int b = *pos;
	uint32_t uninitialized_var(val);
	const int bytes = (nrbits + b + 7) >> 3;

	ubifs_assert(nrbits > 0);
	ubifs_assert(nrbits <= 32);
	ubifs_assert(*pos >= 0);
	ubifs_assert(*pos < 8);
	if (b) {
		switch (bytes) {
		case 2:
			val = p[1];
			break;
		case 3:
			val = p[1] | ((uint32_t)p[2] << 8);
			break;
		case 4:
			val = p[1] | ((uint32_t)p[2] << 8) |
				     ((uint32_t)p[3] << 16);
			break;
		case 5:
			val = p[1] | ((uint32_t)p[2] << 8) |
				     ((uint32_t)p[3] << 16) |
				     ((uint32_t)p[4] << 24);
		}
		val <<= (8 - b);
		val |= *p >> b;
		nrbits += b;
	} else {
		switch (bytes) {
		case 1:
			val = p[0];
			break;
		case 2:
			val = p[0] | ((uint32_t)p[1] << 8);
			break;
		case 3:
			val = p[0] | ((uint32_t)p[1] << 8) |
				     ((uint32_t)p[2] << 16);
			break;
		case 4:
			val = p[0] | ((uint32_t)p[1] << 8) |
				     ((uint32_t)p[2] << 16) |
				     ((uint32_t)p[3] << 24);
			break;
		}
	}
	val <<= k;
	val >>= k;
	b = nrbits & 7;
	p += nrbits >> 3;
	*addr = p;
	*pos = b;
	ubifs_assert((val >> nrbits) == 0 || nrbits - b == 32);
	return val;
}

/**
 * ubifs_add_lpt_dirt - add dirty space to LPT LEB properties.
 * @c: UBIFS file-system description object
 * @lnum: LEB number to which to add dirty space
 * @dirty: amount of dirty space to add
 */
void ubifs_add_lpt_dirt(struct ubifs_info *c, int lnum, int dirty)
{
	if (!dirty || !lnum)
		return;
	dbg_lp("LEB %d add %d to %d",
	       lnum, dirty, c->ltab[lnum - c->lpt_first].dirty);
	ubifs_assert(lnum >= c->lpt_first && lnum <= c->lpt_last);
	c->ltab[lnum - c->lpt_first].dirty += dirty;
}

/**
 * ubifs_add_nnode_dirt - add dirty space to LPT LEB properties.
 * @c: UBIFS file-system description object
 * @nnode: nnode for which to add dirt
 */
void ubifs_add_nnode_dirt(struct ubifs_info *c, struct ubifs_nnode *nnode)
{
	struct ubifs_nnode *np = nnode->parent;

	if (np)
		ubifs_add_lpt_dirt(c, np->nbranch[nnode->iip].lnum,
				   c->nnode_sz);
	else {
		ubifs_add_lpt_dirt(c, c->lpt_lnum, c->nnode_sz);
		if (!(c->lpt_drty_flgs & LTAB_DIRTY)) {
			c->lpt_drty_flgs |= LTAB_DIRTY;
			ubifs_add_lpt_dirt(c, c->ltab_lnum, c->ltab_sz);
		}
	}
}

/**
 * add_pnode_dirt - add dirty space to LPT LEB properties.
 * @c: UBIFS file-system description object
 * @pnode: pnode for which to add dirt
 */
static void add_pnode_dirt(struct ubifs_info *c, struct ubifs_pnode *pnode)
{
	ubifs_add_lpt_dirt(c, pnode->parent->nbranch[pnode->iip].lnum,
			   c->pnode_sz);
}

/**
 * calc_nnode_num_from_parent - calculate nnode number.
 * @c: UBIFS file-system description object
 * @parent: parent nnode
 * @iip: index in parent
 *
 * The nnode number is a number that uniquely identifies a nnode and can be used
 * easily to traverse the tree from the root to that nnode.
 *
 * This function calculates and returns the nnode number based on the parent's
 * nnode number and the index in parent.
 */
static int calc_nnode_num_from_parent(const struct ubifs_info *c,
				      struct ubifs_nnode *parent, int iip)
{
	int num, shft;

	if (!parent)
		return 1;
	shft = (c->lpt_hght - parent->level) * UBIFS_LPT_FANOUT_SHIFT;
	num = parent->num ^ (1 << shft);
	num |= (UBIFS_LPT_FANOUT + iip) << shft;
	return num;
}

/**
 * calc_pnode_num_from_parent - calculate pnode number.
 * @c: UBIFS file-system description object
 * @parent: parent nnode
 * @iip: index in parent
 *
 * The pnode number is a number that uniquely identifies a pnode and can be used
 * easily to traverse the tree from the root to that pnode.
 *
 * This function calculates and returns the pnode number based on the parent's
 * nnode number and the index in parent.
 */
static int calc_pnode_num_from_parent(const struct ubifs_info *c,
				      struct ubifs_nnode *parent, int iip)
{
	int i, n = c->lpt_hght - 1, pnum = parent->num, num = 0;

	for (i = 0; i < n; i++) {
		num <<= UBIFS_LPT_FANOUT_SHIFT;
		num |= pnum & (UBIFS_LPT_FANOUT - 1);
		pnum >>= UBIFS_LPT_FANOUT_SHIFT;
	}
	num <<= UBIFS_LPT_FANOUT_SHIFT;
	num |= iip;
	return num;
}

/**
 * update_cats - add LEB properties of a pnode to LEB category lists and heaps.
 * @c: UBIFS file-system description object
 * @pnode: pnode
 *
 * When a pnode is loaded into memory, the LEB properties it contains are added,
 * by this function, to the LEB category lists and heaps.
 */
static void update_cats(struct ubifs_info *c, struct ubifs_pnode *pnode)
{
	int i;

	for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
		int cat = pnode->lprops[i].flags & LPROPS_CAT_MASK;
		int lnum = pnode->lprops[i].lnum;

		if (!lnum)
			return;
		ubifs_add_to_cat(c, &pnode->lprops[i], cat);
	}
}

/**
 * replace_cats - add LEB properties of a pnode to LEB category lists and heaps.
 * @c: UBIFS file-system description object
 * @old_pnode: pnode copied
 * @new_pnode: pnode copy
 *
 * During commit it is sometimes necessary to copy a pnode
 * (see dirty_cow_pnode).  When that happens, references in
 * category lists and heaps must be replaced.  This function does that.
 */
static void replace_cats(struct ubifs_info *c, struct ubifs_pnode *old_pnode,
			 struct ubifs_pnode *new_pnode)
{
	int i;

	for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
		if (!new_pnode->lprops[i].lnum)
			return;
		ubifs_replace_cat(c, &old_pnode->lprops[i],
				  &new_pnode->lprops[i]);
	}
}

/**
 * check_lpt_crc - check LPT node crc is correct.
 * @c: UBIFS file-system description object
 * @buf: buffer containing node
 * @len: length of node
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int check_lpt_crc(void *buf, int len)
{
	int pos = 0;
	uint8_t *addr = buf;
	uint16_t crc, calc_crc;

	crc = ubifs_unpack_bits(&addr, &pos, UBIFS_LPT_CRC_BITS);
	calc_crc = crc16(-1, buf + UBIFS_LPT_CRC_BYTES,
			 len - UBIFS_LPT_CRC_BYTES);
	if (crc != calc_crc) {
		ubifs_err("invalid crc in LPT node: crc %hx calc %hx", crc,
			  calc_crc);
		dbg_dump_stack();
		return -EINVAL;
	}
	return 0;
}

/**
 * check_lpt_type - check LPT node type is correct.
 * @c: UBIFS file-system description object
 * @addr: address of type bit field is passed and returned updated here
 * @pos: position of type bit field is passed and returned updated here
 * @type: expected type
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int check_lpt_type(uint8_t **addr, int *pos, int type)
{
	int node_type;

	node_type = ubifs_unpack_bits(addr, pos, UBIFS_LPT_TYPE_BITS);
	if (node_type != type) {
		ubifs_err("invalid type (%d) in LPT node type %d", node_type,
			  type);
		dbg_dump_stack();
		return -EINVAL;
	}
	return 0;
}

/**
 * unpack_pnode - unpack a pnode.
 * @c: UBIFS file-system description object
 * @buf: buffer containing packed pnode to unpack
 * @pnode: pnode structure to fill
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int unpack_pnode(const struct ubifs_info *c, void *buf,
			struct ubifs_pnode *pnode)
{
	uint8_t *addr = buf + UBIFS_LPT_CRC_BYTES;
	int i, pos = 0, err;

	err = check_lpt_type(&addr, &pos, UBIFS_LPT_PNODE);
	if (err)
		return err;
	if (c->big_lpt)
		pnode->num = ubifs_unpack_bits(&addr, &pos, c->pcnt_bits);
	for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
		struct ubifs_lprops * const lprops = &pnode->lprops[i];

		lprops->free = ubifs_unpack_bits(&addr, &pos, c->space_bits);
		lprops->free <<= 3;
		lprops->dirty = ubifs_unpack_bits(&addr, &pos, c->space_bits);
		lprops->dirty <<= 3;

		if (ubifs_unpack_bits(&addr, &pos, 1))
			lprops->flags = LPROPS_INDEX;
		else
			lprops->flags = 0;
		lprops->flags |= ubifs_categorize_lprops(c, lprops);
	}
	err = check_lpt_crc(buf, c->pnode_sz);
	return err;
}

/**
 * ubifs_unpack_nnode - unpack a nnode.
 * @c: UBIFS file-system description object
 * @buf: buffer containing packed nnode to unpack
 * @nnode: nnode structure to fill
 *
 * This function returns %0 on success and a negative error code on failure.
 */
int ubifs_unpack_nnode(const struct ubifs_info *c, void *buf,
		       struct ubifs_nnode *nnode)
{
	uint8_t *addr = buf + UBIFS_LPT_CRC_BYTES;
	int i, pos = 0, err;

	err = check_lpt_type(&addr, &pos, UBIFS_LPT_NNODE);
	if (err)
		return err;
	if (c->big_lpt)
		nnode->num = ubifs_unpack_bits(&addr, &pos, c->pcnt_bits);
	for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
		int lnum;

		lnum = ubifs_unpack_bits(&addr, &pos, c->lpt_lnum_bits) +
		       c->lpt_first;
		if (lnum == c->lpt_last + 1)
			lnum = 0;
		nnode->nbranch[i].lnum = lnum;
		nnode->nbranch[i].offs = ubifs_unpack_bits(&addr, &pos,
						     c->lpt_offs_bits);
	}
	err = check_lpt_crc(buf, c->nnode_sz);
	return err;
}

/**
 * unpack_ltab - unpack the LPT's own lprops table.
 * @c: UBIFS file-system description object
 * @buf: buffer from which to unpack
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int unpack_ltab(const struct ubifs_info *c, void *buf)
{
	uint8_t *addr = buf + UBIFS_LPT_CRC_BYTES;
	int i, pos = 0, err;

	err = check_lpt_type(&addr, &pos, UBIFS_LPT_LTAB);
	if (err)
		return err;
	for (i = 0; i < c->lpt_lebs; i++) {
		int free = ubifs_unpack_bits(&addr, &pos, c->lpt_spc_bits);
		int dirty = ubifs_unpack_bits(&addr, &pos, c->lpt_spc_bits);

		if (free < 0 || free > c->leb_size || dirty < 0 ||
		    dirty > c->leb_size || free + dirty > c->leb_size)
			return -EINVAL;

		c->ltab[i].free = free;
		c->ltab[i].dirty = dirty;
		c->ltab[i].tgc = 0;
		c->ltab[i].cmt = 0;
	}
	err = check_lpt_crc(buf, c->ltab_sz);
	return err;
}

/**
 * validate_nnode - validate a nnode.
 * @c: UBIFS file-system description object
 * @nnode: nnode to validate
 * @parent: parent nnode (or NULL for the root nnode)
 * @iip: index in parent
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int validate_nnode(const struct ubifs_info *c, struct ubifs_nnode *nnode,
			  struct ubifs_nnode *parent, int iip)
{
	int i, lvl, max_offs;

	if (c->big_lpt) {
		int num = calc_nnode_num_from_parent(c, parent, iip);

		if (nnode->num != num)
			return -EINVAL;
	}
	lvl = parent ? parent->level - 1 : c->lpt_hght;
	if (lvl < 1)
		return -EINVAL;
	if (lvl == 1)
		max_offs = c->leb_size - c->pnode_sz;
	else
		max_offs = c->leb_size - c->nnode_sz;
	for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
		int lnum = nnode->nbranch[i].lnum;
		int offs = nnode->nbranch[i].offs;

		if (lnum == 0) {
			if (offs != 0)
				return -EINVAL;
			continue;
		}
		if (lnum < c->lpt_first || lnum > c->lpt_last)
			return -EINVAL;
		if (offs < 0 || offs > max_offs)
			return -EINVAL;
	}
	return 0;
}

/**
 * validate_pnode - validate a pnode.
 * @c: UBIFS file-system description object
 * @pnode: pnode to validate
 * @parent: parent nnode
 * @iip: index in parent
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int validate_pnode(const struct ubifs_info *c, struct ubifs_pnode *pnode,
			  struct ubifs_nnode *parent, int iip)
{
	int i;

	if (c->big_lpt) {
		int num = calc_pnode_num_from_parent(c, parent, iip);

		if (pnode->num != num)
			return -EINVAL;
	}
	for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
		int free = pnode->lprops[i].free;
		int dirty = pnode->lprops[i].dirty;

		if (free < 0 || free > c->leb_size || free % c->min_io_size ||
		    (free & 7))
			return -EINVAL;
		if (dirty < 0 || dirty > c->leb_size || (dirty & 7))
			return -EINVAL;
		if (dirty + free > c->leb_size)
			return -EINVAL;
	}
	return 0;
}

/**
 * set_pnode_lnum - set LEB numbers on a pnode.
 * @c: UBIFS file-system description object
 * @pnode: pnode to update
 *
 * This function calculates the LEB numbers for the LEB properties it contains
 * based on the pnode number.
 */
static void set_pnode_lnum(const struct ubifs_info *c,
			   struct ubifs_pnode *pnode)
{
	int i, lnum;

	lnum = (pnode->num << UBIFS_LPT_FANOUT_SHIFT) + c->main_first;
	for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
		if (lnum >= c->leb_cnt)
			return;
		pnode->lprops[i].lnum = lnum++;
	}
}

/**
 * ubifs_read_nnode - read a nnode from flash and link it to the tree in memory.
 * @c: UBIFS file-system description object
 * @parent: parent nnode (or NULL for the root)
 * @iip: index in parent
 *
 * This function returns %0 on success and a negative error code on failure.
 */
int ubifs_read_nnode(struct ubifs_info *c, struct ubifs_nnode *parent, int iip)
{
	struct ubifs_nbranch *branch = NULL;
	struct ubifs_nnode *nnode = NULL;
	void *buf = c->lpt_nod_buf;
	int err, lnum, offs;

	if (parent) {
		branch = &parent->nbranch[iip];
		lnum = branch->lnum;
		offs = branch->offs;
	} else {
		lnum = c->lpt_lnum;
		offs = c->lpt_offs;
	}
	nnode = kzalloc(sizeof(struct ubifs_nnode), GFP_NOFS);
	if (!nnode) {
		err = -ENOMEM;
		goto out;
	}
	if (lnum == 0) {
		/*
		 * This nnode was not written which just means that the LEB
		 * properties in the subtree below it describe empty LEBs. We
		 * make the nnode as though we had read it, which in fact means
		 * doing almost nothing.
		 */
		if (c->big_lpt)
			nnode->num = calc_nnode_num_from_parent(c, parent, iip);
	} else {
		err = ubi_read(c->ubi, lnum, buf, offs, c->nnode_sz);
		if (err)
			goto out;
		err = ubifs_unpack_nnode(c, buf, nnode);
		if (err)
			goto out;
	}
	err = validate_nnode(c, nnode, parent, iip);
	if (err)
		goto out;
	if (!c->big_lpt)
		nnode->num = calc_nnode_num_from_parent(c, parent, iip);
	if (parent) {
		branch->nnode = nnode;
		nnode->level = parent->level - 1;
	} else {
		c->nroot = nnode;
		nnode->level = c->lpt_hght;
	}
	nnode->parent = parent;
	nnode->iip = iip;
	return 0;

out:
	ubifs_err("error %d reading nnode at %d:%d", err, lnum, offs);
	kfree(nnode);
	return err;
}

/**
 * read_pnode - read a pnode from flash and link it to the tree in memory.
 * @c: UBIFS file-system description object
 * @parent: parent nnode
 * @iip: index in parent
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int read_pnode(struct ubifs_info *c, struct ubifs_nnode *parent, int iip)
{
	struct ubifs_nbranch *branch;
	struct ubifs_pnode *pnode = NULL;
	void *buf = c->lpt_nod_buf;
	int err, lnum, offs;

	branch = &parent->nbranch[iip];
	lnum = branch->lnum;
	offs = branch->offs;
	pnode = kzalloc(sizeof(struct ubifs_pnode), GFP_NOFS);
	if (!pnode) {
		err = -ENOMEM;
		goto out;
	}
	if (lnum == 0) {
		/*
		 * This pnode was not written which just means that the LEB
		 * properties in it describe empty LEBs. We make the pnode as
		 * though we had read it.
		 */
		int i;

		if (c->big_lpt)
			pnode->num = calc_pnode_num_from_parent(c, parent, iip);
		for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
			struct ubifs_lprops * const lprops = &pnode->lprops[i];

			lprops->free = c->leb_size;
			lprops->flags = ubifs_categorize_lprops(c, lprops);
		}
	} else {
		err = ubi_read(c->ubi, lnum, buf, offs, c->pnode_sz);
		if (err)
			goto out;
		err = unpack_pnode(c, buf, pnode);
		if (err)
			goto out;
	}
	err = validate_pnode(c, pnode, parent, iip);
	if (err)
		goto out;
	if (!c->big_lpt)
		pnode->num = calc_pnode_num_from_parent(c, parent, iip);
	branch->pnode = pnode;
	pnode->parent = parent;
	pnode->iip = iip;
	set_pnode_lnum(c, pnode);
	c->pnodes_have += 1;
	return 0;

out:
	ubifs_err("error %d reading pnode at %d:%d", err, lnum, offs);
	dbg_dump_pnode(c, pnode, parent, iip);
	dbg_msg("calc num: %d", calc_pnode_num_from_parent(c, parent, iip));
	kfree(pnode);
	return err;
}

/**
 * read_ltab - read LPT's own lprops table.
 * @c: UBIFS file-system description object
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int read_ltab(struct ubifs_info *c)
{
	int err;
	void *buf;

	buf = vmalloc(c->ltab_sz);
	if (!buf)
		return -ENOMEM;
	err = ubi_read(c->ubi, c->ltab_lnum, buf, c->ltab_offs, c->ltab_sz);
	if (err)
		goto out;
	err = unpack_ltab(c, buf);
out:
	vfree(buf);
	return err;
}

/**
 * ubifs_get_nnode - get a nnode.
 * @c: UBIFS file-system description object
 * @parent: parent nnode (or NULL for the root)
 * @iip: index in parent
 *
 * This function returns a pointer to the nnode on success or a negative error
 * code on failure.
 */
struct ubifs_nnode *ubifs_get_nnode(struct ubifs_info *c,
				    struct ubifs_nnode *parent, int iip)
{
	struct ubifs_nbranch *branch;
	struct ubifs_nnode *nnode;
	int err;

	branch = &parent->nbranch[iip];
	nnode = branch->nnode;
	if (nnode)
		return nnode;
	err = ubifs_read_nnode(c, parent, iip);
	if (err)
		return ERR_PTR(err);
	return branch->nnode;
}

/**
 * ubifs_get_pnode - get a pnode.
 * @c: UBIFS file-system description object
 * @parent: parent nnode
 * @iip: index in parent
 *
 * This function returns a pointer to the pnode on success or a negative error
 * code on failure.
 */
struct ubifs_pnode *ubifs_get_pnode(struct ubifs_info *c,
				    struct ubifs_nnode *parent, int iip)
{
	struct ubifs_nbranch *branch;
	struct ubifs_pnode *pnode;
	int err;

	branch = &parent->nbranch[iip];
	pnode = branch->pnode;
	if (pnode)
		return pnode;
	err = read_pnode(c, parent, iip);
	if (err)
		return ERR_PTR(err);
	update_cats(c, branch->pnode);
	return branch->pnode;
}

/**
 * ubifs_lpt_lookup - lookup LEB properties in the LPT.
 * @c: UBIFS file-system description object
 * @lnum: LEB number to lookup
 *
 * This function returns a pointer to the LEB properties on success or a
 * negative error code on failure.
 */
struct ubifs_lprops *ubifs_lpt_lookup(struct ubifs_info *c, int lnum)
{
	int err, i, h, iip, shft;
	struct ubifs_nnode *nnode;
	struct ubifs_pnode *pnode;

	if (!c->nroot) {
		err = ubifs_read_nnode(c, NULL, 0);
		if (err)
			return ERR_PTR(err);
	}
	nnode = c->nroot;
	i = lnum - c->main_first;
	shft = c->lpt_hght * UBIFS_LPT_FANOUT_SHIFT;
	for (h = 1; h < c->lpt_hght; h++) {
		iip = ((i >> shft) & (UBIFS_LPT_FANOUT - 1));
		shft -= UBIFS_LPT_FANOUT_SHIFT;
		nnode = ubifs_get_nnode(c, nnode, iip);
		if (IS_ERR(nnode))
			return ERR_PTR(PTR_ERR(nnode));
	}
	iip = ((i >> shft) & (UBIFS_LPT_FANOUT - 1));
	shft -= UBIFS_LPT_FANOUT_SHIFT;
	pnode = ubifs_get_pnode(c, nnode, iip);
	if (IS_ERR(pnode))
		return ERR_PTR(PTR_ERR(pnode));
	iip = (i & (UBIFS_LPT_FANOUT - 1));
	dbg_lp("LEB %d, free %d, dirty %d, flags %d", lnum,
	       pnode->lprops[iip].free, pnode->lprops[iip].dirty,
	       pnode->lprops[iip].flags);
	return &pnode->lprops[iip];
}

/**
 * dirty_cow_nnode - ensure a nnode is not being committed.
 * @c: UBIFS file-system description object
 * @nnode: nnode to check
 *
 * Returns dirtied nnode on success or negative error code on failure.
 */
static struct ubifs_nnode *dirty_cow_nnode(struct ubifs_info *c,
					   struct ubifs_nnode *nnode)
{
	struct ubifs_nnode *n;
	int i;

	if (!test_bit(COW_CNODE, &nnode->flags)) {
		/* nnode is not being committed */
		if (!test_and_set_bit(DIRTY_CNODE, &nnode->flags)) {
			c->dirty_nn_cnt += 1;
			ubifs_add_nnode_dirt(c, nnode);
		}
		return nnode;
	}

	/* nnode is being committed, so copy it */
	n = kmalloc(sizeof(struct ubifs_nnode), GFP_NOFS);
	if (unlikely(!n))
		return ERR_PTR(-ENOMEM);

	memcpy(n, nnode, sizeof(struct ubifs_nnode));
	n->cnext = NULL;
	__set_bit(DIRTY_CNODE, &n->flags);
	__clear_bit(COW_CNODE, &n->flags);

	/* The children now have new parent */
	for (i = 0; i < UBIFS_LPT_FANOUT; i++) {
		struct ubifs_nbranch *branch = &n->nbranch[i];

		if (branch->cnode)
			branch->cnode->parent = n;
	}

	ubifs_assert(!test_bit(OBSOLETE_CNODE, &nnode->flags));
	__set_bit(OBSOLETE_CNODE, &nnode->flags);

	c->dirty_nn_cnt += 1;
	ubifs_add_nnode_dirt(c, nnode);
	if (nnode->parent)
		nnode->parent->nbranch[n->iip].nnode = n;
	else
		c->nroot = n;
	return n;
}

/**
 * dirty_cow_pnode - ensure a pnode is not being committed.
 * @c: UBIFS file-system description object
 * @pnode: pnode to check
 *
 * Returns dirtied pnode on success or negative error code on failure.
 */
static struct ubifs_pnode *dirty_cow_pnode(struct ubifs_info *c,
					   struct ubifs_pnode *pnode)
{
	struct ubifs_pnode *p;

	if (!test_bit(COW_CNODE, &pnode->flags)) {
		/* pnode is not being committed */
		if (!test_and_set_bit(DIRTY_CNODE, &pnode->flags)) {
			c->dirty_pn_cnt += 1;
			add_pnode_dirt(c, pnode);
		}
		return pnode;
	}

	/* pnode is being committed, so copy it */
	p = kmalloc(sizeof(struct ubifs_pnode), GFP_NOFS);
	if (unlikely(!p))
		return ERR_PTR(-ENOMEM);

	memcpy(p, pnode, sizeof(struct ubifs_pnode));
	p->cnext = NULL;
	__set_bit(DIRTY_CNODE, &p->flags);
	__clear_bit(COW_CNODE, &p->flags);
	replace_cats(c, pnode, p);

	ubifs_assert(!test_bit(OBSOLETE_CNODE, &pnode->flags));
	__set_bit(OBSOLETE_CNODE, &pnode->flags);

	c->dirty_pn_cnt += 1;
	add_pnode_dirt(c, pnode);
	pnode->parent->nbranch[p->iip].pnode = p;
	return p;
}

/**
 * ubifs_lpt_lookup_dirty - lookup LEB properties in the LPT.
 * @c: UBIFS file-system description object
 * @lnum: LEB number to lookup
 *
 * This function returns a pointer to the LEB properties on success or a
 * negative error code on failure.
 */
struct ubifs_lprops *ubifs_lpt_lookup_dirty(struct ubifs_info *c, int lnum)
{
	int err, i, h, iip, shft;
	struct ubifs_nnode *nnode;
	struct ubifs_pnode *pnode;

	if (!c->nroot) {
		err = ubifs_read_nnode(c, NULL, 0);
		if (err)
			return ERR_PTR(err);
	}
	nnode = c->nroot;
	nnode = dirty_cow_nnode(c, nnode);
	if (IS_ERR(nnode))
		return ERR_PTR(PTR_ERR(nnode));
	i = lnum - c->main_first;
	shft = c->lpt_hght * UBIFS_LPT_FANOUT_SHIFT;
	for (h = 1; h < c->lpt_hght; h++) {
		iip = ((i >> shft) & (UBIFS_LPT_FANOUT - 1));
		shft -= UBIFS_LPT_FANOUT_SHIFT;
		nnode = ubifs_get_nnode(c, nnode, iip);
		if (IS_ERR(nnode))
			return ERR_PTR(PTR_ERR(nnode));
		nnode = dirty_cow_nnode(c, nnode);
		if (IS_ERR(nnode))
			return ERR_PTR(PTR_ERR(nnode));
	}
	iip = ((i >> shft) & (UBIFS_LPT_FANOUT - 1));
	shft -= UBIFS_LPT_FANOUT_SHIFT;
	pnode = ubifs_get_pnode(c, nnode, iip);
	if (IS_ERR(pnode))
		return ERR_PTR(PTR_ERR(pnode));
	pnode = dirty_cow_pnode(c, pnode);
	if (IS_ERR(pnode))
		return ERR_PTR(PTR_ERR(pnode));
	iip = (i & (UBIFS_LPT_FANOUT - 1));
	dbg_lp("LEB %d, free %d, dirty %d, flags %d", lnum,
	       pnode->lprops[iip].free, pnode->lprops[iip].dirty,
	       pnode->lprops[iip].flags);
	ubifs_assert(test_bit(DIRTY_CNODE, &pnode->flags));
	return &pnode->lprops[iip];
}

/**
 * lpt_init_rd - initialize the LPT for reading.
 * @c: UBIFS file-system description object
 *
 * This function returns %0 on success and a negative error code on failure.
 */
static int lpt_init_rd(struct ubifs_info *c)
{
	int err, i;

	c->ltab = vmalloc(sizeof(struct ubifs_lpt_lprops) * c->lpt_lebs);
	if (!c->ltab)
		return -ENOMEM;

	i = max_t(int, c->nnode_sz, c->pnode_sz);
	c->lpt_nod_buf = kmalloc(i, GFP_KERNEL);
	if (!c->lpt_nod_buf)
		return -ENOMEM;

	for (i = 0; i < LPROPS_HEAP_CNT; i++) {
		c->lpt_heap[i].arr = kmalloc(sizeof(void *) * LPT_HEAP_SZ,
					     GFP_KERNEL);
		if (!c->lpt_heap[i].arr)
			return -ENOMEM;
		c->lpt_heap[i].cnt = 0;
		c->lpt_heap[i].max_cnt = LPT_HEAP_SZ;
	}

	c->dirty_idx.arr = kmalloc(sizeof(void *) * LPT_HEAP_SZ, GFP_KERNEL);
	if (!c->dirty_idx.arr)
		return -ENOMEM;
	c->dirty_idx.cnt = 0;
	c->dirty_idx.max_cnt = LPT_HEAP_SZ;

	err = read_ltab(c);
	if (err)
		return err;

	dbg_lp("space_bits %d", c->space_bits);
	dbg_lp("lpt_lnum_bits %d", c->lpt_lnum_bits);
	dbg_lp("lpt_offs_bits %d", c->lpt_offs_bits);
	dbg_lp("lpt_spc_bits %d", c->lpt_spc_bits);
	dbg_lp("pcnt_bits %d", c->pcnt_bits);
	dbg_lp("lnum_bits %d", c->lnum_bits);
	dbg_lp("pnode_sz %d", c->pnode_sz);
	dbg_lp("nnode_sz %d", c->nnode_sz);
	dbg_lp("ltab_sz %d", c->ltab_sz);
	dbg_lp("lsave_sz %d", c->lsave_sz);
	dbg_lp("lsave_cnt %d", c->lsave_cnt);
	dbg_lp("lpt_hght %d", c->lpt_hght);
	dbg_lp("big_lpt %d", c->big_lpt);
	dbg_lp("LPT root is at %d:%d", c->lpt_lnum, c->lpt_offs);
	dbg_lp("LPT head is at %d:%d", c->nhead_lnum, c->nhead_offs);
	dbg_lp("LPT ltab is at %d:%d", c->ltab_lnum, c->ltab_offs);
	if (c->big_lpt)
		dbg_lp("LPT lsave is at %d:%d", c->lsave_lnum, c->lsave_offs);

	return 0;
}

/**
 * ubifs_lpt_init - initialize the LPT.
 * @c: UBIFS file-system description object
 * @rd: whether to initialize lpt for reading
 * @wr: whether to initialize lpt for writing
 *
 * For mounting 'rw', @rd and @wr are both true. For mounting 'ro', @rd is true
 * and @wr is false. For mounting from 'ro' to 'rw', @rd is false and @wr is
 * true.
 *
 * This function returns %0 on success and a negative error code on failure.
 */
int ubifs_lpt_init(struct ubifs_info *c, int rd, int wr)
{
	int err;

	if (rd) {
		err = lpt_init_rd(c);
		if (err)
			return err;
	}

	return 0;
}
