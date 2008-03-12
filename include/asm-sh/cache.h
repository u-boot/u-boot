#ifndef __ASM_SH_CACHE_H
#define __ASM_SH_CACHE_H

#if defined(CONFIG_SH4) || defined(CONFIG_SH4A)

#define L1_CACHE_BYTES 32
struct __large_struct { unsigned long buf[100]; };
#define __m(x) (*(struct __large_struct *)(x))

void dcache_wback_range(u32 start, u32 end)
{
    u32 v;

    start &= ~(L1_CACHE_BYTES-1);
    for (v = start; v < end; v+=L1_CACHE_BYTES) {
        asm volatile("ocbwb     %0"
                     : /* no output */
                     : "m" (__m(v)));
    }
}

void dcache_invalid_range(u32 start, u32 end)
{
    u32 v;

    start &= ~(L1_CACHE_BYTES-1);
    for (v = start; v < end; v+=L1_CACHE_BYTES) {
        asm volatile("ocbi     %0"
                     : /* no output */
                     : "m" (__m(v)));
    }
}
#endif /* CONFIG_SH4 || CONFIG_SH4A */

#endif	/* __ASM_SH_CACHE_H */
