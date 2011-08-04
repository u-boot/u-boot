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
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>
#include <asm/interrupt.h>

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
} __attribute__((packed));

static void reload_gdt(void)
{
	/*
	 * There are machines which are known to not boot with the GDT
	 * being 8-byte unaligned.  Intel recommends 16 byte alignment
	 */
	static const u64 boot_gdt[] __attribute__((aligned(16))) = {
		/* CS: code, read/execute, 4 GB, base 0 */
		[GDT_ENTRY_32BIT_CS] = GDT_ENTRY(0xc09b, 0, 0xfffff),
		/* DS: data, read/write, 4 GB, base 0 */
		[GDT_ENTRY_32BIT_DS] = GDT_ENTRY(0xc093, 0, 0xfffff),
		/* 16-bit CS: code, read/execute, 64 kB, base 0 */
		[GDT_ENTRY_16BIT_CS] = GDT_ENTRY(0x109b, 0, 0x0ffff),
		/* 16-bit DS: data, read/write, 64 kB, base 0 */
		[GDT_ENTRY_16BIT_DS] = GDT_ENTRY(0x1093, 0, 0x0ffff),
	};
	static struct gdt_ptr gdt;

	gdt.len = sizeof(boot_gdt)-1;
	gdt.ptr = (u32)&boot_gdt;

	asm volatile("lgdtl %0\n" \
		     "movl $((2+1)*8), %%ecx\n" \
		     "movl %%ecx, %%ds\n" \
		     "movl %%ecx, %%es\n" \
		     "movl %%ecx, %%fs\n" \
		     "movl %%ecx, %%gs\n" \
		     "movl %%ecx, %%ss" \
		     : : "m" (gdt) : "ecx");
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

	return 0;
}
int cpu_init_f(void) __attribute__((weak, alias("x86_cpu_init_f")));

int x86_cpu_init_r(void)
{
	const u32 nw_cd_rst = ~(X86_CR0_NW | X86_CR0_CD);

	/* turn on the cache and disable write through */
	asm("movl	%%cr0, %%eax\n"
	    "andl	%0, %%eax\n"
	    "movl	%%eax, %%cr0\n"
	    "wbinvd\n" : : "i" (nw_cd_rst) : "eax");

	reload_gdt();

	/* Initialize core interrupt and exception functionality of CPU */
	cpu_init_interrupts ();
	return 0;
}
int cpu_init_r(void) __attribute__((weak, alias("x86_cpu_init_r")));

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf ("resetting ...\n");

	/* wait 50 ms */
	udelay(50000);
	disable_interrupts();
	reset_cpu(0);

	/*NOTREACHED*/
	return 0;
}

void  flush_cache (unsigned long dummy1, unsigned long dummy2)
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

void __reset_cpu(ulong addr)
{
	printf("Resetting using x86 Triple Fault\n");
	set_vector(13, generate_gpf);  /* general protection fault handler */
	set_vector(8, generate_gpf);   /* double fault handler */
	generate_gpf();                /* start the show */
}
void reset_cpu(ulong addr) __attribute__((weak, alias("__reset_cpu")));
