/******************************************************************************
 *
 * Name:	skgeinit.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.75 $
 * Date:	$Date: 2003/02/05 13:36:39 $
 * Purpose:	Structures and prototypes for the GE Init Module
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2003 SysKonnect GmbH.
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
 *	$Log: skgeinit.h,v $
 *	Revision 1.75  2003/02/05 13:36:39  rschmidt
 *	Added define SK_FACT_78	for YUKON's Host Clock of 78.12 MHz
 *	Editorial changes
 *
 *	Revision 1.74  2003/01/28 09:39:16  rschmidt
 *	Added entry GIYukonLite in s_GeInit structure
 *	Editorial changes
 *
 *	Revision 1.73  2002/11/15 12:47:25  rschmidt
 *	Replaced error message SKERR_HWI_E024 for Cable Diagnostic with
 *	Rx queue error in SkGeStopPort().
 *
 *	Revision 1.72  2002/11/12 17:08:35  rschmidt
 *	Added entries for Cable Diagnostic to Port structure
 *	Added entries GIPciSlot64 and GIPciClock66 in s_GeInit structure
 *	Added error message for Cable Diagnostic
 *	Added prototypes for SkGmCableDiagStatus()
 *	Editorial changes
 *
 *	Revision 1.71  2002/10/21 11:26:10  mkarl
 *	Changed interface of SkGeInitAssignRamToQueues().
 *
 *	Revision 1.70  2002/10/14 08:21:32  rschmidt
 *	Changed type of GICopperType, GIVauxAvail to SK_BOOL
 *	Added entry PRxOverCnt to Port structure
 *	Added entry GIYukon32Bit in s_GeInit structure
 *	Editorial changes
 *
 *	Revision 1.69  2002/10/09 16:57:15  mkarl
 *	Added some constants and macros for SkGeInitAssignRamToQueues().
 *
 *	Revision 1.68  2002/09/12 08:58:51  rwahl
 *	Retrieve counters needed for XMAC errata workarounds directly because
 *	PNMI returns corrected counter values (e.g. #10620).
 *
 *	Revision 1.67  2002/08/16 14:40:30  rschmidt
 *	Added entries GIGenesis and GICopperType in s_GeInit structure
 *	Added prototypes for SkMacHashing()
 *	Editorial changes
 *
 *	Revision 1.66  2002/08/12 13:27:21  rschmidt
 *	Added defines for Link speed capabilities
 *	Added entry PLinkSpeedCap to Port structure
 *	Added entry GIVauxAvail in s_GeInit structure
 *	Added prototypes for SkMacPromiscMode()
 *	Editorial changes
 *
 *	Revision 1.65  2002/08/08 15:46:18  rschmidt
 *	Added define SK_PHY_ACC_TO for PHY access timeout
 *	Added define SK_XM_RX_HI_WM for XMAC Rx High Watermark
 *	Added define SK_MIN_TXQ_SIZE for Min RAM Buffer Tx Queue Size
 *	Added entry PhyId1 to Port structure
 *
 *	Revision 1.64  2002/07/23 16:02:56  rschmidt
 *	Added entry GIWolOffs in s_GeInit struct (HW-Bug in YUKON 1st rev.)
 *	Added prototypes for: SkGePhyRead(), SkGePhyWrite()
 *
 *	Revision 1.63  2002/07/18 08:17:38  rwahl
 *	Corrected definitions for SK_LSPEED_xxx & SK_LSPEED_STAT_xxx.
 *
 *	Revision 1.62  2002/07/17 18:21:55  rwahl
 *	Added SK_LSPEED_INDETERMINATED define.
 *
 *	Revision 1.61  2002/07/17 17:16:03  rwahl
 *	- MacType now member of GIni struct.
 *	- Struct alignment to 32bit.
 *	- Editorial change.
 *
 *	Revision 1.60  2002/07/15 18:23:39  rwahl
 *	Added GeMacFunc to GE Init structure.
 *	Added prototypes for SkXmUpdateStats(), SkGmUpdateStats(),
 *	  SkXmMacStatistic(), SkGmMacStatistic(), SkXmResetCounter(),
 *	  SkGmResetCounter(), SkXmOverflowStatus(), SkGmOverflowStatus().
 *	Added defines for current link speed state.
 *	Added ERRMSG defintions for MacUpdateStat() & MacStatistics().
 *
 *	Revision 1.59  2002/07/15 15:40:22  rschmidt
 *	Added entry PLinkSpeedUsed to Port structure
 *	Editorial changes
 *
 *	Revision 1.58  2002/06/10 09:36:30  rschmidt
 *	Editorial changes.
 *
 *	Revision 1.57  2002/06/05 08:18:00  rschmidt
 *	Corrected alignment in Port Structure
 *	Added new prototypes for GMAC
 *	Editorial changes
 *
 *	Revision 1.56  2002/04/25 11:38:12  rschmidt
 *	Added defines for Link speed values
 *	Added defines for Loopback parameters for MAC and PHY
 *	Removed entry PRxCmd from Port structure
 *	Added entry PLinkSpeed to Port structure
 *	Added entries GIChipId and GIChipRev to GE Init structure
 *	Removed entry GIAnyPortAct from GE Init structure
 *	Added prototypes for: SkMacInit(), SkMacInitPhy(),
 *	SkMacRxTxDisable(), SkMacSoftRst(), SkMacHardRst(), SkMacIrq(),
 *	SkMacIrqDisable(), SkMacFlushTxFifo(), SkMacFlushRxFifo(),
 *	SkMacAutoNegDone(), SkMacAutoNegLipaPhy(), SkMacSetRxTxEn(),
 *	SkXmPhyRead(), SkXmPhyRead(), SkGmPhyWrite(), SkGmPhyWrite();
 *	Removed prototypes for static functions in SkXmac2.c
 *	Editorial changes
 *
 *	Revision 1.55  2002/02/26 15:24:53  rwahl
 *	Fix: no link with manual configuration (#10673). The previous fix for
 *	#10639 was removed. So for RLMT mode = CLS the RLMT may switch to
 *	misconfigured port. It should not occur for the other RLMT modes.
 *
 *	Revision 1.54  2002/01/18 16:52:52  rwahl
 *	Editorial corrections.
 *
 *	Revision 1.53  2001/11/20 09:19:58  rwahl
 *	Reworked bugfix #10639 (no dependency to RLMT mode).
 *
 *	Revision 1.52  2001/10/26 07:52:23  afischer
 *	Port switching bug in `check local link` mode
 *
 *	Revision 1.51  2001/02/09 12:26:38  cgoos
 *	Inserted #ifdef DIAG for half duplex workaround timer.
 *
 *	Revision 1.50  2001/02/07 07:56:40  rassmann
 *	Corrected copyright.
 *
 *	Revision 1.49  2001/01/31 15:32:18  gklug
 *	fix: problem with autosensing an SR8800 switch
 *	add: counter for autoneg timeouts
 *
 *	Revision 1.48  2000/11/09 11:30:10  rassmann
 *	WA: Waiting after releasing reset until BCom chip is accessible.
 *
 *	Revision 1.47  2000/10/18 12:22:40  cgoos
 *	Added workaround for half duplex hangup.
 *
 *	Revision 1.46  2000/08/10 11:28:00  rassmann
 *	Editorial changes.
 *	Preserving 32-bit alignment in structs for the adapter context.
 *
 *	Revision 1.45  1999/11/22 13:56:19  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.44  1999/10/26 07:34:15  malthoff
 *	The define SK_LNK_ON has been lost in v1.41.
 *
 *	Revision 1.43  1999/10/06 09:30:16  cgoos
 *	Changed SK_XM_THR_JUMBO.
 *
 *	Revision 1.42  1999/09/16 12:58:26  cgoos
 *	Changed SK_LED_STANDY macro to be independent of HW link sync.
 *
 *	Revision 1.41  1999/07/30 06:56:14  malthoff
 *	Correct comment for SK_MS_STAT_UNSET.
 *
 *	Revision 1.40  1999/05/27 13:38:46  cgoos
 *	Added SK_BMU_TX_WM.
 *	Made SK_BMU_TX_WM and SK_BMU_RX_WM user-definable.
 *	Changed XMAC Tx treshold to max. values.
 *
 *	Revision 1.39  1999/05/20 14:35:26  malthoff
 *	Remove prototypes for SkGeLinkLED().
 *
 *	Revision 1.38  1999/05/19 11:59:12  cgoos
 *	Added SK_MS_CAP_INDETERMINATED define.
 *
 *	Revision 1.37  1999/05/19 07:32:33  cgoos
 *	Changes for 1000Base-T.
 *	LED-defines for HWAC_LINK_LED macro.
 *
 *	Revision 1.36  1999/04/08 14:00:24  gklug
 *	add:Port struct field PLinkResCt
 *
 *	Revision 1.35  1999/03/25 07:43:07  malthoff
 *	Add error string for SKERR_HWI_E018MSG.
 *
 *	Revision 1.34  1999/03/12 16:25:57  malthoff
 *	Remove PPollRxD and PPollTxD.
 *	Add SKERR_HWI_E017MSG. and SK_DPOLL_MAX.
 *
 *	Revision 1.33  1999/03/12 13:34:41  malthoff
 *	Add Autonegotiation error codes.
 *	Change defines for parameter Mode in SkXmSetRxCmd().
 *	Replace __STDC__ by SK_KR_PROTO.
 *
 *	Revision 1.32  1999/01/25 14:40:20  mhaveman
 *	Added new return states for the virtual management port if multiple
 *	ports are active but differently configured.
 *
 *	Revision 1.31  1998/12/11 15:17:02  gklug
 *	add: Link partnet autoneg states : Unknown Manual and Auto-negotiation
 *
 *	Revision 1.30  1998/12/07 12:17:04  gklug
 *	add: Link Partner auto-negotiation flag
 *
 *	Revision 1.29  1998/12/01 10:54:42  gklug
 *	add: variables for XMAC Errata
 *
 *	Revision 1.28  1998/12/01 10:14:15  gklug
 *	add: PIsave saves the Interrupt status word
 *
 *	Revision 1.27  1998/11/26 15:24:52  mhaveman
 *	Added link status states SK_LMODE_STAT_AUTOHALF and
 *	SK_LMODE_STAT_AUTOFULL which are used by PNMI.
 *
 *	Revision 1.26  1998/11/26 14:53:01  gklug
 *	add:autoNeg Timeout variable
 *
 *	Revision 1.25  1998/11/26 08:58:50  gklug
 *	add: Link Mode configuration (AUTO Sense mode)
 *
 *	Revision 1.24  1998/11/24 13:30:27  gklug
 *	add: PCheckPar to port struct
 *
 *	Revision 1.23  1998/11/18 13:23:26  malthoff
 *	Add SK_PKT_TO_MAX.
 *
 *	Revision 1.22  1998/11/18 13:19:54  gklug
 *	add: PPrevShorts and PLinkBroken to port struct for WA XMAC Errata #C1
 *
 *	Revision 1.21  1998/10/26 08:02:57  malthoff
 *	Add GIRamOffs.
 *
 *	Revision 1.20  1998/10/19 07:28:37  malthoff
 *	Add prototype for SkGeInitRamIface().
 *
 *	Revision 1.19  1998/10/14 14:47:48  malthoff
 *	SK_TIMER should not be defined for Diagnostics.
 *	Add SKERR_HWI_E015MSG and SKERR_HWI_E016MSG.
 *
 *	Revision 1.18  1998/10/14 14:00:03  gklug
 *	add: timer to port struct for workaround of Errata #2
 *
 *	Revision 1.17  1998/10/14 11:23:09  malthoff
 *	Add prototype for SkXmAutoNegDone().
 *	Fix SkXmSetRxCmd() prototype statement.
 *
 *	Revision 1.16  1998/10/14 05:42:29  gklug
 *	add: HWLinkUp flag to Port struct
 *
 *	Revision 1.15  1998/10/09 08:26:33  malthoff
 *	Rename SK_RB_ULPP_B to SK_RB_LLPP_B.
 *
 *	Revision 1.14  1998/10/09 07:11:13  malthoff
 *	bug fix: SK_FACT_53 is 85 not 117.
 *	Rework time out init values.
 *	Add GIPortUsage and corresponding defines.
 *	Add some error log messages.
 *
 *	Revision 1.13  1998/10/06 14:13:14  malthoff
 *	Add prototype for SkGeLoadLnkSyncCnt().
 *
 *	Revision 1.12  1998/10/05 11:29:53  malthoff
 *	bug fix: A comment was not closed.
 *
 *	Revision 1.11  1998/10/05 08:01:59  malthoff
 *	Add default Timeout- Threshold- and
 *	Watermark constants. Add QRam start and end
 *	variables. Also add vars to store the polling
 *	mode and receive command. Add new Error Log
 *	Messages and function prototypes.
 *
 *	Revision 1.10  1998/09/28 13:34:48  malthoff
 *	Add mode bits for LED functions.
 *	Move Autoneg and Flow Ctrl bits from shgesirq.h
 *	Add the required Error Log Entries
 *	and Function Prototypes.
 *
 *	Revision 1.9  1998/09/16 14:38:41  malthoff
 *	Rework the SK_LNK_xxx defines.
 *	Add error log message defines.
 *	Add prototypes for skxmac2.c
 *
 *	Revision 1.8  1998/09/11 05:29:18  gklug
 *	add: init state of a port
 *
 *	Revision 1.7  1998/09/08 08:35:52  gklug
 *	add: defines of the Init Levels
 *
 *	Revision 1.6  1998/09/03 13:48:42  gklug
 *	add: Link strati, capabilities to Port struct
 *
 *	Revision 1.5  1998/09/03 13:30:59  malthoff
 *	Add SK_LNK_BLINK and SK_LNK_PERM.
 *
 *	Revision 1.4  1998/09/03 09:55:31  malthoff
 *	Add constants for parameters Dir and RstMode
 *	when calling SkGeStopPort().
 *	Rework the prototype section.
 *	Add Queue Address offsets PRxQOff, PXsQOff, and PXaQOff.
 *	Remove Ioc with IoC.
 *
 *	Revision 1.3  1998/08/19 09:11:54  gklug
 *	fix: struct are removed from c-source (see CCC)
 *	add: typedefs for all structs
 *
 *	Revision 1.2  1998/07/28 12:38:26  malthoff
 *	The prototypes got the parameter 'IoC'.
 *
 *	Revision 1.1  1998/07/23 09:50:24  malthoff
 *	Created.
 *
 ******************************************************************************/

#ifndef __INC_SKGEINIT_H_
#define __INC_SKGEINIT_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* defines ********************************************************************/

/* modifying Link LED behaviour (used with SkGeLinkLED()) */
#define SK_LNK_OFF		LED_OFF
#define SK_LNK_ON		(LED_ON | LED_BLK_OFF | LED_SYNC_OFF)
#define SK_LNK_BLINK	(LED_ON | LED_BLK_ON  | LED_SYNC_ON)
#define SK_LNK_PERM		(LED_ON | LED_BLK_OFF | LED_SYNC_ON)
#define SK_LNK_TST		(LED_ON | LED_BLK_ON  | LED_SYNC_OFF)

/* parameter 'Mode' when calling SK_HWAC_LINK_LED() */
#define SK_LED_OFF		LED_OFF
#define SK_LED_ACTIVE	(LED_ON | LED_BLK_OFF | LED_SYNC_OFF)
#define SK_LED_STANDBY	(LED_ON | LED_BLK_ON  | LED_SYNC_OFF)

/* addressing LED Registers in SkGeXmitLED() */
#define XMIT_LED_INI	0
#define XMIT_LED_CNT	(RX_LED_VAL - RX_LED_INI)
#define XMIT_LED_CTRL	(RX_LED_CTRL- RX_LED_INI)
#define XMIT_LED_TST	(RX_LED_TST - RX_LED_INI)

/* parameter 'Mode' when calling SkGeXmitLED() */
#define SK_LED_DIS	0
#define SK_LED_ENA	1
#define SK_LED_TST	2

/* Counter and Timer constants, for a host clock of 62.5 MHz */
#define SK_XMIT_DUR		0x002faf08L		/*  50 ms */
#define SK_BLK_DUR		0x01dcd650L		/* 500 ms */

#define SK_DPOLL_DEF	0x00ee6b28L		/* 250 ms at 62.5 MHz */

#define SK_DPOLL_MAX	0x00ffffffL		/* 268 ms at 62.5 MHz */
										/* 215 ms at 78.12 MHz */

#define SK_FACT_62		100			/* is given in percent */
#define SK_FACT_53		 85         /* on GENESIS:	53.12 MHz */
#define SK_FACT_78		125			/* on YUKON:	78.12 MHz */

/* Timeout values */
#define SK_MAC_TO_53	72			/* MAC arbiter timeout */
#define SK_PKT_TO_53	0x2000		/* Packet arbiter timeout */
#define SK_PKT_TO_MAX	0xffff		/* Maximum value */
#define SK_RI_TO_53		36			/* RAM interface timeout */

#define SK_PHY_ACC_TO	600000		/* PHY access timeout */

/* RAM Buffer High Pause Threshold values */
#define SK_RB_ULPP		( 8 * 1024)	/* Upper Level in kB/8 */
#define SK_RB_LLPP_S	(10 * 1024)	/* Lower Level for small Queues */
#define SK_RB_LLPP_B	(16 * 1024)	/* Lower Level for big Queues */

#ifndef SK_BMU_RX_WM
#define SK_BMU_RX_WM	0x600		/* BMU Rx Watermark */
#endif
#ifndef SK_BMU_TX_WM
#define SK_BMU_TX_WM	0x600		/* BMU Tx Watermark */
#endif

/* XMAC II Rx High Watermark */
#define SK_XM_RX_HI_WM	0x05aa		/* 1450 */

/* XMAC II Tx Threshold */
#define SK_XM_THR_REDL	0x01fb		/* .. for redundant link usage */
#define SK_XM_THR_SL	0x01fb		/* .. for single link adapters */
#define SK_XM_THR_MULL	0x01fb		/* .. for multiple link usage */
#define SK_XM_THR_JUMBO	0x03fc		/* .. for jumbo frame usage */

/* values for GIPortUsage */
#define SK_RED_LINK		1		/* redundant link usage */
#define SK_MUL_LINK		2		/* multiple link usage */
#define SK_JUMBO_LINK	3		/* driver uses jumbo frames */

/* Minimum RAM Buffer Rx Queue Size */
#define SK_MIN_RXQ_SIZE	16		/* 16 kB */

/* Minimum RAM Buffer Tx Queue Size */
#define SK_MIN_TXQ_SIZE	16		/* 16 kB */

/* Queue Size units */
#define QZ_UNITS		0x7
#define QZ_STEP			8

/* Percentage of queue size from whole memory */
/* 80 % for receive */
#define RAM_QUOTA_RX	80L
/* 0% for sync transfer */
#define	RAM_QUOTA_SYNC	0L
/* the rest (20%) is taken for async transfer */

/* Get the rounded queue size in Bytes in 8k steps */
#define ROUND_QUEUE_SIZE(SizeInBytes)					\
	((((unsigned long) (SizeInBytes) + (QZ_STEP*1024L)-1) / 1024) &	\
	~(QZ_STEP-1))

/* Get the rounded queue size in KBytes in 8k steps */
#define ROUND_QUEUE_SIZE_KB(Kilobytes) \
	ROUND_QUEUE_SIZE((Kilobytes) * 1024L)

/* Types of RAM Buffer Queues */
#define SK_RX_SRAM_Q	1	/* small receive queue */
#define SK_RX_BRAM_Q	2	/* big receive queue */
#define SK_TX_RAM_Q		3	/* small or big transmit queue */

/* parameter 'Dir' when calling SkGeStopPort() */
#define SK_STOP_TX	1	/* Stops the transmit path, resets the XMAC */
#define SK_STOP_RX	2	/* Stops the receive path */
#define SK_STOP_ALL	3	/* Stops Rx and Tx path, resets the XMAC */

/* parameter 'RstMode' when calling SkGeStopPort() */
#define SK_SOFT_RST	1	/* perform a software reset */
#define SK_HARD_RST	2	/* perform a hardware reset */

/* Init Levels */
#define SK_INIT_DATA	0	/* Init level 0: init data structures */
#define SK_INIT_IO		1	/* Init level 1: init with IOs */
#define SK_INIT_RUN		2	/* Init level 2: init for run time */

/* Link Mode Parameter */
#define SK_LMODE_HALF		1	/* Half Duplex Mode */
#define SK_LMODE_FULL		2	/* Full Duplex Mode */
#define SK_LMODE_AUTOHALF	3	/* AutoHalf Duplex Mode */
#define SK_LMODE_AUTOFULL	4	/* AutoFull Duplex Mode */
#define SK_LMODE_AUTOBOTH	5	/* AutoBoth Duplex Mode */
#define SK_LMODE_AUTOSENSE	6	/* configured mode auto sensing */
#define SK_LMODE_INDETERMINATED	7	/* indeterminated */

/* Auto-negotiation timeout in 100ms granularity */
#define SK_AND_MAX_TO		6	/* Wait 600 msec before link comes up */

/* Auto-negotiation error codes */
#define SK_AND_OK			0	/* no error */
#define SK_AND_OTHER		1	/* other error than below */
#define SK_AND_DUP_CAP		2	/* Duplex capabilities error */


/* Link Speed Capabilities */
#define SK_LSPEED_CAP_AUTO			(1<<0)	/* Automatic resolution */
#define SK_LSPEED_CAP_10MBPS		(1<<1)	/* 10 Mbps */
#define SK_LSPEED_CAP_100MBPS		(1<<2)	/* 100 Mbps */
#define SK_LSPEED_CAP_1000MBPS		(1<<3)	/* 1000 Mbps */
#define SK_LSPEED_CAP_INDETERMINATED (1<<4) /* indeterminated */

/* Link Speed Parameter */
#define SK_LSPEED_AUTO				1	/* Automatic resolution */
#define SK_LSPEED_10MBPS			2	/* 10 Mbps */
#define SK_LSPEED_100MBPS			3	/* 100 Mbps */
#define SK_LSPEED_1000MBPS			4	/* 1000 Mbps */
#define SK_LSPEED_INDETERMINATED	5	/* indeterminated */

/* Link Speed Current State */
#define SK_LSPEED_STAT_UNKNOWN		1
#define SK_LSPEED_STAT_10MBPS		2
#define SK_LSPEED_STAT_100MBPS 		3
#define SK_LSPEED_STAT_1000MBPS		4
#define SK_LSPEED_STAT_INDETERMINATED 5


/* Link Capability Parameter */
#define SK_LMODE_CAP_HALF		(1<<0)	/* Half Duplex Mode */
#define SK_LMODE_CAP_FULL		(1<<1)	/* Full Duplex Mode */
#define SK_LMODE_CAP_AUTOHALF	(1<<2)	/* AutoHalf Duplex Mode */
#define SK_LMODE_CAP_AUTOFULL	(1<<3)	/* AutoFull Duplex Mode */
#define SK_LMODE_CAP_INDETERMINATED (1<<4) /* indeterminated */

/* Link Mode Current State */
#define SK_LMODE_STAT_UNKNOWN	1	/* Unknown Duplex Mode */
#define SK_LMODE_STAT_HALF		2	/* Half Duplex Mode */
#define SK_LMODE_STAT_FULL		3	/* Full Duplex Mode */
#define SK_LMODE_STAT_AUTOHALF	4	/* Half Duplex Mode obtained by Auto-Neg */
#define SK_LMODE_STAT_AUTOFULL	5	/* Full Duplex Mode obtained by Auto-Neg */
#define SK_LMODE_STAT_INDETERMINATED 6	/* indeterminated */

/* Flow Control Mode Parameter (and capabilities) */
#define SK_FLOW_MODE_NONE		1	/* No Flow-Control */
#define SK_FLOW_MODE_LOC_SEND	2	/* Local station sends PAUSE */
#define SK_FLOW_MODE_SYMMETRIC	3	/* Both stations may send PAUSE */
#define SK_FLOW_MODE_SYM_OR_REM	4	/* Both stations may send PAUSE or
					 * just the remote station may send PAUSE
					 */
#define SK_FLOW_MODE_INDETERMINATED 5	/* indeterminated */

/* Flow Control Status Parameter */
#define SK_FLOW_STAT_NONE		1	/* No Flow Control */
#define SK_FLOW_STAT_REM_SEND	2	/* Remote Station sends PAUSE */
#define SK_FLOW_STAT_LOC_SEND	3	/* Local station sends PAUSE */
#define SK_FLOW_STAT_SYMMETRIC	4	/* Both station may send PAUSE */
#define SK_FLOW_STAT_INDETERMINATED 5	/* indeterminated */

/* Master/Slave Mode Capabilities */
#define SK_MS_CAP_AUTO		(1<<0)	/* Automatic resolution */
#define SK_MS_CAP_MASTER	(1<<1)	/* This station is master */
#define SK_MS_CAP_SLAVE		(1<<2)	/* This station is slave */
#define SK_MS_CAP_INDETERMINATED (1<<3)	/* indeterminated */

/* Set Master/Slave Mode Parameter (and capabilities) */
#define SK_MS_MODE_AUTO		1	/* Automatic resolution */
#define SK_MS_MODE_MASTER	2	/* This station is master */
#define SK_MS_MODE_SLAVE	3	/* This station is slave */
#define SK_MS_MODE_INDETERMINATED 4	/* indeterminated */

/* Master/Slave Status Parameter */
#define SK_MS_STAT_UNSET	1	/* The M/S status is not set */
#define SK_MS_STAT_MASTER	2	/* This station is Master */
#define SK_MS_STAT_SLAVE	3	/* This station is Dlave */
#define SK_MS_STAT_FAULT	4	/* M/S resolution failed */
#define SK_MS_STAT_INDETERMINATED 5	/* indeterminated */

/* parameter 'Mode' when calling SkXmSetRxCmd() */
#define SK_STRIP_FCS_ON		(1<<0)	/* Enable  FCS stripping of Rx frames */
#define SK_STRIP_FCS_OFF	(1<<1)	/* Disable FCS stripping of Rx frames */
#define SK_STRIP_PAD_ON		(1<<2)	/* Enable  pad byte stripping of Rx fr */
#define SK_STRIP_PAD_OFF	(1<<3)	/* Disable pad byte stripping of Rx fr */
#define SK_LENERR_OK_ON		(1<<4)	/* Don't chk fr for in range len error */
#define SK_LENERR_OK_OFF	(1<<5)	/* Check frames for in range len error */
#define SK_BIG_PK_OK_ON		(1<<6)	/* Don't set Rx Error bit for big frames */
#define SK_BIG_PK_OK_OFF	(1<<7)	/* Set Rx Error bit for big frames */
#define SK_SELF_RX_ON		(1<<8)	/* Enable  Rx of own packets */
#define SK_SELF_RX_OFF		(1<<9)	/* Disable Rx of own packets */

/* parameter 'Para' when calling SkMacSetRxTxEn() */
#define SK_MAC_LOOPB_ON		(1<<0)	/* Enable  MAC Loopback Mode */
#define SK_MAC_LOOPB_OFF	(1<<1)	/* Disable MAC Loopback Mode */
#define SK_PHY_LOOPB_ON		(1<<2)	/* Enable  PHY Loopback Mode */
#define SK_PHY_LOOPB_OFF	(1<<3)	/* Disable PHY Loopback Mode */
#define SK_PHY_FULLD_ON		(1<<4)	/* Enable  GMII Full Duplex */
#define SK_PHY_FULLD_OFF	(1<<5)	/* Disable GMII Full Duplex */

/* States of PState */
#define SK_PRT_RESET	0	/* the port is reset */
#define SK_PRT_STOP		1	/* the port is stopped (similar to SW reset) */
#define SK_PRT_INIT		2	/* the port is initialized */
#define SK_PRT_RUN		3	/* the port has an active link */

/* Default receive frame limit for Workaround of XMAC Errata */
#define SK_DEF_RX_WA_LIM	SK_CONSTU64(100)

/* Link Partner Status */
#define SK_LIPA_UNKNOWN	0	/* Link partner is in unknown state */
#define SK_LIPA_MANUAL	1	/* Link partner is in detected manual state */
#define SK_LIPA_AUTO	2	/* Link partner is in auto-negotiation state */

/* Maximum Restarts before restart is ignored (3Com WA) */
#define SK_MAX_LRESTART	3	/* Max. 3 times the link is restarted */

/* Max. Auto-neg. timeouts before link detection in sense mode is reset */
#define SK_MAX_ANEG_TO	10	/* Max. 10 times the sense mode is reset */

/* structures *****************************************************************/

/*
 * MAC specific functions
 */
typedef struct s_GeMacFunc {
	int  (*pFnMacUpdateStats)(SK_AC *pAC, SK_IOC IoC, unsigned int Port);
	int  (*pFnMacStatistic)(SK_AC *pAC, SK_IOC IoC, unsigned int Port,
							SK_U16 StatAddr, SK_U32 *pVal);
	int  (*pFnMacResetCounter)(SK_AC *pAC, SK_IOC IoC, unsigned int Port);
	int  (*pFnMacOverflow)(SK_AC *pAC, SK_IOC IoC, unsigned int Port,
						   SK_U16 IStatus, SK_U64 *pVal);
} SK_GEMACFUNC;

/*
 * Port Structure
 */
typedef	struct s_GePort {
#ifndef SK_DIAG
	SK_TIMER	PWaTimer;	/* Workaround Timer */
	SK_TIMER	HalfDupChkTimer;
#endif /* SK_DIAG */
	SK_U32	PPrevShorts;	/* Previous short Counter checking */
	SK_U32	PPrevFcs;		/* Previous FCS Error Counter checking */
	SK_U64	PPrevRx;		/* Previous RxOk Counter checking */
	SK_U64	PRxLim;			/* Previous RxOk Counter checking */
	SK_U64	LastOctets;		/* For half duplex hang check */
	int		PLinkResCt;		/* Link Restart Counter */
	int		PAutoNegTimeOut;/* Auto-negotiation timeout current value */
	int		PAutoNegTOCt;	/* Auto-negotiation Timeout Counter */
	int		PRxQSize;		/* Port Rx Queue Size in kB */
	int		PXSQSize;		/* Port Synchronous  Transmit Queue Size in kB */
	int		PXAQSize;		/* Port Asynchronous Transmit Queue Size in kB */
	SK_U32	PRxQRamStart;	/* Receive Queue RAM Buffer Start Address */
	SK_U32	PRxQRamEnd;		/* Receive Queue RAM Buffer End Address */
	SK_U32	PXsQRamStart;	/* Sync Tx Queue RAM Buffer Start Address */
	SK_U32	PXsQRamEnd;		/* Sync Tx Queue RAM Buffer End Address */
	SK_U32	PXaQRamStart;	/* Async Tx Queue RAM Buffer Start Address */
	SK_U32	PXaQRamEnd;		/* Async Tx Queue RAM Buffer End Address */
	SK_U32	PRxOverCnt;		/* Receive Overflow Counter */
	int		PRxQOff;		/* Rx Queue Address Offset */
	int		PXsQOff;		/* Synchronous Tx Queue Address Offset */
	int		PXaQOff;		/* Asynchronous Tx Queue Address Offset */
	int		PhyType;		/* PHY used on this port */
	SK_U16	PhyId1;			/* PHY Id1 on this port */
	SK_U16	PhyAddr;		/* MDIO/MDC PHY address */
	SK_U16	PIsave;			/* Saved Interrupt status word */
	SK_U16	PSsave;			/* Saved PHY status word */
	SK_BOOL	PHWLinkUp;		/* The hardware Link is up (wiring) */
	SK_BOOL	PState;			/* Is port initialized ? */
	SK_BOOL	PLinkBroken;	/* Is Link broken ? */
	SK_BOOL	PCheckPar;		/* Do we check for parity errors ? */
	SK_BOOL	HalfDupTimerActive;
	SK_U8	PLinkCap;		/* Link Capabilities */
	SK_U8	PLinkModeConf;	/* Link Mode configured */
	SK_U8	PLinkMode;		/* Link Mode currently used */
	SK_U8	PLinkModeStatus;/* Link Mode Status */
	SK_U8	PLinkSpeedCap;	/* Link Speed Capabilities(10/100/1000 Mbps) */
	SK_U8	PLinkSpeed;		/* configured Link Speed (10/100/1000 Mbps) */
	SK_U8	PLinkSpeedUsed;	/* current Link Speed (10/100/1000 Mbps) */
	SK_U8	PFlowCtrlCap;	/* Flow Control Capabilities */
	SK_U8	PFlowCtrlMode;	/* Flow Control Mode */
	SK_U8	PFlowCtrlStatus;/* Flow Control Status */
	SK_U8	PMSCap;			/* Master/Slave Capabilities */
	SK_U8	PMSMode;		/* Master/Slave Mode */
	SK_U8	PMSStatus;		/* Master/Slave Status */
	SK_U8	PAutoNegFail;	/* Auto-negotiation fail flag */
	SK_U8	PLipaAutoNeg;	/* Auto-negotiation possible with Link Partner */
	SK_U8	PCableLen;		/* Cable Length */
	SK_U8	PMdiPairLen[4];	/* MDI[0..3] Pair Length */
	SK_U8	PMdiPairSts[4];	/* MDI[0..3] Pair Diagnostic Status */
} SK_GEPORT;

/*
 * Gigabit Ethernet Initialization Struct
 * (has to be included in the adapter context)
 */
typedef	struct s_GeInit {
	SK_U8		GIPciHwRev;		/* PCI HW Revision Number */
	SK_U8		GIChipId;		/* Chip Identification Number */
	SK_U8		GIChipRev;		/* Chip Revision Number */
	SK_BOOL		GIGenesis;		/* Genesis adapter ? */
	SK_BOOL		GICopperType;	/* Copper Type adapter ? */
	SK_BOOL		GIPciSlot64;	/* 64-bit PCI Slot */
	SK_BOOL		GIPciClock66;	/* 66 MHz PCI Clock */
	SK_BOOL		GIVauxAvail;	/* VAUX available (YUKON) */
	SK_BOOL		GIYukon32Bit;	/* 32-Bit YUKON adapter */
	SK_BOOL		GIYukonLite;	/* YUKON-Lite chip */
	int			GIMacsFound;	/* Number of MACs found on this adapter */
	int			GIMacType;		/* MAC Type used on this adapter */
	int			GIHstClkFact;	/* Host Clock Factor (62.5 / HstClk * 100) */
	int			GIPortUsage;	/* Driver Port Usage */
	int			GILevel;		/* Initialization Level completed */
	int			GIRamSize;		/* The RAM size of the adapter in kB */
	int			GIWolOffs;		/* WOL Register Offset (HW-Bug in Rev. A) */
	SK_U32		GIRamOffs;		/* RAM Address Offset for addr calculation */
	SK_U32		GIPollTimerVal;	/* Descr. Poll Timer Init Val (HstClk ticks) */
	SK_GEPORT	GP[SK_MAX_MACS];/* Port Dependent Information */
	SK_GEMACFUNC GIFunc;		/* MAC depedent functions */
} SK_GEINIT;

/*
 * Error numbers and messages for skxmac2.c and skgeinit.c
 */
#define SKERR_HWI_E001		(SK_ERRBASE_HWINIT)
#define SKERR_HWI_E001MSG	"SkXmClrExactAddr() has got illegal parameters"
#define SKERR_HWI_E002		(SKERR_HWI_E001+1)
#define SKERR_HWI_E002MSG	"SkGeInit(): Level 1 call missing"
#define SKERR_HWI_E003		(SKERR_HWI_E002+1)
#define SKERR_HWI_E003MSG	"SkGeInit() called with illegal init Level"
#define SKERR_HWI_E004		(SKERR_HWI_E003+1)
#define SKERR_HWI_E004MSG	"SkGeInitPort(): Queue Size illegal configured"
#define SKERR_HWI_E005		(SKERR_HWI_E004+1)
#define SKERR_HWI_E005MSG	"SkGeInitPort(): cannot init running ports"
#define SKERR_HWI_E006		(SKERR_HWI_E005+1)
#define SKERR_HWI_E006MSG	"SkGeMacInit(): PState does not match HW state"
#define SKERR_HWI_E007		(SKERR_HWI_E006+1)
#define SKERR_HWI_E007MSG	"SkXmInitDupMd() called with invalid Dup Mode"
#define SKERR_HWI_E008		(SKERR_HWI_E007+1)
#define SKERR_HWI_E008MSG	"SkXmSetRxCmd() called with invalid Mode"
#define SKERR_HWI_E009		(SKERR_HWI_E008+1)
#define SKERR_HWI_E009MSG	"SkGeCfgSync() called although PXSQSize zero"
#define SKERR_HWI_E010		(SKERR_HWI_E009+1)
#define SKERR_HWI_E010MSG	"SkGeCfgSync() called with invalid parameters"
#define SKERR_HWI_E011		(SKERR_HWI_E010+1)
#define SKERR_HWI_E011MSG	"SkGeInitPort(): Receive Queue Size too small"
#define SKERR_HWI_E012		(SKERR_HWI_E011+1)
#define SKERR_HWI_E012MSG	"SkGeInitPort(): invalid Queue Size specified"
#define SKERR_HWI_E013		(SKERR_HWI_E012+1)
#define SKERR_HWI_E013MSG	"SkGeInitPort(): cfg changed for running queue"
#define SKERR_HWI_E014		(SKERR_HWI_E013+1)
#define SKERR_HWI_E014MSG	"SkGeInitPort(): unknown GIPortUsage specified"
#define SKERR_HWI_E015		(SKERR_HWI_E014+1)
#define SKERR_HWI_E015MSG	"Illegal Link mode parameter"
#define SKERR_HWI_E016		(SKERR_HWI_E015+1)
#define SKERR_HWI_E016MSG	"Illegal Flow control mode parameter"
#define SKERR_HWI_E017		(SKERR_HWI_E016+1)
#define SKERR_HWI_E017MSG	"Illegal value specified for GIPollTimerVal"
#define SKERR_HWI_E018		(SKERR_HWI_E017+1)
#define SKERR_HWI_E018MSG	"FATAL: SkGeStopPort() does not terminate (Tx)"
#define SKERR_HWI_E019		(SKERR_HWI_E018+1)
#define SKERR_HWI_E019MSG	"Illegal Speed parameter"
#define SKERR_HWI_E020		(SKERR_HWI_E019+1)
#define SKERR_HWI_E020MSG	"Illegal Master/Slave parameter"
#define SKERR_HWI_E021		(SKERR_HWI_E020+1)
#define	SKERR_HWI_E021MSG	"MacUpdateStats(): cannot update statistic counter"
#define	SKERR_HWI_E022		(SKERR_HWI_E021+1)
#define	SKERR_HWI_E022MSG	"MacStatistic(): illegal statistic base address"
#define SKERR_HWI_E023		(SKERR_HWI_E022+1)
#define SKERR_HWI_E023MSG	"SkGeInitPort(): Transmit Queue Size too small"
#define SKERR_HWI_E024		(SKERR_HWI_E023+1)
#define SKERR_HWI_E024MSG	"FATAL: SkGeStopPort() does not terminate (Rx)"
#define SKERR_HWI_E025		(SKERR_HWI_E024+1)
#define SKERR_HWI_E025MSG	""

/* function prototypes ********************************************************/

#ifndef	SK_KR_PROTO

/*
 * public functions in skgeinit.c
 */
extern void	SkGePollRxD(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	PollRxD);

extern void	SkGePollTxD(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL PollTxD);

extern void	SkGeYellowLED(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		State);

extern int	SkGeCfgSync(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U32	IntTime,
	SK_U32	LimCount,
	int		SyncMode);

extern void	SkGeLoadLnkSyncCnt(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U32	CntVal);

extern void	SkGeStopPort(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Dir,
	int		RstMode);

extern int	SkGeInit(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Level);

extern void	SkGeDeInit(
	SK_AC	*pAC,
	SK_IOC	IoC);

extern int	SkGeInitPort(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkGeXmitLED(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Led,
	int		Mode);

extern void	SkGeInitRamIface(
	SK_AC	*pAC,
	SK_IOC	IoC);

extern int	SkGeInitAssignRamToQueues(
	SK_AC	*pAC,
	int		ActivePort,
	SK_BOOL	DualNet);

/*
 * public functions in skxmac2.c
 */
extern void SkMacRxTxDisable(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacSoftRst(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacHardRst(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkXmInitMac(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkGmInitMac(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void SkMacInitPhy(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	DoLoop);

extern void SkMacIrqDisable(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacFlushTxFifo(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacFlushRxFifo(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacIrq(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern int	SkMacAutoNegDone(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacAutoNegLipaPhy(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U16	IStatus);

extern void  SkMacSetRxTxEn(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Para);

extern int  SkMacRxTxEnable(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacPromiscMode(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);

extern void	SkMacHashing(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);

extern void	SkXmPhyRead(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	*pVal);

extern void	SkXmPhyWrite(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	Val);

extern void	SkGmPhyRead(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	*pVal);

extern void	SkGmPhyWrite(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	Val);

extern void	SkGePhyRead(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	*pVal);

extern void	SkGePhyWrite(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	Val);

extern void	SkXmClrExactAddr(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		StartNum,
	int		StopNum);

extern void	SkXmInitDupMd(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkXmInitPauseMd(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkXmAutoNegLipaXmac(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U16	IStatus);

extern int SkXmUpdateStats(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port);

extern int SkGmUpdateStats(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port);

extern int SkXmMacStatistic(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port,
	SK_U16	StatAddr,
	SK_U32	*pVal);

extern int SkGmMacStatistic(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port,
	SK_U16	StatAddr,
	SK_U32	*pVal);

extern int SkXmResetCounter(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port);

extern int SkGmResetCounter(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port);

extern int SkXmOverflowStatus(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port,
	SK_U16  IStatus,
	SK_U64	*pStatus);

extern int SkGmOverflowStatus(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port,
	SK_U16	MacStatus,
	SK_U64	*pStatus);

extern int SkGmCableDiagStatus(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	StartTest);

#ifdef SK_DIAG
extern void	SkMacSetRxCmd(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Mode);
extern void	SkMacCrcGener(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);
extern void	SkMacTimeStamp(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);
extern void	SkXmSendCont(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);
#endif /* SK_DIAG */

#else	/* SK_KR_PROTO */

/*
 * public functions in skgeinit.c
 */
extern void	SkGePollRxD();
extern void	SkGePollTxD();
extern void	SkGeYellowLED();
extern int	SkGeCfgSync();
extern void	SkGeLoadLnkSyncCnt();
extern void	SkGeStopPort();
extern int	SkGeInit();
extern void	SkGeDeInit();
extern int	SkGeInitPort();
extern void	SkGeXmitLED();
extern void	SkGeInitRamIface();
extern int	SkGeInitAssignRamToQueues();

/*
 * public functions in skxmac2.c
 */
extern void SkMacRxTxDisable();
extern void	SkMacSoftRst();
extern void	SkMacHardRst();
extern void SkMacInitPhy();
extern int  SkMacRxTxEnable();
extern void SkMacPromiscMode();
extern void SkMacHashing();
extern void SkMacIrqDisable();
extern void	SkMacFlushTxFifo();
extern void	SkMacFlushRxFifo();
extern void	SkMacIrq();
extern int	SkMacAutoNegDone();
extern void	SkMacAutoNegLipaPhy();
extern void SkMacSetRxTxEn();
extern void	SkGePhyRead();
extern void	SkGePhyWrite();
extern void	SkXmInitMac();
extern void	SkXmPhyRead();
extern void	SkXmPhyWrite();
extern void	SkGmInitMac();
extern void	SkGmPhyRead();
extern void	SkGmPhyWrite();
extern void	SkXmClrExactAddr();
extern void	SkXmInitDupMd();
extern void	SkXmInitPauseMd();
extern void	SkXmAutoNegLipaXmac();
extern int	SkXmUpdateStats();
extern int	SkGmUpdateStats();
extern int	SkXmMacStatistic();
extern int	SkGmMacStatistic();
extern int	SkXmResetCounter();
extern int	SkGmResetCounter();
extern int	SkXmOverflowStatus();
extern int	SkGmOverflowStatus();
extern int	SkGmCableDiagStatus();

#ifdef SK_DIAG
extern void	SkMacSetRxCmd();
extern void	SkMacCrcGener();
extern void	SkMacTimeStamp();
extern void	SkXmSendCont();
#endif /* SK_DIAG */

#endif	/* SK_KR_PROTO */

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __INC_SKGEINIT_H_ */
