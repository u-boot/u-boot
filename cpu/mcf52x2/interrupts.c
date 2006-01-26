/*
 * (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <asm/processor.h>

#ifdef	CONFIG_M5271
#include <asm/m5271.h>
#include <asm/immap_5271.h>
#endif

#ifdef	CONFIG_M5272
#include <asm/m5272.h>
#include <asm/immap_5272.h>
#endif

#ifdef	CONFIG_M5282
#include <asm/m5282.h>
#include <asm/immap_5282.h>
#endif

#ifdef	CONFIG_M5249
#include <asm/m5249.h>
#endif


#define	NR_IRQS		31

/*
 * Interrupt vector functions.
 */
struct interrupt_action {
	interrupt_handler_t *handler;
	void *arg;
};

static struct interrupt_action irq_vecs[NR_IRQS];

static __inline__ unsigned short get_sr (void)
{
	unsigned short sr;

	asm volatile ("move.w %%sr,%0":"=r" (sr):);

	return sr;
}

static __inline__ void set_sr (unsigned short sr)
{
	asm volatile ("move.w %0,%%sr"::"r" (sr));
}

/************************************************************************/
/*
 * Install and free an interrupt handler
 */
void irq_install_handler (int vec, interrupt_handler_t * handler, void *arg)
{
#ifdef	CONFIG_M5272
	volatile intctrl_t *intp = (intctrl_t *) (CFG_MBAR + MCFSIM_ICR1);
#endif
	int vec_base = 0;

#ifdef	CONFIG_M5272
	vec_base = intp->int_pivr & 0xe0;
#endif

	if ((vec < vec_base) || (vec > vec_base + NR_IRQS)) {
		printf ("irq_install_handler: wrong interrupt vector %d\n",
			vec);
		return;
	}

	irq_vecs[vec - vec_base].handler = handler;
	irq_vecs[vec - vec_base].arg = arg;
}

void irq_free_handler (int vec)
{
#ifdef	CONFIG_M5272
	volatile intctrl_t *intp = (intctrl_t *) (CFG_MBAR + MCFSIM_ICR1);
#endif
	int vec_base = 0;

#ifdef	CONFIG_M5272
	vec_base = intp->int_pivr & 0xe0;
#endif

	if ((vec < vec_base) || (vec > vec_base + NR_IRQS)) {
		return;
	}

	irq_vecs[vec - vec_base].handler = NULL;
	irq_vecs[vec - vec_base].arg = NULL;
}

void enable_interrupts (void)
{
	unsigned short sr;

	sr = get_sr ();
	set_sr (sr & ~0x0700);
}

int disable_interrupts (void)
{
	unsigned short sr;

	sr = get_sr ();
	set_sr (sr | 0x0700);

	return ((sr & 0x0700) == 0);	/* return TRUE, if interrupts were enabled before */
}

void int_handler (struct pt_regs *fp)
{
#ifdef	CONFIG_M5272
	volatile intctrl_t *intp = (intctrl_t *) (CFG_MBAR + MCFSIM_ICR1);
#endif
	int vec, vec_base = 0;

	vec = (fp->vector >> 2) & 0xff;
#ifdef	CONFIG_M5272
	vec_base = intp->int_pivr & 0xe0;
#endif

	if (irq_vecs[vec - vec_base].handler != NULL) {
		irq_vecs[vec -
			 vec_base].handler (irq_vecs[vec - vec_base].arg);
	} else {
		printf ("\nBogus External Interrupt Vector %d\n", vec);
	}
}


#ifdef	CONFIG_M5272
int interrupt_init (void)
{
	volatile intctrl_t *intp = (intctrl_t *) (CFG_MBAR + MCFSIM_ICR1);

	/* disable all external interrupts */
	intp->int_icr1 = 0x88888888;
	intp->int_icr2 = 0x88888888;
	intp->int_icr3 = 0x88888888;
	intp->int_icr4 = 0x88888888;
	intp->int_pitr = 0x00000000;
	/* initialize vector register */
	intp->int_pivr = 0x40;

	enable_interrupts ();

	return 0;
}
#endif

#if defined(CONFIG_M5282) || defined(CONFIG_M5271)
int interrupt_init (void)
{
	return 0;
}
#endif

#ifdef	CONFIG_M5249
int interrupt_init (void)
{
	enable_interrupts ();

	return 0;
}
#endif
