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
#include <asm/system.h>

int cpu_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	IRQ_STACK_START = _armboot_start - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE - 4;
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
static void cp_delay (void)
{
	volatile int i;

	/* copro seems to need some delay between reading and writing */
	for (i = 0; i < 100; i++);
}

void icache_enable (void)
{
	ulong reg;

	reg = get_cr ();
	cp_delay ();
	set_cr (reg | CR_C);
}

void icache_disable (void)
{
	ulong reg;

	reg = get_cr ();
	cp_delay ();
	set_cr (reg & ~CR_C);
}

int icache_status (void)
{
	return (get_cr () & CR_C) != 0;
}

void dcache_enable (void)
{
	ulong reg;

	reg = get_cr ();
	cp_delay ();
	set_cr (reg | CR_C);
}

void dcache_disable (void)
{
	ulong reg;

	reg = get_cr ();
	cp_delay ();
	set_cr (reg & ~CR_C);
}

int dcache_status (void)
{
	return (get_cr () & CR_C) != 0;
}
#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No specific cache setup for IntegratorAP/CM720T as yet */
	void icache_enable (void)
	{
	}
#endif
