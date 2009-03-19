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
 * This file implements the budgeting sub-system which is responsible for UBIFS
 * space management.
 *
 * Factors such as compression, wasted space at the ends of LEBs, space in other
 * journal heads, the effect of updates on the index, and so on, make it
 * impossible to accurately predict the amount of space needed. Consequently
 * approximations are used.
 */

#include "ubifs.h"
#include <linux/math64.h>

/**
 * ubifs_calc_min_idx_lebs - calculate amount of eraseblocks for the index.
 * @c: UBIFS file-system description object
 *
 * This function calculates and returns the number of eraseblocks which should
 * be kept for index usage.
 */
int ubifs_calc_min_idx_lebs(struct ubifs_info *c)
{
	int idx_lebs, eff_leb_size = c->leb_size - c->max_idx_node_sz;
	long long idx_size;

	idx_size = c->old_idx_sz + c->budg_idx_growth + c->budg_uncommitted_idx;

	/* And make sure we have thrice the index size of space reserved */
	idx_size = idx_size + (idx_size << 1);

	/*
	 * We do not maintain 'old_idx_size' as 'old_idx_lebs'/'old_idx_bytes'
	 * pair, nor similarly the two variables for the new index size, so we
	 * have to do this costly 64-bit division on fast-path.
	 */
	idx_size += eff_leb_size - 1;
	idx_lebs = div_u64(idx_size, eff_leb_size);
	/*
	 * The index head is not available for the in-the-gaps method, so add an
	 * extra LEB to compensate.
	 */
	idx_lebs += 1;
	if (idx_lebs < MIN_INDEX_LEBS)
		idx_lebs = MIN_INDEX_LEBS;
	return idx_lebs;
}

/**
 * ubifs_reported_space - calculate reported free space.
 * @c: the UBIFS file-system description object
 * @free: amount of free space
 *
 * This function calculates amount of free space which will be reported to
 * user-space. User-space application tend to expect that if the file-system
 * (e.g., via the 'statfs()' call) reports that it has N bytes available, they
 * are able to write a file of size N. UBIFS attaches node headers to each data
 * node and it has to write indexing nodes as well. This introduces additional
 * overhead, and UBIFS has to report slightly less free space to meet the above
 * expectations.
 *
 * This function assumes free space is made up of uncompressed data nodes and
 * full index nodes (one per data node, tripled because we always allow enough
 * space to write the index thrice).
 *
 * Note, the calculation is pessimistic, which means that most of the time
 * UBIFS reports less space than it actually has.
 */
long long ubifs_reported_space(const struct ubifs_info *c, long long free)
{
	int divisor, factor, f;

	/*
	 * Reported space size is @free * X, where X is UBIFS block size
	 * divided by UBIFS block size + all overhead one data block
	 * introduces. The overhead is the node header + indexing overhead.
	 *
	 * Indexing overhead calculations are based on the following formula:
	 * I = N/(f - 1) + 1, where I - number of indexing nodes, N - number
	 * of data nodes, f - fanout. Because effective UBIFS fanout is twice
	 * as less than maximum fanout, we assume that each data node
	 * introduces 3 * @c->max_idx_node_sz / (@c->fanout/2 - 1) bytes.
	 * Note, the multiplier 3 is because UBIFS reserves thrice as more space
	 * for the index.
	 */
	f = c->fanout > 3 ? c->fanout >> 1 : 2;
	factor = UBIFS_BLOCK_SIZE;
	divisor = UBIFS_MAX_DATA_NODE_SZ;
	divisor += (c->max_idx_node_sz * 3) / (f - 1);
	free *= factor;
	return div_u64(free, divisor);
}
