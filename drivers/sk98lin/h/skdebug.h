/******************************************************************************
 *
 * Name:	skdebug.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.12 $
 * Date:	$Date: 2002/07/15 15:37:13 $
 * Purpose:	SK specific DEBUG support
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
 *	$Log: skdebug.h,v $
 *	Revision 1.12  2002/07/15 15:37:13  rschmidt
 *	Power Management support
 *	Editorial changes
 *
 *	Revision 1.11  2002/04/25 11:04:39  rschmidt
 *	Editorial changes
 *
 *	Revision 1.10  1999/11/22 13:47:40  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.9  1999/09/14 14:02:43  rwahl
 *	Added SK_DBGMOD_PECP.
 *
 *	Revision 1.8  1998/11/25 08:31:54  gklug
 *	fix: no C++ comments allowed in common sources
 *
 *	Revision 1.7  1998/11/24 16:47:24  swolf
 *	Driver may now define its own SK_DBG_MSG() (eg. in "h/skdrv1st.h").
 *
 *	Revision 1.6  1998/10/28 10:23:55  rassmann
 *	ADDED SK_DBGMOD_ADDR.
 *
 *	Revision 1.5  1998/10/22 09:43:55  gklug
 *	add: CSUM module
 *
 *	Revision 1.4  1998/10/01 07:54:44  gklug
 *	add: PNMI debug module
 *
 *	Revision 1.3  1998/09/18 08:32:34  afischer
 *	Macros changed according ssr-spec.:
 *		SK_DBG_MODCHK -> SK_DBG_CHKMOD
 *		SK_DBG_CATCHK -> SK_DBG_CHKCAT
 *
 *	Revision 1.2  1998/07/03 14:38:25  malthoff
 *	Add category SK_DBGCAT_FATAL.
 *
 *	Revision 1.1  1998/06/19 13:39:01  malthoff
 *	created.
 *
 *
 ******************************************************************************/

#ifndef __INC_SKDEBUG_H
#define __INC_SKDEBUG_H

#ifdef	DEBUG
#ifndef SK_DBG_MSG
#define SK_DBG_MSG(pAC,comp,cat,arg) \
		if ( ((comp) & SK_DBG_CHKMOD(pAC)) && 	\
		      ((cat) & SK_DBG_CHKCAT(pAC)) ) { 	\
			SK_DBG_PRINTF arg ;		\
		}
#endif
#else
#define SK_DBG_MSG(pAC,comp,lev,arg)
#endif

/* PLS NOTE:
 * =========
 * Due to any restrictions of kernel printf routines do not use other
 * format identifiers as: %x %d %c %s .
 * Never use any combined format identifiers such as: %lx %ld in your
 * printf - argument (arg) because some OS specific kernel printfs may
 * only support some basic identifiers.
 */

/* Debug modules */

#define SK_DBGMOD_MERR	0x00000001L	/* general module error indication */
#define SK_DBGMOD_HWM	0x00000002L	/* Hardware init module */
#define SK_DBGMOD_RLMT	0x00000004L	/* RLMT module */
#define SK_DBGMOD_VPD	0x00000008L	/* VPD module */
#define SK_DBGMOD_I2C	0x00000010L	/* I2C module */
#define SK_DBGMOD_PNMI	0x00000020L	/* PNMI module */
#define SK_DBGMOD_CSUM	0x00000040L	/* CSUM module */
#define SK_DBGMOD_ADDR	0x00000080L	/* ADDR module */
#define SK_DBGMOD_PECP	0x00000100L	/* PECP module */
#define SK_DBGMOD_POWM	0x00000200L	/* Power Management module */

/* Debug events */

#define SK_DBGCAT_INIT	0x00000001L	/* module/driver initialization */
#define SK_DBGCAT_CTRL	0x00000002L	/* controlling devices */
#define SK_DBGCAT_ERR	0x00000004L	/* error handling paths */
#define SK_DBGCAT_TX	0x00000008L	/* transmit path */
#define SK_DBGCAT_RX	0x00000010L	/* receive path */
#define SK_DBGCAT_IRQ	0x00000020L	/* general IRQ handling */
#define SK_DBGCAT_QUEUE	0x00000040L	/* any queue management */
#define SK_DBGCAT_DUMP	0x00000080L	/* large data output e.g. hex dump */
#define SK_DBGCAT_FATAL	0x00000100L	/* fatal error */

#endif	/* __INC_SKDEBUG_H */
