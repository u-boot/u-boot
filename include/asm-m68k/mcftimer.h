/*
 * mcftimer.h -- ColdFire internal TIMER support defines.
 *
 * Based on mcftimer.h of uCLinux distribution:
 *      (C) Copyright 1999-2002, Greg Ungerer (gerg@snapgear.com)
 *      (C) Copyright 2000, Lineo Inc. (www.lineo.com)
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

/****************************************************************************/
#ifndef	mcftimer_h
#define	mcftimer_h
/****************************************************************************/

#include <linux/config.h>

/*
 *	Get address specific defines for this ColdFire member.
 */
#if defined(CONFIG_M5204) || defined(CONFIG_M5206) || defined(CONFIG_M5206e)
#define	MCFTIMER_BASE1		0x100	/* Base address of TIMER1 */
#define	MCFTIMER_BASE2		0x120	/* Base address of TIMER2 */
#elif defined(CONFIG_M5272)
#define MCFTIMER_BASE1		0x200	/* Base address of TIMER1 */
#define MCFTIMER_BASE2		0x220	/* Base address of TIMER2 */
#define MCFTIMER_BASE3		0x240	/* Base address of TIMER4 */
#define MCFTIMER_BASE4		0x260	/* Base address of TIMER3 */
#elif defined(CONFIG_M5249) || defined(CONFIG_M5307) || defined(CONFIG_M5407)
#define MCFTIMER_BASE1		0x140	/* Base address of TIMER1 */
#define MCFTIMER_BASE2		0x180	/* Base address of TIMER2 */
#elif defined(CONFIG_M5282) | defined(CONFIG_M5271)
#define MCFTIMER_BASE1		0x150000	/* Base address of TIMER1 */
#define MCFTIMER_BASE2		0x160000	/* Base address of TIMER2 */
#define MCFTIMER_BASE3		0x170000	/* Base address of TIMER4 */
#define MCFTIMER_BASE4		0x180000	/* Base address of TIMER3 */
#endif

/*
 *	Define the TIMER register set addresses.
 */
#define	MCFTIMER_TMR		0x00	/* Timer Mode reg (r/w) */
#define	MCFTIMER_TRR		0x02	/* Timer Reference (r/w) */
#define	MCFTIMER_TCR		0x04	/* Timer Capture reg (r/w) */
#define	MCFTIMER_TCN		0x06	/* Timer Counter reg (r/w) */
#define	MCFTIMER_TER		0x11	/* Timer Event reg (r/w) */

/*
 *	Define the TIMER register set addresses for 5282.
 */
#define MCFTIMER_PCSR		0
#define MCFTIMER_PMR		1
#define MCFTIMER_PCNTR		2

/*
 *	Bit definitions for the Timer Mode Register (TMR).
 *	Register bit flags are common accross ColdFires.
 */
#define	MCFTIMER_TMR_PREMASK	0xff00	/* Prescalar mask */
#define	MCFTIMER_TMR_DISCE	0x0000	/* Disable capture */
#define	MCFTIMER_TMR_ANYCE	0x00c0	/* Capture any edge */
#define	MCFTIMER_TMR_FALLCE	0x0080	/* Capture fallingedge */
#define	MCFTIMER_TMR_RISECE	0x0040	/* Capture rising edge */
#define	MCFTIMER_TMR_ENOM	0x0020	/* Enable output toggle */
#define	MCFTIMER_TMR_DISOM	0x0000	/* Do single output pulse  */
#define	MCFTIMER_TMR_ENORI	0x0010	/* Enable ref interrupt */
#define	MCFTIMER_TMR_DISORI	0x0000	/* Disable ref interrupt */
#define	MCFTIMER_TMR_RESTART	0x0008	/* Restart counter */
#define	MCFTIMER_TMR_FREERUN	0x0000	/* Free running counter */
#define	MCFTIMER_TMR_CLKTIN	0x0006	/* Input clock is TIN */
#define	MCFTIMER_TMR_CLK16	0x0004	/* Input clock is /16 */
#define	MCFTIMER_TMR_CLK1	0x0002	/* Input clock is /1 */
#define	MCFTIMER_TMR_CLKSTOP	0x0000	/* Stop counter */
#define	MCFTIMER_TMR_ENABLE	0x0001	/* Enable timer */
#define	MCFTIMER_TMR_DISABLE	0x0000	/* Disable timer */

/*
 *	Bit definitions for the Timer Event Registers (TER).
 */
#define	MCFTIMER_TER_CAP	0x01	/* Capture event */
#define	MCFTIMER_TER_REF	0x02	/* Refernece event */

/*
 *	Bit definitions for the 5282 PIT Control and Status Register (PCSR).
 */
#define MCFTIMER_PCSR_EN	0x0001
#define MCFTIMER_PCSR_RLD	0x0002
#define MCFTIMER_PCSR_PIF	0x0004
#define MCFTIMER_PCSR_PIE	0x0008
#define MCFTIMER_PCSR_OVW	0x0010
#define MCFTIMER_PCSR_HALTED	0x0020
#define MCFTIMER_PCSR_DOZE	0x0040

/****************************************************************************/
/* New Timer structure */
/****************************************************************************/
/* DMA Timer module registers */
typedef struct dtimer_ctrl {
	u16 tmr;		/* 0x00 Mode register */
	u8 txmr;		/* 0x02 Extended Mode register */
	u8 ter;			/* 0x03 Event register */
	u32 trr;		/* 0x04 Reference register */
	u32 tcr;		/* 0x08 Capture register */
	u32 tcn;		/* 0x0C Counter register */
} dtmr_t;

/*Programmable Interrupt Timer */
typedef struct pit_ctrl {
	u16 pcsr;		/* 0x00 Control and Status Register */
	u16 pmr;		/* 0x02 Modulus Register */
	u16 pcntr;		/* 0x04 Count Register */
} pit_t;

/*********************************************************************
* DMA Timers (DTIM)
*********************************************************************/
/* Bit definitions and macros for DTMR */
#define DTIM_DTMR_RST		(0x0001)	/* Reset */
#define DTIM_DTMR_CLK(x)	(((x)&0x0003)<<1)	/* Input clock source */
#define DTIM_DTMR_FRR		(0x0008)	/* Free run/restart */
#define DTIM_DTMR_ORRI		(0x0010)	/* Output reference request/interrupt enable */
#define DTIM_DTMR_OM		(0x0020)	/* Output Mode */
#define DTIM_DTMR_CE(x)		(((x)&0x0003)<<6)	/* Capture Edge */
#define DTIM_DTMR_PS(x)		(((x)&0x00FF)<<8)	/* Prescaler value */
#define DTIM_DTMR_RST_EN	(0x0001)
#define DTIM_DTMR_RST_RST	(0x0000)
#define DTIM_DTMR_CE_ANY	(0x00C0)
#define DTIM_DTMR_CE_FALL	(0x0080)
#define DTIM_DTMR_CE_RISE	(0x0040)
#define DTIM_DTMR_CE_NONE	(0x0000)
#define DTIM_DTMR_CLK_DTIN	(0x0006)
#define DTIM_DTMR_CLK_DIV16	(0x0004)
#define DTIM_DTMR_CLK_DIV1	(0x0002)
#define DTIM_DTMR_CLK_STOP	(0x0000)

/* Bit definitions and macros for DTXMR */
#define DTIM_DTXMR_MODE16	(0x01)	/* Increment Mode */
#define DTIM_DTXMR_DMAEN	(0x80)	/* DMA request */

/* Bit definitions and macros for DTER */
#define DTIM_DTER_CAP		(0x01)	/* Capture event */
#define DTIM_DTER_REF		(0x02)	/* Output reference event */

/*********************************************************************
*
* Programmable Interrupt Timer Modules (PIT)
*
*********************************************************************/

/* Bit definitions and macros for PCSR */
#define PIT_PCSR_EN		(0x0001)
#define PIT_PCSR_RLD		(0x0002)
#define PIT_PCSR_PIF		(0x0004)
#define PIT_PCSR_PIE		(0x0008)
#define PIT_PCSR_OVW		(0x0010)
#define PIT_PCSR_HALTED		(0x0020)
#define PIT_PCSR_DOZE		(0x0040)
#define PIT_PCSR_PRE(x)		(((x)&0x000F)<<8)

/* Bit definitions and macros for PMR */
#define PIT_PMR_PM(x)		(x)

/* Bit definitions and macros for PCNTR */
#define PIT_PCNTR_PC(x)		(x)

/****************************************************************************/
#endif				/* mcftimer_h */
