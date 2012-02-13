/*
 * U-boot - blackfin_local.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __BLACKFIN_LOCAL_H__
#define __BLACKFIN_LOCAL_H__

#include <asm/mem_map.h>

#define LO(con32) ((con32) & 0xFFFF)
#define lo(con32) ((con32) & 0xFFFF)
#define HI(con32) (((con32) >> 16) & 0xFFFF)
#define hi(con32) (((con32) >> 16) & 0xFFFF)

#define OFFSET_(x) (x & 0x0000FFFF)
#define MK_BMSK_(x) (1 << x)

/* Ideally this should be USEC not MSEC, but the USEC multiplication
 * likes to overflow 32bit quantities which is all our assembler
 * currently supports ;(
 */
#define USEC_PER_MSEC 1000
#define MSEC_PER_SEC 1000
#define BFIN_SCLK (100000000)
#define SCLK_TO_MSEC(sclk) ((MSEC_PER_SEC * ((sclk) / USEC_PER_MSEC)) / (BFIN_SCLK / USEC_PER_MSEC))
#define MSEC_TO_SCLK(msec) ((((BFIN_SCLK / USEC_PER_MSEC) * (msec)) / MSEC_PER_SEC) * USEC_PER_MSEC)

#define L1_CACHE_SHIFT 5
#define L1_CACHE_BYTES (1 << L1_CACHE_SHIFT)

#include <linux/linkage.h>
#include <asm/cache.h>

#ifndef __ASSEMBLY__
# ifdef SHARED_RESOURCES
#  include <asm/shared_resources.h>
# endif

# include <linux/types.h>

extern u_long get_vco(void);
extern u_long get_cclk(void);
extern u_long get_sclk(void);

# define bfin_revid() (bfin_read_CHIPID() >> 28)

extern bool bfin_os_log_check(void);
extern void bfin_os_log_dump(void);

extern void blackfin_icache_flush_range(const void *, const void *);
extern void blackfin_dcache_flush_range(const void *, const void *);
extern void blackfin_icache_dcache_flush_range(const void *, const void *);
extern void blackfin_dcache_flush_invalidate_range(const void *, const void *);

/* Use DMA to move data from on chip to external memory.  The L1 instruction
 * regions can only be accessed via DMA, so if the address in question is in
 * that region, make sure we attempt to DMA indirectly.
 */
# ifdef __ADSPBF561__
  /* Core B regions all need dma from Core A */
#  define addr_bfin_on_chip_mem(addr) \
	((((unsigned long)(addr) & 0xFFF00000) == 0xFFA00000) || \
	 (((unsigned long)(addr) & 0xFFC00000) == 0xFF400000))
# else
#  define addr_bfin_on_chip_mem(addr) \
	(((unsigned long)(addr) & 0xFFF00000) == 0xFFA00000)
# endif

# include <asm/system.h>

#if ANOMALY_05000198
# define NOP_PAD_ANOMALY_05000198 "nop;"
#else
# define NOP_PAD_ANOMALY_05000198
#endif

#define _bfin_readX(addr, size, asm_size, asm_ext) ({ \
	u32 __v; \
	__asm__ __volatile__( \
		NOP_PAD_ANOMALY_05000198 \
		"%0 = " #asm_size "[%1]" #asm_ext ";" \
		: "=d" (__v) \
		: "a" (addr) \
	); \
	__v; })
#define _bfin_writeX(addr, val, size, asm_size) \
	__asm__ __volatile__( \
		NOP_PAD_ANOMALY_05000198 \
		#asm_size "[%0] = %1;" \
		: \
		: "a" (addr), "d" ((u##size)(val)) \
		: "memory" \
	)

#define bfin_read8(addr)  _bfin_readX(addr,  8, b, (z))
#define bfin_read16(addr) _bfin_readX(addr, 16, w, (z))
#define bfin_read32(addr) _bfin_readX(addr, 32,  ,    )
#define bfin_write8(addr, val)  _bfin_writeX(addr, val,  8, b)
#define bfin_write16(addr, val) _bfin_writeX(addr, val, 16, w)
#define bfin_write32(addr, val) _bfin_writeX(addr, val, 32,  )

#define bfin_read(addr) \
({ \
	sizeof(*(addr)) == 1 ? bfin_read8(addr)  : \
	sizeof(*(addr)) == 2 ? bfin_read16(addr) : \
	sizeof(*(addr)) == 4 ? bfin_read32(addr) : \
	({ BUG(); 0; }); \
})
#define bfin_write(addr, val) \
do { \
	switch (sizeof(*(addr))) { \
	case 1: bfin_write8(addr, val);  break; \
	case 2: bfin_write16(addr, val); break; \
	case 4: bfin_write32(addr, val); break; \
	default: BUG(); \
	} \
} while (0)

#define bfin_write_or(addr, bits) \
do { \
	typeof(addr) __addr = (addr); \
	bfin_write(__addr, bfin_read(__addr) | (bits)); \
} while (0)

#define bfin_write_and(addr, bits) \
do { \
	typeof(addr) __addr = (addr); \
	bfin_write(__addr, bfin_read(__addr) & (bits)); \
} while (0)

#define bfin_readPTR(addr) bfin_read32(addr)
#define bfin_writePTR(addr, val) bfin_write32(addr, val)

/* SSYNC implementation for C file */
static inline void SSYNC(void)
{
	int _tmp;
	if (ANOMALY_05000312)
		__asm__ __volatile__(
			"cli %0;"
			"nop;"
			"nop;"
			"ssync;"
			"sti %0;"
			: "=d" (_tmp)
		);
	else if (ANOMALY_05000244)
		__asm__ __volatile__(
			"nop;"
			"nop;"
			"nop;"
			"ssync;"
		);
	else
		__asm__ __volatile__("ssync;");
}

/* CSYNC implementation for C file */
static inline void CSYNC(void)
{
	int _tmp;
	if (ANOMALY_05000312)
		__asm__ __volatile__(
			"cli %0;"
			"nop;"
			"nop;"
			"csync;"
			"sti %0;"
			: "=d" (_tmp)
		);
	else if (ANOMALY_05000244)
		__asm__ __volatile__(
			"nop;"
			"nop;"
			"nop;"
			"csync;"
		);
	else
		__asm__ __volatile__("csync;");
}

#else  /* __ASSEMBLY__ */

/* SSYNC & CSYNC implementations for assembly files */

#define ssync(x) SSYNC(x)
#define csync(x) CSYNC(x)

#if ANOMALY_05000312
#define SSYNC(scratch) cli scratch; nop; nop; SSYNC; sti scratch;
#define CSYNC(scratch) cli scratch; nop; nop; CSYNC; sti scratch;

#elif ANOMALY_05000244
#define SSYNC(scratch) nop; nop; nop; SSYNC;
#define CSYNC(scratch) nop; nop; nop; CSYNC;

#else
#define SSYNC(scratch) SSYNC;
#define CSYNC(scratch) CSYNC;

#endif /* ANOMALY_05000312 & ANOMALY_05000244 handling */

#endif /* __ASSEMBLY__ */

#endif
