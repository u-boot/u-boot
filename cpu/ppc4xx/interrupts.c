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
#include <watchdog.h>
#include <command.h>
#include <asm/processor.h>
#include <ppc4xx.h>
#include <ppc_asm.tmpl>
#include <commproc.h>
#include "vecnum.h"

DECLARE_GLOBAL_DATA_PTR;

/****************************************************************************/

/*
 * CPM interrupt vector functions.
 */
struct	irq_action {
	interrupt_handler_t *handler;
	void *arg;
	int count;
};

static struct irq_action irq_vecs[32];
void uic0_interrupt( void * parms); /* UIC0 handler */

#if defined(CONFIG_440)
static struct irq_action irq_vecs1[32]; /* For UIC1 */

void uic1_interrupt( void * parms); /* UIC1 handler */

#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
static struct irq_action irq_vecs2[32]; /* For UIC2 */
void uic2_interrupt( void * parms); /* UIC2 handler */
#endif /* CONFIG_440GX CONFIG_440SPE */

#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
static struct irq_action irq_vecs3[32]; /* For UIC3 */
void uic3_interrupt( void * parms); /* UIC3 handler */
#endif /* CONFIG_440SPE */

#endif /* CONFIG_440 */

/****************************************************************************/
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


static __inline__ void set_tcr(unsigned long val)
{
	asm volatile("mttcr %0" : : "r" (val));
}


static __inline__ void set_evpr(unsigned long val)
{
	asm volatile("mtevpr %0" : : "r" (val));
}
#endif /* defined(CONFIG_440 */

/****************************************************************************/

int interrupt_init_cpu (unsigned *decrementer_count)
{
	int vec;
	unsigned long val;

	/* decrementer is automatically reloaded */
	*decrementer_count = 0;

	/*
	 * Mark all irqs as free
	 */
	for (vec=0; vec<32; vec++) {
		irq_vecs[vec].handler = NULL;
		irq_vecs[vec].arg = NULL;
		irq_vecs[vec].count = 0;
#if defined(CONFIG_440)
		irq_vecs1[vec].handler = NULL;
		irq_vecs1[vec].arg = NULL;
		irq_vecs1[vec].count = 0;
#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
		irq_vecs2[vec].handler = NULL;
		irq_vecs2[vec].arg = NULL;
		irq_vecs2[vec].count = 0;
#endif /* CONFIG_440GX */
#if defined(CONFIG_440SPE) || defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
		irq_vecs3[vec].handler = NULL;
		irq_vecs3[vec].arg = NULL;
		irq_vecs3[vec].count = 0;
#endif /* CONFIG_440SPE */
#endif
	}

#ifdef CONFIG_4xx
	/*
	 * Init PIT
	 */
#if defined(CONFIG_440)
	val = mfspr( tcr );
	val &= (~0x04400000);		/* clear DIS & ARE */
	mtspr( tcr, val );
	mtspr( dec, 0 );		/* Prevent exception after TSR clear*/
	mtspr( decar, 0 );		/* clear reload */
	mtspr( tsr, 0x08000000 );	/* clear DEC status */
	val = gd->bd->bi_intfreq/1000;	/* 1 msec */
	mtspr( decar, val );		/* Set auto-reload value */
	mtspr( dec, val );		/* Set inital val */
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
	val = mfspr(tcr);
	val |= 0x04400000;
	mtspr(tcr, val);

	/*
	 * Set EVPR to 0
	 */
	set_evpr(0x00000000);

#if defined(CONFIG_440)
#if !defined(CONFIG_440GX)
	/* Install the UIC1 handlers */
	irq_install_handler(VECNUM_UIC1NC, uic1_interrupt, 0);
	irq_install_handler(VECNUM_UIC1C, uic1_interrupt, 0);
#endif
#endif

#if defined(CONFIG_440GX)
	/* Take the GX out of compatibility mode
	 * Travis Sawyer, 9 Mar 2004
	 * NOTE: 440gx user manual inconsistency here
	 *       Compatibility mode and Ethernet Clock select are not
	 *       correct in the manual
	 */
	mfsdr(sdr_mfr, val);
	val &= ~0x10000000;
	mtsdr(sdr_mfr,val);

	/* Enable UIC interrupts via UIC Base Enable Register */
	mtdcr(uicb0sr, UICB0_ALL);
	mtdcr(uicb0er, 0x54000000);
	/* None are critical */
	mtdcr(uicb0cr, 0);
#endif

	return (0);
}

/****************************************************************************/

/*
 * Handle external interrupts
 */
#if defined(CONFIG_440GX)
void external_interrupt(struct pt_regs *regs)
{
	ulong uic_msr;

	/*
	 * Read masked interrupt status register to determine interrupt source
	 */
	/* 440 GX uses base uic register */
	uic_msr = mfdcr(uicb0msr);

	if ( (UICB0_UIC0CI & uic_msr) || (UICB0_UIC0NCI & uic_msr) )
		uic0_interrupt(0);

	if ( (UICB0_UIC1CI & uic_msr) || (UICB0_UIC1NCI & uic_msr) )
		uic1_interrupt(0);

	if ( (UICB0_UIC2CI & uic_msr) || (UICB0_UIC2NCI & uic_msr) )
		uic2_interrupt(0);

	mtdcr(uicb0sr, uic_msr);

	return;

} /* external_interrupt CONFIG_440GX */

#elif defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
void external_interrupt(struct pt_regs *regs)
{
	ulong uic_msr;

	/*
	 * Read masked interrupt status register to determine interrupt source
	 */
	/* 440 SPe uses base uic register */
	uic_msr = mfdcr(uic0msr);

	if ( (UICB0_UIC1CI & uic_msr) || (UICB0_UIC1NCI & uic_msr) )
		uic1_interrupt(0);

	if ( (UICB0_UIC2CI & uic_msr) || (UICB0_UIC2NCI & uic_msr) )
		uic2_interrupt(0);

	if (uic_msr & ~(UICB0_ALL))
		uic0_interrupt(0);

	mtdcr(uic0sr, uic_msr);

	return;

} /* external_interrupt CONFIG_440EPX & CONFIG_440GRX */

#elif defined(CONFIG_440SPE)
void external_interrupt(struct pt_regs *regs)
{
	ulong uic_msr;

	/*
	 * Read masked interrupt status register to determine interrupt source
	 */
	/* 440 SPe uses base uic register */
	uic_msr = mfdcr(uic0msr);

	if ( (UICB0_UIC1CI & uic_msr) || (UICB0_UIC1NCI & uic_msr) )
		uic1_interrupt(0);

	if ( (UICB0_UIC2CI & uic_msr) || (UICB0_UIC2NCI & uic_msr) )
		uic2_interrupt(0);

	if ( (UICB0_UIC3CI & uic_msr) || (UICB0_UIC3NCI & uic_msr) )
		uic3_interrupt(0);

	if (uic_msr & ~(UICB0_ALL))
		uic0_interrupt(0);

	mtdcr(uic0sr, uic_msr);

	return;
} /* external_interrupt CONFIG_440SPE */

#else

void external_interrupt(struct pt_regs *regs)
{
	ulong uic_msr;
	ulong msr_shift;
	int vec;

	/*
	 * Read masked interrupt status register to determine interrupt source
	 */
	uic_msr = mfdcr(uicmsr);
	msr_shift = uic_msr;
	vec = 0;

	while (msr_shift != 0) {
		if (msr_shift & 0x80000000) {
			/*
			 * Increment irq counter (for debug purpose only)
			 */
			irq_vecs[vec].count++;

			if (irq_vecs[vec].handler != NULL) {
				/* call isr */
				(*irq_vecs[vec].handler)(irq_vecs[vec].arg);
			} else {
				mtdcr(uicer, mfdcr(uicer) & ~(0x80000000 >> vec));
				printf ("Masking bogus interrupt vector 0x%x\n", vec);
			}

			/*
			 * After servicing the interrupt, we have to remove the status indicator.
			 */
			mtdcr(uicsr, (0x80000000 >> vec));
		}

		/*
		 * Shift msr to next position and increment vector
		 */
		msr_shift <<= 1;
		vec++;
	}
}
#endif

#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
/* Handler for UIC0 interrupt */
void uic0_interrupt( void * parms)
{
	ulong uic_msr;
	ulong msr_shift;
	int vec;

	/*
	 * Read masked interrupt status register to determine interrupt source
	 */
	uic_msr = mfdcr(uicmsr);
	msr_shift = uic_msr;
	vec = 0;

	while (msr_shift != 0) {
		if (msr_shift & 0x80000000) {
			/*
			 * Increment irq counter (for debug purpose only)
			 */
			irq_vecs[vec].count++;

			if (irq_vecs[vec].handler != NULL) {
				/* call isr */
				(*irq_vecs[vec].handler)(irq_vecs[vec].arg);
			} else {
				mtdcr(uicer, mfdcr(uicer) & ~(0x80000000 >> vec));
				printf ("Masking bogus interrupt vector (uic0) 0x%x\n", vec);
			}

			/*
			 * After servicing the interrupt, we have to remove the status indicator.
			 */
			mtdcr(uicsr, (0x80000000 >> vec));
		}

		/*
		 * Shift msr to next position and increment vector
		 */
		msr_shift <<= 1;
		vec++;
	}
}

#endif /* CONFIG_440GX */

#if defined(CONFIG_440)
/* Handler for UIC1 interrupt */
void uic1_interrupt( void * parms)
{
	ulong uic1_msr;
	ulong msr_shift;
	int vec;

	/*
	 * Read masked interrupt status register to determine interrupt source
	 */
	uic1_msr = mfdcr(uic1msr);
	msr_shift = uic1_msr;
	vec = 0;

	while (msr_shift != 0) {
		if (msr_shift & 0x80000000) {
			/*
			 * Increment irq counter (for debug purpose only)
			 */
			irq_vecs1[vec].count++;

			if (irq_vecs1[vec].handler != NULL) {
				/* call isr */
				(*irq_vecs1[vec].handler)(irq_vecs1[vec].arg);
			} else {
				mtdcr(uic1er, mfdcr(uic1er) & ~(0x80000000 >> vec));
				printf ("Masking bogus interrupt vector (uic1) 0x%x\n", vec);
			}

			/*
			 * After servicing the interrupt, we have to remove the status indicator.
			 */
			mtdcr(uic1sr, (0x80000000 >> vec));
		}

		/*
		 * Shift msr to next position and increment vector
		 */
		msr_shift <<= 1;
		vec++;
	}
}
#endif /* defined(CONFIG_440) */

#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
/* Handler for UIC2 interrupt */
void uic2_interrupt( void * parms)
{
	ulong uic2_msr;
	ulong msr_shift;
	int vec;

	/*
	 * Read masked interrupt status register to determine interrupt source
	 */
	uic2_msr = mfdcr(uic2msr);
	msr_shift = uic2_msr;
	vec = 0;

	while (msr_shift != 0) {
		if (msr_shift & 0x80000000) {
			/*
			 * Increment irq counter (for debug purpose only)
			 */
			irq_vecs2[vec].count++;

			if (irq_vecs2[vec].handler != NULL) {
				/* call isr */
				(*irq_vecs2[vec].handler)(irq_vecs2[vec].arg);
			} else {
				mtdcr(uic2er, mfdcr(uic2er) & ~(0x80000000 >> vec));
				printf ("Masking bogus interrupt vector (uic2) 0x%x\n", vec);
			}

			/*
			 * After servicing the interrupt, we have to remove the status indicator.
			 */
			mtdcr(uic2sr, (0x80000000 >> vec));
		}

		/*
		 * Shift msr to next position and increment vector
		 */
		msr_shift <<= 1;
		vec++;
	}
}
#endif /* defined(CONFIG_440GX) */

#if defined(CONFIG_440SPE)
/* Handler for UIC3 interrupt */
void uic3_interrupt( void * parms)
{
	ulong uic3_msr;
	ulong msr_shift;
	int vec;

	/*
	 * Read masked interrupt status register to determine interrupt source
	 */
	uic3_msr = mfdcr(uic3msr);
	msr_shift = uic3_msr;
	vec = 0;

	while (msr_shift != 0) {
		if (msr_shift & 0x80000000) {
			/*
			 * Increment irq counter (for debug purpose only)
			 */
			irq_vecs3[vec].count++;

			if (irq_vecs3[vec].handler != NULL) {
				/* call isr */
				(*irq_vecs3[vec].handler)(irq_vecs3[vec].arg);
			} else {
				mtdcr(uic3er, mfdcr(uic3er) & ~(0x80000000 >> vec));
				printf ("Masking bogus interrupt vector (uic3) 0x%x\n", vec);
			}

			/*
			 * After servicing the interrupt, we have to remove the status indicator.
			 */
			mtdcr(uic3sr, (0x80000000 >> vec));
		}

		/*
		 * Shift msr to next position and increment vector
		 */
		msr_shift <<= 1;
		vec++;
	}
}
#endif /* defined(CONFIG_440SPE) */

/****************************************************************************/

/*
 * Install and free a interrupt handler.
 */

void irq_install_handler (int vec, interrupt_handler_t * handler, void *arg)
{
	struct irq_action *irqa = irq_vecs;
	int i = vec;

#if defined(CONFIG_440)
#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	if ((vec > 31) && (vec < 64)) {
		i = vec - 32;
		irqa = irq_vecs1;
	} else if (vec > 63) {
		i = vec - 64;
		irqa = irq_vecs2;
	}
#else  /* CONFIG_440GX */
	if (vec > 31) {
		i = vec - 32;
		irqa = irq_vecs1;
	}
#endif /* CONFIG_440GX */
#endif /* CONFIG_440 */

	/*
	 * print warning when replacing with a different irq vector
	 */
	if ((irqa[i].handler != NULL) && (irqa[i].handler != handler)) {
		printf ("Interrupt vector %d: handler 0x%x replacing 0x%x\n",
			vec, (uint) handler, (uint) irqa[i].handler);
	}
	irqa[i].handler = handler;
	irqa[i].arg = arg;

#if defined(CONFIG_440)
#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	if ((vec > 31) && (vec < 64))
		mtdcr (uic1er, mfdcr (uic1er) | (0x80000000 >> i));
	else if (vec > 63)
		mtdcr (uic2er, mfdcr (uic2er) | (0x80000000 >> i));
	else
#endif /* CONFIG_440GX */
	if (vec > 31)
		mtdcr (uic1er, mfdcr (uic1er) | (0x80000000 >> i));
	else
#endif
		mtdcr (uicer, mfdcr (uicer) | (0x80000000 >> i));
#if 0
	printf ("Install interrupt for vector %d ==> %p\n", vec, handler);
#endif
}

void irq_free_handler (int vec)
{
	struct irq_action *irqa = irq_vecs;
	int i = vec;

#if defined(CONFIG_440)
#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	if ((vec > 31) && (vec < 64)) {
		irqa = irq_vecs1;
		i = vec - 32;
	} else if (vec > 63) {
		irqa = irq_vecs2;
		i = vec - 64;
	}
#endif /* CONFIG_440GX */
	if (vec > 31) {
		irqa = irq_vecs1;
		i = vec - 32;
	}
#endif

#if 0
	printf ("Free interrupt for vector %d ==> %p\n",
		vec, irq_vecs[vec].handler);
#endif

#if defined(CONFIG_440)
#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	if ((vec > 31) && (vec < 64))
		mtdcr (uic1er, mfdcr (uic1er) & ~(0x80000000 >> i));
	else if (vec > 63)
		mtdcr (uic2er, mfdcr (uic2er) & ~(0x80000000 >> i));
	else
#endif /* CONFIG_440GX */
	if (vec > 31)
		mtdcr (uic1er, mfdcr (uic1er) & ~(0x80000000 >> i));
	else
#endif
		mtdcr (uicer, mfdcr (uicer) & ~(0x80000000 >> i));

	irqa[i].handler = NULL;
	irqa[i].arg = NULL;
}

/****************************************************************************/

void timer_interrupt_cpu (struct pt_regs *regs)
{
	/* nothing to do here */
	return;
}

/****************************************************************************/

#if (CONFIG_COMMANDS & CFG_CMD_IRQ) || defined(CONFIG_CMD_IRQ)

/*******************************************************************************
 *
 * irqinfo - print information about PCI devices
 *
 */
int
do_irqinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int vec;

	printf ("\nInterrupt-Information:\n");
#if defined(CONFIG_440)
	printf ("\nUIC 0\n");
#endif
	printf ("Nr  Routine   Arg       Count\n");

	for (vec=0; vec<32; vec++) {
		if (irq_vecs[vec].handler != NULL) {
			printf ("%02d  %08lx  %08lx  %d\n",
				vec,
				(ulong)irq_vecs[vec].handler,
				(ulong)irq_vecs[vec].arg,
				irq_vecs[vec].count);
		}
	}

#if defined(CONFIG_440)
	printf ("\nUIC 1\n");
	printf ("Nr  Routine   Arg       Count\n");

	for (vec=0; vec<32; vec++) {
		if (irq_vecs1[vec].handler != NULL)
			printf ("%02d  %08lx  %08lx  %d\n",
				vec+31, (ulong)irq_vecs1[vec].handler,
				(ulong)irq_vecs1[vec].arg, irq_vecs1[vec].count);
	}
	printf("\n");
#endif

#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	printf ("\nUIC 2\n");
	printf ("Nr  Routine   Arg       Count\n");

	for (vec=0; vec<32; vec++) {
		if (irq_vecs2[vec].handler != NULL)
			printf ("%02d  %08lx  %08lx  %d\n",
				vec+63, (ulong)irq_vecs2[vec].handler,
				(ulong)irq_vecs2[vec].arg, irq_vecs2[vec].count);
	}
	printf("\n");
#endif

#if defined(CONFIG_440SPE)
	printf ("\nUIC 3\n");
	printf ("Nr  Routine   Arg       Count\n");

	for (vec=0; vec<32; vec++) {
		if (irq_vecs3[vec].handler != NULL)
			printf ("%02d  %08lx  %08lx  %d\n",
					vec+63, (ulong)irq_vecs3[vec].handler,
					(ulong)irq_vecs3[vec].arg, irq_vecs3[vec].count);
	}
	printf("\n");
#endif

	return 0;
}
#endif  /* CONFIG_COMMANDS & CFG_CMD_IRQ */
