/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <asm/arcregs.h>
#include <asm/cache.h>

#define CACHE_LINE_MASK		(~(CONFIG_SYS_CACHELINE_SIZE - 1))

/* Bit values in IC_CTRL */
#define IC_CTRL_CACHE_DISABLE	(1 << 0)

/* Bit values in DC_CTRL */
#define DC_CTRL_CACHE_DISABLE	(1 << 0)
#define DC_CTRL_INV_MODE_FLUSH	(1 << 6)
#define DC_CTRL_FLUSH_STATUS	(1 << 8)
#define CACHE_VER_NUM_MASK	0xF
#define SLC_CTRL_SB		(1 << 2)

#define OP_INV		0x1
#define OP_FLUSH	0x2
#define OP_INV_IC	0x3

#ifdef CONFIG_ISA_ARCV2
/*
 * By default that variable will fall into .bss section.
 * But .bss section is not relocated and so it will be initilized before
 * relocation but will be used after being zeroed.
 */
int slc_line_sz __section(".data");
int slc_exists __section(".data");

static unsigned int __before_slc_op(const int op)
{
	unsigned int reg = reg;

	if (op == OP_INV) {
		/*
		 * IM is set by default and implies Flush-n-inv
		 * Clear it here for vanilla inv
		 */
		reg = read_aux_reg(ARC_AUX_SLC_CTRL);
		write_aux_reg(ARC_AUX_SLC_CTRL, reg & ~DC_CTRL_INV_MODE_FLUSH);
	}

	return reg;
}

static void __after_slc_op(const int op, unsigned int reg)
{
	if (op & OP_FLUSH)	/* flush / flush-n-inv both wait */
		while (read_aux_reg(ARC_AUX_SLC_CTRL) &
		       DC_CTRL_FLUSH_STATUS)
			;

	/* Switch back to default Invalidate mode */
	if (op == OP_INV)
		write_aux_reg(ARC_AUX_SLC_CTRL, reg | DC_CTRL_INV_MODE_FLUSH);
}

static inline void __slc_line_loop(unsigned long paddr, unsigned long sz,
				   const int op)
{
	unsigned int aux_cmd;
	int num_lines;

#define SLC_LINE_MASK	(~(slc_line_sz - 1))

	aux_cmd = op & OP_INV ? ARC_AUX_SLC_IVDL : ARC_AUX_SLC_FLDL;

	sz += paddr & ~SLC_LINE_MASK;
	paddr &= SLC_LINE_MASK;

	num_lines = DIV_ROUND_UP(sz, slc_line_sz);

	while (num_lines-- > 0) {
		write_aux_reg(aux_cmd, paddr);
		paddr += slc_line_sz;
	}
}

static inline void __slc_entire_op(const int cacheop)
{
	int aux;
	unsigned int ctrl_reg = __before_slc_op(cacheop);

	if (cacheop & OP_INV)	/* Inv or flush-n-inv use same cmd reg */
		aux = ARC_AUX_SLC_INVALIDATE;
	else
		aux = ARC_AUX_SLC_FLUSH;

	write_aux_reg(aux, 0x1);

	__after_slc_op(cacheop, ctrl_reg);
}

static inline void __slc_line_op(unsigned long paddr, unsigned long sz,
				 const int cacheop)
{
	unsigned int ctrl_reg = __before_slc_op(cacheop);
	__slc_line_loop(paddr, sz, cacheop);
	__after_slc_op(cacheop, ctrl_reg);
}
#else
#define __slc_entire_op(cacheop)
#define __slc_line_op(paddr, sz, cacheop)
#endif

static inline int icache_exists(void)
{
	/* Check if Instruction Cache is available */
	if (read_aux_reg(ARC_BCR_IC_BUILD) & CACHE_VER_NUM_MASK)
		return 1;
	else
		return 0;
}

static inline int dcache_exists(void)
{
	/* Check if Data Cache is available */
	if (read_aux_reg(ARC_BCR_DC_BUILD) & CACHE_VER_NUM_MASK)
		return 1;
	else
		return 0;
}

void cache_init(void)
{
#ifdef CONFIG_ISA_ARCV2
	/* Check if System-Level Cache (SLC) is available */
	if (read_aux_reg(ARC_BCR_SLC) & CACHE_VER_NUM_MASK) {
#define LSIZE_OFFSET	4
#define LSIZE_MASK	3
		if (read_aux_reg(ARC_AUX_SLC_CONFIG) &
		    (LSIZE_MASK << LSIZE_OFFSET))
			slc_line_sz = 64;
		else
			slc_line_sz = 128;
		slc_exists = 1;
	} else {
		slc_exists = 0;
	}
#endif
}

int icache_status(void)
{
	if (!icache_exists())
		return 0;

	if (read_aux_reg(ARC_AUX_IC_CTRL) & IC_CTRL_CACHE_DISABLE)
		return 0;
	else
		return 1;
}

void icache_enable(void)
{
	if (icache_exists())
		write_aux_reg(ARC_AUX_IC_CTRL, read_aux_reg(ARC_AUX_IC_CTRL) &
			      ~IC_CTRL_CACHE_DISABLE);
}

void icache_disable(void)
{
	if (icache_exists())
		write_aux_reg(ARC_AUX_IC_CTRL, read_aux_reg(ARC_AUX_IC_CTRL) |
			      IC_CTRL_CACHE_DISABLE);
}

#ifndef CONFIG_SYS_DCACHE_OFF
void invalidate_icache_all(void)
{
	/* Any write to IC_IVIC register triggers invalidation of entire I$ */
	if (icache_status()) {
		write_aux_reg(ARC_AUX_IC_IVIC, 1);
		read_aux_reg(ARC_AUX_IC_CTRL);	/* blocks */
	}
}
#else
void invalidate_icache_all(void)
{
}
#endif

int dcache_status(void)
{
	if (!dcache_exists())
		return 0;

	if (read_aux_reg(ARC_AUX_DC_CTRL) & DC_CTRL_CACHE_DISABLE)
		return 0;
	else
		return 1;
}

void dcache_enable(void)
{
	if (!dcache_exists())
		return;

	write_aux_reg(ARC_AUX_DC_CTRL, read_aux_reg(ARC_AUX_DC_CTRL) &
		      ~(DC_CTRL_INV_MODE_FLUSH | DC_CTRL_CACHE_DISABLE));
}

void dcache_disable(void)
{
	if (!dcache_exists())
		return;

	write_aux_reg(ARC_AUX_DC_CTRL, read_aux_reg(ARC_AUX_DC_CTRL) |
		      DC_CTRL_CACHE_DISABLE);
}

#ifndef CONFIG_SYS_DCACHE_OFF
/*
 * Common Helper for Line Operations on {I,D}-Cache
 */
static inline void __cache_line_loop(unsigned long paddr, unsigned long sz,
				     const int cacheop)
{
	unsigned int aux_cmd;
#if (CONFIG_ARC_MMU_VER == 3)
	unsigned int aux_tag;
#endif
	int num_lines;

	if (cacheop == OP_INV_IC) {
		aux_cmd = ARC_AUX_IC_IVIL;
#if (CONFIG_ARC_MMU_VER == 3)
		aux_tag = ARC_AUX_IC_PTAG;
#endif
	} else {
		/* d$ cmd: INV (discard or wback-n-discard) OR FLUSH (wback) */
		aux_cmd = cacheop & OP_INV ? ARC_AUX_DC_IVDL : ARC_AUX_DC_FLDL;
#if (CONFIG_ARC_MMU_VER == 3)
		aux_tag = ARC_AUX_DC_PTAG;
#endif
	}

	sz += paddr & ~CACHE_LINE_MASK;
	paddr &= CACHE_LINE_MASK;

	num_lines = DIV_ROUND_UP(sz, CONFIG_SYS_CACHELINE_SIZE);

	while (num_lines-- > 0) {
#if (CONFIG_ARC_MMU_VER == 3)
		write_aux_reg(aux_tag, paddr);
#endif
		write_aux_reg(aux_cmd, paddr);
		paddr += CONFIG_SYS_CACHELINE_SIZE;
	}
}

static unsigned int __before_dc_op(const int op)
{
	unsigned int reg;

	if (op == OP_INV) {
		/*
		 * IM is set by default and implies Flush-n-inv
		 * Clear it here for vanilla inv
		 */
		reg = read_aux_reg(ARC_AUX_DC_CTRL);
		write_aux_reg(ARC_AUX_DC_CTRL, reg & ~DC_CTRL_INV_MODE_FLUSH);
	}

	return reg;
}

static void __after_dc_op(const int op, unsigned int reg)
{
	if (op & OP_FLUSH)	/* flush / flush-n-inv both wait */
		while (read_aux_reg(ARC_AUX_DC_CTRL) & DC_CTRL_FLUSH_STATUS)
			;

	/* Switch back to default Invalidate mode */
	if (op == OP_INV)
		write_aux_reg(ARC_AUX_DC_CTRL, reg | DC_CTRL_INV_MODE_FLUSH);
}

static inline void __dc_entire_op(const int cacheop)
{
	int aux;
	unsigned int ctrl_reg = __before_dc_op(cacheop);

	if (cacheop & OP_INV)	/* Inv or flush-n-inv use same cmd reg */
		aux = ARC_AUX_DC_IVDC;
	else
		aux = ARC_AUX_DC_FLSH;

	write_aux_reg(aux, 0x1);

	__after_dc_op(cacheop, ctrl_reg);
}

static inline void __dc_line_op(unsigned long paddr, unsigned long sz,
				const int cacheop)
{
	unsigned int ctrl_reg = __before_dc_op(cacheop);
	__cache_line_loop(paddr, sz, cacheop);
	__after_dc_op(cacheop, ctrl_reg);
}
#else
#define __dc_entire_op(cacheop)
#define __dc_line_op(paddr, sz, cacheop)
#endif /* !CONFIG_SYS_DCACHE_OFF */

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	__dc_line_op(start, end - start, OP_INV);
#ifdef CONFIG_ISA_ARCV2
	if (slc_exists)
		__slc_line_op(start, end - start, OP_INV);
#endif
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	__dc_line_op(start, end - start, OP_FLUSH);
#ifdef CONFIG_ISA_ARCV2
	if (slc_exists)
		__slc_line_op(start, end - start, OP_FLUSH);
#endif
}

void flush_cache(unsigned long start, unsigned long size)
{
	flush_dcache_range(start, start + size);
}

void invalidate_dcache_all(void)
{
	__dc_entire_op(OP_INV);
#ifdef CONFIG_ISA_ARCV2
	if (slc_exists)
		__slc_entire_op(OP_INV);
#endif
}

void flush_dcache_all(void)
{
	__dc_entire_op(OP_FLUSH);
#ifdef CONFIG_ISA_ARCV2
	if (slc_exists)
		__slc_entire_op(OP_FLUSH);
#endif
}
