/*
 * include/asm-ppc/cache.h
 */
#ifndef __ARCH_PPC_CACHE_H
#define __ARCH_PPC_CACHE_H

#include <asm/processor.h>

/* bytes per L1 cache line */
#if defined(CONFIG_PPC64BRIDGE)
#define L1_CACHE_SHIFT	7
#elif defined(CONFIG_E500MC)
#define L1_CACHE_SHIFT	6
#else
#define	L1_CACHE_SHIFT	5
#endif

#define L1_CACHE_BYTES          (1 << L1_CACHE_SHIFT)

/*
 * Use the L1 data cache line size value for the minimum DMA buffer alignment
 * on PowerPC.
 */
#define ARCH_DMA_MINALIGN	L1_CACHE_BYTES

/*
 * For compatibility reasons support the CONFIG_SYS_CACHELINE_SIZE too
 */
#ifndef CONFIG_SYS_CACHELINE_SIZE
#define CONFIG_SYS_CACHELINE_SIZE	L1_CACHE_BYTES
#endif

#define	L1_CACHE_ALIGN(x)       (((x)+(L1_CACHE_BYTES-1))&~(L1_CACHE_BYTES-1))
#define	L1_CACHE_PAGES		8

#define	SMP_CACHE_BYTES L1_CACHE_BYTES

#ifdef MODULE
#define __cacheline_aligned __attribute__((__aligned__(L1_CACHE_BYTES)))
#else
#define __cacheline_aligned					\
  __attribute__((__aligned__(L1_CACHE_BYTES),			\
		 __section__(".data.cacheline_aligned")))
#endif

#if defined(__KERNEL__) && !defined(__ASSEMBLY__)
extern void flush_dcache_range(unsigned long start, unsigned long stop);
extern void clean_dcache_range(unsigned long start, unsigned long stop);
extern void invalidate_dcache_range(unsigned long start, unsigned long stop);
extern void flush_dcache(void);
extern void invalidate_dcache(void);
extern void invalidate_icache(void);
#ifdef CONFIG_SYS_INIT_RAM_LOCK
extern void unlock_ram_in_cache(void);
#endif /* CONFIG_SYS_INIT_RAM_LOCK */
#endif /* __ASSEMBLY__ */

#if defined(__KERNEL__) && !defined(__ASSEMBLY__)
int l2cache_init(void);
void enable_cpc(void);
void disable_cpc_sram(void);
#endif

/* prep registers for L2 */
#define CACHECRBA       0x80000823      /* Cache configuration register address */
#define L2CACHE_MASK	0x03	/* Mask for 2 L2 Cache bits */
#define L2CACHE_512KB	0x00	/* 512KB */
#define L2CACHE_256KB	0x01	/* 256KB */
#define L2CACHE_1MB	0x02	/* 1MB */
#define L2CACHE_NONE	0x03	/* NONE */
#define L2CACHE_PARITY  0x08    /* Mask for L2 Cache Parity Protected bit */

#endif
