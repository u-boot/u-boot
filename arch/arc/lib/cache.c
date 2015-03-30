/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm/arcregs.h>
#include <asm/cache.h>

/* Bit values in IC_CTRL */
#define IC_CTRL_CACHE_DISABLE	(1 << 0)

/* Bit values in DC_CTRL */
#define DC_CTRL_CACHE_DISABLE	(1 << 0)
#define DC_CTRL_INV_MODE_FLUSH	(1 << 6)
#define DC_CTRL_FLUSH_STATUS	(1 << 8)
#define CACHE_VER_NUM_MASK	0xF
#define SLC_CTRL_SB		(1 << 2)

int icache_status(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_IC_BUILD) & CACHE_VER_NUM_MASK))
		return 0;

	return (read_aux_reg(ARC_AUX_IC_CTRL) & IC_CTRL_CACHE_DISABLE) !=
	       IC_CTRL_CACHE_DISABLE;
}

void icache_enable(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_IC_BUILD) & CACHE_VER_NUM_MASK))
		return;

	write_aux_reg(ARC_AUX_IC_CTRL, read_aux_reg(ARC_AUX_IC_CTRL) &
		      ~IC_CTRL_CACHE_DISABLE);
}

void icache_disable(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_IC_BUILD) & CACHE_VER_NUM_MASK))
		return;

	write_aux_reg(ARC_AUX_IC_CTRL, read_aux_reg(ARC_AUX_IC_CTRL) |
		      IC_CTRL_CACHE_DISABLE);
}

void invalidate_icache_all(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_IC_BUILD) & CACHE_VER_NUM_MASK))
		return;

	/* Any write to IC_IVIC register triggers invalidation of entire I$ */
	write_aux_reg(ARC_AUX_IC_IVIC, 1);
}

int dcache_status(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_DC_BUILD) & CACHE_VER_NUM_MASK))
		return 0;

	return (read_aux_reg(ARC_AUX_DC_CTRL) & DC_CTRL_CACHE_DISABLE) !=
		DC_CTRL_CACHE_DISABLE;
}

void dcache_enable(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_DC_BUILD) & CACHE_VER_NUM_MASK))
		return;

	write_aux_reg(ARC_AUX_DC_CTRL, read_aux_reg(ARC_AUX_DC_CTRL) &
		      ~(DC_CTRL_INV_MODE_FLUSH | DC_CTRL_CACHE_DISABLE));
}

void dcache_disable(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_DC_BUILD) & CACHE_VER_NUM_MASK))
		return;

	write_aux_reg(ARC_AUX_DC_CTRL, read_aux_reg(ARC_AUX_DC_CTRL) |
		      DC_CTRL_CACHE_DISABLE);
}

void flush_dcache_all(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_DC_BUILD) & CACHE_VER_NUM_MASK))
		return;

	/* Do flush of entire cache */
	write_aux_reg(ARC_AUX_DC_FLSH, 1);

	/* Wait flush end */
	while (read_aux_reg(ARC_AUX_DC_CTRL) & DC_CTRL_FLUSH_STATUS)
		;
}

#ifndef CONFIG_SYS_DCACHE_OFF
static void dcache_flush_line(unsigned addr)
{
#if (CONFIG_ARC_MMU_VER == 3)
	write_aux_reg(ARC_AUX_DC_PTAG, addr);
#endif
	write_aux_reg(ARC_AUX_DC_FLDL, addr);

	/* Wait flush end */
	while (read_aux_reg(ARC_AUX_DC_CTRL) & DC_CTRL_FLUSH_STATUS)
		;

#ifndef CONFIG_SYS_ICACHE_OFF
	/*
	 * Invalidate I$ for addresses range just flushed from D$.
	 * If we try to execute data flushed above it will be valid/correct
	 */
#if (CONFIG_ARC_MMU_VER == 3)
	write_aux_reg(ARC_AUX_IC_PTAG, addr);
#endif
	write_aux_reg(ARC_AUX_IC_IVIL, addr);
#endif /* CONFIG_SYS_ICACHE_OFF */
}
#endif /* CONFIG_SYS_DCACHE_OFF */

void flush_dcache_range(unsigned long start, unsigned long end)
{
#ifndef CONFIG_SYS_DCACHE_OFF
	unsigned int addr;

	start = start & (~(CONFIG_SYS_CACHELINE_SIZE - 1));
	end = end & (~(CONFIG_SYS_CACHELINE_SIZE - 1));

	for (addr = start; addr <= end; addr += CONFIG_SYS_CACHELINE_SIZE)
		dcache_flush_line(addr);
#endif /* CONFIG_SYS_DCACHE_OFF */
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
#ifndef CONFIG_SYS_DCACHE_OFF
	unsigned int addr;

	start = start & (~(CONFIG_SYS_CACHELINE_SIZE - 1));
	end = end & (~(CONFIG_SYS_CACHELINE_SIZE - 1));

	for (addr = start; addr <= end; addr += CONFIG_SYS_CACHELINE_SIZE) {
#if (CONFIG_ARC_MMU_VER == 3)
		write_aux_reg(ARC_AUX_DC_PTAG, addr);
#endif
		write_aux_reg(ARC_AUX_DC_IVDL, addr);
	}
#endif /* CONFIG_SYS_DCACHE_OFF */
}

void invalidate_dcache_all(void)
{
	/* If no cache in CPU exit immediately */
	if (!(read_aux_reg(ARC_BCR_DC_BUILD) & CACHE_VER_NUM_MASK))
		return;

	/* Write 1 to DC_IVDC register triggers invalidation of entire D$ */
	write_aux_reg(ARC_AUX_DC_IVDC, 1);
}

void flush_cache(unsigned long start, unsigned long size)
{
	flush_dcache_range(start, start + size);
}

#ifdef CONFIG_ISA_ARCV2
void slc_enable(void)
{
	/* If SLC ver = 0, no SLC present in CPU */
	if (!(read_aux_reg(ARC_BCR_SLC) & 0xff))
		return;

	write_aux_reg(ARC_AUX_SLC_CONTROL,
		      read_aux_reg(ARC_AUX_SLC_CONTROL) & ~1);
}

void slc_disable(void)
{
	/* If SLC ver = 0, no SLC present in CPU */
	if (!(read_aux_reg(ARC_BCR_SLC) & 0xff))
		return;

	write_aux_reg(ARC_AUX_SLC_CONTROL,
		      read_aux_reg(ARC_AUX_SLC_CONTROL) | 1);
}

void slc_flush(void)
{
	/* If SLC ver = 0, no SLC present in CPU */
	if (!(read_aux_reg(ARC_BCR_SLC) & 0xff))
		return;

	write_aux_reg(ARC_AUX_SLC_FLUSH, 1);

	/* Wait flush end */
	while (read_aux_reg(ARC_AUX_SLC_CONTROL) & SLC_CTRL_SB)
		;
}

void slc_invalidate(void)
{
	/* If SLC ver = 0, no SLC present in CPU */
	if (!(read_aux_reg(ARC_BCR_SLC) & 0xff))
		return;

	write_aux_reg(ARC_AUX_SLC_INVALIDATE, 1);
}

#endif /* CONFIG_ISA_ARCV2 */
