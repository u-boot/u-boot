/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot file of the same name
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_MTRR_H
#define _ASM_MTRR_H

/* MTRR region types */
#define MTRR_TYPE_UNCACHEABLE	0
#define MTRR_TYPE_WRCOMB	1
#define MTRR_TYPE_WRTHROUGH	4
#define MTRR_TYPE_WRPROT	5
#define MTRR_TYPE_WRBACK	6

#define MTRR_TYPE_COUNT		7

#define MTRR_CAP_MSR		0x0fe
#define MTRR_DEF_TYPE_MSR	0x2ff

#define MTRR_DEF_TYPE_EN	(1 << 11)
#define MTRR_DEF_TYPE_FIX_EN	(1 << 10)

#define MTRR_PHYS_BASE_MSR(reg)	(0x200 + 2 * (reg))
#define MTRR_PHYS_MASK_MSR(reg)	(0x200 + 2 * (reg) + 1)

#define MTRR_PHYS_MASK_VALID	(1 << 11)

#define MTRR_BASE_TYPE_MASK	0x7

/* Number of MTRRs supported */
#define MTRR_COUNT		8

#if !defined(__ASSEMBLER__)

/**
 * Information about the previous MTRR state, set up by mtrr_open()
 *
 * @deftype:		Previous value of MTRR_DEF_TYPE_MSR
 * @enable_cache:	true if cache was enabled
 */
struct mtrr_state {
	uint64_t deftype;
	bool enable_cache;
};

/**
 * mtrr_open() - Prepare to adjust MTRRs
 *
 * Use mtrr_open() passing in a structure - this function will init it. Then
 * when done, pass the same structure to mtrr_close() to re-enable MTRRs and
 * possibly the cache.
 *
 * @state:	Empty structure to pass in to hold settings
 */
void mtrr_open(struct mtrr_state *state);

/**
 * mtrr_open() - Clean up after adjusting MTRRs, and enable them
 *
 * This uses the structure containing information returned from mtrr_open().
 *
 * @state:	Structure from mtrr_open()
 */
void mtrr_close(struct mtrr_state *state);

/**
 * mtrr_add_request() - Add a new MTRR request
 *
 * This adds a request for a memory region to be set up in a particular way.
 *
 * @type:	Requested type (MTRR_TYPE_)
 * @start:	Start address
 * @size:	Size
 *
 * @return:	0 on success, non-zero on failure
 */
int mtrr_add_request(int type, uint64_t start, uint64_t size);

/**
 * mtrr_commit() - set up the MTRR registers based on current requests
 *
 * This sets up MTRRs for the available DRAM and the requests received so far.
 * It must be called with caches disabled.
 *
 * @do_caches:	true if caches are currently on
 *
 * @return:	0 on success, non-zero on failure
 */
int mtrr_commit(bool do_caches);

#endif

#if ((CONFIG_XIP_ROM_SIZE & (CONFIG_XIP_ROM_SIZE - 1)) != 0)
# error "CONFIG_XIP_ROM_SIZE is not a power of 2"
#endif

#if ((CONFIG_CACHE_ROM_SIZE & (CONFIG_CACHE_ROM_SIZE - 1)) != 0)
# error "CONFIG_CACHE_ROM_SIZE is not a power of 2"
#endif

#define CACHE_ROM_BASE	(((1 << 20) - (CONFIG_CACHE_ROM_SIZE >> 12)) << 12)

#endif
