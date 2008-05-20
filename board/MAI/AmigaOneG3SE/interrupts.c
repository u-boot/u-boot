/*
 * (C) Copyright 2002
 * John W. Linville <linville@tuxdriver.com>
 *
 *    Copied and modified from original code by Josh Huber.  Original
 *    copyright notice preserved below.
 *
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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
 * interrupts.c - just enough support for the decrementer/timer
 */

#include <common.h>
#include <asm/processor.h>
#include <command.h>
#include "i8259.h"

#undef DEBUG
#ifdef  DEBUG
#define PRINTF(fmt,args...)     printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif
#define NR_IRQS	16

void irq_alloc_init(void);
long irq_alloc(long wanted);

/****************************************************************************/

unsigned decrementer_count;	     /* count value for 1e6/HZ microseconds */

struct irq_action {
    interrupt_handler_t *handler;
    void *arg;
    ulong count;
};

static struct irq_action irq_handlers[NR_IRQS];

/****************************************************************************/

static __inline__ unsigned long
get_msr(void)
{
	unsigned long msr;

	asm volatile("mfmsr %0" : "=r" (msr) :);
	return msr;
}

static __inline__ void
set_msr(unsigned long msr)
{
	asm volatile("mtmsr %0" : : "r" (msr));
}

static __inline__ unsigned long
get_dec(void)
{
	unsigned long val;

	asm volatile("mfdec %0" : "=r" (val) :);
	return val;
}


static __inline__ void
set_dec(unsigned long val)
{
	asm volatile("mtdec %0" : : "r" (val));
}


void
enable_interrupts(void)
{
    set_msr (get_msr() | MSR_EE);
}

/* returns flag if MSR_EE was set before */
int
disable_interrupts(void)
{
    ulong msr;

    msr = get_msr();
    set_msr (msr & ~MSR_EE);
    return ((msr & MSR_EE) != 0);
}

/****************************************************************************/

int interrupt_init (void)
{
    extern void new_reset(void);
    extern void new_reset_end(void);
#ifdef DEBUG
	puts("interrupt_init: setting decrementer_count\n");
#endif
	decrementer_count = get_tbclk() / CFG_HZ;

#ifdef DEBUG
	puts("interrupt_init: setting actual decremter\n");
#endif
	set_dec (get_tbclk() / CFG_HZ);

#ifdef DEBUG
	puts("interrupt_init: clearing external interrupt table\n");
#endif
	/* clear external interrupt table here */
	memset(irq_handlers, 0, sizeof(irq_handlers));

#ifdef DEBUG
	puts("interrupt_init: initializing interrupt controller\n");
#endif
	i8259_init();

#ifdef DEBUG
	puts("Copying reset trampoline\n");
#endif
	/* WARNING: Assmues that the first megabyte is CACHEINHIBIT! */
	memcpy((void *)0x100, new_reset, new_reset_end - new_reset);

#ifdef DEBUG
	PRINTF("interrupt_init: enabling interrupts (msr = %08x)\n",
		get_msr());
#endif
	set_msr (get_msr() | MSR_EE);

#ifdef DEBUG
	PRINTF("interrupt_init: done. (msr = %08x)\n", get_msr());
#endif

}

/****************************************************************************/

/*
 * Handle external interrupts
 */
void
external_interrupt(struct pt_regs *regs)
{
    extern int i8259_irq(void);

	int irq, unmask = 1;

	irq = i8259_irq(); /*i8259_get_irq(regs); */
/*	printf("irq = %d, handler at %p ack=%d\n", irq, irq_handlers[irq].handler, *(volatile unsigned char *)0xFEF00000); */
	i8259_mask_and_ack(irq);

	if (irq_handlers[irq].handler != NULL)
		(*irq_handlers[irq].handler)(irq_handlers[irq].arg);
	else {
		PRINTF ("\nBogus External Interrupt IRQ %d\n", irq);
		/*
		* turn off the bogus interrupt, otherwise it
		* might repeat forever
		*/
		unmask = 0;
	}

	if (unmask) i8259_unmask_irq(irq);
}

volatile ulong timestamp = 0;

/*
 * timer_interrupt - gets called when the decrementer overflows,
 * with interrupts disabled.
 * Trivial implementation - no need to be really accurate.
 */
void
timer_interrupt(struct pt_regs *regs)
{
	set_dec(decrementer_count);
	timestamp++;
}

/****************************************************************************/

void
reset_timer(void)
{
	timestamp = 0;
}

ulong
get_timer(ulong base)
{
	return (timestamp - base);
}

void
set_timer(ulong t)
{
	timestamp = t;
}

/****************************************************************************/

/*
 * Install and free a interrupt handler.
 */

void
irq_install_handler(int irq, interrupt_handler_t *handler, void *arg)
{
	if (irq < 0 || irq >= NR_IRQS) {
		PRINTF("irq_install_handler: bad irq number %d\n", irq);
		return;
	}

	if (irq_handlers[irq].handler != NULL)
		PRINTF("irq_install_handler: 0x%08lx replacing 0x%08lx\n",
		       (ulong)handler, (ulong)irq_handlers[irq].handler);

	irq_handlers[irq].handler = handler;
	irq_handlers[irq].arg     = arg;

	i8259_unmask_irq(irq);
}

void
irq_free_handler(int irq)
{
	if (irq < 0 || irq >= NR_IRQS) {
		PRINTF("irq_free_handler: bad irq number %d\n", irq);
		return;
	}

	i8259_mask_irq(irq);

	irq_handlers[irq].handler = NULL;
	irq_handlers[irq].arg     = NULL;
}

/****************************************************************************/

void
do_irqinfo(cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	puts("IRQ related functions are unimplemented currently.\n");
}
