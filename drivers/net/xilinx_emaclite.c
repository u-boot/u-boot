/*
 * (C) Copyright 2007-2009 Michal Simek
 * (C) Copyright 2003 Xilinx Inc.
 *
 * Michal SIMEK <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <net.h>
#include <config.h>
#include <malloc.h>
#include <asm/io.h>
#include <phy.h>
#include <miiphy.h>
#include <fdtdec.h>

#undef DEBUG

#define ENET_ADDR_LENGTH	6

/* EmacLite constants */
#define XEL_BUFFER_OFFSET	0x0800	/* Next buffer's offset */
#define XEL_TPLR_OFFSET		0x07F4	/* Tx packet length */
#define XEL_TSR_OFFSET		0x07FC	/* Tx status */
#define XEL_RSR_OFFSET		0x17FC	/* Rx status */
#define XEL_RXBUFF_OFFSET	0x1000	/* Receive Buffer */

/* Xmit complete */
#define XEL_TSR_XMIT_BUSY_MASK		0x00000001UL
/* Xmit interrupt enable bit */
#define XEL_TSR_XMIT_IE_MASK		0x00000008UL
/* Buffer is active, SW bit only */
#define XEL_TSR_XMIT_ACTIVE_MASK	0x80000000UL
/* Program the MAC address */
#define XEL_TSR_PROGRAM_MASK		0x00000002UL
/* define for programming the MAC address into the EMAC Lite */
#define XEL_TSR_PROG_MAC_ADDR	(XEL_TSR_XMIT_BUSY_MASK | XEL_TSR_PROGRAM_MASK)

/* Transmit packet length upper byte */
#define XEL_TPLR_LENGTH_MASK_HI		0x0000FF00UL
/* Transmit packet length lower byte */
#define XEL_TPLR_LENGTH_MASK_LO		0x000000FFUL

/* Recv complete */
#define XEL_RSR_RECV_DONE_MASK		0x00000001UL
/* Recv interrupt enable bit */
#define XEL_RSR_RECV_IE_MASK		0x00000008UL

/* MDIO */
#define XEL_MDIOADDR_OFFSET	0x07E4		/* MDIO Address Register */
#define XEL_MDIOWR_OFFSET	0x07E8		/* MDIO Write Data Register */
#define XEL_MDIORD_OFFSET	0x07EC		/* MDIO Read Data Register */
#define XEL_MDIOCTRL_OFFSET	0x07F0		/* MDIO Control Register */

/* MDIO Address Register Bit Masks */
#define XEL_MDIOADDR_REGADR_MASK  0x0000001F	/* Register Address */
#define XEL_MDIOADDR_PHYADR_MASK  0x000003E0	/* PHY Address */
#define XEL_MDIOADDR_PHYADR_SHIFT 5
#define XEL_MDIOADDR_OP_MASK	  0x00000400	/* RD/WR Operation */

/* MDIO Write Data Register Bit Masks */
#define XEL_MDIOWR_WRDATA_MASK	  0x0000FFFF	/* Data to be Written */

/* MDIO Read Data Register Bit Masks */
#define XEL_MDIORD_RDDATA_MASK	  0x0000FFFF	/* Data to be Read */

/* MDIO Control Register Bit Masks */
#define XEL_MDIOCTRL_MDIOSTS_MASK 0x00000001	/* MDIO Status Mask */
#define XEL_MDIOCTRL_MDIOEN_MASK  0x00000008	/* MDIO Enable */

struct xemaclite {
	u32 nexttxbuffertouse;	/* Next TX buffer to write to */
	u32 nextrxbuffertouse;	/* Next RX buffer to read from */
	u32 txpp;		/* TX ping pong buffer */
	u32 rxpp;		/* RX ping pong buffer */
	int phyaddr;

	struct phy_device *phydev;
	struct mii_dev *bus;
};

static u32 etherrxbuff[PKTSIZE_ALIGN/4]; /* Receive buffer */

static void xemaclite_alignedread(u32 *srcptr, void *destptr, u32 bytecount)
{
	u32 i;
	u32 alignbuffer;
	u32 *to32ptr;
	u32 *from32ptr;
	u8 *to8ptr;
	u8 *from8ptr;

	from32ptr = (u32 *) srcptr;

	/* Word aligned buffer, no correction needed. */
	to32ptr = (u32 *) destptr;
	while (bytecount > 3) {
		*to32ptr++ = *from32ptr++;
		bytecount -= 4;
	}
	to8ptr = (u8 *) to32ptr;

	alignbuffer = *from32ptr++;
	from8ptr = (u8 *) &alignbuffer;

	for (i = 0; i < bytecount; i++)
		*to8ptr++ = *from8ptr++;
}

static void xemaclite_alignedwrite(void *srcptr, u32 destptr, u32 bytecount)
{
	u32 i;
	u32 alignbuffer;
	u32 *to32ptr = (u32 *) destptr;
	u32 *from32ptr;
	u8 *to8ptr;
	u8 *from8ptr;

	from32ptr = (u32 *) srcptr;
	while (bytecount > 3) {

		*to32ptr++ = *from32ptr++;
		bytecount -= 4;
	}

	alignbuffer = 0;
	to8ptr = (u8 *) &alignbuffer;
	from8ptr = (u8 *) from32ptr;

	for (i = 0; i < bytecount; i++)
		*to8ptr++ = *from8ptr++;

	*to32ptr++ = alignbuffer;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
static int mdio_wait(struct eth_device *dev)
{
	u32 timeout = 200;

	/* Wait till MDIO interface is ready to accept a new transaction. */
	while (timeout && in_be32(dev->iobase + XEL_MDIOCTRL_OFFSET)
			& XEL_MDIOCTRL_MDIOSTS_MASK)
		timeout--;

	if (!timeout) {
		printf("%s: Timeout\n", __func__);
		return 1;
	}
	return 0;
}

static u32 phyread(struct eth_device *dev, u32 phyaddress, u32 registernum,
								u16 *data)
{
	if (mdio_wait(dev))
		return 1;

	u32 ctrl_reg = in_be32(dev->iobase + XEL_MDIOCTRL_OFFSET);
	out_be32(dev->iobase + XEL_MDIOADDR_OFFSET,
		 XEL_MDIOADDR_OP_MASK |
		 ((phyaddress << XEL_MDIOADDR_PHYADR_SHIFT) | registernum));
	out_be32(dev->iobase + XEL_MDIOCTRL_OFFSET,
		 ctrl_reg | XEL_MDIOCTRL_MDIOSTS_MASK);

	if (mdio_wait(dev))
		return 1;

	/* Read data */
	*data = in_be32(dev->iobase + XEL_MDIORD_OFFSET);
	return 0;
}

static u32 phywrite(struct eth_device *dev, u32 phyaddress, u32 registernum,
								u16 data)
{
	if (mdio_wait(dev))
		return 1;

	/*
	 * Write the PHY address, register number and clear the OP bit in the
	 * MDIO Address register and then write the value into the MDIO Write
	 * Data register. Finally, set the Status bit in the MDIO Control
	 * register to start a MDIO write transaction.
	 */
	u32 ctrl_reg = in_be32(dev->iobase + XEL_MDIOCTRL_OFFSET);
	out_be32(dev->iobase + XEL_MDIOADDR_OFFSET,
		 ~XEL_MDIOADDR_OP_MASK &
		 ((phyaddress << XEL_MDIOADDR_PHYADR_SHIFT) | registernum));
	out_be32(dev->iobase + XEL_MDIOWR_OFFSET, data);
	out_be32(dev->iobase + XEL_MDIOCTRL_OFFSET,
		 ctrl_reg | XEL_MDIOCTRL_MDIOSTS_MASK);

	if (mdio_wait(dev))
		return 1;

	return 0;
}
#endif

static void emaclite_halt(struct eth_device *dev)
{
	debug("eth_halt\n");
}

/* Use MII register 1 (MII status register) to detect PHY */
#define PHY_DETECT_REG  1

/* Mask used to verify certain PHY features (or register contents)
 * in the register above:
 *  0x1000: 10Mbps full duplex support
 *  0x0800: 10Mbps half duplex support
 *  0x0008: Auto-negotiation support
 */
#define PHY_DETECT_MASK 0x1808

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
static int setup_phy(struct eth_device *dev)
{
	int i;
	u16 phyreg;
	struct xemaclite *emaclite = dev->priv;
	struct phy_device *phydev;

	u32 supported = SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full;

	if (emaclite->phyaddr != -1 ) {
		phyread(dev, emaclite->phyaddr, PHY_DETECT_REG, &phyreg);
		if ((phyreg != 0xFFFF) &&
		((phyreg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			debug("Default phy address %d is valid\n", emaclite->phyaddr);
		} else {
			debug("PHY address is not setup correctly %d\n", emaclite->phyaddr);
			emaclite->phyaddr = -1;
		}
	}

	if (emaclite->phyaddr == -1) {
		/* detect the PHY address */
		for (i = 31; i >= 0; i--) {
			phyread(dev, i, PHY_DETECT_REG, &phyreg);
			if ((phyreg != 0xFFFF) &&
			((phyreg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
				/* Found a valid PHY address */
				emaclite->phyaddr = i;
				debug("emaclite: Found valid phy address, %d\n",
									phyreg);
				break;
			}
		}
	}

	/* interface - look at tsec */
	phydev = phy_connect(emaclite->bus, emaclite->phyaddr, dev,
			     PHY_INTERFACE_MODE_MII);
	/*
	 * Phy can support 1000baseT but device NOT that's why phydev->supported
	 * must be setup for 1000baseT. phydev->advertising setups what speeds
	 * will be used for autonegotiation where 1000baseT must be disabled.
	 */
	phydev->supported = supported | SUPPORTED_1000baseT_Half |
						SUPPORTED_1000baseT_Full;
	phydev->advertising = supported;
	emaclite->phydev = phydev;
	phy_config(phydev);
	phy_startup(phydev);

	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return 0;
	}

	/* Do not setup anything */
	return 1;
}
#endif

static int emaclite_init(struct eth_device *dev, bd_t *bis)
{
	struct xemaclite *emaclite = dev->priv;
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	u32 temp;
#endif

	debug("EmacLite: Initialization Started\n");

/*
 * TX - TX_PING & TX_PONG initialization
 */
	/* Restart PING TX */
	out_be32 ((u32 *)(dev->iobase + XEL_TSR_OFFSET), 0);
	/* Copy MAC address */
	xemaclite_alignedwrite(dev->enetaddr, dev->iobase, ENET_ADDR_LENGTH);
	/* Set the length */
	out_be32 ((u32 *)(dev->iobase + XEL_TPLR_OFFSET), ENET_ADDR_LENGTH);
	/* Update the MAC address in the EMAC Lite */
	out_be32 ((u32 *)(dev->iobase + XEL_TSR_OFFSET), XEL_TSR_PROG_MAC_ADDR);
	/* Wait for EMAC Lite to finish with the MAC address update */
	while ((in_be32 ((u32 *)(dev->iobase + XEL_TSR_OFFSET)) &
		XEL_TSR_PROG_MAC_ADDR) != 0)
		;

	if (emaclite->txpp) {
		/* The same operation with PONG TX */
		out_be32 ((u32 *)(dev->iobase + XEL_TSR_OFFSET + XEL_BUFFER_OFFSET), 0);
		xemaclite_alignedwrite(dev->enetaddr, dev->iobase +
			XEL_BUFFER_OFFSET, ENET_ADDR_LENGTH);
		out_be32 ((u32 *)(dev->iobase + XEL_TPLR_OFFSET), ENET_ADDR_LENGTH);
		out_be32 ((u32 *)(dev->iobase + XEL_TSR_OFFSET + XEL_BUFFER_OFFSET),
			XEL_TSR_PROG_MAC_ADDR);
		while ((in_be32 ((u32 *)(dev->iobase + XEL_TSR_OFFSET +
			XEL_BUFFER_OFFSET)) & XEL_TSR_PROG_MAC_ADDR) != 0)
			;
	}

/*
 * RX - RX_PING & RX_PONG initialization
 */
	/* Write out the value to flush the RX buffer */
	out_be32 ((u32 *)(dev->iobase + XEL_RSR_OFFSET), XEL_RSR_RECV_IE_MASK);

	if (emaclite->rxpp)
		out_be32 ((u32 *)(dev->iobase + XEL_RSR_OFFSET + XEL_BUFFER_OFFSET),
			XEL_RSR_RECV_IE_MASK);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	/* Enable MII PHY */
	out_be32((u32 *)(dev->iobase + XEL_MDIOCTRL_OFFSET), XEL_MDIOCTRL_MDIOEN_MASK);
	temp = in_be32((u32 *)(dev->iobase + XEL_MDIOCTRL_OFFSET));
	if (temp & XEL_MDIOCTRL_MDIOEN_MASK)
		if (!setup_phy(dev))
			return -1;
#endif

	debug("EmacLite Initialization complete\n");
	return 0;
}

static int xemaclite_txbufferavailable(struct eth_device *dev)
{
	u32 reg;
	u32 txpingbusy;
	u32 txpongbusy;
	struct xemaclite *emaclite = dev->priv;

	/*
	 * Read the other buffer register
	 * and determine if the other buffer is available
	 */
	reg = in_be32 ((u32 *)(dev->iobase +
			emaclite->nexttxbuffertouse + 0));
	txpingbusy = ((reg & XEL_TSR_XMIT_BUSY_MASK) ==
			XEL_TSR_XMIT_BUSY_MASK);

	reg = in_be32 ((u32 *)(dev->iobase +
			(emaclite->nexttxbuffertouse ^ XEL_TSR_OFFSET) + 0));
	txpongbusy = ((reg & XEL_TSR_XMIT_BUSY_MASK) ==
			XEL_TSR_XMIT_BUSY_MASK);

	return !(txpingbusy && txpongbusy);
}

static int emaclite_send(struct eth_device *dev, void *ptr, int len)
{
	u32 reg;
	u32 baseaddress;
	struct xemaclite *emaclite = dev->priv;

	u32 maxtry = 1000;

	if (len > PKTSIZE)
		len = PKTSIZE;

	while (!xemaclite_txbufferavailable(dev) && maxtry) {
		udelay(10);
		maxtry--;
	}

	if (!maxtry) {
		printf("Error: Timeout waiting for ethernet TX buffer\n");
		/* Restart PING TX */
		out_be32 ((u32 *)(dev->iobase + XEL_TSR_OFFSET), 0);
		if (emaclite->txpp) {
			out_be32 ((u32 *)(dev->iobase + XEL_TSR_OFFSET +
				XEL_BUFFER_OFFSET), 0);
		}
		return -1;
	}

	/* Determine the expected TX buffer address */
	baseaddress = (dev->iobase + emaclite->nexttxbuffertouse);

	/* Determine if the expected buffer address is empty */
	reg = in_be32 ((u32 *)(baseaddress + XEL_TSR_OFFSET));
	if (((reg & XEL_TSR_XMIT_BUSY_MASK) == 0)
		&& ((in_be32 (((u32 *)(baseaddress) + XEL_TSR_OFFSET))
			& XEL_TSR_XMIT_ACTIVE_MASK) == 0)) {

		if (emaclite->txpp)
			emaclite->nexttxbuffertouse ^= XEL_BUFFER_OFFSET;

		debug("Send packet from 0x%x\n", baseaddress);
		/* Write the frame to the buffer */
		xemaclite_alignedwrite(ptr, baseaddress, len);
		out_be32 ((u32 *)(baseaddress + XEL_TPLR_OFFSET), (len &
			(XEL_TPLR_LENGTH_MASK_HI | XEL_TPLR_LENGTH_MASK_LO)));
		reg = in_be32 ((u32 *)(baseaddress + XEL_TSR_OFFSET));
		reg |= XEL_TSR_XMIT_BUSY_MASK;
		if ((reg & XEL_TSR_XMIT_IE_MASK) != 0)
			reg |= XEL_TSR_XMIT_ACTIVE_MASK;
		out_be32 ((u32 *)(baseaddress + XEL_TSR_OFFSET), reg);
		return 0;
	}

	if (emaclite->txpp) {
		/* Switch to second buffer */
		baseaddress ^= XEL_BUFFER_OFFSET;
		/* Determine if the expected buffer address is empty */
		reg = in_be32 ((u32 *)(baseaddress + XEL_TSR_OFFSET));
		if (((reg & XEL_TSR_XMIT_BUSY_MASK) == 0)
			&& ((in_be32 ((u32 *)(baseaddress + XEL_TSR_OFFSET))
				& XEL_TSR_XMIT_ACTIVE_MASK) == 0)) {
			debug("Send packet from 0x%x\n", baseaddress);
			/* Write the frame to the buffer */
			xemaclite_alignedwrite(ptr, baseaddress, len);
			out_be32 ((u32 *)(baseaddress + XEL_TPLR_OFFSET), (len &
				(XEL_TPLR_LENGTH_MASK_HI |
					XEL_TPLR_LENGTH_MASK_LO)));
			reg = in_be32 ((u32 *)(baseaddress + XEL_TSR_OFFSET));
			reg |= XEL_TSR_XMIT_BUSY_MASK;
			if ((reg & XEL_TSR_XMIT_IE_MASK) != 0)
				reg |= XEL_TSR_XMIT_ACTIVE_MASK;
			out_be32 ((u32 *)(baseaddress + XEL_TSR_OFFSET), reg);
			return 0;
		}
	}

	puts("Error while sending frame\n");
	return -1;
}

static int emaclite_recv(struct eth_device *dev)
{
	u32 length;
	u32 reg;
	u32 baseaddress;
	struct xemaclite *emaclite = dev->priv;

	baseaddress = dev->iobase + emaclite->nextrxbuffertouse;
	reg = in_be32 ((u32 *)(baseaddress + XEL_RSR_OFFSET));
	debug("Testing data at address 0x%x\n", baseaddress);
	if ((reg & XEL_RSR_RECV_DONE_MASK) == XEL_RSR_RECV_DONE_MASK) {
		if (emaclite->rxpp)
			emaclite->nextrxbuffertouse ^= XEL_BUFFER_OFFSET;
	} else {

		if (!emaclite->rxpp) {
			debug("No data was available - address 0x%x\n",
								baseaddress);
			return 0;
		} else {
			baseaddress ^= XEL_BUFFER_OFFSET;
			reg = in_be32 ((u32 *)(baseaddress + XEL_RSR_OFFSET));
			if ((reg & XEL_RSR_RECV_DONE_MASK) !=
						XEL_RSR_RECV_DONE_MASK) {
				debug("No data was available - address 0x%x\n",
						baseaddress);
				return 0;
			}
		}
	}
	/* Get the length of the frame that arrived */
	switch(((ntohl(in_be32 ((u32 *)(baseaddress + XEL_RXBUFF_OFFSET + 0xC)))) &
			0xFFFF0000 ) >> 16) {
		case 0x806:
			length = 42 + 20; /* FIXME size of ARP */
			debug("ARP Packet\n");
			break;
		case 0x800:
			length = 14 + 14 +
			(((ntohl(in_be32 ((u32 *)(baseaddress + XEL_RXBUFF_OFFSET +
						0x10)))) & 0xFFFF0000) >> 16);
			/* FIXME size of IP packet */
			debug ("IP Packet\n");
			break;
		default:
			debug("Other Packet\n");
			length = PKTSIZE;
			break;
	}

	xemaclite_alignedread((u32 *) (baseaddress + XEL_RXBUFF_OFFSET),
			etherrxbuff, length);

	/* Acknowledge the frame */
	reg = in_be32 ((u32 *)(baseaddress + XEL_RSR_OFFSET));
	reg &= ~XEL_RSR_RECV_DONE_MASK;
	out_be32 ((u32 *)(baseaddress + XEL_RSR_OFFSET), reg);

	debug("Packet receive from 0x%x, length %dB\n", baseaddress, length);
	net_process_received_packet((uchar *)etherrxbuff, length);
	return length;

}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
static int emaclite_miiphy_read(const char *devname, uchar addr,
							uchar reg, ushort *val)
{
	u32 ret;
	struct eth_device *dev = eth_get_dev();

	ret = phyread(dev, addr, reg, val);
	debug("emaclite: Read MII 0x%x, 0x%x, 0x%x\n", addr, reg, *val);
	return ret;
}

static int emaclite_miiphy_write(const char *devname, uchar addr,
							uchar reg, ushort val)
{
	struct eth_device *dev = eth_get_dev();

	debug("emaclite: Write MII 0x%x, 0x%x, 0x%x\n", addr, reg, val);
	return phywrite(dev, addr, reg, val);
}
#endif

int xilinx_emaclite_initialize(bd_t *bis, unsigned long base_addr,
							int txpp, int rxpp)
{
	struct eth_device *dev;
	struct xemaclite *emaclite;

	dev = calloc(1, sizeof(*dev));
	if (dev == NULL)
		return -1;

	emaclite = calloc(1, sizeof(struct xemaclite));
	if (emaclite == NULL) {
		free(dev);
		return -1;
	}

	dev->priv = emaclite;

	emaclite->txpp = txpp;
	emaclite->rxpp = rxpp;

	sprintf(dev->name, "Xelite.%lx", base_addr);

	dev->iobase = base_addr;
	dev->init = emaclite_init;
	dev->halt = emaclite_halt;
	dev->send = emaclite_send;
	dev->recv = emaclite_recv;

#ifdef CONFIG_PHY_ADDR
	emaclite->phyaddr = CONFIG_PHY_ADDR;
#else
	emaclite->phyaddr = -1;
#endif

	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	miiphy_register(dev->name, emaclite_miiphy_read, emaclite_miiphy_write);
	emaclite->bus = miiphy_get_dev_by_name(dev->name);
#endif

	return 1;
}

#ifdef CONFIG_OF_CONTROL
int xilinx_emaclite_of_init(const void *blob)
{
	int offset = 0;
	u32 ret = 0;
	u32 reg;

	do {
		offset = fdt_node_offset_by_compatible(blob, offset,
					"xlnx,xps-ethernetlite-1.00.a");
		if (offset != -1) {
			reg = fdtdec_get_addr(blob, offset, "reg");
			if (reg != FDT_ADDR_T_NONE) {
				u32 rxpp = fdtdec_get_int(blob, offset,
							"xlnx,rx-ping-pong", 0);
				u32 txpp = fdtdec_get_int(blob, offset,
							"xlnx,tx-ping-pong", 0);
				ret |= xilinx_emaclite_initialize(NULL, reg,
								txpp, rxpp);
			} else {
				debug("EMACLITE: Can't get base address\n");
				return -1;
			}
		}
	} while (offset != -1);

	return ret;
}
#endif
