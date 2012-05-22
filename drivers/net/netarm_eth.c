/*
 * Copyright (C) 2004 IMMS gGmbH <www.imms.de>
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
 * author(s): Thomas Elste, <info@elste.org>
 *            (some parts derived from uCLinux Netarm Ethernet Driver)
 */


#include <common.h>
#include <command.h>
#include <net.h>
#include "netarm_eth.h"
#include <asm/arch/netarm_registers.h>

static int na_mii_poll_busy (void);

static void na_get_mac_addr (void)
{
	unsigned short p[3];
	char *m_addr;
	char ethaddr[20];

	m_addr = (char *) p;

	p[0] = (unsigned short) GET_EADDR (NETARM_ETH_SAL_STATION_ADDR_1);
	p[1] = (unsigned short) GET_EADDR (NETARM_ETH_SAL_STATION_ADDR_2);
	p[2] = (unsigned short) GET_EADDR (NETARM_ETH_SAL_STATION_ADDR_3);

	sprintf (ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
		 m_addr[0], m_addr[1],
		 m_addr[2], m_addr[3], m_addr[4], m_addr[5]);

	printf ("HW-MAC Address:  %s\n", ethaddr);

	/* set env, todo: check if already an adress is set */
	setenv ("ethaddr", ethaddr);
}

static void na_mii_write (int reg, int value)
{
	int mii_addr;

	/* Select register */
	mii_addr = CONFIG_SYS_ETH_PHY_ADDR + reg;
	SET_EADDR (NETARM_ETH_MII_ADDR, mii_addr);
	/* Write value */
	SET_EADDR (NETARM_ETH_MII_WRITE, value);
	na_mii_poll_busy ();
}

static unsigned int na_mii_read (int reg)
{
	int mii_addr, val;

	/* Select register */
	mii_addr = CONFIG_SYS_ETH_PHY_ADDR + reg;
	SET_EADDR (NETARM_ETH_MII_ADDR, mii_addr);
	/* do one management cycle */
	SET_EADDR (NETARM_ETH_MII_CMD,
		   GET_EADDR (NETARM_ETH_MII_CMD) | NETARM_ETH_MIIC_RSTAT);
	na_mii_poll_busy ();
	/* Return read value */
	val = GET_EADDR (NETARM_ETH_MII_READ);
	return val;
}

static int na_mii_poll_busy (void)
{
	ulong start;
	/* arm simple, non interrupt dependent timer */
	start = get_timer(0));
	while (get_timer(start) < NA_MII_POLL_BUSY_DELAY) {
		if (!(GET_EADDR (NETARM_ETH_MII_IND) & NETARM_ETH_MIII_BUSY)) {
			return 1;
		}
	}
	printf ("na_mii_busy timeout\n");
	return (0);
}

static int na_mii_identify_phy (void)
{
	int id_reg_a = 0;

	/* get phy id register */
	id_reg_a = na_mii_read (MII_PHY_ID);

	if (id_reg_a == 0x0043) {
		/* This must be an Enable or a Lucent LU3X31 PHY chip */
		return 1;
	} else if (id_reg_a == 0x0013) {
		/* it is an Intel LXT971A */
		return 1;
	}
	return (0);
}

static int na_mii_negotiate (void)
{
	int i = 0;

	/* Enable auto-negotiation */
	na_mii_write (MII_PHY_AUTONEGADV, 0x01e1);
	/* FIXME: 0x01E1 is 100Mb half and full duplex, 0x0061 is 10Mb only */
	/* Restart auto-negotiation */
	na_mii_write (MII_PHY_CONTROL, 0x1200);

	/* status register is 0xffff after setting the autoneg restart bit */
	while (na_mii_read (MII_PHY_STATUS) == 0xffff) {
		i++;
	}

	/* na_mii_read uses the timer already, so we can't use it again for
	   timeout checking.
	   Instead we just try some times.
	 */
	for (i = 0; i < 40000; i++) {
		if ((na_mii_read (MII_PHY_STATUS) & 0x0024) == 0x0024) {
			return 0;
		}
	}
	/*
	   printf("*Warning* autonegotiation timeout, status: 0x%x\n",na_mii_read(MII_PHY_STATUS));
	 */
	return (1);
}

static unsigned int na_mii_check_speed (void)
{
	unsigned int status;

	/* Read Status register */
	status = na_mii_read (MII_PHY_STATUS);
	/* Check link status.  If 0, default to 100 Mbps. */
	if ((status & 0x0004) == 0) {
		printf ("*Warning* no link detected, set default speed to 100Mbs\n");
		return 1;
	} else {
		if ((na_mii_read (17) & 0x4000) != 0) {
			printf ("100Mbs link detected\n");
			return 1;
		} else {
			printf ("10Mbs link detected\n");
			return 0;
		}
	}
	return 0;
}

static int reset_eth (void)
{
	int pt;
	ulong start;

	na_get_mac_addr ();
	pt = na_mii_identify_phy ();

	/* reset the phy */
	na_mii_write (MII_PHY_CONTROL, 0x8000);
	start = get_timer(0);
	while (get_timer(start) < NA_MII_NEGOTIATE_DELAY) {
		if ((na_mii_read (MII_PHY_STATUS) & 0x8000) == 0) {
			break;
		}
	}
	if (get_timer(start) >= NA_MII_NEGOTIATE_DELAY)
		printf ("phy reset timeout\n");

	/* set the PCS reg */
	SET_EADDR (NETARM_ETH_PCS_CFG, NETARM_ETH_PCSC_CLKS_25M |
		   NETARM_ETH_PCSC_ENJAB | NETARM_ETH_PCSC_NOCFR);

	na_mii_negotiate ();
	na_mii_check_speed ();

	/* Delay 10 millisecond.  (Maybe this should be 1 second.) */
	udelay (10000);

	/* Turn receive on.
	   Enable statistics register autozero on read.
	   Do not insert MAC address on transmit.
	   Do not enable special test modes.  */
	SET_EADDR (NETARM_ETH_STL_CFG,
		   (NETARM_ETH_STLC_AUTOZ | NETARM_ETH_STLC_RXEN));

	/* Set the inter-packet gap delay to 0.96us for MII.
	   The NET+ARM H/W Reference Guide indicates that the Back-to-back IPG
	   Gap Timer Register should be set to 0x15 and the Non Back-to-back IPG
	   Gap Timer Register should be set to 0x00000C12 for the MII PHY. */
	SET_EADDR (NETARM_ETH_B2B_IPG_GAP_TMR, 0x15);
	SET_EADDR (NETARM_ETH_NB2B_IPG_GAP_TMR, 0x00000C12);

	/* Add CRC to end of packets.
	   Pad packets to minimum length of 64 bytes.
	   Allow unlimited length transmit packets.
	   Receive all broadcast packets.
	   NOTE:  Multicast addressing is NOT enabled here currently. */
	SET_EADDR (NETARM_ETH_MAC_CFG,
		   (NETARM_ETH_MACC_CRCEN |
		    NETARM_ETH_MACC_PADEN | NETARM_ETH_MACC_HUGEN));
	SET_EADDR (NETARM_ETH_SAL_FILTER, NETARM_ETH_SALF_BROAD);

	/* enable fifos */
	SET_EADDR (NETARM_ETH_GEN_CTRL,
		   (NETARM_ETH_GCR_ERX | NETARM_ETH_GCR_ETX));

	return (0);
}


extern int eth_init (bd_t * bd)
{
	reset_eth ();
	return 0;
}

extern void eth_halt (void)
{
	SET_EADDR (NETARM_ETH_GEN_CTRL, 0);
}

/* Get a data block via Ethernet */
extern int eth_rx (void)
{
	int i;
	unsigned short rxlen;
	unsigned int *addr;
	unsigned int rxstatus, lastrxlen;
	char *pa;

	/* RXBR is 1, data block was received */
	if ((GET_EADDR (NETARM_ETH_GEN_STAT) & NETARM_ETH_GST_RXBR) == 0)
		return 0;

	/* get status register and the length of received block */
	rxstatus = GET_EADDR (NETARM_ETH_RX_STAT);
	rxlen = (rxstatus & NETARM_ETH_RXSTAT_SIZE) >> 16;

	if (rxlen == 0)
		return 0;

	/* clear RXBR to make fifo available */
	SET_EADDR (NETARM_ETH_GEN_STAT,
		   GET_EADDR (NETARM_ETH_GEN_STAT) & ~NETARM_ETH_GST_RXBR);

	/* clear TXBC to make fifo available */
	/* According to NETARM50 data manual you just have to clear
	   RXBR but that has no effect. Only after clearing TXBC the
	   Fifo becomes readable. */
	SET_EADDR (NETARM_ETH_GEN_STAT,
		   GET_EADDR (NETARM_ETH_GEN_STAT) & ~NETARM_ETH_GST_TXBC);

	addr = (unsigned int *) NetRxPackets[0];
	pa = (char *) NetRxPackets[0];

	/* read the fifo */
	for (i = 0; i < rxlen / 4; i++) {
		*addr = GET_EADDR (NETARM_ETH_FIFO_DAT1);
		addr++;
	}

	if (GET_EADDR (NETARM_ETH_GEN_STAT) & NETARM_ETH_GST_RXREGR) {
		/* RXFDB indicates wether the last word is 1,2,3 or 4 bytes long */
		lastrxlen =
			(GET_EADDR (NETARM_ETH_GEN_STAT) &
			 NETARM_ETH_GST_RXFDB) >> 28;
		*addr = GET_EADDR (NETARM_ETH_FIFO_DAT1);
		switch (lastrxlen) {
		case 1:
			*addr &= 0xff000000;
			break;
		case 2:
			*addr &= 0xffff0000;
			break;
		case 3:
			*addr &= 0xffffff00;
			break;
		}
	}

	/* Pass the packet up to the protocol layers. */
	NetReceive (NetRxPackets[0], rxlen);

	return rxlen;
}

/* Send a data block via Ethernet. */
extern int eth_send(void *packet, int length)
{
	int i, length32;
	char *pa;
	unsigned int *pa32, lastp = 0, rest;

	pa = (char *) packet;
	pa32 = (unsigned int *) packet;
	length32 = length / 4;
	rest = length % 4;

	/* make sure there's no garbage in the last word */
	switch (rest) {
	case 0:
		lastp = pa32[length32];
		length32--;
		break;
	case 1:
		lastp = pa32[length32] & 0x000000ff;
		break;
	case 2:
		lastp = pa32[length32] & 0x0000ffff;
		break;
	case 3:
		lastp = pa32[length32] & 0x00ffffff;
		break;
	}

	/* write to the fifo */
	for (i = 0; i < length32; i++)
		SET_EADDR (NETARM_ETH_FIFO_DAT1, pa32[i]);

	/* the last word is written to an extra register, this
	   starts the transmission */
	SET_EADDR (NETARM_ETH_FIFO_DAT2, lastp);

	/* NETARM_ETH_TXSTAT_TXOK should be checked, to know if the transmission
	   went fine. But we can't use the timer for a timeout loop because
	   of it is used already in upper layers. So we just try some times. */
	i = 0;
	while (i < 50000) {
		if ((GET_EADDR (NETARM_ETH_TX_STAT) & NETARM_ETH_TXSTAT_TXOK)
		    == NETARM_ETH_TXSTAT_TXOK)
			return 0;
		i++;
	}

	printf ("eth_send timeout\n");
	return 1;
}
