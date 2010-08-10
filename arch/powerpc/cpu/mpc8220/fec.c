/*
 * (C) Copyright 2003-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This file is based on mpc4200fec.c,
 * (C) Copyright Motorola, Inc., 2000
 */

#include <common.h>
#include <mpc8220.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include "dma.h"
#include "fec.h"

#undef  DEBUG
#if defined(CONFIG_CMD_NET) && defined(CONFIG_NET_MULTI) && \
    defined(CONFIG_MPC8220_FEC)

#if !(defined(CONFIG_MII) || defined(CONFIG_CMD_MII))
#error "CONFIG_MII has to be defined!"
#endif

#ifdef DEBUG
static void tfifo_print (char *devname, mpc8220_fec_priv * fec);
static void rfifo_print (char *devname, mpc8220_fec_priv * fec);
#endif /* DEBUG */

typedef struct {
	u8 data[1500];		/* actual data */
	int length;		/* actual length */
	int used;		/* buffer in use or not */
	u8 head[16];		/* MAC header(6 + 6 + 2) + 2(aligned) */
} NBUF;

int fec8220_miiphy_read (const char *devname, u8 phyAddr, u8 regAddr, u16 *retVal);
int fec8220_miiphy_write (const char *devname, u8 phyAddr, u8 regAddr, u16 data);

/********************************************************************/
#ifdef DEBUG
static void mpc8220_fec_phydump (char *devname)
{
	u16 phyStatus, i;
	u8 phyAddr = CONFIG_PHY_ADDR;
	u8 reg_mask[] = {
#if CONFIG_PHY_TYPE == 0x79c874 /* AMD Am79C874 */
		/* regs to print: 0...7, 16...19, 21, 23, 24 */
		1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
#else
		/* regs to print: 0...8, 16...20 */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
	};

	for (i = 0; i < 32; i++) {
		if (reg_mask[i]) {
			miiphy_read (devname, phyAddr, i, &phyStatus);
			printf ("Mii reg %d: 0x%04x\n", i, phyStatus);
		}
	}
}
#endif

/********************************************************************/
static int mpc8220_fec_rbd_init (mpc8220_fec_priv * fec)
{
	int ix;
	char *data;
	static int once = 0;

	for (ix = 0; ix < FEC_RBD_NUM; ix++) {
		if (!once) {
			data = (char *) malloc (FEC_MAX_PKT_SIZE);
			if (data == NULL) {
				printf ("RBD INIT FAILED\n");
				return -1;
			}
			fec->rbdBase[ix].dataPointer = (u32) data;
		}
		fec->rbdBase[ix].status = FEC_RBD_EMPTY;
		fec->rbdBase[ix].dataLength = 0;
	}
	once++;

	/*
	 * have the last RBD to close the ring
	 */
	fec->rbdBase[ix - 1].status |= FEC_RBD_WRAP;
	fec->rbdIndex = 0;

	return 0;
}

/********************************************************************/
static void mpc8220_fec_tbd_init (mpc8220_fec_priv * fec)
{
	int ix;

	for (ix = 0; ix < FEC_TBD_NUM; ix++) {
		fec->tbdBase[ix].status = 0;
	}

	/*
	 * Have the last TBD to close the ring
	 */
	fec->tbdBase[ix - 1].status |= FEC_TBD_WRAP;

	/*
	 * Initialize some indices
	 */
	fec->tbdIndex = 0;
	fec->usedTbdIndex = 0;
	fec->cleanTbdNum = FEC_TBD_NUM;
}

/********************************************************************/
static void mpc8220_fec_rbd_clean (mpc8220_fec_priv * fec, FEC_RBD * pRbd)
{
	/*
	 * Reset buffer descriptor as empty
	 */
	if ((fec->rbdIndex) == (FEC_RBD_NUM - 1))
		pRbd->status = (FEC_RBD_WRAP | FEC_RBD_EMPTY);
	else
		pRbd->status = FEC_RBD_EMPTY;

	pRbd->dataLength = 0;

	/*
	 * Now, we have an empty RxBD, restart the SmartDMA receive task
	 */
	DMA_TASK_ENABLE (FEC_RECV_TASK_NO);

	/*
	 * Increment BD count
	 */
	fec->rbdIndex = (fec->rbdIndex + 1) % FEC_RBD_NUM;
}

/********************************************************************/
static void mpc8220_fec_tbd_scrub (mpc8220_fec_priv * fec)
{
	FEC_TBD *pUsedTbd;

#ifdef DEBUG
	printf ("tbd_scrub: fec->cleanTbdNum = %d, fec->usedTbdIndex = %d\n",
		fec->cleanTbdNum, fec->usedTbdIndex);
#endif

	/*
	 * process all the consumed TBDs
	 */
	while (fec->cleanTbdNum < FEC_TBD_NUM) {
		pUsedTbd = &fec->tbdBase[fec->usedTbdIndex];
		if (pUsedTbd->status & FEC_TBD_READY) {
#ifdef DEBUG
			printf ("Cannot clean TBD %d, in use\n",
				fec->cleanTbdNum);
#endif
			return;
		}

		/*
		 * clean this buffer descriptor
		 */
		if (fec->usedTbdIndex == (FEC_TBD_NUM - 1))
			pUsedTbd->status = FEC_TBD_WRAP;
		else
			pUsedTbd->status = 0;

		/*
		 * update some indeces for a correct handling of the TBD ring
		 */
		fec->cleanTbdNum++;
		fec->usedTbdIndex = (fec->usedTbdIndex + 1) % FEC_TBD_NUM;
	}
}

/********************************************************************/
static void mpc8220_fec_set_hwaddr (mpc8220_fec_priv * fec, char *mac)
{
	u8 currByte;		/* byte for which to compute the CRC */
	int byte;		/* loop - counter */
	int bit;		/* loop - counter */
	u32 crc = 0xffffffff;	/* initial value */

	/*
	 * The algorithm used is the following:
	 * we loop on each of the six bytes of the provided address,
	 * and we compute the CRC by left-shifting the previous
	 * value by one position, so that each bit in the current
	 * byte of the address may contribute the calculation. If
	 * the latter and the MSB in the CRC are different, then
	 * the CRC value so computed is also ex-ored with the
	 * "polynomium generator". The current byte of the address
	 * is also shifted right by one bit at each iteration.
	 * This is because the CRC generatore in hardware is implemented
	 * as a shift-register with as many ex-ores as the radixes
	 * in the polynomium. This suggests that we represent the
	 * polynomiumm itself as a 32-bit constant.
	 */
	for (byte = 0; byte < 6; byte++) {
		currByte = mac[byte];
		for (bit = 0; bit < 8; bit++) {
			if ((currByte & 0x01) ^ (crc & 0x01)) {
				crc >>= 1;
				crc = crc ^ 0xedb88320;
			} else {
				crc >>= 1;
			}
			currByte >>= 1;
		}
	}

	crc = crc >> 26;

	/*
	 * Set individual hash table register
	 */
	if (crc >= 32) {
		fec->eth->iaddr1 = (1 << (crc - 32));
		fec->eth->iaddr2 = 0;
	} else {
		fec->eth->iaddr1 = 0;
		fec->eth->iaddr2 = (1 << crc);
	}

	/*
	 * Set physical address
	 */
	fec->eth->paddr1 =
		(mac[0] << 24) + (mac[1] << 16) + (mac[2] << 8) + mac[3];
	fec->eth->paddr2 = (mac[4] << 24) + (mac[5] << 16) + 0x8808;
}

/********************************************************************/
static int mpc8220_fec_init (struct eth_device *dev, bd_t * bis)
{
	mpc8220_fec_priv *fec = (mpc8220_fec_priv *) dev->priv;
	struct mpc8220_dma *dma = (struct mpc8220_dma *) MMAP_DMA;
	const u8 phyAddr = CONFIG_PHY_ADDR;	/* Only one PHY */

#ifdef DEBUG
	printf ("mpc8220_fec_init... Begin\n");
#endif

	/*
	 * Initialize RxBD/TxBD rings
	 */
	mpc8220_fec_rbd_init (fec);
	mpc8220_fec_tbd_init (fec);

	/*
	 * Set up Pin Muxing for FEC 1
	 */
	*(vu_long *) MMAP_PCFG = 0;
	*(vu_long *) (MMAP_PCFG + 4) = 0;
	/*
	 * Clear FEC-Lite interrupt event register(IEVENT)
	 */
	fec->eth->ievent = 0xffffffff;

	/*
	 * Set interrupt mask register
	 */
	fec->eth->imask = 0x00000000;

	/*
	 * Set FEC-Lite receive control register(R_CNTRL):
	 */
	if (fec->xcv_type == SEVENWIRE) {
		/*
		 * Frame length=1518; 7-wire mode
		 */
		fec->eth->r_cntrl = 0x05ee0020; /*0x05ee0000;FIXME */
	} else {
		/*
		 * Frame length=1518; MII mode;
		 */
		fec->eth->r_cntrl = 0x05ee0024; /*0x05ee0004;FIXME */
	}

	fec->eth->x_cntrl = 0x00000000; /* half-duplex, heartbeat disabled */
	if (fec->xcv_type != SEVENWIRE) {
		/*
		 * Set MII_SPEED = (1/(mii_speed * 2)) * System Clock
		 * and do not drop the Preamble.
		 */
		/* tbd - rtm */
		/*fec->eth->mii_speed = (((gd->ipb_clk >> 20) / 5) << 1); */
		/* No MII for 7-wire mode */
		fec->eth->mii_speed = 0x00000030;
	}

	/*
	 * Set Opcode/Pause Duration Register
	 */
	fec->eth->op_pause = 0x00010020;	/*FIXME0xffff0020; */

	/*
	 * Set Rx FIFO alarm and granularity value
	 */
	fec->eth->rfifo_cntrl = 0x0c000000;
	fec->eth->rfifo_alarm = 0x0000030c;
#ifdef DEBUG
	if (fec->eth->rfifo_status & 0x00700000) {
		printf ("mpc8220_fec_init() RFIFO error\n");
	}
#endif

	/*
	 * Set Tx FIFO granularity value
	 */
	/*fec->eth->tfifo_cntrl = 0x0c000000; */ /*tbd - rtm */
	fec->eth->tfifo_cntrl = 0x0e000000;
#ifdef DEBUG
	printf ("tfifo_status: 0x%08x\n", fec->eth->tfifo_status);
	printf ("tfifo_alarm: 0x%08x\n", fec->eth->tfifo_alarm);
#endif

	/*
	 * Set transmit fifo watermark register(X_WMRK), default = 64
	 */
	fec->eth->tfifo_alarm = 0x00000080;
	fec->eth->x_wmrk = 0x2;

	/*
	 * Set individual address filter for unicast address
	 * and set physical address registers.
	 */
	mpc8220_fec_set_hwaddr (fec, (char *)(dev->enetaddr));

	/*
	 * Set multicast address filter
	 */
	fec->eth->gaddr1 = 0x00000000;
	fec->eth->gaddr2 = 0x00000000;

	/*
	 * Turn ON cheater FSM: ????
	 */
	fec->eth->xmit_fsm = 0x03000000;

#if 1
/*#if defined(CONFIG_MPC5200)*/
	/*
	 * Turn off COMM bus prefetch in the MGT5200 BestComm. It doesn't
	 * work w/ the current receive task.
	 */
	dma->PtdCntrl |= 0x00000001;
#endif

	/*
	 * Set priority of different initiators
	 */
	dma->IPR0 = 7;		/* always */
	dma->IPR3 = 6;		/* Eth RX */
	dma->IPR4 = 5;		/* Eth Tx */

	/*
	 * Clear SmartDMA task interrupt pending bits
	 */
	DMA_CLEAR_IEVENT (FEC_RECV_TASK_NO);

	/*
	 * Initialize SmartDMA parameters stored in SRAM
	 */
	*(int *) FEC_TBD_BASE = (int) fec->tbdBase;
	*(int *) FEC_RBD_BASE = (int) fec->rbdBase;
	*(int *) FEC_TBD_NEXT = (int) fec->tbdBase;
	*(int *) FEC_RBD_NEXT = (int) fec->rbdBase;

	if (fec->xcv_type != SEVENWIRE) {
		/*
		 * Initialize PHY(LXT971A):
		 *
		 *   Generally, on power up, the LXT971A reads its configuration
		 *   pins to check for forced operation, If not cofigured for
		 *   forced operation, it uses auto-negotiation/parallel detection
		 *   to automatically determine line operating conditions.
		 *   If the PHY device on the other side of the link supports
		 *   auto-negotiation, the LXT971A auto-negotiates with it
		 *   using Fast Link Pulse(FLP) Bursts. If the PHY partner does not
		 *   support auto-negotiation, the LXT971A automatically detects
		 *   the presence of either link pulses(10Mbps PHY) or Idle
		 *   symbols(100Mbps) and sets its operating conditions accordingly.
		 *
		 *   When auto-negotiation is controlled by software, the following
		 *   steps are recommended.
		 *
		 * Note:
		 *   The physical address is dependent on hardware configuration.
		 *
		 */
		int timeout = 1;
		u16 phyStatus;

		/*
		 * Reset PHY, then delay 300ns
		 */
		miiphy_write (dev->name, phyAddr, 0x0, 0x8000);
		udelay (1000);

		if (fec->xcv_type == MII10) {
			/*
			 * Force 10Base-T, FDX operation
			 */
#ifdef DEBUG
			printf ("Forcing 10 Mbps ethernet link... ");
#endif
			miiphy_read (dev->name, phyAddr, 0x1, &phyStatus);
			/*
			   miiphy_write(fec, phyAddr, 0x0, 0x0100);
			 */
			miiphy_write (dev->name, phyAddr, 0x0, 0x0180);

			timeout = 20;
			do {	/* wait for link status to go down */
				udelay (10000);
				if ((timeout--) == 0) {
#ifdef DEBUG
					printf ("hmmm, should not have waited...");
#endif
					break;
				}
				miiphy_read (dev->name, phyAddr, 0x1, &phyStatus);
#ifdef DEBUG
				printf ("=");
#endif
			} while ((phyStatus & 0x0004)); /* !link up */

			timeout = 1000;
			do {	/* wait for link status to come back up */
				udelay (10000);
				if ((timeout--) == 0) {
					printf ("failed. Link is down.\n");
					break;
				}
				miiphy_read (dev->name, phyAddr, 0x1, &phyStatus);
#ifdef DEBUG
				printf ("+");
#endif
			} while (!(phyStatus & 0x0004));	/* !link up */

#ifdef DEBUG
			printf ("done.\n");
#endif
		} else {	/* MII100 */
			/*
			 * Set the auto-negotiation advertisement register bits
			 */
			miiphy_write (dev->name, phyAddr, 0x4, 0x01e1);

			/*
			 * Set MDIO bit 0.12 = 1(&& bit 0.9=1?) to enable auto-negotiation
			 */
			miiphy_write (dev->name, phyAddr, 0x0, 0x1200);

			/*
			 * Wait for AN completion
			 */
			timeout = 5000;
			do {
				udelay (1000);

				if ((timeout--) == 0) {
#ifdef DEBUG
					printf ("PHY auto neg 0 failed...\n");
#endif
					return -1;
				}

				if (miiphy_read (dev->name, phyAddr, 0x1, &phyStatus) !=
				    0) {
#ifdef DEBUG
					printf ("PHY auto neg 1 failed 0x%04x...\n", phyStatus);
#endif
					return -1;
				}
			} while (!(phyStatus & 0x0004));

#ifdef DEBUG
			printf ("PHY auto neg complete! \n");
#endif
		}

	}

	/*
	 * Enable FEC-Lite controller
	 */
	fec->eth->ecntrl |= 0x00000006;

#ifdef DEBUG
	if (fec->xcv_type != SEVENWIRE)
		mpc8220_fec_phydump (dev->name);
#endif

	/*
	 * Enable SmartDMA receive task
	 */
	DMA_TASK_ENABLE (FEC_RECV_TASK_NO);

#ifdef DEBUG
	printf ("mpc8220_fec_init... Done \n");
#endif

	return 1;
}

/********************************************************************/
static void mpc8220_fec_halt (struct eth_device *dev)
{
	mpc8220_fec_priv *fec = (mpc8220_fec_priv *) dev->priv;
	int counter = 0xffff;

#ifdef DEBUG
	if (fec->xcv_type != SEVENWIRE)
		mpc8220_fec_phydump (dev->name);
#endif

	/*
	 * mask FEC chip interrupts
	 */
	fec->eth->imask = 0;

	/*
	 * issue graceful stop command to the FEC transmitter if necessary
	 */
	fec->eth->x_cntrl |= 0x00000001;

	/*
	 * wait for graceful stop to register
	 */
	while ((counter--) && (!(fec->eth->ievent & 0x10000000)));

	/*
	 * Disable SmartDMA tasks
	 */
	DMA_TASK_DISABLE (FEC_XMIT_TASK_NO);
	DMA_TASK_DISABLE (FEC_RECV_TASK_NO);

	/*
	 * Disable the Ethernet Controller
	 */
	fec->eth->ecntrl &= 0xfffffffd;

	/*
	 * Clear FIFO status registers
	 */
	fec->eth->rfifo_status &= 0x00700000;
	fec->eth->tfifo_status &= 0x00700000;

	fec->eth->reset_cntrl = 0x01000000;

	/*
	 * Issue a reset command to the FEC chip
	 */
	fec->eth->ecntrl |= 0x1;

	/*
	 * wait at least 16 clock cycles
	 */
	udelay (10);

#ifdef DEBUG
	printf ("Ethernet task stopped\n");
#endif
}

#ifdef DEBUG
/********************************************************************/

static void tfifo_print (char *devname, mpc8220_fec_priv * fec)
{
	u16 phyAddr = CONFIG_PHY_ADDR;
	u16 phyStatus;

	if ((fec->eth->tfifo_lrf_ptr != fec->eth->tfifo_lwf_ptr)
	    || (fec->eth->tfifo_rdptr != fec->eth->tfifo_wrptr)) {

		miiphy_read (devname, phyAddr, 0x1, &phyStatus);
		printf ("\nphyStatus: 0x%04x\n", phyStatus);
		printf ("ecntrl:   0x%08x\n", fec->eth->ecntrl);
		printf ("ievent:   0x%08x\n", fec->eth->ievent);
		printf ("x_status: 0x%08x\n", fec->eth->x_status);
		printf ("tfifo: status	0x%08x\n", fec->eth->tfifo_status);

		printf ("	control 0x%08x\n", fec->eth->tfifo_cntrl);
		printf ("	lrfp	0x%08x\n", fec->eth->tfifo_lrf_ptr);
		printf ("	lwfp	0x%08x\n", fec->eth->tfifo_lwf_ptr);
		printf ("	alarm	0x%08x\n", fec->eth->tfifo_alarm);
		printf ("	readptr 0x%08x\n", fec->eth->tfifo_rdptr);
		printf ("	writptr 0x%08x\n", fec->eth->tfifo_wrptr);
	}
}

static void rfifo_print (char *devname, mpc8220_fec_priv * fec)
{
	u16 phyAddr = CONFIG_PHY_ADDR;
	u16 phyStatus;

	if ((fec->eth->rfifo_lrf_ptr != fec->eth->rfifo_lwf_ptr)
	    || (fec->eth->rfifo_rdptr != fec->eth->rfifo_wrptr)) {

		miiphy_read (devname, phyAddr, 0x1, &phyStatus);
		printf ("\nphyStatus: 0x%04x\n", phyStatus);
		printf ("ecntrl:   0x%08x\n", fec->eth->ecntrl);
		printf ("ievent:   0x%08x\n", fec->eth->ievent);
		printf ("x_status: 0x%08x\n", fec->eth->x_status);
		printf ("rfifo: status	0x%08x\n", fec->eth->rfifo_status);

		printf ("	control 0x%08x\n", fec->eth->rfifo_cntrl);
		printf ("	lrfp	0x%08x\n", fec->eth->rfifo_lrf_ptr);
		printf ("	lwfp	0x%08x\n", fec->eth->rfifo_lwf_ptr);
		printf ("	alarm	0x%08x\n", fec->eth->rfifo_alarm);
		printf ("	readptr 0x%08x\n", fec->eth->rfifo_rdptr);
		printf ("	writptr 0x%08x\n", fec->eth->rfifo_wrptr);
	}
}
#endif /* DEBUG */

/********************************************************************/

static int mpc8220_fec_send (struct eth_device *dev, volatile void *eth_data,
			     int data_length)
{
	/*
	 * This routine transmits one frame.  This routine only accepts
	 * 6-byte Ethernet addresses.
	 */
	mpc8220_fec_priv *fec = (mpc8220_fec_priv *) dev->priv;
	FEC_TBD *pTbd;

#ifdef DEBUG
	printf ("tbd status: 0x%04x\n", fec->tbdBase[0].status);
	tfifo_print (dev->name, fec);
#endif

	/*
	 * Clear Tx BD ring at first
	 */
	mpc8220_fec_tbd_scrub (fec);

	/*
	 * Check for valid length of data.
	 */
	if ((data_length > 1500) || (data_length <= 0)) {
		return -1;
	}

	/*
	 * Check the number of vacant TxBDs.
	 */
	if (fec->cleanTbdNum < 1) {
#ifdef DEBUG
		printf ("No available TxBDs ...\n");
#endif
		return -1;
	}

	/*
	 * Get the first TxBD to send the mac header
	 */
	pTbd = &fec->tbdBase[fec->tbdIndex];
	pTbd->dataLength = data_length;
	pTbd->dataPointer = (u32) eth_data;
	pTbd->status |= FEC_TBD_LAST | FEC_TBD_TC | FEC_TBD_READY;
	fec->tbdIndex = (fec->tbdIndex + 1) % FEC_TBD_NUM;

#ifdef DEBUG
	printf ("DMA_TASK_ENABLE, fec->tbdIndex = %d \n", fec->tbdIndex);
#endif

	/*
	 * Kick the MII i/f
	 */
	if (fec->xcv_type != SEVENWIRE) {
		u16 phyStatus;

		miiphy_read (dev->name, 0, 0x1, &phyStatus);
	}

	/*
	 * Enable SmartDMA transmit task
	 */

#ifdef DEBUG
	tfifo_print (dev->name, fec);
#endif

	DMA_TASK_ENABLE (FEC_XMIT_TASK_NO);

#ifdef DEBUG
	tfifo_print (dev->name, fec);
#endif

#ifdef DEBUG
	printf ("+");
#endif

	fec->cleanTbdNum -= 1;

#ifdef DEBUG
	printf ("smartDMA ethernet Tx task enabled\n");
#endif
	/*
	 * wait until frame is sent .
	 */
	while (pTbd->status & FEC_TBD_READY) {
		udelay (10);
#ifdef DEBUG
		printf ("TDB status = %04x\n", pTbd->status);
#endif
	}

	return 0;
}


/********************************************************************/
static int mpc8220_fec_recv (struct eth_device *dev)
{
	/*
	 * This command pulls one frame from the card
	 */
	mpc8220_fec_priv *fec = (mpc8220_fec_priv *) dev->priv;
	FEC_RBD *pRbd = &fec->rbdBase[fec->rbdIndex];
	unsigned long ievent;
	int frame_length, len = 0;
	NBUF *frame;

#ifdef DEBUG
	printf ("mpc8220_fec_recv %d Start...\n", fec->rbdIndex);
	printf ("-");
#endif

	/*
	 * Check if any critical events have happened
	 */
	ievent = fec->eth->ievent;
	fec->eth->ievent = ievent;
	if (ievent & 0x20060000) {
		/* BABT, Rx/Tx FIFO errors */
		mpc8220_fec_halt (dev);
		mpc8220_fec_init (dev, NULL);
		return 0;
	}
	if (ievent & 0x80000000) {
		/* Heartbeat error */
		fec->eth->x_cntrl |= 0x00000001;
	}
	if (ievent & 0x10000000) {
		/* Graceful stop complete */
		if (fec->eth->x_cntrl & 0x00000001) {
			mpc8220_fec_halt (dev);
			fec->eth->x_cntrl &= ~0x00000001;
			mpc8220_fec_init (dev, NULL);
		}
	}

	if (!(pRbd->status & FEC_RBD_EMPTY)) {
		if ((pRbd->status & FEC_RBD_LAST)
		    && !(pRbd->status & FEC_RBD_ERR)
		    && ((pRbd->dataLength - 4) > 14)) {

			/*
			 * Get buffer address and size
			 */
			frame = (NBUF *) pRbd->dataPointer;
			frame_length = pRbd->dataLength - 4;

#if (0)
			{
				int i;

				printf ("recv data hdr:");
				for (i = 0; i < 14; i++)
					printf ("%x ", *(frame->head + i));
				printf ("\n");
			}
#endif
			/*
			 *  Fill the buffer and pass it to upper layers
			 */
/*			memcpy(buff, frame->head, 14);
			memcpy(buff + 14, frame->data, frame_length);*/
			NetReceive ((volatile uchar *) pRbd->dataPointer,
				    frame_length);
			len = frame_length;
		}
		/*
		 * Reset buffer descriptor as empty
		 */
		mpc8220_fec_rbd_clean (fec, pRbd);
	}
	DMA_CLEAR_IEVENT (FEC_RECV_TASK_NO);
	return len;
}


/********************************************************************/
int mpc8220_fec_initialize (bd_t * bis)
{
	mpc8220_fec_priv *fec;

#ifdef CONFIG_HAS_ETH1
	mpc8220_fec_priv *fec2;
#endif
	struct eth_device *dev;
	char *tmp, *end;
	char env_enetaddr[6];

#ifdef CONFIG_HAS_ETH1
	char env_enet1addr[6];
#endif
	int i;

	fec = (mpc8220_fec_priv *) malloc (sizeof (*fec));
	dev = (struct eth_device *) malloc (sizeof (*dev));
	memset (dev, 0, sizeof *dev);

	fec->eth = (ethernet_regs *) MMAP_FEC1;
#ifdef CONFIG_HAS_ETH1
	fec2 = (mpc8220_fec_priv *) malloc (sizeof (*fec));
	fec2->eth = (ethernet_regs *) MMAP_FEC2;
#endif
	fec->tbdBase = (FEC_TBD *) FEC_BD_BASE;
	fec->rbdBase =
		(FEC_RBD *) (FEC_BD_BASE + FEC_TBD_NUM * sizeof (FEC_TBD));
	fec->xcv_type = MII100;

	dev->priv = (void *) fec;
	dev->iobase = MMAP_FEC1;
	dev->init = mpc8220_fec_init;
	dev->halt = mpc8220_fec_halt;
	dev->send = mpc8220_fec_send;
	dev->recv = mpc8220_fec_recv;

	sprintf (dev->name, "FEC");
	eth_register (dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	miiphy_register (dev->name,
			fec8220_miiphy_read, fec8220_miiphy_write);
#endif

	/*
	 * Try to set the mac address now. The fec mac address is
	 * a garbage after reset. When not using fec for booting
	 * the Linux fec driver will try to work with this garbage.
	 */
	tmp = getenv ("ethaddr");
	if (tmp) {
		for (i = 0; i < 6; i++) {
			env_enetaddr[i] =
				tmp ? simple_strtoul (tmp, &end, 16) : 0;
			if (tmp)
				tmp = (*end) ? end + 1 : end;
		}
		mpc8220_fec_set_hwaddr (fec, env_enetaddr);
	}
#ifdef CONFIG_HAS_ETH1
	tmp = getenv ("eth1addr");
	if (tmp) {
		for (i = 0; i < 6; i++) {
			env_enet1addr[i] =
				tmp ? simple_strtoul (tmp, &end, 16) : 0;
			if (tmp)
				tmp = (*end) ? end + 1 : end;
		}
		mpc8220_fec_set_hwaddr (fec2, env_enet1addr);
	}
#endif

	return 1;
}

/* MII-interface related functions */
/********************************************************************/
int fec8220_miiphy_read (const char *devname, u8 phyAddr, u8 regAddr, u16 *retVal)
{
	ethernet_regs *eth = (ethernet_regs *) MMAP_FEC1;
	u32 reg;		/* convenient holder for the PHY register */
	u32 phy;		/* convenient holder for the PHY */
	int timeout = 0xffff;

	/*
	 * reading from any PHY's register is done by properly
	 * programming the FEC's MII data register.
	 */
	reg = regAddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyAddr << FEC_MII_DATA_PA_SHIFT;

	eth->mii_data =
		(FEC_MII_DATA_ST | FEC_MII_DATA_OP_RD | FEC_MII_DATA_TA | phy
		 | reg);

	/*
	 * wait for the related interrupt
	 */
	while ((timeout--) && (!(eth->ievent & 0x00800000)));

	if (timeout == 0) {
#ifdef DEBUG
		printf ("Read MDIO failed...\n");
#endif
		return -1;
	}

	/*
	 * clear mii interrupt bit
	 */
	eth->ievent = 0x00800000;

	/*
	 * it's now safe to read the PHY's register
	 */
	*retVal = (u16) eth->mii_data;

	return 0;
}

/********************************************************************/
int fec8220_miiphy_write(const char *devname, u8 phyAddr, u8 regAddr, u16 data)
{
	ethernet_regs *eth = (ethernet_regs *) MMAP_FEC1;
	u32 reg;		/* convenient holder for the PHY register */
	u32 phy;		/* convenient holder for the PHY */
	int timeout = 0xffff;

	reg = regAddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyAddr << FEC_MII_DATA_PA_SHIFT;

	eth->mii_data = (FEC_MII_DATA_ST | FEC_MII_DATA_OP_WR |
			 FEC_MII_DATA_TA | phy | reg | data);

	/*
	 * wait for the MII interrupt
	 */
	while ((timeout--) && (!(eth->ievent & 0x00800000)));

	if (timeout == 0) {
#ifdef DEBUG
		printf ("Write MDIO failed...\n");
#endif
		return -1;
	}

	/*
	 * clear MII interrupt bit
	 */
	eth->ievent = 0x00800000;

	return 0;
}

#endif /* CONFIG_MPC8220_FEC */
