/*
 * (C) Copyright 2009 Ilya Yanok, Emcraft Systems Ltd <yanok@emcraft.com>
 * (C) Copyright 2008,2009 Eric Jarrige <eric.jarrige@armadeus.org>
 * (C) Copyright 2008 Armadeus Systems nc
 * (C) Copyright 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 * (C) Copyright 2007 Pengutronix, Juergen Beisert <j.beisert@pengutronix.de>
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
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include "fec_mxc.h"

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <asm/errno.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_MII
#error "CONFIG_MII has to be defined!"
#endif

#undef DEBUG

struct nbuf {
	uint8_t data[1500];	/**< actual data */
	int length;		/**< actual length */
	int used;		/**< buffer in use or not */
	uint8_t head[16];	/**< MAC header(6 + 6 + 2) + 2(aligned) */
};

struct fec_priv gfec = {
	.eth       = (struct ethernet_regs *)IMX_FEC_BASE,
	.xcv_type  = MII100,
	.rbd_base  = NULL,
	.rbd_index = 0,
	.tbd_base  = NULL,
	.tbd_index = 0,
	.bd        = NULL,
	.rdb_ptr   = NULL,
	.base_ptr  = NULL,
};

/*
 * MII-interface related functions
 */
static int fec_miiphy_read(const char *dev, uint8_t phyAddr, uint8_t regAddr,
		uint16_t *retVal)
{
	struct eth_device *edev = eth_get_dev_by_name(dev);
	struct fec_priv *fec = (struct fec_priv *)edev->priv;

	uint32_t reg;		/* convenient holder for the PHY register */
	uint32_t phy;		/* convenient holder for the PHY */
	uint32_t start;

	/*
	 * reading from any PHY's register is done by properly
	 * programming the FEC's MII data register.
	 */
	writel(FEC_IEVENT_MII, &fec->eth->ievent);
	reg = regAddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyAddr << FEC_MII_DATA_PA_SHIFT;

	writel(FEC_MII_DATA_ST | FEC_MII_DATA_OP_RD | FEC_MII_DATA_TA |
			phy | reg, &fec->eth->mii_data);

	/*
	 * wait for the related interrupt
	 */
	start = get_timer(0);
	while (!(readl(&fec->eth->ievent) & FEC_IEVENT_MII)) {
		if (get_timer(start) > (CONFIG_SYS_HZ / 1000)) {
			printf("Read MDIO failed...\n");
			return -1;
		}
	}

	/*
	 * clear mii interrupt bit
	 */
	writel(FEC_IEVENT_MII, &fec->eth->ievent);

	/*
	 * it's now safe to read the PHY's register
	 */
	*retVal = readl(&fec->eth->mii_data);
	debug("fec_miiphy_read: phy: %02x reg:%02x val:%#x\n", phyAddr,
			regAddr, *retVal);
	return 0;
}

static void fec_mii_setspeed(struct fec_priv *fec)
{
	/*
	 * Set MII_SPEED = (1/(mii_speed * 2)) * System Clock
	 * and do not drop the Preamble.
	 */
	writel((((imx_get_fecclk() / 1000000) + 2) / 5) << 1,
			&fec->eth->mii_speed);
	debug("fec_init: mii_speed %#lx\n",
			fec->eth->mii_speed);
}
static int fec_miiphy_write(const char *dev, uint8_t phyAddr, uint8_t regAddr,
		uint16_t data)
{
	struct eth_device *edev = eth_get_dev_by_name(dev);
	struct fec_priv *fec = (struct fec_priv *)edev->priv;

	uint32_t reg;		/* convenient holder for the PHY register */
	uint32_t phy;		/* convenient holder for the PHY */
	uint32_t start;

	reg = regAddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyAddr << FEC_MII_DATA_PA_SHIFT;

	writel(FEC_MII_DATA_ST | FEC_MII_DATA_OP_WR |
		FEC_MII_DATA_TA | phy | reg | data, &fec->eth->mii_data);

	/*
	 * wait for the MII interrupt
	 */
	start = get_timer(0);
	while (!(readl(&fec->eth->ievent) & FEC_IEVENT_MII)) {
		if (get_timer(start) > (CONFIG_SYS_HZ / 1000)) {
			printf("Write MDIO failed...\n");
			return -1;
		}
	}

	/*
	 * clear MII interrupt bit
	 */
	writel(FEC_IEVENT_MII, &fec->eth->ievent);
	debug("fec_miiphy_write: phy: %02x reg:%02x val:%#x\n", phyAddr,
			regAddr, data);

	return 0;
}

static int miiphy_restart_aneg(struct eth_device *dev)
{
	/*
	 * Wake up from sleep if necessary
	 * Reset PHY, then delay 300ns
	 */
#ifdef CONFIG_MX27
	miiphy_write(dev->name, CONFIG_FEC_MXC_PHYADDR, MII_DCOUNTER, 0x00FF);
#endif
	miiphy_write(dev->name, CONFIG_FEC_MXC_PHYADDR, MII_BMCR,
			BMCR_RESET);
	udelay(1000);

	/*
	 * Set the auto-negotiation advertisement register bits
	 */
	miiphy_write(dev->name, CONFIG_FEC_MXC_PHYADDR, MII_ADVERTISE,
			LPA_100FULL | LPA_100HALF | LPA_10FULL |
			LPA_10HALF | PHY_ANLPAR_PSB_802_3);
	miiphy_write(dev->name, CONFIG_FEC_MXC_PHYADDR, MII_BMCR,
			BMCR_ANENABLE | BMCR_ANRESTART);

	return 0;
}

static int miiphy_wait_aneg(struct eth_device *dev)
{
	uint32_t start;
	uint16_t status;

	/*
	 * Wait for AN completion
	 */
	start = get_timer(0);
	do {
		if (get_timer(start) > (CONFIG_SYS_HZ * 5)) {
			printf("%s: Autonegotiation timeout\n", dev->name);
			return -1;
		}

		if (miiphy_read(dev->name, CONFIG_FEC_MXC_PHYADDR,
					MII_BMSR, &status)) {
			printf("%s: Autonegotiation failed. status: 0x%04x\n",
					dev->name, status);
			return -1;
		}
	} while (!(status & BMSR_LSTATUS));

	return 0;
}
static int fec_rx_task_enable(struct fec_priv *fec)
{
	writel(1 << 24, &fec->eth->r_des_active);
	return 0;
}

static int fec_rx_task_disable(struct fec_priv *fec)
{
	return 0;
}

static int fec_tx_task_enable(struct fec_priv *fec)
{
	writel(1 << 24, &fec->eth->x_des_active);
	return 0;
}

static int fec_tx_task_disable(struct fec_priv *fec)
{
	return 0;
}

/**
 * Initialize receive task's buffer descriptors
 * @param[in] fec all we know about the device yet
 * @param[in] count receive buffer count to be allocated
 * @param[in] size size of each receive buffer
 * @return 0 on success
 *
 * For this task we need additional memory for the data buffers. And each
 * data buffer requires some alignment. Thy must be aligned to a specific
 * boundary each (DB_DATA_ALIGNMENT).
 */
static int fec_rbd_init(struct fec_priv *fec, int count, int size)
{
	int ix;
	uint32_t p = 0;

	/* reserve data memory and consider alignment */
	if (fec->rdb_ptr == NULL)
		fec->rdb_ptr = malloc(size * count + DB_DATA_ALIGNMENT);
	p = (uint32_t)fec->rdb_ptr;
	if (!p) {
		puts("fec_mxc: not enough malloc memory\n");
		return -ENOMEM;
	}
	memset((void *)p, 0, size * count + DB_DATA_ALIGNMENT);
	p += DB_DATA_ALIGNMENT-1;
	p &= ~(DB_DATA_ALIGNMENT-1);

	for (ix = 0; ix < count; ix++) {
		writel(p, &fec->rbd_base[ix].data_pointer);
		p += size;
		writew(FEC_RBD_EMPTY, &fec->rbd_base[ix].status);
		writew(0, &fec->rbd_base[ix].data_length);
	}
	/*
	 * mark the last RBD to close the ring
	 */
	writew(FEC_RBD_WRAP | FEC_RBD_EMPTY, &fec->rbd_base[ix - 1].status);
	fec->rbd_index = 0;

	return 0;
}

/**
 * Initialize transmit task's buffer descriptors
 * @param[in] fec all we know about the device yet
 *
 * Transmit buffers are created externally. We only have to init the BDs here.\n
 * Note: There is a race condition in the hardware. When only one BD is in
 * use it must be marked with the WRAP bit to use it for every transmitt.
 * This bit in combination with the READY bit results into double transmit
 * of each data buffer. It seems the state machine checks READY earlier then
 * resetting it after the first transfer.
 * Using two BDs solves this issue.
 */
static void fec_tbd_init(struct fec_priv *fec)
{
	writew(0x0000, &fec->tbd_base[0].status);
	writew(FEC_TBD_WRAP, &fec->tbd_base[1].status);
	fec->tbd_index = 0;
}

/**
 * Mark the given read buffer descriptor as free
 * @param[in] last 1 if this is the last buffer descriptor in the chain, else 0
 * @param[in] pRbd buffer descriptor to mark free again
 */
static void fec_rbd_clean(int last, struct fec_bd *pRbd)
{
	/*
	 * Reset buffer descriptor as empty
	 */
	if (last)
		writew(FEC_RBD_WRAP | FEC_RBD_EMPTY, &pRbd->status);
	else
		writew(FEC_RBD_EMPTY, &pRbd->status);
	/*
	 * no data in it
	 */
	writew(0, &pRbd->data_length);
}

static int fec_get_hwaddr(struct eth_device *dev, unsigned char *mac)
{
	imx_get_mac_from_fuse(mac);
	return !is_valid_ether_addr(mac);
}

static int fec_set_hwaddr(struct eth_device *dev)
{
	uchar *mac = dev->enetaddr;
	struct fec_priv *fec = (struct fec_priv *)dev->priv;

	writel(0, &fec->eth->iaddr1);
	writel(0, &fec->eth->iaddr2);
	writel(0, &fec->eth->gaddr1);
	writel(0, &fec->eth->gaddr2);

	/*
	 * Set physical address
	 */
	writel((mac[0] << 24) + (mac[1] << 16) + (mac[2] << 8) + mac[3],
			&fec->eth->paddr1);
	writel((mac[4] << 24) + (mac[5] << 16) + 0x8808, &fec->eth->paddr2);

	return 0;
}

/**
 * Start the FEC engine
 * @param[in] dev Our device to handle
 */
static int fec_open(struct eth_device *edev)
{
	struct fec_priv *fec = (struct fec_priv *)edev->priv;

	debug("fec_open: fec_open(dev)\n");
	/* full-duplex, heartbeat disabled */
	writel(1 << 2, &fec->eth->x_cntrl);
	fec->rbd_index = 0;

	/*
	 * Enable FEC-Lite controller
	 */
	writel(readl(&fec->eth->ecntrl) | FEC_ECNTRL_ETHER_EN,
		&fec->eth->ecntrl);
#if defined(CONFIG_MX25) || defined(CONFIG_MX53)
	udelay(100);
	/*
	 * setup the MII gasket for RMII mode
	 */

	/* disable the gasket */
	writew(0, &fec->eth->miigsk_enr);

	/* wait for the gasket to be disabled */
	while (readw(&fec->eth->miigsk_enr) & MIIGSK_ENR_READY)
		udelay(2);

	/* configure gasket for RMII, 50 MHz, no loopback, and no echo */
	writew(MIIGSK_CFGR_IF_MODE_RMII, &fec->eth->miigsk_cfgr);

	/* re-enable the gasket */
	writew(MIIGSK_ENR_EN, &fec->eth->miigsk_enr);

	/* wait until MII gasket is ready */
	int max_loops = 10;
	while ((readw(&fec->eth->miigsk_enr) & MIIGSK_ENR_READY) == 0) {
		if (--max_loops <= 0) {
			printf("WAIT for MII Gasket ready timed out\n");
			break;
		}
	}
#endif

	miiphy_wait_aneg(edev);
	miiphy_speed(edev->name, CONFIG_FEC_MXC_PHYADDR);
	miiphy_duplex(edev->name, CONFIG_FEC_MXC_PHYADDR);

	/*
	 * Enable SmartDMA receive task
	 */
	fec_rx_task_enable(fec);

	udelay(100000);
	return 0;
}

static int fec_init(struct eth_device *dev, bd_t* bd)
{
	uint32_t base;
	struct fec_priv *fec = (struct fec_priv *)dev->priv;

	/* Initialize MAC address */
	fec_set_hwaddr(dev);

	/*
	 * reserve memory for both buffer descriptor chains at once
	 * Datasheet forces the startaddress of each chain is 16 byte
	 * aligned
	 */
	if (fec->base_ptr == NULL)
		fec->base_ptr = malloc((2 + FEC_RBD_NUM) *
				sizeof(struct fec_bd) + DB_ALIGNMENT);
	base = (uint32_t)fec->base_ptr;
	if (!base) {
		puts("fec_mxc: not enough malloc memory\n");
		return -ENOMEM;
	}
	memset((void *)base, 0, (2 + FEC_RBD_NUM) *
			sizeof(struct fec_bd) + DB_ALIGNMENT);
	base += (DB_ALIGNMENT-1);
	base &= ~(DB_ALIGNMENT-1);

	fec->rbd_base = (struct fec_bd *)base;

	base += FEC_RBD_NUM * sizeof(struct fec_bd);

	fec->tbd_base = (struct fec_bd *)base;

	/*
	 * Set interrupt mask register
	 */
	writel(0x00000000, &fec->eth->imask);

	/*
	 * Clear FEC-Lite interrupt event register(IEVENT)
	 */
	writel(0xffffffff, &fec->eth->ievent);


	/*
	 * Set FEC-Lite receive control register(R_CNTRL):
	 */
	if (fec->xcv_type == SEVENWIRE) {
		/*
		 * Frame length=1518; 7-wire mode
		 */
		writel(0x05ee0020, &fec->eth->r_cntrl);	/* FIXME 0x05ee0000 */
	} else {
		/*
		 * Frame length=1518; MII mode;
		 */
		writel(0x05ee0024, &fec->eth->r_cntrl);	/* FIXME 0x05ee0004 */

		fec_mii_setspeed(fec);
	}
	/*
	 * Set Opcode/Pause Duration Register
	 */
	writel(0x00010020, &fec->eth->op_pause);	/* FIXME 0xffff0020; */
	writel(0x2, &fec->eth->x_wmrk);
	/*
	 * Set multicast address filter
	 */
	writel(0x00000000, &fec->eth->gaddr1);
	writel(0x00000000, &fec->eth->gaddr2);


	/* clear MIB RAM */
	long *mib_ptr = (long *)(IMX_FEC_BASE + 0x200);
	while (mib_ptr <= (long *)(IMX_FEC_BASE + 0x2FC))
		*mib_ptr++ = 0;

	/* FIFO receive start register */
	writel(0x520, &fec->eth->r_fstart);

	/* size and address of each buffer */
	writel(FEC_MAX_PKT_SIZE, &fec->eth->emrbr);
	writel((uint32_t)fec->tbd_base, &fec->eth->etdsr);
	writel((uint32_t)fec->rbd_base, &fec->eth->erdsr);

	/*
	 * Initialize RxBD/TxBD rings
	 */
	if (fec_rbd_init(fec, FEC_RBD_NUM, FEC_MAX_PKT_SIZE) < 0) {
		free(fec->base_ptr);
		fec->base_ptr = NULL;
		return -ENOMEM;
	}
	fec_tbd_init(fec);


	if (fec->xcv_type != SEVENWIRE)
		miiphy_restart_aneg(dev);

	fec_open(dev);
	return 0;
}

/**
 * Halt the FEC engine
 * @param[in] dev Our device to handle
 */
static void fec_halt(struct eth_device *dev)
{
	struct fec_priv *fec = &gfec;
	int counter = 0xffff;

	/*
	 * issue graceful stop command to the FEC transmitter if necessary
	 */
	writel(FEC_TCNTRL_GTS | readl(&fec->eth->x_cntrl),
			&fec->eth->x_cntrl);

	debug("eth_halt: wait for stop regs\n");
	/*
	 * wait for graceful stop to register
	 */
	while ((counter--) && (!(readl(&fec->eth->ievent) & FEC_IEVENT_GRA)))
		udelay(1);

	/*
	 * Disable SmartDMA tasks
	 */
	fec_tx_task_disable(fec);
	fec_rx_task_disable(fec);

	/*
	 * Disable the Ethernet Controller
	 * Note: this will also reset the BD index counter!
	 */
	writel(readl(&fec->eth->ecntrl) & ~FEC_ECNTRL_ETHER_EN,
			&fec->eth->ecntrl);
	fec->rbd_index = 0;
	fec->tbd_index = 0;
	debug("eth_halt: done\n");
}

/**
 * Transmit one frame
 * @param[in] dev Our ethernet device to handle
 * @param[in] packet Pointer to the data to be transmitted
 * @param[in] length Data count in bytes
 * @return 0 on success
 */
static int fec_send(struct eth_device *dev, volatile void* packet, int length)
{
	unsigned int status;

	/*
	 * This routine transmits one frame.  This routine only accepts
	 * 6-byte Ethernet addresses.
	 */
	struct fec_priv *fec = (struct fec_priv *)dev->priv;

	/*
	 * Check for valid length of data.
	 */
	if ((length > 1500) || (length <= 0)) {
		printf("Payload (%d) too large\n", length);
		return -1;
	}

	/*
	 * Setup the transmit buffer
	 * Note: We are always using the first buffer for transmission,
	 * the second will be empty and only used to stop the DMA engine
	 */
	writew(length, &fec->tbd_base[fec->tbd_index].data_length);
	writel((uint32_t)packet, &fec->tbd_base[fec->tbd_index].data_pointer);
	/*
	 * update BD's status now
	 * This block:
	 * - is always the last in a chain (means no chain)
	 * - should transmitt the CRC
	 * - might be the last BD in the list, so the address counter should
	 *   wrap (-> keep the WRAP flag)
	 */
	status = readw(&fec->tbd_base[fec->tbd_index].status) & FEC_TBD_WRAP;
	status |= FEC_TBD_LAST | FEC_TBD_TC | FEC_TBD_READY;
	writew(status, &fec->tbd_base[fec->tbd_index].status);

	/*
	 * Enable SmartDMA transmit task
	 */
	fec_tx_task_enable(fec);

	/*
	 * wait until frame is sent .
	 */
	while (readw(&fec->tbd_base[fec->tbd_index].status) & FEC_TBD_READY) {
		udelay(1);
	}
	debug("fec_send: status 0x%x index %d\n",
			readw(&fec->tbd_base[fec->tbd_index].status),
			fec->tbd_index);
	/* for next transmission use the other buffer */
	if (fec->tbd_index)
		fec->tbd_index = 0;
	else
		fec->tbd_index = 1;

	return 0;
}

/**
 * Pull one frame from the card
 * @param[in] dev Our ethernet device to handle
 * @return Length of packet read
 */
static int fec_recv(struct eth_device *dev)
{
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	struct fec_bd *rbd = &fec->rbd_base[fec->rbd_index];
	unsigned long ievent;
	int frame_length, len = 0;
	struct nbuf *frame;
	uint16_t bd_status;
	uchar buff[FEC_MAX_PKT_SIZE];

	/*
	 * Check if any critical events have happened
	 */
	ievent = readl(&fec->eth->ievent);
	writel(ievent, &fec->eth->ievent);
	debug("fec_recv: ievent 0x%x\n", ievent);
	if (ievent & FEC_IEVENT_BABR) {
		fec_halt(dev);
		fec_init(dev, fec->bd);
		printf("some error: 0x%08lx\n", ievent);
		return 0;
	}
	if (ievent & FEC_IEVENT_HBERR) {
		/* Heartbeat error */
		writel(0x00000001 | readl(&fec->eth->x_cntrl),
				&fec->eth->x_cntrl);
	}
	if (ievent & FEC_IEVENT_GRA) {
		/* Graceful stop complete */
		if (readl(&fec->eth->x_cntrl) & 0x00000001) {
			fec_halt(dev);
			writel(~0x00000001 & readl(&fec->eth->x_cntrl),
					&fec->eth->x_cntrl);
			fec_init(dev, fec->bd);
		}
	}

	/*
	 * ensure reading the right buffer status
	 */
	bd_status = readw(&rbd->status);
	debug("fec_recv: status 0x%x\n", bd_status);

	if (!(bd_status & FEC_RBD_EMPTY)) {
		if ((bd_status & FEC_RBD_LAST) && !(bd_status & FEC_RBD_ERR) &&
			((readw(&rbd->data_length) - 4) > 14)) {
			/*
			 * Get buffer address and size
			 */
			frame = (struct nbuf *)readl(&rbd->data_pointer);
			frame_length = readw(&rbd->data_length) - 4;
			/*
			 *  Fill the buffer and pass it to upper layers
			 */
			memcpy(buff, frame->data, frame_length);
			NetReceive(buff, frame_length);
			len = frame_length;
		} else {
			if (bd_status & FEC_RBD_ERR)
				printf("error frame: 0x%08lx 0x%08x\n",
						(ulong)rbd->data_pointer,
						bd_status);
		}
		/*
		 * free the current buffer, restart the engine
		 * and move forward to the next buffer
		 */
		fec_rbd_clean(fec->rbd_index == (FEC_RBD_NUM - 1) ? 1 : 0, rbd);
		fec_rx_task_enable(fec);
		fec->rbd_index = (fec->rbd_index + 1) % FEC_RBD_NUM;
	}
	debug("fec_recv: stop\n");

	return len;
}

static int fec_probe(bd_t *bd)
{
	struct eth_device *edev;
	struct fec_priv *fec = &gfec;
	unsigned char ethaddr[6];

	/* create and fill edev struct */
	edev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (!edev) {
		puts("fec_mxc: not enough malloc memory\n");
		return -ENOMEM;
	}
	memset(edev, 0, sizeof(*edev));
	edev->priv = fec;
	edev->init = fec_init;
	edev->send = fec_send;
	edev->recv = fec_recv;
	edev->halt = fec_halt;
	edev->write_hwaddr = fec_set_hwaddr;

	fec->eth = (struct ethernet_regs *)IMX_FEC_BASE;
	fec->bd = bd;

	fec->xcv_type = MII100;

	/* Reset chip. */
	writel(readl(&fec->eth->ecntrl) | FEC_ECNTRL_RESET, &fec->eth->ecntrl);
	while (readl(&fec->eth->ecntrl) & 1)
		udelay(10);

	/*
	 * Set interrupt mask register
	 */
	writel(0x00000000, &fec->eth->imask);

	/*
	 * Clear FEC-Lite interrupt event register(IEVENT)
	 */
	writel(0xffffffff, &fec->eth->ievent);

	/*
	 * Set FEC-Lite receive control register(R_CNTRL):
	 */
	/*
	 * Frame length=1518; MII mode;
	 */
	writel(0x05ee0024, &fec->eth->r_cntrl);	/* FIXME 0x05ee0004 */
	fec_mii_setspeed(fec);

	sprintf(edev->name, "FEC");

	miiphy_register(edev->name, fec_miiphy_read, fec_miiphy_write);

	eth_register(edev);

	if (fec_get_hwaddr(edev, ethaddr) == 0) {
		printf("got MAC address from fuse: %pM\n", ethaddr);
		memcpy(edev->enetaddr, ethaddr, 6);
	}

	return 0;
}

int fecmxc_initialize(bd_t *bd)
{
	int lout = 1;

	debug("eth_init: fec_probe(bd)\n");
	lout = fec_probe(bd);

	return lout;
}
