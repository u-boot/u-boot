#include <common.h>
#include <asm/arch/mmu.h>
#include <asm/sysreg.h>

void mmu_init_r(unsigned long dest_addr)
{
	uintptr_t	vmr_table_addr;

	/* Round monitor address down to the nearest page boundary */
	dest_addr &= PAGE_ADDR_MASK;

	/* Initialize TLB entry 0 to cover the monitor, and lock it */
	sysreg_write(TLBEHI, dest_addr | SYSREG_BIT(TLBEHI_V));
	sysreg_write(TLBELO, dest_addr | MMU_VMR_CACHE_WRBACK);
	sysreg_write(MMUCR, SYSREG_BF(DRP, 0) | SYSREG_BF(DLA, 1)
			| SYSREG_BIT(MMUCR_S) | SYSREG_BIT(M));
	__builtin_tlbw();

	/*
	 * Calculate the address of the VM range table in a PC-relative
	 * manner to make sure we hit the SDRAM and not the flash.
	 */
	vmr_table_addr = (uintptr_t)&mmu_vmr_table;
	sysreg_write(PTBR, vmr_table_addr);
	printf("VMR table @ 0x%08x\n", vmr_table_addr);

	/* Enable paging */
	sysreg_write(MMUCR, SYSREG_BF(DRP, 1) | SYSREG_BF(DLA, 1)
			| SYSREG_BIT(MMUCR_S) | SYSREG_BIT(M) | SYSREG_BIT(E));
}

int mmu_handle_tlb_miss(void)
{
	const struct mmu_vm_range *vmr_table;
	const struct mmu_vm_range *vmr;
	unsigned int fault_pgno;
	int first, last;

	fault_pgno = sysreg_read(TLBEAR) >> PAGE_SHIFT;
	vmr_table = (const struct mmu_vm_range *)sysreg_read(PTBR);

	/* Do a binary search through the VM ranges */
	first = 0;
	last = CONFIG_SYS_NR_VM_REGIONS;
	while (first < last) {
		unsigned int start;
		int middle;

		/* Pick the entry in the middle of the remaining range */
		middle = (first + last) >> 1;
		vmr = &vmr_table[middle];
		start = vmr->virt_pgno;

		/* Do the bisection thing */
		if (fault_pgno < start) {
			last = middle;
		} else if (fault_pgno >= (start + vmr->nr_pages)) {
			first = middle + 1;
		} else {
			/* Got it; let's slam it into the TLB */
			uint32_t tlbelo;

			tlbelo = vmr->phys & ~PAGE_ADDR_MASK;
			tlbelo |= fault_pgno << PAGE_SHIFT;
			sysreg_write(TLBELO, tlbelo);
			__builtin_tlbw();

			/* Zero means success */
			return 0;
		}
	}

	/*
	 * Didn't find any matching entries. Return a nonzero value to
	 * indicate that this should be treated as a fatal exception.
	 */
	return -1;
}
