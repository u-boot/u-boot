/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002 (440 port)
 * Scott McNutt, Artesyn Communication Producs, smcnutt@artsyncp.com
 *
 * (C) Copyright 2003 (440GX port)
 * Travis B. Sawyer, Sandburst Corporation, tsawyer@sandburst.com
 *
 * (C) Copyright 2008 (PPC440X05 port for Virtex 5 FX)
 * Ricardo Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@gmail.com
 * Work supported by Qtechnology (htpp://qtec.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/interrupt.h>
#include <asm/ppc4xx.h>
#include <ppc_asm.tmpl>
#include <commproc.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * CPM interrupt vector functions.
 */
struct	irq_action {
	interrupt_handler_t *handler;
	void *arg;
	int count;
};
static struct irq_action irq_vecs[IRQ_MAX];

#if defined(CONFIG_440)

/* SPRN changed in 440 */
static __inline__ void set_evpr(unsigned long val)
{
	asm volatile("mtspr 0x03f,%0" : : "r" (val));
}

#else /* !defined(CONFIG_440) */

static __inline__ void set_pit(unsigned long val)
{
	asm volatile("mtpit %0" : : "r" (val));
}

static __inline__ void set_evpr(unsigned long val)
{
	asm volatile("mtevpr %0" : : "r" (val));
}
#endif /* defined(CONFIG_440 */

int interrupt_init_cpu (unsigned *decrementer_count)
{
	int vec;
	unsigned long val;

	/* decrementer is automatically reloaded */
	*decrementer_count = 0;

	/*
	 * Mark all irqs as free
	 */
	for (vec = 0; vec < IRQ_MAX; vec++) {
		irq_vecs[vec].handler = NULL;
		irq_vecs[vec].arg = NULL;
		irq_vecs[vec].count = 0;
	}

#ifdef CONFIG_4xx
	/*
	 * Init PIT
	 */
#if defined(CONFIG_440)
	val = mfspr( SPRN_TCR );
	val &= (~0x04400000);		/* clear DIS & ARE */
	mtspr( SPRN_TCR, val );
	mtspr( SPRN_DEC, 0 );		/* Prevent exception after TSR clear*/
	mtspr( SPRN_DECAR, 0 );		/* clear reload */
	mtspr( SPRN_TSR, 0x08000000 );	/* clear DEC status */
	val = gd->bd->bi_intfreq/1000;	/* 1 msec */
	mtspr( SPRN_DECAR, val );		/* Set auto-reload value */
	mtspr( SPRN_DEC, val );		/* Set inital val */
#else
	set_pit(gd->bd->bi_intfreq / 1000);
#endif
#endif  /* CONFIG_4xx */

#ifdef CONFIG_ADCIOP
	/*
	 * Init PIT
	 */
	set_pit(66000);
#endif

	/*
	 * Enable PIT
	 */
	val = mfspr(SPRN_TCR);
	val |= 0x04400000;
	mtspr(SPRN_TCR, val);

	/*
	 * Set EVPR to 0
	 */
	set_evpr(0x00000000);

	/*
	 * Call uic or xilinx_irq pic_enable
	 */
	pic_enable();

	return (0);
}

void timer_interrupt_cpu(struct pt_regs *regs)
{
	/* nothing to do here */
	return;
}

void interrupt_run_handler(int vec)
{
	irq_vecs[vec].count++;

	if (irq_vecs[vec].handler != NULL) {
		/* call isr */
		(*irq_vecs[vec].handler) (irq_vecs[vec].arg);
	} else {
		pic_irq_disable(vec);
		printf("Masking bogus interrupt vector %d\n", vec);
	}

	pic_irq_ack(vec);
	return;
}

void irq_install_handler(int vec, interrupt_handler_t * handler, void *arg)
{
	/*
	 * Print warning when replacing with a different irq vector
	 */
	if ((irq_vecs[vec].handler != NULL) && (irq_vecs[vec].handler != handler)) {
		printf("Interrupt vector %d: handler 0x%x replacing 0x%x\n",
		       vec, (uint) handler, (uint) irq_vecs[vec].handler);
	}
	irq_vecs[vec].handler = handler;
	irq_vecs[vec].arg = arg;

	pic_irq_enable(vec);
	return;
}

void irq_free_handler(int vec)
{
	debug("Free interrupt for vector %d ==> %p\n",
	      vec, irq_vecs[vec].handler);

	pic_irq_disable(vec);

	irq_vecs[vec].handler = NULL;
	irq_vecs[vec].arg = NULL;
	return;
}

#if defined(CONFIG_CMD_IRQ)
int do_irqinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int vec;

	printf ("Interrupt-Information:\n");
	printf ("Nr  Routine   Arg       Count\n");

	for (vec = 0; vec < IRQ_MAX; vec++) {
		if (irq_vecs[vec].handler != NULL) {
			printf ("%02d  %08lx  %08lx  %d\n",
				vec,
				(ulong)irq_vecs[vec].handler,
				(ulong)irq_vecs[vec].arg,
				irq_vecs[vec].count);
		}
	}

	return 0;
}
#endif
