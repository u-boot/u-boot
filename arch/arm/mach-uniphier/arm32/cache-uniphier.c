/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <asm/armv7.h>
#include <asm/processor.h>

#include "cache-uniphier.h"
#include "ssc-regs.h"

#define UNIPHIER_SSCOQAD_IS_NEEDED(op) \
		((op & UNIPHIER_SSCOQM_S_MASK) == UNIPHIER_SSCOQM_S_RANGE)
#define UNIPHIER_SSCOQWM_IS_NEEDED(op) \
		((op & UNIPHIER_SSCOQM_TID_MASK) == UNIPHIER_SSCOQM_TID_WAY)

/* uniphier_cache_sync - perform a sync point for a particular cache level */
static void uniphier_cache_sync(void)
{
	/* drain internal buffers */
	writel(UNIPHIER_SSCOPE_CM_SYNC, UNIPHIER_SSCOPE);
	/* need a read back to confirm */
	readl(UNIPHIER_SSCOPE);
}

/**
 * uniphier_cache_maint_common - run a queue operation
 *
 * @start: start address of range operation (don't care for "all" operation)
 * @size: data size of range operation (don't care for "all" operation)
 * @ways: target ways (don't care for operations other than pre-fetch, touch
 * @operation: flags to specify the desired cache operation
 */
static void uniphier_cache_maint_common(u32 start, u32 size, u32 ways,
					u32 operation)
{
	/* clear the complete notification flag */
	writel(UNIPHIER_SSCOLPQS_EF, UNIPHIER_SSCOLPQS);

	do {
		/* set cache operation */
		writel(UNIPHIER_SSCOQM_CE | operation, UNIPHIER_SSCOQM);

		/* set address range if needed */
		if (likely(UNIPHIER_SSCOQAD_IS_NEEDED(operation))) {
			writel(start, UNIPHIER_SSCOQAD);
			writel(size, UNIPHIER_SSCOQSZ);
		}

		/* set target ways if needed */
		if (unlikely(UNIPHIER_SSCOQWM_IS_NEEDED(operation)))
			writel(ways, UNIPHIER_SSCOQWN);
	} while (unlikely(readl(UNIPHIER_SSCOPPQSEF) &
			  (UNIPHIER_SSCOPPQSEF_FE | UNIPHIER_SSCOPPQSEF_OE)));

	/* wait until the operation is completed */
	while (likely(readl(UNIPHIER_SSCOLPQS) != UNIPHIER_SSCOLPQS_EF))
		cpu_relax();
}

static void uniphier_cache_maint_all(u32 operation)
{
	uniphier_cache_maint_common(0, 0, 0, UNIPHIER_SSCOQM_S_ALL | operation);

	uniphier_cache_sync();
}

static void uniphier_cache_maint_range(u32 start, u32 end, u32 ways,
				       u32 operation)
{
	u32 size;

	/*
	 * If the start address is not aligned,
	 * perform a cache operation for the first cache-line
	 */
	start = start & ~(UNIPHIER_SSC_LINE_SIZE - 1);

	size = end - start;

	if (unlikely(size >= (u32)(-UNIPHIER_SSC_LINE_SIZE))) {
		/* this means cache operation for all range */
		uniphier_cache_maint_all(operation);
		return;
	}

	/*
	 * If the end address is not aligned,
	 * perform a cache operation for the last cache-line
	 */
	size = ALIGN(size, UNIPHIER_SSC_LINE_SIZE);

	while (size) {
		u32 chunk_size = min_t(u32, size, UNIPHIER_SSC_RANGE_OP_MAX_SIZE);

		uniphier_cache_maint_common(start, chunk_size, ways,
					    UNIPHIER_SSCOQM_S_RANGE | operation);

		start += chunk_size;
		size -= chunk_size;
	}

	uniphier_cache_sync();
}

void uniphier_cache_prefetch_range(u32 start, u32 end, u32 ways)
{
	uniphier_cache_maint_range(start, end, ways,
				   UNIPHIER_SSCOQM_TID_WAY |
				   UNIPHIER_SSCOQM_CM_PREFETCH);
}

void uniphier_cache_touch_range(u32 start, u32 end, u32 ways)
{
	uniphier_cache_maint_range(start, end, ways,
				   UNIPHIER_SSCOQM_TID_WAY |
				   UNIPHIER_SSCOQM_CM_TOUCH);
}

void uniphier_cache_touch_zero_range(u32 start, u32 end, u32 ways)
{
	uniphier_cache_maint_range(start, end, ways,
				   UNIPHIER_SSCOQM_TID_WAY |
				   UNIPHIER_SSCOQM_CM_TOUCH_ZERO);
}

#ifdef CONFIG_UNIPHIER_L2CACHE_ON
void v7_outer_cache_flush_all(void)
{
	uniphier_cache_maint_all(UNIPHIER_SSCOQM_CM_FLUSH);
}

void v7_outer_cache_inval_all(void)
{
	uniphier_cache_maint_all(UNIPHIER_SSCOQM_CM_INV);
}

void v7_outer_cache_flush_range(u32 start, u32 end)
{
	uniphier_cache_maint_range(start, end, 0, UNIPHIER_SSCOQM_CM_FLUSH);
}

void v7_outer_cache_inval_range(u32 start, u32 end)
{
	if (start & (UNIPHIER_SSC_LINE_SIZE - 1)) {
		start &= ~(UNIPHIER_SSC_LINE_SIZE - 1);
		uniphier_cache_maint_range(start, UNIPHIER_SSC_LINE_SIZE, 0,
					   UNIPHIER_SSCOQM_CM_FLUSH);
		start += UNIPHIER_SSC_LINE_SIZE;
	}

	if (start >= end) {
		uniphier_cache_sync();
		return;
	}

	if (end & (UNIPHIER_SSC_LINE_SIZE - 1)) {
		end &= ~(UNIPHIER_SSC_LINE_SIZE - 1);
		uniphier_cache_maint_range(end, UNIPHIER_SSC_LINE_SIZE, 0,
					   UNIPHIER_SSCOQM_CM_FLUSH);
	}

	if (start >= end) {
		uniphier_cache_sync();
		return;
	}

	uniphier_cache_maint_range(start, end, 0, UNIPHIER_SSCOQM_CM_INV);
}

void v7_outer_cache_enable(void)
{
	u32 tmp;

	writel(U32_MAX, UNIPHIER_SSCLPDAWCR);	/* activate all ways */
	tmp = readl(UNIPHIER_SSCC);
	tmp |= UNIPHIER_SSCC_ON;
	writel(tmp, UNIPHIER_SSCC);
}
#endif

void v7_outer_cache_disable(void)
{
	u32 tmp;

	tmp = readl(UNIPHIER_SSCC);
	tmp &= ~UNIPHIER_SSCC_ON;
	writel(tmp, UNIPHIER_SSCC);
}

void enable_caches(void)
{
	dcache_enable();
}
