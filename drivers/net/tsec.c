/*
 * Freescale Three Speed Ethernet Controller driver
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * Copyright 2004-2011, 2013 Freescale Semiconductor, Inc.
 * (C) Copyright 2003, Motorola, Inc.
 * author Andy Fleming
 *
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <tsec.h>
#include <fsl_mdio.h>
#include <asm/errno.h>
#include <asm/processor.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define TX_BUF_CNT		2

static uint rx_idx;		/* index of the current RX buffer */
static uint tx_idx;		/* index of the current TX buffer */

#ifdef __GNUC__
static struct txbd8 __iomem txbd[TX_BUF_CNT] __aligned(8);
static struct rxbd8 __iomem rxbd[PKTBUFSRX] __aligned(8);

#else
#error "rtx must be 64-bit aligned"
#endif

static int tsec_send(struct eth_device *dev, void *packet, int length);

/* Default initializations for TSEC controllers. */

static struct tsec_info_struct tsec_info[] = {
#ifdef CONFIG_TSEC1
	STD_TSEC_INFO(1),	/* TSEC1 */
#endif
#ifdef CONFIG_TSEC2
	STD_TSEC_INFO(2),	/* TSEC2 */
#endif
#ifdef CONFIG_MPC85XX_FEC
	{
		.regs = TSEC_GET_REGS(2, 0x2000),
		.devname = CONFIG_MPC85XX_FEC_NAME,
		.phyaddr = FEC_PHY_ADDR,
		.flags = FEC_FLAGS,
		.mii_devname = DEFAULT_MII_NAME
	},			/* FEC */
#endif
#ifdef CONFIG_TSEC3
	STD_TSEC_INFO(3),	/* TSEC3 */
#endif
#ifdef CONFIG_TSEC4
	STD_TSEC_INFO(4),	/* TSEC4 */
#endif
};

#define TBIANA_SETTINGS ( \
		TBIANA_ASYMMETRIC_PAUSE \
		| TBIANA_SYMMETRIC_PAUSE \
		| TBIANA_FULL_DUPLEX \
		)

/* By default force the TBI PHY into 1000Mbps full duplex when in SGMII mode */
#ifndef CONFIG_TSEC_TBICR_SETTINGS
#define CONFIG_TSEC_TBICR_SETTINGS ( \
		TBICR_PHY_RESET \
		| TBICR_ANEG_ENABLE \
		| TBICR_FULL_DUPLEX \
		| TBICR_SPEED1_SET \
		)
#endif /* CONFIG_TSEC_TBICR_SETTINGS */

/* Configure the TBI for SGMII operation */
static void tsec_configure_serdes(struct tsec_private *priv)
{
	/* Access TBI PHY registers at given TSEC register offset as opposed
	 * to the register offset used for external PHY accesses */
	tsec_local_mdio_write(priv->phyregs_sgmii, in_be32(&priv->regs->tbipa),
			0, TBI_ANA, TBIANA_SETTINGS);
	tsec_local_mdio_write(priv->phyregs_sgmii, in_be32(&priv->regs->tbipa),
			0, TBI_TBICON, TBICON_CLK_SELECT);
	tsec_local_mdio_write(priv->phyregs_sgmii, in_be32(&priv->regs->tbipa),
			0, TBI_CR, CONFIG_TSEC_TBICR_SETTINGS);
}

#ifdef CONFIG_MCAST_TFTP

/* CREDITS: linux gianfar driver, slightly adjusted... thanx. */

/* Set the appropriate hash bit for the given addr */

/* The algorithm works like so:
 * 1) Take the Destination Address (ie the multicast address), and
 * do a CRC on it (little endian), and reverse the bits of the
 * result.
 * 2) Use the 8 most significant bits as a hash into a 256-entry
 * table.  The table is controlled through 8 32-bit registers:
 * gaddr0-7.  gaddr0's MSB is entry 0, and gaddr7's LSB is entry
 * 255.  This means that the 3 most significant bits in the
 * hash index which gaddr register to use, and the 5 other bits
 * indicate which bit (assuming an IBM numbering scheme, which
 * for PowerPC (tm) is usually the case) in the register holds
 * the entry. */
static int
tsec_mcast_addr(struct eth_device *dev, const u8 *mcast_mac, u8 set)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;
	u32 result, value;
	u8 whichbit, whichreg;

	result = ether_crc(MAC_ADDR_LEN, mcast_mac);
	whichbit = (result >> 24) & 0x1f; /* the 5 LSB = which bit to set */
	whichreg = result >> 29; /* the 3 MSB = which reg to set it in */

	value = 1 << (31-whichbit);

	if (set)
		setbits_be32(&regs->hash.gaddr0 + whichreg, value);
	else
		clrbits_be32(&regs->hash.gaddr0 + whichreg, value);

	return 0;
}
#endif /* Multicast TFTP ? */

/* Initialized required registers to appropriate values, zeroing
 * those we don't care about (unless zero is bad, in which case,
 * choose a more appropriate value)
 */
static void init_registers(struct tsec __iomem *regs)
{
	/* Clear IEVENT */
	out_be32(&regs->ievent, IEVENT_INIT_CLEAR);

	out_be32(&regs->imask, IMASK_INIT_CLEAR);

	out_be32(&regs->hash.iaddr0, 0);
	out_be32(&regs->hash.iaddr1, 0);
	out_be32(&regs->hash.iaddr2, 0);
	out_be32(&regs->hash.iaddr3, 0);
	out_be32(&regs->hash.iaddr4, 0);
	out_be32(&regs->hash.iaddr5, 0);
	out_be32(&regs->hash.iaddr6, 0);
	out_be32(&regs->hash.iaddr7, 0);

	out_be32(&regs->hash.gaddr0, 0);
	out_be32(&regs->hash.gaddr1, 0);
	out_be32(&regs->hash.gaddr2, 0);
	out_be32(&regs->hash.gaddr3, 0);
	out_be32(&regs->hash.gaddr4, 0);
	out_be32(&regs->hash.gaddr5, 0);
	out_be32(&regs->hash.gaddr6, 0);
	out_be32(&regs->hash.gaddr7, 0);

	out_be32(&regs->rctrl, 0x00000000);

	/* Init RMON mib registers */
	memset((void *)&regs->rmon, 0, sizeof(regs->rmon));

	out_be32(&regs->rmon.cam1, 0xffffffff);
	out_be32(&regs->rmon.cam2, 0xffffffff);

	out_be32(&regs->mrblr, MRBLR_INIT_SETTINGS);

	out_be32(&regs->minflr, MINFLR_INIT_SETTINGS);

	out_be32(&regs->attr, ATTR_INIT_SETTINGS);
	out_be32(&regs->attreli, ATTRELI_INIT_SETTINGS);

}

/* Configure maccfg2 based on negotiated speed and duplex
 * reported by PHY handling code
 */
static void adjust_link(struct tsec_private *priv, struct phy_device *phydev)
{
	struct tsec __iomem *regs = priv->regs;
	u32 ecntrl, maccfg2;

	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return;
	}

	/* clear all bits relative with interface mode */
	ecntrl = in_be32(&regs->ecntrl);
	ecntrl &= ~ECNTRL_R100;

	maccfg2 = in_be32(&regs->maccfg2);
	maccfg2 &= ~(MACCFG2_IF | MACCFG2_FULL_DUPLEX);

	if (phydev->duplex)
		maccfg2 |= MACCFG2_FULL_DUPLEX;

	switch (phydev->speed) {
	case 1000:
		maccfg2 |= MACCFG2_GMII;
		break;
	case 100:
	case 10:
		maccfg2 |= MACCFG2_MII;

		/* Set R100 bit in all modes although
		 * it is only used in RGMII mode
		 */
		if (phydev->speed == 100)
			ecntrl |= ECNTRL_R100;
		break;
	default:
		printf("%s: Speed was bad\n", phydev->dev->name);
		break;
	}

	out_be32(&regs->ecntrl, ecntrl);
	out_be32(&regs->maccfg2, maccfg2);

	printf("Speed: %d, %s duplex%s\n", phydev->speed,
			(phydev->duplex) ? "full" : "half",
			(phydev->port == PORT_FIBRE) ? ", fiber mode" : "");
}

#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_ETSEC129
/*
 * When MACCFG1[Rx_EN] is enabled during system boot as part
 * of the eTSEC port initialization sequence,
 * the eTSEC Rx logic may not be properly initialized.
 */
void redundant_init(struct eth_device *dev)
{
	struct tsec_private *priv = dev->priv;
	struct tsec __iomem *regs = priv->regs;
	uint t, count = 0;
	int fail = 1;
	static const u8 pkt[] = {
		0x00, 0x1e, 0x4f, 0x12, 0xcb, 0x2c, 0x00, 0x25,
		0x64, 0xbb, 0xd1, 0xab, 0x08, 0x00, 0x45, 0x00,
		0x00, 0x5c, 0xdd, 0x22, 0x00, 0x00, 0x80, 0x01,
		0x1f, 0x71, 0x0a, 0xc1, 0x14, 0x22, 0x0a, 0xc1,
		0x14, 0x6a, 0x08, 0x00, 0xef, 0x7e, 0x02, 0x00,
		0x94, 0x05, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
		0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
		0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
		0x77, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
		0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
		0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
		0x71, 0x72};

	/* Enable promiscuous mode */
	setbits_be32(&regs->rctrl, 0x8);
	/* Enable loopback mode */
	setbits_be32(&regs->maccfg1, MACCFG1_LOOPBACK);
	/* Enable transmit and receive */
	setbits_be32(&regs->maccfg1, MACCFG1_RX_EN | MACCFG1_TX_EN);

	/* Tell the DMA it is clear to go */
	setbits_be32(&regs->dmactrl, DMACTRL_INIT_SETTINGS);
	out_be32(&regs->tstat, TSTAT_CLEAR_THALT);
	out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
	clrbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);
#ifdef CONFIG_LS102XA
	setbits_be32(&regs->dmactrl, DMACTRL_LE);
#endif

	do {
		uint16_t status;
		tsec_send(dev, (void *)pkt, sizeof(pkt));

		/* Wait for buffer to be received */
		for (t = 0; in_be16(&rxbd[rx_idx].status) & RXBD_EMPTY; t++) {
			if (t >= 10 * TOUT_LOOP) {
				printf("%s: tsec: rx error\n", dev->name);
				break;
			}
		}

		if (!memcmp(pkt, (void *)NetRxPackets[rx_idx], sizeof(pkt)))
			fail = 0;

		out_be16(&rxbd[rx_idx].length, 0);
		status = RXBD_EMPTY;
		if ((rx_idx + 1) == PKTBUFSRX)
			status |= RXBD_WRAP;
		out_be16(&rxbd[rx_idx].status, status);
		rx_idx = (rx_idx + 1) % PKTBUFSRX;

		if (in_be32(&regs->ievent) & IEVENT_BSY) {
			out_be32(&regs->ievent, IEVENT_BSY);
			out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
		}
		if (fail) {
			printf("loopback recv packet error!\n");
			clrbits_be32(&regs->maccfg1, MACCFG1_RX_EN);
			udelay(1000);
			setbits_be32(&regs->maccfg1, MACCFG1_RX_EN);
		}
	} while ((count++ < 4) && (fail == 1));

	if (fail)
		panic("eTSEC init fail!\n");
	/* Disable promiscuous mode */
	clrbits_be32(&regs->rctrl, 0x8);
	/* Disable loopback mode */
	clrbits_be32(&regs->maccfg1, MACCFG1_LOOPBACK);
}
#endif

/* Set up the buffers and their descriptors, and bring up the
 * interface
 */
static void startup_tsec(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;
	uint16_t status;
	int i;

	/* reset the indices to zero */
	rx_idx = 0;
	tx_idx = 0;
#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_ETSEC129
	uint svr;
#endif

	/* Point to the buffer descriptors */
	out_be32(&regs->tbase, (u32)&txbd[0]);
	out_be32(&regs->rbase, (u32)&rxbd[0]);

	/* Initialize the Rx Buffer descriptors */
	for (i = 0; i < PKTBUFSRX; i++) {
		out_be16(&rxbd[i].status, RXBD_EMPTY);
		out_be16(&rxbd[i].length, 0);
		out_be32(&rxbd[i].bufptr, (u32)NetRxPackets[i]);
	}
	status = in_be16(&rxbd[PKTBUFSRX - 1].status);
	out_be16(&rxbd[PKTBUFSRX - 1].status, status | RXBD_WRAP);

	/* Initialize the TX Buffer Descriptors */
	for (i = 0; i < TX_BUF_CNT; i++) {
		out_be16(&txbd[i].status, 0);
		out_be16(&txbd[i].length, 0);
		out_be32(&txbd[i].bufptr, 0);
	}
	status = in_be16(&txbd[TX_BUF_CNT - 1].status);
	out_be16(&txbd[TX_BUF_CNT - 1].status, status | TXBD_WRAP);

#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_ETSEC129
	svr = get_svr();
	if ((SVR_MAJ(svr) == 1) || IS_SVR_REV(svr, 2, 0))
		redundant_init(dev);
#endif
	/* Enable Transmit and Receive */
	setbits_be32(&regs->maccfg1, MACCFG1_RX_EN | MACCFG1_TX_EN);

	/* Tell the DMA it is clear to go */
	setbits_be32(&regs->dmactrl, DMACTRL_INIT_SETTINGS);
	out_be32(&regs->tstat, TSTAT_CLEAR_THALT);
	out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
	clrbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);
#ifdef CONFIG_LS102XA
	setbits_be32(&regs->dmactrl, DMACTRL_LE);
#endif
}

/* This returns the status bits of the device.	The return value
 * is never checked, and this is what the 8260 driver did, so we
 * do the same.	 Presumably, this would be zero if there were no
 * errors
 */
static int tsec_send(struct eth_device *dev, void *packet, int length)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;
	uint16_t status;
	int result = 0;
	int i;

	/* Find an empty buffer descriptor */
	for (i = 0; in_be16(&txbd[tx_idx].status) & TXBD_READY; i++) {
		if (i >= TOUT_LOOP) {
			debug("%s: tsec: tx buffers full\n", dev->name);
			return result;
		}
	}

	out_be32(&txbd[tx_idx].bufptr, (u32)packet);
	out_be16(&txbd[tx_idx].length, length);
	status = in_be16(&txbd[tx_idx].status);
	out_be16(&txbd[tx_idx].status, status |
		(TXBD_READY | TXBD_LAST | TXBD_CRC | TXBD_INTERRUPT));

	/* Tell the DMA to go */
	out_be32(&regs->tstat, TSTAT_CLEAR_THALT);

	/* Wait for buffer to be transmitted */
	for (i = 0; in_be16(&txbd[tx_idx].status) & TXBD_READY; i++) {
		if (i >= TOUT_LOOP) {
			debug("%s: tsec: tx error\n", dev->name);
			return result;
		}
	}

	tx_idx = (tx_idx + 1) % TX_BUF_CNT;
	result = in_be16(&txbd[tx_idx].status) & TXBD_STATS;

	return result;
}

static int tsec_recv(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;

	while (!(in_be16(&rxbd[rx_idx].status) & RXBD_EMPTY)) {
		int length = in_be16(&rxbd[rx_idx].length);
		uint16_t status = in_be16(&rxbd[rx_idx].status);

		/* Send the packet up if there were no errors */
		if (!(status & RXBD_STATS))
			NetReceive(NetRxPackets[rx_idx], length - 4);
		else
			printf("Got error %x\n", (status & RXBD_STATS));

		out_be16(&rxbd[rx_idx].length, 0);

		status = RXBD_EMPTY;
		/* Set the wrap bit if this is the last element in the list */
		if ((rx_idx + 1) == PKTBUFSRX)
			status |= RXBD_WRAP;
		out_be16(&rxbd[rx_idx].status, status);

		rx_idx = (rx_idx + 1) % PKTBUFSRX;
	}

	if (in_be32(&regs->ievent) & IEVENT_BSY) {
		out_be32(&regs->ievent, IEVENT_BSY);
		out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
	}

	return -1;

}

/* Stop the interface */
static void tsec_halt(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;

	clrbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);
	setbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);

	while ((in_be32(&regs->ievent) & (IEVENT_GRSC | IEVENT_GTSC))
			!= (IEVENT_GRSC | IEVENT_GTSC))
		;

	clrbits_be32(&regs->maccfg1, MACCFG1_TX_EN | MACCFG1_RX_EN);

	/* Shut down the PHY, as needed */
	phy_shutdown(priv->phydev);
}

/* Initializes data structures and registers for the controller,
 * and brings the interface up.	 Returns the link status, meaning
 * that it returns success if the link is up, failure otherwise.
 * This allows u-boot to find the first active controller.
 */
static int tsec_init(struct eth_device *dev, bd_t * bd)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct tsec __iomem *regs = priv->regs;
	u32 tempval;
	int ret;

	/* Make sure the controller is stopped */
	tsec_halt(dev);

	/* Init MACCFG2.  Defaults to GMII */
	out_be32(&regs->maccfg2, MACCFG2_INIT_SETTINGS);

	/* Init ECNTRL */
	out_be32(&regs->ecntrl, ECNTRL_INIT_SETTINGS);

	/* Copy the station address into the address registers.
	 * For a station address of 0x12345678ABCD in transmission
	 * order (BE), MACnADDR1 is set to 0xCDAB7856 and
	 * MACnADDR2 is set to 0x34120000.
	 */
	tempval = (dev->enetaddr[5] << 24) | (dev->enetaddr[4] << 16) |
		  (dev->enetaddr[3] << 8)  |  dev->enetaddr[2];

	out_be32(&regs->macstnaddr1, tempval);

	tempval = (dev->enetaddr[1] << 24) | (dev->enetaddr[0] << 16);

	out_be32(&regs->macstnaddr2, tempval);

	/* Clear out (for the most part) the other registers */
	init_registers(regs);

	/* Ready the device for tx/rx */
	startup_tsec(dev);

	/* Start up the PHY */
	ret = phy_startup(priv->phydev);
	if (ret) {
		printf("Could not initialize PHY %s\n",
		       priv->phydev->dev->name);
		return ret;
	}

	adjust_link(priv, priv->phydev);

	/* If there's no link, fail */
	return priv->phydev->link ? 0 : -1;
}

static phy_interface_t tsec_get_interface(struct tsec_private *priv)
{
	struct tsec __iomem *regs = priv->regs;
	u32 ecntrl;

	ecntrl = in_be32(&regs->ecntrl);

	if (ecntrl & ECNTRL_SGMII_MODE)
		return PHY_INTERFACE_MODE_SGMII;

	if (ecntrl & ECNTRL_TBI_MODE) {
		if (ecntrl & ECNTRL_REDUCED_MODE)
			return PHY_INTERFACE_MODE_RTBI;
		else
			return PHY_INTERFACE_MODE_TBI;
	}

	if (ecntrl & ECNTRL_REDUCED_MODE) {
		if (ecntrl & ECNTRL_REDUCED_MII_MODE)
			return PHY_INTERFACE_MODE_RMII;
		else {
			phy_interface_t interface = priv->interface;

			/*
			 * This isn't autodetected, so it must
			 * be set by the platform code.
			 */
			if ((interface == PHY_INTERFACE_MODE_RGMII_ID) ||
				 (interface == PHY_INTERFACE_MODE_RGMII_TXID) ||
				 (interface == PHY_INTERFACE_MODE_RGMII_RXID))
				return interface;

			return PHY_INTERFACE_MODE_RGMII;
		}
	}

	if (priv->flags & TSEC_GIGABIT)
		return PHY_INTERFACE_MODE_GMII;

	return PHY_INTERFACE_MODE_MII;
}


/* Discover which PHY is attached to the device, and configure it
 * properly.  If the PHY is not recognized, then return 0
 * (failure).  Otherwise, return 1
 */
static int init_phy(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct phy_device *phydev;
	struct tsec __iomem *regs = priv->regs;
	u32 supported = (SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full);

	if (priv->flags & TSEC_GIGABIT)
		supported |= SUPPORTED_1000baseT_Full;

	/* Assign a Physical address to the TBI */
	out_be32(&regs->tbipa, CONFIG_SYS_TBIPA_VALUE);

	priv->interface = tsec_get_interface(priv);

	if (priv->interface == PHY_INTERFACE_MODE_SGMII)
		tsec_configure_serdes(priv);

	phydev = phy_connect(priv->bus, priv->phyaddr, dev, priv->interface);

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;

	phy_config(phydev);

	return 1;
}

/* Initialize device structure. Returns success if PHY
 * initialization succeeded (i.e. if it recognizes the PHY)
 */
static int tsec_initialize(bd_t *bis, struct tsec_info_struct *tsec_info)
{
	struct eth_device *dev;
	int i;
	struct tsec_private *priv;

	dev = (struct eth_device *)malloc(sizeof *dev);

	if (NULL == dev)
		return 0;

	memset(dev, 0, sizeof *dev);

	priv = (struct tsec_private *)malloc(sizeof(*priv));

	if (NULL == priv)
		return 0;

	priv->regs = tsec_info->regs;
	priv->phyregs_sgmii = tsec_info->miiregs_sgmii;

	priv->phyaddr = tsec_info->phyaddr;
	priv->flags = tsec_info->flags;

	sprintf(dev->name, tsec_info->devname);
	priv->interface = tsec_info->interface;
	priv->bus = miiphy_get_dev_by_name(tsec_info->mii_devname);
	dev->iobase = 0;
	dev->priv = priv;
	dev->init = tsec_init;
	dev->halt = tsec_halt;
	dev->send = tsec_send;
	dev->recv = tsec_recv;
#ifdef CONFIG_MCAST_TFTP
	dev->mcast = tsec_mcast_addr;
#endif

	/* Tell u-boot to get the addr from the env */
	for (i = 0; i < 6; i++)
		dev->enetaddr[i] = 0;

	eth_register(dev);

	/* Reset the MAC */
	setbits_be32(&priv->regs->maccfg1, MACCFG1_SOFT_RESET);
	udelay(2);  /* Soft Reset must be asserted for 3 TX clocks */
	clrbits_be32(&priv->regs->maccfg1, MACCFG1_SOFT_RESET);

	/* Try to initialize PHY here, and return */
	return init_phy(dev);
}

/*
 * Initialize all the TSEC devices
 *
 * Returns the number of TSEC devices that were initialized
 */
int tsec_eth_init(bd_t *bis, struct tsec_info_struct *tsecs, int num)
{
	int i;
	int ret, count = 0;

	for (i = 0; i < num; i++) {
		ret = tsec_initialize(bis, &tsecs[i]);
		if (ret > 0)
			count += ret;
	}

	return count;
}

int tsec_standard_init(bd_t *bis)
{
	struct fsl_pq_mdio_info info;

	info.regs = TSEC_GET_MDIO_REGS_BASE(1);
	info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &info);

	return tsec_eth_init(bis, tsec_info, ARRAY_SIZE(tsec_info));
}
