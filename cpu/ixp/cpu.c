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
#include <asm/arch/ixp425.h>

ulong loops_per_jiffy;

#ifdef CONFIG_USE_IRQ
DECLARE_GLOBAL_DATA_PTR;
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo (void)
{
	unsigned long id;
	int speed = 0;

	asm ("mrc p15, 0, %0, c0, c0, 0":"=r" (id));

	puts("CPU:   Intel IXP425 at ");
	switch ((id & 0x000003f0) >> 4) {
	case 0x1c:
		loops_per_jiffy = 887467;
		speed = 533;
		break;

	case 0x1d:
		loops_per_jiffy = 666016;
		speed = 400;
		break;

	case 0x1f:
		loops_per_jiffy = 442901;
		speed = 266;
		break;
	}

	if (speed)
		printf("%d MHz\n", speed);
	else
		puts("unknown revision\n");

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

int cpu_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	IRQ_STACK_START = _armboot_start - CFG_MALLOC_LEN - CFG_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif

#if (CONFIG_COMMANDS & CFG_CMD_PCI) || defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
	pci_init();
#endif
	return 0;
}

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * just disable everything that can disturb booting linux
	 */

	unsigned long i;

	disable_interrupts ();

	/* turn off I-cache */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
	i &= ~0x1000;
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

	/* flush I-cache */
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));

	return (0);
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf ("resetting ...\n");

	udelay (50000);				/* wait 50 ms */
	disable_interrupts ();
	reset_cpu (0);

	/*NOTREACHED*/
	return (0);
}

/* taken from blob */
void icache_enable (void)
{
	register u32 i;

	/* read control register */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));

	/* set i-cache */
	i |= 0x1000;

	/* write back to control register */
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));
}

void icache_disable (void)
{
	register u32 i;

	/* read control register */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));

	/* clear i-cache */
	i &= ~0x1000;

	/* write back to control register */
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

	/* flush i-cache */
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));
}

int icache_status (void)
{
	register u32 i;

	/* read control register */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));

	/* return bit */
	return (i & 0x1000);
}

/* we will never enable dcache, because we have to setup MMU first */
void dcache_enable (void)
{
	return;
}

void dcache_disable (void)
{
	return;
}

int dcache_status (void)
{
	return 0;					/* always off */
}

/* FIXME */
/*
void pci_init(void)
{
	return;
}
*/

#ifdef CONFIG_BOOTCOUNT_LIMIT

void bootcount_store (ulong a)
{
	volatile ulong *save_addr = (volatile ulong *)(CFG_BOOTCOUNT_ADDR);

	save_addr[0] = a;
	save_addr[1] = BOOTCOUNT_MAGIC;
}

ulong bootcount_load (void)
{
	volatile ulong *save_addr = (volatile ulong *)(CFG_BOOTCOUNT_ADDR);

	if (save_addr[1] != BOOTCOUNT_MAGIC)
		return 0;
	else
		return save_addr[0];
}

#endif /* CONFIG_BOOTCOUNT_LIMIT */
