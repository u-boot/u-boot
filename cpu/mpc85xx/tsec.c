/*
 * tsec.c
 * Motorola Three Speed Ethernet Controller driver
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * (C) Copyright 2003, Motorola, Inc.
 * maintained by Xianghua Xiao (x.xiao@motorola.com)
 * author Andy Fleming
 *
 */

#include <config.h>
#include <mpc85xx.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>

#if defined(CONFIG_TSEC_ENET)
#include "tsec.h"

#define TX_BUF_CNT 2

#undef TSEC_DEBUG
#ifdef TSEC_DEBUG
#define DBGPRINT(x) printf(x)
#else
#define DBGPRINT(x)
#endif

static uint rxIdx;	/* index of the current RX buffer */
static uint txIdx;	/* index of the current TX buffer */

typedef volatile struct rtxbd {
	txbd8_t txbd[TX_BUF_CNT];
	rxbd8_t rxbd[PKTBUFSRX];
}  RTXBD;

#ifdef __GNUC__
static RTXBD rtx __attribute__ ((aligned(8)));
#else
#error "rtx must be 64-bit aligned"
#endif

static int tsec_send(struct eth_device* dev, volatile void *packet, int length);
static int tsec_recv(struct eth_device* dev);
static int tsec_init(struct eth_device* dev, bd_t * bd);
static void tsec_halt(struct eth_device* dev);
static void init_registers(tsec_t *regs);
static void startup_tsec(tsec_t *regs);
static void init_phy(tsec_t *regs);
uint read_phy_reg(tsec_t *regbase, uint phyid, uint offset);

static int	phy_id = -1;

/* Initialize device structure.  returns 0 on failure, 1 on
 * success */
int tsec_initialize(bd_t *bis)
{
	struct eth_device* dev;
	int i;
	tsec_t *regs = (tsec_t *)(TSEC_BASE_ADDR);

	dev = (struct eth_device*) malloc(sizeof *dev);

	if(dev == NULL)
		return 0;

	memset(dev, 0, sizeof *dev);

	sprintf(dev->name, "MOTO ETHERNET");
	dev->iobase = 0;
	dev->priv   = 0;
	dev->init   = tsec_init;
	dev->halt   = tsec_halt;
	dev->send   = tsec_send;
	dev->recv   = tsec_recv;

	/* Tell u-boot to get the addr from the env */
	for(i=0;i<6;i++)
		dev->enetaddr[i] = 0;

	eth_register(dev);

	/* Reconfigure the PHY to advertise everything here
	 * so that it works with both gigabit and 10/100 */
#ifdef CONFIG_PHY_M88E1011
	/* Assign a Physical address to the TBI */
	regs->tbipa=TBIPA_VALUE;

	/* reset the management interface */
	regs->miimcfg=MIIMCFG_RESET;

	regs->miimcfg=MIIMCFG_INIT_VALUE;

	/* Wait until the bus is free */
	while(regs->miimind & MIIMIND_BUSY);

	/* Locate PHYs.  Skip TBIPA, which we know is 31.
	*/
	for (i=0; i<31; i++) {
		if (read_phy_reg(regs, i, 2) == 0x141) {
			if (phy_id == -1)
				phy_id = i;
#ifdef TSEC_DEBUG
			printf("Found Marvell PHY at 0x%02x\n", i);
#endif
		}
	}
#ifdef TSEC_DEBUG
	printf("Using PHY ID 0x%02x\n", phy_id);
#endif
	write_phy_reg(regs, phy_id, MIIM_CONTROL, MIIM_CONTROL_RESET);

	RESET_ERRATA(regs, phy_id);

	/* Configure the PHY to advertise gbit and 10/100 */
	write_phy_reg(regs, phy_id, MIIM_GBIT_CONTROL, MIIM_GBIT_CONTROL_INIT);
	write_phy_reg(regs, phy_id, MIIM_ANAR, MIIM_ANAR_INIT);

	/* Reset the PHY so the new settings take effect */
	write_phy_reg(regs, phy_id, MIIM_CONTROL, MIIM_CONTROL_RESET);
#endif
	return 1;
}


/* Initializes data structures and registers for the controller,
 * and brings the interface up */
int tsec_init(struct eth_device* dev, bd_t * bd)
{
	tsec_t *regs;
	uint tempval;
	char tmpbuf[MAC_ADDR_LEN];
	int i;

	regs = (tsec_t *)(TSEC_BASE_ADDR);

	/* Make sure the controller is stopped */
	tsec_halt(dev);

	/* Reset the MAC */
	regs->maccfg1 |= MACCFG1_SOFT_RESET;

	/* Clear MACCFG1[Soft_Reset] */
	regs->maccfg1 &= ~(MACCFG1_SOFT_RESET);

	/* Init MACCFG2.  Defaults to GMII/MII */
	regs->maccfg2 = MACCFG2_INIT_SETTINGS;

	/* Init ECNTRL */
	regs->ecntrl = ECNTRL_INIT_SETTINGS;

	/* Copy the station address into the address registers.
	 * Backwards, because little endian MACS are dumb */
	for(i=0;i<MAC_ADDR_LEN;i++) {
		tmpbuf[MAC_ADDR_LEN - 1 - i] = bd->bi_enetaddr[i];
	}
	(uint)(regs->macstnaddr1) = *((uint *)(tmpbuf));

	tempval = *((uint *)(tmpbuf +4));

	(uint)(regs->macstnaddr2) = tempval;

	/* Initialize the PHY */
	init_phy(regs);

	/* reset the indices to zero */
	rxIdx = 0;
	txIdx = 0;

	/* Clear out (for the most part) the other registers */
	init_registers(regs);

	/* Ready the device for tx/rx */
	startup_tsec(regs);

	return 1;

}


/* Reads from the register at offset in the PHY at phyid, */
/* using the register set defined in regbase.  It waits until the */
/* bits in the miimstat are valid (miimind notvalid bit cleared), */
/* and then passes those bits on to the variable specified in */
/* value */
/* Before it does the read, it needs to clear the command field */
uint read_phy_reg(tsec_t *regbase, uint phyid, uint offset)
{
	uint value;

	/* Put the address of the phy, and the register number into
	 * MIIMADD
	 */
	regbase->miimadd = (phyid << 8) | offset;

	/* Clear the command register, and wait */
	regbase->miimcom = 0;
	asm("msync");

	/* Initiate a read command, and wait */
	regbase->miimcom = MIIM_READ_COMMAND;
	asm("msync");

	/* Wait for the the indication that the read is done */
	while((regbase->miimind & (MIIMIND_NOTVALID | MIIMIND_BUSY)));

	/* Grab the value read from the PHY */
	value = regbase->miimstat;

	return value;
}

/* Setup the PHY */
static void init_phy(tsec_t *regs)
{
	uint testval;
	unsigned int timeout = TSEC_TIMEOUT;

	/* Assign a Physical address to the TBI */
	regs->tbipa=TBIPA_VALUE;

	/* reset the management interface */
	regs->miimcfg=MIIMCFG_RESET;

	regs->miimcfg=MIIMCFG_INIT_VALUE;

	/* Wait until the bus is free */
	while(regs->miimind & MIIMIND_BUSY);

#ifdef CONFIG_PHY_CIS8201
	/* override PHY config settings */
	write_phy_reg(regs, 0, MIIM_AUX_CONSTAT, MIIM_AUXCONSTAT_INIT);

	/* Set up interface mode */
	write_phy_reg(regs, 0, MIIM_EXT_CON1, MIIM_EXTCON1_INIT);
#endif

	/* Set the PHY to gigabit, full duplex, Auto-negotiate */
	write_phy_reg(regs, phy_id, MIIM_CONTROL, MIIM_CONTROL_INIT);

	/* Wait until STATUS indicates Auto-Negotiation is done */
	DBGPRINT("Waiting for Auto-negotiation to complete\n");
	testval=read_phy_reg(regs, phy_id, MIIM_STATUS);

	while((!(testval & MIIM_STATUS_AN_DONE))&& timeout--) {
		testval=read_phy_reg(regs, phy_id, MIIM_STATUS);
	}

	if(testval & MIIM_STATUS_AN_DONE)
		DBGPRINT("Auto-negotiation done\n");
	else
		DBGPRINT("Auto-negotiation timed-out.\n");

#ifdef CONFIG_PHY_CIS8201
	/* Find out what duplexity (duplicity?) we have */
	/* Read it twice to make sure */
	testval=read_phy_reg(regs, phy_id, MIIM_AUX_CONSTAT);

	if(testval & MIIM_AUXCONSTAT_DUPLEX) {
		DBGPRINT("Enet starting in full duplex\n");
		regs->maccfg2 |= MACCFG2_FULL_DUPLEX;
	} else {
		DBGPRINT("Enet starting in half duplex\n");
		regs->maccfg2 &= ~MACCFG2_FULL_DUPLEX;
	}

	/* Also, we look to see what speed we are at
	 * if Gigabit, MACCFG2 goes in GMII, otherwise,
	 * MII mode.
	 */
	if((testval & MIIM_AUXCONSTAT_SPEED) != MIIM_AUXCONSTAT_GBIT) {
		if((testval & MIIM_AUXCONSTAT_SPEED) == MIIM_AUXCONSTAT_100)
			DBGPRINT("Enet starting in 100BT\n");
		else
			DBGPRINT("Enet starting in 10BT\n");

		/* mark the mode in MACCFG2 */
		regs->maccfg2 = ((regs->maccfg2&~(MACCFG2_IF)) | MACCFG2_MII);
	} else {
		DBGPRINT("Enet starting in 1000BT\n");
	}

#endif

#ifdef CONFIG_PHY_M88E1011
	/* Read the PHY to see what speed and duplex we are */
	testval=read_phy_reg(regs, phy_id, MIIM_PHY_STATUS);

	timeout = TSEC_TIMEOUT;
	while((!(testval & MIIM_PHYSTAT_SPDDONE)) && timeout--) {
		testval = read_phy_reg(regs,phy_id,MIIM_PHY_STATUS);
	}

	if(!(testval & MIIM_PHYSTAT_SPDDONE))
		DBGPRINT("Enet: Speed not resolved\n");

	testval=read_phy_reg(regs, phy_id, MIIM_PHY_STATUS);
	if(testval & MIIM_PHYSTAT_DUPLEX) {
		DBGPRINT("Enet starting in Full Duplex\n");
		regs->maccfg2 |= MACCFG2_FULL_DUPLEX;
	} else {
		DBGPRINT("Enet starting in Half Duplex\n");
		regs->maccfg2 &= ~MACCFG2_FULL_DUPLEX;
	}

	if(!((testval&MIIM_PHYSTAT_SPEED) == MIIM_PHYSTAT_GBIT)) {
		if((testval & MIIM_PHYSTAT_SPEED) == MIIM_PHYSTAT_100)
			DBGPRINT("Enet starting in 100BT\n");
		else
			DBGPRINT("Enet starting in 10BT\n");

		regs->maccfg2 = ((regs->maccfg2&~(MACCFG2_IF)) | MACCFG2_MII);
	} else {
		DBGPRINT("Enet starting in 1000BT\n");
	}
#endif

}


static void init_registers(tsec_t *regs)
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

static void startup_tsec(tsec_t *regs)
{
	int i;

	/* Point to the buffer descriptors */
	regs->tbase = (unsigned int)(&rtx.txbd[txIdx]);
	regs->rbase = (unsigned int)(&rtx.rxbd[rxIdx]);

	/* Initialize the Rx Buffer descriptors */
	for (i = 0; i < PKTBUFSRX; i++) {
		rtx.rxbd[i].status = RXBD_EMPTY;
		rtx.rxbd[i].length = 0;
		rtx.rxbd[i].bufPtr = (uint)NetRxPackets[i];
	}
	rtx.rxbd[PKTBUFSRX -1].status |= RXBD_WRAP;

	/* Initialize the TX Buffer Descriptors */
	for(i=0; i<TX_BUF_CNT; i++) {
		rtx.txbd[i].status = 0;
		rtx.txbd[i].length = 0;
		rtx.txbd[i].bufPtr = 0;
	}
	rtx.txbd[TX_BUF_CNT -1].status |= TXBD_WRAP;

	/* Enable Transmit and Receive */
	regs->maccfg1 |= (MACCFG1_RX_EN | MACCFG1_TX_EN);

	/* Tell the DMA it is clear to go */
	regs->dmactrl |= DMACTRL_INIT_SETTINGS;
	regs->tstat = TSTAT_CLEAR_THALT;
	regs->dmactrl &= ~(DMACTRL_GRS | DMACTRL_GTS);
}

/* This returns the status bits of the device.  The return value
 * is never checked, and this is what the 8260 driver did, so we
 * do the same.  Presumably, this would be zero if there were no
 * errors */
static int tsec_send(struct eth_device* dev, volatile void *packet, int length)
{
	int i;
	int result = 0;
	tsec_t * regs = (tsec_t *)(TSEC_BASE_ADDR);

	/* Find an empty buffer descriptor */
	for(i=0; rtx.txbd[txIdx].status & TXBD_READY; i++) {
		if (i >= TOUT_LOOP) {
			DBGPRINT("tsec: tx buffers full\n");
			return result;
		}
	}

	rtx.txbd[txIdx].bufPtr = (uint)packet;
	rtx.txbd[txIdx].length = length;
	rtx.txbd[txIdx].status |= (TXBD_READY | TXBD_LAST | TXBD_CRC | TXBD_INTERRUPT);

	/* Tell the DMA to go */
	regs->tstat = TSTAT_CLEAR_THALT;

	/* Wait for buffer to be transmitted */
	for(i=0; rtx.txbd[txIdx].status & TXBD_READY; i++) {
		if (i >= TOUT_LOOP) {
			DBGPRINT("tsec: tx error\n");
			return result;
		}
	}

	txIdx = (txIdx + 1) % TX_BUF_CNT;
	result = rtx.txbd[txIdx].status & TXBD_STATS;

	return result;
}

static int tsec_recv(struct eth_device* dev)
{
	int length;
	tsec_t *regs = (tsec_t *)(TSEC_BASE_ADDR);

	while(!(rtx.rxbd[rxIdx].status & RXBD_EMPTY)) {

		length = rtx.rxbd[rxIdx].length;

		/* Send the packet up if there were no errors */
		if (!(rtx.rxbd[rxIdx].status & RXBD_STATS)) {
			NetReceive(NetRxPackets[rxIdx], length - 4);
		}

		rtx.rxbd[rxIdx].length = 0;

		/* Set the wrap bit if this is the last element in the list */
		rtx.rxbd[rxIdx].status = RXBD_EMPTY | (((rxIdx + 1) == PKTBUFSRX) ? RXBD_WRAP : 0);

		rxIdx = (rxIdx + 1) % PKTBUFSRX;
	}

	if(regs->ievent&IEVENT_BSY) {
		regs->ievent = IEVENT_BSY;
		regs->rstat = RSTAT_CLEAR_RHALT;
	}

	return -1;

}


static void tsec_halt(struct eth_device* dev)
{
	tsec_t *regs = (tsec_t *)(TSEC_BASE_ADDR);

	regs->dmactrl &= ~(DMACTRL_GRS | DMACTRL_GTS);
	regs->dmactrl |= (DMACTRL_GRS | DMACTRL_GTS);

	while(!(regs->ievent & (IEVENT_GRSC | IEVENT_GTSC)));

	regs->maccfg1 &= ~(MACCFG1_TX_EN | MACCFG1_RX_EN);

}

#ifndef CONFIG_BITBANGMII
/*
 * Read a MII PHY register.
 *
 * Returns:
 *   0 on success
 */
int miiphy_read(unsigned char  addr,
		unsigned char  reg,
		unsigned short *value)
{
	tsec_t *regs;
	unsigned short rv;

	regs = (tsec_t *)(TSEC_BASE_ADDR);
	rv = (unsigned short)read_phy_reg(regs, addr, reg);
	*value = rv;

	return 0;
}

/*
 * Write a MII PHY register.
 *
 * Returns:
 *   0 on success
 */
int miiphy_write(unsigned char  addr,
		 unsigned char  reg,
		 unsigned short value)
{
	tsec_t *regs;

	regs = (tsec_t *)(TSEC_BASE_ADDR);
	write_phy_reg(regs, addr, reg, value);

	return 0;
}
#endif /* CONFIG_BITBANGMII */
#endif /* CONFIG_TSEC_ENET */
