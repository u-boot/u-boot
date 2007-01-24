/*
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

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <clps7111.h>
#include <asm/hardware.h>

int cpu_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	IRQ_STACK_START = _armboot_start - CFG_MALLOC_LEN - CFG_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif
	return 0;
}

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 * and we set the CPU-speed to 73 MHz - see start.S for details
	 */

#if defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_ARMADILLO)
	unsigned long i;

	disable_interrupts ();

	/* turn off I-cache */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
	i &= ~0x1000;
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

	/* flush I-cache */
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));
#ifdef CONFIG_ARM7_REVD
	/* go to high speed */
	IO_SYSCON3 = (IO_SYSCON3 & ~CLKCTL) | CLKCTL_73;
#endif
#elif defined(CONFIG_NETARM) || defined(CONFIG_S3C4510B) || defined(CONFIG_LPC2292)
	disable_interrupts ();
	/* Nothing more needed */
#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No cleanup before linux for IntegratorAP/CM720T as yet */
#else
#error No cleanup_before_linux() defined for this CPU type
#endif
	return 0;
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	disable_interrupts ();
	reset_cpu (0);
	/*NOTREACHED*/
	return (0);
}

/*
 * Instruction and Data cache enable and disable functions
 *
 */

#if defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_NETARM) || defined(CONFIG_ARMADILLO)
/* read co-processor 15, register #1 (control register) */
static unsigned long read_p15_c1(void)
{
	unsigned long value;

	__asm__ __volatile__(
		"mrc     p15, 0, %0, c1, c0, 0   @ read control reg\n"
		: "=r" (value)
		:
		: "memory");
	/* printf("p15/c1 is = %08lx\n", value); */
	return value;
}

/* write to co-processor 15, register #1 (control register) */
static void write_p15_c1(unsigned long value)
{
	/* printf("write %08lx to p15/c1\n", value); */
	__asm__ __volatile__(
		"mcr     p15, 0, %0, c1, c0, 0   @ write it back\n"
		:
		: "r" (value)
		: "memory");

	read_p15_c1();
}

static void cp_delay (void)
{
	volatile int i;

	/* copro seems to need some delay between reading and writing */
	for (i = 0; i < 100; i++);
}

/* See also ARM Ref. Man. */
#define C1_MMU		(1<<0)	/* mmu off/on */
#define C1_ALIGN	(1<<1)	/* alignment faults off/on */
#define C1_IDC		(1<<2)	/* icache and/or dcache off/on */
#define C1_WRITE_BUFFER	(1<<3)	/* write buffer off/on */
#define C1_BIG_ENDIAN	(1<<7)	/* big endian off/on */
#define C1_SYS_PROT	(1<<8)	/* system protection */
#define C1_ROM_PROT	(1<<9)	/* ROM protection */
#define C1_HIGH_VECTORS	(1<<13)	/* location of vectors: low/high addresses */

void icache_enable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg | C1_IDC);
}

void icache_disable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg & ~C1_IDC);
}

int icache_status (void)
{
	return (read_p15_c1 () & C1_IDC) != 0;
}

void dcache_enable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg | C1_IDC);
}

void dcache_disable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg & ~C1_IDC);
}

int dcache_status (void)
{
	return (read_p15_c1 () & C1_IDC) != 0;
}

#elif defined(CONFIG_S3C4510B)

void icache_enable (void)
{
	s32 i;

	/* disable all cache bits */
	CLR_REG( REG_SYSCFG, 0x3F);

	/* 8KB cache, write enable */
	SET_REG( REG_SYSCFG, CACHE_WRITE_BUFF | CACHE_MODE_01);

	/* clear TAG RAM bits */
	for ( i = 0; i < 256; i++)
	  PUT_REG( CACHE_TAG_RAM + 4*i, 0x00000000);

	/* clear SET0 RAM */
	for(i=0; i < 1024; i++)
	  PUT_REG( CACHE_SET0_RAM + 4*i, 0x00000000);

	/* clear SET1 RAM */
	for(i=0; i < 1024; i++)
	  PUT_REG( CACHE_SET1_RAM + 4*i, 0x00000000);

	/* enable cache */
	SET_REG( REG_SYSCFG, CACHE_ENABLE);

}

void icache_disable (void)
{
	/* disable all cache bits */
	CLR_REG( REG_SYSCFG, 0x3F);
}

int icache_status (void)
{
	return GET_REG( REG_SYSCFG) & CACHE_ENABLE;
}

void dcache_enable (void)
{
	/* we don't have seperate instruction/data caches */
	icache_enable();
}

void dcache_disable (void)
{
	/* we don't have seperate instruction/data caches */
	icache_disable();
}

int dcache_status (void)
{
	/* we don't have seperate instruction/data caches */
	return icache_status();
}

#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No specific cache setup for IntegratorAP/CM720T as yet */
	void icache_enable (void)
	{
	}
#elif defined(CONFIG_LPC2292) /* just to satisfy the compiler */
#else
#error No icache/dcache enable/disable functions defined for this CPU type
#endif
