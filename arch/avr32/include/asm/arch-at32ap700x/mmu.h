/*
 * In order to deal with the hardcoded u-boot requirement that virtual
 * addresses are always mapped 1:1 with physical addresses, we implement
 * a small virtual memory manager so that we can use the MMU hardware in
 * order to get the caching properties right.
 *
 * A few pages (or possibly just one) are locked in the TLB permanently
 * in order to avoid recursive TLB misses, but most pages are faulted in
 * on demand.
 */
#ifndef __ASM_ARCH_MMU_H
#define __ASM_ARCH_MMU_H

#include <asm/sysreg.h>

#define PAGE_SHIFT	20
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_ADDR_MASK	(~(PAGE_SIZE - 1))

#define MMU_VMR_CACHE_NONE						\
	(SYSREG_BF(AP, 3) | SYSREG_BF(SZ, 3) | SYSREG_BIT(TLBELO_D))
#define MMU_VMR_CACHE_WBUF						\
	(MMU_VMR_CACHE_NONE | SYSREG_BIT(B))
#define MMU_VMR_CACHE_WRTHRU						\
	(MMU_VMR_CACHE_NONE | SYSREG_BIT(TLBELO_C) | SYSREG_BIT(W))
#define MMU_VMR_CACHE_WRBACK						\
	(MMU_VMR_CACHE_WBUF | SYSREG_BIT(TLBELO_C))

/*
 * This structure is used in our "page table". Instead of the usual
 * x86-inspired radix tree, we let each entry cover an arbitrary-sized
 * virtual address range and store them in a binary search tree. This is
 * somewhat slower, but should use significantly less RAM, and we
 * shouldn't get many TLB misses when using 1 MB pages anyway.
 *
 * With 1 MB pages, we need 12 bits to store the page number. In
 * addition, we stick an Invalid bit in the high bit of virt_pgno (if
 * set, it cannot possibly match any faulting page), and all the bits
 * that need to be written to TLBELO in phys_pgno.
 */
struct mmu_vm_range {
	uint16_t	virt_pgno;
	uint16_t	nr_pages;
	uint32_t	phys;
};

/*
 * An array of mmu_vm_range objects describing all pageable addresses.
 * The array is sorted by virt_pgno so that the TLB miss exception
 * handler can do a binary search to find the correct entry.
 */
extern struct mmu_vm_range mmu_vmr_table[];

/*
 * Initialize the MMU. This will set up a fixed TLB entry for the static
 * u-boot image at dest_addr and enable paging.
 */
void mmu_init_r(unsigned long dest_addr);

/*
 * Handle a TLB miss exception. This function is called directly from
 * the exception vector table written in assembly.
 */
int mmu_handle_tlb_miss(void);

#endif /* __ASM_ARCH_MMU_H */
