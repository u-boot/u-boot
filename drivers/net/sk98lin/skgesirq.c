/******************************************************************************
 *
 * Name:	skgesirq.c
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.83 $
 * Date:	$Date: 2003/02/05 15:10:59 $
 * Purpose:	Special IRQ module
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
 *	$Log: skgesirq.c,v $
 *	Revision 1.83  2003/02/05 15:10:59  rschmidt
 *	Fixed setting of PLinkSpeedUsed in SkHWLinkUp() when
 *	auto-negotiation is disabled.
 *	Editorial changes.
 *
 *	Revision 1.82  2003/01/29 13:34:33  rschmidt
 *	Added some typecasts to avoid compiler warnings.
 *
 *	Revision 1.81  2002/12/05 10:49:51  rschmidt
 *	Fixed missing Link Down Event for fiber (Bug Id #10768)
 *	Added reading of cable length when link is up
 *	Removed testing of unused error bits in PHY ISR
 *	Editorial changes.
 *
 *	Revision 1.80  2002/11/12 17:15:21  rschmidt
 *	Replaced SkPnmiGetVar() by ...MacStatistic() in SkMacParity().
 *	Editorial changes.
 *
 *	Revision 1.79  2002/10/14 15:14:51  rschmidt
 *	Changed clearing of IS_M1_PAR_ERR (MAC 1 Parity Error) in
 *	SkMacParity() depending on GIChipRev (HW-Bug #8).
 *	Added error messages for GPHY Auto-Negotiation Error and
 *	FIFO Overflow/Underrun in SkPhyIsrGmac().
 *	Editorial changes.
 *
 *	Revision 1.78  2002/10/10 15:54:29  mkarl
 *	changes for PLinkSpeedUsed
 *
 *	Revision 1.77  2002/09/12 08:58:51  rwahl
 *	Retrieve counters needed for XMAC errata workarounds directly because
 *	PNMI returns corrected counter values (e.g. #10620).
 *
 *	Revision 1.76  2002/08/16 15:21:54  rschmidt
 *	Replaced all if(GIChipId == CHIP_ID_GENESIS) with new entry GIGenesis.
 *	Replaced wrong 1st para pAC with IoC in SK_IN/OUT macros.
 *	Editorial changes.
 *
 *	Revision 1.75  2002/08/12 13:50:47  rschmidt
 *	Changed clearing of IS_M1_PAR_ERR (MAC 1 Parity Error) in
 *	SkMacParity() by GMF_CLI_TX_FC instead of GMF_CLI_TX_PE (HW-Bug #8).
 *	Added clearing of IS_IRQ_TIST_OV and IS_IRQ_SENSOR in SkGeHwErr().
 *	Corrected handling of Link Up and Auto-Negotiation Over for GPHY.
 *	in SkGePortCheckUpGmac().
 *	Editorial changes.
 *
 *	Revision 1.74  2002/08/08 16:17:04  rschmidt
 *	Added PhyType check for SK_HWEV_SET_ROLE event (copper only)
 *	Changed Link Up check reading PHY Specific Status (YUKON)
 *	Editorial changes
 *
 *	Revision 1.73  2002/07/15 18:36:53  rwahl
 *	Editorial changes.
 *
 *	Revision 1.72  2002/07/15 15:46:26  rschmidt
 *	Added new event: SK_HWEV_SET_SPEED
 *	Editorial changes
 *
 *	Revision 1.71  2002/06/10 09:34:19  rschmidt
 *	Editorial changes
 *
 *	Revision 1.70  2002/06/05 08:29:18  rschmidt
 *	SkXmRxTxEnable() replaced by SkMacRxTxEnable().
 *	Editorial changes.
 *
 *	Revision 1.69  2002/04/25 13:03:49  rschmidt
 *	Changes for handling YUKON.
 *	Use of #ifdef OTHER_PHY to eliminate code for unused Phy types.
 *	Replaced all XMAC-access macros by functions: SkMacRxTxDisable(),
 *	SkMacIrqDisable().
 *	Added handling for GMAC FIFO in SkMacParity().
 *	Replaced all SkXm...() functions with SkMac...() to handle also
 *	YUKON's GMAC.
 *	Macros for XMAC PHY access PHY_READ(), PHY_WRITE() replaced
 *	by functions SkXmPhyRead(), SkXmPhyWrite().
 *	Disabling all PHY interrupts moved to SkMacIrqDisable().
 *	Added handling for GPHY IRQ in SkGeSirqIsr().
 *	Removed status parameter from MAC IRQ handler SkMacIrq().
 *	Added SkGePortCheckUpGmac(), SkPhyIsrGmac() for GMAC.
 *	Editorial changes
 *
 *	Revision 1.68  2002/02/26 15:24:53  rwahl
 *	Fix: no link with manual configuration (#10673). The previous fix for
 *	#10639 was removed. So for RLMT mode = CLS the RLMT may switch to
 *	misconfigured port. It should not occur for the other RLMT modes.
 *
 *	Revision 1.67  2001/11/20 09:19:58  rwahl
 *	Reworked bugfix #10639 (no dependency to RLMT mode).
 *
 *	Revision 1.66  2001/10/26 07:52:53  afischer
 *	Port switching bug in `check local link` mode
 *
 *	Revision 1.65  2001/02/23 13:41:51  gklug
 *	fix: PHYS2INST should be used correctly for Dual Net operation
 *	chg: do no longer work with older PNMI
 *
 *	Revision 1.64  2001/02/15 11:27:04  rassmann
 *	Working with RLMT v1 if SK_MAX_NETS undefined.
 *
 *	Revision 1.63  2001/02/06 10:44:23  mkunz
 *	- NetIndex added to interface functions of pnmi V4 with dual net support
 *
 *	Revision 1.62  2001/01/31 15:31:41  gklug
 *	fix: problem with autosensing an SR8800 switch
 *
 *	Revision 1.61  2000/11/09 11:30:09  rassmann
 *	WA: Waiting after releasing reset until BCom chip is accessible.
 *
 *	Revision 1.60  2000/10/18 12:37:48  cgoos
 *	Reinserted the comment for version 1.56.
 *
 *	Revision 1.59  2000/10/18 12:22:20  cgoos
 *	Added workaround for half duplex hangup.
 *
 *	Revision 1.58  2000/09/28 13:06:04  gklug
 *	fix: BCom may NOT be touched if XMAC is in RESET state
 *
 *	Revision 1.57  2000/09/08 12:38:39  cgoos
 *	Added forgotten variable declaration.
 *
 *	Revision 1.56  2000/09/08 08:12:13  cgoos
 *	Changed handling of parity errors in SkGeHwErr (correct reset of error).
 *
 *	Revision 1.55  2000/06/19 08:36:25  cgoos
 *	Changed comment.
 *
 *	Revision 1.54  2000/05/22 08:45:57  malthoff
 *	Fix: #10523 is valid for all BCom PHYs.
 *
 *	Revision 1.53  2000/05/19 10:20:30  cgoos
 *	Removed Solaris debug output code.
 *
 *	Revision 1.52  2000/05/19 10:19:37  cgoos
 *	Added PHY state check in HWLinkDown.
 *	Move PHY interrupt code to IS_EXT_REG case in SkGeSirqIsr.
 *
 *	Revision 1.51  2000/05/18 05:56:20  cgoos
 *	Fixed typo.
 *
 *	Revision 1.50  2000/05/17 12:49:49  malthoff
 *	Fixes BCom link bugs (#10523).
 *
 *	Revision 1.49  1999/12/17 11:02:50  gklug
 *	fix: read PHY_STAT of Broadcom chip more often to assure good status
 *
 *	Revision 1.48  1999/12/06 10:01:17  cgoos
 *	Added SET function for Role.
 *
 *	Revision 1.47  1999/11/22 13:34:24  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.46  1999/09/16 10:30:07  cgoos
 *	Removed debugging output statement from Linux.
 *
 *	Revision 1.45  1999/09/16 07:32:55  cgoos
 *	Fixed dual-port copperfield bug (PHY_READ from resetted port).
 *	Removed some unused variables.
 *
 *	Revision 1.44  1999/08/03 15:25:04  cgoos
 *	Removed workaround for disabled interrupts in half duplex mode.
 *
 *	Revision 1.43  1999/08/03 14:27:58  cgoos
 *	Removed SENSE mode code from SkGePortCheckUpBcom.
 *
 *	Revision 1.42  1999/07/26 09:16:54  cgoos
 *	Added some typecasts to avoid compiler warnings.
 *
 *	Revision 1.41  1999/05/19 07:28:59  cgoos
 *	Changes for 1000Base-T.
 *
 *	Revision 1.40  1999/04/08 13:59:39  gklug
 *	fix: problem with 3Com switches endless RESTARTs
 *
 *	Revision 1.39  1999/03/08 10:10:52  gklug
 *	fix: AutoSensing did switch to next mode even if LiPa indicated offline
 *
 *	Revision 1.38  1999/03/08 09:49:03  gklug
 *	fix: Bug using pAC instead of IoC, causing AIX problems
 *	fix: change compare for Linux compiler bug workaround
 *
 *	Revision 1.37  1999/01/28 14:51:33  gklug
 *	fix: monitor for autosensing and extra RESETS the RX on wire counters
 *
 *	Revision 1.36  1999/01/22 09:19:55  gklug
 *	fix: Init DupMode and InitPauseMd are now called in RxTxEnable
 *
 *	Revision 1.35  1998/12/11 15:22:59  gklug
 *	chg: autosensing: check for receive if manual mode was guessed
 *	chg: simplified workaround for XMAC errata
 *	chg: wait additional 100 ms before link goes up.
 *	chg: autoneg timeout to 600 ms
 *	chg: restart autoneg even if configured to autonegotiation
 *
 *	Revision 1.34  1998/12/10 10:33:14  gklug
 *	add: more debug messages
 *	fix: do a new InitPhy if link went down (AutoSensing problem)
 *	chg: Check for zero shorts if link is NOT up
 *	chg: reset Port if link goes down
 *	chg: wait additional 100 ms when link comes up to check shorts
 *	fix: dummy read extended autoneg status to prevent link going down immediately
 *
 *	Revision 1.33  1998/12/07 12:18:29  gklug
 *	add: refinement of autosense mode: take into account the autoneg cap of LiPa
 *
 *	Revision 1.32  1998/12/07 07:11:21  gklug
 *	fix: compiler warning
 *
 *	Revision 1.31  1998/12/02 09:29:05  gklug
 *	fix: WA XMAC Errata: FCSCt check was not correct.
 *	fix: WA XMAC Errata: Prec Counter were NOT updated in case of short checks.
 *	fix: Clear Stat : now clears the Prev counters of all known Ports
 *
 *	Revision 1.30  1998/12/01 10:54:15  gklug
 *	dd: workaround for XMAC errata changed. Check RX count and CRC err Count, too.
 *
 *	Revision 1.29  1998/12/01 10:01:53  gklug
 *	fix: if MAC IRQ occurs during port down, this will be handled correctly
 *
 *	Revision 1.28  1998/11/26 16:22:11  gklug
 *	fix: bug in autosense if manual modes are used
 *
 *	Revision 1.27  1998/11/26 15:50:06  gklug
 *	fix: PNMI needs to set PLinkModeConf
 *
 *	Revision 1.26  1998/11/26 14:51:58  gklug
 *	add: AutoSensing functionalty
 *
 *	Revision 1.25  1998/11/26 07:34:37  gklug
 *	fix: Init PrevShorts when restarting port due to Link connection
 *
 *	Revision 1.24  1998/11/25 10:57:32  gklug
 *	fix: remove unreferenced local vars
 *
 *	Revision 1.23  1998/11/25 08:26:40  gklug
 *	fix: don't do a RESET on a starting or stopping port
 *
 *	Revision 1.22  1998/11/24 13:29:44  gklug
 *	add: Workaround for MAC parity errata
 *
 *	Revision 1.21  1998/11/18 15:31:06  gklug
 *	fix: lint bugs
 *
 *	Revision 1.20  1998/11/18 12:58:54  gklug
 *	fix: use PNMI query instead of hardware access
 *
 *	Revision 1.19  1998/11/18 12:54:55  gklug
 *	chg: add new workaround for XMAC Errata
 *	add: short event counter monitoring on active link too
 *
 *	Revision 1.18  1998/11/13 14:27:41  malthoff
 *	Bug Fix: Packet Arbiter Timeout was not cleared correctly
 *	for timeout on TX1 and TX2.
 *
 *	Revision 1.17  1998/11/04 07:01:59  cgoos
 *	Moved HW link poll sequence.
 *	Added call to SkXmRxTxEnable.
 *
 *	Revision 1.16  1998/11/03 13:46:03  gklug
 *	add: functionality of SET_LMODE and SET_FLOW_MODE
 *	fix: send RLMT LinkDown event when Port stop is given with LinkUp
 *
 *	Revision 1.15  1998/11/03 12:56:47  gklug
 *	fix: Needs more events
 *
 *	Revision 1.14  1998/10/30 07:36:35  gklug
 *	rmv: unnecessary code
 *
 *	Revision 1.13  1998/10/29 15:21:57  gklug
 *	add: Poll link feature for activating HW link
 *	fix: Deactivate HWLink when Port STOP is given
 *
 *	Revision 1.12  1998/10/28 07:38:57  cgoos
 *	Checking link status at begin of SkHWLinkUp.
 *
 *	Revision 1.11  1998/10/22 09:46:50  gklug
 *	fix SysKonnectFileId typo
 *
 *	Revision 1.10  1998/10/14 13:57:47  gklug
 *	add: Port start/stop event
 *
 *	Revision 1.9  1998/10/14 05:48:29  cgoos
 *	Added definition for Para.
 *
 *	Revision 1.8  1998/10/14 05:40:09  gklug
 *	add: Hardware Linkup signal used
 *
 *	Revision 1.7  1998/10/09 06:50:20  malthoff
 *	Remove ID_sccs by SysKonnectFileId.
 *
 *	Revision 1.6  1998/10/08 09:11:49  gklug
 *	add: clear IRQ commands
 *
 *	Revision 1.5  1998/10/02 14:27:35  cgoos
 *	Fixed some typos and wrong event names.
 *
 *	Revision 1.4  1998/10/02 06:24:17  gklug
 *	add: HW error function
 *	fix: OUT macros
 *
 *	Revision 1.3  1998/10/01 07:03:00  gklug
 *	add: ISR for the usual interrupt source register
 *
 *	Revision 1.2  1998/09/03 13:50:33  gklug
 *	add: function prototypes
 *
 *	Revision 1.1  1998/08/27 11:50:21  gklug
 *	initial revision
 *
 *
 *
 ******************************************************************************/

#include <config.h>

/*
 *	Special Interrupt handler
 *
 *	The following abstract should show how this module is included
 *	in the driver path:
 *
 *	In the ISR of the driver the bits for frame transmission complete and
 *	for receive complete are checked and handled by the driver itself.
 *	The bits of the slow path mask are checked after that and then the
 *	entry into the so-called "slow path" is prepared. It is an implementors
 *	decision whether this is executed directly or just scheduled by
 *	disabling the mask. In the interrupt service routine some events may be
 *	generated, so it would be a good idea to call the EventDispatcher
 *	right after this ISR.
 *
 *	The Interrupt source register of the adapter is NOT read by this module.
 *  SO if the drivers implementor needs a while loop around the
 *	slow data paths interrupt bits, he needs to call the SkGeSirqIsr() for
 *	each loop entered.
 *
 *	However, the MAC Interrupt status registers are read in a while loop.
 *
 */

static const char SysKonnectFileId[] =
	"$Id: skgesirq.c,v 1.83 2003/02/05 15:10:59 rschmidt Exp $" ;

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#include "h/skgepnmi.h"		/* PNMI Definitions */
#include "h/skrlmt.h"		/* RLMT Definitions */
#include "h/skdrv2nd.h"		/* Adapter Control and Driver specific Def. */

/* local function prototypes */
static int	SkGePortCheckUpXmac(SK_AC*, SK_IOC, int);
static int	SkGePortCheckUpBcom(SK_AC*, SK_IOC, int);
static int	SkGePortCheckUpGmac(SK_AC*, SK_IOC, int);
static void	SkPhyIsrBcom(SK_AC*, SK_IOC, int, SK_U16);
static void	SkPhyIsrGmac(SK_AC*, SK_IOC, int, SK_U16);
#ifdef OTHER_PHY
static int	SkGePortCheckUpLone(SK_AC*, SK_IOC, int);
static int	SkGePortCheckUpNat(SK_AC*, SK_IOC, int);
static void	SkPhyIsrLone(SK_AC*, SK_IOC, int, SK_U16);
#endif /* OTHER_PHY */

/*
 * array of Rx counter from XMAC which are checked
 * in AutoSense mode to check whether a link is not able to auto-negotiate.
 */
static const SK_U16 SkGeRxRegs[]= {
	XM_RXF_64B,
	XM_RXF_127B,
	XM_RXF_255B,
	XM_RXF_511B,
	XM_RXF_1023B,
	XM_RXF_MAX_SZ
} ;

#ifdef __C2MAN__
/*
 *	Special IRQ function
 *
 *	General Description:
 *
 */
intro()
{}
#endif

/* Define return codes of SkGePortCheckUp and CheckShort */
#define	SK_HW_PS_NONE		0	/* No action needed */
#define	SK_HW_PS_RESTART	1	/* Restart needed */
#define	SK_HW_PS_LINK		2	/* Link Up actions needed */

/******************************************************************************
 *
 *	SkHWInitDefSense() - Default Autosensing mode initialization
 *
 * Description: sets the PLinkMode for HWInit
 *
 * Returns: N/A
 */
static void SkHWInitDefSense(
SK_AC	*pAC,	/* adapter context */
SK_IOC	IoC,	/* IO context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */

	pPrt = &pAC->GIni.GP[Port];

	pPrt->PAutoNegTimeOut = 0;

	if (pPrt->PLinkModeConf != SK_LMODE_AUTOSENSE) {
		pPrt->PLinkMode = pPrt->PLinkModeConf;
		return;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("AutoSensing: First mode %d on Port %d\n",
		(int)SK_LMODE_AUTOFULL, Port));

	pPrt->PLinkMode = SK_LMODE_AUTOFULL;

	return;
}	/* SkHWInitDefSense */


/******************************************************************************
 *
 *	SkHWSenseGetNext() - Get Next Autosensing Mode
 *
 * Description: gets the appropriate next mode
 *
 * Note:
 *
 */
SK_U8 SkHWSenseGetNext(
SK_AC	*pAC,	/* adapter context */
SK_IOC	IoC,	/* IO context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */

	pPrt = &pAC->GIni.GP[Port];

	pPrt->PAutoNegTimeOut = 0;

	if (pPrt->PLinkModeConf != SK_LMODE_AUTOSENSE) {
		/* Leave all as configured */
		return(pPrt->PLinkModeConf);
	}

	if (pPrt->PLinkMode == SK_LMODE_AUTOFULL) {
		/* Return next mode AUTOBOTH */
		return(SK_LMODE_AUTOBOTH);
	}

	/* Return default autofull */
	return(SK_LMODE_AUTOFULL);
}	/* SkHWSenseGetNext */


/******************************************************************************
 *
 *	SkHWSenseSetNext() - Autosensing Set next mode
 *
 * Description:	sets the appropriate next mode
 *
 * Returns: N/A
 */
void SkHWSenseSetNext(
SK_AC	*pAC,		/* adapter context */
SK_IOC	IoC,		/* IO context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_U8	NewMode)	/* New Mode to be written in sense mode */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */

	pPrt = &pAC->GIni.GP[Port];

	pPrt->PAutoNegTimeOut = 0;

	if (pPrt->PLinkModeConf != SK_LMODE_AUTOSENSE) {
		return;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("AutoSensing: next mode %d on Port %d\n",
		(int)NewMode, Port));

	pPrt->PLinkMode = NewMode;

	return;
}	/* SkHWSenseSetNext */


/******************************************************************************
 *
 *	SkHWLinkDown() - Link Down handling
 *
 * Description: handles the hardware link down signal
 *
 * Returns: N/A
 */
void SkHWLinkDown(
SK_AC	*pAC,		/* adapter context */
SK_IOC	IoC,		/* IO context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */

	pPrt = &pAC->GIni.GP[Port];

	/* Disable all MAC interrupts */
	SkMacIrqDisable(pAC, IoC, Port);

	/* Disable Receiver and Transmitter */
	SkMacRxTxDisable(pAC, IoC, Port);

	/* Init default sense mode */
	SkHWInitDefSense(pAC, IoC, Port);

	if (!pPrt->PHWLinkUp) {
		return;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("Link down Port %d\n", Port));

	/* Set Link to DOWN */
	pPrt->PHWLinkUp = SK_FALSE;

	/* Reset Port stati */
	pPrt->PLinkModeStatus = SK_LMODE_STAT_UNKNOWN;
	pPrt->PFlowCtrlStatus = SK_FLOW_STAT_NONE;
	pPrt->PLinkSpeedUsed = SK_LSPEED_STAT_INDETERMINATED;

	/* Re-init Phy especially when the AutoSense default is set now */
	SkMacInitPhy(pAC, IoC, Port, SK_FALSE);

	/* GP0: used for workaround of Rev. C Errata 2 */

	/* Do NOT signal to RLMT */

	/* Do NOT start the timer here */
}	/* SkHWLinkDown */


/******************************************************************************
 *
 *	SkHWLinkUp() - Link Up handling
 *
 * Description: handles the hardware link up signal
 *
 * Returns: N/A
 */
void SkHWLinkUp(
SK_AC	*pAC,	/* adapter context */
SK_IOC	IoC,	/* IO context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PHWLinkUp) {
		/* We do NOT need to proceed on active link */
		return;
	}

	pPrt->PHWLinkUp = SK_TRUE;
	pPrt->PAutoNegFail = SK_FALSE;
	pPrt->PLinkModeStatus = SK_LMODE_STAT_UNKNOWN;

	if (pPrt->PLinkMode != SK_LMODE_AUTOHALF &&
	    pPrt->PLinkMode != SK_LMODE_AUTOFULL &&
	    pPrt->PLinkMode != SK_LMODE_AUTOBOTH) {
		/* Link is up and no Auto-negotiation should be done */

		/* Link speed should be the configured one */
		switch (pPrt->PLinkSpeed) {
		case SK_LSPEED_AUTO:
			/* default is 1000 Mbps */
		case SK_LSPEED_1000MBPS:
			pPrt->PLinkSpeedUsed = SK_LSPEED_STAT_1000MBPS;
			break;
		case SK_LSPEED_100MBPS:
			pPrt->PLinkSpeedUsed = SK_LSPEED_STAT_100MBPS;
			break;
		case SK_LSPEED_10MBPS:
			pPrt->PLinkSpeedUsed = SK_LSPEED_STAT_10MBPS;
			break;
		}

		/* Set Link Mode Status */
		if (pPrt->PLinkMode == SK_LMODE_FULL) {
			pPrt->PLinkModeStatus = SK_LMODE_STAT_FULL;
		}
		else {
			pPrt->PLinkModeStatus = SK_LMODE_STAT_HALF;
		}

		/* No flow control without auto-negotiation */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_NONE;

		/* enable Rx/Tx */
		SkMacRxTxEnable(pAC, IoC, Port);
	}
}	/* SkHWLinkUp */


/******************************************************************************
 *
 *	SkMacParity() - MAC parity workaround
 *
 * Description: handles MAC parity errors correctly
 *
 * Returns: N/A
 */
static void SkMacParity(
SK_AC	*pAC,	/* adapter context */
SK_IOC	IoC,	/* IO context */
int		Port)	/* Port Index of the port failed */
{
	SK_EVPARA	Para;
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	SK_U32		TxMax;		/* TxMax Counter */

	pPrt = &pAC->GIni.GP[Port];

	/* Clear IRQ Tx Parity Error */
	if (pAC->GIni.GIGenesis) {
		SK_OUT16(IoC, MR_ADDR(Port, TX_MFF_CTRL1), MFF_CLR_PERR);
	}
	else {
		/* HW-Bug #8: cleared by GMF_CLI_TX_FC instead of GMF_CLI_TX_PE */
		SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T),
			(SK_U8)((pAC->GIni.GIChipRev == 0) ? GMF_CLI_TX_FC : GMF_CLI_TX_PE));
	}

	if (pPrt->PCheckPar) {
		if (Port == MAC_1) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E016, SKERR_SIRQ_E016MSG);
		}
		else {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E017, SKERR_SIRQ_E017MSG);
		}
		Para.Para64 = Port;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = Port;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);

		return;
	}

	/* Check whether frames with a size of 1k were sent */
	if (pAC->GIni.GIGenesis) {
		/* Snap statistic counters */
		(void)SkXmUpdateStats(pAC, IoC, Port);

		(void)SkXmMacStatistic(pAC, IoC, Port, XM_TXF_MAX_SZ, &TxMax);
	}
	else {
		(void)SkGmMacStatistic(pAC, IoC, Port, GM_TXF_1518B, &TxMax);
	}

	if (TxMax > 0) {
		/* From now on check the parity */
		pPrt->PCheckPar = SK_TRUE;
	}
}	/* SkMacParity */


/******************************************************************************
 *
 *	SkGeHwErr() - Hardware Error service routine
 *
 * Description: handles all HW Error interrupts
 *
 * Returns: N/A
 */
static void SkGeHwErr(
SK_AC	*pAC,		/* adapter context */
SK_IOC	IoC,		/* IO context */
SK_U32	HwStatus)	/* Interrupt status word */
{
	SK_EVPARA	Para;
	SK_U16		Word;

	if ((HwStatus & (IS_IRQ_MST_ERR | IS_IRQ_STAT)) != 0) {
		/* PCI Errors occured */
		if ((HwStatus & IS_IRQ_STAT) != 0) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E013, SKERR_SIRQ_E013MSG);
		}
		else {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E012, SKERR_SIRQ_E012MSG);
		}

		/* Reset all bits in the PCI STATUS register */
		SK_IN16(IoC, PCI_C(PCI_STATUS), &Word);

		SK_OUT8(IoC, B2_TST_CTRL1, TST_CFG_WRITE_ON);
		SK_OUT16(IoC, PCI_C(PCI_STATUS), Word | PCI_ERRBITS);
		SK_OUT8(IoC, B2_TST_CTRL1, TST_CFG_WRITE_OFF);

		Para.Para64 = 0;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);
	}

	if (pAC->GIni.GIGenesis) {
		if ((HwStatus & IS_NO_STAT_M1) != 0) {
			/* Ignore it */
			/* This situation is also indicated in the descriptor */
			SK_OUT16(IoC, MR_ADDR(MAC_1, RX_MFF_CTRL1), MFF_CLR_INSTAT);
		}

		if ((HwStatus & IS_NO_STAT_M2) != 0) {
			/* Ignore it */
			/* This situation is also indicated in the descriptor */
			SK_OUT16(IoC, MR_ADDR(MAC_2, RX_MFF_CTRL1), MFF_CLR_INSTAT);
		}

		if ((HwStatus & IS_NO_TIST_M1) != 0) {
			/* Ignore it */
			/* This situation is also indicated in the descriptor */
			SK_OUT16(IoC, MR_ADDR(MAC_1, RX_MFF_CTRL1), MFF_CLR_INTIST);
		}

		if ((HwStatus & IS_NO_TIST_M2) != 0) {
			/* Ignore it */
			/* This situation is also indicated in the descriptor */
			SK_OUT16(IoC, MR_ADDR(MAC_2, RX_MFF_CTRL1), MFF_CLR_INTIST);
		}
	}
	else {	/* YUKON */
		/* This is necessary only for Rx timing measurements */
		if ((HwStatus & IS_IRQ_TIST_OV) != 0) {
			/* Clear Time Stamp Timer IRQ */
			SK_OUT8(IoC, GMAC_TI_ST_CTRL, (SK_U8)GMT_ST_CLR_IRQ);
		}

		if ((HwStatus & IS_IRQ_SENSOR) != 0) {
			/* Clear I2C IRQ */
			SK_OUT32(IoC, B2_I2C_IRQ, I2C_CLR_IRQ);
		}
	}

	if ((HwStatus & IS_RAM_RD_PAR) != 0) {
		SK_OUT16(IoC, B3_RI_CTRL, RI_CLR_RD_PERR);
		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E014, SKERR_SIRQ_E014MSG);
		Para.Para64 = 0;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);
	}

	if ((HwStatus & IS_RAM_WR_PAR) != 0) {
		SK_OUT16(IoC, B3_RI_CTRL, RI_CLR_WR_PERR);
		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E015, SKERR_SIRQ_E015MSG);
		Para.Para64 = 0;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);
	}

	if ((HwStatus & IS_M1_PAR_ERR) != 0) {
		SkMacParity(pAC, IoC, MAC_1);
	}

	if ((HwStatus & IS_M2_PAR_ERR) != 0) {
		SkMacParity(pAC, IoC, MAC_2);
	}

	if ((HwStatus & IS_R1_PAR_ERR) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_R1_CSR, CSR_IRQ_CL_P);

		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E018, SKERR_SIRQ_E018MSG);
		Para.Para64 = MAC_1;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_1;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((HwStatus & IS_R2_PAR_ERR) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_R2_CSR, CSR_IRQ_CL_P);

		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E019, SKERR_SIRQ_E019MSG);
		Para.Para64 = MAC_2;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_2;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}
}	/* SkGeHwErr */


/******************************************************************************
 *
 *	SkGeSirqIsr() - Special Interrupt Service Routine
 *
 * Description: handles all non data transfer specific interrupts (slow path)
 *
 * Returns: N/A
 */
void SkGeSirqIsr(
SK_AC	*pAC,		/* adapter context */
SK_IOC	IoC,		/* IO context */
SK_U32	Istatus)	/* Interrupt status word */
{
	SK_EVPARA	Para;
	SK_U32		RegVal32;	/* Read register value */
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	unsigned	Len;
	SK_U64		Octets;
	SK_U16		PhyInt;
	SK_U16		PhyIMsk;
	int			i;

	if ((Istatus & IS_HW_ERR) != 0) {
		/* read the HW Error Interrupt source */
		SK_IN32(IoC, B0_HWE_ISRC, &RegVal32);

		SkGeHwErr(pAC, IoC, RegVal32);
	}

	/*
	 * Packet Timeout interrupts
	 */
	/* Check whether MACs are correctly initialized */
	if (((Istatus & (IS_PA_TO_RX1 | IS_PA_TO_TX1)) != 0) &&
		pAC->GIni.GP[MAC_1].PState == SK_PRT_RESET) {
		/* MAC 1 was not initialized but Packet timeout occured */
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E004,
			SKERR_SIRQ_E004MSG);
	}

	if (((Istatus & (IS_PA_TO_RX2 | IS_PA_TO_TX2)) != 0) &&
	    pAC->GIni.GP[MAC_2].PState == SK_PRT_RESET) {
		/* MAC 2 was not initialized but Packet timeout occured */
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E005,
			SKERR_SIRQ_E005MSG);
	}

	if ((Istatus & IS_PA_TO_RX1) != 0) {
		/* Means network is filling us up */
		SK_ERR_LOG(pAC, SK_ERRCL_HW | SK_ERRCL_INIT, SKERR_SIRQ_E002,
			SKERR_SIRQ_E002MSG);
		SK_OUT16(IoC, B3_PA_CTRL, PA_CLR_TO_RX1);
	}

	if ((Istatus & IS_PA_TO_RX2) != 0) {
		/* Means network is filling us up */
		SK_ERR_LOG(pAC, SK_ERRCL_HW | SK_ERRCL_INIT, SKERR_SIRQ_E003,
			SKERR_SIRQ_E003MSG);
		SK_OUT16(IoC, B3_PA_CTRL, PA_CLR_TO_RX2);
	}

	if ((Istatus & IS_PA_TO_TX1) != 0) {

		pPrt = &pAC->GIni.GP[0];

		/* May be a normal situation in a server with a slow network */
		SK_OUT16(IoC, B3_PA_CTRL, PA_CLR_TO_TX1);

		/*
		 * workaround: if in half duplex mode, check for Tx hangup.
		 * Read number of TX'ed bytes, wait for 10 ms, then compare
		 * the number with current value. If nothing changed, we assume
		 * that Tx is hanging and do a FIFO flush (see event routine).
		 */
		if ((pPrt->PLinkModeStatus == SK_LMODE_STAT_HALF ||
		    pPrt->PLinkModeStatus == SK_LMODE_STAT_AUTOHALF) &&
		    !pPrt->HalfDupTimerActive) {
			/*
			 * many more pack. arb. timeouts may come in between,
			 * we ignore those
			 */
			pPrt->HalfDupTimerActive = SK_TRUE;

			Len = sizeof(SK_U64);
			SkPnmiGetVar(pAC, IoC, OID_SKGE_STAT_TX_OCTETS, (char *)&Octets,
				&Len, (SK_U32) SK_PNMI_PORT_PHYS2INST(pAC, 0),
				pAC->Rlmt.Port[0].Net->NetNumber);

			pPrt->LastOctets = Octets;

			Para.Para32[0] = 0;
			SkTimerStart(pAC, IoC, &pPrt->HalfDupChkTimer, SK_HALFDUP_CHK_TIME,
				SKGE_HWAC, SK_HWEV_HALFDUP_CHK, Para);
		}
	}

	if ((Istatus & IS_PA_TO_TX2) != 0) {

		pPrt = &pAC->GIni.GP[1];

		/* May be a normal situation in a server with a slow network */
		SK_OUT16(IoC, B3_PA_CTRL, PA_CLR_TO_TX2);

		/* workaround: see above */
		if ((pPrt->PLinkModeStatus == SK_LMODE_STAT_HALF ||
		     pPrt->PLinkModeStatus == SK_LMODE_STAT_AUTOHALF) &&
		    !pPrt->HalfDupTimerActive) {
			pPrt->HalfDupTimerActive = SK_TRUE;

			Len = sizeof(SK_U64);
			SkPnmiGetVar(pAC, IoC, OID_SKGE_STAT_TX_OCTETS, (char *)&Octets,
				&Len, (SK_U32) SK_PNMI_PORT_PHYS2INST(pAC, 1),
				pAC->Rlmt.Port[1].Net->NetNumber);

			pPrt->LastOctets = Octets;

			Para.Para32[0] = 1;
			SkTimerStart(pAC, IoC, &pPrt->HalfDupChkTimer, SK_HALFDUP_CHK_TIME,
				SKGE_HWAC, SK_HWEV_HALFDUP_CHK, Para);
		}
	}

	/* Check interrupts of the particular queues */
	if ((Istatus & IS_R1_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_R1_CSR, CSR_IRQ_CL_C);
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E006,
			SKERR_SIRQ_E006MSG);
		Para.Para64 = MAC_1;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_1;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_R2_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_R2_CSR, CSR_IRQ_CL_C);
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E007,
			SKERR_SIRQ_E007MSG);
		Para.Para64 = MAC_2;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_2;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_XS1_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_XS1_CSR, CSR_IRQ_CL_C);
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E008,
			SKERR_SIRQ_E008MSG);
		Para.Para64 = MAC_1;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_1;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_XA1_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_XA1_CSR, CSR_IRQ_CL_C);
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E009,
			SKERR_SIRQ_E009MSG);
		Para.Para64 = MAC_1;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_1;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_XS2_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_XS2_CSR, CSR_IRQ_CL_C);
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E010,
			SKERR_SIRQ_E010MSG);
		Para.Para64 = MAC_2;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_2;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_XA2_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_XA2_CSR, CSR_IRQ_CL_C);
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E011,
			SKERR_SIRQ_E011MSG);
		Para.Para64 = MAC_2;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_2;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	/* External reg interrupt */
	if ((Istatus & IS_EXT_REG) != 0) {
		/* Test IRQs from PHY */
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

			pPrt = &pAC->GIni.GP[i];

			if (pPrt->PState == SK_PRT_RESET) {
				continue;
			}

			switch (pPrt->PhyType) {

			case SK_PHY_XMAC:
				break;

			case SK_PHY_BCOM:
				SkXmPhyRead(pAC, IoC, i, PHY_BCOM_INT_STAT, &PhyInt);
				SkXmPhyRead(pAC, IoC, i, PHY_BCOM_INT_MASK, &PhyIMsk);

				if ((PhyInt & ~PhyIMsk) != 0) {
					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
						("Port %d Bcom Int: 0x%04X Mask: 0x%04X\n",
						i, PhyInt, PhyIMsk));
					SkPhyIsrBcom(pAC, IoC, i, PhyInt);
				}
				break;

			case SK_PHY_MARV_COPPER:
			case SK_PHY_MARV_FIBER:
				SkGmPhyRead(pAC, IoC, i, PHY_MARV_INT_STAT, &PhyInt);
				SkGmPhyRead(pAC, IoC, i, PHY_MARV_INT_MASK, &PhyIMsk);

				if ((PhyInt & PhyIMsk) != 0) {
					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
						("Port %d Marv Int: 0x%04X Mask: 0x%04X\n",
						i, PhyInt, PhyIMsk));
					SkPhyIsrGmac(pAC, IoC, i, PhyInt);
				}
				break;

#ifdef OTHER_PHY
			case SK_PHY_LONE:
				SkXmPhyRead(pAC, IoC, i, PHY_LONE_INT_STAT, &PhyInt);
				SkXmPhyRead(pAC, IoC, i, PHY_LONE_INT_ENAB, &PhyIMsk);

				if ((PhyInt & PhyIMsk) != 0) {
					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
						("Port %d Lone Int: %x Mask: %x\n",
						i, PhyInt, PhyIMsk));
					SkPhyIsrLone(pAC, IoC, i, PhyInt);
				}
				break;
			case SK_PHY_NAT:
				/* todo: National */
				break;
#endif /* OTHER_PHY */
			}
		}
	}

	/* I2C Ready interrupt */
	if ((Istatus & IS_I2C_READY) != 0) {
		SkI2cIsr(pAC, IoC);
	}

	if ((Istatus & IS_LNK_SYNC_M1) != 0) {
		/*
		 * We do NOT need the Link Sync interrupt, because it shows
		 * us only a link going down.
		 */
		/* clear interrupt */
		SK_OUT8(IoC, MR_ADDR(MAC_1, LNK_SYNC_CTRL), LED_CLR_IRQ);
	}

	/* Check MAC after link sync counter */
	if ((Istatus & IS_MAC1) != 0) {
		/* IRQ from MAC 1 */
		SkMacIrq(pAC, IoC, MAC_1);
	}

	if ((Istatus & IS_LNK_SYNC_M2) != 0) {
		/*
		 * We do NOT need the Link Sync interrupt, because it shows
		 * us only a link going down.
		 */
		/* clear interrupt */
		SK_OUT8(IoC, MR_ADDR(MAC_2, LNK_SYNC_CTRL), LED_CLR_IRQ);
	}

	/* Check MAC after link sync counter */
	if ((Istatus & IS_MAC2) != 0) {
		/* IRQ from MAC 2 */
		SkMacIrq(pAC, IoC, MAC_2);
	}

	/* Timer interrupt (served last) */
	if ((Istatus & IS_TIMINT) != 0) {
		SkHwtIsr(pAC, IoC);
	}
}	/* SkGeSirqIsr */


/******************************************************************************
 *
 * SkGePortCheckShorts() - Implementing XMAC Workaround Errata # 2
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 */
static int	SkGePortCheckShorts(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* IO Context */
int		Port)		/* Which port should be checked */
{
	SK_U32		Shorts;			/* Short Event Counter */
	SK_U32		CheckShorts;	/* Check value for Short Event Counter */
	SK_U64		RxCts;			/* Rx Counter (packets on network) */
	SK_U32		RxTmp;			/* Rx temp. Counter */
	SK_U32		FcsErrCts;		/* FCS Error Counter */
	SK_GEPORT	*pPrt;			/* GIni Port struct pointer */
	int			Rtv;			/* Return value */
	int			i;

	pPrt = &pAC->GIni.GP[Port];

	/* Default: no action */
	Rtv = SK_HW_PS_NONE;

	(void)SkXmUpdateStats(pAC, IoC, Port);

	/* Extra precaution: check for short Event counter */
	(void)SkXmMacStatistic(pAC, IoC, Port, XM_RXE_SHT_ERR, &Shorts);

	/*
	 * Read Rx counter (packets seen on the network and not necessarily
	 * really received.
	 */
	RxCts = 0;

	for (i = 0; i < sizeof(SkGeRxRegs)/sizeof(SkGeRxRegs[0]); i++) {
		(void)SkXmMacStatistic(pAC, IoC, Port, SkGeRxRegs[i], &RxTmp);
		RxCts += (SK_U64)RxTmp;
	}

	/* On default: check shorts against zero */
	CheckShorts = 0;

	/* Extra precaution on active links */
	if (pPrt->PHWLinkUp) {
		/* Reset Link Restart counter */
		pPrt->PLinkResCt = 0;
		pPrt->PAutoNegTOCt = 0;

		/* If link is up check for 2 */
		CheckShorts = 2;

		(void)SkXmMacStatistic(pAC, IoC, Port, XM_RXF_FCS_ERR, &FcsErrCts);

		if (pPrt->PLinkModeConf == SK_LMODE_AUTOSENSE &&
		    pPrt->PLipaAutoNeg == SK_LIPA_UNKNOWN &&
		    (pPrt->PLinkMode == SK_LMODE_HALF ||
			 pPrt->PLinkMode == SK_LMODE_FULL)) {
			/*
			 * This is autosensing and we are in the fallback
			 * manual full/half duplex mode.
			 */
			if (RxCts == pPrt->PPrevRx) {
				/* Nothing received, restart link */
				pPrt->PPrevFcs = FcsErrCts;
				pPrt->PPrevShorts = Shorts;

				return(SK_HW_PS_RESTART);
			}
			else {
				pPrt->PLipaAutoNeg = SK_LIPA_MANUAL;
			}
		}

		if (((RxCts - pPrt->PPrevRx) > pPrt->PRxLim) ||
		    (!(FcsErrCts - pPrt->PPrevFcs))) {
			/*
			 * Note: The compare with zero above has to be done the way shown,
			 * otherwise the Linux driver will have a problem.
			 */
			/*
			 * We received a bunch of frames or no CRC error occured on the
			 * network -> ok.
			 */
			pPrt->PPrevRx = RxCts;
			pPrt->PPrevFcs = FcsErrCts;
			pPrt->PPrevShorts = Shorts;

			return(SK_HW_PS_NONE);
		}

		pPrt->PPrevFcs = FcsErrCts;
	}


	if ((Shorts - pPrt->PPrevShorts) > CheckShorts) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("Short Event Count Restart Port %d \n", Port));
		Rtv = SK_HW_PS_RESTART;
	}

	pPrt->PPrevShorts = Shorts;
	pPrt->PPrevRx = RxCts;

	return(Rtv);
}	/* SkGePortCheckShorts */


/******************************************************************************
 *
 * SkGePortCheckUp() - Check if the link is up
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 *	2	Link came up
 */
static int	SkGePortCheckUp(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* IO Context */
int		Port)		/* Which port should be checked */
{
	switch (pAC->GIni.GP[Port].PhyType) {
	case SK_PHY_XMAC:
		return(SkGePortCheckUpXmac(pAC, IoC, Port));
	case SK_PHY_BCOM:
		return(SkGePortCheckUpBcom(pAC, IoC, Port));
	case SK_PHY_MARV_COPPER:
	case SK_PHY_MARV_FIBER:
		return(SkGePortCheckUpGmac(pAC, IoC, Port));
#ifdef OTHER_PHY
	case SK_PHY_LONE:
		return(SkGePortCheckUpLone(pAC, IoC, Port));
	case SK_PHY_NAT:
		return(SkGePortCheckUpNat(pAC, IoC, Port));
#endif /* OTHER_PHY */
	}
	return(SK_HW_PS_NONE);
}	/* SkGePortCheckUp */


/******************************************************************************
 *
 * SkGePortCheckUpXmac() - Implementing of the Workaround Errata # 2
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 *	2	Link came up
 */
static int SkGePortCheckUpXmac(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* IO Context */
int		Port)		/* Which port should be checked */
{
	SK_U32		Shorts;		/* Short Event Counter */
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	int			Done;
	SK_U32		GpReg;		/* General Purpose register value */
	SK_U32		*pGpReg;	/* Pointer to -- " -- */
	SK_U16		Isrc;		/* Interrupt source register */
	SK_U16		IsrcSum;	/* Interrupt source register sum */
	SK_U16		LpAb;		/* Link Partner Ability */
	SK_U16		ResAb;		/* Resolved Ability */
	SK_U16		ExtStat;	/* Extended Status Register */
	SK_BOOL		AutoNeg;	/* Is Auto-negotiation used ? */
	SK_U8		NextMode;	/* Next AutoSensing Mode */

	pGpReg = &GpReg;

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PHWLinkUp) {
		if (pPrt->PhyType != SK_PHY_XMAC) {
			return(SK_HW_PS_NONE);
		}
		else {
			return(SkGePortCheckShorts(pAC, IoC, Port));
		}
	}

	IsrcSum = pPrt->PIsave;
	pPrt->PIsave = 0;

	/* Now wait for each port's link */
	if (pPrt->PLinkMode == SK_LMODE_HALF || pPrt->PLinkMode == SK_LMODE_FULL) {
		AutoNeg = SK_FALSE;
	}
	else {
		AutoNeg = SK_TRUE;
	}

	if (pPrt->PLinkBroken) {
		/* Link was broken */
		XM_IN32(IoC, Port, XM_GP_PORT, pGpReg);

		if ((GpReg & XM_GP_INP_ASS) == 0) {
			/* The Link is in sync */
			XM_IN16(IoC, Port, XM_ISRC, &Isrc);
			IsrcSum |= Isrc;
			SkXmAutoNegLipaXmac(pAC, IoC, Port, IsrcSum);

			if ((Isrc & XM_IS_INP_ASS) == 0) {
				/* It has been in sync since last time */
				/* Restart the PORT */
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
					("Link in sync Restart Port %d\n", Port));

				(void)SkXmUpdateStats(pAC, IoC, Port);

				/* We now need to reinitialize the PrevShorts counter */
				(void)SkXmMacStatistic(pAC, IoC, Port, XM_RXE_SHT_ERR, &Shorts);
				pPrt->PPrevShorts = Shorts;

				pPrt->PLinkBroken = SK_FALSE;

				/*
				 * Link Restart Workaround:
				 *  it may be possible that the other Link side
				 *  restarts its link as well an we detect
				 *  another LinkBroken. To prevent this
				 *  happening we check for a maximum number
				 *  of consecutive restart. If those happens,
				 *  we do NOT restart the active link and
				 *  check whether the link is now o.k.
				 */
				pPrt->PLinkResCt++;

				pPrt->PAutoNegTimeOut = 0;

				if (pPrt->PLinkResCt < SK_MAX_LRESTART) {
					return(SK_HW_PS_RESTART);
				}

				pPrt->PLinkResCt = 0;

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Do NOT restart on Port %d %x %x\n", Port, Isrc, IsrcSum));
			}
			else {
				pPrt->PIsave = (SK_U16)(IsrcSum & XM_IS_AND);

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Save Sync/nosync Port %d %x %x\n", Port, Isrc, IsrcSum));

				/* Do nothing more if link is broken */
				return(SK_HW_PS_NONE);
			}
		}
		else {
			/* Do nothing more if link is broken */
			return(SK_HW_PS_NONE);
		}

	}
	else {
		/* Link was not broken, check if it is */
		XM_IN16(IoC, Port, XM_ISRC, &Isrc);
		IsrcSum |= Isrc;
		if ((Isrc & XM_IS_INP_ASS) != 0) {
			XM_IN16(IoC, Port, XM_ISRC, &Isrc);
			IsrcSum |= Isrc;
			if ((Isrc & XM_IS_INP_ASS) != 0) {
				XM_IN16(IoC, Port, XM_ISRC, &Isrc);
				IsrcSum |= Isrc;
				if ((Isrc & XM_IS_INP_ASS) != 0) {
					pPrt->PLinkBroken = SK_TRUE;
					/* Re-Init Link partner Autoneg flag */
					pPrt->PLipaAutoNeg = SK_LIPA_UNKNOWN;
					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
						("Link broken Port %d\n", Port));

					/* Cable removed-> reinit sense mode */
					SkHWInitDefSense(pAC, IoC, Port);

					return(SK_HW_PS_RESTART);
				}
			}
		}
		else {
			SkXmAutoNegLipaXmac(pAC, IoC, Port, Isrc);
			if (SkGePortCheckShorts(pAC, IoC, Port) == SK_HW_PS_RESTART) {
				return(SK_HW_PS_RESTART);
			}
		}
	}

	/*
	 * here we usually can check whether the link is in sync and
	 * auto-negotiation is done.
	 */
	XM_IN32(IoC, Port, XM_GP_PORT, pGpReg);
	XM_IN16(IoC, Port, XM_ISRC, &Isrc);
	IsrcSum |= Isrc;

	SkXmAutoNegLipaXmac(pAC, IoC, Port, IsrcSum);

	if ((GpReg & XM_GP_INP_ASS) != 0 || (IsrcSum & XM_IS_INP_ASS) != 0) {
		if ((GpReg & XM_GP_INP_ASS) == 0) {
			/* Save Auto-negotiation Done interrupt only if link is in sync */
			pPrt->PIsave = (SK_U16)(IsrcSum & XM_IS_AND);
		}
#ifdef DEBUG
		if ((pPrt->PIsave & XM_IS_AND) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("AutoNeg done rescheduled Port %d\n", Port));
		}
#endif /* DEBUG */
		return(SK_HW_PS_NONE);
	}

	if (AutoNeg) {
		if ((IsrcSum & XM_IS_AND) != 0) {
			SkHWLinkUp(pAC, IoC, Port);
			Done = SkMacAutoNegDone(pAC, IoC, Port);
			if (Done != SK_AND_OK) {
				/* Get PHY parameters, for debugging only */
				SkXmPhyRead(pAC, IoC, Port, PHY_XMAC_AUNE_LP, &LpAb);
				SkXmPhyRead(pAC, IoC, Port, PHY_XMAC_RES_ABI, &ResAb);
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("AutoNeg FAIL Port %d (LpAb %x, ResAb %x)\n",
					 Port, LpAb, ResAb));

				/* Try next possible mode */
				NextMode = SkHWSenseGetNext(pAC, IoC, Port);
				SkHWLinkDown(pAC, IoC, Port);
				if (Done == SK_AND_DUP_CAP) {
					/* GoTo next mode */
					SkHWSenseSetNext(pAC, IoC, Port, NextMode);
				}

				return(SK_HW_PS_RESTART);
			}
			/*
			 * Dummy Read extended status to prevent extra link down/ups
			 * (clear Page Received bit if set)
			 */
			SkXmPhyRead(pAC, IoC, Port, PHY_XMAC_AUNE_EXP, &ExtStat);
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("AutoNeg done Port %d\n", Port));
			return(SK_HW_PS_LINK);
		}

		/* AutoNeg not done, but HW link is up. Check for timeouts */
		pPrt->PAutoNegTimeOut++;
		if (pPrt->PAutoNegTimeOut >= SK_AND_MAX_TO) {
			/* Increase the Timeout counter */
			pPrt->PAutoNegTOCt++;

			/* Timeout occured */
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
				("AutoNeg timeout Port %d\n", Port));
			if (pPrt->PLinkModeConf == SK_LMODE_AUTOSENSE &&
				pPrt->PLipaAutoNeg != SK_LIPA_AUTO) {
				/* Set Link manually up */
				SkHWSenseSetNext(pAC, IoC, Port, SK_LMODE_FULL);
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
					("Set manual full duplex Port %d\n", Port));
			}

			if (pPrt->PLinkModeConf == SK_LMODE_AUTOSENSE &&
				pPrt->PLipaAutoNeg == SK_LIPA_AUTO &&
				pPrt->PAutoNegTOCt >= SK_MAX_ANEG_TO) {
				/*
				 * This is rather complicated.
				 * we need to check here whether the LIPA_AUTO
				 * we saw before is false alert. We saw at one
				 * switch ( SR8800) that on boot time it sends
				 * just one auto-neg packet and does no further
				 * auto-negotiation.
				 * Solution: we restart the autosensing after
				 * a few timeouts.
				 */
				pPrt->PAutoNegTOCt = 0;
				pPrt->PLipaAutoNeg = SK_LIPA_UNKNOWN;
				SkHWInitDefSense(pAC, IoC, Port);
			}

			/* Do the restart */
			return(SK_HW_PS_RESTART);
		}
	}
	else {
		/* Link is up and we don't need more */
#ifdef DEBUG
		if (pPrt->PLipaAutoNeg == SK_LIPA_AUTO) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("ERROR: Lipa auto detected on port %d\n", Port));
		}
#endif /* DEBUG */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("Link sync(GP), Port %d\n", Port));
		SkHWLinkUp(pAC, IoC, Port);

		/*
		 * Link sync (GP) and so assume a good connection. But if not received
		 * a bunch of frames received in a time slot (maybe broken tx cable)
		 * the port is restart.
		 */
		return(SK_HW_PS_LINK);
	}

	return(SK_HW_PS_NONE);
}	/* SkGePortCheckUpXmac */


/******************************************************************************
 *
 * SkGePortCheckUpBcom() - Check if the link is up on Bcom PHY
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 *	2	Link came up
 */
static int SkGePortCheckUpBcom(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* IO Context */
int		Port)	/* Which port should be checked */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	int			Done;
	SK_U16		Isrc;		/* Interrupt source register */
	SK_U16		PhyStat;	/* Phy Status Register */
	SK_U16		ResAb;		/* Master/Slave resolution */
	SK_U16		Ctrl;		/* Broadcom control flags */
#ifdef DEBUG
	SK_U16		LpAb;
	SK_U16		ExtStat;
#endif /* DEBUG */
	SK_BOOL		AutoNeg;	/* Is Auto-negotiation used ? */

	pPrt = &pAC->GIni.GP[Port];

	/* Check for No HCD Link events (#10523) */
	SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_INT_STAT, &Isrc);

#ifdef xDEBUG
	if ((Isrc & ~(PHY_B_IS_HCT | PHY_B_IS_LCT) ==
		(PHY_B_IS_SCR_S_ER | PHY_B_IS_RRS_CHANGE | PHY_B_IS_LRS_CHANGE)) {

		SK_U32	Stat1, Stat2, Stat3;

		Stat1 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_INT_MASK, &Stat1);
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"CheckUp1 - Stat: %x, Mask: %x",
			(void *)Isrc,
			(void *)Stat1);

		Stat1 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_CTRL, &Stat1);
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_STAT, &Stat2);
		Stat1 = Stat1 << 16 | Stat2;
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUNE_ADV, &Stat2);
		Stat3 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUNE_LP, &Stat3);
		Stat2 = Stat2 << 16 | Stat3;
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"Ctrl/Stat: %x, AN Adv/LP: %x",
			(void *)Stat1,
			(void *)Stat2);

		Stat1 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUNE_EXP, &Stat1);
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_EXT_STAT, &Stat2);
		Stat1 = Stat1 << 16 | Stat2;
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_1000T_CTRL, &Stat2);
		Stat3 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_1000T_STAT, &Stat3);
		Stat2 = Stat2 << 16 | Stat3;
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"AN Exp/IEEE Ext: %x, 1000T Ctrl/Stat: %x",
			(void *)Stat1,
			(void *)Stat2);

		Stat1 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_P_EXT_CTRL, &Stat1);
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_P_EXT_STAT, &Stat2);
		Stat1 = Stat1 << 16 | Stat2;
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUX_CTRL, &Stat2);
		Stat3 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUX_STAT, &Stat3);
		Stat2 = Stat2 << 16 | Stat3;
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"PHY Ext Ctrl/Stat: %x, Aux Ctrl/Stat: %x",
			(void *)Stat1,
			(void *)Stat2);
	}
#endif /* DEBUG */

	if ((Isrc & (PHY_B_IS_NO_HDCL /* | PHY_B_IS_NO_HDC */)) != 0) {
		/*
		 * Workaround BCom Errata:
		 *	enable and disable loopback mode if "NO HCD" occurs.
		 */
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_CTRL, &Ctrl);
		SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_CTRL,
			(SK_U16)(Ctrl | PHY_CT_LOOP));
		SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_CTRL,
			(SK_U16)(Ctrl & ~PHY_CT_LOOP));
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("No HCD Link event, Port %d\n", Port));
#ifdef xDEBUG
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"No HCD link event, port %d.",
			(void *)Port,
			(void *)NULL);
#endif /* DEBUG */
	}

	/* Not obsolete: link status bit is latched to 0 and autoclearing! */
	SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_STAT, &PhyStat);

	if (pPrt->PHWLinkUp) {
		return(SK_HW_PS_NONE);
	}

#ifdef xDEBUG
	{
		SK_U32	Stat1, Stat2, Stat3;

		Stat1 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_INT_MASK, &Stat1);
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"CheckUp1a - Stat: %x, Mask: %x",
			(void *)Isrc,
			(void *)Stat1);

		Stat1 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_CTRL, &Stat1);
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_STAT, &PhyStat);
		Stat1 = Stat1 << 16 | PhyStat;
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUNE_ADV, &Stat2);
		Stat3 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUNE_LP, &Stat3);
		Stat2 = Stat2 << 16 | Stat3;
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"Ctrl/Stat: %x, AN Adv/LP: %x",
			(void *)Stat1,
			(void *)Stat2);

		Stat1 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUNE_EXP, &Stat1);
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_EXT_STAT, &Stat2);
		Stat1 = Stat1 << 16 | Stat2;
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_1000T_CTRL, &Stat2);
		Stat3 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_1000T_STAT, &ResAb);
		Stat2 = Stat2 << 16 | ResAb;
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"AN Exp/IEEE Ext: %x, 1000T Ctrl/Stat: %x",
			(void *)Stat1,
			(void *)Stat2);

		Stat1 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_P_EXT_CTRL, &Stat1);
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_P_EXT_STAT, &Stat2);
		Stat1 = Stat1 << 16 | Stat2;
		Stat2 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUX_CTRL, &Stat2);
		Stat3 = 0;
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUX_STAT, &Stat3);
		Stat2 = Stat2 << 16 | Stat3;
		CMSMPrintString(
			pAC->pConfigTable,
			MSG_TYPE_RUNTIME_INFO,
			"PHY Ext Ctrl/Stat: %x, Aux Ctrl/Stat: %x",
			(void *)Stat1,
			(void *)Stat2);
	}
#endif /* DEBUG */

	/* Now wait for each port's link */
	if (pPrt->PLinkMode == SK_LMODE_HALF || pPrt->PLinkMode == SK_LMODE_FULL) {
		AutoNeg = SK_FALSE;
	}
	else {
		AutoNeg = SK_TRUE;
	}

	/*
	 * Here we usually can check whether the link is in sync and
	 * auto-negotiation is done.
	 */

	SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_STAT, &PhyStat);

	SkMacAutoNegLipaPhy(pAC, IoC, Port, PhyStat);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNeg: %d, PhyStat: 0x%04x\n", AutoNeg, PhyStat));

	SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_1000T_STAT, &ResAb);

	if ((ResAb & PHY_B_1000S_MSF) != 0) {
		/* Error */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Master/Slave Fault port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;
		pPrt->PMSStatus = SK_MS_STAT_FAULT;

		return(SK_HW_PS_RESTART);
	}

	if ((PhyStat & PHY_ST_LSYNC) == 0) {
		return(SK_HW_PS_NONE);
	}

	pPrt->PMSStatus = ((ResAb & PHY_B_1000S_MSR) != 0) ?
		SK_MS_STAT_MASTER : SK_MS_STAT_SLAVE;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNeg: %d, PhyStat: 0x%04x\n", AutoNeg, PhyStat));

	if (AutoNeg) {
		if ((PhyStat & PHY_ST_AN_OVER) != 0) {
			SkHWLinkUp(pAC, IoC, Port);
			Done = SkMacAutoNegDone(pAC, IoC, Port);
			if (Done != SK_AND_OK) {
#ifdef DEBUG
				/* Get PHY parameters, for debugging only */
				SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUNE_LP, &LpAb);
				SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_1000T_STAT, &ExtStat);
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("AutoNeg FAIL Port %d (LpAb %x, 1000TStat %x)\n",
					Port, LpAb, ExtStat));
#endif /* DEBUG */
				return(SK_HW_PS_RESTART);
			}
			else {
#ifdef xDEBUG
				/* Dummy read ISR to prevent extra link downs/ups */
				SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_INT_STAT, &ExtStat);

				if ((ExtStat & ~(PHY_B_IS_HCT | PHY_B_IS_LCT)) != 0) {
					CMSMPrintString(
						pAC->pConfigTable,
						MSG_TYPE_RUNTIME_INFO,
						"CheckUp2 - Stat: %x",
						(void *)ExtStat,
						(void *)NULL);
				}
#endif /* DEBUG */

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("AutoNeg done Port %d\n", Port));
				return(SK_HW_PS_LINK);
			}
		}
	}
	else {	/* !AutoNeg */
		/* Link is up and we don't need more. */
#ifdef DEBUG
		if (pPrt->PLipaAutoNeg == SK_LIPA_AUTO) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("ERROR: Lipa auto detected on port %d\n", Port));
		}
#endif /* DEBUG */

#ifdef xDEBUG
		/* Dummy read ISR to prevent extra link downs/ups */
		SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_INT_STAT, &ExtStat);

		if ((ExtStat & ~(PHY_B_IS_HCT | PHY_B_IS_LCT)) != 0) {
			CMSMPrintString(
				pAC->pConfigTable,
				MSG_TYPE_RUNTIME_INFO,
				"CheckUp3 - Stat: %x",
				(void *)ExtStat,
				(void *)NULL);
		}
#endif /* DEBUG */

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("Link sync(GP), Port %d\n", Port));
		SkHWLinkUp(pAC, IoC, Port);
		return(SK_HW_PS_LINK);
	}

	return(SK_HW_PS_NONE);
}	/* SkGePortCheckUpBcom */


/******************************************************************************
 *
 * SkGePortCheckUpGmac() - Check if the link is up on Marvell PHY
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 *	2	Link came up
 */
static int SkGePortCheckUpGmac(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* IO Context */
int		Port)	/* Which port should be checked */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	int			Done;
	SK_U16		Isrc;		/* Interrupt source */
	SK_U16		PhyStat;	/* Phy Status */
	SK_U16		PhySpecStat;/* Phy Specific Status */
	SK_U16		ResAb;		/* Master/Slave resolution */
	SK_BOOL		AutoNeg;	/* Is Auto-negotiation used ? */

	pPrt = &pAC->GIni.GP[Port];

	/* Read PHY Interrupt Status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_INT_STAT, &Isrc);

	if ((Isrc & PHY_M_IS_AN_COMPL) != 0) {
		/* TBD */
	}

	if ((Isrc & PHY_M_IS_DOWNSH_DET) != 0) {
		/* TBD */
	}

	if (pPrt->PHWLinkUp) {
		return(SK_HW_PS_NONE);
	}

	/* Now wait for each port's link */
	if (pPrt->PLinkMode == SK_LMODE_HALF || pPrt->PLinkMode == SK_LMODE_FULL) {
		AutoNeg = SK_FALSE;
	}
	else {
		AutoNeg = SK_TRUE;
	}

	/* Read PHY Status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_STAT, &PhyStat);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNeg: %d, PhyStat: 0x%04x\n", AutoNeg, PhyStat));

	SkMacAutoNegLipaPhy(pAC, IoC, Port, PhyStat);

	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_1000T_STAT, &ResAb);

	if ((ResAb & PHY_B_1000S_MSF) != 0) {
		/* Error */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Master/Slave Fault port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;
		pPrt->PMSStatus = SK_MS_STAT_FAULT;

		return(SK_HW_PS_RESTART);
	}

	/* Read PHY Specific Status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_STAT, &PhySpecStat);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNeg: %d, PhySpecStat: 0x%04x\n", AutoNeg, PhySpecStat));

	if ((PhySpecStat & PHY_M_PS_LINK_UP) == 0) {
		return(SK_HW_PS_NONE);
	}

	pPrt->PMSStatus = ((ResAb & PHY_B_1000S_MSR) != 0) ?
		SK_MS_STAT_MASTER : SK_MS_STAT_SLAVE;

	pPrt->PCableLen = (SK_U8)((PhySpecStat & PHY_M_PS_CABLE_MSK) >> 7);

	if (AutoNeg) {
		/* Auto-Negotiation Over ? */
		if ((PhyStat & PHY_ST_AN_OVER) != 0) {

			SkHWLinkUp(pAC, IoC, Port);

			Done = SkMacAutoNegDone(pAC, IoC, Port);

			if (Done != SK_AND_OK) {
				return(SK_HW_PS_RESTART);
			}

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("AutoNeg done Port %d\n", Port));
			return(SK_HW_PS_LINK);
		}
	}
	else {	/* !AutoNeg */
		/* Link is up and we don't need more */
#ifdef DEBUG
		if (pPrt->PLipaAutoNeg == SK_LIPA_AUTO) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("ERROR: Lipa auto detected on port %d\n", Port));
		}
#endif /* DEBUG */

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("Link sync, Port %d\n", Port));
		SkHWLinkUp(pAC, IoC, Port);

		return(SK_HW_PS_LINK);
	}

	return(SK_HW_PS_NONE);
}	/* SkGePortCheckUpGmac */


#ifdef OTHER_PHY
/******************************************************************************
 *
 * SkGePortCheckUpLone() - Check if the link is up on Level One PHY
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 *	2	Link came up
 */
static int SkGePortCheckUpLone(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* IO Context */
int		Port)		/* Which port should be checked */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	int			Done;
	SK_U16		Isrc;		/* Interrupt source register */
	SK_U16		LpAb;		/* Link Partner Ability */
	SK_U16		ExtStat;	/* Extended Status Register */
	SK_U16		PhyStat;	/* Phy Status Register */
	SK_U16		StatSum;
	SK_BOOL		AutoNeg;	/* Is Auto-negotiation used ? */
	SK_U8		NextMode;	/* Next AutoSensing Mode */

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PHWLinkUp) {
		return(SK_HW_PS_NONE);
	}

	StatSum = pPrt->PIsave;
	pPrt->PIsave = 0;

	/* Now wait for each ports link */
	if (pPrt->PLinkMode == SK_LMODE_HALF || pPrt->PLinkMode == SK_LMODE_FULL) {
		AutoNeg = SK_FALSE;
	}
	else {
		AutoNeg = SK_TRUE;
	}

	/*
	 * here we usually can check whether the link is in sync and
	 * auto-negotiation is done.
	 */
	SkXmPhyRead(pAC, IoC, Port, PHY_LONE_STAT, &PhyStat);
	StatSum |= PhyStat;

	SkMacAutoNegLipaPhy(pAC, IoC, Port, PhyStat);

	if ((PhyStat & PHY_ST_LSYNC) == 0) {
		/* Save Auto-negotiation Done bit */
		pPrt->PIsave = (SK_U16)(StatSum & PHY_ST_AN_OVER);
#ifdef DEBUG
		if ((pPrt->PIsave & PHY_ST_AN_OVER) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("AutoNeg done rescheduled Port %d\n", Port));
		}
#endif /* DEBUG */
		return(SK_HW_PS_NONE);
	}

	if (AutoNeg) {
		if ((StatSum & PHY_ST_AN_OVER) != 0) {
			SkHWLinkUp(pAC, IoC, Port);
			Done = SkMacAutoNegDone(pAC, IoC, Port);
			if (Done != SK_AND_OK) {
				/* Get PHY parameters, for debugging only */
				SkXmPhyRead(pAC, IoC, Port, PHY_LONE_AUNE_LP, &LpAb);
				SkXmPhyRead(pAC, IoC, Port, PHY_LONE_1000T_STAT, &ExtStat);
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("AutoNeg FAIL Port %d (LpAb %x, 1000TStat %x)\n",
					 Port, LpAb, ExtStat));

				/* Try next possible mode */
				NextMode = SkHWSenseGetNext(pAC, IoC, Port);
				SkHWLinkDown(pAC, IoC, Port);
				if (Done == SK_AND_DUP_CAP) {
					/* GoTo next mode */
					SkHWSenseSetNext(pAC, IoC, Port, NextMode);
				}

				return(SK_HW_PS_RESTART);

			}
			else {
				/*
				 * Dummy Read interrupt status to prevent
				 * extra link down/ups
				 */
				SkXmPhyRead(pAC, IoC, Port, PHY_LONE_INT_STAT, &ExtStat);
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("AutoNeg done Port %d\n", Port));
				return(SK_HW_PS_LINK);
			}
		}

		/* AutoNeg not done, but HW link is up. Check for timeouts */
		pPrt->PAutoNegTimeOut++;
		if (pPrt->PAutoNegTimeOut >= SK_AND_MAX_TO) {
			/* Timeout occured */
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
				("AutoNeg timeout Port %d\n", Port));
			if (pPrt->PLinkModeConf == SK_LMODE_AUTOSENSE &&
				pPrt->PLipaAutoNeg != SK_LIPA_AUTO) {
				/* Set Link manually up */
				SkHWSenseSetNext(pAC, IoC, Port, SK_LMODE_FULL);
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
					("Set manual full duplex Port %d\n", Port));
			}

			/* Do the restart */
			return(SK_HW_PS_RESTART);
		}
	}
	else {
		/* Link is up and we don't need more */
#ifdef DEBUG
		if (pPrt->PLipaAutoNeg == SK_LIPA_AUTO) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("ERROR: Lipa auto detected on port %d\n", Port));
		}
#endif /* DEBUG */

		/*
		 * Dummy Read interrupt status to prevent
		 * extra link down/ups
		 */
		SkXmPhyRead(pAC, IoC, Port, PHY_LONE_INT_STAT, &ExtStat);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("Link sync(GP), Port %d\n", Port));
		SkHWLinkUp(pAC, IoC, Port);
		return(SK_HW_PS_LINK);
	}

	return(SK_HW_PS_NONE);
}	/* SkGePortCheckUpLone */


/******************************************************************************
 *
 * SkGePortCheckUpNat() - Check if the link is up on National PHY
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 *	2	Link came up
 */
static int SkGePortCheckUpNat(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* IO Context */
int		Port)		/* Which port should be checked */
{
	/* todo: National */
	return(SK_HW_PS_NONE);
}	/* SkGePortCheckUpNat */
#endif /* OTHER_PHY */


/******************************************************************************
 *
 *	SkGeSirqEvent() - Event Service Routine
 *
 * Description:
 *
 * Notes:
 */
int	SkGeSirqEvent(
SK_AC		*pAC,		/* Adapter Context */
SK_IOC		IoC,		/* Io Context */
SK_U32		Event,		/* Module specific Event */
SK_EVPARA	Para)		/* Event specific Parameter */
{
	SK_U64		Octets;
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	SK_U32		Port;
	SK_U32		Time;
	unsigned	Len;
	int			PortStat;
	SK_U8		Val8;

	Port = Para.Para32[0];
	pPrt = &pAC->GIni.GP[Port];

	switch (Event) {
	case SK_HWEV_WATIM:
		/* Check whether port came up */
		PortStat = SkGePortCheckUp(pAC, IoC, Port);

		switch (PortStat) {
		case SK_HW_PS_RESTART:
			if (pPrt->PHWLinkUp) {
				/*
				 * Set Link to down.
				 */
				SkHWLinkDown(pAC, IoC, Port);

				/*
				 * Signal directly to RLMT to ensure correct
				 * sequence of SWITCH and RESET event.
				 */
				SkRlmtEvent(pAC, IoC, SK_RLMT_LINK_DOWN, Para);
			}

			/* Restart needed */
			SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_RESET, Para);
			break;

		case SK_HW_PS_LINK:
			/* Signal to RLMT */
			SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_UP, Para);
			break;

		}

		/* Start again the check Timer */
		if (pPrt->PHWLinkUp) {
			Time = SK_WA_ACT_TIME;
		}
		else {
			Time = SK_WA_INA_TIME;
		}

		/* Todo: still needed for non-XMAC PHYs??? */
		/* Start workaround Errata #2 timer */
		SkTimerStart(pAC, IoC, &pPrt->PWaTimer, Time,
			SKGE_HWAC, SK_HWEV_WATIM, Para);
		break;

	case SK_HWEV_PORT_START:
		if (pPrt->PHWLinkUp) {
			/*
			 * Signal directly to RLMT to ensure correct
			 * sequence of SWITCH and RESET event.
			 */
			SkRlmtEvent(pAC, IoC, SK_RLMT_LINK_DOWN, Para);
		}

		SkHWLinkDown(pAC, IoC, Port);

		/* Schedule Port RESET */
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_RESET, Para);

		/* Start workaround Errata #2 timer */
		SkTimerStart(pAC, IoC, &pPrt->PWaTimer, SK_WA_INA_TIME,
			SKGE_HWAC, SK_HWEV_WATIM, Para);
		break;

	case SK_HWEV_PORT_STOP:
		if (pPrt->PHWLinkUp) {
			/*
			 * Signal directly to RLMT to ensure correct
			 * sequence of SWITCH and RESET event.
			 */
			SkRlmtEvent(pAC, IoC, SK_RLMT_LINK_DOWN, Para);
		}

		/* Stop Workaround Timer */
		SkTimerStop(pAC, IoC, &pPrt->PWaTimer);

		SkHWLinkDown(pAC, IoC, Port);
		break;

	case SK_HWEV_UPDATE_STAT:
		/* We do NOT need to update any statistics */
		break;

	case SK_HWEV_CLEAR_STAT:
		/* We do NOT need to clear any statistics */
		for (Port = 0; Port < (SK_U32)pAC->GIni.GIMacsFound; Port++) {
			pPrt->PPrevRx = 0;
			pPrt->PPrevFcs = 0;
			pPrt->PPrevShorts = 0;
		}
		break;

	case SK_HWEV_SET_LMODE:
		Val8 = (SK_U8)Para.Para32[1];
		if (pPrt->PLinkModeConf != Val8) {
			/* Set New link mode */
			pPrt->PLinkModeConf = Val8;

			/* Restart Port */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP, Para);
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START, Para);
		}
		break;

	case SK_HWEV_SET_FLOWMODE:
		Val8 = (SK_U8)Para.Para32[1];
		if (pPrt->PFlowCtrlMode != Val8) {
			/* Set New Flow Control mode */
			pPrt->PFlowCtrlMode = Val8;

			/* Restart Port */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP, Para);
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START, Para);
		}
		break;

	case SK_HWEV_SET_ROLE:
		/* not possible for fiber */
		if (!pAC->GIni.GICopperType) {
			break;
		}
		Val8 = (SK_U8)Para.Para32[1];
		if (pPrt->PMSMode != Val8) {
			/* Set New link mode */
			pPrt->PMSMode = Val8;

			/* Restart Port */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP, Para);
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START, Para);
		}
		break;

	case SK_HWEV_SET_SPEED:
		if (pPrt->PhyType != SK_PHY_MARV_COPPER) {
			break;
		}
		Val8 = (SK_U8)Para.Para32[1];
		if (pPrt->PLinkSpeed != Val8) {
			/* Set New Speed parameter */
			pPrt->PLinkSpeed = Val8;

			/* Restart Port */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP, Para);
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START, Para);
		}
		break;

	case SK_HWEV_HALFDUP_CHK:
		/*
		 * half duplex hangup workaround.
		 * See packet arbiter timeout interrupt for description
		 */
		pPrt->HalfDupTimerActive = SK_FALSE;
		if (pPrt->PLinkModeStatus == SK_LMODE_STAT_HALF ||
		    pPrt->PLinkModeStatus == SK_LMODE_STAT_AUTOHALF) {

			Len = sizeof(SK_U64);
			SkPnmiGetVar(pAC, IoC, OID_SKGE_STAT_TX_OCTETS, (char *)&Octets,
				&Len, (SK_U32)SK_PNMI_PORT_PHYS2INST(pAC, Port),
				pAC->Rlmt.Port[Port].Net->NetNumber);

			if (pPrt->LastOctets == Octets) {
				/* Tx hanging, a FIFO flush restarts it */
				SkMacFlushTxFifo(pAC, IoC, Port);
			}
		}
		break;

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_SIRQ_E001, SKERR_SIRQ_E001MSG);
		break;
	}

	return(0);
}	/* SkGeSirqEvent */


/******************************************************************************
 *
 *	SkPhyIsrBcom() - PHY interrupt service routine
 *
 * Description: handles all interrupts from BCom PHY
 *
 * Returns: N/A
 */
static void SkPhyIsrBcom(
SK_AC		*pAC,		/* Adapter Context */
SK_IOC		IoC,		/* Io Context */
int			Port,		/* Port Num = PHY Num */
SK_U16		IStatus)	/* Interrupt Status */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	SK_EVPARA	Para;

	pPrt = &pAC->GIni.GP[Port];

	if ((IStatus & PHY_B_IS_PSE) != 0) {
		/* Incorrectable pair swap error */
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E022,
			SKERR_SIRQ_E022MSG);
	}

	if ((IStatus & (PHY_B_IS_AN_PR | PHY_B_IS_LST_CHANGE)) != 0) {
		Para.Para32[0] = (SK_U32)Port;

		SkHWLinkDown(pAC, IoC, Port);

		/* Signal to RLMT */
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);

		/* Start workaround Errata #2 timer */
		SkTimerStart(pAC, IoC, &pPrt->PWaTimer, SK_WA_INA_TIME,
			SKGE_HWAC, SK_HWEV_WATIM, Para);
	}

}	/* SkPhyIsrBcom */


/******************************************************************************
 *
 *	SkPhyIsrGmac() - PHY interrupt service routine
 *
 * Description: handles all interrupts from Marvell PHY
 *
 * Returns: N/A
 */
static void SkPhyIsrGmac(
SK_AC		*pAC,		/* Adapter Context */
SK_IOC		IoC,		/* Io Context */
int			Port,		/* Port Num = PHY Num */
SK_U16		IStatus)	/* Interrupt Status */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	SK_EVPARA	Para;

	pPrt = &pAC->GIni.GP[Port];

	if ((IStatus & (PHY_M_IS_AN_PR | PHY_M_IS_LST_CHANGE)) != 0) {
		Para.Para32[0] = (SK_U32)Port;

		SkHWLinkDown(pAC, IoC, Port);

		/* Signal to RLMT */
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((IStatus & PHY_M_IS_AN_ERROR) != 0) {
		/* Auto-Negotiation Error */
		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E023, SKERR_SIRQ_E023MSG);
	}

	if ((IStatus & PHY_M_IS_LSP_CHANGE) != 0) {
		/* TBD */
	}

	if ((IStatus & PHY_M_IS_FIFO_ERROR) != 0) {
		/* FIFO Overflow/Underrun Error */
		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E024, SKERR_SIRQ_E024MSG);
	}
}	/* SkPhyIsrGmac */


#ifdef OTHER_PHY
/******************************************************************************
 *
 *	SkPhyIsrLone() - PHY interrupt service routine
 *
 * Description: handles all interrupts from LONE PHY
 *
 * Returns: N/A
 */
static void SkPhyIsrLone(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* Io Context */
int		Port,		/* Port Num = PHY Num */
SK_U16	IStatus)	/* Interrupt Status */
{
	SK_EVPARA	Para;

	if (IStatus & (PHY_L_IS_DUP | PHY_L_IS_ISOL)) {
		SkHWLinkDown(pAC, IoC, Port);

		/* Signal to RLMT */
		Para.Para32[0] = (SK_U32)Port;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

}	/* SkPhyIsrLone */
#endif /* OTHER_PHY */

/* End of File */
