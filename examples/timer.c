/*
 * (C) Copyright 2000
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
#include <commproc.h>
#include <mpc8xx_irq.h>
#include <exports.h>

#undef	DEBUG

#define	TIMER_PERIOD	1000000		/* 1 second clock */

static void timer_handler (void *arg);


/* Access functions for the Machine State Register */
static __inline__ unsigned long get_msr(void)
{
    unsigned long msr;

    asm volatile("mfmsr %0" : "=r" (msr) :);
    return msr;
}

static __inline__ void set_msr(unsigned long msr)
{
    asm volatile("mtmsr %0" : : "r" (msr));
}

/*
 * Definitions to access the CPM Timer registers
 * See 8xx_immap.h for Internal Memory Map layout,
 * and commproc.h for CPM Interrupt vectors (aka "IRQ"s)
 */

typedef struct tid_8xx_cpmtimer_s {
  int		 cpm_vec;	/* CPM Interrupt Vector for this timer	*/
  ushort	*tgcrp;		/* Pointer to Timer Global Config Reg.	*/
  ushort	*tmrp;		/* Pointer to Timer Mode Register	*/
  ushort	*trrp;		/* Pointer to Timer Reference Register	*/
  ushort	*tcrp;		/* Pointer to Timer Capture Register	*/
  ushort	*tcnp;		/* Pointer to Timer Counter Register	*/
  ushort	*terp;		/* Pointer to Timer Event Register	*/
} tid_8xx_cpmtimer_t;

#ifndef CLOCKRATE
#  define CLOCKRATE 64
#endif

#define	CPMT_CLOCK_DIV		16
#define	CPMT_MAX_PRESCALER	256
#define CPMT_MAX_REFERENCE	65535	/* max. unsigned short */

#define	CPMT_MAX_TICKS		(CPMT_MAX_REFERENCE * CPMT_MAX_PRESCALER)
#define	CPMT_MAX_TICKS_WITH_DIV	(CPMT_MAX_REFERENCE * CPMT_MAX_PRESCALER * CPMT_CLOCK_DIV)
#define	CPMT_MAX_INTERVAL	(CPMT_MAX_TICKS_WITH_DIV / CLOCKRATE)

/* For now: always use max. prescaler value */
#define	CPMT_PRESCALER		(CPMT_MAX_PRESCALER)

/* CPM Timer Event Register Bits */
#define	CPMT_EVENT_CAP		0x0001	/* Capture Event		*/
#define	CPMT_EVENT_REF		0x0002	/* Reference Counter Event	*/

/* CPM Timer Global Config Register */
#define	CPMT_GCR_RST		0x0001	/* Reset  Timer			*/
#define	CPMT_GCR_STP		0x0002	/* Stop   Timer			*/
#define	CPMT_GCR_FRZ		0x0004	/* Freeze Timer			*/
#define	CPMT_GCR_GM_CAS		0x0008	/* Gate Mode / Cascade Timers	*/
#define	CPMT_GCR_MASK		(CPMT_GCR_RST|CPMT_GCR_STP|CPMT_GCR_FRZ|CPMT_GCR_GM_CAS)

/* CPM Timer Mode register */
#define	CPMT_MR_GE		0x0001	/* Gate Enable			*/
#define	CPMT_MR_ICLK_CASC	0x0000	/* Clock internally cascaded	*/
#define	CPMT_MR_ICLK_CLK	0x0002	/* Clock = system clock		*/
#define	CPMT_MR_ICLK_CLKDIV	0x0004	/* Clock = system clock / 16	*/
#define	CPMT_MR_ICLK_TIN	0x0006	/* Clock = TINx signal		*/
#define	CPMT_MR_FRR		0x0008	/* Free Run / Restart		*/
#define	CPMT_MR_ORI		0x0010	/* Out. Reference Interrupt En.	*/
#define	CPMT_MR_OM		0x0020	/* Output Mode			*/
#define	CPMT_MR_CE_DIS		0x0000	/* Capture/Interrupt disabled	*/
#define	CPMT_MR_CE_RISE		0x0040	/* Capt./Interr. on rising  TIN	*/
#define CPMT_MR_CE_FALL		0x0080	/* Capt./Interr. on falling TIN	*/
#define	CPMT_MR_CE_ANY		0x00C0	/* Capt./Interr. on any TIN edge*/


/*
 * which CPM timer to use - index starts at 0 (= timer 1)
 */
#define	TID_TIMER_ID	0	/* use CPM timer 1		*/

void setPeriod (tid_8xx_cpmtimer_t *hwp, ulong interval);

static char *usage = "\n[q, b, e, ?] ";

int timer (int argc, char *argv[])
{
	DECLARE_GLOBAL_DATA_PTR;

	cpmtimer8xx_t *cpmtimerp;	/* Pointer to the CPM Timer structure   */
	tid_8xx_cpmtimer_t hw;
	tid_8xx_cpmtimer_t *hwp = &hw;
	int c;
	int running;

	app_startup(argv);

	/* Pointer to CPM Timer structure */
	cpmtimerp = &((immap_t *) gd->bd->bi_immr_base)->im_cpmtimer;

	printf ("TIMERS=0x%x\n", (unsigned) cpmtimerp);

	/* Initialize pointers depending on which timer we use */
	switch (TID_TIMER_ID) {
	case 0:
		hwp->tmrp = &(cpmtimerp->cpmt_tmr1);
		hwp->trrp = &(cpmtimerp->cpmt_trr1);
		hwp->tcrp = &(cpmtimerp->cpmt_tcr1);
		hwp->tcnp = &(cpmtimerp->cpmt_tcn1);
		hwp->terp = &(cpmtimerp->cpmt_ter1);
		hwp->cpm_vec = CPMVEC_TIMER1;
		break;
	case 1:
		hwp->tmrp = &(cpmtimerp->cpmt_tmr2);
		hwp->trrp = &(cpmtimerp->cpmt_trr2);
		hwp->tcrp = &(cpmtimerp->cpmt_tcr2);
		hwp->tcnp = &(cpmtimerp->cpmt_tcn2);
		hwp->terp = &(cpmtimerp->cpmt_ter2);
		hwp->cpm_vec = CPMVEC_TIMER2;
		break;
	case 2:
		hwp->tmrp = &(cpmtimerp->cpmt_tmr3);
		hwp->trrp = &(cpmtimerp->cpmt_trr3);
		hwp->tcrp = &(cpmtimerp->cpmt_tcr3);
		hwp->tcnp = &(cpmtimerp->cpmt_tcn3);
		hwp->terp = &(cpmtimerp->cpmt_ter3);
		hwp->cpm_vec = CPMVEC_TIMER3;
		break;
	case 3:
		hwp->tmrp = &(cpmtimerp->cpmt_tmr4);
		hwp->trrp = &(cpmtimerp->cpmt_trr4);
		hwp->tcrp = &(cpmtimerp->cpmt_tcr4);
		hwp->tcnp = &(cpmtimerp->cpmt_tcn4);
		hwp->terp = &(cpmtimerp->cpmt_ter4);
		hwp->cpm_vec = CPMVEC_TIMER4;
		break;
	}

	hwp->tgcrp = &cpmtimerp->cpmt_tgcr;

	printf ("Using timer %d\n"
			"tgcr @ 0x%x, tmr @ 0x%x, trr @ 0x%x,"
			" tcr @ 0x%x, tcn @ 0x%x, ter @ 0x%x\n",
			TID_TIMER_ID + 1,
			(unsigned) hwp->tgcrp,
			(unsigned) hwp->tmrp,
			(unsigned) hwp->trrp,
			(unsigned) hwp->tcrp,
			(unsigned) hwp->tcnp,
			(unsigned) hwp->terp
			);

	/* reset timer    */
	*hwp->tgcrp &= ~(CPMT_GCR_MASK << TID_TIMER_ID);

	/* clear all events */
	*hwp->terp = (CPMT_EVENT_CAP | CPMT_EVENT_REF);

	printf (usage);
	running = 0;
	while ((c = getc()) != 'q') {
	    if (c == 'b') {

		setPeriod (hwp, TIMER_PERIOD);	/* Set period and start ticking */

		/* Install interrupt handler (enable timer in CIMR) */
		install_hdlr (hwp->cpm_vec, timer_handler, hwp);

		printf ("Enabling timer\n");

		/* enable timer */
		*hwp->tgcrp |= (CPMT_GCR_RST << TID_TIMER_ID);
		running = 1;

#ifdef	DEBUG
		printf ("tgcr=0x%x, tmr=0x%x, trr=0x%x,"
			" tcr=0x%x, tcn=0x%x, ter=0x%x\n",
				*hwp->tgcrp, *hwp->tmrp, *hwp->trrp,
				*hwp->tcrp,  *hwp->tcnp, *hwp->terp
				);
#endif
	    } else if (c == 'e') {

		printf ("Stopping timer\n");

		*hwp->tgcrp &= ~(CPMT_GCR_MASK << TID_TIMER_ID);
		running = 0;

#ifdef	DEBUG
		printf ("tgcr=0x%x, tmr=0x%x, trr=0x%x,"
			" tcr=0x%x, tcn=0x%x, ter=0x%x\n",
				*hwp->tgcrp, *hwp->tmrp, *hwp->trrp,
				*hwp->tcrp,  *hwp->tcnp, *hwp->terp
			);
#endif
		/* Uninstall interrupt handler */
		free_hdlr (hwp->cpm_vec);

	    } else if (c == '?') {
#ifdef	DEBUG
		cpic8xx_t *cpm_icp = &((immap_t *) gd->bd->bi_immr_base)->im_cpic;
		sysconf8xx_t *siup = &((immap_t *) gd->bd->bi_immr_base)->im_siu_conf;
#endif

		printf ("\ntgcr=0x%x, tmr=0x%x, trr=0x%x,"
			" tcr=0x%x, tcn=0x%x, ter=0x%x\n",
				*hwp->tgcrp, *hwp->tmrp, *hwp->trrp,
				*hwp->tcrp,  *hwp->tcnp, *hwp->terp
			);
#ifdef	DEBUG
		printf ("SIUMCR=0x%08lx, SYPCR=0x%08lx,"
			" SIMASK=0x%08lx, SIPEND=0x%08lx\n",
				siup->sc_siumcr,
				siup->sc_sypcr,
				siup->sc_simask,
				siup->sc_sipend
			);

		printf ("CIMR=0x%08lx, CICR=0x%08lx, CIPR=0x%08lx\n",
			cpm_icp->cpic_cimr,
			cpm_icp->cpic_cicr,
			cpm_icp->cpic_cipr
			);
#endif
	    } else {
	    	printf ("\nEnter: q - quit, b - start timer, e - stop timer, ? - get status\n");
	    }
	    printf (usage);
	}
	if (running) {
		printf ("Stopping timer\n");
		*hwp->tgcrp &= ~(CPMT_GCR_MASK << TID_TIMER_ID);
		free_hdlr (hwp->cpm_vec);
	}

	return (0);
}


/* Set period in microseconds and start.
 * Truncate to maximum period if more than this is requested - but warn about it.
 */

void setPeriod (tid_8xx_cpmtimer_t *hwp, ulong interval)
{
	unsigned short prescaler;
	unsigned long ticks;

	printf ("Set interval %ld us\n", interval);

	/* Warn if requesting longer period than possible */
	if (interval > CPMT_MAX_INTERVAL) {
		printf ("Truncate interval %ld to maximum (%d)\n",
				interval, CPMT_MAX_INTERVAL);
		interval = CPMT_MAX_INTERVAL;
	}
	/*
	 * Check if we want to use clock divider:
	 * Since the reference counter can be incremented only in integer steps,
	 * we try to keep it as big as possible to allow the resulting period to be
	 * as precise as possible.
	 */
	/* prescaler, enable interrupt, restart after ref count is reached */
	prescaler = (ushort) ((CPMT_PRESCALER - 1) << 8) |
			CPMT_MR_ORI |
			CPMT_MR_FRR;

	ticks = ((ulong) CLOCKRATE * interval);

	if (ticks > CPMT_MAX_TICKS) {
		ticks /= CPMT_CLOCK_DIV;
		prescaler |= CPMT_MR_ICLK_CLKDIV;	/* use system clock divided by 16 */
	} else {
		prescaler |= CPMT_MR_ICLK_CLK;	/* use system clock without divider */
	}

#ifdef	DEBUG
	printf ("clock/%d, prescale factor %d, reference %ld, ticks %ld\n",
			(ticks > CPMT_MAX_TICKS) ? CPMT_CLOCK_DIV : 1,
			CPMT_PRESCALER,
			(ticks / CPMT_PRESCALER),
			ticks
			);
#endif

	/* set prescaler register */
	*hwp->tmrp = prescaler;

	/* clear timer counter */
	*hwp->tcnp = 0;

	/* set reference register */
	*hwp->trrp = (unsigned short) (ticks / CPMT_PRESCALER);

#ifdef	DEBUG
	printf ("tgcr=0x%x, tmr=0x%x, trr=0x%x,"
		" tcr=0x%x, tcn=0x%x, ter=0x%x\n",
			*hwp->tgcrp, *hwp->tmrp, *hwp->trrp,
			*hwp->tcrp,  *hwp->tcnp, *hwp->terp
		);
#endif
}

/*
 * Handler for CPMVEC_TIMER1 interrupt
 */
static
void timer_handler (void *arg)
{
	tid_8xx_cpmtimer_t *hwp = (tid_8xx_cpmtimer_t *)arg;

	/* printf ("** TER1=%04x ** ", *hwp->terp); */

	/* just for demonstration */
	printf (".");

	/* clear all possible events: Ref. and Cap. */
	*hwp->terp = (CPMT_EVENT_CAP | CPMT_EVENT_REF);
}
