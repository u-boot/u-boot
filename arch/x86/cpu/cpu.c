/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Part of this file is adapted from coreboot
 * src/arch/x86/lib/cpu.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <syscon.h>
#include <asm/control_regs.h>
#include <asm/coreboot_tables.h>
#include <asm/cpu.h>
#include <asm/lapic.h>
#include <asm/microcode.h>
#include <asm/mp.h>
#include <asm/mrccache.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>
#include <asm/interrupt.h>
#include <asm/tables.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Constructor for a conventional segment GDT (or LDT) entry
 * This is a macro so it can be used in initialisers
 */
#define GDT_ENTRY(flags, base, limit)			\
	((((base)  & 0xff000000ULL) << (56-24)) |	\
	 (((flags) & 0x0000f0ffULL) << 40) |		\
	 (((limit) & 0x000f0000ULL) << (48-16)) |	\
	 (((base)  & 0x00ffffffULL) << 16) |		\
	 (((limit) & 0x0000ffffULL)))

struct gdt_ptr {
	u16 len;
	u32 ptr;
} __packed;

struct cpu_device_id {
	unsigned vendor;
	unsigned device;
};

struct cpuinfo_x86 {
	uint8_t x86;            /* CPU family */
	uint8_t x86_vendor;     /* CPU vendor */
	uint8_t x86_model;
	uint8_t x86_mask;
};

/*
 * List of cpu vendor strings along with their normalized
 * id values.
 */
static const struct {
	int vendor;
	const char *name;
} x86_vendors[] = {
	{ X86_VENDOR_INTEL,     "GenuineIntel", },
	{ X86_VENDOR_CYRIX,     "CyrixInstead", },
	{ X86_VENDOR_AMD,       "AuthenticAMD", },
	{ X86_VENDOR_UMC,       "UMC UMC UMC ", },
	{ X86_VENDOR_NEXGEN,    "NexGenDriven", },
	{ X86_VENDOR_CENTAUR,   "CentaurHauls", },
	{ X86_VENDOR_RISE,      "RiseRiseRise", },
	{ X86_VENDOR_TRANSMETA, "GenuineTMx86", },
	{ X86_VENDOR_TRANSMETA, "TransmetaCPU", },
	{ X86_VENDOR_NSC,       "Geode by NSC", },
	{ X86_VENDOR_SIS,       "SiS SiS SiS ", },
};

static const char *const x86_vendor_name[] = {
	[X86_VENDOR_INTEL]     = "Intel",
	[X86_VENDOR_CYRIX]     = "Cyrix",
	[X86_VENDOR_AMD]       = "AMD",
	[X86_VENDOR_UMC]       = "UMC",
	[X86_VENDOR_NEXGEN]    = "NexGen",
	[X86_VENDOR_CENTAUR]   = "Centaur",
	[X86_VENDOR_RISE]      = "Rise",
	[X86_VENDOR_TRANSMETA] = "Transmeta",
	[X86_VENDOR_NSC]       = "NSC",
	[X86_VENDOR_SIS]       = "SiS",
};

static void load_ds(u32 segment)
{
	asm volatile("movl %0, %%ds" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_es(u32 segment)
{
	asm volatile("movl %0, %%es" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_fs(u32 segment)
{
	asm volatile("movl %0, %%fs" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_gs(u32 segment)
{
	asm volatile("movl %0, %%gs" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_ss(u32 segment)
{
	asm volatile("movl %0, %%ss" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_gdt(const u64 *boot_gdt, u16 num_entries)
{
	struct gdt_ptr gdt;

	gdt.len = (num_entries * X86_GDT_ENTRY_SIZE) - 1;
	gdt.ptr = (ulong)boot_gdt;

	asm volatile("lgdtl %0\n" : : "m" (gdt));
}

void arch_setup_gd(gd_t *new_gd)
{
	u64 *gdt_addr;

	gdt_addr = new_gd->arch.gdt;

	/*
	 * CS: code, read/execute, 4 GB, base 0
	 *
	 * Some OS (like VxWorks) requires GDT entry 1 to be the 32-bit CS
	 */
	gdt_addr[X86_GDT_ENTRY_UNUSED] = GDT_ENTRY(0xc09b, 0, 0xfffff);
	gdt_addr[X86_GDT_ENTRY_32BIT_CS] = GDT_ENTRY(0xc09b, 0, 0xfffff);

	/* DS: data, read/write, 4 GB, base 0 */
	gdt_addr[X86_GDT_ENTRY_32BIT_DS] = GDT_ENTRY(0xc093, 0, 0xfffff);

	/* FS: data, read/write, 4 GB, base (Global Data Pointer) */
	new_gd->arch.gd_addr = new_gd;
	gdt_addr[X86_GDT_ENTRY_32BIT_FS] = GDT_ENTRY(0xc093,
		     (ulong)&new_gd->arch.gd_addr, 0xfffff);

	/* 16-bit CS: code, read/execute, 64 kB, base 0 */
	gdt_addr[X86_GDT_ENTRY_16BIT_CS] = GDT_ENTRY(0x009b, 0, 0x0ffff);

	/* 16-bit DS: data, read/write, 64 kB, base 0 */
	gdt_addr[X86_GDT_ENTRY_16BIT_DS] = GDT_ENTRY(0x0093, 0, 0x0ffff);

	gdt_addr[X86_GDT_ENTRY_16BIT_FLAT_CS] = GDT_ENTRY(0x809b, 0, 0xfffff);
	gdt_addr[X86_GDT_ENTRY_16BIT_FLAT_DS] = GDT_ENTRY(0x8093, 0, 0xfffff);

	load_gdt(gdt_addr, X86_GDT_NUM_ENTRIES);
	load_ds(X86_GDT_ENTRY_32BIT_DS);
	load_es(X86_GDT_ENTRY_32BIT_DS);
	load_gs(X86_GDT_ENTRY_32BIT_DS);
	load_ss(X86_GDT_ENTRY_32BIT_DS);
	load_fs(X86_GDT_ENTRY_32BIT_FS);
}

#ifdef CONFIG_HAVE_FSP
/*
 * Setup FSP execution environment GDT
 *
 * Per Intel FSP external architecture specification, before calling any FSP
 * APIs, we need make sure the system is in flat 32-bit mode and both the code
 * and data selectors should have full 4GB access range. Here we reuse the one
 * we used in arch/x86/cpu/start16.S, and reload the segement registers.
 */
void setup_fsp_gdt(void)
{
	load_gdt((const u64 *)(gdt_rom + CONFIG_RESET_SEG_START), 4);
	load_ds(X86_GDT_ENTRY_32BIT_DS);
	load_ss(X86_GDT_ENTRY_32BIT_DS);
	load_es(X86_GDT_ENTRY_32BIT_DS);
	load_fs(X86_GDT_ENTRY_32BIT_DS);
	load_gs(X86_GDT_ENTRY_32BIT_DS);
}
#endif

int __weak x86_cleanup_before_linux(void)
{
#ifdef CONFIG_BOOTSTAGE_STASH
	bootstage_stash((void *)CONFIG_BOOTSTAGE_STASH_ADDR,
			CONFIG_BOOTSTAGE_STASH_SIZE);
#endif

	return 0;
}

/*
 * Cyrix CPUs without cpuid or with cpuid not yet enabled can be detected
 * by the fact that they preserve the flags across the division of 5/2.
 * PII and PPro exhibit this behavior too, but they have cpuid available.
 */

/*
 * Perform the Cyrix 5/2 test. A Cyrix won't change
 * the flags, while other 486 chips will.
 */
static inline int test_cyrix_52div(void)
{
	unsigned int test;

	__asm__ __volatile__(
	     "sahf\n\t"		/* clear flags (%eax = 0x0005) */
	     "div %b2\n\t"	/* divide 5 by 2 */
	     "lahf"		/* store flags into %ah */
	     : "=a" (test)
	     : "0" (5), "q" (2)
	     : "cc");

	/* AH is 0x02 on Cyrix after the divide.. */
	return (unsigned char) (test >> 8) == 0x02;
}

/*
 *	Detect a NexGen CPU running without BIOS hypercode new enough
 *	to have CPUID. (Thanks to Herbert Oppmann)
 */

static int deep_magic_nexgen_probe(void)
{
	int ret;

	__asm__ __volatile__ (
		"	movw	$0x5555, %%ax\n"
		"	xorw	%%dx,%%dx\n"
		"	movw	$2, %%cx\n"
		"	divw	%%cx\n"
		"	movl	$0, %%eax\n"
		"	jnz	1f\n"
		"	movl	$1, %%eax\n"
		"1:\n"
		: "=a" (ret) : : "cx", "dx");
	return  ret;
}

static bool has_cpuid(void)
{
	return flag_is_changeable_p(X86_EFLAGS_ID);
}

static bool has_mtrr(void)
{
	return cpuid_edx(0x00000001) & (1 << 12) ? true : false;
}

static int build_vendor_name(char *vendor_name)
{
	struct cpuid_result result;
	result = cpuid(0x00000000);
	unsigned int *name_as_ints = (unsigned int *)vendor_name;

	name_as_ints[0] = result.ebx;
	name_as_ints[1] = result.edx;
	name_as_ints[2] = result.ecx;

	return result.eax;
}

static void identify_cpu(struct cpu_device_id *cpu)
{
	char vendor_name[16];
	int i;

	vendor_name[0] = '\0'; /* Unset */
	cpu->device = 0; /* fix gcc 4.4.4 warning */

	/* Find the id and vendor_name */
	if (!has_cpuid()) {
		/* Its a 486 if we can modify the AC flag */
		if (flag_is_changeable_p(X86_EFLAGS_AC))
			cpu->device = 0x00000400; /* 486 */
		else
			cpu->device = 0x00000300; /* 386 */
		if ((cpu->device == 0x00000400) && test_cyrix_52div()) {
			memcpy(vendor_name, "CyrixInstead", 13);
			/* If we ever care we can enable cpuid here */
		}
		/* Detect NexGen with old hypercode */
		else if (deep_magic_nexgen_probe())
			memcpy(vendor_name, "NexGenDriven", 13);
	}
	if (has_cpuid()) {
		int  cpuid_level;

		cpuid_level = build_vendor_name(vendor_name);
		vendor_name[12] = '\0';

		/* Intel-defined flags: level 0x00000001 */
		if (cpuid_level >= 0x00000001) {
			cpu->device = cpuid_eax(0x00000001);
		} else {
			/* Have CPUID level 0 only unheard of */
			cpu->device = 0x00000400;
		}
	}
	cpu->vendor = X86_VENDOR_UNKNOWN;
	for (i = 0; i < ARRAY_SIZE(x86_vendors); i++) {
		if (memcmp(vendor_name, x86_vendors[i].name, 12) == 0) {
			cpu->vendor = x86_vendors[i].vendor;
			break;
		}
	}
}

static inline void get_fms(struct cpuinfo_x86 *c, uint32_t tfms)
{
	c->x86 = (tfms >> 8) & 0xf;
	c->x86_model = (tfms >> 4) & 0xf;
	c->x86_mask = tfms & 0xf;
	if (c->x86 == 0xf)
		c->x86 += (tfms >> 20) & 0xff;
	if (c->x86 >= 0x6)
		c->x86_model += ((tfms >> 16) & 0xF) << 4;
}

u32 cpu_get_family_model(void)
{
	return gd->arch.x86_device & 0x0fff0ff0;
}

u32 cpu_get_stepping(void)
{
	return gd->arch.x86_mask;
}

int x86_cpu_init_f(void)
{
	const u32 em_rst = ~X86_CR0_EM;
	const u32 mp_ne_set = X86_CR0_MP | X86_CR0_NE;

	if (ll_boot_init()) {
		/* initialize FPU, reset EM, set MP and NE */
		asm ("fninit\n" \
		"movl %%cr0, %%eax\n" \
		"andl %0, %%eax\n" \
		"orl  %1, %%eax\n" \
		"movl %%eax, %%cr0\n" \
		: : "i" (em_rst), "i" (mp_ne_set) : "eax");
	}

	/* identify CPU via cpuid and store the decoded info into gd->arch */
	if (has_cpuid()) {
		struct cpu_device_id cpu;
		struct cpuinfo_x86 c;

		identify_cpu(&cpu);
		get_fms(&c, cpu.device);
		gd->arch.x86 = c.x86;
		gd->arch.x86_vendor = cpu.vendor;
		gd->arch.x86_model = c.x86_model;
		gd->arch.x86_mask = c.x86_mask;
		gd->arch.x86_device = cpu.device;

		gd->arch.has_mtrr = has_mtrr();
	}
	/* Don't allow PCI region 3 to use memory in the 2-4GB memory hole */
	gd->pci_ram_top = 0x80000000U;

	/* Configure fixed range MTRRs for some legacy regions */
	if (gd->arch.has_mtrr) {
		u64 mtrr_cap;

		mtrr_cap = native_read_msr(MTRR_CAP_MSR);
		if (mtrr_cap & MTRR_CAP_FIX) {
			/* Mark the VGA RAM area as uncacheable */
			native_write_msr(MTRR_FIX_16K_A0000_MSR,
					 MTRR_FIX_TYPE(MTRR_TYPE_UNCACHEABLE),
					 MTRR_FIX_TYPE(MTRR_TYPE_UNCACHEABLE));

			/*
			 * Mark the PCI ROM area as cacheable to improve ROM
			 * execution performance.
			 */
			native_write_msr(MTRR_FIX_4K_C0000_MSR,
					 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
					 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
			native_write_msr(MTRR_FIX_4K_C8000_MSR,
					 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
					 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
			native_write_msr(MTRR_FIX_4K_D0000_MSR,
					 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
					 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
			native_write_msr(MTRR_FIX_4K_D8000_MSR,
					 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
					 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));

			/* Enable the fixed range MTRRs */
			msr_setbits_64(MTRR_DEF_TYPE_MSR, MTRR_DEF_TYPE_FIX_EN);
		}
	}

#ifdef CONFIG_I8254_TIMER
	/* Set up the i8254 timer if required */
	i8254_init();
#endif

	return 0;
}

void x86_enable_caches(void)
{
	unsigned long cr0;

	cr0 = read_cr0();
	cr0 &= ~(X86_CR0_NW | X86_CR0_CD);
	write_cr0(cr0);
	wbinvd();
}
void enable_caches(void) __attribute__((weak, alias("x86_enable_caches")));

void x86_disable_caches(void)
{
	unsigned long cr0;

	cr0 = read_cr0();
	cr0 |= X86_CR0_NW | X86_CR0_CD;
	wbinvd();
	write_cr0(cr0);
	wbinvd();
}
void disable_caches(void) __attribute__((weak, alias("x86_disable_caches")));

int x86_init_cache(void)
{
	enable_caches();

	return 0;
}
int init_cache(void) __attribute__((weak, alias("x86_init_cache")));

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("resetting ...\n");

	/* wait 50 ms */
	udelay(50000);
	disable_interrupts();
	reset_cpu(0);

	/*NOTREACHED*/
	return 0;
}

void  flush_cache(unsigned long dummy1, unsigned long dummy2)
{
	asm("wbinvd\n");
}

__weak void reset_cpu(ulong addr)
{
	/* Do a hard reset through the chipset's reset control register */
	outb(SYS_RST | RST_CPU, IO_PORT_RESET);
	for (;;)
		cpu_hlt();
}

void x86_full_reset(void)
{
	outb(FULL_RST | SYS_RST | RST_CPU, IO_PORT_RESET);
}

int dcache_status(void)
{
	return !(read_cr0() & X86_CR0_CD);
}

/* Define these functions to allow ehch-hcd to function */
void flush_dcache_range(unsigned long start, unsigned long stop)
{
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
}

void dcache_enable(void)
{
	enable_caches();
}

void dcache_disable(void)
{
	disable_caches();
}

void icache_enable(void)
{
}

void icache_disable(void)
{
}

int icache_status(void)
{
	return 1;
}

void cpu_enable_paging_pae(ulong cr3)
{
	__asm__ __volatile__(
		/* Load the page table address */
		"movl	%0, %%cr3\n"
		/* Enable pae */
		"movl	%%cr4, %%eax\n"
		"orl	$0x00000020, %%eax\n"
		"movl	%%eax, %%cr4\n"
		/* Enable paging */
		"movl	%%cr0, %%eax\n"
		"orl	$0x80000000, %%eax\n"
		"movl	%%eax, %%cr0\n"
		:
		: "r" (cr3)
		: "eax");
}

void cpu_disable_paging_pae(void)
{
	/* Turn off paging */
	__asm__ __volatile__ (
		/* Disable paging */
		"movl	%%cr0, %%eax\n"
		"andl	$0x7fffffff, %%eax\n"
		"movl	%%eax, %%cr0\n"
		/* Disable pae */
		"movl	%%cr4, %%eax\n"
		"andl	$0xffffffdf, %%eax\n"
		"movl	%%eax, %%cr4\n"
		:
		:
		: "eax");
}

static bool can_detect_long_mode(void)
{
	return cpuid_eax(0x80000000) > 0x80000000UL;
}

static bool has_long_mode(void)
{
	return cpuid_edx(0x80000001) & (1 << 29) ? true : false;
}

int cpu_has_64bit(void)
{
	return has_cpuid() && can_detect_long_mode() &&
		has_long_mode();
}

const char *cpu_vendor_name(int vendor)
{
	const char *name;
	name = "<invalid cpu vendor>";
	if ((vendor < (ARRAY_SIZE(x86_vendor_name))) &&
	    (x86_vendor_name[vendor] != 0))
		name = x86_vendor_name[vendor];

	return name;
}

char *cpu_get_name(char *name)
{
	unsigned int *name_as_ints = (unsigned int *)name;
	struct cpuid_result regs;
	char *ptr;
	int i;

	/* This bit adds up to 48 bytes */
	for (i = 0; i < 3; i++) {
		regs = cpuid(0x80000002 + i);
		name_as_ints[i * 4 + 0] = regs.eax;
		name_as_ints[i * 4 + 1] = regs.ebx;
		name_as_ints[i * 4 + 2] = regs.ecx;
		name_as_ints[i * 4 + 3] = regs.edx;
	}
	name[CPU_MAX_NAME_LEN - 1] = '\0';

	/* Skip leading spaces. */
	ptr = name;
	while (*ptr == ' ')
		ptr++;

	return ptr;
}

int default_print_cpuinfo(void)
{
	printf("CPU: %s, vendor %s, device %xh\n",
	       cpu_has_64bit() ? "x86_64" : "x86",
	       cpu_vendor_name(gd->arch.x86_vendor), gd->arch.x86_device);

	return 0;
}

#define PAGETABLE_SIZE		(6 * 4096)

/**
 * build_pagetable() - build a flat 4GiB page table structure for 64-bti mode
 *
 * @pgtable: Pointer to a 24iKB block of memory
 */
static void build_pagetable(uint32_t *pgtable)
{
	uint i;

	memset(pgtable, '\0', PAGETABLE_SIZE);

	/* Level 4 needs a single entry */
	pgtable[0] = (ulong)&pgtable[1024] + 7;

	/* Level 3 has one 64-bit entry for each GiB of memory */
	for (i = 0; i < 4; i++)
		pgtable[1024 + i * 2] = (ulong)&pgtable[2048] + 0x1000 * i + 7;

	/* Level 2 has 2048 64-bit entries, each repesenting 2MiB */
	for (i = 0; i < 2048; i++)
		pgtable[2048 + i * 2] = 0x183 + (i << 21UL);
}

int cpu_jump_to_64bit(ulong setup_base, ulong target)
{
	uint32_t *pgtable;

	pgtable = memalign(4096, PAGETABLE_SIZE);
	if (!pgtable)
		return -ENOMEM;

	build_pagetable(pgtable);
	cpu_call64((ulong)pgtable, setup_base, target);
	free(pgtable);

	return -EFAULT;
}

void show_boot_progress(int val)
{
	outb(val, POST_PORT);
}

#ifndef CONFIG_SYS_COREBOOT
/*
 * Implement a weak default function for boards that optionally
 * need to clean up the system before jumping to the kernel.
 */
__weak void board_final_cleanup(void)
{
}

int last_stage_init(void)
{
	write_tables();

	board_final_cleanup();

	return 0;
}
#endif

#ifdef CONFIG_SMP
static int enable_smis(struct udevice *cpu, void *unused)
{
	return 0;
}

static struct mp_flight_record mp_steps[] = {
	MP_FR_BLOCK_APS(mp_init_cpu, NULL, mp_init_cpu, NULL),
	/* Wait for APs to finish initialization before proceeding */
	MP_FR_BLOCK_APS(NULL, NULL, enable_smis, NULL),
};

static int x86_mp_init(void)
{
	struct mp_params mp_params;

	mp_params.parallel_microcode_load = 0,
	mp_params.flight_plan = &mp_steps[0];
	mp_params.num_records = ARRAY_SIZE(mp_steps);
	mp_params.microcode_pointer = 0;

	if (mp_init(&mp_params)) {
		printf("Warning: MP init failure\n");
		return -EIO;
	}

	return 0;
}
#endif

static int x86_init_cpus(void)
{
#ifdef CONFIG_SMP
	debug("Init additional CPUs\n");
	x86_mp_init();
#else
	struct udevice *dev;

	/*
	 * This causes the cpu-x86 driver to be probed.
	 * We don't check return value here as we want to allow boards
	 * which have not been converted to use cpu uclass driver to boot.
	 */
	uclass_first_device(UCLASS_CPU, &dev);
#endif

	return 0;
}

int cpu_init_r(void)
{
	struct udevice *dev;
	int ret;

	if (!ll_boot_init())
		return 0;

	ret = x86_init_cpus();
	if (ret)
		return ret;

	/*
	 * Set up the northbridge, PCH and LPC if available. Note that these
	 * may have had some limited pre-relocation init if they were probed
	 * before relocation, but this is post relocation.
	 */
	uclass_first_device(UCLASS_NORTHBRIDGE, &dev);
	uclass_first_device(UCLASS_PCH, &dev);
	uclass_first_device(UCLASS_LPC, &dev);

	/* Set up pin control if available */
	ret = syscon_get_by_driver_data(X86_SYSCON_PINCONF, &dev);
	debug("%s, pinctrl=%p, ret=%d\n", __func__, dev, ret);

	return 0;
}

#ifndef CONFIG_EFI_STUB
int reserve_arch(void)
{
#ifdef CONFIG_ENABLE_MRC_CACHE
	mrccache_reserve();
#endif

#ifdef CONFIG_SEABIOS
	high_table_reserve();
#endif

	return 0;
}
#endif
