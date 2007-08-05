/*
 * Freescale Three Speed Ethernet Controller driver
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * Copyright 2004, 2007 Freescale Semiconductor, Inc.
 * (C) Copyright 2003, Motorola, Inc.
 * author Andy Fleming
 *
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>

#if defined(CONFIG_TSEC_ENET)
#include "tsec.h"
#include "miiphy.h"

DECLARE_GLOBAL_DATA_PTR;

#define TX_BUF_CNT		2

static uint rxIdx;		/* index of the current RX buffer */
static uint txIdx;		/* index of the current TX buffer */

typedef volatile struct rtxbd {
	txbd8_t txbd[TX_BUF_CNT];
	rxbd8_t rxbd[PKTBUFSRX];
} RTXBD;

struct tsec_info_struct {
	unsigned int phyaddr;
	u32 flags;
	unsigned int phyregidx;
};

/* The tsec_info structure contains 3 values which the
 * driver uses to determine how to operate a given ethernet
 * device. The information needed is:
 *  phyaddr - The address of the PHY which is attached to
 *	the given device.
 *
 *  flags - This variable indicates whether the device
 *	supports gigabit speed ethernet, and whether it should be
 *	in reduced mode.
 *
 *  phyregidx - This variable specifies which ethernet device
 *	controls the MII Management registers which are connected
 *	to the PHY.  For now, only TSEC1 (index 0) has
 *	access to the PHYs, so all of the entries have "0".
 *
 * The values specified in the table are taken from the board's
 * config file in include/configs/.  When implementing a new
 * board with ethernet capability, it is necessary to define:
 *   TSECn_PHY_ADDR
 *   TSECn_PHYIDX
 *
 * for n = 1,2,3, etc.  And for FEC:
 *   FEC_PHY_ADDR
 *   FEC_PHYIDX
 */
static struct tsec_info_struct tsec_info[] = {
#if defined(CONFIG_TSEC1)
#if defined(CONFIG_MPC8544DS) || defined(CONFIG_MPC8641HPCN)
	{TSEC1_PHY_ADDR, TSEC_GIGABIT | TSEC_REDUCED, TSEC1_PHYIDX},
#else
	{TSEC1_PHY_ADDR, TSEC_GIGABIT, TSEC1_PHYIDX},
#endif
	{0, 0, 0},
#endif
#if defined(CONFIG_TSEC2)
#if defined(CONFIG_MPC8641HPCN)
	{TSEC2_PHY_ADDR, TSEC_GIGABIT | TSEC_REDUCED, TSEC2_PHYIDX},
#else
	{TSEC2_PHY_ADDR, TSEC_GIGABIT, TSEC2_PHYIDX},
#endif
	{0, 0, 0},
#endif
#ifdef CONFIG_MPC85XX_FEC
	{FEC_PHY_ADDR, 0, FEC_PHYIDX},
#else
#if defined(CONFIG_TSEC3)
	{TSEC3_PHY_ADDR, TSEC_GIGABIT | TSEC_REDUCED, TSEC3_PHYIDX},
#else
	{0, 0, 0},
#endif
#if defined(CONFIG_TSEC4)
	{TSEC4_PHY_ADDR, TSEC_GIGABIT | TSEC_REDUCED, TSEC4_PHYIDX},
#else
	{0, 0, 0},
#endif
#endif
};

#define MAXCONTROLLERS	(4)

static int relocated = 0;

static struct tsec_private *privlist[MAXCONTROLLERS];

#ifdef __GNUC__
static RTXBD rtx __attribute__ ((aligned(8)));
#else
#error "rtx must be 64-bit aligned"
#endif

static int tsec_send(struct eth_device *dev,
		     volatile void *packet, int length);
static int tsec_recv(struct eth_device *dev);
static int tsec_init(struct eth_device *dev, bd_t * bd);
static void tsec_halt(struct eth_device *dev);
static void init_registers(volatile tsec_t * regs);
static void startup_tsec(struct eth_device *dev);
static int init_phy(struct eth_device *dev);
void write_phy_reg(struct tsec_private *priv, uint regnum, uint value);
uint read_phy_reg(struct tsec_private *priv, uint regnum);
struct phy_info *get_phy_info(struct eth_device *dev);
void phy_run_commands(struct tsec_private *priv, struct phy_cmd *cmd);
static void adjust_link(struct eth_device *dev);
static void relocate_cmds(void);
static int tsec_miiphy_write(char *devname, unsigned char addr,
			     unsigned char reg, unsigned short value);
static int tsec_miiphy_read(char *devname, unsigned char addr,
			    unsigned char reg, unsigned short *value);

/* Initialize device structure. Returns success if PHY
 * initialization succeeded (i.e. if it recognizes the PHY)
 */
int tsec_initialize(bd_t * bis, int index, char *devname)
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

	privlist[index] = priv;
	priv->regs = (volatile tsec_t *)(TSEC_BASE_ADDR + index * TSEC_SIZE);
	priv->phyregs = (volatile tsec_t *)(TSEC_BASE_ADDR +
					    tsec_info[index].phyregidx *
					    TSEC_SIZE);

	priv->phyaddr = tsec_info[index].phyaddr;
	priv->flags = tsec_info[index].flags;

	sprintf(dev->name, devname);
	dev->iobase = 0;
	dev->priv = priv;
	dev->init = tsec_init;
	dev->halt = tsec_halt;
	dev->send = tsec_send;
	dev->recv = tsec_recv;

	/* Tell u-boot to get the addr from the env */
	for (i = 0; i < 6; i++)
		dev->enetaddr[i] = 0;

	eth_register(dev);

	/* Reset the MAC */
	priv->regs->maccfg1 |= MACCFG1_SOFT_RESET;
	priv->regs->maccfg1 &= ~(MACCFG1_SOFT_RESET);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) \
	&& !defined(BITBANGMII)
	miiphy_register(dev->name, tsec_miiphy_read, tsec_miiphy_write);
#endif

	/* Try to initialize PHY here, and return */
	return init_phy(dev);
}

/* Initializes data structures and registers for the controller,
 * and brings the interface up.	 Returns the link status, meaning
 * that it returns success if the link is up, failure otherwise.
 * This allows u-boot to find the first active controller.
 */
int tsec_init(struct eth_device *dev, bd_t * bd)
{
	uint tempval;
	char tmpbuf[MAC_ADDR_LEN];
	int i;
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	volatile tsec_t *regs = priv->regs;

	/* Make sure the controller is stopped */
	tsec_halt(dev);

	/* Init MACCFG2.  Defaults to GMII */
	regs->maccfg2 = MACCFG2_INIT_SETTINGS;

	/* Init ECNTRL */
	regs->ecntrl = ECNTRL_INIT_SETTINGS;

	/* Copy the station address into the address registers.
	 * Backwards, because little endian MACS are dumb */
	for (i = 0; i < MAC_ADDR_LEN; i++) {
		tmpbuf[MAC_ADDR_LEN - 1 - i] = dev->enetaddr[i];
	}
	regs->macstnaddr1 = *((uint *) (tmpbuf));

	tempval = *((uint *) (tmpbuf + 4));

	regs->macstnaddr2 = tempval;

	/* reset the indices to zero */
	rxIdx = 0;
	txIdx = 0;

	/* Clear out (for the most part) the other registers */
	init_registers(regs);

	/* Ready the device for tx/rx */
	startup_tsec(dev);

	/* If there's no link, fail */
	return priv->link;

}

/* Write value to the device's PHY through the registers
 * specified in priv, modifying the register specified in regnum.
 * It will wait for the write to be done (or for a timeout to
 * expire) before exiting
 */
void write_phy_reg(struct tsec_private *priv, uint regnum, uint value)
{
	volatile tsec_t *regbase = priv->phyregs;
	uint phyid = priv->phyaddr;
	int timeout = 1000000;

	regbase->miimadd = (phyid << 8) | regnum;
	regbase->miimcon = value;
	asm("sync");

	timeout = 1000000;
	while ((regbase->miimind & MIIMIND_BUSY) && timeout--) ;
}

/* Reads register regnum on the device's PHY through the
 * registers specified in priv.	 It lowers and raises the read
 * command, and waits for the data to become valid (miimind
 * notvalid bit cleared), and the bus to cease activity (miimind
 * busy bit cleared), and then returns the value
 */
uint read_phy_reg(struct tsec_private *priv, uint regnum)
{
	uint value;
	volatile tsec_t *regbase = priv->phyregs;
	uint phyid = priv->phyaddr;

	/* Put the address of the phy, and the register
	 * number into MIIMADD */
	regbase->miimadd = (phyid << 8) | regnum;

	/* Clear the command register, and wait */
	regbase->miimcom = 0;
	asm("sync");

	/* Initiate a read command, and wait */
	regbase->miimcom = MIIM_READ_COMMAND;
	asm("sync");

	/* Wait for the the indication that the read is done */
	while ((regbase->miimind & (MIIMIND_NOTVALID | MIIMIND_BUSY))) ;

	/* Grab the value read from the PHY */
	value = regbase->miimstat;

	return value;
}

/* Discover which PHY is attached to the device, and configure it
 * properly.  If the PHY is not recognized, then return 0
 * (failure).  Otherwise, return 1
 */
static int init_phy(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	struct phy_info *curphy;
	volatile tsec_t *regs = (volatile tsec_t *)(TSEC_BASE_ADDR);

	/* Assign a Physical address to the TBI */
	regs->tbipa = TBIPA_VALUE;
	regs = (volatile tsec_t *)(TSEC_BASE_ADDR + TSEC_SIZE);
	regs->tbipa = TBIPA_VALUE;
	asm("sync");

	/* Reset MII (due to new addresses) */
	priv->phyregs->miimcfg = MIIMCFG_RESET;
	asm("sync");
	priv->phyregs->miimcfg = MIIMCFG_INIT_VALUE;
	asm("sync");
	while (priv->phyregs->miimind & MIIMIND_BUSY) ;

	if (0 == relocated)
		relocate_cmds();

	/* Get the cmd structure corresponding to the attached
	 * PHY */
	curphy = get_phy_info(dev);

	if (curphy == NULL) {
		priv->phyinfo = NULL;
		printf("%s: No PHY found\n", dev->name);

		return 0;
	}

	priv->phyinfo = curphy;

	phy_run_commands(priv, priv->phyinfo->config);

	return 1;
}

/*
 * Returns which value to write to the control register.
 * For 10/100, the value is slightly different
 */
uint mii_cr_init(uint mii_reg, struct tsec_private * priv)
{
	if (priv->flags & TSEC_GIGABIT)
		return MIIM_CONTROL_INIT;
	else
		return MIIM_CR_INIT;
}

/* Parse the status register for link, and then do
 * auto-negotiation
 */
uint mii_parse_sr(uint mii_reg, struct tsec_private * priv)
{
	/*
	 * Wait if PHY is capable of autonegotiation and autonegotiation
	 * is not complete.
	 */
	mii_reg = read_phy_reg(priv, MIIM_STATUS);
	if ((mii_reg & PHY_BMSR_AUTN_ABLE)
	    && !(mii_reg & PHY_BMSR_AUTN_COMP)) {
		int i = 0;

		puts("Waiting for PHY auto negotiation to complete");
		while (!((mii_reg & PHY_BMSR_AUTN_COMP)
			 && (mii_reg & MIIM_STATUS_LINK))) {
			/*
			 * Timeout reached ?
			 */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				priv->link = 0;
				return 0;
			}

			if ((i++ % 1000) == 0) {
				putc('.');
			}
			udelay(1000);	/* 1 ms */
			mii_reg = read_phy_reg(priv, MIIM_STATUS);
		}
		puts(" done\n");
		priv->link = 1;
		udelay(500000);	/* another 500 ms (results in faster booting) */
	} else {
		priv->link = 1;
	}

	return 0;
}

/* Generic function which updates the speed and duplex.  If
 * autonegotiation is enabled, it uses the AND of the link
 * partner's advertised capabilities and our advertised
 * capabilities.  If autonegotiation is disabled, we use the
 * appropriate bits in the control register.
 *
 * Stolen from Linux's mii.c and phy_device.c
 */
uint mii_parse_link(uint mii_reg, struct tsec_private *priv)
{
	/* We're using autonegotiation */
	if (mii_reg & PHY_BMSR_AUTN_ABLE) {
		uint lpa = 0;
		uint gblpa = 0;

		/* Check for gigabit capability */
		if (mii_reg & PHY_BMSR_EXT) {
			/* We want a list of states supported by
			 * both PHYs in the link
			 */
			gblpa = read_phy_reg(priv, PHY_1000BTSR);
			gblpa &= read_phy_reg(priv, PHY_1000BTCR) << 2;
		}

		/* Set the baseline so we only have to set them
		 * if they're different
		 */
		priv->speed = 10;
		priv->duplexity = 0;

		/* Check the gigabit fields */
		if (gblpa & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)) {
			priv->speed = 1000;

			if (gblpa & PHY_1000BTSR_1000FD)
				priv->duplexity = 1;

			/* We're done! */
			return 0;
		}

		lpa = read_phy_reg(priv, PHY_ANAR);
		lpa &= read_phy_reg(priv, PHY_ANLPAR);

		if (lpa & (PHY_ANLPAR_TXFD | PHY_ANLPAR_TX)) {
			priv->speed = 100;

			if (lpa & PHY_ANLPAR_TXFD)
				priv->duplexity = 1;

		} else if (lpa & PHY_ANLPAR_10FD)
			priv->duplexity = 1;
	} else {
		uint bmcr = read_phy_reg(priv, PHY_BMCR);

		priv->speed = 10;
		priv->duplexity = 0;

		if (bmcr & PHY_BMCR_DPLX)
			priv->duplexity = 1;

		if (bmcr & PHY_BMCR_1000_MBPS)
			priv->speed = 1000;
		else if (bmcr & PHY_BMCR_100_MBPS)
			priv->speed = 100;
	}

	return 0;
}

/*
 * Parse the BCM54xx status register for speed and duplex information.
 * The linux sungem_phy has this information, but in a table format.
 */
uint mii_parse_BCM54xx_sr(uint mii_reg, struct tsec_private *priv)
{

	switch((mii_reg & MIIM_BCM54xx_AUXSTATUS_LINKMODE_MASK) >> MIIM_BCM54xx_AUXSTATUS_LINKMODE_SHIFT){

		case 1:
			printf("Enet starting in 10BT/HD\n");
			priv->duplexity = 0;
			priv->speed = 10;
			break;

		case 2:
			printf("Enet starting in 10BT/FD\n");
			priv->duplexity = 1;
			priv->speed = 10;
			break;

		case 3:
			printf("Enet starting in 100BT/HD\n");
			priv->duplexity = 0;
			priv->speed = 100;
			break;

		case 5:
			printf("Enet starting in 100BT/FD\n");
			priv->duplexity = 1;
			priv->speed = 100;
			break;

		case 6:
			printf("Enet starting in 1000BT/HD\n");
			priv->duplexity = 0;
			priv->speed = 1000;
			break;

		case 7:
			printf("Enet starting in 1000BT/FD\n");
			priv->duplexity = 1;
			priv->speed = 1000;
			break;

		default:
			printf("Auto-neg error, defaulting to 10BT/HD\n");
			priv->duplexity = 0;
			priv->speed = 10;
			break;
	}

	return 0;

}
/* Parse the 88E1011's status register for speed and duplex
 * information
 */
uint mii_parse_88E1011_psr(uint mii_reg, struct tsec_private * priv)
{
	uint speed;

	mii_reg = read_phy_reg(priv, MIIM_88E1011_PHY_STATUS);

	if (!((mii_reg & MIIM_88E1011_PHYSTAT_SPDDONE) &&
	      (mii_reg & MIIM_88E1011_PHYSTAT_LINK))) {
		int i = 0;

		puts("Waiting for PHY realtime link");
		while (!((mii_reg & MIIM_88E1011_PHYSTAT_SPDDONE) &&
			 (mii_reg & MIIM_88E1011_PHYSTAT_LINK))) {
			/*
			 * Timeout reached ?
			 */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				priv->link = 0;
				break;
			}

			if ((i++ % 1000) == 0) {
				putc('.');
			}
			udelay(1000);	/* 1 ms */
			mii_reg = read_phy_reg(priv, MIIM_88E1011_PHY_STATUS);
		}
		puts(" done\n");
		udelay(500000);	/* another 500 ms (results in faster booting) */
	}

	if (mii_reg & MIIM_88E1011_PHYSTAT_DUPLEX)
		priv->duplexity = 1;
	else
		priv->duplexity = 0;

	speed = (mii_reg & MIIM_88E1011_PHYSTAT_SPEED);

	switch (speed) {
	case MIIM_88E1011_PHYSTAT_GBIT:
		priv->speed = 1000;
		break;
	case MIIM_88E1011_PHYSTAT_100:
		priv->speed = 100;
		break;
	default:
		priv->speed = 10;
	}

	return 0;
}

/* Parse the cis8201's status register for speed and duplex
 * information
 */
uint mii_parse_cis8201(uint mii_reg, struct tsec_private * priv)
{
	uint speed;

	if (mii_reg & MIIM_CIS8201_AUXCONSTAT_DUPLEX)
		priv->duplexity = 1;
	else
		priv->duplexity = 0;

	speed = mii_reg & MIIM_CIS8201_AUXCONSTAT_SPEED;
	switch (speed) {
	case MIIM_CIS8201_AUXCONSTAT_GBIT:
		priv->speed = 1000;
		break;
	case MIIM_CIS8201_AUXCONSTAT_100:
		priv->speed = 100;
		break;
	default:
		priv->speed = 10;
		break;
	}

	return 0;
}

/* Parse the vsc8244's status register for speed and duplex
 * information
 */
uint mii_parse_vsc8244(uint mii_reg, struct tsec_private * priv)
{
	uint speed;

	if (mii_reg & MIIM_VSC8244_AUXCONSTAT_DUPLEX)
		priv->duplexity = 1;
	else
		priv->duplexity = 0;

	speed = mii_reg & MIIM_VSC8244_AUXCONSTAT_SPEED;
	switch (speed) {
	case MIIM_VSC8244_AUXCONSTAT_GBIT:
		priv->speed = 1000;
		break;
	case MIIM_VSC8244_AUXCONSTAT_100:
		priv->speed = 100;
		break;
	default:
		priv->speed = 10;
		break;
	}

	return 0;
}

/* Parse the DM9161's status register for speed and duplex
 * information
 */
uint mii_parse_dm9161_scsr(uint mii_reg, struct tsec_private * priv)
{
	if (mii_reg & (MIIM_DM9161_SCSR_100F | MIIM_DM9161_SCSR_100H))
		priv->speed = 100;
	else
		priv->speed = 10;

	if (mii_reg & (MIIM_DM9161_SCSR_100F | MIIM_DM9161_SCSR_10F))
		priv->duplexity = 1;
	else
		priv->duplexity = 0;

	return 0;
}

/*
 * Hack to write all 4 PHYs with the LED values
 */
uint mii_cis8204_fixled(uint mii_reg, struct tsec_private * priv)
{
	uint phyid;
	volatile tsec_t *regbase = priv->phyregs;
	int timeout = 1000000;

	for (phyid = 0; phyid < 4; phyid++) {
		regbase->miimadd = (phyid << 8) | mii_reg;
		regbase->miimcon = MIIM_CIS8204_SLEDCON_INIT;
		asm("sync");

		timeout = 1000000;
		while ((regbase->miimind & MIIMIND_BUSY) && timeout--) ;
	}

	return MIIM_CIS8204_SLEDCON_INIT;
}

uint mii_cis8204_setmode(uint mii_reg, struct tsec_private * priv)
{
	if (priv->flags & TSEC_REDUCED)
		return MIIM_CIS8204_EPHYCON_INIT | MIIM_CIS8204_EPHYCON_RGMII;
	else
		return MIIM_CIS8204_EPHYCON_INIT;
}

/* Initialized required registers to appropriate values, zeroing
 * those we don't care about (unless zero is bad, in which case,
 * choose a more appropriate value)
 */
static void init_registers(volatile tsec_t * regs)
{
	/* Clear IEVENT */
	regs->ievent = IEVENT_INIT_CLEAR;

	regs->imask = IMASK_INIT_CLEAR;

	regs->hash.iaddr0 = 0;
	regs->hash.iaddr1 = 0;
	regs->hash.iaddr2 = 0;
	regs->hash.iaddr3 = 0;
	regs->hash.iaddr4 = 0;
	regs->hash.iaddr5 = 0;
	regs->hash.iaddr6 = 0;
	regs->hash.iaddr7 = 0;

	regs->hash.gaddr0 = 0;
	regs->hash.gaddr1 = 0;
	regs->hash.gaddr2 = 0;
	regs->hash.gaddr3 = 0;
	regs->hash.gaddr4 = 0;
	regs->hash.gaddr5 = 0;
	regs->hash.gaddr6 = 0;
	regs->hash.gaddr7 = 0;

	regs->rctrl = 0x00000000;

	/* Init RMON mib registers */
	memset((void *)&(regs->rmon), 0, sizeof(rmon_mib_t));

	regs->rmon.cam1 = 0xffffffff;
	regs->rmon.cam2 = 0xffffffff;

	regs->mrblr = MRBLR_INIT_SETTINGS;

	regs->minflr = MINFLR_INIT_SETTINGS;

	regs->attr = ATTR_INIT_SETTINGS;
	regs->attreli = ATTRELI_INIT_SETTINGS;

}

/* Configure maccfg2 based on negotiated speed and duplex
 * reported by PHY handling code
 */
static void adjust_link(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	volatile tsec_t *regs = priv->regs;

	if (priv->link) {
		if (priv->duplexity != 0)
			regs->maccfg2 |= MACCFG2_FULL_DUPLEX;
		else
			regs->maccfg2 &= ~(MACCFG2_FULL_DUPLEX);

		switch (priv->speed) {
		case 1000:
			regs->maccfg2 = ((regs->maccfg2 & ~(MACCFG2_IF))
					 | MACCFG2_GMII);
			break;
		case 100:
		case 10:
			regs->maccfg2 = ((regs->maccfg2 & ~(MACCFG2_IF))
					 | MACCFG2_MII);

			/* Set R100 bit in all modes although
			 * it is only used in RGMII mode
			 */
			if (priv->speed == 100)
				regs->ecntrl |= ECNTRL_R100;
			else
				regs->ecntrl &= ~(ECNTRL_R100);
			break;
		default:
			printf("%s: Speed was bad\n", dev->name);
			break;
		}

		printf("Speed: %d, %s duplex\n", priv->speed,
		       (priv->duplexity) ? "full" : "half");

	} else {
		printf("%s: No link.\n", dev->name);
	}
}

/* Set up the buffers and their descriptors, and bring up the
 * interface
 */
static void startup_tsec(struct eth_device *dev)
{
	int i;
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	volatile tsec_t *regs = priv->regs;

	/* Point to the buffer descriptors */
	regs->tbase = (unsigned int)(&rtx.txbd[txIdx]);
	regs->rbase = (unsigned int)(&rtx.rxbd[rxIdx]);

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

	/* Start up the PHY */
	if(priv->phyinfo)
		phy_run_commands(priv, priv->phyinfo->startup);

	adjust_link(dev);

	/* Enable Transmit and Receive */
	regs->maccfg1 |= (MACCFG1_RX_EN | MACCFG1_TX_EN);

	/* Tell the DMA it is clear to go */
	regs->dmactrl |= DMACTRL_INIT_SETTINGS;
	regs->tstat = TSTAT_CLEAR_THALT;
	regs->dmactrl &= ~(DMACTRL_GRS | DMACTRL_GTS);
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
	volatile tsec_t *regs = priv->regs;

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
	regs->tstat = TSTAT_CLEAR_THALT;

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
	volatile tsec_t *regs = priv->regs;

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

	if (regs->ievent & IEVENT_BSY) {
		regs->ievent = IEVENT_BSY;
		regs->rstat = RSTAT_CLEAR_RHALT;
	}

	return -1;

}

/* Stop the interface */
static void tsec_halt(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	volatile tsec_t *regs = priv->regs;

	regs->dmactrl &= ~(DMACTRL_GRS | DMACTRL_GTS);
	regs->dmactrl |= (DMACTRL_GRS | DMACTRL_GTS);

	while (!(regs->ievent & (IEVENT_GRSC | IEVENT_GTSC))) ;

	regs->maccfg1 &= ~(MACCFG1_TX_EN | MACCFG1_RX_EN);

	/* Shut down the PHY, as needed */
	if(priv->phyinfo)
		phy_run_commands(priv, priv->phyinfo->shutdown);
}

struct phy_info phy_info_M88E1149S = {
       0x1410ca,
       "Marvell 88E1149S",
       4,
       (struct phy_cmd[]){     /* config */
               /* Reset and configure the PHY */
               {MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
               {0x1d, 0x1f, NULL},
               {0x1e, 0x200c, NULL},
               {0x1d, 0x5, NULL},
               {0x1e, 0x0, NULL},
               {0x1e, 0x100, NULL},
               {MIIM_GBIT_CONTROL, MIIM_GBIT_CONTROL_INIT, NULL},
               {MIIM_ANAR, MIIM_ANAR_INIT, NULL},
               {MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
               {MIIM_CONTROL, MIIM_CONTROL_INIT, &mii_cr_init},
               {miim_end,}
       },
       (struct phy_cmd[]){     /* startup */
               /* Status is read once to clear old link state */
               {MIIM_STATUS, miim_read, NULL},
               /* Auto-negotiate */
               {MIIM_STATUS, miim_read, &mii_parse_sr},
               /* Read the status */
               {MIIM_88E1011_PHY_STATUS, miim_read,
                &mii_parse_88E1011_psr},
               {miim_end,}
       },
       (struct phy_cmd[]){     /* shutdown */
               {miim_end,}
       },
};

/* The 5411 id is 0x206070, the 5421 is 0x2060e0 */
struct phy_info phy_info_BCM5461S = {
	0x02060c1,	/* 5461 ID */
	"Broadcom BCM5461S",
	0, /* not clear to me what minor revisions we can shift away */
	(struct phy_cmd[]) { /* config */
		/* Reset and configure the PHY */
		{MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
		{MIIM_GBIT_CONTROL, MIIM_GBIT_CONTROL_INIT, NULL},
		{MIIM_ANAR, MIIM_ANAR_INIT, NULL},
		{MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
		{MIIM_CONTROL, MIIM_CONTROL_INIT, &mii_cr_init},
		{miim_end,}
	},
	(struct phy_cmd[]) { /* startup */
		/* Status is read once to clear old link state */
		{MIIM_STATUS, miim_read, NULL},
		/* Auto-negotiate */
		{MIIM_STATUS, miim_read, &mii_parse_sr},
		/* Read the status */
		{MIIM_BCM54xx_AUXSTATUS, miim_read, &mii_parse_BCM54xx_sr},
		{miim_end,}
	},
	(struct phy_cmd[]) { /* shutdown */
		{miim_end,}
	},
};

struct phy_info phy_info_BCM5464S = {
	0x02060b1,	/* 5464 ID */
	"Broadcom BCM5464S",
	0, /* not clear to me what minor revisions we can shift away */
	(struct phy_cmd[]) { /* config */
		/* Reset and configure the PHY */
		{MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
		{MIIM_GBIT_CONTROL, MIIM_GBIT_CONTROL_INIT, NULL},
		{MIIM_ANAR, MIIM_ANAR_INIT, NULL},
		{MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
		{MIIM_CONTROL, MIIM_CONTROL_INIT, &mii_cr_init},
		{miim_end,}
	},
	(struct phy_cmd[]) { /* startup */
		/* Status is read once to clear old link state */
		{MIIM_STATUS, miim_read, NULL},
		/* Auto-negotiate */
		{MIIM_STATUS, miim_read, &mii_parse_sr},
		/* Read the status */
		{MIIM_BCM54xx_AUXSTATUS, miim_read, &mii_parse_BCM54xx_sr},
		{miim_end,}
	},
	(struct phy_cmd[]) { /* shutdown */
		{miim_end,}
	},
};

struct phy_info phy_info_M88E1011S = {
	0x01410c6,
	"Marvell 88E1011S",
	4,
	(struct phy_cmd[]){	/* config */
			   /* Reset and configure the PHY */
			   {MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
			   {0x1d, 0x1f, NULL},
			   {0x1e, 0x200c, NULL},
			   {0x1d, 0x5, NULL},
			   {0x1e, 0x0, NULL},
			   {0x1e, 0x100, NULL},
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

struct phy_info phy_info_M88E1111S = {
	0x01410cc,
	"Marvell 88E1111S",
	4,
	(struct phy_cmd[]){	/* config */
			   /* Reset and configure the PHY */
			   {MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
			   {0x14, 0x0cd2, NULL}, /* Delay RGMII TX and RX */
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

static unsigned int m88e1145_setmode(uint mii_reg, struct tsec_private *priv)
{
	uint mii_data = read_phy_reg(priv, mii_reg);

	/* Setting MIIM_88E1145_PHY_EXT_CR */
	if (priv->flags & TSEC_REDUCED)
		return mii_data |
		    MIIM_M88E1145_RGMII_RX_DELAY | MIIM_M88E1145_RGMII_TX_DELAY;
	else
		return mii_data;
}

static struct phy_info phy_info_M88E1145 = {
	0x01410cd,
	"Marvell 88E1145",
	4,
	(struct phy_cmd[]){	/* config */
			   /* Reset the PHY */
			   {MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},

			   /* Errata E0, E1 */
			   {29, 0x001b, NULL},
			   {30, 0x418f, NULL},
			   {29, 0x0016, NULL},
			   {30, 0xa2da, NULL},

			   /* Configure the PHY */
			   {MIIM_GBIT_CONTROL, MIIM_GBIT_CONTROL_INIT, NULL},
			   {MIIM_ANAR, MIIM_ANAR_INIT, NULL},
			   {MIIM_88E1011_PHY_SCR, MIIM_88E1011_PHY_MDI_X_AUTO,
			    NULL},
			   {MIIM_88E1145_PHY_EXT_CR, 0, &m88e1145_setmode},
			   {MIIM_CONTROL, MIIM_CONTROL_RESET, NULL},
			   {MIIM_CONTROL, MIIM_CONTROL_INIT, NULL},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup */
			   /* Status is read once to clear old link state */
			   {MIIM_STATUS, miim_read, NULL},
			   /* Auto-negotiate */
			   {MIIM_STATUS, miim_read, &mii_parse_sr},
			   {MIIM_88E1111_PHY_LED_CONTROL,
			    MIIM_88E1111_PHY_LED_DIRECT, NULL},
			   /* Read the Status */
			   {MIIM_88E1011_PHY_STATUS, miim_read,
			    &mii_parse_88E1011_psr},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown */
			   {miim_end,}
			   },
};

struct phy_info phy_info_cis8204 = {
	0x3f11,
	"Cicada Cis8204",
	6,
	(struct phy_cmd[]){	/* config */
			   /* Override PHY config settings */
			   {MIIM_CIS8201_AUX_CONSTAT,
			    MIIM_CIS8201_AUXCONSTAT_INIT, NULL},
			   /* Configure some basic stuff */
			   {MIIM_CONTROL, MIIM_CONTROL_INIT, &mii_cr_init},
			   {MIIM_CIS8204_SLED_CON, MIIM_CIS8204_SLEDCON_INIT,
			    &mii_cis8204_fixled},
			   {MIIM_CIS8204_EPHY_CON, MIIM_CIS8204_EPHYCON_INIT,
			    &mii_cis8204_setmode},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup */
			   /* Read the Status (2x to make sure link is right) */
			   {MIIM_STATUS, miim_read, NULL},
			   /* Auto-negotiate */
			   {MIIM_STATUS, miim_read, &mii_parse_sr},
			   /* Read the status */
			   {MIIM_CIS8201_AUX_CONSTAT, miim_read,
			    &mii_parse_cis8201},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown */
			   {miim_end,}
			   },
};

/* Cicada 8201 */
struct phy_info phy_info_cis8201 = {
	0xfc41,
	"CIS8201",
	4,
	(struct phy_cmd[]){	/* config */
			   /* Override PHY config settings */
			   {MIIM_CIS8201_AUX_CONSTAT,
			    MIIM_CIS8201_AUXCONSTAT_INIT, NULL},
			   /* Set up the interface mode */
			   {MIIM_CIS8201_EXT_CON1, MIIM_CIS8201_EXTCON1_INIT,
			    NULL},
			   /* Configure some basic stuff */
			   {MIIM_CONTROL, MIIM_CONTROL_INIT, &mii_cr_init},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup */
			   /* Read the Status (2x to make sure link is right) */
			   {MIIM_STATUS, miim_read, NULL},
			   /* Auto-negotiate */
			   {MIIM_STATUS, miim_read, &mii_parse_sr},
			   /* Read the status */
			   {MIIM_CIS8201_AUX_CONSTAT, miim_read,
			    &mii_parse_cis8201},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown */
			   {miim_end,}
			   },
};
struct phy_info phy_info_VSC8244 = {
	0x3f1b,
	"Vitesse VSC8244",
	6,
	(struct phy_cmd[]){	/* config */
			   /* Override PHY config settings */
			   /* Configure some basic stuff */
			   {MIIM_CONTROL, MIIM_CONTROL_INIT, &mii_cr_init},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup */
			   /* Read the Status (2x to make sure link is right) */
			   {MIIM_STATUS, miim_read, NULL},
			   /* Auto-negotiate */
			   {MIIM_STATUS, miim_read, &mii_parse_sr},
			   /* Read the status */
			   {MIIM_VSC8244_AUX_CONSTAT, miim_read,
			    &mii_parse_vsc8244},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown */
			   {miim_end,}
			   },
};

struct phy_info phy_info_dm9161 = {
	0x0181b88,
	"Davicom DM9161E",
	4,
	(struct phy_cmd[]){	/* config */
			   {MIIM_CONTROL, MIIM_DM9161_CR_STOP, NULL},
			   /* Do not bypass the scrambler/descrambler */
			   {MIIM_DM9161_SCR, MIIM_DM9161_SCR_INIT, NULL},
			   /* Clear 10BTCSR to default */
			   {MIIM_DM9161_10BTCSR, MIIM_DM9161_10BTCSR_INIT,
			    NULL},
			   /* Configure some basic stuff */
			   {MIIM_CONTROL, MIIM_CR_INIT, NULL},
			   /* Restart Auto Negotiation */
			   {MIIM_CONTROL, MIIM_DM9161_CR_RSTAN, NULL},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup */
			   /* Status is read once to clear old link state */
			   {MIIM_STATUS, miim_read, NULL},
			   /* Auto-negotiate */
			   {MIIM_STATUS, miim_read, &mii_parse_sr},
			   /* Read the status */
			   {MIIM_DM9161_SCSR, miim_read,
			    &mii_parse_dm9161_scsr},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown */
			   {miim_end,}
			   },
};
/* a generic flavor.  */
struct phy_info phy_info_generic =  {
	0,
	"Unknown/Generic PHY",
	32,
	(struct phy_cmd[]) { /* config */
		{PHY_BMCR, PHY_BMCR_RESET, NULL},
		{PHY_BMCR, PHY_BMCR_AUTON|PHY_BMCR_RST_NEG, NULL},
		{miim_end,}
	},
	(struct phy_cmd[]) { /* startup */
		{PHY_BMSR, miim_read, NULL},
		{PHY_BMSR, miim_read, &mii_parse_sr},
		{PHY_BMSR, miim_read, &mii_parse_link},
		{miim_end,}
	},
	(struct phy_cmd[]) { /* shutdown */
		{miim_end,}
	}
};


uint mii_parse_lxt971_sr2(uint mii_reg, struct tsec_private *priv)
{
	unsigned int speed;
	if (priv->link) {
		speed = mii_reg & MIIM_LXT971_SR2_SPEED_MASK;

		switch (speed) {
		case MIIM_LXT971_SR2_10HDX:
			priv->speed = 10;
			priv->duplexity = 0;
			break;
		case MIIM_LXT971_SR2_10FDX:
			priv->speed = 10;
			priv->duplexity = 1;
			break;
		case MIIM_LXT971_SR2_100HDX:
			priv->speed = 100;
			priv->duplexity = 0;
		default:
			priv->speed = 100;
			priv->duplexity = 1;
			break;
		}
	} else {
		priv->speed = 0;
		priv->duplexity = 0;
	}

	return 0;
}

static struct phy_info phy_info_lxt971 = {
	0x0001378e,
	"LXT971",
	4,
	(struct phy_cmd[]){	/* config */
			   {MIIM_CR, MIIM_CR_INIT, mii_cr_init},	/* autonegotiate */
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup - enable interrupts */
			   /* { 0x12, 0x00f2, NULL }, */
			   {MIIM_STATUS, miim_read, NULL},
			   {MIIM_STATUS, miim_read, &mii_parse_sr},
			   {MIIM_LXT971_SR2, miim_read, &mii_parse_lxt971_sr2},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown - disable interrupts */
			   {miim_end,}
			   },
};

/* Parse the DP83865's link and auto-neg status register for speed and duplex
 * information
 */
uint mii_parse_dp83865_lanr(uint mii_reg, struct tsec_private *priv)
{
	switch (mii_reg & MIIM_DP83865_SPD_MASK) {

	case MIIM_DP83865_SPD_1000:
		priv->speed = 1000;
		break;

	case MIIM_DP83865_SPD_100:
		priv->speed = 100;
		break;

	default:
		priv->speed = 10;
		break;

	}

	if (mii_reg & MIIM_DP83865_DPX_FULL)
		priv->duplexity = 1;
	else
		priv->duplexity = 0;

	return 0;
}

struct phy_info phy_info_dp83865 = {
	0x20005c7,
	"NatSemi DP83865",
	4,
	(struct phy_cmd[]){	/* config */
			   {MIIM_CONTROL, MIIM_DP83865_CR_INIT, NULL},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* startup */
			   /* Status is read once to clear old link state */
			   {MIIM_STATUS, miim_read, NULL},
			   /* Auto-negotiate */
			   {MIIM_STATUS, miim_read, &mii_parse_sr},
			   /* Read the link and auto-neg status */
			   {MIIM_DP83865_LANR, miim_read,
			    &mii_parse_dp83865_lanr},
			   {miim_end,}
			   },
	(struct phy_cmd[]){	/* shutdown */
			   {miim_end,}
			   },
};

struct phy_info *phy_info[] = {
	&phy_info_cis8204,
	&phy_info_cis8201,
	&phy_info_BCM5461S,
	&phy_info_BCM5464S,
	&phy_info_M88E1011S,
	&phy_info_M88E1111S,
	&phy_info_M88E1145,
        &phy_info_M88E1149S,
	&phy_info_dm9161,
	&phy_info_lxt971,
	&phy_info_VSC8244,
	&phy_info_dp83865,
	&phy_info_generic,
	NULL
};

/* Grab the identifier of the device's PHY, and search through
 * all of the known PHYs to see if one matches.	 If so, return
 * it, if not, return NULL
 */
struct phy_info *get_phy_info(struct eth_device *dev)
{
	struct tsec_private *priv = (struct tsec_private *)dev->priv;
	uint phy_reg, phy_ID;
	int i;
	struct phy_info *theInfo = NULL;

	/* Grab the bits from PHYIR1, and put them in the upper half */
	phy_reg = read_phy_reg(priv, MIIM_PHYIR1);
	phy_ID = (phy_reg & 0xffff) << 16;

	/* Grab the bits from PHYIR2, and put them in the lower half */
	phy_reg = read_phy_reg(priv, MIIM_PHYIR2);
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
		printf("%s: PHY id %x is not supported!\n", dev->name, phy_ID);
		return NULL;
	} else {
		debug("%s: PHY is %s (%x)\n", dev->name, theInfo->name, phy_ID);
	}

	return theInfo;
}

/* Execute the given series of commands on the given device's
 * PHY, running functions as necessary
 */
void phy_run_commands(struct tsec_private *priv, struct phy_cmd *cmd)
{
	int i;
	uint result;
	volatile tsec_t *phyregs = priv->phyregs;

	phyregs->miimcfg = MIIMCFG_RESET;

	phyregs->miimcfg = MIIMCFG_INIT_VALUE;

	while (phyregs->miimind & MIIMIND_BUSY) ;

	for (i = 0; cmd->mii_reg != miim_end; i++) {
		if (cmd->mii_data == miim_read) {
			result = read_phy_reg(priv, cmd->mii_reg);

			if (cmd->funct != NULL)
				(*(cmd->funct)) (result, priv);

		} else {
			if (cmd->funct != NULL)
				result = (*(cmd->funct)) (cmd->mii_reg, priv);
			else
				result = cmd->mii_data;

			write_phy_reg(priv, cmd->mii_reg, result);

		}
		cmd++;
	}
}

/* Relocate the function pointers in the phy cmd lists */
static void relocate_cmds(void)
{
	struct phy_cmd **cmdlistptr;
	struct phy_cmd *cmd;
	int i, j, k;

	for (i = 0; phy_info[i]; i++) {
		/* First thing's first: relocate the pointers to the
		 * PHY command structures (the structs were done) */
		phy_info[i] = (struct phy_info *)((uint) phy_info[i]
						  + gd->reloc_off);
		phy_info[i]->name += gd->reloc_off;
		phy_info[i]->config =
		    (struct phy_cmd *)((uint) phy_info[i]->config
				       + gd->reloc_off);
		phy_info[i]->startup =
		    (struct phy_cmd *)((uint) phy_info[i]->startup
				       + gd->reloc_off);
		phy_info[i]->shutdown =
		    (struct phy_cmd *)((uint) phy_info[i]->shutdown
				       + gd->reloc_off);

		cmdlistptr = &phy_info[i]->config;
		j = 0;
		for (; cmdlistptr <= &phy_info[i]->shutdown; cmdlistptr++) {
			k = 0;
			for (cmd = *cmdlistptr;
			     cmd->mii_reg != miim_end;
			     cmd++) {
				/* Only relocate non-NULL pointers */
				if (cmd->funct)
					cmd->funct += gd->reloc_off;

				k++;
			}
			j++;
		}
	}

	relocated = 1;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) \
	&& !defined(BITBANGMII)

struct tsec_private *get_priv_for_phy(unsigned char phyaddr)
{
	int i;

	for (i = 0; i < MAXCONTROLLERS; i++) {
		if (privlist[i]->phyaddr == phyaddr)
			return privlist[i];
	}

	return NULL;
}

/*
 * Read a MII PHY register.
 *
 * Returns:
 *  0 on success
 */
static int tsec_miiphy_read(char *devname, unsigned char addr,
			    unsigned char reg, unsigned short *value)
{
	unsigned short ret;
	struct tsec_private *priv = get_priv_for_phy(addr);

	if (NULL == priv) {
		printf("Can't read PHY at address %d\n", addr);
		return -1;
	}

	ret = (unsigned short)read_phy_reg(priv, reg);
	*value = ret;

	return 0;
}

/*
 * Write a MII PHY register.
 *
 * Returns:
 *  0 on success
 */
static int tsec_miiphy_write(char *devname, unsigned char addr,
			     unsigned char reg, unsigned short value)
{
	struct tsec_private *priv = get_priv_for_phy(addr);

	if (NULL == priv) {
		printf("Can't write PHY at address %d\n", addr);
		return -1;
	}

	write_phy_reg(priv, reg, value);

	return 0;
}

#endif

#endif /* CONFIG_TSEC_ENET */
