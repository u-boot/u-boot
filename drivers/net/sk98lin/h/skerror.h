/******************************************************************************
 *
 * Name:	skerror.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.5 $
 * Date:	$Date: 2002/04/25 11:05:10 $
 * Purpose:	SK specific Error log support
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
 *	$Log: skerror.h,v $
 *	Revision 1.5  2002/04/25 11:05:10  rschmidt
 *	Editorial changes
 *
 *	Revision 1.4  1999/11/22 13:51:59  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.3  1999/09/14 14:04:42  rwahl
 *	Added error base SK_ERRBASE_PECP.
 *	Changed error base for driver.
 *
 *	Revision 1.2  1998/08/11 11:15:41  gklug
 *	chg: comments
 *
 *	Revision 1.1  1998/08/11 11:09:38  gklug
 *	add: error bases
 *	add: error Classes
 *	first version
 *
 *
 *
 ******************************************************************************/

#ifndef _INC_SKERROR_H_
#define _INC_SKERROR_H_

/*
 * Define Error Classes
 */
#define	SK_ERRCL_OTHER		(0)		/* Other error */
#define	SK_ERRCL_CONFIG		(1L<<0)	/* Configuration error */
#define	SK_ERRCL_INIT		(1L<<1)	/* Initialization error */
#define	SK_ERRCL_NORES		(1L<<2)	/* Out of Resources error */
#define	SK_ERRCL_SW			(1L<<3)	/* Internal Software error */
#define	SK_ERRCL_HW			(1L<<4)	/* Hardware Failure */
#define	SK_ERRCL_COMM		(1L<<5)	/* Communication error */


/*
 * Define Error Code Bases
 */
#define	SK_ERRBASE_RLMT		 100	/* Base Error number for RLMT */
#define	SK_ERRBASE_HWINIT	 200	/* Base Error number for HWInit */
#define	SK_ERRBASE_VPD		 300	/* Base Error number for VPD */
#define	SK_ERRBASE_PNMI		 400	/* Base Error number for PNMI */
#define	SK_ERRBASE_CSUM		 500	/* Base Error number for Checksum */
#define	SK_ERRBASE_SIRQ		 600	/* Base Error number for Special IRQ */
#define	SK_ERRBASE_I2C		 700	/* Base Error number for I2C module */
#define	SK_ERRBASE_QUEUE	 800	/* Base Error number for Scheduler */
#define	SK_ERRBASE_ADDR		 900	/* Base Error number for Address module */
#define SK_ERRBASE_PECP		1000    /* Base Error number for PECP */
#define	SK_ERRBASE_DRV		1100	/* Base Error number for Driver */

#endif	/* _INC_SKERROR_H_ */
