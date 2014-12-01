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
#include <errno.h>
#include <malloc.h>
#include <asm/control_regs.h>
#include <asm/cpu.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>
#include <asm/interrupt.h>
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
static struct {
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
	gdt.ptr = (u32)boot_gdt;

	asm volatile("lgdtl %0\n" : : "m" (gdt));
}

void setup_gdt(gd_t *id, u64 *gdt_addr)
{
	/* CS: code, read/execute, 4 GB, base 0 */
	gdt_addr[X86_GDT_ENTRY_32BIT_CS] = GDT_ENTRY(0xc09b, 0, 0xfffff);

	/* DS: data, read/write, 4 GB, base 0 */
	gdt_addr[X86_GDT_ENTRY_32BIT_DS] = GDT_ENTRY(0xc093, 0, 0xfffff);

	/* FS: data, read/write, 4 GB, base (Global Data Pointer) */
	id->arch.gd_addr = id;
	gdt_addr[X86_GDT_ENTRY_32BIT_FS] = GDT_ENTRY(0xc093,
		     (ulong)&id->arch.gd_addr, 0xfffff);

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

int __weak x86_cleanup_before_linux(void)
{
#ifdef CONFIG_BOOTSTAGE_STASH
	bootstage_stash((void *)CONFIG_BOOTSTAGE_STASH,
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

int x86_cpu_init_f(void)
{
	const u32 em_rst = ~X86_CR0_EM;
	const u32 mp_ne_set = X86_CR0_MP | X86_CR0_NE;

	/* initialize FPU, reset EM, set MP and NE */
	asm ("fninit\n" \
	     "movl %%cr0, %%eax\n" \
	     "andl %0, %%eax\n" \
	     "orl  %1, %%eax\n" \
	     "movl %%eax, %%cr0\n" \
	     : : "i" (em_rst), "i" (mp_ne_set) : "eax");

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
	}

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

void __attribute__ ((regparm(0))) generate_gpf(void);

/* segment 0x70 is an arbitrary segment which does not exist */
asm(".globl generate_gpf\n"
	".hidden generate_gpf\n"
	".type generate_gpf, @function\n"
	"generate_gpf:\n"
	"ljmp   $0x70, $0x47114711\n");

__weak void reset_cpu(ulong addr)
{
	printf("Resetting using x86 Triple Fault\n");
	set_vector(13, generate_gpf);	/* general protection fault handler */
	set_vector(8, generate_gpf);	/* double fault handler */
	generate_gpf();			/* start the show */
}

int dcache_status(void)
{
	return !(read_cr0() & 0x40000000);
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
	pgtable[0] = (uint32_t)&pgtable[1024] + 7;

	/* Level 3 has one 64-bit entry for each GiB of memory */
	for (i = 0; i < 4; i++) {
		pgtable[1024 + i * 2] = (uint32_t)&pgtable[2048] +
							0x1000 * i + 7;
	}

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
#if MIN_PORT80_KCLOCKS_DELAY
	/*
	 * Scale the time counter reading to avoid using 64 bit arithmetics.
	 * Can't use get_timer() here becuase it could be not yet
	 * initialized or even implemented.
	 */
	if (!gd->arch.tsc_prev) {
		gd->arch.tsc_base_kclocks = rdtsc() / 1000;
		gd->arch.tsc_prev = 0;
	} else {
		uint32_t now;

		do {
			now = rdtsc() / 1000 - gd->arch.tsc_base_kclocks;
		} while (now < (gd->arch.tsc_prev + MIN_PORT80_KCLOCKS_DELAY));
		gd->arch.tsc_prev = now;
	}
#endif
	outb(val, POST_PORT);
}
