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

#ifndef CONFIG_FEC_XCV_TYPE
#define CONFIG_FEC_XCV_TYPE MII100
#endif

/*
 * The i.MX28 operates with packets in big endian. We need to swap them before
 * sending and after receiving.
 */
#ifdef CONFIG_MX28
#define CONFIG_FEC_MXC_SWAP_PACKET
#endif

#define RXDESC_PER_CACHELINE (ARCH_DMA_MINALIGN/sizeof(struct fec_bd))

/* Check various alignment issues at compile time */
#if ((ARCH_DMA_MINALIGN < 16) || (ARCH_DMA_MINALIGN % 16 != 0))
#error "ARCH_DMA_MINALIGN must be multiple of 16!"
#endif

#if ((PKTALIGN < ARCH_DMA_MINALIGN) || \
	(PKTALIGN % ARCH_DMA_MINALIGN != 0))
#error "PKTALIGN must be multiple of ARCH_DMA_MINALIGN!"
#endif

#undef DEBUG

struct nbuf {
	uint8_t data[1500];	/**< actual data */
	int length;		/**< actual length */
	int used;		/**< buffer in use or not */
	uint8_t head[16];	/**< MAC header(6 + 6 + 2) + 2(aligned) */
};

#ifdef CONFIG_FEC_MXC_SWAP_PACKET
static void swap_packet(uint32_t *packet, int length)
{
	int i;

	for (i = 0; i < DIV_ROUND_UP(length, 4); i++)
		packet[i] = __swab32(packet[i]);
}
#endif

/*
 * MII-interface related functions
 */
static int fec_mdio_read(struct ethernet_regs *eth, uint8_t phyAddr,
		uint8_t regAddr)
{
	uint32_t reg;		/* convenient holder for the PHY register */
	uint32_t phy;		/* convenient holder for the PHY */
	uint32_t start;
	int val;

	/*
	 * reading from any PHY's register is done by properly
	 * programming the FEC's MII data register.
	 */
	writel(FEC_IEVENT_MII, &eth->ievent);
	reg = regAddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyAddr << FEC_MII_DATA_PA_SHIFT;

	writel(FEC_MII_DATA_ST | FEC_MII_DATA_OP_RD | FEC_MII_DATA_TA |
			phy | reg, &eth->mii_data);

	/*
	 * wait for the related interrupt
	 */
	start = get_timer(0);
	while (!(readl(&eth->ievent) & FEC_IEVENT_MII)) {
		if (get_timer(start) > (CONFIG_SYS_HZ / 1000)) {
			printf("Read MDIO failed...\n");
			return -1;
		}
	}

	/*
	 * clear mii interrupt bit
	 */
	writel(FEC_IEVENT_MII, &eth->ievent);

	/*
	 * it's now safe to read the PHY's register
	 */
	val = (unsigned short)readl(&eth->mii_data);
	debug("%s: phy: %02x reg:%02x val:%#x\n", __func__, phyAddr,
			regAddr, val);
	return val;
}

static void fec_mii_setspeed(struct fec_priv *fec)
{
	/*
	 * Set MII_SPEED = (1/(mii_speed * 2)) * System Clock
	 * and do not drop the Preamble.
	 */
	writel((((imx_get_fecclk() / 1000000) + 2) / 5) << 1,
			&fec->eth->mii_speed);
	debug("%s: mii_speed %08x\n", __func__, readl(&fec->eth->mii_speed));
}

static int fec_mdio_write(struct ethernet_regs *eth, uint8_t phyAddr,
		uint8_t regAddr, uint16_t data)
{
	uint32_t reg;		/* convenient holder for the PHY register */
	uint32_t phy;		/* convenient holder for the PHY */
	uint32_t start;

	reg = regAddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyAddr << FEC_MII_DATA_PA_SHIFT;

	writel(FEC_MII_DATA_ST | FEC_MII_DATA_OP_WR |
		FEC_MII_DATA_TA | phy | reg | data, &eth->mii_data);

	/*
	 * wait for the MII interrupt
	 */
	start = get_timer(0);
	while (!(readl(&eth->ievent) & FEC_IEVENT_MII)) {
		if (get_timer(start) > (CONFIG_SYS_HZ / 1000)) {
			printf("Write MDIO failed...\n");
			return -1;
		}
	}

	/*
	 * clear MII interrupt bit
	 */
	writel(FEC_IEVENT_MII, &eth->ievent);
	debug("%s: phy: %02x reg:%02x val:%#x\n", __func__, phyAddr,
			regAddr, data);

	return 0;
}

int fec_phy_read(struct mii_dev *bus, int phyAddr, int dev_addr, int regAddr)
{
	return fec_mdio_read(bus->priv, phyAddr, regAddr);
}

int fec_phy_write(struct mii_dev *bus, int phyAddr, int dev_addr, int regAddr,
		u16 data)
{
	return fec_mdio_write(bus->priv, phyAddr, regAddr, data);
}

#ifndef CONFIG_PHYLIB
static int miiphy_restart_aneg(struct eth_device *dev)
{
	int ret = 0;
#if !defined(CONFIG_FEC_MXC_NO_ANEG)
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	struct ethernet_regs *eth = fec->bus->priv;

	/*
	 * Wake up from sleep if necessary
	 * Reset PHY, then delay 300ns
	 */
#ifdef CONFIG_MX27
	fec_mdio_write(eth, fec->phy_id, MII_DCOUNTER, 0x00FF);
#endif
	fec_mdio_write(eth, fec->phy_id, MII_BMCR, BMCR_RESET);
	udelay(1000);

	/*
	 * Set the auto-negotiation advertisement register bits
	 */
	fec_mdio_write(eth, fec->phy_id, MII_ADVERTISE,
			LPA_100FULL | LPA_100HALF | LPA_10FULL |
			LPA_10HALF | PHY_ANLPAR_PSB_802_3);
	fec_mdio_write(eth, fec->phy_id, MII_BMCR,
			BMCR_ANENABLE | BMCR_ANRESTART);

	if (fec->mii_postcall)
		ret = fec->mii_postcall(fec->phy_id);

#endif
	return ret;
}

static int miiphy_wait_aneg(struct eth_device *dev)
{
	uint32_t start;
	int status;
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	struct ethernet_regs *eth = fec->bus->priv;

	/*
	 * Wait for AN completion
	 */
	start = get_timer(0);
	do {
		if (get_timer(start) > (CONFIG_SYS_HZ * 5)) {
			printf("%s: Autonegotiation timeout\n", dev->name);
			return -1;
		}

		status = fec_mdio_read(eth, fec->phy_id, MII_BMSR);
		if (status < 0) {
			printf("%s: Autonegotiation failed. status: %d\n",
					dev->name, status);
			return -1;
		}
	} while (!(status & BMSR_LSTATUS));

	return 0;
}
#endif

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
 * @param[in] dsize desired size of each receive buffer
 * @return 0 on success
 *
 * For this task we need additional memory for the data buffers. And each
 * data buffer requires some alignment. Thy must be aligned to a specific
 * boundary each.
 */
static int fec_rbd_init(struct fec_priv *fec, int count, int dsize)
{
	uint32_t size;
	int i;

	/*
	 * Allocate memory for the buffers. This allocation respects the
	 * alignment
	 */
	size = roundup(dsize, ARCH_DMA_MINALIGN);
	for (i = 0; i < count; i++) {
		uint32_t data_ptr = readl(&fec->rbd_base[i].data_pointer);
		if (data_ptr == 0) {
			uint8_t *data = memalign(ARCH_DMA_MINALIGN,
						 size);
			if (!data) {
				printf("%s: error allocating rxbuf %d\n",
				       __func__, i);
				goto err;
			}
			writel((uint32_t)data, &fec->rbd_base[i].data_pointer);
		} /* needs allocation */
		writew(FEC_RBD_EMPTY, &fec->rbd_base[i].status);
		writew(0, &fec->rbd_base[i].data_length);
	}

	/* Mark the last RBD to close the ring. */
	writew(FEC_RBD_WRAP | FEC_RBD_EMPTY, &fec->rbd_base[i - 1].status);
	fec->rbd_index = 0;

	return 0;

err:
	for (; i >= 0; i--) {
		uint32_t data_ptr = readl(&fec->rbd_base[i].data_pointer);
		free((void *)data_ptr);
	}

	return -ENOMEM;
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
	unsigned addr = (unsigned)fec->tbd_base;
	unsigned size = roundup(2 * sizeof(struct fec_bd),
				ARCH_DMA_MINALIGN);
	writew(0x0000, &fec->tbd_base[0].status);
	writew(FEC_TBD_WRAP, &fec->tbd_base[1].status);
	fec->tbd_index = 0;
	flush_dcache_range(addr, addr+size);
}

/**
 * Mark the given read buffer descriptor as free
 * @param[in] last 1 if this is the last buffer descriptor in the chain, else 0
 * @param[in] pRbd buffer descriptor to mark free again
 */
static void fec_rbd_clean(int last, struct fec_bd *pRbd)
{
	unsigned short flags = FEC_RBD_EMPTY;
	if (last)
		flags |= FEC_RBD_WRAP;
	writew(flags, &pRbd->status);
	writew(0, &pRbd->data_length);
}

static int fec_get_hwaddr(struct eth_device *dev, int dev_id,
						unsigned char *mac)
{
	imx_get_mac_from_fuse(dev_id, mac);
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

static void fec_eth_phy_config(struct eth_device *dev)
{
#ifdef CONFIG_PHYLIB
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	struct phy_device *phydev;

	phydev = phy_connect(fec->bus, fec->phy_id, dev,
			PHY_INTERFACE_MODE_RGMII);
	if (phydev) {
		fec->phydev = phydev;
		phy_config(phydev);
	}
#endif
}

/*
 * Do initial configuration of the FEC registers
 */
static void fec_reg_setup(struct fec_priv *fec)
{
	uint32_t rcntrl;

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

	/* Start with frame length = 1518, common for all modes. */
	rcntrl = PKTSIZE << FEC_RCNTRL_MAX_FL_SHIFT;
	if (fec->xcv_type == SEVENWIRE)
		rcntrl |= FEC_RCNTRL_FCE;
	else if (fec->xcv_type == RGMII)
		rcntrl |= FEC_RCNTRL_RGMII;
	else if (fec->xcv_type == RMII)
		rcntrl |= FEC_RCNTRL_RMII;
	else	/* MII mode */
		rcntrl |= FEC_RCNTRL_FCE | FEC_RCNTRL_MII_MODE;

	writel(rcntrl, &fec->eth->r_cntrl);
}

/**
 * Start the FEC engine
 * @param[in] dev Our device to handle
 */
static int fec_open(struct eth_device *edev)
{
	struct fec_priv *fec = (struct fec_priv *)edev->priv;
	int speed;
	uint32_t addr, size;
	int i;

	debug("fec_open: fec_open(dev)\n");
	/* full-duplex, heartbeat disabled */
	writel(1 << 2, &fec->eth->x_cntrl);
	fec->rbd_index = 0;

	/* Invalidate all descriptors */
	for (i = 0; i < FEC_RBD_NUM - 1; i++)
		fec_rbd_clean(0, &fec->rbd_base[i]);
	fec_rbd_clean(1, &fec->rbd_base[i]);

	/* Flush the descriptors into RAM */
	size = roundup(FEC_RBD_NUM * sizeof(struct fec_bd),
			ARCH_DMA_MINALIGN);
	addr = (uint32_t)fec->rbd_base;
	flush_dcache_range(addr, addr + size);

#ifdef FEC_QUIRK_ENET_MAC
	/* Enable ENET HW endian SWAP */
	writel(readl(&fec->eth->ecntrl) | FEC_ECNTRL_DBSWAP,
		&fec->eth->ecntrl);
	/* Enable ENET store and forward mode */
	writel(readl(&fec->eth->x_wmrk) | FEC_X_WMRK_STRFWD,
		&fec->eth->x_wmrk);
#endif
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

#ifdef CONFIG_PHYLIB
	if (!fec->phydev)
		fec_eth_phy_config(edev);
	if (fec->phydev) {
		/* Start up the PHY */
		phy_startup(fec->phydev);
		speed = fec->phydev->speed;
	} else {
		speed = _100BASET;
	}
#else
	miiphy_wait_aneg(edev);
	speed = miiphy_speed(edev->name, fec->phy_id);
	miiphy_duplex(edev->name, fec->phy_id);
#endif

#ifdef FEC_QUIRK_ENET_MAC
	{
		u32 ecr = readl(&fec->eth->ecntrl) & ~FEC_ECNTRL_SPEED;
		u32 rcr = (readl(&fec->eth->r_cntrl) &
				~(FEC_RCNTRL_RMII | FEC_RCNTRL_RMII_10T)) |
				FEC_RCNTRL_RGMII | FEC_RCNTRL_MII_MODE;
		if (speed == _1000BASET)
			ecr |= FEC_ECNTRL_SPEED;
		else if (speed != _100BASET)
			rcr |= FEC_RCNTRL_RMII_10T;
		writel(ecr, &fec->eth->ecntrl);
		writel(rcr, &fec->eth->r_cntrl);
	}
#endif
	debug("%s:Speed=%i\n", __func__, speed);

	/*
	 * Enable SmartDMA receive task
	 */
	fec_rx_task_enable(fec);

	udelay(100000);
	return 0;
}

static int fec_init(struct eth_device *dev, bd_t* bd)
{
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	uint32_t mib_ptr = (uint32_t)&fec->eth->rmon_t_drop;
	uint32_t size;
	int i, ret;

	/* Initialize MAC address */
	fec_set_hwaddr(dev);

	/*
	 * Allocate transmit descriptors, there are two in total. This
	 * allocation respects cache alignment.
	 */
	if (!fec->tbd_base) {
		size = roundup(2 * sizeof(struct fec_bd),
				ARCH_DMA_MINALIGN);
		fec->tbd_base = memalign(ARCH_DMA_MINALIGN, size);
		if (!fec->tbd_base) {
			ret = -ENOMEM;
			goto err1;
		}
		memset(fec->tbd_base, 0, size);
		fec_tbd_init(fec);
		flush_dcache_range((unsigned)fec->tbd_base, size);
	}

	/*
	 * Allocate receive descriptors. This allocation respects cache
	 * alignment.
	 */
	if (!fec->rbd_base) {
		size = roundup(FEC_RBD_NUM * sizeof(struct fec_bd),
				ARCH_DMA_MINALIGN);
		fec->rbd_base = memalign(ARCH_DMA_MINALIGN, size);
		if (!fec->rbd_base) {
			ret = -ENOMEM;
			goto err2;
		}
		memset(fec->rbd_base, 0, size);
		/*
		 * Initialize RxBD ring
		 */
		if (fec_rbd_init(fec, FEC_RBD_NUM, FEC_MAX_PKT_SIZE) < 0) {
			ret = -ENOMEM;
			goto err3;
		}
		flush_dcache_range((unsigned)fec->rbd_base,
				   (unsigned)fec->rbd_base + size);
	}

	fec_reg_setup(fec);

	if (fec->xcv_type == MII10 || fec->xcv_type == MII100)
		fec_mii_setspeed(fec);

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
	for (i = mib_ptr; i <= mib_ptr + 0xfc; i += 4)
		writel(0, i);

	/* FIFO receive start register */
	writel(0x520, &fec->eth->r_fstart);

	/* size and address of each buffer */
	writel(FEC_MAX_PKT_SIZE, &fec->eth->emrbr);
	writel((uint32_t)fec->tbd_base, &fec->eth->etdsr);
	writel((uint32_t)fec->rbd_base, &fec->eth->erdsr);

#ifndef CONFIG_PHYLIB
	if (fec->xcv_type != SEVENWIRE)
		miiphy_restart_aneg(dev);
#endif
	fec_open(dev);
	return 0;

err3:
	free(fec->rbd_base);
err2:
	free(fec->tbd_base);
err1:
	return ret;
}

/**
 * Halt the FEC engine
 * @param[in] dev Our device to handle
 */
static void fec_halt(struct eth_device *dev)
{
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
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
static int fec_send(struct eth_device *dev, void *packet, int length)
{
	unsigned int status;
	uint32_t size;
	uint32_t addr;

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
	 * Setup the transmit buffer. We are always using the first buffer for
	 * transmission, the second will be empty and only used to stop the DMA
	 * engine. We also flush the packet to RAM here to avoid cache trouble.
	 */
#ifdef CONFIG_FEC_MXC_SWAP_PACKET
	swap_packet((uint32_t *)packet, length);
#endif

	addr = (uint32_t)packet;
	size = roundup(length, ARCH_DMA_MINALIGN);
	flush_dcache_range(addr, addr + size);

	writew(length, &fec->tbd_base[fec->tbd_index].data_length);
	writel(addr, &fec->tbd_base[fec->tbd_index].data_pointer);

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
	 * Flush data cache. This code flushes both TX descriptors to RAM.
	 * After this code, the descriptors will be safely in RAM and we
	 * can start DMA.
	 */
	size = roundup(2 * sizeof(struct fec_bd), ARCH_DMA_MINALIGN);
	addr = (uint32_t)fec->tbd_base;
	flush_dcache_range(addr, addr + size);

	/*
	 * Enable SmartDMA transmit task
	 */
	fec_tx_task_enable(fec);

	/*
	 * Wait until frame is sent. On each turn of the wait cycle, we must
	 * invalidate data cache to see what's really in RAM. Also, we need
	 * barrier here.
	 */
	invalidate_dcache_range(addr, addr + size);
	while (readw(&fec->tbd_base[fec->tbd_index].status) & FEC_TBD_READY) {
		udelay(1);
		invalidate_dcache_range(addr, addr + size);
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
	uint32_t addr, size;
	int i;
	uchar buff[FEC_MAX_PKT_SIZE];

	/*
	 * Check if any critical events have happened
	 */
	ievent = readl(&fec->eth->ievent);
	writel(ievent, &fec->eth->ievent);
	debug("fec_recv: ievent 0x%lx\n", ievent);
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
	 * Read the buffer status. Before the status can be read, the data cache
	 * must be invalidated, because the data in RAM might have been changed
	 * by DMA. The descriptors are properly aligned to cachelines so there's
	 * no need to worry they'd overlap.
	 *
	 * WARNING: By invalidating the descriptor here, we also invalidate
	 * the descriptors surrounding this one. Therefore we can NOT change the
	 * contents of this descriptor nor the surrounding ones. The problem is
	 * that in order to mark the descriptor as processed, we need to change
	 * the descriptor. The solution is to mark the whole cache line when all
	 * descriptors in the cache line are processed.
	 */
	addr = (uint32_t)rbd;
	addr &= ~(ARCH_DMA_MINALIGN - 1);
	size = roundup(sizeof(struct fec_bd), ARCH_DMA_MINALIGN);
	invalidate_dcache_range(addr, addr + size);

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
			 * Invalidate data cache over the buffer
			 */
			addr = (uint32_t)frame;
			size = roundup(frame_length, ARCH_DMA_MINALIGN);
			invalidate_dcache_range(addr, addr + size);

			/*
			 *  Fill the buffer and pass it to upper layers
			 */
#ifdef CONFIG_FEC_MXC_SWAP_PACKET
			swap_packet((uint32_t *)frame->data, frame_length);
#endif
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
		 * Free the current buffer, restart the engine and move forward
		 * to the next buffer. Here we check if the whole cacheline of
		 * descriptors was already processed and if so, we mark it free
		 * as whole.
		 */
		size = RXDESC_PER_CACHELINE - 1;
		if ((fec->rbd_index & size) == size) {
			i = fec->rbd_index - size;
			addr = (uint32_t)&fec->rbd_base[i];
			for (; i <= fec->rbd_index ; i++) {
				fec_rbd_clean(i == (FEC_RBD_NUM - 1),
					      &fec->rbd_base[i]);
			}
			flush_dcache_range(addr,
				addr + ARCH_DMA_MINALIGN);
		}

		fec_rx_task_enable(fec);
		fec->rbd_index = (fec->rbd_index + 1) % FEC_RBD_NUM;
	}
	debug("fec_recv: stop\n");

	return len;
}

static int fec_probe(bd_t *bd, int dev_id, int phy_id, uint32_t base_addr)
{
	struct eth_device *edev;
	struct fec_priv *fec;
	struct mii_dev *bus;
	unsigned char ethaddr[6];
	uint32_t start;
	int ret = 0;

	/* create and fill edev struct */
	edev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (!edev) {
		puts("fec_mxc: not enough malloc memory for eth_device\n");
		ret = -ENOMEM;
		goto err1;
	}

	fec = (struct fec_priv *)malloc(sizeof(struct fec_priv));
	if (!fec) {
		puts("fec_mxc: not enough malloc memory for fec_priv\n");
		ret = -ENOMEM;
		goto err2;
	}

	memset(edev, 0, sizeof(*edev));
	memset(fec, 0, sizeof(*fec));

	edev->priv = fec;
	edev->init = fec_init;
	edev->send = fec_send;
	edev->recv = fec_recv;
	edev->halt = fec_halt;
	edev->write_hwaddr = fec_set_hwaddr;

	fec->eth = (struct ethernet_regs *)base_addr;
	fec->bd = bd;

	fec->xcv_type = CONFIG_FEC_XCV_TYPE;

	/* Reset chip. */
	writel(readl(&fec->eth->ecntrl) | FEC_ECNTRL_RESET, &fec->eth->ecntrl);
	start = get_timer(0);
	while (readl(&fec->eth->ecntrl) & FEC_ECNTRL_RESET) {
		if (get_timer(start) > (CONFIG_SYS_HZ * 5)) {
			printf("FEC MXC: Timeout reseting chip\n");
			goto err3;
		}
		udelay(10);
	}

	fec_reg_setup(fec);
	fec_mii_setspeed(fec);

	if (dev_id == -1) {
		sprintf(edev->name, "FEC");
		fec->dev_id = 0;
	} else {
		sprintf(edev->name, "FEC%i", dev_id);
		fec->dev_id = dev_id;
	}
	fec->phy_id = phy_id;

	bus = mdio_alloc();
	if (!bus) {
		printf("mdio_alloc failed\n");
		ret = -ENOMEM;
		goto err3;
	}
	bus->read = fec_phy_read;
	bus->write = fec_phy_write;
	sprintf(bus->name, edev->name);
#ifdef CONFIG_MX28
	/*
	 * The i.MX28 has two ethernet interfaces, but they are not equal.
	 * Only the first one can access the MDIO bus.
	 */
	bus->priv = (struct ethernet_regs *)MXS_ENET0_BASE;
#else
	bus->priv = fec->eth;
#endif
	ret = mdio_register(bus);
	if (ret) {
		printf("mdio_register failed\n");
		free(bus);
		ret = -ENOMEM;
		goto err3;
	}
	fec->bus = bus;
	eth_register(edev);

	if (fec_get_hwaddr(edev, dev_id, ethaddr) == 0) {
		debug("got MAC%d address from fuse: %pM\n", dev_id, ethaddr);
		memcpy(edev->enetaddr, ethaddr, 6);
	}
	/* Configure phy */
	fec_eth_phy_config(edev);
	return ret;

err3:
	free(fec);
err2:
	free(edev);
err1:
	return ret;
}

#ifndef CONFIG_FEC_MXC_MULTI
int fecmxc_initialize(bd_t *bd)
{
	int lout = 1;

	debug("eth_init: fec_probe(bd)\n");
	lout = fec_probe(bd, -1, CONFIG_FEC_MXC_PHYADDR, IMX_FEC_BASE);

	return lout;
}
#endif

int fecmxc_initialize_multi(bd_t *bd, int dev_id, int phy_id, uint32_t addr)
{
	int lout = 1;

	debug("eth_init: fec_probe(bd, %i, %i) @ %08x\n", dev_id, phy_id, addr);
	lout = fec_probe(bd, dev_id, phy_id, addr);

	return lout;
}

#ifndef CONFIG_PHYLIB
int fecmxc_register_mii_postcall(struct eth_device *dev, int (*cb)(int))
{
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	fec->mii_postcall = cb;
	return 0;
}
#endif
