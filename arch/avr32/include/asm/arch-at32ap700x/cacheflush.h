/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_CACHEFLUSH_H
#define __ASM_AVR32_CACHEFLUSH_H

/*
 * Invalidate any cacheline containing virtual address vaddr without
 * writing anything back to memory.
 *
 * Note that this function may corrupt unrelated data structures when
 * applied on buffers that are not cacheline aligned in both ends.
 */
static inline void dcache_invalidate_line(volatile void *vaddr)
{
	asm volatile("cache %0[0], 0x0b" : : "r"(vaddr) : "memory");
}

/*
 * Make sure any cacheline containing virtual address vaddr is written
 * to memory.
 */
static inline void dcache_clean_line(volatile void *vaddr)
{
	asm volatile("cache %0[0], 0x0c" : : "r"(vaddr) : "memory");
}

/*
 * Make sure any cacheline containing virtual address vaddr is written
 * to memory and then invalidate it.
 */
static inline void dcache_flush_line(volatile void *vaddr)
{
	asm volatile("cache %0[0], 0x0d" : : "r"(vaddr) : "memory");
}

/*
 * Invalidate any instruction cacheline containing virtual address
 * vaddr.
 */
static inline void icache_invalidate_line(volatile void *vaddr)
{
	asm volatile("cache %0[0], 0x01" : : "r"(vaddr) : "memory");
}

/*
 * Applies the above functions on all lines that are touched by the
 * specified virtual address range.
 */
void dcache_invalidate_range(volatile void *start, size_t len);
void dcache_clean_range(volatile void *start, size_t len);
void dcache_flush_range(volatile void *start, size_t len);
void icache_invalidate_range(volatile void *start, size_t len);

static inline void dcache_flush_unlocked(void)
{
	asm volatile("cache %0[5], 0x08" : : "r"(0) : "memory");
}

/*
 * Make sure any pending writes are completed before continuing.
 */
#define sync_write_buffer() asm volatile("sync 0" : : : "memory")

#endif /* __ASM_AVR32_CACHEFLUSH_H */
