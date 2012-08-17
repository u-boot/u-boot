/*
 * Altera 10/100/1000 triple speed ethernet mac driver
 *
 * Copyright (C) 2008 Altera Corporation.
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <asm/cache.h>
#include <asm/dma-mapping.h>
#include <miiphy.h>
#include "altera_tse.h"

/* sgdma debug - print descriptor */
static void alt_sgdma_print_desc(volatile struct alt_sgdma_descriptor *desc)
{
	debug("SGDMA DEBUG :\n");
	debug("desc->source : 0x%x \n", (unsigned int)desc->source);
	debug("desc->destination : 0x%x \n", (unsigned int)desc->destination);
	debug("desc->next : 0x%x \n", (unsigned int)desc->next);
	debug("desc->source_pad : 0x%x \n", (unsigned int)desc->source_pad);
	debug("desc->destination_pad : 0x%x \n",
	      (unsigned int)desc->destination_pad);
	debug("desc->next_pad : 0x%x \n", (unsigned int)desc->next_pad);
	debug("desc->bytes_to_transfer : 0x%x \n",
	      (unsigned int)desc->bytes_to_transfer);
	debug("desc->actual_bytes_transferred : 0x%x \n",
	      (unsigned int)desc->actual_bytes_transferred);
	debug("desc->descriptor_status : 0x%x \n",
	      (unsigned int)desc->descriptor_status);
	debug("desc->descriptor_control : 0x%x \n",
	      (unsigned int)desc->descriptor_control);
}

/* This is a generic routine that the SGDMA mode-specific routines
 * call to populate a descriptor.
 * arg1	    :pointer to first SGDMA descriptor.
 * arg2	    :pointer to next  SGDMA descriptor.
 * arg3	    :Address to where data to be written.
 * arg4	    :Address from where data to be read.
 * arg5	    :no of byte to transaction.
 * arg6	    :variable indicating to generate start of packet or not
 * arg7	    :read fixed
 * arg8	    :write fixed
 * arg9	    :read burst
 * arg10    :write burst
 * arg11    :atlantic_channel number
 */
static void alt_sgdma_construct_descriptor_burst(
	volatile struct alt_sgdma_descriptor *desc,
	volatile struct alt_sgdma_descriptor *next,
	unsigned int *read_addr,
	unsigned int *write_addr,
	unsigned short length_or_eop,
	int generate_eop,
	int read_fixed,
	int write_fixed_or_sop,
	int read_burst,
	int write_burst,
	unsigned char atlantic_channel)
{
	/*
	 * Mark the "next" descriptor as "not" owned by hardware. This prevents
	 * The SGDMA controller from continuing to process the chain. This is
	 * done as a single IO write to bypass cache, without flushing
	 * the entire descriptor, since only the 8-bit descriptor status must
	 * be flushed.
	 */
	if (!next)
		debug("Next descriptor not defined!!\n");

	next->descriptor_control = (next->descriptor_control &
		~ALT_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK);

	desc->source = (unsigned int *)((unsigned int)read_addr & 0x1FFFFFFF);
	desc->destination =
	    (unsigned int *)((unsigned int)write_addr & 0x1FFFFFFF);
	desc->next = (unsigned int *)((unsigned int)next & 0x1FFFFFFF);
	desc->source_pad = 0x0;
	desc->destination_pad = 0x0;
	desc->next_pad = 0x0;
	desc->bytes_to_transfer = length_or_eop;
	desc->actual_bytes_transferred = 0;
	desc->descriptor_status = 0x0;

	/* SGDMA burst not currently supported */
	desc->read_burst = 0;
	desc->write_burst = 0;

	/*
	 * Set the descriptor control block as follows:
	 * - Set "owned by hardware" bit
	 * - Optionally set "generate EOP" bit
	 * - Optionally set the "read from fixed address" bit
	 * - Optionally set the "write to fixed address bit (which serves
	 *   serves as a "generate SOP" control bit in memory-to-stream mode).
	 * - Set the 4-bit atlantic channel, if specified
	 *
	 * Note this step is performed after all other descriptor information
	 * has been filled out so that, if the controller already happens to be
	 * pointing at this descriptor, it will not run (via the "owned by
	 * hardware" bit) until all other descriptor has been set up.
	 */

	desc->descriptor_control =
	    ((ALT_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK) |
	     (generate_eop ?
	      ALT_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK : 0x0) |
	     (read_fixed ?
	      ALT_SGDMA_DESCRIPTOR_CONTROL_READ_FIXED_ADDRESS_MSK : 0x0) |
	     (write_fixed_or_sop ?
	      ALT_SGDMA_DESCRIPTOR_CONTROL_WRITE_FIXED_ADDRESS_MSK : 0x0) |
	     (atlantic_channel ? ((atlantic_channel & 0x0F) << 3) : 0)
		    );
}

static int alt_sgdma_do_sync_transfer(volatile struct alt_sgdma_registers *dev,
			       volatile struct alt_sgdma_descriptor *desc)
{
	unsigned int status;
	int counter = 0;

	/* Wait for any pending transfers to complete */
	alt_sgdma_print_desc(desc);
	status = dev->status;

	counter = 0;
	while (dev->status & ALT_SGDMA_STATUS_BUSY_MSK) {
		if (counter++ > ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR)
			break;
	}

	if (counter >= ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR)
		debug("Timeout waiting sgdma in do sync!\n");

	/*
	 * Clear any (previous) status register information
	 * that might occlude our error checking later.
	 */
	dev->status = 0xFF;

	/* Point the controller at the descriptor */
	dev->next_descriptor_pointer = (unsigned int)desc & 0x1FFFFFFF;
	debug("next desc in sgdma 0x%x\n",
	      (unsigned int)dev->next_descriptor_pointer);

	/*
	 * Set up SGDMA controller to:
	 * - Disable interrupt generation
	 * - Run once a valid descriptor is written to controller
	 * - Stop on an error with any particular descriptor
	 */
	dev->control = (ALT_SGDMA_CONTROL_RUN_MSK |
			ALT_SGDMA_CONTROL_STOP_DMA_ER_MSK);

	/* Wait for the descriptor (chain) to complete */
	status = dev->status;
	debug("wait for sgdma....");
	while (dev->status & ALT_SGDMA_STATUS_BUSY_MSK)
		;
	debug("done\n");

	/* Clear Run */
	dev->control = (dev->control & (~ALT_SGDMA_CONTROL_RUN_MSK));

	/* Get & clear status register contents */
	status = dev->status;
	dev->status = 0xFF;

	/* we really should check if the transfer completes properly */
	debug("tx sgdma status = 0x%x", status);
	return 0;
}

static int alt_sgdma_do_async_transfer(volatile struct alt_sgdma_registers *dev,
				volatile struct alt_sgdma_descriptor *desc)
{
	unsigned int status;
	int counter = 0;

	/* Wait for any pending transfers to complete */
	alt_sgdma_print_desc(desc);
	status = dev->status;

	counter = 0;
	while (dev->status & ALT_SGDMA_STATUS_BUSY_MSK) {
		if (counter++ > ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR)
			break;
	}

	if (counter >= ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR)
		debug("Timeout waiting sgdma in do async!\n");

	/*
	 * Clear the RUN bit in the control register. This is needed
	 * to restart the SGDMA engine later on.
	 */
	dev->control = 0;

	/*
	 * Clear any (previous) status register information
	 * that might occlude our error checking later.
	 */
	dev->status = 0xFF;

	/* Point the controller at the descriptor */
	dev->next_descriptor_pointer = (unsigned int)desc & 0x1FFFFFFF;

	/*
	 * Set up SGDMA controller to:
	 * - Disable interrupt generation
	 * - Run once a valid descriptor is written to controller
	 * - Stop on an error with any particular descriptor
	 */
	dev->control = (ALT_SGDMA_CONTROL_RUN_MSK |
			ALT_SGDMA_CONTROL_STOP_DMA_ER_MSK);

	/* we really should check if the transfer completes properly */
	return 0;
}

/* u-boot interface */
static int tse_adjust_link(struct altera_tse_priv *priv)
{
	unsigned int refvar;

	refvar = priv->mac_dev->command_config.image;

	if (!(priv->duplexity))
		refvar |= ALTERA_TSE_CMD_HD_ENA_MSK;
	else
		refvar &= ~ALTERA_TSE_CMD_HD_ENA_MSK;

	switch (priv->speed) {
	case 1000:
		refvar |= ALTERA_TSE_CMD_ETH_SPEED_MSK;
		refvar &= ~ALTERA_TSE_CMD_ENA_10_MSK;
		break;
	case 100:
		refvar &= ~ALTERA_TSE_CMD_ETH_SPEED_MSK;
		refvar &= ~ALTERA_TSE_CMD_ENA_10_MSK;
		break;
	case 10:
		refvar &= ~ALTERA_TSE_CMD_ETH_SPEED_MSK;
		refvar |= ALTERA_TSE_CMD_ENA_10_MSK;
		break;
	}
	priv->mac_dev->command_config.image = refvar;

	return 0;
}

static int tse_eth_send(struct eth_device *dev,
			volatile void *packet, int length)
{
	struct altera_tse_priv *priv = dev->priv;
	volatile struct alt_sgdma_registers *tx_sgdma = priv->sgdma_tx;
	volatile struct alt_sgdma_descriptor *tx_desc =
	    (volatile struct alt_sgdma_descriptor *)priv->tx_desc;

	volatile struct alt_sgdma_descriptor *tx_desc_cur =
	    (volatile struct alt_sgdma_descriptor *)&tx_desc[0];

	flush_dcache_range((unsigned long)packet,
			(unsigned long)packet + length);
	alt_sgdma_construct_descriptor_burst(
		(volatile struct alt_sgdma_descriptor *)&tx_desc[0],
		(volatile struct alt_sgdma_descriptor *)&tx_desc[1],
		(unsigned int *)packet,	/* read addr */
		(unsigned int *)0,
		length,	/* length or EOP ,will change for each tx */
		0x1,	/* gen eop */
		0x0,	/* read fixed */
		0x1,	/* write fixed or sop */
		0x0,	/* read burst */
		0x0,	/* write burst */
		0x0	/* channel */
		);
	debug("TX Packet @ 0x%x,0x%x bytes", (unsigned int)packet, length);

	/* send the packet */
	debug("sending packet\n");
	alt_sgdma_do_sync_transfer(tx_sgdma, tx_desc_cur);
	debug("sent %d bytes\n", tx_desc_cur->actual_bytes_transferred);
	return tx_desc_cur->actual_bytes_transferred;
}

static int tse_eth_rx(struct eth_device *dev)
{
	int packet_length = 0;
	struct altera_tse_priv *priv = dev->priv;
	volatile struct alt_sgdma_descriptor *rx_desc =
	    (volatile struct alt_sgdma_descriptor *)priv->rx_desc;
	volatile struct alt_sgdma_descriptor *rx_desc_cur = &rx_desc[0];

	if (rx_desc_cur->descriptor_status &
	    ALT_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_MSK) {
		debug("got packet\n");
		packet_length = rx_desc->actual_bytes_transferred;
		NetReceive(NetRxPackets[0], packet_length);

		/* start descriptor again */
		flush_dcache_range((unsigned long)(NetRxPackets[0]),
			(unsigned long)(NetRxPackets[0]) + PKTSIZE_ALIGN);
		alt_sgdma_construct_descriptor_burst(
			(volatile struct alt_sgdma_descriptor *)&rx_desc[0],
			(volatile struct alt_sgdma_descriptor *)&rx_desc[1],
			(unsigned int)0x0,	/* read addr */
			(unsigned int *)NetRxPackets[0],
			0x0,	/* length or EOP */
			0x0,	/* gen eop */
			0x0,	/* read fixed */
			0x0,	/* write fixed or sop */
			0x0,	/* read burst */
			0x0,	/* write burst */
			0x0	/* channel */
		    );

		/* setup the sgdma */
		alt_sgdma_do_async_transfer(priv->sgdma_rx, &rx_desc[0]);

		return packet_length;
	}

	return -1;
}

static void tse_eth_halt(struct eth_device *dev)
{
	/* don't do anything! */
	/* this gets called after each uboot  */
	/* network command.  don't need to reset the thing all of the time */
}

static void tse_eth_reset(struct eth_device *dev)
{
	/* stop sgdmas, disable tse receive */
	struct altera_tse_priv *priv = dev->priv;
	volatile struct alt_tse_mac *mac_dev = priv->mac_dev;
	volatile struct alt_sgdma_registers *rx_sgdma = priv->sgdma_rx;
	volatile struct alt_sgdma_registers *tx_sgdma = priv->sgdma_tx;
	int counter;
	volatile struct alt_sgdma_descriptor *rx_desc =
	    (volatile struct alt_sgdma_descriptor *)&priv->rx_desc[0];

	/* clear rx desc & wait for sgdma to complete */
	rx_desc->descriptor_control = 0;
	rx_sgdma->control = 0;
	counter = 0;
	while (rx_sgdma->status & ALT_SGDMA_STATUS_BUSY_MSK) {
		if (counter++ > ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR)
			break;
	}

	if (counter >= ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR) {
		debug("Timeout waiting for rx sgdma!\n");
		rx_sgdma->control = ALT_SGDMA_CONTROL_SOFTWARERESET_MSK;
		rx_sgdma->control = ALT_SGDMA_CONTROL_SOFTWARERESET_MSK;
	}

	counter = 0;
	tx_sgdma->control = 0;
	while (tx_sgdma->status & ALT_SGDMA_STATUS_BUSY_MSK) {
		if (counter++ > ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR)
			break;
	}

	if (counter >= ALT_TSE_SGDMA_BUSY_WATCHDOG_CNTR) {
		debug("Timeout waiting for tx sgdma!\n");
		tx_sgdma->control = ALT_SGDMA_CONTROL_SOFTWARERESET_MSK;
		tx_sgdma->control = ALT_SGDMA_CONTROL_SOFTWARERESET_MSK;
	}
	/* reset the mac */
	mac_dev->command_config.bits.transmit_enable = 1;
	mac_dev->command_config.bits.receive_enable = 1;
	mac_dev->command_config.bits.software_reset = 1;

	counter = 0;
	while (mac_dev->command_config.bits.software_reset) {
		if (counter++ > ALT_TSE_SW_RESET_WATCHDOG_CNTR)
			break;
	}

	if (counter >= ALT_TSE_SW_RESET_WATCHDOG_CNTR)
		debug("TSEMAC SW reset bit never cleared!\n");
}

static int tse_mdio_read(struct altera_tse_priv *priv, unsigned int regnum)
{
	volatile struct alt_tse_mac *mac_dev;
	unsigned int *mdio_regs;
	unsigned int data;
	u16 value;

	mac_dev = priv->mac_dev;

	/* set mdio address */
	mac_dev->mdio_phy1_addr = priv->phyaddr;
	mdio_regs = (unsigned int *)&mac_dev->mdio_phy1;

	/* get the data */
	data = mdio_regs[regnum];

	value = data & 0xffff;

	return value;
}

static int tse_mdio_write(struct altera_tse_priv *priv, unsigned int regnum,
		   unsigned int value)
{
	volatile struct alt_tse_mac *mac_dev;
	unsigned int *mdio_regs;
	unsigned int data;

	mac_dev = priv->mac_dev;

	/* set mdio address */
	mac_dev->mdio_phy1_addr = priv->phyaddr;
	mdio_regs = (unsigned int *)&mac_dev->mdio_phy1;

	/* get the data */
	data = (unsigned int)value;

	mdio_regs[regnum] = data;

	return 0;
}

/* MDIO access to phy */
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) && !defined(BITBANGMII)
static int altera_tse_miiphy_write(const char *devname, unsigned char addr,
				   unsigned char reg, unsigned short value)
{
	struct eth_device *dev;
	struct altera_tse_priv *priv;
	dev = eth_get_dev_by_name(devname);
	priv = dev->priv;

	tse_mdio_write(priv, (uint) reg, (uint) value);

	return 0;
}

static int altera_tse_miiphy_read(const char *devname, unsigned char addr,
				  unsigned char reg, unsigned short *value)
{
	struct eth_device *dev;
	struct altera_tse_priv *priv;
	volatile struct alt_tse_mac *mac_dev;
	unsigned int *mdio_regs;

	dev = eth_get_dev_by_name(devname);
	priv = dev->priv;

	mac_dev = priv->mac_dev;
	mac_dev->mdio_phy1_addr = (int)addr;
	mdio_regs = (unsigned int *)&mac_dev->mdio_phy1;

	*value = 0xffff & mdio_regs[reg];

	return 0;

}
#endif

/*
 * Also copied from tsec.c
 */
/* Parse the status register for link, and then do
 * auto-negotiation
 */
static uint mii_parse_sr(uint mii_reg, struct altera_tse_priv *priv)
{
	/*
	 * Wait if the link is up, and autonegotiation is in progress
	 * (ie - we're capable and it's not done)
	 */
	mii_reg = tse_mdio_read(priv, MIIM_STATUS);

	if (!(mii_reg & MIIM_STATUS_LINK) && (mii_reg & BMSR_ANEGCAPABLE)
	    && !(mii_reg & BMSR_ANEGCOMPLETE)) {
		int i = 0;

		puts("Waiting for PHY auto negotiation to complete");
		while (!(mii_reg & BMSR_ANEGCOMPLETE)) {
			/*
			 * Timeout reached ?
			 */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				priv->link = 0;
				return 0;
			}

			if ((i++ % 1000) == 0)
				putc('.');
			udelay(1000);	/* 1 ms */
			mii_reg = tse_mdio_read(priv, MIIM_STATUS);
		}
		puts(" done\n");
		priv->link = 1;
		udelay(500000);	/* another 500 ms (results in faster booting) */
	} else {
		if (mii_reg & MIIM_STATUS_LINK) {
			debug("Link is up\n");
			priv->link = 1;
		} else {
			debug("Link is down\n");
			priv->link = 0;
		}
	}

	return 0;
}

/* Parse the 88E1011's status register for speed and duplex
 * information
 */
static uint mii_parse_88E1011_psr(uint mii_reg, struct altera_tse_priv *priv)
{
	uint speed;

	mii_reg = tse_mdio_read(priv, MIIM_88E1011_PHY_STATUS);

	if ((mii_reg & MIIM_88E1011_PHYSTAT_LINK) &&
	    !(mii_reg & MIIM_88E1011_PHYSTAT_SPDDONE)) {
		int i = 0;

		puts("Waiting for PHY realtime link");
		while (!(mii_reg & MIIM_88E1011_PHYSTAT_SPDDONE)) {
			/* Timeout reached ? */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				priv->link = 0;
				break;
			}

			if ((i++ == 1000) == 0) {
				i = 0;
				puts(".");
			}
			udelay(1000);	/* 1 ms */
			mii_reg = tse_mdio_read(priv, MIIM_88E1011_PHY_STATUS);
		}
		puts(" done\n");
		udelay(500000);	/* another 500 ms (results in faster booting) */
	} else {
		if (mii_reg & MIIM_88E1011_PHYSTAT_LINK)
			priv->link = 1;
		else
			priv->link = 0;
	}

	if (mii_reg & MIIM_88E1011_PHYSTAT_DUPLEX)
		priv->duplexity = 1;
	else
		priv->duplexity = 0;

	speed = (mii_reg & MIIM_88E1011_PHYSTAT_SPEED);

	switch (speed) {
	case MIIM_88E1011_PHYSTAT_GBIT:
		priv->speed = 1000;
		debug("PHY Speed is 1000Mbit\n");
		break;
	case MIIM_88E1011_PHYSTAT_100:
		debug("PHY Speed is 100Mbit\n");
		priv->speed = 100;
		break;
	default:
		debug("PHY Speed is 10Mbit\n");
		priv->speed = 10;
	}

	return 0;
}

static uint mii_m88e1111s_setmode_sr(uint mii_reg, struct altera_tse_priv *priv)
{
	uint mii_data = tse_mdio_read(priv, mii_reg);
	mii_data &= 0xfff0;
	if ((priv->flags >= 1) && (priv->flags <= 4))
		mii_data |= 0xb;
	else if (priv->flags == 5)
		mii_data |= 0x4;

	return mii_data;
}

static uint mii_m88e1111s_setmode_cr(uint mii_reg, struct altera_tse_priv *priv)
{
	uint mii_data = tse_mdio_read(priv, mii_reg);
	mii_data &= ~0x82;
	if ((priv->flags >= 1) && (priv->flags <= 4))
		mii_data |= 0x82;

	return mii_data;
}

/*
 * Returns which value to write to the control register.
 * For 10/100, the value is slightly different
 */
static uint mii_cr_init(uint mii_reg, struct altera_tse_priv *priv)
{
	return MIIM_CONTROL_INIT;
}

/*
 * PHY & MDIO code
 * Need to add SGMII stuff
 *
 */

static struct phy_info phy_info_M88E1111S = {
	0x01410cc,
	"Marvell 88E1111S",
	4,
	(struct phy_cmd[]){	/* config */
			   /* Reset and configure the PHY */
			   {MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
			   {MIIM_88E1111_PHY_EXT_SR, 0x848f,
			    &mii_m88e1111s_setmode_sr},
			   /* Delay RGMII TX and RX */
			   {MIIM_88E1111_PHY_EXT_CR, 0x0cd2,
			    &mii_m88e1111s_setmode_cr},
			   {MIIM_GBIT_CONTROL, MIIM_GBIT_CONTROL_INIT, NULL},
			   {MIIM_ANAR, MIIM_ANAR_INIT, NULL},
			   {MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
			   {MIIM_CONTROL, MIIM_CONTROL_INIT, &mii_cr_init},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup */
			   /* Status is read once to clear old link state */
			   {MIIM_STATUS, miim_read, NULL},
			   /* Auto-negotiate */
			   {MIIM_STATUS, miim_read, &mii_parse_sr},
			   /* Read the status */
			   {MIIM_88E1011_PHY_STATUS, miim_read,
			    &mii_parse_88E1011_psr},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown */
			   {miim_end,}
			   },
};

/* a generic flavor.  */
static struct phy_info phy_info_generic = {
	0,
	"Unknown/Generic PHY",
	32,
	(struct phy_cmd[]){	/* config */
			   {MII_BMCR, BMCR_RESET, NULL},
			   {MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART, NULL},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup */
			   {MII_BMSR, miim_read, NULL},
			   {MII_BMSR, miim_read, &mii_parse_sr},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown */
			   {miim_end,}
			   }
};

static struct phy_info *phy_info[] = {
	&phy_info_M88E1111S,
	NULL
};

 /* Grab the identifier of the device's PHY, and search through
  * all of the known PHYs to see if one matches.	 If so, return
  * it, if not, return NULL
  */
static struct phy_info *get_phy_info(struct eth_device *dev)
{
	struct altera_tse_priv *priv = (struct altera_tse_priv *)dev->priv;
	uint phy_reg, phy_ID;
	int i;
	struct phy_info *theInfo = NULL;

	/* Grab the bits from PHYIR1, and put them in the upper half */
	phy_reg = tse_mdio_read(priv, MIIM_PHYIR1);
	phy_ID = (phy_reg & 0xffff) << 16;

	/* Grab the bits from PHYIR2, and put them in the lower half */
	phy_reg = tse_mdio_read(priv, MIIM_PHYIR2);
	phy_ID |= (phy_reg & 0xffff);

	/* loop through all the known PHY types, and find one that */
	/* matches the ID we read from the PHY. */
	for (i = 0; phy_info[i]; i++) {
		if (phy_info[i]->id == (phy_ID >> phy_info[i]->shift)) {
			theInfo = phy_info[i];
			break;
		}
	}

	if (theInfo == NULL) {
		theInfo = &phy_info_generic;
		debug("%s: No support for PHY id %x; assuming generic\n",
		      dev->name, phy_ID);
	} else
		debug("%s: PHY is %s (%x)\n", dev->name, theInfo->name, phy_ID);

	return theInfo;
}

/* Execute the given series of commands on the given device's
 * PHY, running functions as necessary
 */
static void phy_run_commands(struct altera_tse_priv *priv, struct phy_cmd *cmd)
{
	int i;
	uint result;

	for (i = 0; cmd->mii_reg != miim_end; i++) {
		if (cmd->mii_data == miim_read) {
			result = tse_mdio_read(priv, cmd->mii_reg);

			if (cmd->funct != NULL)
				(*(cmd->funct)) (result, priv);

		} else {
			if (cmd->funct != NULL)
				result = (*(cmd->funct)) (cmd->mii_reg, priv);
			else
				result = cmd->mii_data;

			tse_mdio_write(priv, cmd->mii_reg, result);

		}
		cmd++;
	}
}

/* Phy init code */
static int init_phy(struct eth_device *dev)
{
	struct altera_tse_priv *priv = (struct altera_tse_priv *)dev->priv;
	struct phy_info *curphy;

	/* Get the cmd structure corresponding to the attached
	 * PHY */
	curphy = get_phy_info(dev);

	if (curphy == NULL) {
		priv->phyinfo = NULL;
		debug("%s: No PHY found\n", dev->name);

		return 0;
	} else
		debug("%s found\n", curphy->name);
	priv->phyinfo = curphy;

	phy_run_commands(priv, priv->phyinfo->config);

	return 1;
}

static int tse_set_mac_address(struct eth_device *dev)
{
	struct altera_tse_priv *priv = dev->priv;
	volatile struct alt_tse_mac *mac_dev = priv->mac_dev;

	debug("Setting MAC address to 0x%02x%02x%02x%02x%02x%02x\n",
	      dev->enetaddr[5], dev->enetaddr[4],
	      dev->enetaddr[3], dev->enetaddr[2],
	      dev->enetaddr[1], dev->enetaddr[0]);
	mac_dev->mac_addr_0 = ((dev->enetaddr[3]) << 24 |
			       (dev->enetaddr[2]) << 16 |
			       (dev->enetaddr[1]) << 8 | (dev->enetaddr[0]));

	mac_dev->mac_addr_1 = ((dev->enetaddr[5] << 8 |
				(dev->enetaddr[4])) & 0xFFFF);

	/* Set the MAC address */
	mac_dev->supp_mac_addr_0_0 = mac_dev->mac_addr_0;
	mac_dev->supp_mac_addr_0_1 = mac_dev->mac_addr_1;

	/* Set the MAC address */
	mac_dev->supp_mac_addr_1_0 = mac_dev->mac_addr_0;
	mac_dev->supp_mac_addr_1_1 = mac_dev->mac_addr_1;

	/* Set the MAC address */
	mac_dev->supp_mac_addr_2_0 = mac_dev->mac_addr_0;
	mac_dev->supp_mac_addr_2_1 = mac_dev->mac_addr_1;

	/* Set the MAC address */
	mac_dev->supp_mac_addr_3_0 = mac_dev->mac_addr_0;
	mac_dev->supp_mac_addr_3_1 = mac_dev->mac_addr_1;
	return 0;
}

static int tse_eth_init(struct eth_device *dev, bd_t * bd)
{
	int dat;
	struct altera_tse_priv *priv = dev->priv;
	volatile struct alt_tse_mac *mac_dev = priv->mac_dev;
	volatile struct alt_sgdma_descriptor *tx_desc = priv->tx_desc;
	volatile struct alt_sgdma_descriptor *rx_desc = priv->rx_desc;
	volatile struct alt_sgdma_descriptor *rx_desc_cur =
	    (volatile struct alt_sgdma_descriptor *)&rx_desc[0];

	/* stop controller */
	debug("Reseting TSE & SGDMAs\n");
	tse_eth_reset(dev);

	/* start the phy */
	debug("Configuring PHY\n");
	phy_run_commands(priv, priv->phyinfo->startup);

	/* need to create sgdma */
	debug("Configuring tx desc\n");
	alt_sgdma_construct_descriptor_burst(
		(volatile struct alt_sgdma_descriptor *)&tx_desc[0],
		(volatile struct alt_sgdma_descriptor *)&tx_desc[1],
		(unsigned int *)NULL,	/* read addr */
		(unsigned int *)0,
		0,	/* length or EOP ,will change for each tx */
		0x1,	/* gen eop */
		0x0,	/* read fixed */
		0x1,	/* write fixed or sop */
		0x0,	/* read burst */
		0x0,	/* write burst */
		0x0	/* channel */
		);
	debug("Configuring rx desc\n");
	flush_dcache_range((unsigned long)(NetRxPackets[0]),
			(unsigned long)(NetRxPackets[0]) + PKTSIZE_ALIGN);
	alt_sgdma_construct_descriptor_burst(
		(volatile struct alt_sgdma_descriptor *)&rx_desc[0],
		(volatile struct alt_sgdma_descriptor *)&rx_desc[1],
		(unsigned int)0x0,	/* read addr */
		(unsigned int *)NetRxPackets[0],
		0x0,	/* length or EOP */
		0x0,	/* gen eop */
		0x0,	/* read fixed */
		0x0,	/* write fixed or sop */
		0x0,	/* read burst */
		0x0,	/* write burst */
		0x0	/* channel */
		);
	/* start rx async transfer */
	debug("Starting rx sgdma\n");
	alt_sgdma_do_async_transfer(priv->sgdma_rx, rx_desc_cur);

	/* start TSE */
	debug("Configuring TSE Mac\n");
	/* Initialize MAC registers */
	mac_dev->max_frame_length = PKTSIZE_ALIGN;
	mac_dev->rx_almost_empty_threshold = 8;
	mac_dev->rx_almost_full_threshold = 8;
	mac_dev->tx_almost_empty_threshold = 8;
	mac_dev->tx_almost_full_threshold = 3;
	mac_dev->tx_sel_empty_threshold =
	    CONFIG_SYS_ALTERA_TSE_TX_FIFO - 16;
	mac_dev->tx_sel_full_threshold = 0;
	mac_dev->rx_sel_empty_threshold =
	    CONFIG_SYS_ALTERA_TSE_TX_FIFO - 16;
	mac_dev->rx_sel_full_threshold = 0;

	/* NO Shift */
	mac_dev->rx_cmd_stat.bits.rx_shift16 = 0;
	mac_dev->tx_cmd_stat.bits.tx_shift16 = 0;

	/* enable MAC */
	dat = 0;
	dat = ALTERA_TSE_CMD_TX_ENA_MSK | ALTERA_TSE_CMD_RX_ENA_MSK;

	mac_dev->command_config.image = dat;

	/* configure the TSE core  */
	/*  -- output clocks,  */
	/*  -- and later config stuff for SGMII */
	if (priv->link) {
		debug("Adjusting TSE to link speed\n");
		tse_adjust_link(priv);
	}

	return priv->link ? 0 : -1;
}

/* TSE init code */
int altera_tse_initialize(u8 dev_num, int mac_base,
			  int sgdma_rx_base, int sgdma_tx_base,
			  u32 sgdma_desc_base, u32 sgdma_desc_size)
{
	struct altera_tse_priv *priv;
	struct eth_device *dev;
	struct alt_sgdma_descriptor *rx_desc;
	struct alt_sgdma_descriptor *tx_desc;
	unsigned long dma_handle;

	dev = (struct eth_device *)malloc(sizeof *dev);

	if (NULL == dev)
		return 0;

	memset(dev, 0, sizeof *dev);

	priv = malloc(sizeof(*priv));

	if (!priv) {
		free(dev);
		return 0;
	}
	if (sgdma_desc_size) {
		if (sgdma_desc_size < (sizeof(*tx_desc) * (3 + PKTBUFSRX))) {
			printf("ALTERA_TSE-%hu: "
			       "descriptor memory is too small\n", dev_num);
			free(priv);
			free(dev);
			return 0;
		}
		tx_desc = (struct alt_sgdma_descriptor *)sgdma_desc_base;
	} else {
		tx_desc = dma_alloc_coherent(sizeof(*tx_desc) * (3 + PKTBUFSRX),
					     &dma_handle);
	}

	rx_desc = tx_desc + 2;
	debug("tx desc: address = 0x%x\n", (unsigned int)tx_desc);
	debug("rx desc: address = 0x%x\n", (unsigned int)rx_desc);

	if (!tx_desc) {
		free(priv);
		free(dev);
		return 0;
	}
	memset(rx_desc, 0, (sizeof *rx_desc) * (PKTBUFSRX + 1));
	memset(tx_desc, 0, (sizeof *tx_desc) * 2);

	/* initialize tse priv */
	priv->mac_dev = (volatile struct alt_tse_mac *)mac_base;
	priv->sgdma_rx = (volatile struct alt_sgdma_registers *)sgdma_rx_base;
	priv->sgdma_tx = (volatile struct alt_sgdma_registers *)sgdma_tx_base;
	priv->phyaddr = CONFIG_SYS_ALTERA_TSE_PHY_ADDR;
	priv->flags = CONFIG_SYS_ALTERA_TSE_FLAGS;
	priv->rx_desc = rx_desc;
	priv->tx_desc = tx_desc;

	/* init eth structure */
	dev->priv = priv;
	dev->init = tse_eth_init;
	dev->halt = tse_eth_halt;
	dev->send = tse_eth_send;
	dev->recv = tse_eth_rx;
	dev->write_hwaddr = tse_set_mac_address;
	sprintf(dev->name, "%s-%hu", "ALTERA_TSE", dev_num);

	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) && !defined(BITBANGMII)
	miiphy_register(dev->name, altera_tse_miiphy_read,
			altera_tse_miiphy_write);
#endif

	init_phy(dev);

	return 1;
}
