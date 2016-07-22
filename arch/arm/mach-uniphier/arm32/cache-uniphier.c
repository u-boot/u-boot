/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <asm/armv7.h>

#include "ssc-regs.h"

#ifdef CONFIG_UNIPHIER_L2CACHE_ON
static void uniphier_cache_sync(void)
{
	/* drain internal buffers */
	writel(UNIPHIER_SSCOPE_CM_SYNC, UNIPHIER_SSCOPE);
	/* need a read back to confirm */
	readl(UNIPHIER_SSCOPE);
}

static void uniphier_cache_maint_all(u32 operation)
{
	/* clear the complete notification flag */
	writel(UNIPHIER_SSCOLPQS_EF, UNIPHIER_SSCOLPQS);

	/* try until the command is successfully set */
	do {
		writel(UNIPHIER_SSCOQM_S_ALL | UNIPHIER_SSCOQM_CE | operation,
		       UNIPHIER_SSCOQM);
	} while (readl(UNIPHIER_SSCOPPQSEF) &
		 (UNIPHIER_SSCOPPQSEF_FE | UNIPHIER_SSCOPPQSEF_OE));

	/* wait until the operation is completed */
	while (readl(UNIPHIER_SSCOLPQS) != UNIPHIER_SSCOLPQS_EF)
		;

	uniphier_cache_sync();
}

void v7_outer_cache_flush_all(void)
{
	uniphier_cache_maint_all(UNIPHIER_SSCOQM_CM_FLUSH);
}

void v7_outer_cache_inval_all(void)
{
	uniphier_cache_maint_all(UNIPHIER_SSCOQM_CM_INV);
}

static void __uniphier_cache_maint_range(u32 start, u32 size, u32 operation)
{
	/* clear the complete notification flag */
	writel(UNIPHIER_SSCOLPQS_EF, UNIPHIER_SSCOLPQS);

	/* try until the command is successfully set */
	do {
		writel(UNIPHIER_SSCOQM_S_RANGE | UNIPHIER_SSCOQM_CE | operation,
		       UNIPHIER_SSCOQM);
		writel(start, UNIPHIER_SSCOQAD);
		writel(size, UNIPHIER_SSCOQSZ);

	} while (readl(UNIPHIER_SSCOPPQSEF) &
		 (UNIPHIER_SSCOPPQSEF_FE | UNIPHIER_SSCOPPQSEF_OE));

	/* wait until the operation is completed */
	while (readl(UNIPHIER_SSCOLPQS) != UNIPHIER_SSCOLPQS_EF)
		;
}

static void uniphier_cache_maint_range(u32 start, u32 end, u32 operation)
{
	u32 size;

	/*
	 * If start address is not aligned to cache-line,
	 * do cache operation for the first cache-line
	 */
	start = start & ~(UNIPHIER_SSC_LINE_SIZE - 1);

	size = end - start;

	if (unlikely(size >= (u32)(-UNIPHIER_SSC_LINE_SIZE))) {
		/* this means cache operation for all range */
		uniphier_cache_maint_all(operation);
		return;
	}

	/*
	 * If end address is not aligned to cache-line,
	 * do cache operation for the last cache-line
	 */
	size = ALIGN(size, UNIPHIER_SSC_LINE_SIZE);

	while (size) {
		u32 chunk_size = size > UNIPHIER_SSC_RANGE_OP_MAX_SIZE ?
					UNIPHIER_SSC_RANGE_OP_MAX_SIZE : size;
		__uniphier_cache_maint_range(start, chunk_size, operation);

		start += chunk_size;
		size -= chunk_size;
	}

	uniphier_cache_sync();
}

void v7_outer_cache_flush_range(u32 start, u32 end)
{
	uniphier_cache_maint_range(start, end, UNIPHIER_SSCOQM_CM_FLUSH);
}

void v7_outer_cache_inval_range(u32 start, u32 end)
{
	if (start & (UNIPHIER_SSC_LINE_SIZE - 1)) {
		start &= ~(UNIPHIER_SSC_LINE_SIZE - 1);
		__uniphier_cache_maint_range(start, UNIPHIER_SSC_LINE_SIZE,
					     UNIPHIER_SSCOQM_CM_FLUSH);
		start += UNIPHIER_SSC_LINE_SIZE;
	}

	if (start >= end) {
		uniphier_cache_sync();
		return;
	}

	if (end & (UNIPHIER_SSC_LINE_SIZE - 1)) {
		end &= ~(UNIPHIER_SSC_LINE_SIZE - 1);
		__uniphier_cache_maint_range(end, UNIPHIER_SSC_LINE_SIZE,
					     UNIPHIER_SSCOQM_CM_FLUSH);
	}

	if (start >= end) {
		uniphier_cache_sync();
		return;
	}

	uniphier_cache_maint_range(start, end, UNIPHIER_SSCOQM_CM_INV);
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
