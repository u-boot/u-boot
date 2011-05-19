/*
 * Freescale Three Speed Ethernet Controller driver
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * Copyright 2004-2011 Freescale Semiconductor, Inc.
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

DECLARE_GLOBAL_DATA_PTR;

#define TX_BUF_CNT		2

static uint rxIdx;		/* index of the current RX buffer */
static uint txIdx;		/* index of the current TX buffer */

typedef volatile struct rtxbd {
	txbd8_t txbd[TX_BUF_CNT];
	rxbd8_t rxbd[PKTBUFSRX];
} RTXBD;

#define MAXCONTROLLERS	(8)

static struct tsec_private *privlist[MAXCONTROLLERS];
static int num_tsecs = 0;

#ifdef __GNUC__
static RTXBD rtx __attribute__ ((aligned(8)));
#else
#error "rtx must be 64-bit aligned"
#endif

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
		.regs = (tsec_t *)(TSEC_BASE_ADDR + 0x2000),
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
 * gaddr0-7.  gaddr0's MSB is entry 0, and gaddr7's LSB is
 * gaddr7.  This means that the 3 most significant bits in the
 * hash index which gaddr register to use, and the 5 other bits
 * indicate which bit (assuming an IBM numbering scheme, which
 * for PowerPC (tm) is usually the case) in the tregister holds
 * the entry. */
static int
tsec_mcast_addr (struct eth_device *dev, u8 mcast_mac, u8 set)
{
	struct tsec_private *priv = privlist[1];
	volatile tsec_t *regs = priv->regs;
	volatile u32  *reg_array, value;
	u8 result, whichbit, whichreg;

	result = (u8)((ether_crc(MAC_ADDR_LEN,mcast_mac) >> 24) & 0xff);
	whichbit = result & 0x1f;	/* the 5 LSB = which bit to set */
	whichreg = result >> 5;		/* the 3 MSB = which reg to set it in */
	value = (1 << (31-whichbit));

	reg_array = &(regs->hash.gaddr0);

	if (set) {
		reg_array[whichreg] |= value;
	} else {
		reg_array[whichreg] &= ~value;
	}
	return 0;
}
#endif /* Multicast TFTP ? */

/* Initialized required registers to appropriate values, zeroing
 * those we don't care about (unless zero is bad, in which case,
 * choose a more appropriate value)
 */
static void init_registers(tsec_t *regs)
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
	memset((void *)&(regs->rmon), 0, sizeof(rmon_mib_t));

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
	tsec_t *regs = priv->regs;
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

/* Set up the buffers and their descriptors, and bring up the
 * interface
 */
static void startup_tsec(struct eth_device *dev)
{
	int i;
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	tsec_t *regs = priv->regs;

	/* reset the indices to zero */
	rxIdx = 0;
	txIdx = 0;

	/* Point to the buffer descriptors */
	out_be32(&regs->tbase, (unsigned int)(&rtx.txbd[txIdx]));
	out_be32(&regs->rbase, (unsigned int)(&rtx.rxbd[rxIdx]));

	/* Initialize the Rx Buffer descriptors */
	for (i = 0; i < PKTBUFSRX; i++) {
		rtx.rxbd[i].status = RXBD_EMPTY;
		rtx.rxbd[i].length = 0;
		rtx.rxbd[i].bufPtr = (uint) NetRxPackets[i];
	}
	rtx.rxbd[PKTBUFSRX - 1].status |= RXBD_WRAP;

	/* Initialize the TX Buffer Descriptors */
	for (i = 0; i < TX_BUF_CNT; i++) {
		rtx.txbd[i].status = 0;
		rtx.txbd[i].length = 0;
		rtx.txbd[i].bufPtr = 0;
	}
	rtx.txbd[TX_BUF_CNT - 1].status |= TXBD_WRAP;

	/* Enable Transmit and Receive */
	setbits_be32(&regs->maccfg1, MACCFG1_RX_EN | MACCFG1_TX_EN);

	/* Tell the DMA it is clear to go */
	setbits_be32(&regs->dmactrl, DMACTRL_INIT_SETTINGS);
	out_be32(&regs->tstat, TSTAT_CLEAR_THALT);
	out_be32(&regs->rstat, RSTAT_CLEAR_RHALT);
	clrbits_be32(&regs->dmactrl, DMACTRL_GRS | DMACTRL_GTS);
}

/* This returns the status bits of the device.	The return value
 * is never checked, and this is what the 8260 driver did, so we
 * do the same.	 Presumably, this would be zero if there were no
 * errors
 */
static int tsec_send(struct eth_device *dev, volatile void *packet, int length)
{
	int i;
	int result = 0;
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	tsec_t *regs = priv->regs;

	/* Find an empty buffer descriptor */
	for (i = 0; rtx.txbd[txIdx].status & TXBD_READY; i++) {
		if (i >= TOUT_LOOP) {
			debug("%s: tsec: tx buffers full\n", dev->name);
			return result;
		}
	}

	rtx.txbd[txIdx].bufPtr = (uint) packet;
	rtx.txbd[txIdx].length = length;
	rtx.txbd[txIdx].status |=
	    (TXBD_READY | TXBD_LAST | TXBD_CRC | TXBD_INTERRUPT);

	/* Tell the DMA to go */
	out_be32(&regs->tstat, TSTAT_CLEAR_THALT);

	/* Wait for buffer to be transmitted */
	for (i = 0; rtx.txbd[txIdx].status & TXBD_READY; i++) {
		if (i >= TOUT_LOOP) {
			debug("%s: tsec: tx error\n", dev->name);
			return result;
		}
	}

	txIdx = (txIdx + 1) % TX_BUF_CNT;
	result = rtx.txbd[txIdx].status & TXBD_STATS;

	return result;
}

static int tsec_recv(struct eth_device *dev)
{
	int length;
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	tsec_t *regs = priv->regs;

	while (!(rtx.rxbd[rxIdx].status & RXBD_EMPTY)) {

		length = rtx.rxbd[rxIdx].length;

		/* Send the packet up if there were no errors */
		if (!(rtx.rxbd[rxIdx].status & RXBD_STATS)) {
			NetReceive(NetRxPackets[rxIdx], length - 4);
		} else {
			printf("Got error %x\n",
			       (rtx.rxbd[rxIdx].status & RXBD_STATS));
		}

		rtx.rxbd[rxIdx].length = 0;

		/* Set the wrap bit if this is the last element in the list */
		rtx.rxbd[rxIdx].status =
		    RXBD_EMPTY | (((rxIdx + 1) == PKTBUFSRX) ? RXBD_WRAP : 0);

		rxIdx = (rxIdx + 1) % PKTBUFSRX;
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
	tsec_t *regs = priv->regs;

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
	uint tempval;
	char tmpbuf[MAC_ADDR_LEN];
	int i;
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	tsec_t *regs = priv->regs;

	/* Make sure the controller is stopped */
	tsec_halt(dev);

	/* Init MACCFG2.  Defaults to GMII */
	out_be32(&regs->maccfg2, MACCFG2_INIT_SETTINGS);

	/* Init ECNTRL */
	out_be32(&regs->ecntrl, ECNTRL_INIT_SETTINGS);

	/* Copy the station address into the address registers.
	 * Backwards, because little endian MACS are dumb */
	for (i = 0; i < MAC_ADDR_LEN; i++)
		tmpbuf[MAC_ADDR_LEN - 1 - i] = dev->enetaddr[i];

	tempval = (tmpbuf[0] << 24) | (tmpbuf[1] << 16) | (tmpbuf[2] << 8) |
		  tmpbuf[3];

	out_be32(&regs->macstnaddr1, tempval);

	tempval = *((uint *) (tmpbuf + 4));

	out_be32(&regs->macstnaddr2, tempval);

	/* Clear out (for the most part) the other registers */
	init_registers(regs);

	/* Ready the device for tx/rx */
	startup_tsec(dev);

	/* Start up the PHY */
	phy_startup(priv->phydev);

	adjust_link(priv, priv->phydev);

	/* If there's no link, fail */
	return priv->phydev->link ? 0 : -1;
}

static phy_interface_t tsec_get_interface(struct tsec_private *priv)
{
	tsec_t *regs = priv->regs;
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
	tsec_t *regs = priv->regs;
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

	privlist[num_tsecs++] = priv;
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

	info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &info);

	return tsec_eth_init(bis, tsec_info, ARRAY_SIZE(tsec_info));
}
