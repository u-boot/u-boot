/*
 * (C) Copyright 2003-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Derived from the MPC8xx FEC driver.
 * Adapted for MPC512x by Grzegorz Bernacki <gjb@semihalf.com>
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include <asm/io.h>
#include "mpc512x_fec.h"

DECLARE_GLOBAL_DATA_PTR;

#define DEBUG 0

#if !(defined(CONFIG_MII) || defined(CONFIG_CMD_MII))
#error "CONFIG_MII has to be defined!"
#endif

int fec512x_miiphy_read(const char *devname, u8 phyAddr, u8 regAddr, u16 * retVal);
int fec512x_miiphy_write(const char *devname, u8 phyAddr, u8 regAddr, u16 data);
int mpc512x_fec_init_phy(struct eth_device *dev, bd_t * bis);

static uchar rx_buff[FEC_BUFFER_SIZE];
static int rx_buff_idx = 0;

/********************************************************************/
#if (DEBUG & 0x2)
static void mpc512x_fec_phydump (char *devname)
{
	u16 phyStatus, i;
	u8 phyAddr = CONFIG_PHY_ADDR;
	u8 reg_mask[] = {
		/* regs to print: 0...8, 21,27,31 */
		1, 1, 1, 1,  1, 1, 1, 1,     1, 0, 0, 0,  0, 0, 0, 0,
		0, 0, 0, 0,  0, 1, 0, 0,     0, 0, 0, 1,  0, 0, 0, 1,
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
static int mpc512x_fec_bd_init (mpc512x_fec_priv *fec)
{
	int ix;

	/*
	 * Receive BDs init
	 */
	for (ix = 0; ix < FEC_RBD_NUM; ix++) {
		fec->bdBase->rbd[ix].dataPointer =
				(u32)&fec->bdBase->recv_frames[ix];
		fec->bdBase->rbd[ix].status = FEC_RBD_EMPTY;
		fec->bdBase->rbd[ix].dataLength = 0;
	}

	/*
	 * have the last RBD to close the ring
	 */
	fec->bdBase->rbd[ix - 1].status |= FEC_RBD_WRAP;
	fec->rbdIndex = 0;

	/*
	 * Trasmit BDs init
	 */
	for (ix = 0; ix < FEC_TBD_NUM; ix++) {
		fec->bdBase->tbd[ix].status = 0;
	}

	/*
	 * Have the last TBD to close the ring
	 */
	fec->bdBase->tbd[ix - 1].status |= FEC_TBD_WRAP;

	/*
	 * Initialize some indices
	 */
	fec->tbdIndex = 0;
	fec->usedTbdIndex = 0;
	fec->cleanTbdNum = FEC_TBD_NUM;

	return 0;
}

/********************************************************************/
static void mpc512x_fec_rbd_clean (mpc512x_fec_priv *fec, volatile FEC_RBD * pRbd)
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
	 * Increment BD count
	 */
	fec->rbdIndex = (fec->rbdIndex + 1) % FEC_RBD_NUM;

	/*
	 * Now, we have an empty RxBD, notify FEC
	 * Set Descriptor polling active
	 */
	out_be32(&fec->eth->r_des_active, 0x01000000);
}

/********************************************************************/
static void mpc512x_fec_tbd_scrub (mpc512x_fec_priv *fec)
{
	volatile FEC_TBD *pUsedTbd;

#if (DEBUG & 0x1)
	printf ("tbd_scrub: fec->cleanTbdNum = %d, fec->usedTbdIndex = %d\n",
		fec->cleanTbdNum, fec->usedTbdIndex);
#endif

	/*
	 * process all the consumed TBDs
	 */
	while (fec->cleanTbdNum < FEC_TBD_NUM) {
		pUsedTbd = &fec->bdBase->tbd[fec->usedTbdIndex];
		if (pUsedTbd->status & FEC_TBD_READY) {
#if (DEBUG & 0x20)
			printf ("Cannot clean TBD %d, in use\n", fec->usedTbdIndex);
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
static void mpc512x_fec_set_hwaddr (mpc512x_fec_priv *fec, unsigned char *mac)
{
	u8 currByte;			/* byte for which to compute the CRC */
	int byte;			/* loop - counter */
	int bit;			/* loop - counter */
	u32 crc = 0xffffffff;		/* initial value */

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
		out_be32(&fec->eth->iaddr1, (1 << (crc - 32)));
		out_be32(&fec->eth->iaddr2, 0);
	} else {
		out_be32(&fec->eth->iaddr1, 0);
		out_be32(&fec->eth->iaddr2, (1 << crc));
	}

	/*
	 * Set physical address
	 */
	out_be32(&fec->eth->paddr1, (mac[0] << 24) + (mac[1] << 16) +
				    (mac[2] <<  8) + mac[3]);
	out_be32(&fec->eth->paddr2, (mac[4] << 24) + (mac[5] << 16) +
				     0x8808);
}

/********************************************************************/
static int mpc512x_fec_init (struct eth_device *dev, bd_t * bis)
{
	mpc512x_fec_priv *fec = (mpc512x_fec_priv *)dev->priv;

#if (DEBUG & 0x1)
	printf ("mpc512x_fec_init... Begin\n");
#endif

	mpc512x_fec_set_hwaddr (fec, dev->enetaddr);
	out_be32(&fec->eth->gaddr1, 0x00000000);
	out_be32(&fec->eth->gaddr2, 0x00000000);

	mpc512x_fec_init_phy (dev, bis);

	/* Set interrupt mask register */
	out_be32(&fec->eth->imask, 0x00000000);

	/* Clear FEC-Lite interrupt event register(IEVENT) */
	out_be32(&fec->eth->ievent, 0xffffffff);

	/* Set transmit fifo watermark register(X_WMRK), default = 64 */
	out_be32(&fec->eth->x_wmrk, 0x0);

	/* Set Opcode/Pause Duration Register */
	out_be32(&fec->eth->op_pause, 0x00010020);

	/* Frame length=1522; MII mode */
	out_be32(&fec->eth->r_cntrl, (FEC_MAX_FRAME_LEN << 16) | 0x24);

	/* Half-duplex, heartbeat disabled */
	out_be32(&fec->eth->x_cntrl, 0x00000000);

	/* Enable MIB counters */
	out_be32(&fec->eth->mib_control, 0x0);

	/* Setup recv fifo start and buff size */
	out_be32(&fec->eth->r_fstart, 0x500);
	out_be32(&fec->eth->r_buff_size, FEC_BUFFER_SIZE);

	/* Setup BD base addresses */
	out_be32(&fec->eth->r_des_start, (u32)fec->bdBase->rbd);
	out_be32(&fec->eth->x_des_start, (u32)fec->bdBase->tbd);

	/* DMA Control */
	out_be32(&fec->eth->dma_control, 0xc0000000);

	/* Enable FEC */
	setbits_be32(&fec->eth->ecntrl, 0x00000006);

	/* Initilize addresses and status words of BDs */
	mpc512x_fec_bd_init (fec);

	 /* Descriptor polling active */
	out_be32(&fec->eth->r_des_active, 0x01000000);

#if (DEBUG & 0x1)
	printf("mpc512x_fec_init... Done \n");
#endif
	return 1;
}

/********************************************************************/
int mpc512x_fec_init_phy (struct eth_device *dev, bd_t * bis)
{
	mpc512x_fec_priv *fec = (mpc512x_fec_priv *)dev->priv;
	const u8 phyAddr = CONFIG_PHY_ADDR;	/* Only one PHY */
	int timeout = 1;
	u16 phyStatus;

#if (DEBUG & 0x1)
	printf ("mpc512x_fec_init_phy... Begin\n");
#endif

	/*
	 * Clear FEC-Lite interrupt event register(IEVENT)
	 */
	out_be32(&fec->eth->ievent, 0xffffffff);

	/*
	 * Set interrupt mask register
	 */
	out_be32(&fec->eth->imask, 0x00000000);

	if (fec->xcv_type != SEVENWIRE) {
		/*
		 * Set MII_SPEED = (1/(mii_speed * 2)) * System Clock
		 * and do not drop the Preamble.
		 */
		out_be32(&fec->eth->mii_speed,
			 (((gd->arch.ips_clk / 1000000) / 5) + 1) << 1);

		/*
		 * Reset PHY, then delay 300ns
		 */
		miiphy_write (dev->name, phyAddr, 0x0, 0x8000);
		udelay (1000);

		if (fec->xcv_type == MII10) {
		/*
		 * Force 10Base-T, FDX operation
		 */
#if (DEBUG & 0x2)
			printf ("Forcing 10 Mbps ethernet link... ");
#endif
			miiphy_read (dev->name, phyAddr, 0x1, &phyStatus);

			miiphy_write (dev->name, phyAddr, 0x0, 0x0180);

			timeout = 20;
			do {    /* wait for link status to go down */
				udelay (10000);
				if ((timeout--) == 0) {
#if (DEBUG & 0x2)
					printf ("hmmm, should not have waited...");
#endif
					break;
				}
				miiphy_read (dev->name, phyAddr, 0x1, &phyStatus);
#if (DEBUG & 0x2)
				printf ("=");
#endif
			} while ((phyStatus & 0x0004)); /* !link up */

			timeout = 1000;
			do {    /* wait for link status to come back up */
				udelay (10000);
				if ((timeout--) == 0) {
					printf ("failed. Link is down.\n");
					break;
				}
				miiphy_read (dev->name, phyAddr, 0x1, &phyStatus);
#if (DEBUG & 0x2)
				printf ("+");
#endif
			} while (!(phyStatus & 0x0004)); /* !link up */

#if (DEBUG & 0x2)
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
			timeout = 2500;
			do {
				udelay (1000);

				if ((timeout--) == 0) {
#if (DEBUG & 0x2)
					printf ("PHY auto neg 0 failed...\n");
#endif
					return -1;
				}

				if (miiphy_read (dev->name, phyAddr, 0x1, &phyStatus) != 0) {
#if (DEBUG & 0x2)
					printf ("PHY auto neg 1 failed 0x%04x...\n", phyStatus);
#endif
					return -1;
				}
			} while (!(phyStatus & 0x0004));

#if (DEBUG & 0x2)
			printf ("PHY auto neg complete! \n");
#endif
		}
	}

#if (DEBUG & 0x2)
	if (fec->xcv_type != SEVENWIRE)
		mpc512x_fec_phydump (dev->name);
#endif

#if (DEBUG & 0x1)
	printf ("mpc512x_fec_init_phy... Done \n");
#endif
	return 1;
}

/********************************************************************/
static void mpc512x_fec_halt (struct eth_device *dev)
{
	mpc512x_fec_priv *fec = (mpc512x_fec_priv *)dev->priv;
	int counter = 0xffff;

#if (DEBUG & 0x2)
	if (fec->xcv_type != SEVENWIRE)
		mpc512x_fec_phydump (dev->name);
#endif

	/*
	 * mask FEC chip interrupts
	 */
	out_be32(&fec->eth->imask, 0);

	/*
	 * issue graceful stop command to the FEC transmitter if necessary
	 */
	setbits_be32(&fec->eth->x_cntrl, 0x00000001);

	/*
	 * wait for graceful stop to register
	 */
	while ((counter--) && (!(in_be32(&fec->eth->ievent) & 0x10000000)))
		;

	/*
	 * Disable the Ethernet Controller
	 */
	clrbits_be32(&fec->eth->ecntrl, 0x00000002);

	/*
	 * Issue a reset command to the FEC chip
	 */
	setbits_be32(&fec->eth->ecntrl, 0x1);

	/*
	 * wait at least 16 clock cycles
	 */
	udelay (10);
#if (DEBUG & 0x3)
	printf ("Ethernet task stopped\n");
#endif
}

/********************************************************************/

static int mpc512x_fec_send(struct eth_device *dev, void *eth_data,
			    int data_length)
{
	/*
	 * This routine transmits one frame.  This routine only accepts
	 * 6-byte Ethernet addresses.
	 */
	mpc512x_fec_priv *fec = (mpc512x_fec_priv *)dev->priv;
	volatile FEC_TBD *pTbd;

#if (DEBUG & 0x20)
	printf("tbd status: 0x%04x\n", fec->tbdBase[fec->tbdIndex].status);
#endif

	/*
	 * Clear Tx BD ring at first
	 */
	mpc512x_fec_tbd_scrub (fec);

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
#if (DEBUG & 0x20)
		printf ("No available TxBDs ...\n");
#endif
		return -1;
	}

	/*
	 * Get the first TxBD to send the mac header
	 */
	pTbd = &fec->bdBase->tbd[fec->tbdIndex];
	pTbd->dataLength = data_length;
	pTbd->dataPointer = (u32)eth_data;
	pTbd->status |= FEC_TBD_LAST | FEC_TBD_TC | FEC_TBD_READY;
	fec->tbdIndex = (fec->tbdIndex + 1) % FEC_TBD_NUM;

	/* Activate transmit Buffer Descriptor polling */
	out_be32(&fec->eth->x_des_active, 0x01000000);

#if (DEBUG & 0x8)
	printf ( "+" );
#endif

	fec->cleanTbdNum -= 1;

	/*
	 * wait until frame is sent .
	 */
	while (pTbd->status & FEC_TBD_READY) {
		udelay (10);
#if (DEBUG & 0x8)
		printf ("TDB status = %04x\n", pTbd->status);
#endif
	}

	return 0;
}


/********************************************************************/
static int mpc512x_fec_recv (struct eth_device *dev)
{
	/*
	 * This command pulls one frame from the card
	 */
	mpc512x_fec_priv *fec = (mpc512x_fec_priv *)dev->priv;
	volatile FEC_RBD *pRbd = &fec->bdBase->rbd[fec->rbdIndex];
	unsigned long ievent;
	int frame_length = 0;

#if (DEBUG & 0x1)
	printf ("mpc512x_fec_recv %d Start...\n", fec->rbdIndex);
#endif
#if (DEBUG & 0x8)
	printf( "-" );
#endif

	/*
	 * Check if any critical events have happened
	 */
	ievent = in_be32(&fec->eth->ievent);
	out_be32(&fec->eth->ievent, ievent);
	if (ievent & 0x20060000) {
		/* BABT, Rx/Tx FIFO errors */
		mpc512x_fec_halt (dev);
		mpc512x_fec_init (dev, NULL);
		return 0;
	}
	if (ievent & 0x80000000) {
		/* Heartbeat error */
		setbits_be32(&fec->eth->x_cntrl, 0x00000001);
	}
	if (ievent & 0x10000000) {
		/* Graceful stop complete */
		if (in_be32(&fec->eth->x_cntrl) & 0x00000001) {
			mpc512x_fec_halt (dev);
			clrbits_be32(&fec->eth->x_cntrl, 0x00000001);;
			mpc512x_fec_init (dev, NULL);
		}
	}

	if (!(pRbd->status & FEC_RBD_EMPTY)) {
		if (!(pRbd->status & FEC_RBD_ERR) &&
			((pRbd->dataLength - 4) > 14)) {

			/*
			 * Get buffer size
			 */
			if (pRbd->status & FEC_RBD_LAST)
				frame_length = pRbd->dataLength - 4;
			else
				frame_length = pRbd->dataLength;
#if (DEBUG & 0x20)
			{
				int i;
				printf ("recv data length 0x%08x data hdr: ",
					pRbd->dataLength);
				for (i = 0; i < 14; i++)
					printf ("%x ", *((u8*)pRbd->dataPointer + i));
				printf("\n");
			}
#endif
			/*
			 *  Fill the buffer and pass it to upper layers
			 */
			memcpy (&rx_buff[rx_buff_idx], (void*)pRbd->dataPointer,
				frame_length - rx_buff_idx);
			rx_buff_idx = frame_length;

			if (pRbd->status & FEC_RBD_LAST) {
				NetReceive ((uchar*)rx_buff, frame_length);
				rx_buff_idx = 0;
			}
		}

		/*
		 * Reset buffer descriptor as empty
		 */
		mpc512x_fec_rbd_clean (fec, pRbd);
	}

	/* Try to fill Buffer Descriptors */
	out_be32(&fec->eth->r_des_active, 0x01000000);

	return frame_length;
}

/********************************************************************/
int mpc512x_fec_initialize (bd_t * bis)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	mpc512x_fec_priv *fec;
	struct eth_device *dev;
	void * bd;

	fec = (mpc512x_fec_priv *) malloc (sizeof(*fec));
	dev = (struct eth_device *) malloc (sizeof(*dev));
	memset (dev, 0, sizeof *dev);

	fec->eth = &im->fec;

# ifndef CONFIG_FEC_10MBIT
	fec->xcv_type = MII100;
# else
	fec->xcv_type = MII10;
# endif
	dev->priv = (void *)fec;
	dev->iobase = (int)&im->fec;
	dev->init = mpc512x_fec_init;
	dev->halt = mpc512x_fec_halt;
	dev->send = mpc512x_fec_send;
	dev->recv = mpc512x_fec_recv;

	sprintf (dev->name, "FEC");
	eth_register (dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	miiphy_register (dev->name,
			fec512x_miiphy_read, fec512x_miiphy_write);
#endif

	/* Clean up space FEC's MIB and FIFO RAM ...*/
	memset ((void *)&im->fec.mib,  0x00, sizeof(im->fec.mib));
	memset ((void *)&im->fec.fifo, 0x00, sizeof(im->fec.fifo));

	/*
	 * Malloc space for BDs  (must be quad word-aligned)
	 * this pointer is lost, so cannot be freed
	 */
	bd = malloc (sizeof(mpc512x_buff_descs) + 0x1f);
	fec->bdBase = (mpc512x_buff_descs*)((u32)bd & 0xfffffff0);
	memset ((void *) bd, 0x00, sizeof(mpc512x_buff_descs) + 0x1f);

	/*
	 * Set interrupt mask register
	 */
	out_be32(&fec->eth->imask, 0x00000000);

	/*
	 * Clear FEC-Lite interrupt event register(IEVENT)
	 */
	out_be32(&fec->eth->ievent, 0xffffffff);

	return 1;
}

/* MII-interface related functions */
/********************************************************************/
int fec512x_miiphy_read(const char *devname, u8 phyAddr, u8 regAddr, u16 *retVal)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile fec512x_t *eth = &im->fec;
	u32 reg;		/* convenient holder for the PHY register */
	u32 phy;		/* convenient holder for the PHY */
	int timeout = 0xffff;

	/*
	 * reading from any PHY's register is done by properly
	 * programming the FEC's MII data register.
	 */
	reg = regAddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyAddr << FEC_MII_DATA_PA_SHIFT;

	out_be32(&eth->mii_data, FEC_MII_DATA_ST |
				 FEC_MII_DATA_OP_RD |
				 FEC_MII_DATA_TA |
				 phy | reg);

	/*
	 * wait for the related interrupt
	 */
	while ((timeout--) && (!(in_be32(&eth->ievent) & 0x00800000)))
		;

	if (timeout == 0) {
#if (DEBUG & 0x2)
		printf ("Read MDIO failed...\n");
#endif
		return -1;
	}

	/*
	 * clear mii interrupt bit
	 */
	out_be32(&eth->ievent, 0x00800000);

	/*
	 * it's now safe to read the PHY's register
	 */
	*retVal = (u16) in_be32(&eth->mii_data);

	return 0;
}

/********************************************************************/
int fec512x_miiphy_write(const char *devname, u8 phyAddr, u8 regAddr, u16 data)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile fec512x_t *eth = &im->fec;
	u32 reg;		/* convenient holder for the PHY register */
	u32 phy;		/* convenient holder for the PHY */
	int timeout = 0xffff;

	reg = regAddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyAddr << FEC_MII_DATA_PA_SHIFT;

	out_be32(&eth->mii_data, FEC_MII_DATA_ST |
				 FEC_MII_DATA_OP_WR |
				 FEC_MII_DATA_TA |
				 phy | reg | data);

	/*
	 * wait for the MII interrupt
	 */
	while ((timeout--) && (!(in_be32(&eth->ievent) & 0x00800000)))
		;

	if (timeout == 0) {
#if (DEBUG & 0x2)
		printf ("Write MDIO failed...\n");
#endif
		return -1;
	}

	/*
	 * clear MII interrupt bit
	 */
	out_be32(&eth->ievent, 0x00800000);

	return 0;
}
