/*
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
 *
 * (C) Copyright 2006
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de
 *
 * (C) Copyright -2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <asm/stack.h>
#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <command.h>
#include <asm/irq.h>

#include <asm/leon.h>
#include <ambapp.h>

/* 15 normal irqs and a non maskable interrupt */
#define NR_IRQS 15

struct irq_action {
	interrupt_handler_t *handler;
	void *arg;
	unsigned int count;
};

extern ambapp_dev_irqmp *irqmp;
extern ambapp_dev_gptimer *gptimer;

static struct irq_action irq_handlers[NR_IRQS] = { {0}, };
static int spurious_irq_cnt = 0;
static int spurious_irq = 0;

static inline unsigned int irqmp_get_irqmask(unsigned int irq)
{
	if ((irq < 0) || (irq >= NR_IRQS)) {
		return 0;
	} else {
		return (1 << irq);
	}

}

static void leon3_ic_disable(unsigned int irq)
{
	unsigned int mask, pil;
	if (!irqmp)
		return;

	pil = intLock();

	/* get mask of interrupt */
	mask = irqmp_get_irqmask(irq);

	/* set int level */
	irqmp->cpu_mask[0] = SPARC_NOCACHE_READ(&irqmp->cpu_mask[0]) & (~mask);

	intUnlock(pil);
}

static void leon3_ic_enable(unsigned int irq)
{
	unsigned int mask, pil;
	if (!irqmp)
		return;

	pil = intLock();

	/* get mask of interrupt */
	mask = irqmp_get_irqmask(irq);

	/* set int level */
	irqmp->cpu_mask[0] = SPARC_NOCACHE_READ(&irqmp->cpu_mask[0]) | mask;

	intUnlock(pil);

}

void handler_irq(int irq, struct pt_regs *regs)
{
	if (irq_handlers[irq].handler) {
		if (((unsigned int)irq_handlers[irq].handler > CFG_RAM_END) ||
		    ((unsigned int)irq_handlers[irq].handler < CFG_RAM_BASE)
		    ) {
			printf("handler_irq: bad handler: %x, irq number %d\n",
			       (unsigned int)irq_handlers[irq].handler, irq);
			return;
		}
		irq_handlers[irq].handler(irq_handlers[irq].arg);
		irq_handlers[irq].count++;
	} else {
		spurious_irq_cnt++;
		spurious_irq = irq;
	}
}

void leon3_force_int(int irq)
{
	if (!irqmp || (irq >= NR_IRQS) || (irq < 0))
		return;
	printf("Forcing interrupt %d\n", irq);

	irqmp->iforce = SPARC_NOCACHE_READ(&irqmp->iforce) | (1 << irq);
}

/****************************************************************************/

int interrupt_init_cpu(void)
{

	return (0);
}

/****************************************************************************/

/* Handle Timer 0 IRQ */
void timer_interrupt_cpu(void *arg)
{
	gptimer->e[0].ctrl = (LEON3_GPTIMER_EN |
			      LEON3_GPTIMER_RL |
			      LEON3_GPTIMER_LD | LEON3_GPTIMER_IRQEN);
	/* nothing to do here */
	return;
}

/****************************************************************************/

/*
 * Install and free a interrupt handler.
 */

void irq_install_handler(int irq, interrupt_handler_t * handler, void *arg)
{
	if (irq < 0 || irq >= NR_IRQS) {
		printf("irq_install_handler: bad irq number %d\n", irq);
		return;
	}

	if (irq_handlers[irq].handler != NULL)
		printf("irq_install_handler: 0x%08lx replacing 0x%08lx\n",
		       (ulong) handler, (ulong) irq_handlers[irq].handler);

	if (((unsigned int)handler > CFG_RAM_END) ||
	    ((unsigned int)handler < CFG_RAM_BASE)
	    ) {
		printf("irq_install_handler: bad handler: %x, irq number %d\n",
		       (unsigned int)handler, irq);
		return;
	}
	irq_handlers[irq].handler = handler;
	irq_handlers[irq].arg = arg;

	/* enable irq on IRQMP hardware */
	leon3_ic_enable(irq);

}

void irq_free_handler(int irq)
{
	if (irq < 0 || irq >= NR_IRQS) {
		printf("irq_free_handler: bad irq number %d\n", irq);
		return;
	}

	/* disable irq on IRQMP hardware */
	leon3_ic_disable(irq);

	irq_handlers[irq].handler = NULL;
	irq_handlers[irq].arg = NULL;
}

/****************************************************************************/

#if defined(CONFIG_CMD_IRQ)
void do_irqinfo(cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc, char *argv[])
{
	int irq;
	unsigned int pil = get_pil();
	printf("PIL level: %u\n\r", pil);
	printf("Spurious IRQ: %u, last unknown IRQ: %d\n",
	       spurious_irq_cnt, spurious_irq);

	puts("\nInterrupt-Information:\n" "Nr  Routine   Arg       Count\n");

	for (irq = 0; irq < NR_IRQS; irq++) {
		if (irq_handlers[irq].handler != NULL) {
			printf("%02d  %08lx  %08lx  %ld\n", irq,
			       (unsigned int)irq_handlers[irq].handler,
			       (unsigned int)irq_handlers[irq].arg,
			       irq_handlers[irq].count);
		}
	}
}
#endif
