/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on:
 *
 * ----------------------------------------------------------------------------
 *
 * dm644x_emac.h
 *
 * TI DaVinci (DM644X) EMAC peripheral driver header for DV-EVM
 *
 * Copyright (C) 2005 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------

 * Modifications:
 * ver. 1.0: Sep 2005, TI PSP Team - Created EMAC version for uBoot.
 *
 */

#ifndef _DM644X_EMAC_H_
#define _DM644X_EMAC_H_

#include <asm/arch/hardware.h>

#ifdef CONFIG_SOC_DM365
#define EMAC_BASE_ADDR			(0x01d07000)
#define EMAC_WRAPPER_BASE_ADDR		(0x01d0a000)
#define EMAC_WRAPPER_RAM_ADDR		(0x01d08000)
#define EMAC_MDIO_BASE_ADDR		(0x01d0b000)
#else
#define EMAC_BASE_ADDR			(0x01c80000)
#define EMAC_WRAPPER_BASE_ADDR		(0x01c81000)
#define EMAC_WRAPPER_RAM_ADDR		(0x01c82000)
#define EMAC_MDIO_BASE_ADDR		(0x01c84000)
#endif

#ifdef CONFIG_SOC_DM646x
/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		76500000
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		2500000		/* 2.5 MHz */
#elif defined(CONFIG_SOC_DM365)
/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		121500000
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		2200000		/* 2.2 MHz */
#else
/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		99000000	/* PLL/6 - 99 MHz */
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		2000000		/* 2.0 MHz */
#endif

/* PHY mask - set only those phy number bits where phy is/can be connected */
#define EMAC_MDIO_PHY_NUM           1
#define EMAC_MDIO_PHY_MASK          (1 << EMAC_MDIO_PHY_NUM)

/* Ethernet Min/Max packet size */
#define EMAC_MIN_ETHERNET_PKT_SIZE	60
#define EMAC_MAX_ETHERNET_PKT_SIZE	1518
#define EMAC_PKT_ALIGN			18	/* 1518 + 18 = 1536 (packet aligned on 32 byte boundry) */

/* Number of RX packet buffers
 * NOTE: Only 1 buffer supported as of now
 */
#define EMAC_MAX_RX_BUFFERS		10


/***********************************************
 ******** Internally used macros ***************
 ***********************************************/

#define EMAC_CH_TX			1
#define EMAC_CH_RX			0

/* Each descriptor occupies 4 words, lets start RX desc's at 0 and
 * reserve space for 64 descriptors max
 */
#define EMAC_RX_DESC_BASE		0x0
#define EMAC_TX_DESC_BASE		0x1000

/* EMAC Teardown value */
#define EMAC_TEARDOWN_VALUE		0xfffffffc

/* MII Status Register */
#define MII_STATUS_REG			1

/* Number of statistics registers */
#define EMAC_NUM_STATS			36


/* EMAC Descriptor */
typedef volatile struct _emac_desc
{
	u_int32_t	next;		/* Pointer to next descriptor in chain */
	u_int8_t	*buffer;	/* Pointer to data buffer */
	u_int32_t	buff_off_len;	/* Buffer Offset(MSW) and Length(LSW) */
	u_int32_t	pkt_flag_len;	/* Packet Flags(MSW) and Length(LSW) */
} emac_desc;

/* CPPI bit positions */
#define EMAC_CPPI_SOP_BIT		(0x80000000)
#define EMAC_CPPI_EOP_BIT		(0x40000000)
#define EMAC_CPPI_OWNERSHIP_BIT		(0x20000000)
#define EMAC_CPPI_EOQ_BIT		(0x10000000)
#define EMAC_CPPI_TEARDOWN_COMPLETE_BIT	(0x08000000)
#define EMAC_CPPI_PASS_CRC_BIT		(0x04000000)

#define EMAC_CPPI_RX_ERROR_FRAME	(0x03fc0000)

#define EMAC_MACCONTROL_MIIEN_ENABLE		(0x20)
#define EMAC_MACCONTROL_FULLDUPLEX_ENABLE	(0x1)
#define EMAC_MACCONTROL_GIGABIT_ENABLE		(1 << 7)
#define EMAC_MACCONTROL_GIGFORCE		(1 << 17)

#define EMAC_RXMBPENABLE_RXCAFEN_ENABLE	(0x200000)
#define EMAC_RXMBPENABLE_RXBROADEN	(0x2000)


#define MDIO_CONTROL_IDLE		(0x80000000)
#define MDIO_CONTROL_ENABLE		(0x40000000)
#define MDIO_CONTROL_FAULT_ENABLE	(0x40000)
#define MDIO_CONTROL_FAULT		(0x80000)
#define MDIO_USERACCESS0_GO		(0x80000000)
#define MDIO_USERACCESS0_WRITE_READ	(0x0)
#define MDIO_USERACCESS0_WRITE_WRITE	(0x40000000)
#define MDIO_USERACCESS0_ACK		(0x20000000)

/* Ethernet MAC Registers Structure */
typedef struct  {
	dv_reg		TXIDVER;
	dv_reg		TXCONTROL;
	dv_reg		TXTEARDOWN;
	u_int8_t	RSVD0[4];
	dv_reg		RXIDVER;
	dv_reg		RXCONTROL;
	dv_reg		RXTEARDOWN;
	u_int8_t	RSVD1[100];
	dv_reg		TXINTSTATRAW;
	dv_reg		TXINTSTATMASKED;
	dv_reg		TXINTMASKSET;
	dv_reg		TXINTMASKCLEAR;
	dv_reg		MACINVECTOR;
	u_int8_t	RSVD2[12];
	dv_reg		RXINTSTATRAW;
	dv_reg		RXINTSTATMASKED;
	dv_reg		RXINTMASKSET;
	dv_reg		RXINTMASKCLEAR;
	dv_reg		MACINTSTATRAW;
	dv_reg		MACINTSTATMASKED;
	dv_reg		MACINTMASKSET;
	dv_reg		MACINTMASKCLEAR;
	u_int8_t	RSVD3[64];
	dv_reg		RXMBPENABLE;
	dv_reg		RXUNICASTSET;
	dv_reg		RXUNICASTCLEAR;
	dv_reg		RXMAXLEN;
	dv_reg		RXBUFFEROFFSET;
	dv_reg		RXFILTERLOWTHRESH;
	u_int8_t	RSVD4[8];
	dv_reg		RX0FLOWTHRESH;
	dv_reg		RX1FLOWTHRESH;
	dv_reg		RX2FLOWTHRESH;
	dv_reg		RX3FLOWTHRESH;
	dv_reg		RX4FLOWTHRESH;
	dv_reg		RX5FLOWTHRESH;
	dv_reg		RX6FLOWTHRESH;
	dv_reg		RX7FLOWTHRESH;
	dv_reg		RX0FREEBUFFER;
	dv_reg		RX1FREEBUFFER;
	dv_reg		RX2FREEBUFFER;
	dv_reg		RX3FREEBUFFER;
	dv_reg		RX4FREEBUFFER;
	dv_reg		RX5FREEBUFFER;
	dv_reg		RX6FREEBUFFER;
	dv_reg		RX7FREEBUFFER;
	dv_reg		MACCONTROL;
	dv_reg		MACSTATUS;
	dv_reg		EMCONTROL;
	dv_reg		FIFOCONTROL;
	dv_reg		MACCONFIG;
	dv_reg		SOFTRESET;
	u_int8_t	RSVD5[88];
	dv_reg		MACSRCADDRLO;
	dv_reg		MACSRCADDRHI;
	dv_reg		MACHASH1;
	dv_reg		MACHASH2;
	dv_reg		BOFFTEST;
	dv_reg		TPACETEST;
	dv_reg		RXPAUSE;
	dv_reg		TXPAUSE;
	u_int8_t	RSVD6[16];
	dv_reg		RXGOODFRAMES;
	dv_reg		RXBCASTFRAMES;
	dv_reg		RXMCASTFRAMES;
	dv_reg		RXPAUSEFRAMES;
	dv_reg		RXCRCERRORS;
	dv_reg		RXALIGNCODEERRORS;
	dv_reg		RXOVERSIZED;
	dv_reg		RXJABBER;
	dv_reg		RXUNDERSIZED;
	dv_reg		RXFRAGMENTS;
	dv_reg		RXFILTERED;
	dv_reg		RXQOSFILTERED;
	dv_reg		RXOCTETS;
	dv_reg		TXGOODFRAMES;
	dv_reg		TXBCASTFRAMES;
	dv_reg		TXMCASTFRAMES;
	dv_reg		TXPAUSEFRAMES;
	dv_reg		TXDEFERRED;
	dv_reg		TXCOLLISION;
	dv_reg		TXSINGLECOLL;
	dv_reg		TXMULTICOLL;
	dv_reg		TXEXCESSIVECOLL;
	dv_reg		TXLATECOLL;
	dv_reg		TXUNDERRUN;
	dv_reg		TXCARRIERSENSE;
	dv_reg		TXOCTETS;
	dv_reg		FRAME64;
	dv_reg		FRAME65T127;
	dv_reg		FRAME128T255;
	dv_reg		FRAME256T511;
	dv_reg		FRAME512T1023;
	dv_reg		FRAME1024TUP;
	dv_reg		NETOCTETS;
	dv_reg		RXSOFOVERRUNS;
	dv_reg		RXMOFOVERRUNS;
	dv_reg		RXDMAOVERRUNS;
	u_int8_t	RSVD7[624];
	dv_reg		MACADDRLO;
	dv_reg		MACADDRHI;
	dv_reg		MACINDEX;
	u_int8_t	RSVD8[244];
	dv_reg		TX0HDP;
	dv_reg		TX1HDP;
	dv_reg		TX2HDP;
	dv_reg		TX3HDP;
	dv_reg		TX4HDP;
	dv_reg		TX5HDP;
	dv_reg		TX6HDP;
	dv_reg		TX7HDP;
	dv_reg		RX0HDP;
	dv_reg		RX1HDP;
	dv_reg		RX2HDP;
	dv_reg		RX3HDP;
	dv_reg		RX4HDP;
	dv_reg		RX5HDP;
	dv_reg		RX6HDP;
	dv_reg		RX7HDP;
	dv_reg		TX0CP;
	dv_reg		TX1CP;
	dv_reg		TX2CP;
	dv_reg		TX3CP;
	dv_reg		TX4CP;
	dv_reg		TX5CP;
	dv_reg		TX6CP;
	dv_reg		TX7CP;
	dv_reg		RX0CP;
	dv_reg		RX1CP;
	dv_reg		RX2CP;
	dv_reg		RX3CP;
	dv_reg		RX4CP;
	dv_reg		RX5CP;
	dv_reg		RX6CP;
	dv_reg		RX7CP;
} emac_regs;

/* EMAC Wrapper Registers Structure */
typedef struct  {
#if defined(CONFIG_SOC_DM646x) || defined(CONFIG_SOC_DM365)
	dv_reg		IDVER;
	dv_reg		SOFTRST;
	dv_reg		EMCTRL;
#else
	u_int8_t	RSVD0[4100];
	dv_reg		EWCTL;
	dv_reg		EWINTTCNT;
#endif
} ewrap_regs;

/* EMAC MDIO Registers Structure */
typedef struct  {
	dv_reg		VERSION;
	dv_reg		CONTROL;
	dv_reg		ALIVE;
	dv_reg		LINK;
	dv_reg		LINKINTRAW;
	dv_reg		LINKINTMASKED;
	u_int8_t	RSVD0[8];
	dv_reg		USERINTRAW;
	dv_reg		USERINTMASKED;
	dv_reg		USERINTMASKSET;
	dv_reg		USERINTMASKCLEAR;
	u_int8_t	RSVD1[80];
	dv_reg		USERACCESS0;
	dv_reg		USERPHYSEL0;
	dv_reg		USERACCESS1;
	dv_reg		USERPHYSEL1;
} mdio_regs;

int davinci_eth_phy_read(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t *data);
int davinci_eth_phy_write(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t data);

typedef struct
{
	char	name[64];
	int	(*init)(int phy_addr);
	int	(*is_phy_connected)(int phy_addr);
	int	(*get_link_speed)(int phy_addr);
	int	(*auto_negotiate)(int phy_addr);
} phy_t;

#define PHY_LXT972	(0x001378e2)
int lxt972_is_phy_connected(int phy_addr);
int lxt972_get_link_speed(int phy_addr);
int lxt972_init_phy(int phy_addr);
int lxt972_auto_negotiate(int phy_addr);

#define PHY_DP83848	(0x20005c90)
int dp83848_is_phy_connected(int phy_addr);
int dp83848_get_link_speed(int phy_addr);
int dp83848_init_phy(int phy_addr);
int dp83848_auto_negotiate(int phy_addr);

#endif  /* _DM644X_EMAC_H_ */
