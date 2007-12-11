/******************************************************************************
 *
 * Name:	skaddr.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.26 $
 * Date:	$Date: 2002/11/15 07:24:42 $
 * Purpose:	Header file for Address Management (MC, UC, Prom).
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2001 SysKonnect GmbH.
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
 *	$Log: skaddr.h,v $
 *	Revision 1.26  2002/11/15 07:24:42  tschilli
 *	SK_ADDR_EQUAL macro fixed.
 *
 *	Revision 1.25  2002/06/10 13:55:18  tschilli
 *	Changes for handling YUKON.
 *	All changes are internally and not visible to the programmer
 *	using this module.
 *
 *	Revision 1.24  2001/01/22 13:41:34  rassmann
 *	Supporting two nets on dual-port adapters.
 *
 *	Revision 1.23  2000/08/10 11:27:50  rassmann
 *	Editorial changes.
 *	Preserving 32-bit alignment in structs for the adapter context.
 *
 *	Revision 1.22  2000/08/07 11:10:40  rassmann
 *	Editorial changes.
 *
 *	Revision 1.21  2000/05/04 09:39:59  rassmann
 *	Editorial changes.
 *	Corrected multicast address hashing.
 *
 *	Revision 1.20  1999/11/22 13:46:14  cgoos
 *	Changed license header to GPL.
 *	Allowing overwrite for SK_ADDR_EQUAL.
 *
 *	Revision 1.19  1999/05/28 10:56:07  rassmann
 *	Editorial changes.
 *
 *	Revision 1.18  1999/04/06 17:22:04  rassmann
 *	Added private "ActivePort".
 *
 *	Revision 1.17  1999/01/14 16:18:19  rassmann
 *	Corrected multicast initialization.
 *
 *	Revision 1.16  1999/01/04 10:30:36  rassmann
 *	SkAddrOverride only possible after SK_INIT_IO phase.
 *
 *	Revision 1.15  1998/12/29 13:13:11  rassmann
 *	An address override is now preserved in the SK_INIT_IO phase.
 *	All functions return an int now.
 *	Extended parameter checking.
 *
 *	Revision 1.14  1998/11/24 12:39:45  rassmann
 *	Reserved multicast entry for BPDU address.
 *	13 multicast entries left for protocol.
 *
 *	Revision 1.13  1998/11/13 17:24:32  rassmann
 *	Changed return value of SkAddrOverride to int.
 *
 *	Revision 1.12  1998/11/13 16:56:19  rassmann
 *	Added macro SK_ADDR_COMPARE.
 *	Changed return type of SkAddrOverride to SK_BOOL.
 *
 *	Revision 1.11  1998/10/28 18:16:35  rassmann
 *	Avoiding I/Os before SK_INIT_RUN level.
 *	Aligning InexactFilter.
 *
 *	Revision 1.10  1998/10/22 11:39:10  rassmann
 *	Corrected signed/unsigned mismatches.
 *
 *	Revision 1.9  1998/10/15 15:15:49  rassmann
 *	Changed Flags Parameters from SK_U8 to int.
 *	Checked with lint.
 *
 *	Revision 1.8  1998/09/24 19:15:12  rassmann
 *	Code cleanup.
 *
 *	Revision 1.7  1998/09/18 20:22:13  rassmann
 *	Added HW access.
 *
 *	Revision 1.6  1998/09/04 19:40:20  rassmann
 *	Interface enhancements.
 *
 *	Revision 1.5  1998/09/04 12:40:57  rassmann
 *	Interface cleanup.
 *
 *	Revision 1.4  1998/09/04 12:14:13  rassmann
 *	Interface cleanup.
 *
 *	Revision 1.3  1998/09/02 16:56:40  rassmann
 *	Updated interface.
 *
 *	Revision 1.2  1998/08/27 14:26:09  rassmann
 *	Updated interface.
 *
 *	Revision 1.1  1998/08/21 08:31:08  rassmann
 *	First public version.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * Description:
 *
 * This module is intended to manage multicast addresses and promiscuous mode
 * on GEnesis adapters.
 *
 * Include File Hierarchy:
 *
 *	"skdrv1st.h"
 *	...
 *	"sktypes.h"
 *	"skqueue.h"
 *	"skaddr.h"
 *	...
 *	"skdrv2nd.h"
 *
 ******************************************************************************/

#ifndef __INC_SKADDR_H
#define __INC_SKADDR_H

#ifdef __cplusplus
#error C++ is not yet supported.
extern "C" {
#endif	/* cplusplus */

/* defines ********************************************************************/

#define SK_MAC_ADDR_LEN				6	/* Length of MAC address. */
#define	SK_MAX_ADDRS				14	/* #Addrs for exact match. */

/* ----- Common return values ----- */

#define SK_ADDR_SUCCESS				0	/* Function returned successfully. */
#define SK_ADDR_ILLEGAL_PORT			100	/* Port number too high. */
#define SK_ADDR_TOO_EARLY			101	/* Function called too early. */

/* ----- Clear/Add flag bits ----- */

#define SK_ADDR_PERMANENT			1	/* RLMT Address */

/* ----- Additional Clear flag bits ----- */

#define SK_MC_SW_ONLY				2	/* Do not update HW when clearing. */

/* ----- Override flag bits ----- */

#define SK_ADDR_LOGICAL_ADDRESS		0
#define SK_ADDR_VIRTUAL_ADDRESS		(SK_ADDR_LOGICAL_ADDRESS)	/* old */
#define SK_ADDR_PHYSICAL_ADDRESS	1
#define SK_ADDR_CLEAR_LOGICAL		2
#define SK_ADDR_SET_LOGICAL			4

/* ----- Override return values ----- */

#define SK_ADDR_OVERRIDE_SUCCESS	(SK_ADDR_SUCCESS)
#define SK_ADDR_DUPLICATE_ADDRESS	1
#define SK_ADDR_MULTICAST_ADDRESS	2

/* ----- Partitioning of excact match table ----- */

#define SK_ADDR_EXACT_MATCHES		16	/* #Exact match entries. */

#define SK_ADDR_FIRST_MATCH_RLMT	1
#define SK_ADDR_LAST_MATCH_RLMT		2
#define SK_ADDR_FIRST_MATCH_DRV		3
#define SK_ADDR_LAST_MATCH_DRV		(SK_ADDR_EXACT_MATCHES - 1)

/* ----- SkAddrMcAdd/SkAddrMcUpdate return values ----- */

#define SK_MC_FILTERING_EXACT		0	/* Exact filtering. */
#define SK_MC_FILTERING_INEXACT		1	/* Inexact filtering. */

/* ----- Additional SkAddrMcAdd return values ----- */

#define SK_MC_ILLEGAL_ADDRESS		2	/* Illegal address. */
#define SK_MC_ILLEGAL_PORT			3	/* Illegal port (not the active one). */
#define SK_MC_RLMT_OVERFLOW			4	/* Too many RLMT mc addresses. */

/* Promiscuous mode bits ----- */

#define SK_PROM_MODE_NONE			0	/* Normal receive. */
#define SK_PROM_MODE_LLC			1	/* Receive all LLC frames. */
#define SK_PROM_MODE_ALL_MC			2	/* Receive all multicast frames. */
/* #define SK_PROM_MODE_NON_LLC		4 */	/* Receive all non-LLC frames. */

/* Macros */

#if 0
#ifndef SK_ADDR_EQUAL
/*
 * "&" instead of "&&" allows better optimization on IA-64.
 * The replacement is safe here, as all bytes exist.
 */
#ifndef SK_ADDR_DWORD_COMPARE
#define SK_ADDR_EQUAL(A1,A2)	( \
	(((SK_U8 *)(A1))[5] == ((SK_U8 *)(A2))[5]) & \
	(((SK_U8 *)(A1))[4] == ((SK_U8 *)(A2))[4]) & \
	(((SK_U8 *)(A1))[3] == ((SK_U8 *)(A2))[3]) & \
	(((SK_U8 *)(A1))[2] == ((SK_U8 *)(A2))[2]) & \
	(((SK_U8 *)(A1))[1] == ((SK_U8 *)(A2))[1]) & \
	(((SK_U8 *)(A1))[0] == ((SK_U8 *)(A2))[0]))
#else	/* SK_ADDR_DWORD_COMPARE */
#define SK_ADDR_EQUAL(A1,A2)	( \
	(*(SK_U32 *)&(((SK_U8 *)(A1))[2]) == *(SK_U32 *)&(((SK_U8 *)(A2))[2])) & \
	(*(SK_U32 *)&(((SK_U8 *)(A1))[0]) == *(SK_U32 *)&(((SK_U8 *)(A2))[0])))
#endif	/* SK_ADDR_DWORD_COMPARE */
#endif	/* SK_ADDR_EQUAL */
#endif /* 0 */

#ifndef SK_ADDR_EQUAL
#ifndef SK_ADDR_DWORD_COMPARE
#define SK_ADDR_EQUAL(A1,A2)	( \
	(((SK_U8 *)(A1))[5] == ((SK_U8 *)(A2))[5]) & \
	(((SK_U8 *)(A1))[4] == ((SK_U8 *)(A2))[4]) & \
	(((SK_U8 *)(A1))[3] == ((SK_U8 *)(A2))[3]) & \
	(((SK_U8 *)(A1))[2] == ((SK_U8 *)(A2))[2]) & \
	(((SK_U8 *)(A1))[1] == ((SK_U8 *)(A2))[1]) & \
	(((SK_U8 *)(A1))[0] == ((SK_U8 *)(A2))[0]))
#else	/* SK_ADDR_DWORD_COMPARE */
#define SK_ADDR_EQUAL(A1,A2)	( \
	(*(SK_U16 *)&(((SK_U8 *)(A1))[4]) == *(SK_U16 *)&(((SK_U8 *)(A2))[4])) && \
	(*(SK_U32 *)&(((SK_U8 *)(A1))[0]) == *(SK_U32 *)&(((SK_U8 *)(A2))[0])))
#endif	/* SK_ADDR_DWORD_COMPARE */
#endif	/* SK_ADDR_EQUAL */

/* typedefs *******************************************************************/

typedef struct s_MacAddr {
	SK_U8	a[SK_MAC_ADDR_LEN];
} SK_MAC_ADDR;


/* SK_FILTER is used to ensure alignment of the filter. */
typedef union s_InexactFilter {
	SK_U8	Bytes[8];
	SK_U64	Val;	/* Dummy entry for alignment only. */
} SK_FILTER64;


typedef struct s_AddrNet SK_ADDR_NET;


typedef struct s_AddrPort {

/* ----- Public part (read-only) ----- */

	SK_MAC_ADDR	CurrentMacAddress;	/* Current physical MAC Address. */
	SK_MAC_ADDR	PermanentMacAddress;	/* Permanent physical MAC Address. */
	int		PromMode;		/* Promiscuous Mode. */

/* ----- Private part ----- */

	SK_MAC_ADDR	PreviousMacAddress;	/* Prev. phys. MAC Address. */
	SK_BOOL		CurrentMacAddressSet;	/* CurrentMacAddress is set. */
	SK_U8		Align01;

	SK_U32		FirstExactMatchRlmt;
	SK_U32		NextExactMatchRlmt;
	SK_U32		FirstExactMatchDrv;
	SK_U32		NextExactMatchDrv;
	SK_MAC_ADDR	Exact[SK_ADDR_EXACT_MATCHES];
	SK_FILTER64	InexactFilter;			/* For 64-bit hash register. */
	SK_FILTER64	InexactRlmtFilter;		/* For 64-bit hash register. */
	SK_FILTER64	InexactDrvFilter;		/* For 64-bit hash register. */
} SK_ADDR_PORT;


struct s_AddrNet {
/* ----- Public part (read-only) ----- */

	SK_MAC_ADDR		CurrentMacAddress;	/* Logical MAC Address. */
	SK_MAC_ADDR		PermanentMacAddress;	/* Logical MAC Address. */

/* ----- Private part ----- */

	SK_U32			ActivePort;		/* View of module ADDR. */
	SK_BOOL			CurrentMacAddressSet;	/* CurrentMacAddress is set. */
	SK_U8			Align01;
	SK_U16			Align02;
};


typedef struct s_Addr {

/* ----- Public part (read-only) ----- */

	SK_ADDR_NET		Net[SK_MAX_NETS];
	SK_ADDR_PORT	Port[SK_MAX_MACS];

/* ----- Private part ----- */
} SK_ADDR;

/* function prototypes ********************************************************/

#ifndef SK_KR_PROTO

/* Functions provided by SkAddr */

/* ANSI/C++ compliant function prototypes */

extern	int	SkAddrInit(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int	Level);

extern	int	SkAddrMcClear(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber,
	int	Flags);

extern	int	SkAddrXmacMcClear(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber,
	int	Flags);

extern	int	SkAddrGmacMcClear(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber,
	int	Flags);

extern	int	SkAddrMcAdd(
	SK_AC		*pAC,
	SK_IOC		IoC,
	SK_U32		PortNumber,
	SK_MAC_ADDR	*pMc,
	int		Flags);

extern	int	SkAddrXmacMcAdd(
	SK_AC		*pAC,
	SK_IOC		IoC,
	SK_U32		PortNumber,
	SK_MAC_ADDR	*pMc,
	int		Flags);

extern	int	SkAddrGmacMcAdd(
	SK_AC		*pAC,
	SK_IOC		IoC,
	SK_U32		PortNumber,
	SK_MAC_ADDR	*pMc,
	int		Flags);

extern	int	SkAddrMcUpdate(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber);

extern	int	SkAddrXmacMcUpdate(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber);

extern	int	SkAddrGmacMcUpdate(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber);

extern	int	SkAddrOverride(
	SK_AC		*pAC,
	SK_IOC		IoC,
	SK_U32		PortNumber,
	SK_MAC_ADDR	*pNewAddr,
	int		Flags);

extern	int	SkAddrPromiscuousChange(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber,
	int	NewPromMode);

extern	int	SkAddrXmacPromiscuousChange(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber,
	int	NewPromMode);

extern	int	SkAddrGmacPromiscuousChange(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	PortNumber,
	int	NewPromMode);

extern	int	SkAddrSwap(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U32	FromPortNumber,
	SK_U32	ToPortNumber);

#else	/* defined(SK_KR_PROTO)) */

/* Non-ANSI/C++ compliant function prototypes */

#error KR-style prototypes are not yet provided.

#endif	/* defined(SK_KR_PROTO)) */


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __INC_SKADDR_H */
