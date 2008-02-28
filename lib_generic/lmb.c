/*
 * Procedures for maintaining information about logical memory blocks.
 *
 * Peter Bergner, IBM Corp.	June 2001.
 * Copyright (C) 2001 Peter Bergner.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#include <common.h>
#include <lmb.h>

#define LMB_ALLOC_ANYWHERE	0

void lmb_dump_all(struct lmb *lmb)
{
#ifdef DEBUG
	unsigned long i;

	debug("lmb_dump_all:\n");
	debug("    memory.cnt		   = 0x%lx\n", lmb->memory.cnt);
	debug("    memory.size		   = 0x%08x\n", lmb->memory.size);
	for (i=0; i < lmb->memory.cnt ;i++) {
		debug("    memory.reg[0x%x].base   = 0x%08x\n", i,
			lmb->memory.region[i].base);
		debug("		   .size   = 0x%08x\n",
			lmb->memory.region[i].size);
	}

	debug("\n    reserved.cnt	   = 0x%lx\n", lmb->reserved.cnt);
	debug("    reserved.size	   = 0x%08x\n", lmb->reserved.size);
	for (i=0; i < lmb->reserved.cnt ;i++) {
		debug("    reserved.reg[0x%x].base = 0x%08x\n", i,
			lmb->reserved.region[i].base);
		debug("		     .size = 0x%08x\n",
			lmb->reserved.region[i].size);
	}
#endif /* DEBUG */
}

static unsigned long lmb_addrs_overlap(ulong base1,
		ulong size1, ulong base2, ulong size2)
{
	return ((base1 < (base2+size2)) && (base2 < (base1+size1)));
}

static long lmb_addrs_adjacent(ulong base1, ulong size1,
		ulong base2, ulong size2)
{
	if (base2 == base1 + size1)
		return 1;
	else if (base1 == base2 + size2)
		return -1;

	return 0;
}

static long lmb_regions_adjacent(struct lmb_region *rgn,
		unsigned long r1, unsigned long r2)
{
	ulong base1 = rgn->region[r1].base;
	ulong size1 = rgn->region[r1].size;
	ulong base2 = rgn->region[r2].base;
	ulong size2 = rgn->region[r2].size;

	return lmb_addrs_adjacent(base1, size1, base2, size2);
}

static void lmb_remove_region(struct lmb_region *rgn, unsigned long r)
{
	unsigned long i;

	for (i = r; i < rgn->cnt - 1; i++) {
		rgn->region[i].base = rgn->region[i + 1].base;
		rgn->region[i].size = rgn->region[i + 1].size;
	}
	rgn->cnt--;
}

/* Assumption: base addr of region 1 < base addr of region 2 */
static void lmb_coalesce_regions(struct lmb_region *rgn,
		unsigned long r1, unsigned long r2)
{
	rgn->region[r1].size += rgn->region[r2].size;
	lmb_remove_region(rgn, r2);
}

void lmb_init(struct lmb *lmb)
{
	/* Create a dummy zero size LMB which will get coalesced away later.
	 * This simplifies the lmb_add() code below...
	 */
	lmb->memory.region[0].base = 0;
	lmb->memory.region[0].size = 0;
	lmb->memory.cnt = 1;
	lmb->memory.size = 0;

	/* Ditto. */
	lmb->reserved.region[0].base = 0;
	lmb->reserved.region[0].size = 0;
	lmb->reserved.cnt = 1;
	lmb->reserved.size = 0;
}

/* This routine called with relocation disabled. */
static long lmb_add_region(struct lmb_region *rgn, ulong base, ulong size)
{
	unsigned long coalesced = 0;
	long adjacent, i;

	if ((rgn->cnt == 1) && (rgn->region[0].size == 0)) {
		rgn->region[0].base = base;
		rgn->region[0].size = size;
		return 0;
	}

	/* First try and coalesce this LMB with another. */
	for (i=0; i < rgn->cnt; i++) {
		ulong rgnbase = rgn->region[i].base;
		ulong rgnsize = rgn->region[i].size;

		if ((rgnbase == base) && (rgnsize == size))
			/* Already have this region, so we're done */
			return 0;

		adjacent = lmb_addrs_adjacent(base,size,rgnbase,rgnsize);
		if ( adjacent > 0 ) {
			rgn->region[i].base -= size;
			rgn->region[i].size += size;
			coalesced++;
			break;
		}
		else if ( adjacent < 0 ) {
			rgn->region[i].size += size;
			coalesced++;
			break;
		}
	}

	if ((i < rgn->cnt-1) && lmb_regions_adjacent(rgn, i, i+1) ) {
		lmb_coalesce_regions(rgn, i, i+1);
		coalesced++;
	}

	if (coalesced)
		return coalesced;
	if (rgn->cnt >= MAX_LMB_REGIONS)
		return -1;

	/* Couldn't coalesce the LMB, so add it to the sorted table. */
	for (i = rgn->cnt-1; i >= 0; i--) {
		if (base < rgn->region[i].base) {
			rgn->region[i+1].base = rgn->region[i].base;
			rgn->region[i+1].size = rgn->region[i].size;
		} else {
			rgn->region[i+1].base = base;
			rgn->region[i+1].size = size;
			break;
		}
	}

	if (base < rgn->region[0].base) {
		rgn->region[0].base = base;
		rgn->region[0].size = size;
	}

	rgn->cnt++;

	return 0;
}

/* This routine may be called with relocation disabled. */
long lmb_add(struct lmb *lmb, ulong base, ulong size)
{
	struct lmb_region *_rgn = &(lmb->memory);

	return lmb_add_region(_rgn, base, size);
}

long lmb_reserve(struct lmb *lmb, ulong base, ulong size)
{
	struct lmb_region *_rgn = &(lmb->reserved);

	return lmb_add_region(_rgn, base, size);
}

long lmb_overlaps_region(struct lmb_region *rgn, ulong base,
				ulong size)
{
	unsigned long i;

	for (i=0; i < rgn->cnt; i++) {
		ulong rgnbase = rgn->region[i].base;
		ulong rgnsize = rgn->region[i].size;
		if ( lmb_addrs_overlap(base,size,rgnbase,rgnsize) ) {
			break;
		}
	}

	return (i < rgn->cnt) ? i : -1;
}

ulong lmb_alloc(struct lmb *lmb, ulong size, ulong align)
{
	return lmb_alloc_base(lmb, size, align, LMB_ALLOC_ANYWHERE);
}

ulong lmb_alloc_base(struct lmb *lmb, ulong size, ulong align, ulong max_addr)
{
	ulong alloc;

	alloc = __lmb_alloc_base(lmb, size, align, max_addr);

	if (alloc == 0)
		printf("ERROR: Failed to allocate 0x%lx bytes below 0x%lx.\n",
		      size, max_addr);

	return alloc;
}

static ulong lmb_align_down(ulong addr, ulong size)
{
	return addr & ~(size - 1);
}

static ulong lmb_align_up(ulong addr, ulong size)
{
	return (addr + (size - 1)) & ~(size - 1);
}

ulong __lmb_alloc_base(struct lmb *lmb, ulong size, ulong align, ulong max_addr)
{
	long i, j;
	ulong base = 0;

	for (i = lmb->memory.cnt-1; i >= 0; i--) {
		ulong lmbbase = lmb->memory.region[i].base;
		ulong lmbsize = lmb->memory.region[i].size;

		if (max_addr == LMB_ALLOC_ANYWHERE)
			base = lmb_align_down(lmbbase + lmbsize - size, align);
		else if (lmbbase < max_addr) {
			base = min(lmbbase + lmbsize, max_addr);
			base = lmb_align_down(base - size, align);
		} else
			continue;

		while ((lmbbase <= base) &&
		       ((j = lmb_overlaps_region(&(lmb->reserved), base, size)) >= 0) )
			base = lmb_align_down(lmb->reserved.region[j].base - size,
					      align);

		if ((base != 0) && (lmbbase <= base))
			break;
	}

	if (i < 0)
		return 0;

	if (lmb_add_region(&(lmb->reserved), base, lmb_align_up(size, align)) < 0)
		return 0;

	return base;
}

int lmb_is_reserved(struct lmb *lmb, ulong addr)
{
	int i;

	for (i = 0; i < lmb->reserved.cnt; i++) {
		ulong upper = lmb->reserved.region[i].base +
			lmb->reserved.region[i].size - 1;
		if ((addr >= lmb->reserved.region[i].base) && (addr <= upper))
			return 1;
	}
	return 0;
}
