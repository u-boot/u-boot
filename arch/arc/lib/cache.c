/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/log2.h>
#include <asm/arcregs.h>
#include <asm/cache.h>

/* Bit values in IC_CTRL */
#define IC_CTRL_CACHE_DISABLE	BIT(0)

/* Bit values in DC_CTRL */
#define DC_CTRL_CACHE_DISABLE	BIT(0)
#define DC_CTRL_INV_MODE_FLUSH	BIT(6)
#define DC_CTRL_FLUSH_STATUS	BIT(8)
#define CACHE_VER_NUM_MASK	0xF

#define OP_INV		0x1
#define OP_FLUSH	0x2
#define OP_INV_IC	0x3

/* Bit val in SLC_CONTROL */
#define SLC_CTRL_DIS		0x001
#define SLC_CTRL_IM		0x040
#define SLC_CTRL_BUSY		0x100
#define SLC_CTRL_RGN_OP_INV	0x200

/*
 * By default that variable will fall into .bss section.
 * But .bss section is not relocated and so it will be initilized before
 * relocation but will be used after being zeroed.
 */
int l1_line_sz __section(".data");
bool dcache_exists __section(".data") = false;
bool icache_exists __section(".data") = false;

#define CACHE_LINE_MASK		(~(l1_line_sz - 1))

#ifdef CONFIG_ISA_ARCV2
int slc_line_sz __section(".data");
bool slc_exists __section(".data") = false;
bool ioc_exists __section(".data") = false;
bool pae_exists __section(".data") = false;

/* To force enable IOC set ioc_enable to 'true' */
bool ioc_enable __section(".data") = false;

void read_decode_mmu_bcr(void)
{
	/* TODO: should we compare mmu version from BCR and from CONFIG? */
#if (CONFIG_ARC_MMU_VER >= 4)
	u32 tmp;

	tmp = read_aux_reg(ARC_AUX_MMU_BCR);

	struct bcr_mmu_4 {
#ifdef CONFIG_CPU_BIG_ENDIAN
	unsigned int ver:8, sasid:1, sz1:4, sz0:4, res:2, pae:1,
		     n_ways:2, n_entry:2, n_super:2, u_itlb:3, u_dtlb:3;
#else
	/*           DTLB      ITLB      JES        JE         JA      */
	unsigned int u_dtlb:3, u_itlb:3, n_super:2, n_entry:2, n_ways:2,
		     pae:1, res:2, sz0:4, sz1:4, sasid:1, ver:8;
#endif /* CONFIG_CPU_BIG_ENDIAN */
	} *mmu4;

	mmu4 = (struct bcr_mmu_4 *)&tmp;

	pae_exists = !!mmu4->pae;
#endif /* (CONFIG_ARC_MMU_VER >= 4) */
}

static void __slc_entire_op(const int op)
{
	unsigned int ctrl;

	ctrl = read_aux_reg(ARC_AUX_SLC_CTRL);

	if (!(op & OP_FLUSH))		/* i.e. OP_INV */
		ctrl &= ~SLC_CTRL_IM;	/* clear IM: Disable flush before Inv */
	else
		ctrl |= SLC_CTRL_IM;

	write_aux_reg(ARC_AUX_SLC_CTRL, ctrl);

	if (op & OP_INV)	/* Inv or flush-n-inv use same cmd reg */
		write_aux_reg(ARC_AUX_SLC_INVALIDATE, 0x1);
	else
		write_aux_reg(ARC_AUX_SLC_FLUSH, 0x1);

	/* Make sure "busy" bit reports correct stataus, see STAR 9001165532 */
	read_aux_reg(ARC_AUX_SLC_CTRL);

	/* Important to wait for flush to complete */
	while (read_aux_reg(ARC_AUX_SLC_CTRL) & SLC_CTRL_BUSY);
}

static void slc_upper_region_init(void)
{
	/*
	 * ARC_AUX_SLC_RGN_END1 and ARC_AUX_SLC_RGN_START1 are always == 0
	 * as we don't use PAE40.
	 */
	write_aux_reg(ARC_AUX_SLC_RGN_END1, 0);
	write_aux_reg(ARC_AUX_SLC_RGN_START1, 0);
}

static void __slc_rgn_op(unsigned long paddr, unsigned long sz, const int op)
{
	unsigned int ctrl;
	unsigned long end;

	/*
	 * The Region Flush operation is specified by CTRL.RGN_OP[11..9]
	 *  - b'000 (default) is Flush,
	 *  - b'001 is Invalidate if CTRL.IM == 0
	 *  - b'001 is Flush-n-Invalidate if CTRL.IM == 1
	 */
	ctrl = read_aux_reg(ARC_AUX_SLC_CTRL);

	/* Don't rely on default value of IM bit */
	if (!(op & OP_FLUSH))		/* i.e. OP_INV */
		ctrl &= ~SLC_CTRL_IM;	/* clear IM: Disable flush before Inv */
	else
		ctrl |= SLC_CTRL_IM;

	if (op & OP_INV)
		ctrl |= SLC_CTRL_RGN_OP_INV;	/* Inv or flush-n-inv */
	else
		ctrl &= ~SLC_CTRL_RGN_OP_INV;

	write_aux_reg(ARC_AUX_SLC_CTRL, ctrl);

	/*
	 * Lower bits are ignored, no need to clip
	 * END needs to be setup before START (latter triggers the operation)
	 * END can't be same as START, so add (l2_line_sz - 1) to sz
	 */
	end = paddr + sz + slc_line_sz - 1;

	/*
	 * Upper addresses (ARC_AUX_SLC_RGN_END1 and ARC_AUX_SLC_RGN_START1)
	 * are always == 0 as we don't use PAE40, so we only setup lower ones
	 * (ARC_AUX_SLC_RGN_END and ARC_AUX_SLC_RGN_START)
	 */
	write_aux_reg(ARC_AUX_SLC_RGN_END, end);
	write_aux_reg(ARC_AUX_SLC_RGN_START, paddr);

	/* Make sure "busy" bit reports correct stataus, see STAR 9001165532 */
	read_aux_reg(ARC_AUX_SLC_CTRL);

	while (read_aux_reg(ARC_AUX_SLC_CTRL) & SLC_CTRL_BUSY);
}
#endif /* CONFIG_ISA_ARCV2 */

#ifdef CONFIG_ISA_ARCV2
static void read_decode_cache_bcr_arcv2(void)
{
	union {
		struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
			unsigned int pad:24, way:2, lsz:2, sz:4;
#else
			unsigned int sz:4, lsz:2, way:2, pad:24;
#endif
		} fields;
		unsigned int word;
	} slc_cfg;

	union {
		struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
			unsigned int pad:24, ver:8;
#else
			unsigned int ver:8, pad:24;
#endif
		} fields;
		unsigned int word;
	} sbcr;

	sbcr.word = read_aux_reg(ARC_BCR_SLC);
	if (sbcr.fields.ver) {
		slc_cfg.word = read_aux_reg(ARC_AUX_SLC_CONFIG);
		slc_exists = true;
		slc_line_sz = (slc_cfg.fields.lsz == 0) ? 128 : 64;
	}

	union {
		struct bcr_clust_cfg {
#ifdef CONFIG_CPU_BIG_ENDIAN
			unsigned int pad:7, c:1, num_entries:8, num_cores:8, ver:8;
#else
			unsigned int ver:8, num_cores:8, num_entries:8, c:1, pad:7;
#endif
		} fields;
		unsigned int word;
	} cbcr;

	cbcr.word = read_aux_reg(ARC_BCR_CLUSTER);
	if (cbcr.fields.c && ioc_enable)
		ioc_exists = true;
}
#endif

void read_decode_cache_bcr(void)
{
	int dc_line_sz = 0, ic_line_sz = 0;

	union {
		struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
			unsigned int pad:12, line_len:4, sz:4, config:4, ver:8;
#else
			unsigned int ver:8, config:4, sz:4, line_len:4, pad:12;
#endif
		} fields;
		unsigned int word;
	} ibcr, dbcr;

	ibcr.word = read_aux_reg(ARC_BCR_IC_BUILD);
	if (ibcr.fields.ver) {
		icache_exists = true;
		l1_line_sz = ic_line_sz = 8 << ibcr.fields.line_len;
		if (!ic_line_sz)
			panic("Instruction exists but line length is 0\n");
	}

	dbcr.word = read_aux_reg(ARC_BCR_DC_BUILD);
	if (dbcr.fields.ver) {
		dcache_exists = true;
		l1_line_sz = dc_line_sz = 16 << dbcr.fields.line_len;
		if (!dc_line_sz)
			panic("Data cache exists but line length is 0\n");
	}

	if (ic_line_sz && dc_line_sz && (ic_line_sz != dc_line_sz))
		panic("Instruction and data cache line lengths differ\n");
}

void cache_init(void)
{
	read_decode_cache_bcr();

#ifdef CONFIG_ISA_ARCV2
	read_decode_cache_bcr_arcv2();

	if (ioc_exists) {
		/* IOC Aperture start is equal to DDR start */
		unsigned int ap_base = CONFIG_SYS_SDRAM_BASE;
		/* IOC Aperture size is equal to DDR size */
		long ap_size = CONFIG_SYS_SDRAM_SIZE;

		flush_dcache_all();
		invalidate_dcache_all();

		if (!is_power_of_2(ap_size) || ap_size < 4096)
			panic("IOC Aperture size must be power of 2 and bigger 4Kib");

		/*
		 * IOC Aperture size decoded as 2 ^ (SIZE + 2) KB,
		 * so setting 0x11 implies 512M, 0x12 implies 1G...
		 */
		write_aux_reg(ARC_AUX_IO_COH_AP0_SIZE,
			      order_base_2(ap_size / 1024) - 2);

		/* IOC Aperture start must be aligned to the size of the aperture */
		if (ap_base % ap_size != 0)
			panic("IOC Aperture start must be aligned to the size of the aperture");

		write_aux_reg(ARC_AUX_IO_COH_AP0_BASE, ap_base >> 12);
		write_aux_reg(ARC_AUX_IO_COH_PARTIAL, 1);
		write_aux_reg(ARC_AUX_IO_COH_ENABLE, 1);
	}

	read_decode_mmu_bcr();

	/*
	 * ARC_AUX_SLC_RGN_START1 and ARC_AUX_SLC_RGN_END1 register exist
	 * only if PAE exists in current HW. So we had to check pae_exist
	 * before using them.
	 */
	if (slc_exists && pae_exists)
		slc_upper_region_init();
#endif /* CONFIG_ISA_ARCV2 */
}

int icache_status(void)
{
	if (!icache_exists)
		return 0;

	if (read_aux_reg(ARC_AUX_IC_CTRL) & IC_CTRL_CACHE_DISABLE)
		return 0;
	else
		return 1;
}

void icache_enable(void)
{
	if (icache_exists)
		write_aux_reg(ARC_AUX_IC_CTRL, read_aux_reg(ARC_AUX_IC_CTRL) &
			      ~IC_CTRL_CACHE_DISABLE);
}

void icache_disable(void)
{
	if (icache_exists)
		write_aux_reg(ARC_AUX_IC_CTRL, read_aux_reg(ARC_AUX_IC_CTRL) |
			      IC_CTRL_CACHE_DISABLE);
}

void invalidate_icache_all(void)
{
	/* Any write to IC_IVIC register triggers invalidation of entire I$ */
	if (icache_status()) {
		write_aux_reg(ARC_AUX_IC_IVIC, 1);
		/*
		 * As per ARC HS databook (see chapter 5.3.3.2)
		 * it is required to add 3 NOPs after each write to IC_IVIC.
		 */
		__builtin_arc_nop();
		__builtin_arc_nop();
		__builtin_arc_nop();
		read_aux_reg(ARC_AUX_IC_CTRL);	/* blocks */
	}

#ifdef CONFIG_ISA_ARCV2
	if (slc_exists)
		__slc_entire_op(OP_INV);
#endif
}

int dcache_status(void)
{
	if (!dcache_exists)
		return 0;

	if (read_aux_reg(ARC_AUX_DC_CTRL) & DC_CTRL_CACHE_DISABLE)
		return 0;
	else
		return 1;
}

void dcache_enable(void)
{
	if (!dcache_exists)
		return;

	write_aux_reg(ARC_AUX_DC_CTRL, read_aux_reg(ARC_AUX_DC_CTRL) &
		      ~(DC_CTRL_INV_MODE_FLUSH | DC_CTRL_CACHE_DISABLE));
}

void dcache_disable(void)
{
	if (!dcache_exists)
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

	num_lines = DIV_ROUND_UP(sz, l1_line_sz);

	while (num_lines-- > 0) {
#if (CONFIG_ARC_MMU_VER == 3)
		write_aux_reg(aux_tag, paddr);
#endif
		write_aux_reg(aux_cmd, paddr);
		paddr += l1_line_sz;
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
		while (read_aux_reg(ARC_AUX_DC_CTRL) & DC_CTRL_FLUSH_STATUS);

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
	if (start >= end)
		return;

#ifdef CONFIG_ISA_ARCV2
	if (!ioc_exists)
#endif
		__dc_line_op(start, end - start, OP_INV);

#ifdef CONFIG_ISA_ARCV2
	if (slc_exists && !ioc_exists)
		__slc_rgn_op(start, end - start, OP_INV);
#endif
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	if (start >= end)
		return;

#ifdef CONFIG_ISA_ARCV2
	if (!ioc_exists)
#endif
		__dc_line_op(start, end - start, OP_FLUSH);

#ifdef CONFIG_ISA_ARCV2
	if (slc_exists && !ioc_exists)
		__slc_rgn_op(start, end - start, OP_FLUSH);
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
