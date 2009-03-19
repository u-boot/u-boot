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
 * This file is a part of UBIFS journal implementation and contains various
 * functions which manipulate the log. The log is a fixed area on the flash
 * which does not contain any data but refers to buds. The log is a part of the
 * journal.
 */

#include "ubifs.h"

/**
 * ubifs_search_bud - search bud LEB.
 * @c: UBIFS file-system description object
 * @lnum: logical eraseblock number to search
 *
 * This function searches bud LEB @lnum. Returns bud description object in case
 * of success and %NULL if there is no bud with this LEB number.
 */
struct ubifs_bud *ubifs_search_bud(struct ubifs_info *c, int lnum)
{
	struct rb_node *p;
	struct ubifs_bud *bud;

	spin_lock(&c->buds_lock);
	p = c->buds.rb_node;
	while (p) {
		bud = rb_entry(p, struct ubifs_bud, rb);
		if (lnum < bud->lnum)
			p = p->rb_left;
		else if (lnum > bud->lnum)
			p = p->rb_right;
		else {
			spin_unlock(&c->buds_lock);
			return bud;
		}
	}
	spin_unlock(&c->buds_lock);
	return NULL;
}

/**
 * ubifs_add_bud - add bud LEB to the tree of buds and its journal head list.
 * @c: UBIFS file-system description object
 * @bud: the bud to add
 */
void ubifs_add_bud(struct ubifs_info *c, struct ubifs_bud *bud)
{
	struct rb_node **p, *parent = NULL;
	struct ubifs_bud *b;
	struct ubifs_jhead *jhead;

	spin_lock(&c->buds_lock);
	p = &c->buds.rb_node;
	while (*p) {
		parent = *p;
		b = rb_entry(parent, struct ubifs_bud, rb);
		ubifs_assert(bud->lnum != b->lnum);
		if (bud->lnum < b->lnum)
			p = &(*p)->rb_left;
		else
			p = &(*p)->rb_right;
	}

	rb_link_node(&bud->rb, parent, p);
	rb_insert_color(&bud->rb, &c->buds);
	if (c->jheads) {
		jhead = &c->jheads[bud->jhead];
		list_add_tail(&bud->list, &jhead->buds_list);
	} else
		ubifs_assert(c->replaying && (c->vfs_sb->s_flags & MS_RDONLY));

	/*
	 * Note, although this is a new bud, we anyway account this space now,
	 * before any data has been written to it, because this is about to
	 * guarantee fixed mount time, and this bud will anyway be read and
	 * scanned.
	 */
	c->bud_bytes += c->leb_size - bud->start;

	dbg_log("LEB %d:%d, jhead %d, bud_bytes %lld", bud->lnum,
		bud->start, bud->jhead, c->bud_bytes);
	spin_unlock(&c->buds_lock);
}
