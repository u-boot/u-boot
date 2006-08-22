/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002 (440 port)
 * Scott McNutt, Artesyn Communication Producs, smcnutt@artsyncp.com
 *
 * (C) Copyright 2003 Motorola Inc. (MPC85xx port)
 * Xianghua Xiao (X.Xiao@motorola.com)
 *
 * (C) Copyright 2004 Freescale Semiconductor. (MPC86xx Port)
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
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
#include <mpc86xx.h>
#include <command.h>
#include <asm/processor.h>
#include <ppc_asm.tmpl>

unsigned long decrementer_count;    /* count value for 1e6/HZ microseconds */
unsigned long timestamp;


static __inline__ unsigned long get_msr(void)
{
	unsigned long msr;

	asm volatile ("mfmsr %0":"=r" (msr):);

	return msr;
}

static __inline__ void set_msr(unsigned long msr)
{
	asm volatile ("mtmsr %0"::"r" (msr));
}

static __inline__ unsigned long get_dec(void)
{
	unsigned long val;

	asm volatile ("mfdec %0":"=r" (val):);

	return val;
}

static __inline__ void set_dec(unsigned long val)
{
	if (val)
		asm volatile ("mtdec %0"::"r" (val));
}

/* interrupt is not supported yet */
int interrupt_init_cpu(unsigned *decrementer_count)
{
	return 0;
}

int interrupt_init(void)
{
	int ret;

	/* call cpu specific function from $(CPU)/interrupts.c */
	ret = interrupt_init_cpu(&decrementer_count);

	if (ret)
		return ret;

	decrementer_count = get_tbclk() / CFG_HZ;
	debug("interrupt init: tbclk() = %d MHz, decrementer_count = %d\n",
	      (get_tbclk() / 1000000),
	      decrementer_count);

	set_dec(decrementer_count);

	set_msr(get_msr() | MSR_EE);

	debug("MSR = 0x%08lx, Decrementer reg = 0x%08lx\n",
	      get_msr(),
	      get_dec());

	return 0;
}

void enable_interrupts(void)
{
	set_msr(get_msr() | MSR_EE);
}

/* returns flag if MSR_EE was set before */
int disable_interrupts(void)
{
	ulong msr = get_msr();

	set_msr(msr & ~MSR_EE);
	return (msr & MSR_EE) != 0;
}

void increment_timestamp(void)
{
	timestamp++;
}

/*
 * timer_interrupt - gets called when the decrementer overflows,
 * with interrupts disabled.
 * Trivial implementation - no need to be really accurate.
 */
void timer_interrupt_cpu(struct pt_regs *regs)
{
	/* nothing to do here */
}

void timer_interrupt(struct pt_regs *regs)
{
	/* call cpu specific function from $(CPU)/interrupts.c */
	timer_interrupt_cpu(regs);

	timestamp++;

	ppcDcbf(&timestamp);

	/* Restore Decrementer Count */
	set_dec(decrementer_count);

#if defined(CONFIG_WATCHDOG) || defined (CONFIG_HW_WATCHDOG)
	if ((timestamp % (CFG_WATCHDOG_FREQ)) == 0)
		WATCHDOG_RESET();
#endif /* CONFIG_WATCHDOG || CONFIG_HW_WATCHDOG */

#ifdef CONFIG_STATUS_LED
	status_led_tick(timestamp);
#endif /* CONFIG_STATUS_LED */

#ifdef CONFIG_SHOW_ACTIVITY
	board_show_activity(timestamp);
#endif /* CONFIG_SHOW_ACTIVITY */

}

void reset_timer(void)
{
	timestamp = 0;
}

ulong get_timer(ulong base)
{
	return timestamp - base;
}

void set_timer(ulong t)
{
	timestamp = t;
}

/*
 * Install and free a interrupt handler. Not implemented yet.
 */

void irq_install_handler(int vec, interrupt_handler_t *handler, void *arg)
{
}

void irq_free_handler(int vec)
{
}

/*
 * irqinfo - print information about PCI devices,not implemented.
 */
int do_irqinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("\nInterrupt-unsupported:\n");

	return 0;
}

/*
 * Handle external interrupts
 */
void external_interrupt(struct pt_regs *regs)
{
	puts("external_interrupt (oops!)\n");
}
