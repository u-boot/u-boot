/*
 * Broadcom BCM5221 Ethernet PHY
 *
 * (C) Copyright 2005 REA Elektronik GmbH <www.rea.de>
 * Anders Larsen <alarsen@rea.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define	BCM5221_BMCR		0	/* Basic Mode Control Register */
#define BCM5221_BMSR		1	/* Basic Mode Status Register */
#define BCM5221_PHYID1		2	/* PHY Identifier Register 1 */
#define BCM5221_PHYID2		3	/* PHY Identifier Register 2 */
#define BCM5221_ANAR		4	/* Auto-negotiation Advertisement Register  */
#define BCM5221_ANLPAR		5	/* Auto-negotiation Link Partner Ability Register */
#define BCM5221_ANER		6	/* Auto-negotiation Expansion Register  */
#define BCM5221_ACSR		24	/* Auxiliary Control/Status Register */
#define BCM5221_INTR		26	/* Interrupt Register */

/* --Bit definitions: BCM5221_BMCR */
#define BCM5221_RESET		(1 << 15)	/* 1= Software Reset; 0=Normal Operation */
#define BCM5221_LOOPBACK	(1 << 14)	/* 1=loopback Enabled; 0=Normal Operation */
#define BCM5221_SPEED_SELECT	(1 << 13)	/* 1=100Mbps; 0=10Mbps */
#define BCM5221_AUTONEG		(1 << 12)
#define BCM5221_POWER_DOWN	(1 << 11)
#define BCM5221_ISOLATE		(1 << 10)
#define BCM5221_RESTART_AUTONEG	(1 << 9)
#define BCM5221_DUPLEX_MODE	(1 << 8)
#define BCM5221_COLLISION_TEST	(1 << 7)

/*--Bit definitions: BCM5221_BMSR */
#define BCM5221_100BASE_T4	(1 << 15)
#define BCM5221_100BASE_TX_FD	(1 << 14)
#define BCM5221_100BASE_TX_HD	(1 << 13)
#define BCM5221_10BASE_T_FD	(1 << 12)
#define BCM5221_10BASE_T_HD	(1 << 11)
#define BCM5221_MF_PREAMB_SUPPR	(1 << 6)
#define BCM5221_AUTONEG_COMP	(1 << 5)
#define BCM5221_REMOTE_FAULT	(1 << 4)
#define BCM5221_AUTONEG_ABILITY	(1 << 3)
#define BCM5221_LINK_STATUS	(1 << 2)
#define BCM5221_JABBER_DETECT	(1 << 1)
#define BCM5221_EXTEND_CAPAB	(1 << 0)

/*--definitions: BCM5221_PHYID1 */
#define BCM5221_PHYID1_OUI	0x1018
#define BCM5221_LSB_MASK	0x3F

/*--Bit definitions: BCM5221_ANAR, BCM5221_ANLPAR */
#define BCM5221_NP		(1 << 15)
#define BCM5221_ACK		(1 << 14)
#define BCM5221_RF		(1 << 13)
#define BCM5221_FCS		(1 << 10)
#define BCM5221_T4		(1 << 9)
#define BCM5221_TX_FDX		(1 << 8)
#define BCM5221_TX_HDX		(1 << 7)
#define BCM5221_10_FDX		(1 << 6)
#define BCM5221_10_HDX		(1 << 5)
#define BCM5221_AN_IEEE_802_3	0x0001

/*--Bit definitions: BCM5221_ANER */
#define BCM5221_PDF		(1 << 4)
#define BCM5221_LP_NP_ABLE	(1 << 3)
#define BCM5221_NP_ABLE		(1 << 2)
#define BCM5221_PAGE_RX		(1 << 1)
#define BCM5221_LP_AN_ABLE	(1 << 0)

/*--Bit definitions: BCM5221_ACSR */
#define BCM5221_100		(1 << 1)
#define BCM5221_FDX		(1 << 0)

/*--Bit definitions: BCM5221_INTR */
#define BCM5221_FDX_LED		(1 << 15)
#define BCM5221_INTR_ENABLE	(1 << 14)
#define BCM5221_FDX_MASK	(1 << 11)
#define BCM5221_SPD_MASK	(1 << 10)
#define BCM5221_LINK_MASK	(1 << 9)
#define BCM5221_INTR_MASK	(1 << 8)
#define BCM5221_FDX_CHG		(1 << 3)
#define BCM5221_SPD_CHG		(1 << 2)
#define BCM5221_LINK_CHG	(1 << 1)
#define BCM5221_INTR_STATUS	(1 << 0)

/******************  function prototypes **********************/
unsigned int  bcm5221_IsPhyConnected(AT91PS_EMAC p_mac);
unsigned char bcm5221_GetLinkSpeed(AT91PS_EMAC p_mac);
unsigned char bcm5221_AutoNegotiate(AT91PS_EMAC p_mac, int *status);
unsigned char bcm5221_InitPhy(AT91PS_EMAC p_mac);
