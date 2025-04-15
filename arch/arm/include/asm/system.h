#ifndef __ASM_ARM_SYSTEM_H
#define __ASM_ARM_SYSTEM_H

#include <linux/compiler.h>
#include <asm/barriers.h>

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

#define ES_TO_AARCH64		1
#define ES_TO_AARCH32		0

/*
 * SCR_EL3 bits definitions
 */
#define SCR_EL3_RW_AARCH64	(1 << 10) /* Next lower level is AArch64     */
#define SCR_EL3_RW_AARCH32	(0 << 10) /* Lower lowers level are AArch32  */
#define SCR_EL3_HCE_EN		(1 << 8)  /* Hypervisor Call enable          */
#define SCR_EL3_SMD_DIS		(1 << 7)  /* Secure Monitor Call disable     */
#define SCR_EL3_RES1		(3 << 4)  /* Reserved, RES1                  */
#define SCR_EL3_EA_EN		(1 << 3)  /* External aborts taken to EL3    */
#define SCR_EL3_NS_EN		(1 << 0)  /* EL0 and EL1 in Non-scure state  */

/*
 * SPSR_EL3/SPSR_EL2 bits definitions
 */
#define SPSR_EL_END_LE		(0 << 9)  /* Exception Little-endian          */
#define SPSR_EL_DEBUG_MASK	(1 << 9)  /* Debug exception masked           */
#define SPSR_EL_ASYN_MASK	(1 << 8)  /* Asynchronous data abort masked   */
#define SPSR_EL_SERR_MASK	(1 << 8)  /* System Error exception masked    */
#define SPSR_EL_IRQ_MASK	(1 << 7)  /* IRQ exception masked             */
#define SPSR_EL_FIQ_MASK	(1 << 6)  /* FIQ exception masked             */
#define SPSR_EL_T_A32		(0 << 5)  /* AArch32 instruction set A32      */
#define SPSR_EL_M_AARCH64	(0 << 4)  /* Exception taken from AArch64     */
#define SPSR_EL_M_AARCH32	(1 << 4)  /* Exception taken from AArch32     */
#define SPSR_EL_M_SVC		(0x3)     /* Exception taken from SVC mode    */
#define SPSR_EL_M_HYP		(0xa)     /* Exception taken from HYP mode    */
#define SPSR_EL_M_EL1H		(5)       /* Exception taken from EL1h mode   */
#define SPSR_EL_M_EL2H		(9)       /* Exception taken from EL2h mode   */

/*
 * CPTR_EL2 bits definitions
 */
#define CPTR_EL2_RES1		(3 << 12 | 0x3ff)           /* Reserved, RES1 */

/*
 * SCTLR_EL2 bits definitions
 */
#define SCTLR_EL2_RES1		(3 << 28 | 3 << 22 | 1 << 18 | 1 << 16 |\
				 1 << 11 | 3 << 4)	    /* Reserved, RES1 */
#define SCTLR_EL2_EE_LE		(0 << 25) /* Exception Little-endian          */
#define SCTLR_EL2_WXN_DIS	(0 << 19) /* Write permission is not XN       */
#define SCTLR_EL2_ICACHE_DIS	(0 << 12) /* Instruction cache disabled       */
#define SCTLR_EL2_SA_DIS	(0 << 3)  /* Stack Alignment Check disabled   */
#define SCTLR_EL2_DCACHE_DIS	(0 << 2)  /* Data cache disabled              */
#define SCTLR_EL2_ALIGN_DIS	(0 << 1)  /* Alignment check disabled         */
#define SCTLR_EL2_MMU_DIS	(0)       /* MMU disabled                     */

/*
 * CNTHCTL_EL2 bits definitions
 */
#define CNTHCTL_EL2_EVNT_EN	BIT(2)	     /* Enable the event stream       */
#define CNTHCTL_EL2_EVNT_I(val)	((val) << 4) /* Event stream trigger bits     */
#define CNTHCTL_EL2_EL1PCEN_EN	(1 << 1)     /* Physical timer regs accessible */
#define CNTHCTL_EL2_EL1PCTEN_EN	(1 << 0)     /* Physical counter accessible   */

/*
 * HCR_EL2 bits definitions
 */
#define HCR_EL2_API		(1 << 41) /* Trap pointer authentication
				             instructions                     */
#define HCR_EL2_APK		(1 << 40) /* Trap pointer authentication
				             key access                       */
#define HCR_EL2_RW_AARCH64	(1 << 31) /* EL1 is AArch64                   */
#define HCR_EL2_RW_AARCH32	(0 << 31) /* Lower levels are AArch32         */
#define HCR_EL2_HCD_DIS		(1 << 29) /* Hypervisor Call disabled         */
#define HCR_EL2_AMO_EL2		(1 <<  5) /* Route SErrors to EL2             */

#define ID_AA64ISAR0_EL1_RNDR	(0xFUL << 60) /* RNDR random registers */
/*
 * ID_AA64ISAR1_EL1 bits definitions
 */
#define ID_AA64ISAR1_EL1_GPI	(0xF << 28) /* Implementation-defined generic
				               code auth algorithm            */
#define ID_AA64ISAR1_EL1_GPA	(0xF << 24) /* QARMA generic code auth
				               algorithm                      */
#define ID_AA64ISAR1_EL1_API	(0xF << 8)  /* Implementation-defined address
				               auth algorithm                 */
#define ID_AA64ISAR1_EL1_APA	(0xF << 4)  /* QARMA address auth algorithm   */

/*
 * ID_AA64PFR0_EL1 bits definitions
 */
#define ID_AA64PFR0_EL1_EL3	(0xF << 12) /* EL3 implemented                */
#define ID_AA64PFR0_EL1_EL2	(0xF << 8)  /* EL2 implemented                */

/*
 * CPACR_EL1 bits definitions
 */
#define CPACR_EL1_FPEN_EN	(3 << 20) /* SIMD and FP instruction enabled  */

/*
 * SCTLR_EL1 bits definitions
 */
#define SCTLR_EL1_RES1		(3 << 28 | 3 << 22 | 1 << 20 |\
				 1 << 11) /* Reserved, RES1                   */
#define SCTLR_EL1_UCI_DIS	(0 << 26) /* Cache instruction disabled       */
#define SCTLR_EL1_EE_LE		(0 << 25) /* Exception Little-endian          */
#define SCTLR_EL1_WXN_DIS	(0 << 19) /* Write permission is not XN       */
#define SCTLR_EL1_NTWE_DIS	(0 << 18) /* WFE instruction disabled         */
#define SCTLR_EL1_NTWI_DIS	(0 << 16) /* WFI instruction disabled         */
#define SCTLR_EL1_UCT_DIS	(0 << 15) /* CTR_EL0 access disabled          */
#define SCTLR_EL1_DZE_DIS	(0 << 14) /* DC ZVA instruction disabled      */
#define SCTLR_EL1_ICACHE_DIS	(0 << 12) /* Instruction cache disabled       */
#define SCTLR_EL1_UMA_DIS	(0 << 9)  /* User Mask Access disabled        */
#define SCTLR_EL1_SED_EN	(0 << 8)  /* SETEND instruction enabled       */
#define SCTLR_EL1_ITD_EN	(0 << 7)  /* IT instruction enabled           */
#define SCTLR_EL1_CP15BEN_DIS	(0 << 5)  /* CP15 barrier operation disabled  */
#define SCTLR_EL1_SA0_DIS	(0 << 4)  /* Stack Alignment EL0 disabled     */
#define SCTLR_EL1_SA_DIS	(0 << 3)  /* Stack Alignment EL1 disabled     */
#define SCTLR_EL1_DCACHE_DIS	(0 << 2)  /* Data cache disabled              */
#define SCTLR_EL1_ALIGN_DIS	(0 << 1)  /* Alignment check disabled         */
#define SCTLR_EL1_MMU_DIS	(0)       /* MMU disabled                     */

#ifndef __ASSEMBLY__

struct pt_regs;

u64 get_page_table_size(void);
#define PGTABLE_SIZE	get_page_table_size()

/* 2MB granularity */
#define MMU_SECTION_SHIFT	21
#define MMU_SECTION_SIZE	(1 << MMU_SECTION_SHIFT)

/* These constants need to be synced to the MT_ types in asm/armv8/mmu.h */
enum dcache_option {
	DCACHE_OFF = 0 << 2,
	DCACHE_WRITETHROUGH = 3 << 2,
	DCACHE_WRITEBACK = 4 << 2,
	DCACHE_WRITEALLOC = 4 << 2,
};

#define wfi()				\
	({asm volatile(			\
	"wfi" : : : "memory");		\
	})

#define wfe()				\
	({asm volatile(			\
	"wfe" : : : "memory");		\
	})

#define sev() asm volatile("sev")

static inline unsigned int current_el(void)
{
	unsigned long el;

	asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
	return 3 & (el >> 2);
}

static inline unsigned long get_sctlr(void)
{
	unsigned int el;
	unsigned long val;

	el = current_el();
	if (el == 1)
		asm volatile("mrs %0, sctlr_el1" : "=r" (val) : : "cc");
	else if (el == 2)
		asm volatile("mrs %0, sctlr_el2" : "=r" (val) : : "cc");
	else
		asm volatile("mrs %0, sctlr_el3" : "=r" (val) : : "cc");

	return val;
}

static inline void set_sctlr(unsigned long val)
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

static inline unsigned long read_mpidr(void)
{
	unsigned long val;

	asm volatile("mrs %0, mpidr_el1" : "=r" (val));

	return val;
}

#define BSP_COREID	0

void __asm_flush_dcache_all(void);
void __asm_invalidate_dcache_all(void);
void __asm_flush_dcache_range(u64 start, u64 end);

/**
 * __asm_invalidate_dcache_range() - Invalidate a range of virtual addresses
 *
 * This performance an invalidate from @start to @end - 1. Both addresses
 * should be cache-aligned, otherwise this function will align the start
 * address and may continue past the end address.
 *
 * Data in the address range is evicted from the cache and is not written back
 * to memory.
 *
 * @start: Start address to invalidate
 * @end: End address to invalidate up to (exclusive)
 */
void __asm_invalidate_dcache_range(u64 start, u64 end);
void __asm_invalidate_tlb_all(void);
void __asm_invalidate_icache_all(void);
int __asm_invalidate_l3_dcache(void);
int __asm_flush_l3_dcache(void);
int __asm_invalidate_l3_icache(void);
void __asm_switch_ttbr(u64 new_ttbr);

/*
 * armv8_switch_to_el2_prep() - prepare for switch from EL3 to EL2 for ARMv8
 *
 * @args:        For loading 64-bit OS, fdt address.
 *               For loading 32-bit OS, zero.
 * @mach_nr:     For loading 64-bit OS, zero.
 *               For loading 32-bit OS, machine nr
 * @fdt_addr:    For loading 64-bit OS, zero.
 *               For loading 32-bit OS, fdt address.
 * @arg4:	 Input argument.
 * @entry_point: kernel entry point
 * @es_flag:     execution state flag, ES_TO_AARCH64 or ES_TO_AARCH32
 */
void armv8_switch_to_el2_prep(u64 args, u64 mach_nr, u64 fdt_addr,
			      u64 arg4, u64 entry_point, u64 es_flag);

/*
 * armv8_switch_to_el2() - switch from EL3 to EL2 for ARMv8
 *
 * @args:        For loading 64-bit OS, fdt address.
 *               For loading 32-bit OS, zero.
 * @mach_nr:     For loading 64-bit OS, zero.
 *               For loading 32-bit OS, machine nr
 * @fdt_addr:    For loading 64-bit OS, zero.
 *               For loading 32-bit OS, fdt address.
 * @arg4:	 Input argument.
 * @entry_point: kernel entry point
 * @es_flag:     execution state flag, ES_TO_AARCH64 or ES_TO_AARCH32
 */
void __noreturn armv8_switch_to_el2(u64 args, u64 mach_nr, u64 fdt_addr,
				    u64 arg4, u64 entry_point, u64 es_flag);
/*
 * armv8_switch_to_el1() - switch from EL2 to EL1 for ARMv8
 *
 * @args:        For loading 64-bit OS, fdt address.
 *               For loading 32-bit OS, zero.
 * @mach_nr:     For loading 64-bit OS, zero.
 *               For loading 32-bit OS, machine nr
 * @fdt_addr:    For loading 64-bit OS, zero.
 *               For loading 32-bit OS, fdt address.
 * @arg4:	 Input argument.
 * @entry_point: kernel entry point
 * @es_flag:     execution state flag, ES_TO_AARCH64 or ES_TO_AARCH32
 */
void armv8_switch_to_el1(u64 args, u64 mach_nr, u64 fdt_addr,
			 u64 arg4, u64 entry_point, u64 es_flag);
void armv8_el2_to_aarch32(u64 args, u64 mach_nr, u64 fdt_addr,
			  u64 arg4, u64 entry_point);
void gic_init(void);
void gic_send_sgi(unsigned long sgino);
void wait_for_wakeup(void);
void protect_secure_region(void);
void smp_kick_all_cpus(void);

void flush_l3_cache(void);

/**
 * mmu_map_region() - map a region of previously unmapped memory.
 * Will be mapped MT_NORMAL & PTE_BLOCK_INNER_SHARE.
 *
 * @start: Start address of the region
 * @size: Size of the region
 * @emerg: Also map the region in the emergency table
 */
void mmu_map_region(phys_addr_t start, u64 size, bool emerg);

/**
 * mmu_change_region_attr() - change a mapped region attributes
 *
 * @start: Start address of the region
 * @size:  Size of the region
 * @aatrs: New attributes
 */
void mmu_change_region_attr(phys_addr_t start, size_t size, u64 attrs);

/**
 * mmu_change_region_attr_nobreak() - change a mapped region attributes without doing
 *                                    break-before-make
 *
 * @start: Start address of the region
 * @size:  Size of the region
 * @aatrs: New attributes
 */
void mmu_change_region_attr_nobreak(phys_addr_t addr, size_t size, u64 attrs);

/*
 * smc_call() - issue a secure monitor call
 *
 * Issue a secure monitor call in accordance with ARM "SMC Calling convention",
 * DEN0028A
 *
 * @args: input and output arguments
 */
void smc_call(struct pt_regs *args);

void __noreturn psci_system_reset(void);
void __noreturn psci_system_reset2(u32 reset_level, u32 cookie);
void __noreturn psci_system_off(void);

#ifdef CONFIG_ARMV8_PSCI
extern char __secure_start[];
extern char __secure_end[];
extern char __secure_stack_start[];
extern char __secure_stack_end[];

void armv8_setup_psci(void);
void psci_setup_vectors(void);
void psci_arch_init(void);
#endif

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

#if defined(CONFIG_ARMV7_LPAE) && !defined(PGTABLE_SIZE)
#define PGTABLE_SIZE		(4096 * 5)
#elif !defined(PGTABLE_SIZE)
#define PGTABLE_SIZE		(4096 * 4)
#endif

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

#ifdef CONFIG_ARMV7_LPAE
void switch_to_hypervisor_ret(void);
#endif

#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");

#ifdef __ARM_ARCH_7A__
#define wfi() __asm__ __volatile__ ("wfi" : : : "memory")
#define wfe() __asm__ __volatile__ ("wfe" : : : "memory")
#define sev() __asm__ __volatile__ ("sev")
#else
#define wfi()
#endif

#if !defined(__thumb2__)
/*
 * We will need to switch to ARM mode (.arm) for some instructions such as
 * mrc p15 etc.
 */
#define asm_arm_or_thumb2(insn) asm volatile(".arm\n\t" insn)
#else
#define asm_arm_or_thumb2(insn) asm volatile(insn)
#endif

static inline unsigned long read_mpidr(void)
{
	unsigned long val;

	asm_arm_or_thumb2("mrc p15, 0, %0, c0, c0, 5" : "=r" (val));

	return val;
}

static inline unsigned long get_cpsr(void)
{
	unsigned long cpsr;

	asm volatile("mrs %0, cpsr" : "=r"(cpsr): );
	return cpsr;
}

static inline int is_hyp(void)
{
#ifdef CONFIG_ARMV7_LPAE
	/* HYP mode requires LPAE ... */
	return ((get_cpsr() & 0x1f) == 0x1a);
#else
	/* ... so without LPAE support we can optimize all hyp code away */
	return 0;
#endif
}

static inline unsigned int get_cr(void)
{
	unsigned int val;

	if (is_hyp())
		asm_arm_or_thumb2("mrc p15, 4, %0, c1, c0, 0	@ get CR"
								  : "=r" (val)
								  :
								  : "cc");
	else
		asm_arm_or_thumb2("mrc p15, 0, %0, c1, c0, 0	@ get CR"
								  : "=r" (val)
								  :
								  : "cc");
	return val;
}

static inline void set_cr(unsigned int val)
{
	if (is_hyp())
		asm_arm_or_thumb2("mcr p15, 4, %0, c1, c0, 0	@ set CR" :
								  : "r" (val)
								  : "cc");
	else
		asm_arm_or_thumb2("mcr p15, 0, %0, c1, c0, 0	@ set CR" :
								  : "r" (val)
								  : "cc");
	isb();
}

#ifdef CONFIG_ARMV7_LPAE
/* Long-Descriptor Translation Table Level 1/2 Bits */
#define TTB_SECT_XN_MASK	(1ULL << 54)
#define TTB_SECT_NG_MASK	(1 << 11)
#define TTB_SECT_AF		(1 << 10)
#define TTB_SECT_SH_MASK	(3 << 8)
#define TTB_SECT_NS_MASK	(1 << 5)
#define TTB_SECT_AP		(1 << 6)
/* Note: TTB AP bits are set elsewhere */
#define TTB_SECT_MAIR(x)	((x & 0x7) << 2) /* Index into MAIR */
#define TTB_SECT		(1 << 0)
#define TTB_PAGETABLE		(3 << 0)

/* TTBCR flags */
#define TTBCR_EAE		(1 << 31)
#define TTBCR_T0SZ(x)		((x) << 0)
#define TTBCR_T1SZ(x)		((x) << 16)
#define TTBCR_USING_TTBR0	(TTBCR_T0SZ(0) | TTBCR_T1SZ(0))
#define TTBCR_IRGN0_NC		(0 << 8)
#define TTBCR_IRGN0_WBWA	(1 << 8)
#define TTBCR_IRGN0_WT		(2 << 8)
#define TTBCR_IRGN0_WBNWA	(3 << 8)
#define TTBCR_IRGN0_MASK	(3 << 8)
#define TTBCR_ORGN0_NC		(0 << 10)
#define TTBCR_ORGN0_WBWA	(1 << 10)
#define TTBCR_ORGN0_WT		(2 << 10)
#define TTBCR_ORGN0_WBNWA	(3 << 10)
#define TTBCR_ORGN0_MASK	(3 << 10)
#define TTBCR_SHARED_NON	(0 << 12)
#define TTBCR_SHARED_OUTER	(2 << 12)
#define TTBCR_SHARED_INNER	(3 << 12)
#define TTBCR_EPD0		(0 << 7)

/*
 * VMSAv8-32 Long-descriptor format memory region attributes
 * (ARM Architecture Reference Manual section G5.7.4 [DDI0487E.a])
 *
 * MAIR0[ 7: 0] 0x00 Device-nGnRnE (aka Strongly-Ordered)
 * MAIR0[15: 8] 0xaa Outer/Inner Write-Through, Read-Allocate No Write-Allocate
 * MAIR0[23:16] 0xee Outer/Inner Write-Back, Read-Allocate No Write-Allocate
 * MAIR0[31:24] 0xff Outer/Inner Write-Back, Read-Allocate Write-Allocate
 */
#define MEMORY_ATTRIBUTES	((0x00 << (0 * 8)) | (0xaa << (1 * 8)) | \
				 (0xee << (2 * 8)) | (0xff << (3 * 8)))

/* options available for data cache on each page */
enum dcache_option {
	DCACHE_OFF = TTB_SECT | TTB_SECT_MAIR(0) | TTB_SECT_XN_MASK,
	DCACHE_WRITETHROUGH = TTB_SECT | TTB_SECT_MAIR(1),
	DCACHE_WRITEBACK = TTB_SECT | TTB_SECT_MAIR(2),
	DCACHE_WRITEALLOC = TTB_SECT | TTB_SECT_MAIR(3),
};
#elif defined(CONFIG_CPU_V7A)
/* Short-Descriptor Translation Table Level 1 Bits */
#define TTB_SECT_NS_MASK	(1 << 19)
#define TTB_SECT_NG_MASK	(1 << 17)
#define TTB_SECT_S_MASK		(1 << 16)
/* Note: TTB AP bits are set elsewhere */
#define TTB_SECT_AP		(3 << 10)
#define TTB_SECT_TEX(x)		((x & 0x7) << 12)
#define TTB_SECT_DOMAIN(x)	((x & 0xf) << 5)
#define TTB_SECT_XN_MASK	(1 << 4)
#define TTB_SECT_C_MASK		(1 << 3)
#define TTB_SECT_B_MASK		(1 << 2)
#define TTB_SECT		(2 << 0)

/*
 * Short-descriptor format memory region attributes, without TEX remap
 * (ARM Architecture Reference Manual section G5.7.2 [DDI0487E.a])
 *
 * TEX[0] C  B
 *   0    0  0   Device-nGnRnE (aka Strongly-Ordered)
 *   0    1  0   Outer/Inner Write-Through, Read-Allocate No Write-Allocate
 *   0    1  1   Outer/Inner Write-Back, Read-Allocate No Write-Allocate
 *   1    1  1   Outer/Inner Write-Back, Read-Allocate Write-Allocate
 */
enum dcache_option {
	DCACHE_OFF = TTB_SECT_DOMAIN(0) | TTB_SECT_XN_MASK | TTB_SECT,
	DCACHE_WRITETHROUGH = TTB_SECT_DOMAIN(0) | TTB_SECT | TTB_SECT_C_MASK,
	DCACHE_WRITEBACK = DCACHE_WRITETHROUGH | TTB_SECT_B_MASK,
	DCACHE_WRITEALLOC = DCACHE_WRITEBACK | TTB_SECT_TEX(1),
};
#else
#define TTB_SECT_AP		(3 << 10)
/* options available for data cache on each page */
enum dcache_option {
	DCACHE_OFF = 0x12,
	DCACHE_WRITETHROUGH = 0x1a,
	DCACHE_WRITEBACK = 0x1e,
	DCACHE_WRITEALLOC = 0x16,
};
#endif

/* Size of an MMU section */
enum {
#ifdef CONFIG_ARMV7_LPAE
	MMU_SECTION_SHIFT	= 21, /* 2MB */
#else
	MMU_SECTION_SHIFT	= 20, /* 1MB */
#endif
	MMU_SECTION_SIZE	= 1 << MMU_SECTION_SHIFT,
};

#ifdef CONFIG_CPU_V7A
/* TTBR0 bits */
#define TTBR0_BASE_ADDR_MASK	0xFFFFC000
#define TTBR0_RGN_NC			(0 << 3)
#define TTBR0_RGN_WBWA			(1 << 3)
#define TTBR0_RGN_WT			(2 << 3)
#define TTBR0_RGN_WB			(3 << 3)
/* TTBR0[6] is IRGN[0] and TTBR[0] is IRGN[1] */
#define TTBR0_IRGN_NC			(0 << 0 | 0 << 6)
#define TTBR0_IRGN_WBWA			(0 << 0 | 1 << 6)
#define TTBR0_IRGN_WT			(1 << 0 | 0 << 6)
#define TTBR0_IRGN_WB			(1 << 0 | 1 << 6)
#endif

/**
 * mmu_page_table_flush() - register an update to page tables
 *
 * Register an update to the page tables, and flush the TLB
 *
 * @start:	start address of update in page table
 * @stop:	stop address of update in page table
 */
void mmu_page_table_flush(unsigned long start, unsigned long stop);

#ifdef CONFIG_ARMV7_PSCI
void psci_arch_cpu_entry(void);
void psci_arch_init(void);
u32 psci_version(void);
s32 psci_features(u32 function_id, u32 psci_fid);
s32 psci_cpu_off(void);
s32 psci_cpu_on(u32 function_id, u32 target_cpu, u32 pc,
		u32 context_id);
s32 psci_affinity_info(u32 function_id, u32 target_affinity,
		       u32  lowest_affinity_level);
u32 psci_migrate_info_type(void);
void psci_system_off(void);
void psci_system_reset(void);
#endif

#endif /* __ASSEMBLY__ */

#define arch_align_stack(x) (x)

#endif /* __KERNEL__ */

#endif /* CONFIG_ARM64 */

#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
#define DCACHE_DEFAULT_OPTION	DCACHE_WRITETHROUGH
#elif defined(CONFIG_SYS_ARM_CACHE_WRITEALLOC)
#define DCACHE_DEFAULT_OPTION	DCACHE_WRITEALLOC
#elif defined(CONFIG_SYS_ARM_CACHE_WRITEBACK)
#define DCACHE_DEFAULT_OPTION	DCACHE_WRITEBACK
#endif

#ifndef __ASSEMBLY__
/**
 * save_boot_params() - Save boot parameters before starting reset sequence
 *
 * If you provide this function it will be called immediately U-Boot starts,
 * both for SPL and U-Boot proper.
 *
 * All registers are unchanged from U-Boot entry. No registers need be
 * preserved.
 *
 * This is not a normal C function. There is no stack. Return by branching to
 * save_boot_params_ret.
 *
 * void save_boot_params(u32 r0, u32 r1, u32 r2, u32 r3);
 */

/**
 * save_boot_params_ret() - Return from save_boot_params()
 *
 * If you provide save_boot_params(), then you should jump back to this
 * function when done. Try to preserve all registers.
 *
 * If your implementation of save_boot_params() is in C then it is acceptable
 * to simply call save_boot_params_ret() at the end of your function. Since
 * there is no link register set up, you cannot just exit the function. U-Boot
 * will return to the (initialised) value of lr, and likely crash/hang.
 *
 * If your implementation of save_boot_params() is in assembler then you
 * should use 'b' or 'bx' to return to save_boot_params_ret.
 */
void save_boot_params_ret(void);

/**
 * mmu_set_region_dcache_behaviour_phys() - set virt/phys mapping
 *
 * Change the virt/phys mapping and cache settings for a region.
 *
 * @virt:	virtual start address of memory region to change
 * @phys:	physical address for the memory region to set
 * @size:	size of memory region to change
 * @option:	dcache option to select
 */
void mmu_set_region_dcache_behaviour_phys(phys_addr_t virt, phys_addr_t phys,
					size_t size, enum dcache_option option);

/**
 * mmu_set_region_dcache_behaviour() - set cache settings
 *
 * Change the cache settings for a region.
 *
 * @start:	start address of memory region to change
 * @size:	size of memory region to change
 * @option:	dcache option to select
 */
void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option);

#endif /* __ASSEMBLY__ */

#endif
