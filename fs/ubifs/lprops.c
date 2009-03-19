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
 * This file implements the functions that access LEB properties and their
 * categories. LEBs are categorized based on the needs of UBIFS, and the
 * categories are stored as either heaps or lists to provide a fast way of
 * finding a LEB in a particular category. For example, UBIFS may need to find
 * an empty LEB for the journal, or a very dirty LEB for garbage collection.
 */

#include "ubifs.h"

/**
 * get_heap_comp_val - get the LEB properties value for heap comparisons.
 * @lprops: LEB properties
 * @cat: LEB category
 */
static int get_heap_comp_val(struct ubifs_lprops *lprops, int cat)
{
	switch (cat) {
	case LPROPS_FREE:
		return lprops->free;
	case LPROPS_DIRTY_IDX:
		return lprops->free + lprops->dirty;
	default:
		return lprops->dirty;
	}
}

/**
 * move_up_lpt_heap - move a new heap entry up as far as possible.
 * @c: UBIFS file-system description object
 * @heap: LEB category heap
 * @lprops: LEB properties to move
 * @cat: LEB category
 *
 * New entries to a heap are added at the bottom and then moved up until the
 * parent's value is greater.  In the case of LPT's category heaps, the value
 * is either the amount of free space or the amount of dirty space, depending
 * on the category.
 */
static void move_up_lpt_heap(struct ubifs_info *c, struct ubifs_lpt_heap *heap,
			     struct ubifs_lprops *lprops, int cat)
{
	int val1, val2, hpos;

	hpos = lprops->hpos;
	if (!hpos)
		return; /* Already top of the heap */
	val1 = get_heap_comp_val(lprops, cat);
	/* Compare to parent and, if greater, move up the heap */
	do {
		int ppos = (hpos - 1) / 2;

		val2 = get_heap_comp_val(heap->arr[ppos], cat);
		if (val2 >= val1)
			return;
		/* Greater than parent so move up */
		heap->arr[ppos]->hpos = hpos;
		heap->arr[hpos] = heap->arr[ppos];
		heap->arr[ppos] = lprops;
		lprops->hpos = ppos;
		hpos = ppos;
	} while (hpos);
}

/**
 * adjust_lpt_heap - move a changed heap entry up or down the heap.
 * @c: UBIFS file-system description object
 * @heap: LEB category heap
 * @lprops: LEB properties to move
 * @hpos: heap position of @lprops
 * @cat: LEB category
 *
 * Changed entries in a heap are moved up or down until the parent's value is
 * greater.  In the case of LPT's category heaps, the value is either the amount
 * of free space or the amount of dirty space, depending on the category.
 */
static void adjust_lpt_heap(struct ubifs_info *c, struct ubifs_lpt_heap *heap,
			    struct ubifs_lprops *lprops, int hpos, int cat)
{
	int val1, val2, val3, cpos;

	val1 = get_heap_comp_val(lprops, cat);
	/* Compare to parent and, if greater than parent, move up the heap */
	if (hpos) {
		int ppos = (hpos - 1) / 2;

		val2 = get_heap_comp_val(heap->arr[ppos], cat);
		if (val1 > val2) {
			/* Greater than parent so move up */
			while (1) {
				heap->arr[ppos]->hpos = hpos;
				heap->arr[hpos] = heap->arr[ppos];
				heap->arr[ppos] = lprops;
				lprops->hpos = ppos;
				hpos = ppos;
				if (!hpos)
					return;
				ppos = (hpos - 1) / 2;
				val2 = get_heap_comp_val(heap->arr[ppos], cat);
				if (val1 <= val2)
					return;
				/* Still greater than parent so keep going */
			}
		}
	}

	/* Not greater than parent, so compare to children */
	while (1) {
		/* Compare to left child */
		cpos = hpos * 2 + 1;
		if (cpos >= heap->cnt)
			return;
		val2 = get_heap_comp_val(heap->arr[cpos], cat);
		if (val1 < val2) {
			/* Less than left child, so promote biggest child */
			if (cpos + 1 < heap->cnt) {
				val3 = get_heap_comp_val(heap->arr[cpos + 1],
							 cat);
				if (val3 > val2)
					cpos += 1; /* Right child is bigger */
			}
			heap->arr[cpos]->hpos = hpos;
			heap->arr[hpos] = heap->arr[cpos];
			heap->arr[cpos] = lprops;
			lprops->hpos = cpos;
			hpos = cpos;
			continue;
		}
		/* Compare to right child */
		cpos += 1;
		if (cpos >= heap->cnt)
			return;
		val3 = get_heap_comp_val(heap->arr[cpos], cat);
		if (val1 < val3) {
			/* Less than right child, so promote right child */
			heap->arr[cpos]->hpos = hpos;
			heap->arr[hpos] = heap->arr[cpos];
			heap->arr[cpos] = lprops;
			lprops->hpos = cpos;
			hpos = cpos;
			continue;
		}
		return;
	}
}

/**
 * add_to_lpt_heap - add LEB properties to a LEB category heap.
 * @c: UBIFS file-system description object
 * @lprops: LEB properties to add
 * @cat: LEB category
 *
 * This function returns %1 if @lprops is added to the heap for LEB category
 * @cat, otherwise %0 is returned because the heap is full.
 */
static int add_to_lpt_heap(struct ubifs_info *c, struct ubifs_lprops *lprops,
			   int cat)
{
	struct ubifs_lpt_heap *heap = &c->lpt_heap[cat - 1];

	if (heap->cnt >= heap->max_cnt) {
		const int b = LPT_HEAP_SZ / 2 - 1;
		int cpos, val1, val2;

		/* Compare to some other LEB on the bottom of heap */
		/* Pick a position kind of randomly */
		cpos = (((size_t)lprops >> 4) & b) + b;
		ubifs_assert(cpos >= b);
		ubifs_assert(cpos < LPT_HEAP_SZ);
		ubifs_assert(cpos < heap->cnt);

		val1 = get_heap_comp_val(lprops, cat);
		val2 = get_heap_comp_val(heap->arr[cpos], cat);
		if (val1 > val2) {
			struct ubifs_lprops *lp;

			lp = heap->arr[cpos];
			lp->flags &= ~LPROPS_CAT_MASK;
			lp->flags |= LPROPS_UNCAT;
			list_add(&lp->list, &c->uncat_list);
			lprops->hpos = cpos;
			heap->arr[cpos] = lprops;
			move_up_lpt_heap(c, heap, lprops, cat);
			dbg_check_heap(c, heap, cat, lprops->hpos);
			return 1; /* Added to heap */
		}
		dbg_check_heap(c, heap, cat, -1);
		return 0; /* Not added to heap */
	} else {
		lprops->hpos = heap->cnt++;
		heap->arr[lprops->hpos] = lprops;
		move_up_lpt_heap(c, heap, lprops, cat);
		dbg_check_heap(c, heap, cat, lprops->hpos);
		return 1; /* Added to heap */
	}
}

/**
 * remove_from_lpt_heap - remove LEB properties from a LEB category heap.
 * @c: UBIFS file-system description object
 * @lprops: LEB properties to remove
 * @cat: LEB category
 */
static void remove_from_lpt_heap(struct ubifs_info *c,
				 struct ubifs_lprops *lprops, int cat)
{
	struct ubifs_lpt_heap *heap;
	int hpos = lprops->hpos;

	heap = &c->lpt_heap[cat - 1];
	ubifs_assert(hpos >= 0 && hpos < heap->cnt);
	ubifs_assert(heap->arr[hpos] == lprops);
	heap->cnt -= 1;
	if (hpos < heap->cnt) {
		heap->arr[hpos] = heap->arr[heap->cnt];
		heap->arr[hpos]->hpos = hpos;
		adjust_lpt_heap(c, heap, heap->arr[hpos], hpos, cat);
	}
	dbg_check_heap(c, heap, cat, -1);
}

/**
 * lpt_heap_replace - replace lprops in a category heap.
 * @c: UBIFS file-system description object
 * @old_lprops: LEB properties to replace
 * @new_lprops: LEB properties with which to replace
 * @cat: LEB category
 *
 * During commit it is sometimes necessary to copy a pnode (see dirty_cow_pnode)
 * and the lprops that the pnode contains.  When that happens, references in
 * the category heaps to those lprops must be updated to point to the new
 * lprops.  This function does that.
 */
static void lpt_heap_replace(struct ubifs_info *c,
			     struct ubifs_lprops *old_lprops,
			     struct ubifs_lprops *new_lprops, int cat)
{
	struct ubifs_lpt_heap *heap;
	int hpos = new_lprops->hpos;

	heap = &c->lpt_heap[cat - 1];
	heap->arr[hpos] = new_lprops;
}

/**
 * ubifs_add_to_cat - add LEB properties to a category list or heap.
 * @c: UBIFS file-system description object
 * @lprops: LEB properties to add
 * @cat: LEB category to which to add
 *
 * LEB properties are categorized to enable fast find operations.
 */
void ubifs_add_to_cat(struct ubifs_info *c, struct ubifs_lprops *lprops,
		      int cat)
{
	switch (cat) {
	case LPROPS_DIRTY:
	case LPROPS_DIRTY_IDX:
	case LPROPS_FREE:
		if (add_to_lpt_heap(c, lprops, cat))
			break;
		/* No more room on heap so make it uncategorized */
		cat = LPROPS_UNCAT;
		/* Fall through */
	case LPROPS_UNCAT:
		list_add(&lprops->list, &c->uncat_list);
		break;
	case LPROPS_EMPTY:
		list_add(&lprops->list, &c->empty_list);
		break;
	case LPROPS_FREEABLE:
		list_add(&lprops->list, &c->freeable_list);
		c->freeable_cnt += 1;
		break;
	case LPROPS_FRDI_IDX:
		list_add(&lprops->list, &c->frdi_idx_list);
		break;
	default:
		ubifs_assert(0);
	}
	lprops->flags &= ~LPROPS_CAT_MASK;
	lprops->flags |= cat;
}

/**
 * ubifs_remove_from_cat - remove LEB properties from a category list or heap.
 * @c: UBIFS file-system description object
 * @lprops: LEB properties to remove
 * @cat: LEB category from which to remove
 *
 * LEB properties are categorized to enable fast find operations.
 */
static void ubifs_remove_from_cat(struct ubifs_info *c,
				  struct ubifs_lprops *lprops, int cat)
{
	switch (cat) {
	case LPROPS_DIRTY:
	case LPROPS_DIRTY_IDX:
	case LPROPS_FREE:
		remove_from_lpt_heap(c, lprops, cat);
		break;
	case LPROPS_FREEABLE:
		c->freeable_cnt -= 1;
		ubifs_assert(c->freeable_cnt >= 0);
		/* Fall through */
	case LPROPS_UNCAT:
	case LPROPS_EMPTY:
	case LPROPS_FRDI_IDX:
		ubifs_assert(!list_empty(&lprops->list));
		list_del(&lprops->list);
		break;
	default:
		ubifs_assert(0);
	}
}

/**
 * ubifs_replace_cat - replace lprops in a category list or heap.
 * @c: UBIFS file-system description object
 * @old_lprops: LEB properties to replace
 * @new_lprops: LEB properties with which to replace
 *
 * During commit it is sometimes necessary to copy a pnode (see dirty_cow_pnode)
 * and the lprops that the pnode contains. When that happens, references in
 * category lists and heaps must be replaced. This function does that.
 */
void ubifs_replace_cat(struct ubifs_info *c, struct ubifs_lprops *old_lprops,
		       struct ubifs_lprops *new_lprops)
{
	int cat;

	cat = new_lprops->flags & LPROPS_CAT_MASK;
	switch (cat) {
	case LPROPS_DIRTY:
	case LPROPS_DIRTY_IDX:
	case LPROPS_FREE:
		lpt_heap_replace(c, old_lprops, new_lprops, cat);
		break;
	case LPROPS_UNCAT:
	case LPROPS_EMPTY:
	case LPROPS_FREEABLE:
	case LPROPS_FRDI_IDX:
		list_replace(&old_lprops->list, &new_lprops->list);
		break;
	default:
		ubifs_assert(0);
	}
}

/**
 * ubifs_ensure_cat - ensure LEB properties are categorized.
 * @c: UBIFS file-system description object
 * @lprops: LEB properties
 *
 * A LEB may have fallen off of the bottom of a heap, and ended up as
 * uncategorized even though it has enough space for us now. If that is the case
 * this function will put the LEB back onto a heap.
 */
void ubifs_ensure_cat(struct ubifs_info *c, struct ubifs_lprops *lprops)
{
	int cat = lprops->flags & LPROPS_CAT_MASK;

	if (cat != LPROPS_UNCAT)
		return;
	cat = ubifs_categorize_lprops(c, lprops);
	if (cat == LPROPS_UNCAT)
		return;
	ubifs_remove_from_cat(c, lprops, LPROPS_UNCAT);
	ubifs_add_to_cat(c, lprops, cat);
}

/**
 * ubifs_categorize_lprops - categorize LEB properties.
 * @c: UBIFS file-system description object
 * @lprops: LEB properties to categorize
 *
 * LEB properties are categorized to enable fast find operations. This function
 * returns the LEB category to which the LEB properties belong. Note however
 * that if the LEB category is stored as a heap and the heap is full, the
 * LEB properties may have their category changed to %LPROPS_UNCAT.
 */
int ubifs_categorize_lprops(const struct ubifs_info *c,
			    const struct ubifs_lprops *lprops)
{
	if (lprops->flags & LPROPS_TAKEN)
		return LPROPS_UNCAT;

	if (lprops->free == c->leb_size) {
		ubifs_assert(!(lprops->flags & LPROPS_INDEX));
		return LPROPS_EMPTY;
	}

	if (lprops->free + lprops->dirty == c->leb_size) {
		if (lprops->flags & LPROPS_INDEX)
			return LPROPS_FRDI_IDX;
		else
			return LPROPS_FREEABLE;
	}

	if (lprops->flags & LPROPS_INDEX) {
		if (lprops->dirty + lprops->free >= c->min_idx_node_sz)
			return LPROPS_DIRTY_IDX;
	} else {
		if (lprops->dirty >= c->dead_wm &&
		    lprops->dirty > lprops->free)
			return LPROPS_DIRTY;
		if (lprops->free > 0)
			return LPROPS_FREE;
	}

	return LPROPS_UNCAT;
}

/**
 * change_category - change LEB properties category.
 * @c: UBIFS file-system description object
 * @lprops: LEB properties to recategorize
 *
 * LEB properties are categorized to enable fast find operations. When the LEB
 * properties change they must be recategorized.
 */
static void change_category(struct ubifs_info *c, struct ubifs_lprops *lprops)
{
	int old_cat = lprops->flags & LPROPS_CAT_MASK;
	int new_cat = ubifs_categorize_lprops(c, lprops);

	if (old_cat == new_cat) {
		struct ubifs_lpt_heap *heap = &c->lpt_heap[new_cat - 1];

		/* lprops on a heap now must be moved up or down */
		if (new_cat < 1 || new_cat > LPROPS_HEAP_CNT)
			return; /* Not on a heap */
		heap = &c->lpt_heap[new_cat - 1];
		adjust_lpt_heap(c, heap, lprops, lprops->hpos, new_cat);
	} else {
		ubifs_remove_from_cat(c, lprops, old_cat);
		ubifs_add_to_cat(c, lprops, new_cat);
	}
}

/**
 * calc_dark - calculate LEB dark space size.
 * @c: the UBIFS file-system description object
 * @spc: amount of free and dirty space in the LEB
 *
 * This function calculates amount of dark space in an LEB which has @spc bytes
 * of free and dirty space. Returns the calculations result.
 *
 * Dark space is the space which is not always usable - it depends on which
 * nodes are written in which order. E.g., if an LEB has only 512 free bytes,
 * it is dark space, because it cannot fit a large data node. So UBIFS cannot
 * count on this LEB and treat these 512 bytes as usable because it is not true
 * if, for example, only big chunks of uncompressible data will be written to
 * the FS.
 */
static int calc_dark(struct ubifs_info *c, int spc)
{
	ubifs_assert(!(spc & 7));

	if (spc < c->dark_wm)
		return spc;

	/*
	 * If we have slightly more space then the dark space watermark, we can
	 * anyway safely assume it we'll be able to write a node of the
	 * smallest size there.
	 */
	if (spc - c->dark_wm < MIN_WRITE_SZ)
		return spc - MIN_WRITE_SZ;

	return c->dark_wm;
}

/**
 * is_lprops_dirty - determine if LEB properties are dirty.
 * @c: the UBIFS file-system description object
 * @lprops: LEB properties to test
 */
static int is_lprops_dirty(struct ubifs_info *c, struct ubifs_lprops *lprops)
{
	struct ubifs_pnode *pnode;
	int pos;

	pos = (lprops->lnum - c->main_first) & (UBIFS_LPT_FANOUT - 1);
	pnode = (struct ubifs_pnode *)container_of(lprops - pos,
						   struct ubifs_pnode,
						   lprops[0]);
	return !test_bit(COW_ZNODE, &pnode->flags) &&
	       test_bit(DIRTY_CNODE, &pnode->flags);
}

/**
 * ubifs_change_lp - change LEB properties.
 * @c: the UBIFS file-system description object
 * @lp: LEB properties to change
 * @free: new free space amount
 * @dirty: new dirty space amount
 * @flags: new flags
 * @idx_gc_cnt: change to the count of idx_gc list
 *
 * This function changes LEB properties (@free, @dirty or @flag). However, the
 * property which has the %LPROPS_NC value is not changed. Returns a pointer to
 * the updated LEB properties on success and a negative error code on failure.
 *
 * Note, the LEB properties may have had to be copied (due to COW) and
 * consequently the pointer returned may not be the same as the pointer
 * passed.
 */
const struct ubifs_lprops *ubifs_change_lp(struct ubifs_info *c,
					   const struct ubifs_lprops *lp,
					   int free, int dirty, int flags,
					   int idx_gc_cnt)
{
	/*
	 * This is the only function that is allowed to change lprops, so we
	 * discard the const qualifier.
	 */
	struct ubifs_lprops *lprops = (struct ubifs_lprops *)lp;

	dbg_lp("LEB %d, free %d, dirty %d, flags %d",
	       lprops->lnum, free, dirty, flags);

	ubifs_assert(mutex_is_locked(&c->lp_mutex));
	ubifs_assert(c->lst.empty_lebs >= 0 &&
		     c->lst.empty_lebs <= c->main_lebs);
	ubifs_assert(c->freeable_cnt >= 0);
	ubifs_assert(c->freeable_cnt <= c->main_lebs);
	ubifs_assert(c->lst.taken_empty_lebs >= 0);
	ubifs_assert(c->lst.taken_empty_lebs <= c->lst.empty_lebs);
	ubifs_assert(!(c->lst.total_free & 7) && !(c->lst.total_dirty & 7));
	ubifs_assert(!(c->lst.total_dead & 7) && !(c->lst.total_dark & 7));
	ubifs_assert(!(c->lst.total_used & 7));
	ubifs_assert(free == LPROPS_NC || free >= 0);
	ubifs_assert(dirty == LPROPS_NC || dirty >= 0);

	if (!is_lprops_dirty(c, lprops)) {
		lprops = ubifs_lpt_lookup_dirty(c, lprops->lnum);
		if (IS_ERR(lprops))
			return lprops;
	} else
		ubifs_assert(lprops == ubifs_lpt_lookup_dirty(c, lprops->lnum));

	ubifs_assert(!(lprops->free & 7) && !(lprops->dirty & 7));

	spin_lock(&c->space_lock);
	if ((lprops->flags & LPROPS_TAKEN) && lprops->free == c->leb_size)
		c->lst.taken_empty_lebs -= 1;

	if (!(lprops->flags & LPROPS_INDEX)) {
		int old_spc;

		old_spc = lprops->free + lprops->dirty;
		if (old_spc < c->dead_wm)
			c->lst.total_dead -= old_spc;
		else
			c->lst.total_dark -= calc_dark(c, old_spc);

		c->lst.total_used -= c->leb_size - old_spc;
	}

	if (free != LPROPS_NC) {
		free = ALIGN(free, 8);
		c->lst.total_free += free - lprops->free;

		/* Increase or decrease empty LEBs counter if needed */
		if (free == c->leb_size) {
			if (lprops->free != c->leb_size)
				c->lst.empty_lebs += 1;
		} else if (lprops->free == c->leb_size)
			c->lst.empty_lebs -= 1;
		lprops->free = free;
	}

	if (dirty != LPROPS_NC) {
		dirty = ALIGN(dirty, 8);
		c->lst.total_dirty += dirty - lprops->dirty;
		lprops->dirty = dirty;
	}

	if (flags != LPROPS_NC) {
		/* Take care about indexing LEBs counter if needed */
		if ((lprops->flags & LPROPS_INDEX)) {
			if (!(flags & LPROPS_INDEX))
				c->lst.idx_lebs -= 1;
		} else if (flags & LPROPS_INDEX)
			c->lst.idx_lebs += 1;
		lprops->flags = flags;
	}

	if (!(lprops->flags & LPROPS_INDEX)) {
		int new_spc;

		new_spc = lprops->free + lprops->dirty;
		if (new_spc < c->dead_wm)
			c->lst.total_dead += new_spc;
		else
			c->lst.total_dark += calc_dark(c, new_spc);

		c->lst.total_used += c->leb_size - new_spc;
	}

	if ((lprops->flags & LPROPS_TAKEN) && lprops->free == c->leb_size)
		c->lst.taken_empty_lebs += 1;

	change_category(c, lprops);
	c->idx_gc_cnt += idx_gc_cnt;
	spin_unlock(&c->space_lock);
	return lprops;
}

/**
 * ubifs_get_lp_stats - get lprops statistics.
 * @c: UBIFS file-system description object
 * @st: return statistics
 */
void ubifs_get_lp_stats(struct ubifs_info *c, struct ubifs_lp_stats *lst)
{
	spin_lock(&c->space_lock);
	memcpy(lst, &c->lst, sizeof(struct ubifs_lp_stats));
	spin_unlock(&c->space_lock);
}

/**
 * ubifs_change_one_lp - change LEB properties.
 * @c: the UBIFS file-system description object
 * @lnum: LEB to change properties for
 * @free: amount of free space
 * @dirty: amount of dirty space
 * @flags_set: flags to set
 * @flags_clean: flags to clean
 * @idx_gc_cnt: change to the count of idx_gc list
 *
 * This function changes properties of LEB @lnum. It is a helper wrapper over
 * 'ubifs_change_lp()' which hides lprops get/release. The arguments are the
 * same as in case of 'ubifs_change_lp()'. Returns zero in case of success and
 * a negative error code in case of failure.
 */
int ubifs_change_one_lp(struct ubifs_info *c, int lnum, int free, int dirty,
			int flags_set, int flags_clean, int idx_gc_cnt)
{
	int err = 0, flags;
	const struct ubifs_lprops *lp;

	ubifs_get_lprops(c);

	lp = ubifs_lpt_lookup_dirty(c, lnum);
	if (IS_ERR(lp)) {
		err = PTR_ERR(lp);
		goto out;
	}

	flags = (lp->flags | flags_set) & ~flags_clean;
	lp = ubifs_change_lp(c, lp, free, dirty, flags, idx_gc_cnt);
	if (IS_ERR(lp))
		err = PTR_ERR(lp);

out:
	ubifs_release_lprops(c);
	return err;
}

/**
 * ubifs_update_one_lp - update LEB properties.
 * @c: the UBIFS file-system description object
 * @lnum: LEB to change properties for
 * @free: amount of free space
 * @dirty: amount of dirty space to add
 * @flags_set: flags to set
 * @flags_clean: flags to clean
 *
 * This function is the same as 'ubifs_change_one_lp()' but @dirty is added to
 * current dirty space, not substitutes it.
 */
int ubifs_update_one_lp(struct ubifs_info *c, int lnum, int free, int dirty,
			int flags_set, int flags_clean)
{
	int err = 0, flags;
	const struct ubifs_lprops *lp;

	ubifs_get_lprops(c);

	lp = ubifs_lpt_lookup_dirty(c, lnum);
	if (IS_ERR(lp)) {
		err = PTR_ERR(lp);
		goto out;
	}

	flags = (lp->flags | flags_set) & ~flags_clean;
	lp = ubifs_change_lp(c, lp, free, lp->dirty + dirty, flags, 0);
	if (IS_ERR(lp))
		err = PTR_ERR(lp);

out:
	ubifs_release_lprops(c);
	return err;
}

/**
 * ubifs_read_one_lp - read LEB properties.
 * @c: the UBIFS file-system description object
 * @lnum: LEB to read properties for
 * @lp: where to store read properties
 *
 * This helper function reads properties of a LEB @lnum and stores them in @lp.
 * Returns zero in case of success and a negative error code in case of
 * failure.
 */
int ubifs_read_one_lp(struct ubifs_info *c, int lnum, struct ubifs_lprops *lp)
{
	int err = 0;
	const struct ubifs_lprops *lpp;

	ubifs_get_lprops(c);

	lpp = ubifs_lpt_lookup(c, lnum);
	if (IS_ERR(lpp)) {
		err = PTR_ERR(lpp);
		goto out;
	}

	memcpy(lp, lpp, sizeof(struct ubifs_lprops));

out:
	ubifs_release_lprops(c);
	return err;
}

/**
 * ubifs_fast_find_free - try to find a LEB with free space quickly.
 * @c: the UBIFS file-system description object
 *
 * This function returns LEB properties for a LEB with free space or %NULL if
 * the function is unable to find a LEB quickly.
 */
const struct ubifs_lprops *ubifs_fast_find_free(struct ubifs_info *c)
{
	struct ubifs_lprops *lprops;
	struct ubifs_lpt_heap *heap;

	ubifs_assert(mutex_is_locked(&c->lp_mutex));

	heap = &c->lpt_heap[LPROPS_FREE - 1];
	if (heap->cnt == 0)
		return NULL;

	lprops = heap->arr[0];
	ubifs_assert(!(lprops->flags & LPROPS_TAKEN));
	ubifs_assert(!(lprops->flags & LPROPS_INDEX));
	return lprops;
}

/**
 * ubifs_fast_find_empty - try to find an empty LEB quickly.
 * @c: the UBIFS file-system description object
 *
 * This function returns LEB properties for an empty LEB or %NULL if the
 * function is unable to find an empty LEB quickly.
 */
const struct ubifs_lprops *ubifs_fast_find_empty(struct ubifs_info *c)
{
	struct ubifs_lprops *lprops;

	ubifs_assert(mutex_is_locked(&c->lp_mutex));

	if (list_empty(&c->empty_list))
		return NULL;

	lprops = list_entry(c->empty_list.next, struct ubifs_lprops, list);
	ubifs_assert(!(lprops->flags & LPROPS_TAKEN));
	ubifs_assert(!(lprops->flags & LPROPS_INDEX));
	ubifs_assert(lprops->free == c->leb_size);
	return lprops;
}

/**
 * ubifs_fast_find_freeable - try to find a freeable LEB quickly.
 * @c: the UBIFS file-system description object
 *
 * This function returns LEB properties for a freeable LEB or %NULL if the
 * function is unable to find a freeable LEB quickly.
 */
const struct ubifs_lprops *ubifs_fast_find_freeable(struct ubifs_info *c)
{
	struct ubifs_lprops *lprops;

	ubifs_assert(mutex_is_locked(&c->lp_mutex));

	if (list_empty(&c->freeable_list))
		return NULL;

	lprops = list_entry(c->freeable_list.next, struct ubifs_lprops, list);
	ubifs_assert(!(lprops->flags & LPROPS_TAKEN));
	ubifs_assert(!(lprops->flags & LPROPS_INDEX));
	ubifs_assert(lprops->free + lprops->dirty == c->leb_size);
	ubifs_assert(c->freeable_cnt > 0);
	return lprops;
}

/**
 * ubifs_fast_find_frdi_idx - try to find a freeable index LEB quickly.
 * @c: the UBIFS file-system description object
 *
 * This function returns LEB properties for a freeable index LEB or %NULL if the
 * function is unable to find a freeable index LEB quickly.
 */
const struct ubifs_lprops *ubifs_fast_find_frdi_idx(struct ubifs_info *c)
{
	struct ubifs_lprops *lprops;

	ubifs_assert(mutex_is_locked(&c->lp_mutex));

	if (list_empty(&c->frdi_idx_list))
		return NULL;

	lprops = list_entry(c->frdi_idx_list.next, struct ubifs_lprops, list);
	ubifs_assert(!(lprops->flags & LPROPS_TAKEN));
	ubifs_assert((lprops->flags & LPROPS_INDEX));
	ubifs_assert(lprops->free + lprops->dirty == c->leb_size);
	return lprops;
}
