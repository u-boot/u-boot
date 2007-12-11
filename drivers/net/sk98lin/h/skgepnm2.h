/*****************************************************************************
 *
 * Name:	skgepnm2.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.34 $
 * Date:	$Date: 2002/12/16 09:05:18 $
 * Purpose:	Defines for Private Network Management Interface
 *
 ****************************************************************************/

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

/*****************************************************************************
 *
 * History:
 *
 *	$Log: skgepnm2.h,v $
 *	Revision 1.34  2002/12/16 09:05:18  tschilli
 *	Code for VCT handling added.
 *
 *	Revision 1.33  2002/09/10 09:00:03  rwahl
 *	Adapted boolean definitions according sktypes.
 *
 *	Revision 1.32  2002/08/09 09:47:01  rwahl
 *	Added write-only flag to oid access defines.
 *	Editorial changes.
 *
 *	Revision 1.31  2002/07/17 19:23:18  rwahl
 *	- Replaced MAC counter definitions by enumeration.
 *	- Added definition SK_PNMI_MAC_TYPES.
 *	- Added chipset defnition for Yukon.
 *
 *	Revision 1.30  2001/02/06 10:03:41  mkunz
 *	- Pnmi V4 dual net support added. Interface functions and macros extended
 *	- Vpd bug fixed
 *	- OID_SKGE_MTU added
 *
 *	Revision 1.29  2001/01/22 13:41:37  rassmann
 *	Supporting two nets on dual-port adapters.
 *
 *	Revision 1.28  2000/08/03 15:12:48  rwahl
 *	- Additional comment for MAC statistic data structure.
 *
 *	Revision 1.27  2000/08/01 16:10:18  rwahl
 *	- Added mac statistic data structure for StatRxLongFrame counter.
 *
 *	Revision 1.26  2000/03/31 13:51:34  rwahl
 *	Added SK_UPTR cast to offset calculation for PNMI struct fields;
 *	missing cast caused compiler warnings by Win64 compiler.
 *
 *	Revision 1.25  1999/11/22 13:57:41  cgoos
 *	Changed license header to GPL.
 *	Allowing overwrite for SK_PNMI_STORE/_READ defines.
 *
 *	Revision 1.24  1999/04/13 15:11:11  mhaveman
 *	Changed copyright.
 *
 *	Revision 1.23  1999/01/28 15:07:12  mhaveman
 *	Changed default threshold for port switches per hour from 10
 *	to 240 which means 4 switches per minute. This fits better
 *	the granularity of 32 for the port switch estimate
 *	counter.
 *
 *	Revision 1.22  1999/01/05 12:52:30  mhaveman
 *	Removed macro SK_PNMI_MICRO_SEC.
 *
 *	Revision 1.21  1999/01/05 12:50:34  mhaveman
 *	Enlarged macro definition SK_PNMI_HUNDREDS_SEC() so that no 64-bit
 *	arithmetic is necessary if SK_TICKS_PER_SEC is 100.
 *
 *	Revision 1.20  1998/12/09 14:02:53  mhaveman
 *	Defined macro SK_PNMI_DEF_RLMT_CHG_THRES for default port switch
 *	threshold.
 *
 *	Revision 1.19  1998/12/03 11:28:41  mhaveman
 *	Removed SK_PNMI_CHECKPTR macro.
 *
 *	Revision 1.18  1998/12/03 11:21:00  mhaveman
 *	-Added pointer check macro SK_PNMI_CHECKPTR
 *	-Added macros SK_PNMI_VPD_ARR_SIZE and SK_PNMI_VPD_STR_SIZE for
 *	 VPD key evaluation.
 *
 *	Revision 1.17  1998/11/20 13:20:33  mhaveman
 *	Fixed bug in SK_PNMI_SET_STAT macro. ErrorStatus was not correctly set.
 *
 *	Revision 1.16  1998/11/20 08:08:49  mhaveman
 *	Macro SK_PNMI_CHECKFLAGS has got a if clause.
 *
 *	Revision 1.15  1998/11/03 13:53:40  mhaveman
 *	Fixed alignment problem in macor SK_PNMI_SET_STAT macro.
 *
 *	Revision 1.14  1998/10/30 15:50:13  mhaveman
 *	Added macro SK_PNMI_MICRO_SEC()
 *
 *	Revision 1.13  1998/10/30 12:32:20  mhaveman
 *	Added forgotten cast in SK_PNMI_READ_U32 macro.
 *
 *	Revision 1.12  1998/10/29 15:40:26  mhaveman
 *	-Changed SK_PNMI_TRAP_SENSOR_LEN because SensorDescr has now
 *	 variable string length.
 *	-Defined SK_PNMI_CHECKFLAGS macro
 *
 *	Revision 1.11  1998/10/29 08:53:34  mhaveman
 *	Removed SK_PNMI_RLM_XXX table indexed because these counters need
 *	not been saved over XMAC resets.
 *
 *	Revision 1.10  1998/10/28 08:48:20  mhaveman
 *	-Added macros for storage according to alignment
 *	-Changed type of Instance to SK_U32 because of VPD
 *	-Removed trap structures. Not needed because of alignment problem
 *	-Changed type of Action form SK_U8 to int
 *
 *	Revision 1.9  1998/10/21 13:34:45  mhaveman
 *	Shit, mismatched calculation of SK_PNMI_HUNDREDS_SEC. Corrected.
 *
 *	Revision 1.8  1998/10/21 13:24:58  mhaveman
 *	Changed calculation of hundreds of seconds.
 *
 *	Revision 1.7  1998/10/20 07:31:41  mhaveman
 *	Made type changes to unsigned int where possible.
 *
 *	Revision 1.6  1998/09/04 17:04:05  mhaveman
 *	Added Sync counters to offset storage to provided settled values on
 *	port switch.
 *
 *	Revision 1.5  1998/09/04 12:45:35  mhaveman
 *	Removed dummies for SK_DRIVER_ macros. They should be added by driver
 *	writer in skdrv2nd.h.
 *
 *	Revision 1.4  1998/09/04 11:59:50  mhaveman
 *	Everything compiles now. Driver Macros for counting still missing.
 *
 *	Revision 1.3  1998/08/24 12:01:35  mhaveman
 *	Intermediate state.
 *
 *	Revision 1.2  1998/08/17 07:51:40  mhaveman
 *	Intermediate state.
 *
 *	Revision 1.1  1998/08/11 09:08:40  mhaveman
 *	Intermediate state.
 *
 ****************************************************************************/

#ifndef _SKGEPNM2_H_
#define _SKGEPNM2_H_

/*
 * General definitions
 */
#define SK_PNMI_CHIPSET_XMAC	1	/* XMAC11800FP */
#define SK_PNMI_CHIPSET_YUKON	2	/* YUKON */

#define	SK_PNMI_BUS_PCI		1	/* PCI bus*/

/*
 * Actions
 */
#define SK_PNMI_ACT_IDLE		1
#define SK_PNMI_ACT_RESET		2
#define SK_PNMI_ACT_SELFTEST	3
#define SK_PNMI_ACT_RESETCNT	4

/*
 * VPD releated defines
 */

#define SK_PNMI_VPD_RW		1
#define SK_PNMI_VPD_RO		2

#define SK_PNMI_VPD_OK			0
#define SK_PNMI_VPD_NOTFOUND	1
#define SK_PNMI_VPD_CUT			2
#define SK_PNMI_VPD_TIMEOUT		3
#define SK_PNMI_VPD_FULL		4
#define SK_PNMI_VPD_NOWRITE		5
#define SK_PNMI_VPD_FATAL		6

#define SK_PNMI_VPD_IGNORE	0
#define SK_PNMI_VPD_CREATE	1
#define SK_PNMI_VPD_DELETE	2


/*
 * RLMT related defines
 */
#define SK_PNMI_DEF_RLMT_CHG_THRES	240	/* 4 changes per minute */


/*
 * VCT internal status values
 */
#define SK_PNMI_VCT_PENDING	32
#define SK_PNMI_VCT_TEST_DONE	64
#define SK_PNMI_VCT_LINK	128

/*
 * Internal table definitions
 */
#define SK_PNMI_GET		0
#define SK_PNMI_PRESET	1
#define SK_PNMI_SET		2

#define SK_PNMI_RO		0
#define SK_PNMI_RW		1
#define SK_PNMI_WO		2

typedef struct s_OidTabEntry {
	SK_U32			Id;
	SK_U32			InstanceNo;
	unsigned int	StructSize;
	unsigned int	Offset;
	int				Access;
	int				(* Func)(SK_AC *pAc, SK_IOC pIo, int action,
							 SK_U32 Id, char* pBuf, unsigned int* pLen,
							 SK_U32 Instance, unsigned int TableIndex,
							 SK_U32 NetNumber);
	SK_U16			Param;
} SK_PNMI_TAB_ENTRY;


/*
 * Trap lengths
 */
#define SK_PNMI_TRAP_SIMPLE_LEN			17
#define SK_PNMI_TRAP_SENSOR_LEN_BASE	46
#define SK_PNMI_TRAP_RLMT_CHANGE_LEN	23
#define SK_PNMI_TRAP_RLMT_PORT_LEN		23

/*
 * Number of MAC types supported
 */
#define SK_PNMI_MAC_TYPES	(SK_MAC_GMAC + 1)

/*
 * MAC statistic data list (overall set for MAC types used)
 */
enum SK_MACSTATS {
	SK_PNMI_HTX				= 0,
	SK_PNMI_HTX_OCTET,
	SK_PNMI_HTX_OCTETHIGH 	= SK_PNMI_HTX_OCTET,
	SK_PNMI_HTX_OCTETLOW,
	SK_PNMI_HTX_BROADCAST,
	SK_PNMI_HTX_MULTICAST,
	SK_PNMI_HTX_UNICAST,
	SK_PNMI_HTX_BURST,
	SK_PNMI_HTX_PMACC,
	SK_PNMI_HTX_MACC,
	SK_PNMI_HTX_COL,
	SK_PNMI_HTX_SINGLE_COL,
	SK_PNMI_HTX_MULTI_COL,
	SK_PNMI_HTX_EXCESS_COL,
	SK_PNMI_HTX_LATE_COL,
	SK_PNMI_HTX_DEFFERAL,
	SK_PNMI_HTX_EXCESS_DEF,
	SK_PNMI_HTX_UNDERRUN,
	SK_PNMI_HTX_CARRIER,
	SK_PNMI_HTX_UTILUNDER,
	SK_PNMI_HTX_UTILOVER,
	SK_PNMI_HTX_64,
	SK_PNMI_HTX_127,
	SK_PNMI_HTX_255,
	SK_PNMI_HTX_511,
	SK_PNMI_HTX_1023,
	SK_PNMI_HTX_MAX,
	SK_PNMI_HTX_LONGFRAMES,
	SK_PNMI_HTX_SYNC,
	SK_PNMI_HTX_SYNC_OCTET,
	SK_PNMI_HTX_RESERVED,

	SK_PNMI_HRX,
	SK_PNMI_HRX_OCTET,
	SK_PNMI_HRX_OCTETHIGH	= SK_PNMI_HRX_OCTET,
	SK_PNMI_HRX_OCTETLOW,
	SK_PNMI_HRX_BADOCTET,
	SK_PNMI_HRX_BADOCTETHIGH = SK_PNMI_HRX_BADOCTET,
	SK_PNMI_HRX_BADOCTETLOW,
	SK_PNMI_HRX_BROADCAST,
	SK_PNMI_HRX_MULTICAST,
	SK_PNMI_HRX_UNICAST,
	SK_PNMI_HRX_PMACC,
	SK_PNMI_HRX_MACC,
	SK_PNMI_HRX_PMACC_ERR,
	SK_PNMI_HRX_MACC_UNKWN,
	SK_PNMI_HRX_BURST,
	SK_PNMI_HRX_MISSED,
	SK_PNMI_HRX_FRAMING,
	SK_PNMI_HRX_UNDERSIZE,
	SK_PNMI_HRX_OVERFLOW,
	SK_PNMI_HRX_JABBER,
	SK_PNMI_HRX_CARRIER,
	SK_PNMI_HRX_IRLENGTH,
	SK_PNMI_HRX_SYMBOL,
	SK_PNMI_HRX_SHORTS,
	SK_PNMI_HRX_RUNT,
	SK_PNMI_HRX_TOO_LONG,
	SK_PNMI_HRX_FCS,
	SK_PNMI_HRX_CEXT,
	SK_PNMI_HRX_UTILUNDER,
	SK_PNMI_HRX_UTILOVER,
	SK_PNMI_HRX_64,
	SK_PNMI_HRX_127,
	SK_PNMI_HRX_255,
	SK_PNMI_HRX_511,
	SK_PNMI_HRX_1023,
	SK_PNMI_HRX_MAX,
	SK_PNMI_HRX_LONGFRAMES,

	SK_PNMI_HRX_RESERVED,

	SK_PNMI_MAX_IDX		/* NOTE: Ensure SK_PNMI_CNT_NO is set to this value */
};

/*
 * MAC specific data
 */
typedef struct s_PnmiStatAddr {
	SK_U16		Reg;		/* MAC register containing the value */
	SK_BOOL		GetOffset;	/* TRUE: Offset managed by PNMI (call GetStatVal())*/
} SK_PNMI_STATADDR;


/*
 * SK_PNMI_STRUCT_DATA copy offset evaluation macros
 */
#define SK_PNMI_OFF(e)		((SK_U32)(SK_UPTR)&(((SK_PNMI_STRUCT_DATA *)0)->e))
#define SK_PNMI_MAI_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_STRUCT_DATA *)0)->e))
#define SK_PNMI_VPD_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_VPD *)0)->e))
#define SK_PNMI_SEN_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_SENSOR *)0)->e))
#define SK_PNMI_CHK_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_CHECKSUM *)0)->e))
#define SK_PNMI_STA_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_STAT *)0)->e))
#define SK_PNMI_CNF_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_CONF *)0)->e))
#define SK_PNMI_RLM_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_RLMT *)0)->e))
#define SK_PNMI_MON_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_RLMT_MONITOR *)0)->e))
#define SK_PNMI_TRP_OFF(e)	((SK_U32)(SK_UPTR)&(((SK_PNMI_TRAP *)0)->e))

#define SK_PNMI_SET_STAT(b,s,o)	{SK_U32	Val32; char *pVal; \
					Val32 = (s); \
					pVal = (char *)(b) + ((SK_U32)(SK_UPTR) \
						&(((SK_PNMI_STRUCT_DATA *)0)-> \
						ReturnStatus.ErrorStatus)); \
					SK_PNMI_STORE_U32(pVal, Val32); \
					Val32 = (o); \
					pVal = (char *)(b) + ((SK_U32)(SK_UPTR) \
						&(((SK_PNMI_STRUCT_DATA *)0)-> \
						ReturnStatus.ErrorOffset)); \
					SK_PNMI_STORE_U32(pVal, Val32);}

/*
 * Time macros
 */
#if SK_TICKS_PER_SEC == 100
#define SK_PNMI_HUNDREDS_SEC(t)	(t)
#else
#define SK_PNMI_HUNDREDS_SEC(t)	(((t) * 100) / (SK_TICKS_PER_SEC))
#endif

/*
 * Macros to work around alignment problems
 */
#ifndef SK_PNMI_STORE_U16
#define SK_PNMI_STORE_U16(p,v)	{*(char *)(p) = *((char *)&(v)); \
					*((char *)(p) + 1) = \
						*(((char *)&(v)) + 1);}
#endif

#ifndef SK_PNMI_STORE_U32
#define SK_PNMI_STORE_U32(p,v)	{*(char *)(p) = *((char *)&(v)); \
					*((char *)(p) + 1) = \
						*(((char *)&(v)) + 1); \
					*((char *)(p) + 2) = \
						*(((char *)&(v)) + 2); \
					*((char *)(p) + 3) = \
						*(((char *)&(v)) + 3);}
#endif

#ifndef SK_PNMI_STORE_U64
#define SK_PNMI_STORE_U64(p,v)	{*(char *)(p) = *((char *)&(v)); \
					*((char *)(p) + 1) = \
						*(((char *)&(v)) + 1); \
					*((char *)(p) + 2) = \
						*(((char *)&(v)) + 2); \
					*((char *)(p) + 3) = \
						*(((char *)&(v)) + 3); \
					*((char *)(p) + 4) = \
						*(((char *)&(v)) + 4); \
					*((char *)(p) + 5) = \
						*(((char *)&(v)) + 5); \
					*((char *)(p) + 6) = \
						*(((char *)&(v)) + 6); \
					*((char *)(p) + 7) = \
						*(((char *)&(v)) + 7);}
#endif

#ifndef SK_PNMI_READ_U16
#define SK_PNMI_READ_U16(p,v)	{*((char *)&(v)) = *(char *)(p); \
					*(((char *)&(v)) + 1) = \
						*((char *)(p) + 1);}
#endif

#ifndef SK_PNMI_READ_U32
#define SK_PNMI_READ_U32(p,v)	{*((char *)&(v)) = *(char *)(p); \
					*(((char *)&(v)) + 1) = \
						*((char *)(p) + 1); \
					*(((char *)&(v)) + 2) = \
						*((char *)(p) + 2); \
					*(((char *)&(v)) + 3) = \
						*((char *)(p) + 3);}
#endif

#ifndef SK_PNMI_READ_U64
#define SK_PNMI_READ_U64(p,v)	{*((char *)&(v)) = *(char *)(p); \
					*(((char *)&(v)) + 1) = \
						*((char *)(p) + 1); \
					*(((char *)&(v)) + 2) = \
						*((char *)(p) + 2); \
					*(((char *)&(v)) + 3) = \
						*((char *)(p) + 3); \
					*(((char *)&(v)) + 4) = \
						*((char *)(p) + 4); \
					*(((char *)&(v)) + 5) = \
						*((char *)(p) + 5); \
					*(((char *)&(v)) + 6) = \
						*((char *)(p) + 6); \
					*(((char *)&(v)) + 7) = \
						*((char *)(p) + 7);}
#endif

/*
 * Macros for Debug
 */
#ifdef DEBUG

#define SK_PNMI_CHECKFLAGS(vSt)	{if (pAC->Pnmi.MacUpdatedFlag > 0 || \
					pAC->Pnmi.RlmtUpdatedFlag > 0 || \
					pAC->Pnmi.SirqUpdatedFlag > 0) { \
						SK_DBG_MSG(pAC, \
						SK_DBGMOD_PNMI, \
						SK_DBGCAT_CTRL,	\
						("PNMI: ERR: %s MacUFlag=%d, RlmtUFlag=%d, SirqUFlag=%d\n", \
						vSt, \
						pAC->Pnmi.MacUpdatedFlag, \
						pAC->Pnmi.RlmtUpdatedFlag, \
						pAC->Pnmi.SirqUpdatedFlag))}}

#else	/* !DEBUG */

#define SK_PNMI_CHECKFLAGS(vSt)	/* Nothing */

#endif	/* !DEBUG */

#endif	/* _SKGEPNM2_H_ */
