/*
 * Ethernet MAC Masks
 */

#ifndef __BFIN_PERIPHERAL_EMAC__
#define __BFIN_PERIPHERAL_EMAC__

/* EMAC_OPMODE Masks */
#define	RE		0x00000001	/* Receiver Enable */
#define	ASTP		0x00000002	/* Enable Automatic Pad Stripping On RX Frames */
#define	HU		0x00000010	/* Hash Filter Unicast Address */
#define	HM		0x00000020	/* Hash Filter Multicast Address */
#define	PAM		0x00000040	/* Pass-All-Multicast Mode Enable */
#define	PR		0x00000080	/* Promiscuous Mode Enable */
#define	IFE		0x00000100	/* Inverse Filtering Enable */
#define	DBF		0x00000200	/* Disable Broadcast Frame Reception */
#define	PBF		0x00000400	/* Pass Bad Frames Enable */
#define	PSF		0x00000800	/* Pass Short Frames Enable */
#define	RAF		0x00001000	/* Receive-All Mode */
#define	TE		0x00010000	/* Transmitter Enable */
#define	DTXPAD		0x00020000	/* Disable Automatic TX Padding */
#define	DTXCRC		0x00040000	/* Disable Automatic TX CRC Generation */
#define	DC		0x00080000	/* Deferral Check */
#define	BOLMT		0x00300000	/* Back-Off Limit */
#define	BOLMT_10	0x00000000	/* 10-bit range */
#define	BOLMT_8		0x00100000	/* 8-bit range */
#define	BOLMT_4		0x00200000	/* 4-bit range */
#define	BOLMT_1		0x00300000	/* 1-bit range */
#define	DRTY		0x00400000	/* Disable TX Retry On Collision */
#define	LCTRE		0x00800000	/* Enable TX Retry On Late Collision */
#define	RMII		0x01000000	/* RMII/MII* Mode */
#define	RMII_10		0x02000000	/* Speed Select for RMII Port (10MBit/100MBit*) */
#define	FDMODE		0x04000000	/* Duplex Mode Enable (Full/Half*) */
#define	LB		0x08000000	/* Internal Loopback Enable */
#define	DRO		0x10000000	/* Disable Receive Own Frames (Half-Duplex Mode) */

/* EMAC_STAADD Masks */
#define	STABUSY		0x00000001	/* Initiate Station Mgt Reg Access / STA Busy Stat */
#define	STAOP		0x00000002	/* Station Management Operation Code (Write/Read*) */
#define	STADISPRE	0x00000004	/* Disable Preamble Generation */
#define	STAIE		0x00000008	/* Station Mgt. Transfer Done Interrupt Enable */
#define	REGAD		0x000007C0	/* STA Register Address */
#define	PHYAD		0x0000F800	/* PHY Device Address */

#define	SET_REGAD(x)	(((x) & 0x1F) <<  6)	/* Set STA Register Address */
#define	SET_PHYAD(x)	(((x) & 0x1F) << 11)	/* Set PHY Device Address */

/* EMAC_STADAT Mask */
#define	STADATA		0x0000FFFF	/* Station Management Data */

/* EMAC_FLC Masks */
#define	FLCBUSY		0x00000001	/* Send Flow Ctrl Frame / Flow Ctrl Busy Status */
#define	FLCE		0x00000002	/* Flow Control Enable */
#define	PCF		0x00000004	/* Pass Control Frames */
#define	BKPRSEN		0x00000008	/* Enable Backpressure */
#define	FLCPAUSE	0xFFFF0000	/* Pause Time */

#define	SET_FLCPAUSE(x)	(((x) & 0xFFFF) << 16)	/* Set Pause Time */

/* EMAC_WKUP_CTL Masks */
#define	CAPWKFRM	0x00000001	/* Capture Wake-Up Frames */
#define	MPKE		0x00000002	/* Magic Packet Enable */
#define	RWKE		0x00000004	/* Remote Wake-Up Frame Enable */
#define	GUWKE		0x00000008	/* Global Unicast Wake Enable */
#define	MPKS		0x00000020	/* Magic Packet Received Status */
#define	RWKS		0x00000F00	/* Wake-Up Frame Received Status, Filters 3:0 */

/* EMAC_WKUP_FFCMD Masks */
#define	WF0_E		0x00000001	/* Enable Wake-Up Filter 0 */
#define	WF0_T		0x00000008	/* Wake-Up Filter 0 Addr Type (Multicast/Unicast*) */
#define	WF1_E		0x00000100	/* Enable Wake-Up Filter 1 */
#define	WF1_T		0x00000800	/* Wake-Up Filter 1 Addr Type (Multicast/Unicast*) */
#define	WF2_E		0x00010000	/* Enable Wake-Up Filter 2 */
#define	WF2_T		0x00080000	/* Wake-Up Filter 2 Addr Type (Multicast/Unicast*) */
#define	WF3_E		0x01000000	/* Enable Wake-Up Filter 3 */
#define	WF3_T		0x08000000	/* Wake-Up Filter 3 Addr Type (Multicast/Unicast*) */

/* EMAC_WKUP_FFOFF Masks */
#define	WF0_OFF		0x000000FF	/* Wake-Up Filter 0 Pattern Offset */
#define	WF1_OFF		0x0000FF00	/* Wake-Up Filter 1 Pattern Offset */
#define	WF2_OFF		0x00FF0000	/* Wake-Up Filter 2 Pattern Offset */
#define	WF3_OFF		0xFF000000	/* Wake-Up Filter 3 Pattern Offset */

#define	SET_WF0_OFF(x)	(((x) & 0xFF) <<  0)	/* Set Wake-Up Filter 0 Byte Offset */
#define	SET_WF1_OFF(x)	(((x) & 0xFF) <<  8)	/* Set Wake-Up Filter 1 Byte Offset */
#define	SET_WF2_OFF(x)	(((x) & 0xFF) << 16)	/* Set Wake-Up Filter 2 Byte Offset */
#define	SET_WF3_OFF(x)	(((x) & 0xFF) << 24)	/* Set Wake-Up Filter 3 Byte Offset */
/* Set ALL Offsets */
#define	SET_WF_OFFS(x0,x1,x2,x3)	(SET_WF0_OFF((x0))|SET_WF1_OFF((x1))|SET_WF2_OFF((x2))|SET_WF3_OFF((x3)))

/* EMAC_WKUP_FFCRC0 Masks */
#define	WF0_CRC		0x0000FFFF	/* Wake-Up Filter 0 Pattern CRC */
#define	WF1_CRC		0xFFFF0000	/* Wake-Up Filter 1 Pattern CRC */

#define	SET_WF0_CRC(x)	(((x) & 0xFFFF) << 0)	/* Set Wake-Up Filter 0 Target CRC */
#define	SET_WF1_CRC(x)	(((x) & 0xFFFF) << 16)	/* Set Wake-Up Filter 1 Target CRC */

/* EMAC_WKUP_FFCRC1 Masks */
#define	WF2_CRC		0x0000FFFF	/* Wake-Up Filter 2 Pattern CRC */
#define	WF3_CRC		0xFFFF0000	/* Wake-Up Filter 3 Pattern CRC */

#define	SET_WF2_CRC(x)	(((x) & 0xFFFF) << 0)	/* Set Wake-Up Filter 2 Target CRC */
#define	SET_WF3_CRC(x)	(((x) & 0xFFFF) << 16)	/* Set Wake-Up Filter 3 Target CRC */

/* EMAC_SYSCTL Masks */
#define	PHYIE		0x00000001	/* PHY_INT Interrupt Enable */
#define	RXDWA		0x00000002	/* Receive Frame DMA Word Alignment (Odd/Even*) */
#define	RXCKS		0x00000004	/* Enable RX Frame TCP/UDP Checksum Computation */
#define	MDCDIV		0x00003F00	/* SCLK:MDC Clock Divisor [MDC=SCLK/(2*(N+1))] */

#define	SET_MDCDIV(x)	(((x) & 0x3F) << 8)	/* Set MDC Clock Divisor */

/* EMAC_SYSTAT Masks */
#define	PHYINT		0x00000001	/* PHY_INT Interrupt Status */
#define	MMCINT		0x00000002	/* MMC Counter Interrupt Status */
#define	RXFSINT		0x00000004	/* RX Frame-Status Interrupt Status */
#define	TXFSINT		0x00000008	/* TX Frame-Status Interrupt Status */
#define	WAKEDET		0x00000010	/* Wake-Up Detected Status */
#define	RXDMAERR	0x00000020	/* RX DMA Direction Error Status */
#define	TXDMAERR	0x00000040	/* TX DMA Direction Error Status */
#define	STMDONE		0x00000080	/* Station Mgt. Transfer Done Interrupt Status */

/* EMAC_RX_STAT, EMAC_RX_STKY, and EMAC_RX_IRQE Masks */
#define	RX_FRLEN	0x000007FF	/* Frame Length In Bytes */
#define	RX_COMP		0x00001000	/* RX Frame Complete */
#define	RX_OK		0x00002000	/* RX Frame Received With No Errors */
#define	RX_LONG		0x00004000	/* RX Frame Too Long Error */
#define	RX_ALIGN	0x00008000	/* RX Frame Alignment Error */
#define	RX_CRC		0x00010000	/* RX Frame CRC Error */
#define	RX_LEN		0x00020000	/* RX Frame Length Error */
#define	RX_FRAG		0x00040000	/* RX Frame Fragment Error */
#define	RX_ADDR		0x00080000	/* RX Frame Address Filter Failed Error */
#define	RX_DMAO		0x00100000	/* RX Frame DMA Overrun Error */
#define	RX_PHY		0x00200000	/* RX Frame PHY Error */
#define	RX_LATE		0x00400000	/* RX Frame Late Collision Error */
#define	RX_RANGE	0x00800000	/* RX Frame Length Field Out of Range Error */
#define	RX_MULTI	0x01000000	/* RX Multicast Frame Indicator */
#define	RX_BROAD	0x02000000	/* RX Broadcast Frame Indicator */
#define	RX_CTL		0x04000000	/* RX Control Frame Indicator */
#define	RX_UCTL		0x08000000	/* Unsupported RX Control Frame Indicator */
#define	RX_TYPE		0x10000000	/* RX Typed Frame Indicator */
#define	RX_VLAN1	0x20000000	/* RX VLAN1 Frame Indicator */
#define	RX_VLAN2	0x40000000	/* RX VLAN2 Frame Indicator */
#define	RX_ACCEPT	0x80000000	/* RX Frame Accepted Indicator */

/*  EMAC_TX_STAT, EMAC_TX_STKY, and EMAC_TX_IRQE Masks */
#define	TX_COMP		0x00000001	/* TX Frame Complete */
#define	TX_OK		0x00000002	/* TX Frame Sent With No Errors */
#define	TX_ECOLL	0x00000004	/* TX Frame Excessive Collision Error */
#define	TX_LATE		0x00000008	/* TX Frame Late Collision Error */
#define	TX_DMAU		0x00000010	/* TX Frame DMA Underrun Error (STAT) */
#define	TX_MACE		0x00000010	/* Internal MAC Error Detected (STKY and IRQE) */
#define	TX_EDEFER	0x00000020	/* TX Frame Excessive Deferral Error */
#define	TX_BROAD	0x00000040	/* TX Broadcast Frame Indicator */
#define	TX_MULTI	0x00000080	/* TX Multicast Frame Indicator */
#define	TX_CCNT		0x00000F00	/* TX Frame Collision Count */
#define	TX_DEFER	0x00001000	/* TX Frame Deferred Indicator */
#define	TX_CRS		0x00002000	/* TX Frame Carrier Sense Not Asserted Error */
#define	TX_LOSS		0x00004000	/* TX Frame Carrier Lost During TX Error */
#define	TX_RETRY	0x00008000	/* TX Frame Successful After Retry */
#define	TX_FRLEN	0x07FF0000	/* TX Frame Length (Bytes) */

/* EMAC_MMC_CTL Masks */
#define	RSTC		0x00000001	/* Reset All Counters */
#define	CROLL		0x00000002	/* Counter Roll-Over Enable */
#define	CCOR		0x00000004	/* Counter Clear-On-Read Mode Enable */
#define	MMCE		0x00000008	/* Enable MMC Counter Operation */

/* EMAC_MMC_RIRQS and EMAC_MMC_RIRQE Masks */
#define	RX_OK_CNT	0x00000001	/* RX Frames Received With No Errors */
#define	RX_FCS_CNT	0x00000002	/* RX Frames W/Frame Check Sequence Errors */
#define	RX_ALIGN_CNT	0x00000004	/* RX Frames With Alignment Errors */
#define	RX_OCTET_CNT	0x00000008	/* RX Octets Received OK */
#define	RX_LOST_CNT	0x00000010	/* RX Frames Lost Due To Internal MAC RX Error */
#define	RX_UNI_CNT	0x00000020	/* Unicast RX Frames Received OK */
#define	RX_MULTI_CNT	0x00000040	/* Multicast RX Frames Received OK */
#define	RX_BROAD_CNT	0x00000080	/* Broadcast RX Frames Received OK */
#define	RX_IRL_CNT	0x00000100	/* RX Frames With In-Range Length Errors */
#define	RX_ORL_CNT	0x00000200	/* RX Frames With Out-Of-Range Length Errors */
#define	RX_LONG_CNT	0x00000400	/* RX Frames With Frame Too Long Errors */
#define	RX_MACCTL_CNT	0x00000800	/* MAC Control RX Frames Received */
#define	RX_OPCODE_CTL	0x00001000	/* Unsupported Op-Code RX Frames Received */
#define	RX_PAUSE_CNT	0x00002000	/* PAUSEMAC Control RX Frames Received */
#define	RX_ALLF_CNT	0x00004000	/* All RX Frames Received */
#define	RX_ALLO_CNT	0x00008000	/* All RX Octets Received */
#define	RX_TYPED_CNT	0x00010000	/* Typed RX Frames Received */
#define	RX_SHORT_CNT	0x00020000	/* RX Frame Fragments (< 64 Bytes) Received */
#define	RX_EQ64_CNT	0x00040000	/* 64-Byte RX Frames Received */
#define	RX_LT128_CNT	0x00080000	/* 65-127-Byte RX Frames Received */
#define	RX_LT256_CNT	0x00100000	/* 128-255-Byte RX Frames Received */
#define	RX_LT512_CNT	0x00200000	/* 256-511-Byte RX Frames Received */
#define	RX_LT1024_CNT	0x00400000	/* 512-1023-Byte RX Frames Received */
#define	RX_GE1024_CNT	0x00800000	/* 1024-Max-Byte RX Frames Received */

/* EMAC_MMC_TIRQS and EMAC_MMC_TIRQE Masks */
#define	TX_OK_CNT	0x00000001	/* TX Frames Sent OK */
#define	TX_SCOLL_CNT	0x00000002	/* TX Frames With Single Collisions */
#define	TX_MCOLL_CNT	0x00000004	/* TX Frames With Multiple Collisions */
#define	TX_OCTET_CNT	0x00000008	/* TX Octets Sent OK */
#define	TX_DEFER_CNT	0x00000010	/* TX Frames With Deferred Transmission */
#define	TX_LATE_CNT	0x00000020	/* TX Frames With Late Collisions */
#define	TX_ABORTC_CNT	0x00000040	/* TX Frames Aborted Due To Excess Collisions */
#define	TX_LOST_CNT	0x00000080	/* TX Frames Lost Due To Internal MAC TX Error */
#define	TX_CRS_CNT	0x00000100	/* TX Frames With Carrier Sense Errors */
#define	TX_UNI_CNT	0x00000200	/* Unicast TX Frames Sent */
#define	TX_MULTI_CNT	0x00000400	/* Multicast TX Frames Sent */
#define	TX_BROAD_CNT	0x00000800	/* Broadcast TX Frames Sent */
#define	TX_EXDEF_CTL	0x00001000	/* TX Frames With Excessive Deferral */
#define	TX_MACCTL_CNT	0x00002000	/* MAC Control TX Frames Sent */
#define	TX_ALLF_CNT	0x00004000	/* All TX Frames Sent */
#define	TX_ALLO_CNT	0x00008000	/* All TX Octets Sent */
#define	TX_EQ64_CNT	0x00010000	/* 64-Byte TX Frames Sent */
#define	TX_LT128_CNT	0x00020000	/* 65-127-Byte TX Frames Sent */
#define	TX_LT256_CNT	0x00040000	/* 128-255-Byte TX Frames Sent */
#define	TX_LT512_CNT	0x00080000	/* 256-511-Byte TX Frames Sent */
#define	TX_LT1024_CNT	0x00100000	/* 512-1023-Byte TX Frames Sent */
#define	TX_GE1024_CNT	0x00200000	/* 1024-Max-Byte TX Frames Sent */
#define	TX_ABORT_CNT	0x00400000	/* TX Frames Aborted */

/*default value for EMAC_VLANx reg*/
#define EMAC_VLANX_DEF_VAL 0xFFFF

#endif
