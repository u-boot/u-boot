/******************************************************************************
 *
 * Name:	sktimer.c
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.12 $
 * Date:	$Date: 1999/11/22 13:38:51 $
 * Purpose:	High level timer functions.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998,1999 SysKonnect,
 *	a business unit of Schneider & Koch & Co. Datensysteme GmbH.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * History:
 *
 *	$Log: sktimer.c,v $
 *	Revision 1.12  1999/11/22 13:38:51  cgoos
 *	Changed license header to GPL.
 *	
 *	Revision 1.11  1998/12/17 13:24:13  gklug
 *	fix: restart problem: do NOT destroy timer queue if init 1 is done
 *	
 *	Revision 1.10  1998/10/15 15:11:36  gklug
 *	fix: ID_sccs to SysKonnectFileId
 *	
 *	Revision 1.9  1998/09/15 15:15:04  cgoos
 *	Changed TRUE/FALSE to SK_TRUE/SK_FALSE
 *	
 *	Revision 1.8  1998/09/08 08:47:55  gklug
 *	add: init level handling
 *	
 *	Revision 1.7  1998/08/19 09:50:53  gklug
 *	fix: remove struct keyword from c-code (see CCC) add typedefs
 *	
 *	Revision 1.6  1998/08/17 13:43:13  gklug
 *	chg: Parameter will be union of 64bit para, 2 times SK_U32 or SK_PTR
 *	
 *	Revision 1.5  1998/08/14 07:09:14  gklug
 *	fix: chg pAc -> pAC
 *	
 *	Revision 1.4  1998/08/07 12:53:46  gklug
 *	fix: first compiled version
 *	
 *	Revision 1.3  1998/08/07 09:31:53  gklug
 *	fix: delta spelling
 *	
 *	Revision 1.2  1998/08/07 09:31:02  gklug
 *	adapt functions to new c coding conventions
 *	rmv: "fast" handling
 *	chg: inserting of new timer in queue.
 *	chg: event queue generation when timer runs out
 *	
 *	Revision 1.1  1998/08/05 11:27:55  gklug
 *	first version: adapted from SMT
 *	
 *	
 *	
 *
 ******************************************************************************/


/*
	Event queue and dispatcher
*/
static const char SysKonnectFileId[] =
	"$Header: /usr56/projects/ge/schedule/sktimer.c,v 1.12 1999/11/22 13:38:51 cgoos Exp $" ;

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#include "h/skdrv2nd.h"		/* Adapter Control- and Driver specific Def. */

#ifdef __C2MAN__
/*
	Event queue management.

	General Description:

 */
intro()
{}
#endif


/* Forward declaration */
static void timer_done(SK_AC *pAC,SK_IOC Ioc,int Restart);


/*
 * Inits the software timer
 *
 * needs to be called during Init level 1.
 */
void	SkTimerInit(
SK_AC	*pAC,		/* Adapters context */
SK_IOC	Ioc,		/* IoContext */
int	Level)		/* Init Level */
{
	switch (Level) {
	case SK_INIT_DATA:
		pAC->Tim.StQueue = 0 ;
		break;
	case SK_INIT_IO:
		SkHwtInit(pAC,Ioc) ;
		SkTimerDone(pAC, Ioc);
		break;
	default:
		break;
	}
}

/*
 * Stops a high level timer
 * - If a timer is not in the queue the function returns normally, too.
 */
void	SkTimerStop(
SK_AC		*pAC,		/* Adapters context */
SK_IOC		Ioc,		/* IoContext */
SK_TIMER	*pTimer)	/* Timer Pointer to be started */
{
	SK_TIMER	**ppTimPrev ;
	SK_TIMER	*pTm ;

	/*
	 * remove timer from queue
	 */
	pTimer->TmActive = SK_FALSE ;
	if (pAC->Tim.StQueue == pTimer && !pTimer->TmNext) {
		SkHwtStop(pAC,Ioc) ;
	}
	for (ppTimPrev = &pAC->Tim.StQueue ; (pTm = *ppTimPrev) ;
		ppTimPrev = &pTm->TmNext ) {
		if (pTm == pTimer) {
			/*
			 * Timer found in queue
			 * - dequeue it and
			 * - correct delta of the next timer
			 */
			*ppTimPrev = pTm->TmNext ;

			if (pTm->TmNext) {
				/* correct delta of next timer in queue */
				pTm->TmNext->TmDelta += pTm->TmDelta ;
			}
			return ;
		}
	}
}

/*
 * Start a high level software timer
 */
void	SkTimerStart(
SK_AC		*pAC,		/* Adapters context */
SK_IOC		Ioc,		/* IoContext */
SK_TIMER	*pTimer,	/* Timer Pointer to be started */
SK_U32		Time,		/* Time value */
SK_U32		Class,		/* Event Class for this timer */
SK_U32		Event,		/* Event Value for this timer */
SK_EVPARA	Para)		/* Event Parameter for this timer */
{
	SK_TIMER	**ppTimPrev ;
	SK_TIMER	*pTm ;
	SK_U32		Delta ;

	Time /= 16 ;		/* input is uS, clock ticks are 16uS */
	if (!Time)
		Time = 1 ;

	SkTimerStop(pAC,Ioc,pTimer) ;

	pTimer->TmClass = Class ;
	pTimer->TmEvent = Event ;
	pTimer->TmPara = Para ;
	pTimer->TmActive = SK_TRUE ;

	if (!pAC->Tim.StQueue) {
		/* First Timer to be started */
		pAC->Tim.StQueue = pTimer ;
		pTimer->TmNext = 0 ;
		pTimer->TmDelta = Time ;
		SkHwtStart(pAC,Ioc,Time) ;
		return ;
	}

	/*
	 * timer correction
	 */
	timer_done(pAC,Ioc,0) ;

	/*
	 * find position in queue
	 */
	Delta = 0 ;
	for (ppTimPrev = &pAC->Tim.StQueue ; (pTm = *ppTimPrev) ;
		ppTimPrev = &pTm->TmNext ) {
		if (Delta + pTm->TmDelta > Time) {
			/* Position found */
			/* Here the timer needs to be inserted. */
			break ;
		}
		Delta += pTm->TmDelta ;
	}

	/* insert in queue */
	*ppTimPrev = pTimer ;
	pTimer->TmNext = pTm ;
	pTimer->TmDelta = Time - Delta ;

	if (pTm) {
		/* There is a next timer
		 * -> correct its Delta value.
		 */
		pTm->TmDelta -= pTimer->TmDelta ;
	}

	/*
	 * start new with first
	 */
	SkHwtStart(pAC,Ioc,pAC->Tim.StQueue->TmDelta) ;
}


void	SkTimerDone(
SK_AC	*pAC,		/* Adapters context */
SK_IOC	Ioc)		/* IoContext */
{
	timer_done(pAC,Ioc,1) ;
}


static void	timer_done(
SK_AC	*pAC,		/* Adapters context */
SK_IOC	Ioc,		/* IoContext */
int	Restart)	/* Do we need to restart the Hardware timer ? */
{
	SK_U32		Delta ;
	SK_TIMER	*pTm ;
	SK_TIMER	*pTComp ;	/* Timer completed now now */
	SK_TIMER	**ppLast ;	/* Next field of Last timer to be deq */
	int		Done = 0 ;

	Delta = SkHwtRead(pAC,Ioc) ;
	ppLast = &pAC->Tim.StQueue ;
	pTm = pAC->Tim.StQueue ;
	while (pTm && !Done) {
		if (Delta >= pTm->TmDelta) {
			/* Timer ran out */
			pTm->TmActive = SK_FALSE ;
			Delta -= pTm->TmDelta ;
			ppLast = &pTm->TmNext ;
			pTm = pTm->TmNext ;
		} else {
			/* We found the first timer that did not run out */
			pTm->TmDelta -= Delta ;
			Delta = 0 ;
			Done = 1 ;
		}
	}
	*ppLast = 0 ;
	/*
	 * pTm points to the first Timer that did not run out.
	 * StQueue points to the first Timer that run out.
	 */

	for ( pTComp = pAC->Tim.StQueue ; pTComp ; pTComp = pTComp->TmNext) {
		SkEventQueue(pAC,pTComp->TmClass, pTComp->TmEvent,
			pTComp->TmPara) ;
	}

	/* Set head of timer queue to the first timer that did not run out */
	pAC->Tim.StQueue = pTm ;

	if (Restart && pAC->Tim.StQueue) {
		/* Restart HW timer */
		SkHwtStart(pAC,Ioc,pAC->Tim.StQueue->TmDelta) ;
	}
}

/* End of file */
