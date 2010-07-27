/*----------------------------------------------------------------------------+
|   This source code is dual-licensed.  You may use it under the terms of the
|   GNU General Public License version 2, or under the license below.
|
|	This source code has been made available to you by IBM on an AS-IS
|	basis.	Anyone receiving this source is licensed under IBM
|	copyrights to use it in any way he or she deems fit, including
|	copying it, modifying it, compiling it, and redistributing it either
|	with or without modifications.	No license under IBM patents or
|	patent applications is to be implied by the copyright license.
|
|	Any user of this software should understand that IBM cannot provide
|	technical support for this software and will not be responsible for
|	any consequences resulting from the use of this software.
|
|	Any person who transfers this source code or any derivative work
|	must include the IBM copyright notice, this paragraph, and the
|	preceding two paragraphs in the transferred software.
|
|	COPYRIGHT   I B M   CORPORATION 1999
|	LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
|
|   Additions (C) Copyright 2009 Industrie Dial Face S.p.A.
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|
|  File Name:	miiphy.h
|
|  Function:	Include file defining PHY registers.
|
|  Author:	Mark Wisner
|
+----------------------------------------------------------------------------*/
#ifndef _miiphy_h_
#define _miiphy_h_

#include <net.h>

int miiphy_read (const char *devname, unsigned char addr, unsigned char reg,
		 unsigned short *value);
int miiphy_write (const char *devname, unsigned char addr, unsigned char reg,
		  unsigned short value);
int miiphy_info (const char *devname, unsigned char addr, unsigned int *oui,
		 unsigned char *model, unsigned char *rev);
int miiphy_reset (const char *devname, unsigned char addr);
int miiphy_speed (const char *devname, unsigned char addr);
int miiphy_duplex (const char *devname, unsigned char addr);
int miiphy_is_1000base_x (const char *devname, unsigned char addr);
#ifdef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
int miiphy_link (const char *devname, unsigned char addr);
#endif

void miiphy_init (void);

void miiphy_register (const char *devname,
		      int (*read) (const char *devname, unsigned char addr,
				   unsigned char reg, unsigned short *value),
		      int (*write) (const char *devname, unsigned char addr,
				    unsigned char reg, unsigned short value));

int miiphy_set_current_dev (const char *devname);
const char *miiphy_get_current_dev (void);

void miiphy_listdev (void);

#ifdef CONFIG_BITBANGMII

#define BB_MII_DEVNAME	"bb_miiphy"

struct bb_miiphy_bus {
	char name[NAMESIZE];
	int (*init)(struct bb_miiphy_bus *bus);
	int (*mdio_active)(struct bb_miiphy_bus *bus);
	int (*mdio_tristate)(struct bb_miiphy_bus *bus);
	int (*set_mdio)(struct bb_miiphy_bus *bus, int v);
	int (*get_mdio)(struct bb_miiphy_bus *bus, int *v);
	int (*set_mdc)(struct bb_miiphy_bus *bus, int v);
	int (*delay)(struct bb_miiphy_bus *bus);
#ifdef CONFIG_BITBANGMII_MULTI
	void *priv;
#endif
};

extern struct bb_miiphy_bus bb_miiphy_buses[];
extern int bb_miiphy_buses_num;

void bb_miiphy_init (void);
int bb_miiphy_read (const char *devname, unsigned char addr,
		    unsigned char reg, unsigned short *value);
int bb_miiphy_write (const char *devname, unsigned char addr,
		     unsigned char reg, unsigned short value);
#endif

/* phy seed setup */
#define AUTO			99
#define _1000BASET		1000
#define _100BASET		100
#define _10BASET		10
#define HALF			22
#define FULL			44

/* phy register offsets */
#define PHY_BMCR		0x00
#define PHY_BMSR		0x01
#define PHY_PHYIDR1		0x02
#define PHY_PHYIDR2		0x03
#define PHY_ANAR		0x04
#define PHY_ANLPAR		0x05
#define PHY_ANER		0x06
#define PHY_ANNPTR		0x07
#define PHY_ANLPNP		0x08
#define PHY_1000BTCR		0x09
#define PHY_1000BTSR		0x0A
#define PHY_EXSR		0x0F
#define PHY_PHYSTS		0x10
#define PHY_MIPSCR		0x11
#define PHY_MIPGSR		0x12
#define PHY_DCR			0x13
#define PHY_FCSCR		0x14
#define PHY_RECR		0x15
#define PHY_PCSR		0x16
#define PHY_LBR			0x17
#define PHY_10BTSCR		0x18
#define PHY_PHYCTRL		0x19

/* PHY BMCR */
#define PHY_BMCR_RESET		0x8000
#define PHY_BMCR_LOOP		0x4000
#define PHY_BMCR_100MB		0x2000
#define PHY_BMCR_AUTON		0x1000
#define PHY_BMCR_POWD		0x0800
#define PHY_BMCR_ISO		0x0400
#define PHY_BMCR_RST_NEG	0x0200
#define PHY_BMCR_DPLX		0x0100
#define PHY_BMCR_COL_TST	0x0080

#define PHY_BMCR_SPEED_MASK	0x2040
#define PHY_BMCR_1000_MBPS	0x0040
#define PHY_BMCR_100_MBPS	0x2000
#define PHY_BMCR_10_MBPS	0x0000

/* phy BMSR */
#define PHY_BMSR_100T4		0x8000
#define PHY_BMSR_100TXF		0x4000
#define PHY_BMSR_100TXH		0x2000
#define PHY_BMSR_10TF		0x1000
#define PHY_BMSR_10TH		0x0800
#define PHY_BMSR_EXT_STAT	0x0100
#define PHY_BMSR_PRE_SUP	0x0040
#define PHY_BMSR_AUTN_COMP	0x0020
#define PHY_BMSR_RF		0x0010
#define PHY_BMSR_AUTN_ABLE	0x0008
#define PHY_BMSR_LS		0x0004
#define PHY_BMSR_JD		0x0002
#define PHY_BMSR_EXT		0x0001

/*phy ANLPAR */
#define PHY_ANLPAR_NP		0x8000
#define PHY_ANLPAR_ACK		0x4000
#define PHY_ANLPAR_RF		0x2000
#define PHY_ANLPAR_ASYMP	0x0800
#define PHY_ANLPAR_PAUSE	0x0400
#define PHY_ANLPAR_T4		0x0200
#define PHY_ANLPAR_TXFD		0x0100
#define PHY_ANLPAR_TX		0x0080
#define PHY_ANLPAR_10FD		0x0040
#define PHY_ANLPAR_10		0x0020
#define PHY_ANLPAR_100		0x0380	/* we can run at 100 */
/* phy ANLPAR 1000BASE-X */
#define PHY_X_ANLPAR_NP		0x8000
#define PHY_X_ANLPAR_ACK	0x4000
#define PHY_X_ANLPAR_RF_MASK	0x3000
#define PHY_X_ANLPAR_PAUSE_MASK	0x0180
#define PHY_X_ANLPAR_HD		0x0040
#define PHY_X_ANLPAR_FD		0x0020

#define PHY_ANLPAR_PSB_MASK	0x001f
#define PHY_ANLPAR_PSB_802_3	0x0001
#define PHY_ANLPAR_PSB_802_9	0x0002

/* phy 1000BTCR */
#define PHY_1000BTCR_1000FD	0x0200
#define PHY_1000BTCR_1000HD	0x0100

/* phy 1000BTSR */
#define PHY_1000BTSR_MSCF	0x8000
#define PHY_1000BTSR_MSCR	0x4000
#define PHY_1000BTSR_LRS	0x2000
#define PHY_1000BTSR_RRS	0x1000
#define PHY_1000BTSR_1000FD	0x0800
#define PHY_1000BTSR_1000HD	0x0400

/* phy EXSR */
#define PHY_EXSR_1000XF		0x8000
#define PHY_EXSR_1000XH		0x4000
#define PHY_EXSR_1000TF		0x2000
#define PHY_EXSR_1000TH		0x1000

#endif
