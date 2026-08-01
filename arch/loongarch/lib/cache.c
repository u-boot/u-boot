// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 * Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/loongarch.h>
#include <cpu_func.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

#define CPUCFG_LX_IUPRE		BIT(0)
#define CPUCFG_LX_IUUNIFY	BIT(1)
#define CPUCFG_LX_IUINCL	BIT(3)

DECLARE_GLOBAL_DATA_PTR;

static inline void flush_cache_line_index(unsigned int index,
					  unsigned long addr)
{
#define do_flush(index)							\
	case index:							\
		cache_op(FIELD_PREP(CACHE_OP, CACHE_INDEX_INVWB) |	\
			 FIELD_PREP(CACHE_INDEX, index),		\
			 index);					\
		break;

	switch (index) {
		do_flush(0);
		do_flush(1);
		do_flush(2);
		do_flush(3);
		do_flush(4);
		do_flush(5);
	}

#undef do_flush
}

static inline void flush_cache_line_hit(unsigned int index, unsigned long addr)
{
#define do_flush(index)							\
	case index:							\
		cache_op(FIELD_PREP(CACHE_OP, CACHE_HIT_INVWB) |	\
			 FIELD_PREP(CACHE_INDEX, index),		\
			 addr);						\
		break;

	switch (index) {
		do_flush(0);
		do_flush(1);
		do_flush(2);
		do_flush(3);
		do_flush(4);
		do_flush(5);
	}

#undef do_flush
}

void invalidate_icache_all(void)
{
	asm volatile ("\tibar 0\n"::);
}

static void flush_dcache_level_all(unsigned int level)
{
	unsigned int way, set;
	unsigned long index = 0;

	for (set = 0; set < gd->arch.dcache_sets[level]; set++) {
		for (way = 0; way < gd->arch.dcache_ways[level]; way++) {
			flush_cache_line_index(gd->arch.dcache_index[level],
					       index);
			index++;
		}

		index -= gd->arch.dcache_ways[level];
		index += gd->arch.dcache_linesizes[level];
	}
}

__weak void flush_dcache_all(void)
{
	unsigned int level;

	if (gd->arch.dcache_inclusive) {
		flush_dcache_level_all(gd->arch.dcache_levels - 1);
		return;
	}

	for (level = 0; level < gd->arch.dcache_levels; level++)
		flush_dcache_level_all(level);
}

static void flush_dcache_level(unsigned int level, unsigned long addr,
			       unsigned long end)
{
	for (; addr < end; addr += gd->arch.dcache_linesizes[level])
		flush_cache_line_hit(gd->arch.dcache_index[level], addr);
}

__weak void flush_dcache_range(unsigned long start, unsigned long end)
{
	unsigned int level;

	if (gd->arch.dcache_inclusive) {
		flush_dcache_level(gd->arch.dcache_levels - 1, start, end);
		return;
	}

	for (level = 0; level < gd->arch.dcache_levels; level++)
		flush_dcache_level(level, start, end);
}

__weak void invalidate_icache_range(unsigned long start, unsigned long end)
{
	/* LoongArch mandatory hardware I-Cache coherence */
	invalidate_icache_all();
}

__weak void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	flush_dcache_range(start, end);
}

__weak void cache_flush(void)
{
	flush_dcache_all();
}

__weak void cache_invalidate(void)
{
	flush_dcache_all();
}

__weak void flush_cache(unsigned long addr, unsigned long size)
{
	invalidate_icache_range(addr, addr + size);
	flush_dcache_range(addr, addr + size);
}

__weak void dcache_enable(void)
{
}

__weak void dcache_disable(void)
{
}

__weak int dcache_status(void)
{
	return 0;
}

static void populate_dcache_properties(u32 cfg0, unsigned int level,
				       unsigned int cfgoff)
{
	u32 cfg1 = read_cpucfg(cfgoff);

	if (level != 0)
		gd->arch.dcache_inclusive = cfg0 & CPUCFG_LX_IUINCL;

	gd->arch.dcache_ways[level] = FIELD_GET(CPUCFG_CACHE_WAYS_M, cfg1) + 1;
	gd->arch.dcache_sets[level] = 1 << FIELD_GET(CPUCFG_CACHE_SETS_M, cfg1);
	gd->arch.dcache_linesizes[level] = 1 << FIELD_GET(CPUCFG_CACHE_LSIZE_M, cfg1);
}

void probe_caches(void)
{
	unsigned int level = 0, index = 0;
	u32 cfg = read_cpucfg(LOONGARCH_CPUCFG16);

	if (cfg & CPUCFG16_L1_IUPRE) {
		if (cfg & CPUCFG16_L1_IUUNIFY) {
			gd->arch.dcache_index[level] = index;
			populate_dcache_properties(cfg, level++,
						   LOONGARCH_CPUCFG17);
		}

		index++;
	}

	if (cfg & CPUCFG16_L1_DPRE) {
		gd->arch.dcache_index[level] = index++;
		populate_dcache_properties(cfg, level++, LOONGARCH_CPUCFG18);
	}

	cfg >>= 3;

	for (; level < 3; level++) {
		if (cfg & CPUCFG_LX_IUPRE && cfg & CPUCFG_LX_IUUNIFY) {
			gd->arch.dcache_index[level] = index++;
			populate_dcache_properties(cfg, level,
						   LOONGARCH_CPUCFG17 + level);
		} else {
			break;
		}

		cfg >>= 7;
	}

	gd->arch.dcache_levels = level;
}

__weak void enable_caches(void)
{
	cache_invalidate();
	/* Enable cache for direct address translation mode */
	csr_xchg64(1 << CSR_CRMD_DACM_SHIFT, CSR_CRMD_DACM, LOONGARCH_CSR_CRMD);
}
