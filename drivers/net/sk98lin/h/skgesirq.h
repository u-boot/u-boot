/******************************************************************************
 *
 * Name:	skgesirq.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.26 $
 * Date:	$Date: 2002/10/14 09:52:36 $
 * Purpose:	SK specific Gigabit Ethernet special IRQ functions
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect GmbH.
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
 *	$Log: skgesirq.h,v $
 *	Revision 1.26  2002/10/14 09:52:36  rschmidt
 *	Added SKERR_SIRQ_E023 and SKERR_SIRQ_E023 for GPHY (Yukon)
 *	Editorial changes
 *
 *	Revision 1.25  2002/07/15 18:15:52  rwahl
 *	Editorial changes.
 *
 *	Revision 1.24  2002/07/15 15:39:21  rschmidt
 *	Corrected define for SKERR_SIRQ_E022
 *	Editorial changes
 *
 *	Revision 1.23  2002/04/25 11:09:45  rschmidt
 *	Removed declarations for SkXmInitPhy(), SkXmRxTxEnable()
 *	Editorial changes
 *
 *	Revision 1.22  2000/11/09 11:30:10  rassmann
 *	WA: Waiting after releasing reset until BCom chip is accessible.
 *
 *	Revision 1.21  2000/10/18 12:22:40  cgoos
 *	Added workaround for half duplex hangup.
 *
 *	Revision 1.20  1999/12/06 10:00:44  cgoos
 *	Added SET event for role.
 *
 *	Revision 1.19  1999/11/22 13:58:26  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.18  1999/05/19 07:32:59  cgoos
 *	Changes for 1000Base-T.
 *
 *	Revision 1.17  1999/03/12 13:29:31  malthoff
 *	Move Autonegotiation Error Codes to skgeinit.h.
 *
 *	Revision 1.16  1999/03/08 10:11:28  gklug
 *	add: AutoNegDone return codes
 *
 *	Revision 1.15  1998/11/18 13:20:53  gklug
 *	add: different timeouts for active and non-active links
 *
 *	Revision 1.14  1998/11/04 07:18:14  cgoos
 *	Added prototype for SkXmRxTxEnable.
 *
 *	Revision 1.13  1998/10/21 05:52:23  gklug
 *	add: parameter DoLoop to InitPhy function
 *
 *	Revision 1.12  1998/10/19 06:45:03  cgoos
 *	Added prototype for SkXmInitPhy.
 *
 *	Revision 1.11  1998/10/15 14:34:10  gklug
 *	add: WA_TIME is 500 msec
 *
 *	Revision 1.10  1998/10/14 14:49:41  malthoff
 *	Remove err log defines E021 and E022. They are
 *	defined in skgeinit.h now.
 *
 *	Revision 1.9  1998/10/14 14:00:39  gklug
 *	add: error logs for init phys
 *
 *	Revision 1.8  1998/10/14 05:44:05  gklug
 *	add: E020
 *
 *	Revision 1.7  1998/10/02 06:24:58  gklug
 *	add: error messages
 *
 *	Revision 1.6  1998/10/01 07:54:45  gklug
 *	add: PNMI debug module
 *
 *	Revision 1.5  1998/09/28 13:36:31  malthoff
 *	Move the bit definitions for Autonegotiation
 *	and Flow Control to skgeinit.h.
 *
 *	Revision 1.4  1998/09/15 12:29:34  gklug
 *	add: error logs
 *
 *	Revision 1.3  1998/09/03 13:54:02  gklug
 *	add: function prototypes
 *
 *	Revision 1.2  1998/09/03 10:24:36  gklug
 *	add: Events send by PNMI
 *	add: parameter definition for Flow Control etc.
 *
 *	Revision 1.1  1998/08/27 11:50:27  gklug
 *	initial revision
 *
 *
 ******************************************************************************/

#ifndef _INC_SKGESIRQ_H_
#define _INC_SKGESIRQ_H_

/*
 * Define the Event the special IRQ/INI module can handle
 */
#define SK_HWEV_WATIM			1	/* Timeout for WA errata #2 XMAC */
#define SK_HWEV_PORT_START		2	/* Port Start Event by RLMT */
#define SK_HWEV_PORT_STOP		3	/* Port Stop Event by RLMT */
#define SK_HWEV_CLEAR_STAT		4	/* Clear Statistics by PNMI */
#define SK_HWEV_UPDATE_STAT		5	/* Update Statistics by PNMI */
#define SK_HWEV_SET_LMODE		6	/* Set Link Mode by PNMI */
#define SK_HWEV_SET_FLOWMODE	7	/* Set Flow Control Mode by PNMI */
#define SK_HWEV_SET_ROLE		8	/* Set Master/Slave (Role) by PNMI */
#define SK_HWEV_SET_SPEED		9	/* Set Link Speed by PNMI */
#define SK_HWEV_HALFDUP_CHK		10	/* Half Duplex Hangup Workaround */

#define SK_WA_ACT_TIME		(5000000L)	/* 5 sec */
#define SK_WA_INA_TIME		(100000L)	/* 100 msec */

#define SK_HALFDUP_CHK_TIME	(10000L)	/* 10 msec */

/*
 * Define the error numbers and messages
 */
#define SKERR_SIRQ_E001		(SK_ERRBASE_SIRQ+0)
#define SKERR_SIRQ_E001MSG	"Unknown event"
#define SKERR_SIRQ_E002		(SKERR_SIRQ_E001+1)
#define SKERR_SIRQ_E002MSG	"Packet timeout RX1"
#define SKERR_SIRQ_E003		(SKERR_SIRQ_E002+1)
#define SKERR_SIRQ_E003MSG	"Packet timeout RX2"
#define SKERR_SIRQ_E004		(SKERR_SIRQ_E003+1)
#define SKERR_SIRQ_E004MSG	"MAC 1 not correctly initialized"
#define SKERR_SIRQ_E005		(SKERR_SIRQ_E004+1)
#define SKERR_SIRQ_E005MSG	"MAC 2 not correctly initialized"
#define SKERR_SIRQ_E006		(SKERR_SIRQ_E005+1)
#define SKERR_SIRQ_E006MSG	"CHECK failure R1"
#define SKERR_SIRQ_E007		(SKERR_SIRQ_E006+1)
#define SKERR_SIRQ_E007MSG	"CHECK failure R2"
#define SKERR_SIRQ_E008		(SKERR_SIRQ_E007+1)
#define SKERR_SIRQ_E008MSG	"CHECK failure XS1"
#define SKERR_SIRQ_E009		(SKERR_SIRQ_E008+1)
#define SKERR_SIRQ_E009MSG	"CHECK failure XA1"
#define SKERR_SIRQ_E010		(SKERR_SIRQ_E009+1)
#define SKERR_SIRQ_E010MSG	"CHECK failure XS2"
#define SKERR_SIRQ_E011		(SKERR_SIRQ_E010+1)
#define SKERR_SIRQ_E011MSG	"CHECK failure XA2"
#define SKERR_SIRQ_E012		(SKERR_SIRQ_E011+1)
#define SKERR_SIRQ_E012MSG	"unexpected IRQ Master error"
#define SKERR_SIRQ_E013		(SKERR_SIRQ_E012+1)
#define SKERR_SIRQ_E013MSG	"unexpected IRQ Status error"
#define SKERR_SIRQ_E014		(SKERR_SIRQ_E013+1)
#define SKERR_SIRQ_E014MSG	"Parity error on RAM (read)"
#define SKERR_SIRQ_E015		(SKERR_SIRQ_E014+1)
#define SKERR_SIRQ_E015MSG	"Parity error on RAM (write)"
#define SKERR_SIRQ_E016		(SKERR_SIRQ_E015+1)
#define SKERR_SIRQ_E016MSG	"Parity error MAC 1"
#define SKERR_SIRQ_E017		(SKERR_SIRQ_E016+1)
#define SKERR_SIRQ_E017MSG	"Parity error MAC 2"
#define SKERR_SIRQ_E018		(SKERR_SIRQ_E017+1)
#define SKERR_SIRQ_E018MSG	"Parity error RX 1"
#define SKERR_SIRQ_E019		(SKERR_SIRQ_E018+1)
#define SKERR_SIRQ_E019MSG	"Parity error RX 2"
#define SKERR_SIRQ_E020		(SKERR_SIRQ_E019+1)
#define SKERR_SIRQ_E020MSG	"MAC transmit FIFO underrun"
#define SKERR_SIRQ_E021		(SKERR_SIRQ_E020+1)
#define SKERR_SIRQ_E021MSG	"Spurious TWSI interrupt"
#define SKERR_SIRQ_E022		(SKERR_SIRQ_E021+1)
#define SKERR_SIRQ_E022MSG	"Cable pair swap error"
#define SKERR_SIRQ_E023		(SKERR_SIRQ_E022+1)
#define SKERR_SIRQ_E023MSG	"Auto-negotiation error"
#define SKERR_SIRQ_E024		(SKERR_SIRQ_E023+1)
#define SKERR_SIRQ_E024MSG	"FIFO overflow error"

extern void SkGeSirqIsr(SK_AC *pAC, SK_IOC IoC, SK_U32 Istatus);
extern int  SkGeSirqEvent(SK_AC *pAC, SK_IOC IoC, SK_U32 Event, SK_EVPARA Para);
extern void SkHWLinkUp(SK_AC *pAC, SK_IOC IoC, int Port);
extern void SkHWLinkDown(SK_AC *pAC, SK_IOC IoC, int Port);

#endif	/* _INC_SKGESIRQ_H_ */
