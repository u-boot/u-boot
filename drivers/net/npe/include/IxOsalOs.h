#ifndef IxOsalOs_H
#define IxOsalOs_H

#ifndef IX_OSAL_CACHED
#error "Uncached memory not supported in linux environment"
#endif

static inline unsigned long __v2p(unsigned long v)
{
	if (v < 0x40000000)
		return (v & 0xfffffff);
	else
		return v;
}

#define IX_OSAL_OS_MMU_VIRT_TO_PHYS(addr)        __v2p((u32)addr)
#define IX_OSAL_OS_MMU_PHYS_TO_VIRT(addr)        (addr)

/*
 * Data cache not enabled (hopefully)
 */
#define IX_OSAL_OS_CACHE_INVALIDATE(addr, size)
#define IX_OSAL_OS_CACHE_FLUSH(addr, size)
#define HAL_DCACHE_INVALIDATE(addr, size)
#define HAL_DCACHE_FLUSH(addr, size)

#define __ixp42X			/* sr: U-Boot needs this define */

#endif /* IxOsalOs_H */

