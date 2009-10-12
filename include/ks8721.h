/*
 * NOTE:	MICREL ethernet Physical layer
 *
 * Version:	KS8721.h
 *
 * Authors:	Eric Benard (based on dm9161.h)
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

/* MICREL PHYSICAL LAYER TRANSCEIVER KS8721 */

#define	KS8721_BMCR		0
#define KS8721_BMSR		1
#define KS8721_PHYID1		2
#define KS8721_PHYID2		3
#define KS8721_ANAR		4
#define KS8721_ANLPAR		5
#define KS8721_ANER		6
#define KS8721_RECR		15
#define KS8721_MDINTR		27
#define KS8721_100BT		31

/* --Bit definitions: KS8721_BMCR */
#define KS8721_RESET		(1 << 15)
#define KS8721_LOOPBACK		(1 << 14)
#define KS8721_SPEED_SELECT	(1 << 13)
#define KS8721_AUTONEG		(1 << 12)
#define KS8721_POWER_DOWN	(1 << 11)
#define KS8721_ISOLATE		(1 << 10)
#define KS8721_RESTART_AUTONEG	(1 << 9)
#define KS8721_DUPLEX_MODE	(1 << 8)
#define KS8721_COLLISION_TEST	(1 << 7)
#define	KS8721_DISABLE		(1 << 0)

/*--Bit definitions: KS8721_BMSR */
#define KS8721_100BASE_T4	(1 << 15)
#define KS8721_100BASE_TX_FD	(1 << 14)
#define KS8721_100BASE_T4_HD	(1 << 13)
#define KS8721_10BASE_T_FD	(1 << 12)
#define KS8721_10BASE_T_HD	(1 << 11)
#define KS8721_MF_PREAMB_SUPPR	(1 << 6)
#define KS8721_AUTONEG_COMP	(1 << 5)
#define KS8721_REMOTE_FAULT	(1 << 4)
#define KS8721_AUTONEG_ABILITY	(1 << 3)
#define KS8721_LINK_STATUS	(1 << 2)
#define KS8721_JABBER_DETECT	(1 << 1)
#define KS8721_EXTEND_CAPAB	(1 << 0)

/*--Bit definitions: KS8721_PHYID */
#define KS8721_PHYID_OUI	0x0885
#define KS8721_LSB_MASK		0x3F

#define	KS8721BL_MODEL		0x21
#define	KS8721_MODELMASK	0x3F0
#define	KS8721BL_REV		0x9
#define KS8721_REVMASK		0xF

/*--Bit definitions: KS8721_ANAR, KS8721_ANLPAR */
#define KS8721_NP		(1 << 15)
#define KS8721_ACK		(1 << 14)
#define KS8721_RF		(1 << 13)
#define KS8721_PAUSE		(1 << 10)
#define KS8721_T4		(1 << 9)
#define KS8721_TX_FDX		(1 << 8)
#define KS8721_TX_HDX		(1 << 7)
#define KS8721_10_FDX		(1 << 6)
#define KS8721_10_HDX		(1 << 5)
#define KS8721_AN_IEEE_802_3	0x0001

/******************  function prototypes **********************/
unsigned int  ks8721_isphyconnected(AT91PS_EMAC p_mac);
unsigned char ks8721_getlinkspeed(AT91PS_EMAC p_mac);
unsigned char ks8721_autonegotiate(AT91PS_EMAC p_mac, int *status);
unsigned char ks8721_initphy(AT91PS_EMAC p_mac);
