/******************************************************************************
 *
 * Name:	skdrv2nd.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.15 $
 * Date:	$Date: 2003/02/25 14:16:40 $
 * Purpose:	Second header file for driver and all other modules
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
 *	$Log: skdrv2nd.h,v $
 *	Revision 1.15  2003/02/25 14:16:40  mlindner
 *	Fix: Copyright statement
 *
 *	Revision 1.14  2003/02/25 13:26:26  mlindner
 *	Add: Support for various vendors
 *
 *	Revision 1.13  2002/10/02 12:46:02  mlindner
 *	Add: Support for Yukon
 *
 *	Revision 1.12.2.2  2001/09/05 12:14:50  mlindner
 *	add: New hardware revision int
 *
 *	Revision 1.12.2.1  2001/03/12 16:50:59  mlindner
 *	chg: kernel 2.4 adaption
 *
 *	Revision 1.12  2001/03/01 12:52:15  mlindner
 *	Fixed ring size
 *
 *	Revision 1.11  2001/02/19 13:28:02  mlindner
 *	Changed PNMI parameter values
 *
 *	Revision 1.10  2001/01/22 14:16:04  mlindner
 *	added ProcFs functionality
 *	Dual Net functionality integrated
 *	Rlmt networks added
 *
 *	Revision 1.1  2000/10/05 19:46:50  phargrov
 *	Add directory src/vipk_devs_nonlbl/vipk_sk98lin/
 *	This is the SysKonnect SK-98xx Gigabit Ethernet driver,
 *	contributed by SysKonnect.
 *
 *	Revision 1.9  2000/02/21 10:39:55  cgoos
 *	Added flag for jumbo support usage.
 *
 *	Revision 1.8  1999/11/22 13:50:44  cgoos
 *	Changed license header to GPL.
 *	Fixed two comments.
 *
 *	Revision 1.7  1999/09/28 12:38:21  cgoos
 *	Added CheckQueue to SK_AC.
 *
 *	Revision 1.6  1999/07/27 08:04:05  cgoos
 *	Added checksumming variables to SK_AC.
 *
 *	Revision 1.5  1999/03/29 12:33:26  cgoos
 *	Rreversed to fine lock granularity.
 *
 *	Revision 1.4  1999/03/15 12:14:02  cgoos
 *	Added DriverLock to SK_AC.
 *	Removed other locks.
 *
 *	Revision 1.3  1999/03/01 08:52:27  cgoos
 *	Changed pAC->PciDev declaration.
 *
 *	Revision 1.2  1999/02/18 10:57:14  cgoos
 *	Removed SkDrvTimeStamp prototype.
 *	Fixed SkGeOsGetTime prototype.
 *
 *	Revision 1.1  1999/02/16 07:41:01  cgoos
 *	First version.
 *
 *
 *
 ******************************************************************************/

/******************************************************************************
 *
 * Description:
 *
 * This is the second include file of the driver, which includes all other
 * neccessary files and defines all structures and constants used by the
 * driver and the common modules.
 *
 * Include File Hierarchy:
 *
 *	see skge.c
 *
 ******************************************************************************/

#ifndef __INC_SKDRV2ND_H
#define __INC_SKDRV2ND_H

#include "h/skqueue.h"
#include "h/skgehwt.h"
#include "h/sktimer.h"
#include "h/ski2c.h"
#include "h/skgepnmi.h"
#include "h/skvpd.h"
#include "h/skgehw.h"
#include "h/skgeinit.h"
#include "h/skaddr.h"
#include "h/skgesirq.h"
#include "h/skcsum.h"
#include "h/skrlmt.h"
#include "h/skgedrv.h"

#define SK_PCI_ISCOMPLIANT(result, pdev) {     \
    result = SK_FALSE; /* default */     \
    /* 3Com (0x10b7) */     \
    if (pdev->vendor == 0x10b7) {     \
	/* Gigabit Ethernet Adapter (0x1700) */     \
	if ((pdev->device == 0x1700)) { \
	    result = SK_TRUE;     \
	}     \
    /* SysKonnect (0x1148) */     \
    } else if (pdev->vendor == 0x1148) {     \
	/* SK-98xx Gigabit Ethernet Server Adapter (0x4300) */     \
	/* SK-98xx V2 Gigabit Ethernet Adapter (0x4320) */     \
	if ((pdev->device == 0x4300) || \
	    (pdev->device == 0x4320)) { \
	    result = SK_TRUE;     \
	}     \
    /* D-Link (0x1186) */     \
    } else if (pdev->vendor == 0x1186) {     \
	/* Gigabit Ethernet Adapter (0x4c00) */     \
	if ((pdev->device == 0x4c00)) { \
	    result = SK_TRUE;     \
	}     \
    /* CNet (0x1371) */     \
    } else if (pdev->vendor == 0x1371) {     \
	/* GigaCard Network Adapter (0x434e) */     \
	if ((pdev->device == 0x434e)) { \
	    result = SK_TRUE;     \
	}     \
    /* Linksys (0x1737) */     \
    } else if (pdev->vendor == 0x1737) {     \
	/* Gigabit Network Adapter (0x1032) */     \
	/* Gigabit Network Adapter (0x1064) */     \
	if ((pdev->device == 0x1032) || \
	    (pdev->device == 0x1064)) { \
	    result = SK_TRUE;     \
	}     \
    } else {     \
	result = SK_FALSE;     \
    }     \
}


extern SK_MBUF		*SkDrvAllocRlmtMbuf(SK_AC*, SK_IOC, unsigned);
extern void		SkDrvFreeRlmtMbuf(SK_AC*, SK_IOC, SK_MBUF*);
extern SK_U64		SkOsGetTime(SK_AC*);
extern int		SkPciReadCfgDWord(SK_AC*, int, SK_U32*);
extern int		SkPciReadCfgWord(SK_AC*, int, SK_U16*);
extern int		SkPciReadCfgByte(SK_AC*, int, SK_U8*);
extern int		SkPciWriteCfgDWord(SK_AC*, int, SK_U32);
extern int		SkPciWriteCfgWord(SK_AC*, int, SK_U16);
extern int		SkPciWriteCfgByte(SK_AC*, int, SK_U8);
extern int		SkDrvEvent(SK_AC*, SK_IOC IoC, SK_U32, SK_EVPARA);

struct s_DrvRlmtMbuf {
	SK_MBUF		*pNext;		/* Pointer to next RLMT Mbuf. */
	SK_U8		*pData;		/* Data buffer (virtually contig.). */
	unsigned	Size;		/* Data buffer size. */
	unsigned	Length;		/* Length of packet (<= Size). */
	SK_U32		PortIdx;	/* Receiving/transmitting port. */
#ifdef SK_RLMT_MBUF_PRIVATE
	SK_RLMT_MBUF	Rlmt;		/* Private part for RLMT. */
#endif  /* SK_RLMT_MBUF_PRIVATE */
	struct sk_buff	*pOs;		/* Pointer to message block */
};


/*
 * ioctl definitions
 */
#define		SK_IOCTL_BASE		(SIOCDEVPRIVATE)
#define		SK_IOCTL_GETMIB		(SK_IOCTL_BASE + 0)
#define		SK_IOCTL_SETMIB		(SK_IOCTL_BASE + 1)
#define		SK_IOCTL_PRESETMIB	(SK_IOCTL_BASE + 2)

typedef struct s_IOCTL	SK_GE_IOCTL;

struct s_IOCTL {
	char*		pData;
	unsigned int	Len;
};


/*
 * define sizes of descriptor rings in bytes
 */

#if 0
#define		TX_RING_SIZE	(8*1024)
#define		RX_RING_SIZE	(24*1024)
#else
#define		TX_RING_SIZE	(10 * 40)
#define		RX_RING_SIZE	(10 * 40)
#endif

/*
 * Buffer size for ethernet packets
 */
#define	ETH_BUF_SIZE	1540
#define	ETH_MAX_MTU	1514
#define ETH_MIN_MTU	60
#define ETH_MULTICAST_BIT	0x01
#define SK_JUMBO_MTU	9000

/*
 * transmit priority selects the queue: LOW=asynchron, HIGH=synchron
 */
#define TX_PRIO_LOW	0
#define TX_PRIO_HIGH	1

/*
 * alignment of rx/tx descriptors
 */
#define DESCR_ALIGN	8

/*
 * definitions for pnmi. TODO
 */
#define SK_DRIVER_RESET(pAC, IoC)	0
#define SK_DRIVER_SENDEVENT(pAC, IoC)	0
#define SK_DRIVER_SELFTEST(pAC, IoC)	0
/* For get mtu you must add an own function */
#define SK_DRIVER_GET_MTU(pAc,IoC,i)	0
#define SK_DRIVER_SET_MTU(pAc,IoC,i,v)	0
#define SK_DRIVER_PRESET_MTU(pAc,IoC,i,v)	0


/* TX and RX descriptors *****************************************************/

typedef struct s_RxD RXD; /* the receive descriptor */

struct s_RxD {
	volatile SK_U32	RBControl;	/* Receive Buffer Control */
	SK_U32		VNextRxd;	/* Next receive descriptor,low dword */
	SK_U32		VDataLow;	/* Receive buffer Addr, low dword */
	SK_U32		VDataHigh;	/* Receive buffer Addr, high dword */
	SK_U32		FrameStat;	/* Receive Frame Status word */
	SK_U32		TimeStamp;	/* Time stamp from XMAC */
	SK_U32		TcpSums;	/* TCP Sum 2 / TCP Sum 1 */
	SK_U32		TcpSumStarts;	/* TCP Sum Start 2 / TCP Sum Start 1 */
	RXD		*pNextRxd;	/* Pointer to next Rxd */
	struct sk_buff	*pMBuf;		/* Pointer to Linux' socket buffer */
};

typedef struct s_TxD TXD; /* the transmit descriptor */

struct s_TxD {
	volatile SK_U32	TBControl;	/* Transmit Buffer Control */
	SK_U32		VNextTxd;	/* Next transmit descriptor,low dword */
	SK_U32		VDataLow;	/* Transmit Buffer Addr, low dword */
	SK_U32		VDataHigh;	/* Transmit Buffer Addr, high dword */
	SK_U32		FrameStat;	/* Transmit Frame Status Word */
	SK_U32		TcpSumOfs;	/* Reserved / TCP Sum Offset */
	SK_U16		TcpSumSt;	/* TCP Sum Start */
	SK_U16		TcpSumWr;	/* TCP Sum Write */
	SK_U32		TcpReserved;	/* not used */
	TXD		*pNextTxd;	/* Pointer to next Txd */
	struct sk_buff	*pMBuf;		/* Pointer to Linux' socket buffer */
};


/* definition of flags in descriptor control field */
#define	RX_CTRL_OWN_BMU		UINT32_C(0x80000000)
#define	RX_CTRL_STF		UINT32_C(0x40000000)
#define	RX_CTRL_EOF		UINT32_C(0x20000000)
#define	RX_CTRL_EOB_IRQ		UINT32_C(0x10000000)
#define	RX_CTRL_EOF_IRQ		UINT32_C(0x08000000)
#define RX_CTRL_DEV_NULL	UINT32_C(0x04000000)
#define RX_CTRL_STAT_VALID	UINT32_C(0x02000000)
#define RX_CTRL_TIME_VALID	UINT32_C(0x01000000)
#define RX_CTRL_CHECK_DEFAULT	UINT32_C(0x00550000)
#define RX_CTRL_CHECK_CSUM	UINT32_C(0x00560000)
#define	RX_CTRL_LEN_MASK	UINT32_C(0x0000FFFF)

#define	TX_CTRL_OWN_BMU		UINT32_C(0x80000000)
#define	TX_CTRL_STF		UINT32_C(0x40000000)
#define	TX_CTRL_EOF		UINT32_C(0x20000000)
#define	TX_CTRL_EOB_IRQ		UINT32_C(0x10000000)
#define	TX_CTRL_EOF_IRQ		UINT32_C(0x08000000)
#define TX_CTRL_ST_FWD		UINT32_C(0x04000000)
#define TX_CTRL_DISAB_CRC	UINT32_C(0x02000000)
#define TX_CTRL_SOFTWARE	UINT32_C(0x01000000)
#define TX_CTRL_CHECK_DEFAULT	UINT32_C(0x00550000)
#define TX_CTRL_CHECK_CSUM	UINT32_C(0x00560000)
#define	TX_CTRL_LEN_MASK	UINT32_C(0x0000FFFF)


/* The offsets of registers in the TX and RX queue control io area ***********/

#define RX_Q_BUF_CTRL_CNT	0x00
#define RX_Q_NEXT_DESCR_LOW	0x04
#define RX_Q_BUF_ADDR_LOW	0x08
#define RX_Q_BUF_ADDR_HIGH	0x0c
#define RX_Q_FRAME_STAT		0x10
#define RX_Q_TIME_STAMP		0x14
#define RX_Q_CSUM_1_2		0x18
#define RX_Q_CSUM_START_1_2	0x1c
#define RX_Q_CUR_DESCR_LOW	0x20
#define RX_Q_DESCR_HIGH		0x24
#define RX_Q_CUR_ADDR_LOW	0x28
#define RX_Q_CUR_ADDR_HIGH	0x2c
#define RX_Q_CUR_BYTE_CNT	0x30
#define RX_Q_CTRL		0x34
#define RX_Q_FLAG		0x38
#define RX_Q_TEST1		0x3c
#define RX_Q_TEST2		0x40
#define RX_Q_TEST3		0x44

#define TX_Q_BUF_CTRL_CNT	0x00
#define TX_Q_NEXT_DESCR_LOW	0x04
#define TX_Q_BUF_ADDR_LOW	0x08
#define TX_Q_BUF_ADDR_HIGH	0x0c
#define TX_Q_FRAME_STAT		0x10
#define TX_Q_CSUM_START		0x14
#define TX_Q_CSUM_START_POS	0x18
#define TX_Q_RESERVED		0x1c
#define TX_Q_CUR_DESCR_LOW	0x20
#define TX_Q_DESCR_HIGH		0x24
#define TX_Q_CUR_ADDR_LOW	0x28
#define TX_Q_CUR_ADDR_HIGH	0x2c
#define TX_Q_CUR_BYTE_CNT	0x30
#define TX_Q_CTRL		0x34
#define TX_Q_FLAG		0x38
#define TX_Q_TEST1		0x3c
#define TX_Q_TEST2		0x40
#define TX_Q_TEST3		0x44

/* definition of flags in the queue control field */
#define RX_Q_CTRL_POLL_ON	0x00000080
#define RX_Q_CTRL_POLL_OFF	0x00000040
#define RX_Q_CTRL_STOP		0x00000020
#define RX_Q_CTRL_START		0x00000010
#define RX_Q_CTRL_CLR_I_PAR	0x00000008
#define RX_Q_CTRL_CLR_I_EOB	0x00000004
#define RX_Q_CTRL_CLR_I_EOF	0x00000002
#define RX_Q_CTRL_CLR_I_ERR	0x00000001

#define TX_Q_CTRL_POLL_ON	0x00000080
#define TX_Q_CTRL_POLL_OFF	0x00000040
#define TX_Q_CTRL_STOP		0x00000020
#define TX_Q_CTRL_START		0x00000010
#define TX_Q_CTRL_CLR_I_EOB	0x00000004
#define TX_Q_CTRL_CLR_I_EOF	0x00000002
#define TX_Q_CTRL_CLR_I_ERR	0x00000001


/* Interrupt bits in the interrupts source register **************************/
#define IRQ_HW_ERROR		0x80000000
#define IRQ_RESERVED		0x40000000
#define IRQ_PKT_TOUT_RX1	0x20000000
#define IRQ_PKT_TOUT_RX2	0x10000000
#define IRQ_PKT_TOUT_TX1	0x08000000
#define IRQ_PKT_TOUT_TX2	0x04000000
#define IRQ_I2C_READY		0x02000000
#define IRQ_SW			0x01000000
#define IRQ_EXTERNAL_REG	0x00800000
#define IRQ_TIMER		0x00400000
#define IRQ_MAC1		0x00200000
#define IRQ_LINK_SYNC_C_M1	0x00100000
#define IRQ_MAC2		0x00080000
#define IRQ_LINK_SYNC_C_M2	0x00040000
#define IRQ_EOB_RX1		0x00020000
#define IRQ_EOF_RX1		0x00010000
#define IRQ_CHK_RX1		0x00008000
#define IRQ_EOB_RX2		0x00004000
#define IRQ_EOF_RX2		0x00002000
#define IRQ_CHK_RX2		0x00001000
#define IRQ_EOB_SY_TX1		0x00000800
#define IRQ_EOF_SY_TX1		0x00000400
#define IRQ_CHK_SY_TX1		0x00000200
#define IRQ_EOB_AS_TX1		0x00000100
#define IRQ_EOF_AS_TX1		0x00000080
#define IRQ_CHK_AS_TX1		0x00000040
#define IRQ_EOB_SY_TX2		0x00000020
#define IRQ_EOF_SY_TX2		0x00000010
#define IRQ_CHK_SY_TX2		0x00000008
#define IRQ_EOB_AS_TX2		0x00000004
#define IRQ_EOF_AS_TX2		0x00000002
#define IRQ_CHK_AS_TX2		0x00000001

#define DRIVER_IRQS	(IRQ_SW | IRQ_EOF_RX1 | IRQ_EOF_RX2 | \
			IRQ_EOF_SY_TX1 | IRQ_EOF_AS_TX1 | \
			IRQ_EOF_SY_TX2 | IRQ_EOF_AS_TX2)

#define SPECIAL_IRQS	(IRQ_HW_ERROR | IRQ_PKT_TOUT_RX1 | IRQ_PKT_TOUT_RX2 | \
			IRQ_PKT_TOUT_TX1 | IRQ_PKT_TOUT_TX2 | \
			IRQ_I2C_READY | IRQ_EXTERNAL_REG | IRQ_TIMER | \
			IRQ_MAC1 | IRQ_LINK_SYNC_C_M1 | \
			IRQ_MAC2 | IRQ_LINK_SYNC_C_M2 | \
			IRQ_CHK_RX1 | IRQ_CHK_RX2 | \
			IRQ_CHK_SY_TX1 | IRQ_CHK_AS_TX1 | \
			IRQ_CHK_SY_TX2 | IRQ_CHK_AS_TX2)

#define IRQ_MASK	(IRQ_SW | IRQ_EOB_RX1 | IRQ_EOF_RX1 | \
			IRQ_EOB_RX2 | IRQ_EOF_RX2 | \
			IRQ_EOB_SY_TX1 | IRQ_EOF_SY_TX1 | \
			IRQ_EOB_AS_TX1 | IRQ_EOF_AS_TX1 | \
			IRQ_EOB_SY_TX2 | IRQ_EOF_SY_TX2 | \
			IRQ_EOB_AS_TX2 | IRQ_EOF_AS_TX2 | \
			IRQ_HW_ERROR | IRQ_PKT_TOUT_RX1 | IRQ_PKT_TOUT_RX2 | \
			IRQ_PKT_TOUT_TX1 | IRQ_PKT_TOUT_TX2 | \
			IRQ_I2C_READY | IRQ_EXTERNAL_REG | IRQ_TIMER | \
			IRQ_MAC1 | \
			IRQ_MAC2 | \
			IRQ_CHK_RX1 | IRQ_CHK_RX2 | \
			IRQ_CHK_SY_TX1 | IRQ_CHK_AS_TX1 | \
			IRQ_CHK_SY_TX2 | IRQ_CHK_AS_TX2)

#define IRQ_HWE_MASK	0x00000FFF /* enable all HW irqs */

typedef struct s_DevNet DEV_NET;

struct s_DevNet {
	int             PortNr;
	int             NetNr;
	int             Mtu;
	int             Up;
	SK_AC   *pAC;
};

typedef struct s_TxPort		TX_PORT;

struct s_TxPort {
	/* the transmit descriptor rings */
	caddr_t		pTxDescrRing;	/* descriptor area memory */
	SK_U64		VTxDescrRing;	/* descr. area bus virt. addr. */
	TXD		*pTxdRingHead;	/* Head of Tx rings */
	TXD		*pTxdRingTail;	/* Tail of Tx rings */
	TXD		*pTxdRingPrev;	/* descriptor sent previously */
	int		TxdRingFree;	/* # of free entrys */
#if 0
	spinlock_t	TxDesRingLock;	/* serialize descriptor accesses */
#endif
	caddr_t		HwAddr;		/* bmu registers address */
	int		PortIndex;	/* index number of port (0 or 1) */
};

typedef struct s_RxPort		RX_PORT;

struct s_RxPort {
	/* the receive descriptor rings */
	caddr_t		pRxDescrRing;	/* descriptor area memory */
	SK_U64		VRxDescrRing;   /* descr. area bus virt. addr. */
	RXD		*pRxdRingHead;	/* Head of Rx rings */
	RXD		*pRxdRingTail;	/* Tail of Rx rings */
	RXD		*pRxdRingPrev;	/* descriptor given to BMU previously */
	int		RxdRingFree;	/* # of free entrys */
#if 0
	spinlock_t	RxDesRingLock;	/* serialize descriptor accesses */
#endif
	int		RxFillLimit;	/* limit for buffers in ring */
	caddr_t		HwAddr;		/* bmu registers address */
	int		PortIndex;	/* index number of port (0 or 1) */
};

typedef struct s_PerStrm	PER_STRM;

#define SK_ALLOC_IRQ	0x00000001

/****************************************************************************
 * Per board structure / Adapter Context structure:
 *	Allocated within attach(9e) and freed within detach(9e).
 *	Contains all 'per device' necessary handles, flags, locks etc.:
 */
struct s_AC  {
	SK_GEINIT	GIni;		/* GE init struct */
	SK_PNMI		Pnmi;		/* PNMI data struct */
	SK_VPD		vpd;		/* vpd data struct */
	SK_QUEUE	Event;		/* Event queue */
	SK_HWT		Hwt;		/* Hardware Timer control struct */
	SK_TIMCTRL	Tim;		/* Software Timer control struct */
	SK_I2C		I2c;		/* I2C relevant data structure */
	SK_ADDR		Addr;		/* for Address module */
	SK_CSUM		Csum;		/* for checksum module */
	SK_RLMT		Rlmt;		/* for rlmt module */
#if 0
	spinlock_t	SlowPathLock;	/* Normal IRQ lock */
#endif
	SK_PNMI_STRUCT_DATA PnmiStruct;	/* structure to get all Pnmi-Data */
	int			RlmtMode;	/* link check mode to set */
	int			RlmtNets;	/* Number of nets */

	SK_IOC		IoBase;		/* register set of adapter */
	int		BoardLevel;	/* level of active hw init (0-2) */
	char		DeviceStr[80];	/* adapter string from vpd */
	SK_U32		AllocFlag;	/* flag allocation of resources */
#if 0
	struct pci_dev	*PciDev;	/* for access to pci config space */
	SK_U32		PciDevId;	/* pci device id */
#else
	int		PciDev;
#endif
	struct SK_NET_DEVICE	*dev[2];	/* pointer to device struct */
	char		Name[30];	/* driver name */
	struct SK_NET_DEVICE	*Next;		/* link all devices (for clearing) */
	int		RxBufSize;	/* length of receive buffers */
#if 0
	struct net_device_stats stats;	/* linux 'netstat -i' statistics */
#endif
	int		Index;		/* internal board index number */

	/* adapter RAM sizes for queues of active port */
	int		RxQueueSize;	/* memory used for receive queue */
	int		TxSQueueSize;	/* memory used for sync. tx queue */
	int		TxAQueueSize;	/* memory used for async. tx queue */

	int		PromiscCount;	/* promiscuous mode counter  */
	int		AllMultiCount;  /* allmulticast mode counter */
	int		MulticCount;	/* number of different MC    */
					/*  addresses for this board */
					/*  (may be more than HW can)*/

	int		HWRevision;	/* Hardware revision */
	int		ActivePort;	/* the active XMAC port */
	int		MaxPorts;		/* number of activated ports */
	int		TxDescrPerRing;	/* # of descriptors per tx ring */
	int		RxDescrPerRing;	/* # of descriptors per rx ring */

	caddr_t		pDescrMem;	/* Pointer to the descriptor area */
	dma_addr_t	pDescrMemDMA;	/* PCI DMA address of area */

	/* the port structures with descriptor rings */
	TX_PORT		TxPort[SK_MAX_MACS][2];
	RX_PORT		RxPort[SK_MAX_MACS];

	unsigned int	CsOfs1;		/* for checksum calculation */
	unsigned int	CsOfs2;		/* for checksum calculation */
	SK_U32		CsOfs;		/* for checksum calculation */

	SK_BOOL		CheckQueue;	/* check event queue soon */

	/* Only for tests */
	int		PortUp;
	int		PortDown;

};

#endif /* __INC_SKDRV2ND_H */
