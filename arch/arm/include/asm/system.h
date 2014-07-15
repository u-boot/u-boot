#ifndef __ASM_ARM_SYSTEM_H
#define __ASM_ARM_SYSTEM_H

#ifdef CONFIG_ARM64

/*
 * SCTLR_EL1/SCTLR_EL2/SCTLR_EL3 bits definitions
 */
#define CR_M		(1 << 0)	/* MMU enable			*/
#define CR_A		(1 << 1)	/* Alignment abort enable	*/
#define CR_C		(1 << 2)	/* Dcache enable		*/
#define CR_SA		(1 << 3)	/* Stack Alignment Check Enable	*/
#define CR_I		(1 << 12)	/* Icache enable		*/
#define CR_WXN		(1 << 19)	/* Write Permision Imply XN	*/
#define CR_EE		(1 << 25)	/* Exception (Big) Endian	*/

#define PGTABLE_SIZE	(0x10000)

#ifndef __ASSEMBLY__

#define isb()				\
	({asm volatile(			\
	"isb" : : : "memory");		\
	})

#define wfi()				\
	({asm volatile(			\
	"wfi" : : : "memory");		\
	})

static inline unsigned int current_el(void)
{
	unsigned int el;
	asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
	return el >> 2;
}

static inline unsigned int get_sctlr(void)
{
	unsigned int el, val;

	el = current_el();
	if (el == 1)
		asm volatile("mrs %0, sctlr_el1" : "=r" (val) : : "cc");
	else if (el == 2)
		asm volatile("mrs %0, sctlr_el2" : "=r" (val) : : "cc");
	else
		asm volatile("mrs %0, sctlr_el3" : "=r" (val) : : "cc");

	return val;
}

static inline void set_sctlr(unsigned int val)
{
	unsigned int el;

	el = current_el();
	if (el == 1)
		asm volatile("msr sctlr_el1, %0" : : "r" (val) : "cc");
	else if (el == 2)
		asm volatile("msr sctlr_el2, %0" : : "r" (val) : "cc");
	else
		asm volatile("msr sctlr_el3, %0" : : "r" (val) : "cc");

	asm volatile("isb");
}

void __asm_flush_dcache_all(void);
void __asm_invalidate_dcache_all(void);
void __asm_flush_dcache_range(u64 start, u64 end);
void __asm_invalidate_tlb_all(void);
void __asm_invalidate_icache_all(void);

void armv8_switch_to_el2(void);
void armv8_switch_to_el1(void);
void gic_init(void);
void gic_send_sgi(unsigned long sgino);
void wait_for_wakeup(void);
void smp_kick_all_cpus(void);

#endif	/* __ASSEMBLY__ */

#else /* CONFIG_ARM64 */

#ifdef __KERNEL__

#define CPU_ARCH_UNKNOWN	0
#define CPU_ARCH_ARMv3		1
#define CPU_ARCH_ARMv4		2
#define CPU_ARCH_ARMv4T		3
#define CPU_ARCH_ARMv5		4
#define CPU_ARCH_ARMv5T		5
#define CPU_ARCH_ARMv5TE	6
#define CPU_ARCH_ARMv5TEJ	7
#define CPU_ARCH_ARMv6		8
#define CPU_ARCH_ARMv7		9

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M	(1 << 0)	/* MMU enable				*/
#define CR_A	(1 << 1)	/* Alignment abort enable		*/
#define CR_C	(1 << 2)	/* Dcache enable			*/
#define CR_W	(1 << 3)	/* Write buffer enable			*/
#define CR_P	(1 << 4)	/* 32-bit exception handler		*/
#define CR_D	(1 << 5)	/* 32-bit data address range		*/
#define CR_L	(1 << 6)	/* Implementation defined		*/
#define CR_B	(1 << 7)	/* Big endian				*/
#define CR_S	(1 << 8)	/* System MMU protection		*/
#define CR_R	(1 << 9)	/* ROM MMU protection			*/
#define CR_F	(1 << 10)	/* Implementation defined		*/
#define CR_Z	(1 << 11)	/* Implementation defined		*/
#define CR_I	(1 << 12)	/* Icache enable			*/
#define CR_V	(1 << 13)	/* Vectors relocated to 0xffff0000	*/
#define CR_RR	(1 << 14)	/* Round Robin cache replacement	*/
#define CR_L4	(1 << 15)	/* LDR pc can set T bit			*/
#define CR_DT	(1 << 16)
#define CR_IT	(1 << 18)
#define CR_ST	(1 << 19)
#define CR_FI	(1 << 21)	/* Fast interrupt (lower latency mode)	*/
#define CR_U	(1 << 22)	/* Unaligned access operation		*/
#define CR_XP	(1 << 23)	/* Extended page tables			*/
#define CR_VE	(1 << 24)	/* Vectored interrupts			*/
#define CR_EE	(1 << 25)	/* Exception (Big) Endian		*/
#define CR_TRE	(1 << 28)	/* TEX remap enable			*/
#define CR_AFE	(1 << 29)	/* Access flag enable			*/
#define CR_TE	(1 << 30)	/* Thumb exception enable		*/

#define PGTABLE_SIZE		(4096 * 4)

/*
 * This is used to ensure the compiler did actually allocate the register we
 * asked it for some inline assembly sequences.  Apparently we can't trust
 * the compiler from one version to another so a bit of paranoia won't hurt.
 * This string is meant to be concatenated with the inline asm string and
 * will cause compilation to stop on mismatch.
 * (for details, see gcc PR 15089)
 */
#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"

#ifndef __ASSEMBLY__

#define isb() __asm__ __volatile__ ("" : : : "memory")

#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");

#ifdef __ARM_ARCH_7A__
#define wfi() __asm__ __volatile__ ("wfi" : : : "memory")
#else
#define wfi()
#endif

static inline unsigned int get_cr(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val) : : "cc");
	return val;
}

static inline void set_cr(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR"
	  : : "r" (val) : "cc");
	isb();
}

static inline unsigned int get_dacr(void)
{
	unsigned int val;
	asm("mrc p15, 0, %0, c3, c0, 0	@ get DACR" : "=r" (val) : : "cc");
	return val;
}

static inline void set_dacr(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c3, c0, 0	@ set DACR"
	  : : "r" (val) : "cc");
	isb();
}

/* options available for data cache on each page */
enum dcache_option {
	DCACHE_OFF = 0x12,
	DCACHE_WRITETHROUGH = 0x1a,
	DCACHE_WRITEBACK = 0x1e,
};

/* Size of an MMU section */
enum {
	MMU_SECTION_SHIFT	= 20,
	MMU_SECTION_SIZE	= 1 << MMU_SECTION_SHIFT,
};

/**
 * Change the cache settings for a region.
 *
 * \param start		start address of memory region to change
 * \param size		size of memory region to change
 * \param option	dcache option to select
 */
void mmu_set_region_dcache_behaviour(u32 start, int size,
				     enum dcache_option option);

/**
 * Register an update to the page tables, and flush the TLB
 *
 * \param start		start address of update in page table
 * \param stop		stop address of update in page table
 */
void mmu_page_table_flush(unsigned long start, unsigned long stop);

#endif /* __ASSEMBLY__ */

#define arch_align_stack(x) (x)

#endif /* __KERNEL__ */

#endif /* CONFIG_ARM64 */

#endif
