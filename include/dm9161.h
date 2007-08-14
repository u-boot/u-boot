/*
 * NOTE:	DAVICOM ethernet Physical layer
 *
 * Version:	@(#)DM9161.h	1.0.0	01/10/2001
 *
 * Authors:	ATMEL Rousset
 *
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */


/* DAVICOM PHYSICAL LAYER TRANSCEIVER DM9161 */

#define	DM9161_BMCR 		0	/* Basic Mode Control Register */
#define DM9161_BMSR		1	/* Basic Mode Status Register */
#define DM9161_PHYID1		2	/* PHY Idendifier Register 1 */
#define DM9161_PHYID2		3	/* PHY Idendifier Register 2 */
#define DM9161_ANAR		4	/* Auto_Negotiation Advertisement Register  */
#define DM9161_ANLPAR		5	/* Auto_negotiation Link Partner Ability Register */
#define DM9161_ANER		6	/* Auto-negotiation Expansion Register  */
#define DM9161_DSCR		16	/* Specified Configuration Register */
#define DM9161_DSCSR		17	/* Specified Configuration and Status Register */
#define DM9161_10BTCSR		18	/* 10BASE-T Configuration and Satus Register */
#define DM9161_MDINTR		21	/* Specified Interrupt Register */
#define DM9161_RECR		22	/* Specified Receive Error Counter Register */
#define DM9161_DISCR		23	/* Specified Disconnect Counter Register */
#define DM9161_RLSR		24	/* Hardware Reset Latch State Register */


/* --Bit definitions: DM9161_BMCR */
#define DM9161_RESET   	         (1 << 15)	/* 1= Software Reset; 0=Normal Operation */
#define DM9161_LOOPBACK	         (1 << 14)	/* 1=loopback Enabled; 0=Normal Operation */
#define DM9161_SPEED_SELECT      (1 << 13)	/* 1=100Mbps; 0=10Mbps */
#define DM9161_AUTONEG	         (1 << 12)
#define DM9161_POWER_DOWN        (1 << 11)
#define DM9161_ISOLATE           (1 << 10)
#define DM9161_RESTART_AUTONEG   (1 << 9)
#define DM9161_DUPLEX_MODE       (1 << 8)
#define DM9161_COLLISION_TEST    (1 << 7)

/*--Bit definitions: DM9161_BMSR */
#define DM9161_100BASE_TX        (1 << 15)
#define DM9161_100BASE_TX_FD     (1 << 14)
#define DM9161_100BASE_TX_HD     (1 << 13)
#define DM9161_10BASE_T_FD       (1 << 12)
#define DM9161_10BASE_T_HD       (1 << 11)
#define DM9161_MF_PREAMB_SUPPR   (1 << 6)
#define DM9161_AUTONEG_COMP      (1 << 5)
#define DM9161_REMOTE_FAULT      (1 << 4)
#define DM9161_AUTONEG_ABILITY   (1 << 3)
#define DM9161_LINK_STATUS       (1 << 2)
#define DM9161_JABBER_DETECT     (1 << 1)
#define DM9161_EXTEND_CAPAB      (1 << 0)

/*--definitions: DM9161_PHYID1 */
#define DM9161_PHYID1_OUI	 0x606E
#define DM9161_LSB_MASK	         0x3F

/*--Bit definitions: DM9161_ANAR, DM9161_ANLPAR */
#define DM9161_NP               (1 << 15)
#define DM9161_ACK              (1 << 14)
#define DM9161_RF               (1 << 13)
#define DM9161_FCS              (1 << 10)
#define DM9161_T4               (1 << 9)
#define DM9161_TX_FDX           (1 << 8)
#define DM9161_TX_HDX           (1 << 7)
#define DM9161_10_FDX           (1 << 6)
#define DM9161_10_HDX           (1 << 5)
#define DM9161_AN_IEEE_802_3	0x0001

/*--Bit definitions: DM9161_ANER */
#define DM9161_PDF              (1 << 4)
#define DM9161_LP_NP_ABLE       (1 << 3)
#define DM9161_NP_ABLE          (1 << 2)
#define DM9161_PAGE_RX          (1 << 1)
#define DM9161_LP_AN_ABLE       (1 << 0)

/*--Bit definitions: DM9161_DSCR */
#define DM9161_BP4B5B           (1 << 15)
#define DM9161_BP_SCR           (1 << 14)
#define DM9161_BP_ALIGN         (1 << 13)
#define DM9161_BP_ADPOK         (1 << 12)
#define DM9161_REPEATER         (1 << 11)
#define DM9161_TX               (1 << 10)
#define DM9161_RMII_ENABLE      (1 << 8)
#define DM9161_F_LINK_100       (1 << 7)
#define DM9161_SPLED_CTL        (1 << 6)
#define DM9161_COLLED_CTL       (1 << 5)
#define DM9161_RPDCTR_EN        (1 << 4)
#define DM9161_SM_RST           (1 << 3)
#define DM9161_MFP SC           (1 << 2)
#define DM9161_SLEEP            (1 << 1)
#define DM9161_RLOUT            (1 << 0)

/*--Bit definitions: DM9161_DSCSR */
#define DM9161_100FDX           (1 << 15)
#define DM9161_100HDX           (1 << 14)
#define DM9161_10FDX            (1 << 13)
#define DM9161_10HDX            (1 << 12)

/*--Bit definitions: DM9161_10BTCSR */
#define DM9161_LP_EN           (1 << 14)
#define DM9161_HBE             (1 << 13)
#define DM9161_SQUELCH         (1 << 12)
#define DM9161_JABEN           (1 << 11)
#define DM9161_10BT_SER        (1 << 10)
#define DM9161_POLR            (1 << 0)


/*--Bit definitions: DM9161_MDINTR */
#define DM9161_INTR_PEND       (1 << 15)
#define DM9161_FDX_MASK        (1 << 11)
#define DM9161_SPD_MASK        (1 << 10)
#define DM9161_LINK_MASK       (1 << 9)
#define DM9161_INTR_MASK       (1 << 8)
#define DM9161_FDX_CHANGE      (1 << 4)
#define DM9161_SPD_CHANGE      (1 << 3)
#define DM9161_LINK_CHANGE     (1 << 2)
#define DM9161_INTR_STATUS     (1 << 0)


/******************  function prototypes **********************/
unsigned int  dm9161_IsPhyConnected(AT91PS_EMAC p_mac);
unsigned char dm9161_GetLinkSpeed(AT91PS_EMAC p_mac);
unsigned char dm9161_AutoNegotiate(AT91PS_EMAC p_mac, int *status);
unsigned char dm9161_InitPhy(AT91PS_EMAC p_mac);
