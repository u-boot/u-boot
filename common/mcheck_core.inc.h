/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Free Software Foundation, Inc.
 * Written by Eugene Uriev, based on glibc 2.0 prototype of Mike Haertel.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * <https://www.gnu.org/licenses/>
 */

/*
 * TL;DR: this is a porting of glibc mcheck into U-Boot
 *
 * This file contains no entities for external linkage.
 * So mcheck protection may be used in parallel, e.g. for "malloc_simple(..)" and "malloc(..)".
 * To do so, the file should be shared/include twice, - without linkage conflicts.
 * I.e. "core"-part is shared as a source, but not as a binary.
 * Maybe some optimization here make sense, to engage more binary sharing too.
 * But, currently I strive to keep it as simple, as possible.
 * And this, programmers'-only, mode don't pretend to be main.
 *
 * This library is aware of U-Boot specific. It's also aware of ARM alignment concerns.
 * Unlike glibc-clients, U-Boot has limited malloc-usage, and only one thread.
 * So it's better to make the protection heavier.
 * Thus overflow canary here is greater, than glibc's one. Underflow canary is bigger too.
 * U-Boot also allows to use fixed-size heap-registry, instead of double-linked list in glibc.
 *
 * Heavy canary allows to catch not only memset(..)-errors,
 * but overflow/underflow of struct-array access:
 *	{
 *		struct mystruct* p = malloc(sizeof(struct mystruct) * N);
 *		p[-1].field1 = 0;
 *		p[N].field2 = 13;
 *	}
 * TODO: In order to guarantee full coverage of that kind of errors, a user can add variable-size
 *       canaries here. So pre- and post-canary with size >= reqested_size, could be provided
 *       (with the price of 3x heap-usage). Therefore, it would catch 100% of changes beyond
 *       an array, for index(+1/-1) errors.
 *
 * U-Boot is a BL, not an OS with a lib. Activity of the library is set not in runtime,
 * rather in compile-time, by MCHECK_HEAP_PROTECTION macro. That guarantees that
 * we haven't missed first malloc.
 */

/*
 * Testing
 *  This library had been successfully tested for U-Boot @ ARM SoC chip / 64bits.
 *  Proven for both default and pedantic mode: confirms U-Boot to be clean, and catches
 *  intentional/testing corruptions. Working with malloc_trim is not tested.
 */
#ifndef _MCHECKCORE_INC_H
#define _MCHECKCORE_INC_H      1
#include "mcheck.h"

#if defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood memset

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((char)0xb6)
#define FREEFLOOD	((char)0xf5)
#define PADDINGFLOOD	((char)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
static char mcheck_pedantic_flag __section(".data") = 0;
static void *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
static size_t mcheck_chunk_count __section(".data") = 0;
static size_t mcheck_chunk_count_max __section(".data") = 0;

typedef unsigned long long mcheck_elem;
typedef struct {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
struct mcheck_hdr {
	size_t size; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

static void mcheck_default_abort(enum mcheck_status status, const void *p)
{
	const char *msg;

	switch (status) {
	case MCHECK_OK:
		msg = "memory is consistent, library is buggy\n";
		break;
	case MCHECK_HEAD:
		msg = "memory clobbered before allocated block\n";
		break;
	case MCHECK_TAIL:
		msg = "memory clobbered past end of allocated block\n";
		break;
	case MCHECK_FREE:
		msg = "block freed twice\n";
		break;
	default:
		msg = "bogus mcheck_status, library is buggy\n";
		break;
	}
	printf("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, msg, mcheck_chunk_count_max);
}

static mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

static inline size_t allign_size_up(size_t sz, size_t grain)
{
	return (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, sizeof(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(sizeof(struct mcheck_hdr), ALIGN)

static enum mcheck_status mcheck_OnNok(enum mcheck_status status, const void *p)
{
	(*mcheck_abortfunc)(status, p);
	return status;
}

static enum mcheck_status mcheck_checkhdr(const struct mcheck_hdr *hdr)
{
	int i;

	for (i = 0; i < CANARY_DEPTH; ++i)
		if (hdr->canary.elems[i] == MAGICFREE)
			return mcheck_OnNok(MCHECK_FREE, hdr + 1);

	for (i = 0; i < CANARY_DEPTH; ++i)
		if (hdr->canary.elems[i] != MAGICWORD)
			return mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	const size_t payload_size = hdr->size;
	const size_t payload_size_aligned = mcheck_allign_customer_size(payload_size);
	const size_t padd_size = payload_size_aligned - hdr->size;

	const char *payload = (const char *)&hdr[1];

	for (i = 0; i < padd_size; ++i)
		if (payload[payload_size + i] != PADDINGFLOOD)
			return mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	const mcheck_canary *tail = (const mcheck_canary *)&payload[payload_size_aligned];

	for (i = 0; i < CANARY_DEPTH; ++i)
		if (tail->elems[i] != MAGICTAIL)
			return mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	return MCHECK_OK;
}

enum { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
static void *mcheck_free_helper(void *ptr, int clean_content)
{
	if (!ptr)
		return ptr;

	struct mcheck_hdr *hdr = &((struct mcheck_hdr *)ptr)[-1];
	int i;

	mcheck_checkhdr(hdr);
	for (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	if (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	for (i = 0; i < REGISTRY_SZ; ++i)
		if (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			break;
		}

	--mcheck_chunk_count;
	return (char *)hdr - hdr->aln_skip;
}

static void *mcheck_free_prehook(void *ptr) { return mcheck_free_helper(ptr, CLEAN_CONTENT); }
static void *mcheck_reallocfree_prehook(void *ptr) { return mcheck_free_helper(ptr, KEEP_CONTENT); }

static size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_size(sz);
	return sizeof(struct mcheck_hdr) + sz + sizeof(mcheck_canary);
}

static void *mcheck_allocated_helper(void *altoghether_ptr, size_t customer_sz,
				     size_t alignment, int clean_content)
{
	const size_t slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - sizeof(struct mcheck_hdr) : 0;
	struct mcheck_hdr *hdr = (struct mcheck_hdr *)((char *)altoghether_ptr + slop);
	int i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	for (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	char *payload = (char *)&hdr[1];

	if (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	const size_t customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	for (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_count;
	if (mcheck_chunk_count > mcheck_chunk_count_max)
		mcheck_chunk_count_max = mcheck_chunk_count;

	for (i = 0; i < REGISTRY_SZ; ++i)
		if (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			return payload; // normal end
		}

	static char *overflow_msg = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	printf("%s", overflow_msg);
	overflow_msg = "(mcheck registry full)";
	return payload;
}

static void *mcheck_alloc_posthook(void *altoghether_ptr, size_t customer_sz)
{
	return mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

static void *mcheck_alloc_noclean_posthook(void *altoghether_ptr, size_t customer_sz)
{
	return mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

static size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	return mcheck_evaluate_memalign_prefix_size(alig) + sz + sizeof(mcheck_canary);
}

static void *mcheck_memalign_posthook(size_t alignment, void *altoghether_ptr, size_t customer_sz)
{
	return mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

static enum mcheck_status mcheck_mprobe(void *ptr)
{
	struct mcheck_hdr *hdr = &((struct mcheck_hdr *)ptr)[-1];

	return mcheck_checkhdr(hdr);
}

static void mcheck_pedantic_check(void)
{
	int i;

	for (i = 0; i < REGISTRY_SZ; ++i)
		if (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

static void mcheck_pedantic_prehook(void)
{
	if (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

static void mcheck_initialize(mcheck_abortfunc_t new_func, char pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

void mcheck_on_ramrelocation(size_t offset)
{
	char *p;
	int i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	for (i = 0; i < REGISTRY_SZ; ++i)
		if ((p = mcheck_registry[i]) != NULL ) {
			printf("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_count = 0;
}
#endif
#endif
