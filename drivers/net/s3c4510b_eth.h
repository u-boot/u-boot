#ifndef __S3C4510B_ETH_H
#define __S3C4510B_ETH_H
/*
 * Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 * Curt Brune <curt@cucy.com>
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
 *
 * MODULE:        $Id:$
 * Description:   Ethernet interface
 * Runtime Env:   ARM7TDMI
 * Change History:
 *     03-02-04    Create (Curt Brune) curt@cucy.com
 *
 */

#define ETH_MAC_ADDR_SIZE           (6)    /*  dst,src addr is 6bytes each */
#define ETH_MaxTxFrames             (16)   /*  Max number of Tx Frames */

/*  Buffered DMA Receiver Control Register  */
#define ETH_BRxBRST     0x0000F  /*  BDMA Rx Burst Size * BRxBRST  */
				 /*  = Burst Data Size 16 */
#define ETH_BRxSTSKO    0x00020  /*  BDMA Rx Stop/Skip  Frame or Interrupt(=1)  */
				 /*  case of not OWNER the current Frame  */
#define ETH_BRxMAINC    0x00040  /*  BDMA Rx Memory Address Inc/Dec  */
#define ETH_BRxDIE      0x00080  /*  BDMA Rx Every Received Frame Interrupt Enable */
#define ETH_BRxNLIE     0x00100  /*  BDMA Rx NULL List Interrupt Enable  */
#define ETH_BRxNOIE     0x00200  /*  BDMA Rx Not Owner Interrupt Enable */
#define ETH_BRxMSOIE    0x00400  /*  BDMA Rx Maximum Size over Interrupr Enable  */
#define ETH_BRxLittle   0x00800  /*  BDMA Rx Big/Little Endian  */
#define ETH_BRxBig      0x00000  /*  BDMA Rx Big/Little Endian */
#define ETH_BRxWA01     0x01000  /*  BDMA Rx Word Alignment- one invalid byte  */
#define ETH_BRxWA10     0x02000  /*  BDMA Rx Word Alignment- two invalid byte */
#define ETH_BRxWA11     0x03000  /*  BDMA Rx Word Alignment- three invalid byte  */
#define ETH_BRxEn       0x04000  /*  BDMA Rx Enable */
#define ETH_BRxRS       0x08000  /*  BDMA Rx Reset */
#define ETH_RxEmpty     0x10000  /*  BDMA Rx Buffer empty interrupt  */
#define ETH_BRxEarly    0x20000  /*  BDMA Rx Early notify Interrupt */

/*  Buffered DMA Trasmit Control Register(BDMATXCON)  */
#define ETH_BTxBRST     0x0000F  /*  BDMA Tx Burst Size = 16  */
#define ETH_BTxSTSKO    0x00020  /*  BDMA Tx Stop/Skip Frame or Interrupt in case */
				 /*  of not Owner the current frame  */
#define ETH_BTxCPIE     0x00080  /*  BDMA Tx Complete to send control  */
				 /*  packet Enable */
#define ETH_BTxNOIE     0x00200  /*  BDMA Tx Buffer Not Owner */
#define ETH_BTxEmpty    0x00400  /*  BDMA Tx Buffer Empty Interrupt  */

/*  BDMA Tx buffer can be moved to the MAC Tx IO when the new frame comes in.  */
#define ETH_BTxMSL000   0x00000  /*  No wait to fill the BDMA  */
#define ETH_BTxMSL001   0x00800  /*  wait to fill 1/8 of the BDMA  */
#define ETH_BTxMSL010   0x01000  /*  wait to fill 2/8 of the BDMA */
#define ETH_BTxMSL011   0x01800  /*  wait to fill 3/8 of the BDMA */
#define ETH_BTxMSL100   0x02000  /*  wait to fill 4/8 of the BDMA */
#define ETH_BTxMSL101   0x02800  /*  wait to fill 5/8 of the BDMA */
#define ETH_BTxMSL110   0x03000  /*  wait to fill 6/8 of the BDMA */
#define ETH_BTxMSL111   0x03800  /*  wait to fill 7/8 of the BDMA */
#define ETH_BTxEn       0x04000  /*  BDMA Tx Enable  */
#define ETH_BTxRS       0x08000  /*  BDMA Tx Reset  */

/*  BDMA Status Register  */
#define ETH_S_BRxRDF    0x00001  /*  BDMA Rx Done Every Received Frame  */
#define ETH_S_BRxNL     0x00002  /*  BDMA Rx NULL List  */
#define ETH_S_BRxNO     0x00004  /*  BDMA Rx Not Owner  */
#define ETH_S_BRxMSO    0x00008  /*  BDMA Rx Maximum Size Over  */
#define ETH_S_BRxEmpty  0x00010  /*  BDMA Rx Buffer Empty  */
#define ETH_S_BRxSEarly 0x00020  /*  Early Notify  */
#define ETH_S_BRxFRF    0x00080  /*  One more frame data in BDMA receive buffer  */
#define ETH_S_BTxCCP    0x10000  /*  BDMA Tx Complete to send Control Packet  */
#define ETH_S_BTxNL     0x20000  /*  BDMA Tx Null List  */
#define ETH_S_BTxNO     0x40000  /*  BDMA Tx Not Owner */
#define ETH_S_BTxEmpty  0x100000 /*  BDMA Tx Buffer Empty  */

/*  MAC Control Register  */
#define ETH_HaltReg     0x0001   /*  stop transmission and reception  */
				 /*  after completion of any current packets  */
#define ETH_HaltImm     0x0002   /*  Stop transmission and reception immediately  */
#define ETH_SwReset     0x0004   /*  reset all Ethernet controller state machines */
				 /*  and FIFOs  */
#define ETH_FullDup     0x0008   /*  allow transmission to begin while reception */
				 /*  is occurring  */
#define ETH_MACLoop     0x0010   /*  MAC loopback */
#define ETH_ConnM00     0x0000   /*  Automatic-default  */
#define ETH_ConnM01     0x0020   /*  Force 10Mbits endec */
#define ETH_ConnM10     0x0040   /*  Force MII (rate determined by MII clock  */
#define ETH_MIIOFF      0x0040   /*  Force MII (rate determined by MII clock  */
#define ETH_Loop10      0x0080   /*  Loop 10Mbps  */
#define ETH_MissRoll    0x0400   /*  Missed error counter rolled over  */
#define ETH_MDCOFF      0x1000   /*  MII Station Management Clock Off */
#define ETH_EnMissRoll  0x2000   /*  Interrupt when missed error counter rolls  */
				 /*  over  */
#define ETH_Link10      0x8000   /*  Link status 10Mbps  */

/*  CAM control register(CAMCON)  */
#define ETH_StationAcc  0x0001   /*  Accept any packet with a unicast station  */
				 /*  address  */
#define ETH_GroupAcc    0x0002   /*  Accept any packet with multicast-group  */
				 /*  station address   */
#define ETH_BroadAcc    0x0004   /*  Accept any packet with a broadcast station */
				 /*  address  */
#define ETH_NegCAM      0x0008   /*  0: Accept packets CAM recognizes,  */
				 /*     reject others */
				 /*  1: reject packets CAM recognizes,  */
				 /*     accept others  */
#define ETH_CompEn      0x0010   /*  Compare Enable mode */

/*  Transmit Control Register(MACTXCON) */
#define ETH_TxEn        0x0001   /*  transmit Enable  */
#define ETH_TxHalt      0x0002   /*  Transmit Halt Request  */
#define ETH_NoPad       0x0004   /*  suppress Padding  */
#define ETH_NoCRC       0x0008   /*  Suppress CRC  */
#define ETH_FBack       0x0010   /*  Fast Back-off */
#define ETH_NoDef       0x0020   /*  Disable the defer counter */
#define ETH_SdPause     0x0040   /*  Send Pause */
#define ETH_MII10En     0x0080   /*  MII 10Mbps mode enable */
#define ETH_EnUnder     0x0100   /*  Enable Underrun */
#define ETH_EnDefer     0x0200   /*  Enable Deferral */
#define ETH_EnNCarr     0x0400   /*  Enable No Carrier  */
#define ETH_EnExColl    0x0800   /*  interrupt if 16 collision occur  */
				 /*  in the same packet  */
#define ETH_EnLateColl  0x1000   /*  interrupt if collision occurs after  */
				 /*  512 bit times(64 bytes times)  */
#define ETH_EnTxPar     0x2000   /*  interrupt if the MAC transmit FIFO  */
				 /*  has a parity error  */
#define ETH_EnComp      0x4000   /*  interrupt when the MAC transmits or  */
				 /*  discards one packet  */

/*  Transmit Status Register(MACTXSTAT) */
#define ETH_ExColl      0x0010   /*  Excessive collision  */
#define ETH_TxDeffered  0x0020   /*  set if 16 collisions occur for same packet */
#define ETH_Paused      0x0040   /*  packet waited because of pause during  */
				 /*  transmission  */
#define ETH_IntTx       0x0080   /*  set if transmission of packet causes an  */
				 /*  interrupt condiftion  */
#define ETH_Under       0x0100   /*  MAC transmit FIFO becomes empty during  */
				 /*  transmission  */
#define ETH_Defer       0x0200   /*  MAC defers for MAC deferral  */
#define ETH_NCarr       0x0400   /*  No carrier sense detected during the  */
				 /*  transmission of a packet  */
#define ETH_SQE         0x0800   /*  Signal Quality Error */
#define ETH_LateColl    0x1000   /*  a collision occures after 512 bit times  */
#define ETH_TxPar       0x2000   /*  MAC transmit FIFO has detected a parity error */
#define ETH_Comp        0x4000   /*  MAC transmit or discards one packet  */
#define ETH_TxHalted    0x8000   /*  Transmission was halted by clearing  */
				 /*  TxEn or Halt immedite  */

/*  Receive Control Register (MACRXCON)  */
#define ETH_RxEn        0x0001
#define ETH_RxHalt      0x0002
#define ETH_LongEn      0x0004
#define ETH_ShortEn     0x0008
#define ETH_StripCRC    0x0010
#define ETH_PassCtl     0x0020
#define ETH_IgnoreCRC   0x0040
#define ETH_EnAlign     0x0100
#define ETH_EnCRCErr    0x0200
#define ETH_EnOver      0x0400
#define ETH_EnLongErr   0x0800
#define ETH_EnRxPar     0x2000
#define ETH_EnGood      0x4000

/*  Receive Status Register(MACRXSTAT) */
#define ETH_MCtlRecd    0x0020
#define ETH_MIntRx      0x0040
#define ETH_MRx10Stat   0x0080
#define ETH_MAllignErr  0x0100
#define ETH_MCRCErr     0x0200
#define ETH_MOverflow   0x0400
#define ETH_MLongErr    0x0800
#define ETH_MRxPar      0x2000
#define ETH_MRxGood     0x4000
#define ETH_MRxHalted   0x8000

/*  type of ethernet packets */
#define ETH_TYPE_ARP  (0x0806)
#define ETH_TYPE_IP   (0x0800)

#define ETH_HDR_SIZE  (14)

/*  bit field for frame data pointer word */
typedef struct __BF_FrameDataPtr {
	u32 dataPtr:31;
	u32   owner: 1;
} BF_FrameDataPtr;

typedef union _FrameDataPtr {
	u32             ui;
	BF_FrameDataPtr bf;
} FrameDataPtr;

typedef struct __BF_TX_Options {
	u32    no_padding: 1;
	u32        no_crc: 1;
	u32  macTxIrqEnbl: 1;
	u32  littleEndian: 1;
	u32  frameDataDir: 1;
	u32   widgetAlign: 2;
	u32      reserved:25;
} BF_TX_Options;

typedef union _TX_Options {
	u32    ui;
	BF_TX_Options   bf;
} TX_Options;

typedef struct __BF_RX_Status {
	u32           len:16;	/*  frame length */
	u32     reserved1: 3;
	u32       overMax: 1;
	u32     reserved2: 1;
	u32       ctrlRcv: 1;
	u32         intRx: 1;
	u32      rx10stat: 1;
	u32      alignErr: 1;
	u32        crcErr: 1;
	u32      overFlow: 1;
	u32       longErr: 1;
	u32     reserved3: 1;
	u32     parityErr: 1;
	u32          good: 1;
	u32        halted: 1;
} BF_RX_Status;

typedef union _RX_Status {
	u32             ui;
	BF_RX_Status    bf;
} RX_Status;

typedef struct __BF_TX_Status {
	u32           len:16;	/*  frame length */
	u32     txCollCnt: 4;
	u32        exColl: 1;
	u32       txDefer: 1;
	u32        paused: 1;
	u32         intTx: 1;
	u32      underRun: 1;
	u32         defer: 1;
	u32     noCarrier: 1;
	u32         SQErr: 1;
	u32      lateColl: 1;
	u32     parityErr: 1;
	u32      complete: 1;
	u32        halted: 1;
} BF_TX_Status;

typedef union _TX_Status {
	u32    ui;
	BF_TX_Status    bf;
} TX_Status;

/*  TX descriptor structure  */
typedef struct __TX_FrameDescriptor {
	volatile FrameDataPtr  m_frameDataPtr;
	TX_Options                      m_opt;
	volatile TX_Status           m_status;
	struct __TX_FrameDescriptor *m_nextFD;
} TX_FrameDescriptor;

/*  RX descriptor structure  */
typedef struct __RX_FrameDescriptor {
	volatile FrameDataPtr  m_frameDataPtr;
	u32                        m_reserved;
	volatile RX_Status           m_status;
	struct __RX_FrameDescriptor *m_nextFD;
} RX_FrameDescriptor;

/*  MAC Frame Structure */
struct __MACFrame {
	u8     m_dstAddr[6];
	u8     m_srcAddr[6];
	u16  m_lengthOrType;
	u8  m_payload[1506];
} __attribute__ ((packed));

typedef struct __MACFrame MACFrame;

/* Ethernet Control block */
typedef struct __ETH {
	TX_FrameDescriptor   *m_curTX_FD; /*  pointer to current TX frame descriptor */
	TX_FrameDescriptor  *m_baseTX_FD; /*  pointer to base TX frame descriptor    */
	RX_FrameDescriptor   *m_curRX_FD; /*  pointer to current RX frame descriptor */
	RX_FrameDescriptor  *m_baseRX_FD; /*  pointer to base RX frame descriptor    */
	u8                      m_mac[6]; /*  pointer to our MAC address             */
} ETH;

#endif
