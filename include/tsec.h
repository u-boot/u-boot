/*
 *  tsec.h
 *
 *  Driver for the Motorola Triple Speed Ethernet Controller
 *
 *  This software may be used and distributed according to the
 *  terms of the GNU Public License, Version 2, incorporated
 *  herein by reference.
 *
 * Copyright 2004, 2007 Freescale Semiconductor, Inc.
 * (C) Copyright 2003, Motorola, Inc.
 * maintained by Xianghua Xiao (x.xiao@motorola.com)
 * author Andy Fleming
 *
 */

#ifndef __TSEC_H
#define __TSEC_H

#include <net.h>
#include <config.h>

#ifndef CONFIG_SYS_TSEC1_OFFSET
    #define CONFIG_SYS_TSEC1_OFFSET	(0x24000)
#endif

#define TSEC_SIZE	0x01000

/* FIXME:  Should these be pushed back to 83xx and 85xx config files? */
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx) \
	|| defined(CONFIG_MPC83xx)
    #define TSEC_BASE_ADDR	(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC1_OFFSET)
#endif

#define STD_TSEC_INFO(num) \
{			\
	.regs = (tsec_t *)(TSEC_BASE_ADDR + ((num - 1) * TSEC_SIZE)), \
	.miiregs = (tsec_t *)TSEC_BASE_ADDR, \
	.devname = CONFIG_TSEC##num##_NAME, \
	.phyaddr = TSEC##num##_PHY_ADDR, \
	.flags = TSEC##num##_FLAGS \
}

#define SET_STD_TSEC_INFO(x, num) \
{			\
	x.regs = (tsec_t *)(TSEC_BASE_ADDR + ((num - 1) * TSEC_SIZE)); \
	x.miiregs = (tsec_t *)TSEC_BASE_ADDR; \
	x.devname = CONFIG_TSEC##num##_NAME; \
	x.phyaddr = TSEC##num##_PHY_ADDR; \
	x.flags = TSEC##num##_FLAGS;\
}

#define MAC_ADDR_LEN 6

/* #define TSEC_TIMEOUT	1000000 */
#define TSEC_TIMEOUT 1000
#define TOUT_LOOP	1000000

#define PHY_AUTONEGOTIATE_TIMEOUT	5000 /* in ms */

/* TBI register addresses */
#define TBI_CR			0x00
#define TBI_SR			0x01
#define TBI_ANA			0x04
#define TBI_ANLPBPA		0x05
#define TBI_ANEX		0x06
#define TBI_TBICON		0x11

/* TBI MDIO register bit fields*/
#define TBICON_CLK_SELECT	0x0020
#define TBIANA_ASYMMETRIC_PAUSE 0x0100
#define TBIANA_SYMMETRIC_PAUSE  0x0080
#define TBIANA_HALF_DUPLEX	0x0040
#define TBIANA_FULL_DUPLEX	0x0020
#define TBICR_PHY_RESET		0x8000
#define TBICR_ANEG_ENABLE	0x1000
#define TBICR_RESTART_ANEG	0x0200
#define TBICR_FULL_DUPLEX	0x0100
#define TBICR_SPEED1_SET	0x0040


/* MAC register bits */
#define MACCFG1_SOFT_RESET	0x80000000
#define MACCFG1_RESET_RX_MC	0x00080000
#define MACCFG1_RESET_TX_MC	0x00040000
#define MACCFG1_RESET_RX_FUN	0x00020000
#define	MACCFG1_RESET_TX_FUN	0x00010000
#define MACCFG1_LOOPBACK	0x00000100
#define MACCFG1_RX_FLOW		0x00000020
#define MACCFG1_TX_FLOW		0x00000010
#define MACCFG1_SYNCD_RX_EN	0x00000008
#define MACCFG1_RX_EN		0x00000004
#define MACCFG1_SYNCD_TX_EN	0x00000002
#define MACCFG1_TX_EN		0x00000001

#define MACCFG2_INIT_SETTINGS	0x00007205
#define MACCFG2_FULL_DUPLEX	0x00000001
#define MACCFG2_IF		0x00000300
#define MACCFG2_GMII		0x00000200
#define MACCFG2_MII		0x00000100

#define ECNTRL_INIT_SETTINGS	0x00001000
#define ECNTRL_TBI_MODE		0x00000020
#define ECNTRL_R100		0x00000008
#define ECNTRL_SGMII_MODE	0x00000002

#define miim_end -2
#define miim_read -1

#ifndef CONFIG_SYS_TBIPA_VALUE
    #define CONFIG_SYS_TBIPA_VALUE	0x1f
#endif
#define MIIMCFG_INIT_VALUE	0x00000003
#define MIIMCFG_RESET		0x80000000

#define MIIMIND_BUSY		0x00000001
#define MIIMIND_NOTVALID	0x00000004

#define MIIM_CONTROL		0x00
#define MIIM_CONTROL_RESET	0x00009140
#define MIIM_CONTROL_INIT	0x00001140
#define MIIM_CONTROL_RESTART	0x00001340
#define MIIM_ANEN		0x00001000

#define MIIM_CR			0x00
#define MIIM_CR_RST		0x00008000
#define MIIM_CR_INIT		0x00001000

#define MIIM_STATUS		0x1
#define MIIM_STATUS_AN_DONE	0x00000020
#define MIIM_STATUS_LINK	0x0004
#define PHY_BMSR_AUTN_ABLE	0x0008
#define PHY_BMSR_AUTN_COMP	0x0020

#define MIIM_PHYIR1		0x2
#define MIIM_PHYIR2		0x3

#define MIIM_ANAR		0x4
#define MIIM_ANAR_INIT		0x1e1

#define MIIM_TBI_ANLPBPA	0x5
#define MIIM_TBI_ANLPBPA_HALF	0x00000040
#define MIIM_TBI_ANLPBPA_FULL	0x00000020

#define MIIM_TBI_ANEX		0x6
#define MIIM_TBI_ANEX_NP	0x00000004
#define MIIM_TBI_ANEX_PRX	0x00000002

#define MIIM_GBIT_CONTROL	0x9
#define MIIM_GBIT_CONTROL_INIT	0xe00

#define MIIM_EXT_PAGE_ACCESS	0x1f

/* Broadcom BCM54xx -- taken from linux sungem_phy */
#define MIIM_BCM54xx_AUXCNTL			0x18
#define MIIM_BCM54xx_AUXCNTL_ENCODE(val)	((val & 0x7) << 12)|(val & 0x7)
#define MIIM_BCM54xx_AUXSTATUS			0x19
#define MIIM_BCM54xx_AUXSTATUS_LINKMODE_MASK	0x0700
#define MIIM_BCM54xx_AUXSTATUS_LINKMODE_SHIFT	8

/* Cicada Auxiliary Control/Status Register */
#define MIIM_CIS8201_AUX_CONSTAT	0x1c
#define MIIM_CIS8201_AUXCONSTAT_INIT	0x0004
#define MIIM_CIS8201_AUXCONSTAT_DUPLEX	0x0020
#define MIIM_CIS8201_AUXCONSTAT_SPEED	0x0018
#define MIIM_CIS8201_AUXCONSTAT_GBIT	0x0010
#define MIIM_CIS8201_AUXCONSTAT_100	0x0008

/* Cicada Extended Control Register 1 */
#define MIIM_CIS8201_EXT_CON1		0x17
#define MIIM_CIS8201_EXTCON1_INIT	0x0000

/* Cicada 8204 Extended PHY Control Register 1 */
#define MIIM_CIS8204_EPHY_CON		0x17
#define MIIM_CIS8204_EPHYCON_INIT	0x0006
#define MIIM_CIS8204_EPHYCON_RGMII	0x1100

/* Cicada 8204 Serial LED Control Register */
#define MIIM_CIS8204_SLED_CON		0x1b
#define MIIM_CIS8204_SLEDCON_INIT	0x1115

#define MIIM_GBIT_CON		0x09
#define MIIM_GBIT_CON_ADVERT	0x0e00

/* Entry for Vitesse VSC8244 regs starts here */
/* Vitesse VSC8244 Auxiliary Control/Status Register */
#define MIIM_VSC8244_AUX_CONSTAT	0x1c
#define MIIM_VSC8244_AUXCONSTAT_INIT	0x0000
#define MIIM_VSC8244_AUXCONSTAT_DUPLEX	0x0020
#define MIIM_VSC8244_AUXCONSTAT_SPEED	0x0018
#define MIIM_VSC8244_AUXCONSTAT_GBIT	0x0010
#define MIIM_VSC8244_AUXCONSTAT_100	0x0008
#define MIIM_CONTROL_INIT_LOOPBACK	0x4000

/* Vitesse VSC8244 Extended PHY Control Register 1 */
#define MIIM_VSC8244_EPHY_CON		0x17
#define MIIM_VSC8244_EPHYCON_INIT	0x0006

/* Vitesse VSC8244 Serial LED Control Register */
#define MIIM_VSC8244_LED_CON		0x1b
#define MIIM_VSC8244_LEDCON_INIT	0xF011

/* Entry for Vitesse VSC8601 regs starts here (Not complete) */
/* Vitesse VSC8601 Extended PHY Control Register 1 */
#define MIIM_VSC8601_EPHY_CON		0x17
#define MIIM_VSC8601_EPHY_CON_INIT_SKEW	0x1120
#define MIIM_VSC8601_SKEW_CTRL		0x1c

/* 88E1011 PHY Status Register */
#define MIIM_88E1011_PHY_STATUS		0x11
#define MIIM_88E1011_PHYSTAT_SPEED	0xc000
#define MIIM_88E1011_PHYSTAT_GBIT	0x8000
#define MIIM_88E1011_PHYSTAT_100	0x4000
#define MIIM_88E1011_PHYSTAT_DUPLEX	0x2000
#define MIIM_88E1011_PHYSTAT_SPDDONE	0x0800
#define MIIM_88E1011_PHYSTAT_LINK	0x0400

#define MIIM_88E1011_PHY_SCR		0x10
#define MIIM_88E1011_PHY_MDI_X_AUTO	0x0060

/* 88E1111 PHY LED Control Register */
#define MIIM_88E1111_PHY_LED_CONTROL	24
#define MIIM_88E1111_PHY_LED_DIRECT	0x4100
#define MIIM_88E1111_PHY_LED_COMBINE	0x411C

/* 88E1121 PHY LED Control Register */
#define MIIM_88E1121_PHY_LED_CTRL	16
#define MIIM_88E1121_PHY_LED_PAGE	3
#define MIIM_88E1121_PHY_LED_DEF	0x0030

/* 88E1121 PHY IRQ Enable/Status Register */
#define MIIM_88E1121_PHY_IRQ_EN		18
#define MIIM_88E1121_PHY_IRQ_STATUS	19

#define MIIM_88E1121_PHY_PAGE		22

/* 88E1145 Extended PHY Specific Control Register */
#define MIIM_88E1145_PHY_EXT_CR 20
#define MIIM_M88E1145_RGMII_RX_DELAY	0x0080
#define MIIM_M88E1145_RGMII_TX_DELAY	0x0002

#define MIIM_88E1145_PHY_PAGE	29
#define MIIM_88E1145_PHY_CAL_OV 30

/* RTL8211B PHY Status Register */
#define MIIM_RTL8211B_PHY_STATUS	0x11
#define MIIM_RTL8211B_PHYSTAT_SPEED	0xc000
#define MIIM_RTL8211B_PHYSTAT_GBIT	0x8000
#define MIIM_RTL8211B_PHYSTAT_100	0x4000
#define MIIM_RTL8211B_PHYSTAT_DUPLEX	0x2000
#define MIIM_RTL8211B_PHYSTAT_SPDDONE	0x0800
#define MIIM_RTL8211B_PHYSTAT_LINK	0x0400

/* DM9161 Control register values */
#define MIIM_DM9161_CR_STOP	0x0400
#define MIIM_DM9161_CR_RSTAN	0x1200

#define MIIM_DM9161_SCR		0x10
#define MIIM_DM9161_SCR_INIT	0x0610

/* DM9161 Specified Configuration and Status Register */
#define MIIM_DM9161_SCSR	0x11
#define MIIM_DM9161_SCSR_100F	0x8000
#define MIIM_DM9161_SCSR_100H	0x4000
#define MIIM_DM9161_SCSR_10F	0x2000
#define MIIM_DM9161_SCSR_10H	0x1000

/* DM9161 10BT Configuration/Status */
#define MIIM_DM9161_10BTCSR	0x12
#define MIIM_DM9161_10BTCSR_INIT	0x7800

/* LXT971 Status 2 registers */
#define MIIM_LXT971_SR2		     0x11  /* Status Register 2  */
#define MIIM_LXT971_SR2_SPEED_MASK 0x4200
#define MIIM_LXT971_SR2_10HDX	   0x0000  /*  10 Mbit half duplex selected */
#define MIIM_LXT971_SR2_10FDX	   0x0200  /*  10 Mbit full duplex selected */
#define MIIM_LXT971_SR2_100HDX	   0x4000  /* 100 Mbit half duplex selected */
#define MIIM_LXT971_SR2_100FDX	   0x4200  /* 100 Mbit full duplex selected */

/* DP83865 Control register values */
#define MIIM_DP83865_CR_INIT	0x9200

/* DP83865 Link and Auto-Neg Status Register */
#define MIIM_DP83865_LANR	0x11
#define MIIM_DP83865_SPD_MASK	0x0018
#define MIIM_DP83865_SPD_1000	0x0010
#define MIIM_DP83865_SPD_100	0x0008
#define MIIM_DP83865_DPX_FULL	0x0002

#define MIIM_READ_COMMAND	0x00000001

#define MRBLR_INIT_SETTINGS	PKTSIZE_ALIGN

#define MINFLR_INIT_SETTINGS	0x00000040

#define DMACTRL_INIT_SETTINGS	0x000000c3
#define DMACTRL_GRS		0x00000010
#define DMACTRL_GTS		0x00000008

#define TSTAT_CLEAR_THALT	0x80000000
#define RSTAT_CLEAR_RHALT	0x00800000


#define IEVENT_INIT_CLEAR	0xffffffff
#define IEVENT_BABR		0x80000000
#define IEVENT_RXC		0x40000000
#define IEVENT_BSY		0x20000000
#define IEVENT_EBERR		0x10000000
#define IEVENT_MSRO		0x04000000
#define IEVENT_GTSC		0x02000000
#define IEVENT_BABT		0x01000000
#define IEVENT_TXC		0x00800000
#define IEVENT_TXE		0x00400000
#define IEVENT_TXB		0x00200000
#define IEVENT_TXF		0x00100000
#define IEVENT_IE		0x00080000
#define IEVENT_LC		0x00040000
#define IEVENT_CRL		0x00020000
#define IEVENT_XFUN		0x00010000
#define IEVENT_RXB0		0x00008000
#define IEVENT_GRSC		0x00000100
#define IEVENT_RXF0		0x00000080

#define IMASK_INIT_CLEAR	0x00000000
#define IMASK_TXEEN		0x00400000
#define IMASK_TXBEN		0x00200000
#define IMASK_TXFEN		0x00100000
#define IMASK_RXFEN0		0x00000080


/* Default Attribute fields */
#define ATTR_INIT_SETTINGS     0x000000c0
#define ATTRELI_INIT_SETTINGS  0x00000000


/* TxBD status field bits */
#define TXBD_READY		0x8000
#define TXBD_PADCRC		0x4000
#define TXBD_WRAP		0x2000
#define TXBD_INTERRUPT		0x1000
#define TXBD_LAST		0x0800
#define TXBD_CRC		0x0400
#define TXBD_DEF		0x0200
#define TXBD_HUGEFRAME		0x0080
#define TXBD_LATECOLLISION	0x0080
#define TXBD_RETRYLIMIT		0x0040
#define	TXBD_RETRYCOUNTMASK	0x003c
#define TXBD_UNDERRUN		0x0002
#define TXBD_STATS		0x03ff

/* RxBD status field bits */
#define RXBD_EMPTY		0x8000
#define RXBD_RO1		0x4000
#define RXBD_WRAP		0x2000
#define RXBD_INTERRUPT		0x1000
#define RXBD_LAST		0x0800
#define RXBD_FIRST		0x0400
#define RXBD_MISS		0x0100
#define RXBD_BROADCAST		0x0080
#define RXBD_MULTICAST		0x0040
#define RXBD_LARGE		0x0020
#define RXBD_NONOCTET		0x0010
#define RXBD_SHORT		0x0008
#define RXBD_CRCERR		0x0004
#define RXBD_OVERRUN		0x0002
#define RXBD_TRUNCATED		0x0001
#define RXBD_STATS		0x003f

typedef struct txbd8
{
	ushort	     status;	     /* Status Fields */
	ushort	     length;	     /* Buffer length */
	uint	     bufPtr;	     /* Buffer Pointer */
} txbd8_t;

typedef struct rxbd8
{
	ushort	     status;	     /* Status Fields */
	ushort	     length;	     /* Buffer Length */
	uint	     bufPtr;	     /* Buffer Pointer */
} rxbd8_t;

typedef struct rmon_mib
{
	/* Transmit and Receive Counters */
	uint	tr64;		/* Transmit and Receive 64-byte Frame Counter */
	uint	tr127;		/* Transmit and Receive 65-127 byte Frame Counter */
	uint	tr255;		/* Transmit and Receive 128-255 byte Frame Counter */
	uint	tr511;		/* Transmit and Receive 256-511 byte Frame Counter */
	uint	tr1k;		/* Transmit and Receive 512-1023 byte Frame Counter */
	uint	trmax;		/* Transmit and Receive 1024-1518 byte Frame Counter */
	uint	trmgv;		/* Transmit and Receive 1519-1522 byte Good VLAN Frame */
	/* Receive Counters */
	uint	rbyt;		/* Receive Byte Counter */
	uint	rpkt;		/* Receive Packet Counter */
	uint	rfcs;		/* Receive FCS Error Counter */
	uint	rmca;		/* Receive Multicast Packet (Counter) */
	uint	rbca;		/* Receive Broadcast Packet */
	uint	rxcf;		/* Receive Control Frame Packet */
	uint	rxpf;		/* Receive Pause Frame Packet */
	uint	rxuo;		/* Receive Unknown OP Code */
	uint	raln;		/* Receive Alignment Error */
	uint	rflr;		/* Receive Frame Length Error */
	uint	rcde;		/* Receive Code Error */
	uint	rcse;		/* Receive Carrier Sense Error */
	uint	rund;		/* Receive Undersize Packet */
	uint	rovr;		/* Receive Oversize Packet */
	uint	rfrg;		/* Receive Fragments */
	uint	rjbr;		/* Receive Jabber */
	uint	rdrp;		/* Receive Drop */
	/* Transmit Counters */
	uint	tbyt;		/* Transmit Byte Counter */
	uint	tpkt;		/* Transmit Packet */
	uint	tmca;		/* Transmit Multicast Packet */
	uint	tbca;		/* Transmit Broadcast Packet */
	uint	txpf;		/* Transmit Pause Control Frame */
	uint	tdfr;		/* Transmit Deferral Packet */
	uint	tedf;		/* Transmit Excessive Deferral Packet */
	uint	tscl;		/* Transmit Single Collision Packet */
	/* (0x2_n700) */
	uint	tmcl;		/* Transmit Multiple Collision Packet */
	uint	tlcl;		/* Transmit Late Collision Packet */
	uint	txcl;		/* Transmit Excessive Collision Packet */
	uint	tncl;		/* Transmit Total Collision */

	uint	res2;

	uint	tdrp;		/* Transmit Drop Frame */
	uint	tjbr;		/* Transmit Jabber Frame */
	uint	tfcs;		/* Transmit FCS Error */
	uint	txcf;		/* Transmit Control Frame */
	uint	tovr;		/* Transmit Oversize Frame */
	uint	tund;		/* Transmit Undersize Frame */
	uint	tfrg;		/* Transmit Fragments Frame */
	/* General Registers */
	uint	car1;		/* Carry Register One */
	uint	car2;		/* Carry Register Two */
	uint	cam1;		/* Carry Register One Mask */
	uint	cam2;		/* Carry Register Two Mask */
} rmon_mib_t;

typedef struct tsec_hash_regs
{
	uint	iaddr0;		/* Individual Address Register 0 */
	uint	iaddr1;		/* Individual Address Register 1 */
	uint	iaddr2;		/* Individual Address Register 2 */
	uint	iaddr3;		/* Individual Address Register 3 */
	uint	iaddr4;		/* Individual Address Register 4 */
	uint	iaddr5;		/* Individual Address Register 5 */
	uint	iaddr6;		/* Individual Address Register 6 */
	uint	iaddr7;		/* Individual Address Register 7 */
	uint	res1[24];
	uint	gaddr0;		/* Group Address Register 0 */
	uint	gaddr1;		/* Group Address Register 1 */
	uint	gaddr2;		/* Group Address Register 2 */
	uint	gaddr3;		/* Group Address Register 3 */
	uint	gaddr4;		/* Group Address Register 4 */
	uint	gaddr5;		/* Group Address Register 5 */
	uint	gaddr6;		/* Group Address Register 6 */
	uint	gaddr7;		/* Group Address Register 7 */
	uint	res2[24];
} tsec_hash_t;

typedef struct tsec
{
	/* General Control and Status Registers (0x2_n000) */
	uint	res000[4];

	uint	ievent;		/* Interrupt Event */
	uint	imask;		/* Interrupt Mask */
	uint	edis;		/* Error Disabled */
	uint	res01c;
	uint	ecntrl;		/* Ethernet Control */
	uint	minflr;		/* Minimum Frame Length */
	uint	ptv;		/* Pause Time Value */
	uint	dmactrl;	/* DMA Control */
	uint	tbipa;		/* TBI PHY Address */

	uint	res034[3];
	uint	res040[48];

	/* Transmit Control and Status Registers (0x2_n100) */
	uint	tctrl;		/* Transmit Control */
	uint	tstat;		/* Transmit Status */
	uint	res108;
	uint	tbdlen;		/* Tx BD Data Length */
	uint	res110[5];
	uint	ctbptr;		/* Current TxBD Pointer */
	uint	res128[23];
	uint	tbptr;		/* TxBD Pointer */
	uint	res188[30];
	/* (0x2_n200) */
	uint	res200;
	uint	tbase;		/* TxBD Base Address */
	uint	res208[42];
	uint	ostbd;		/* Out of Sequence TxBD */
	uint	ostbdp;		/* Out of Sequence Tx Data Buffer Pointer */
	uint	res2b8[18];

	/* Receive Control and Status Registers (0x2_n300) */
	uint	rctrl;		/* Receive Control */
	uint	rstat;		/* Receive Status */
	uint	res308;
	uint	rbdlen;		/* RxBD Data Length */
	uint	res310[4];
	uint	res320;
	uint	crbptr;	/* Current Receive Buffer Pointer */
	uint	res328[6];
	uint	mrblr;	/* Maximum Receive Buffer Length */
	uint	res344[16];
	uint	rbptr;	/* RxBD Pointer */
	uint	res388[30];
	/* (0x2_n400) */
	uint	res400;
	uint	rbase;	/* RxBD Base Address */
	uint	res408[62];

	/* MAC Registers (0x2_n500) */
	uint	maccfg1;	/* MAC Configuration #1 */
	uint	maccfg2;	/* MAC Configuration #2 */
	uint	ipgifg;		/* Inter Packet Gap/Inter Frame Gap */
	uint	hafdup;		/* Half-duplex */
	uint	maxfrm;		/* Maximum Frame */
	uint	res514;
	uint	res518;

	uint	res51c;

	uint	miimcfg;	/* MII Management: Configuration */
	uint	miimcom;	/* MII Management: Command */
	uint	miimadd;	/* MII Management: Address */
	uint	miimcon;	/* MII Management: Control */
	uint	miimstat;	/* MII Management: Status */
	uint	miimind;	/* MII Management: Indicators */

	uint	res538;

	uint	ifstat;		/* Interface Status */
	uint	macstnaddr1;	/* Station Address, part 1 */
	uint	macstnaddr2;	/* Station Address, part 2 */
	uint	res548[46];

	/* (0x2_n600) */
	uint	res600[32];

	/* RMON MIB Registers (0x2_n680-0x2_n73c) */
	rmon_mib_t	rmon;
	uint	res740[48];

	/* Hash Function Registers (0x2_n800) */
	tsec_hash_t	hash;

	uint	res900[128];

	/* Pattern Registers (0x2_nb00) */
	uint	resb00[62];
	uint	attr;	   /* Default Attribute Register */
	uint	attreli;	   /* Default Attribute Extract Length and Index */

	/* TSEC Future Expansion Space (0x2_nc00-0x2_nffc) */
	uint	resc00[256];
} tsec_t;

#define TSEC_GIGABIT (1)

/* This flag currently only has
 * meaning if we're using the eTSEC */
#define TSEC_REDUCED	(1 << 1)

#define TSEC_SGMII	(1 << 2)

struct tsec_private {
	volatile tsec_t *regs;
	volatile tsec_t *phyregs;
	struct phy_info *phyinfo;
	uint phyaddr;
	u32 flags;
	uint link;
	uint duplexity;
	uint speed;
};


/*
 * struct phy_cmd:  A command for reading or writing a PHY register
 *
 * mii_reg:  The register to read or write
 *
 * mii_data:  For writes, the value to put in the register.
 *	A value of -1 indicates this is a read.
 *
 * funct: A function pointer which is invoked for each command.
 *	For reads, this function will be passed the value read
 *	from the PHY, and process it.
 *	For writes, the result of this function will be written
 *	to the PHY register
 */
struct phy_cmd {
	uint mii_reg;
	uint mii_data;
	uint (*funct) (uint mii_reg, struct tsec_private * priv);
};

/* struct phy_info: a structure which defines attributes for a PHY
 *
 * id will contain a number which represents the PHY.  During
 * startup, the driver will poll the PHY to find out what its
 * UID--as defined by registers 2 and 3--is.  The 32-bit result
 * gotten from the PHY will be shifted right by "shift" bits to
 * discard any bits which may change based on revision numbers
 * unimportant to functionality
 *
 * The struct phy_cmd entries represent pointers to an arrays of
 * commands which tell the driver what to do to the PHY.
 */
struct phy_info {
	uint id;
	char *name;
	uint shift;
	/* Called to configure the PHY, and modify the controller
	 * based on the results */
	struct phy_cmd *config;

	/* Called when starting up the controller */
	struct phy_cmd *startup;

	/* Called when bringing down the controller */
	struct phy_cmd *shutdown;
};

struct tsec_info_struct {
	tsec_t *regs;
	tsec_t *miiregs;
	char *devname;
	unsigned int phyaddr;
	u32 flags;
};

int tsec_initialize(bd_t * bis, struct tsec_info_struct *tsec_info);
int tsec_standard_init(bd_t *bis);
int tsec_eth_init(bd_t *bis, struct tsec_info_struct *tsec_info, int num);

#endif /* __TSEC_H */
