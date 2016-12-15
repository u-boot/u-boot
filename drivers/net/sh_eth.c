/*
 * sh_eth.c - Driver for Renesas ethernet controller.
 *
 * Copyright (C) 2008, 2011 Renesas Solutions Corp.
 * Copyright (c) 2008, 2011, 2014 2014 Nobuhiro Iwamatsu
 * Copyright (c) 2007 Carlos Munoz <carlos@kenati.com>
 * Copyright (C) 2013, 2014 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include <asm/errno.h>
#include <asm/io.h>

#include "sh_eth.h"

#ifndef CONFIG_SH_ETHER_USE_PORT
# error "Please define CONFIG_SH_ETHER_USE_PORT"
#endif
#ifndef CONFIG_SH_ETHER_PHY_ADDR
# error "Please define CONFIG_SH_ETHER_PHY_ADDR"
#endif

#if defined(CONFIG_SH_ETHER_CACHE_WRITEBACK) && !defined(CONFIG_SYS_DCACHE_OFF)
#define flush_cache_wback(addr, len)    \
		flush_dcache_range((u32)addr, (u32)(addr + len - 1))
#else
#define flush_cache_wback(...)
#endif

#if defined(CONFIG_SH_ETHER_CACHE_INVALIDATE) && defined(CONFIG_ARM)
#define invalidate_cache(addr, len)		\
	{	\
		u32 line_size = CONFIG_SH_ETHER_ALIGNE_SIZE;	\
		u32 start, end;	\
		\
		start = (u32)addr;	\
		end = start + len;	\
		start &= ~(line_size - 1);	\
		end = ((end + line_size - 1) & ~(line_size - 1));	\
		\
		invalidate_dcache_range(start, end);	\
	}
#else
#define invalidate_cache(...)
#endif

#define TIMEOUT_CNT 1000

int sh_eth_send(struct eth_device *dev, void *packet, int len)
{
	struct sh_eth_dev *eth = dev->priv;
	int port = eth->port, ret = 0, timeout;
	struct sh_eth_info *port_info = &eth->port_info[port];

	if (!packet || len > 0xffff) {
		printf(SHETHER_NAME ": %s: Invalid argument\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	/* packet must be a 4 byte boundary */
	if ((int)packet & 3) {
		printf(SHETHER_NAME ": %s: packet not 4 byte alligned\n"
				, __func__);
		ret = -EFAULT;
		goto err;
	}

	/* Update tx descriptor */
	flush_cache_wback(packet, len);
	port_info->tx_desc_cur->td2 = ADDR_TO_PHY(packet);
	port_info->tx_desc_cur->td1 = len << 16;
	/* Must preserve the end of descriptor list indication */
	if (port_info->tx_desc_cur->td0 & TD_TDLE)
		port_info->tx_desc_cur->td0 = TD_TACT | TD_TFP | TD_TDLE;
	else
		port_info->tx_desc_cur->td0 = TD_TACT | TD_TFP;

	flush_cache_wback(port_info->tx_desc_cur, sizeof(struct tx_desc_s));

	/* Restart the transmitter if disabled */
	if (!(sh_eth_read(eth, EDTRR) & EDTRR_TRNS))
		sh_eth_write(eth, EDTRR_TRNS, EDTRR);

	/* Wait until packet is transmitted */
	timeout = TIMEOUT_CNT;
	do {
		invalidate_cache(port_info->tx_desc_cur,
				 sizeof(struct tx_desc_s));
		udelay(100);
	} while (port_info->tx_desc_cur->td0 & TD_TACT && timeout--);

	if (timeout < 0) {
		printf(SHETHER_NAME ": transmit timeout\n");
		ret = -ETIMEDOUT;
		goto err;
	}

	port_info->tx_desc_cur++;
	if (port_info->tx_desc_cur >= port_info->tx_desc_base + NUM_TX_DESC)
		port_info->tx_desc_cur = port_info->tx_desc_base;

err:
	return ret;
}

int sh_eth_recv(struct eth_device *dev)
{
	struct sh_eth_dev *eth = dev->priv;
	int port = eth->port, len = 0;
	struct sh_eth_info *port_info = &eth->port_info[port];
	uchar *packet;

	/* Check if the rx descriptor is ready */
	invalidate_cache(port_info->rx_desc_cur, sizeof(struct rx_desc_s));
	if (!(port_info->rx_desc_cur->rd0 & RD_RACT)) {
		/* Check for errors */
		if (!(port_info->rx_desc_cur->rd0 & RD_RFE)) {
			len = port_info->rx_desc_cur->rd1 & 0xffff;
			packet = (uchar *)
				ADDR_TO_P2(port_info->rx_desc_cur->rd2);
			invalidate_cache(packet, len);
			net_process_received_packet(packet, len);
		}

		/* Make current descriptor available again */
		if (port_info->rx_desc_cur->rd0 & RD_RDLE)
			port_info->rx_desc_cur->rd0 = RD_RACT | RD_RDLE;
		else
			port_info->rx_desc_cur->rd0 = RD_RACT;

		flush_cache_wback(port_info->rx_desc_cur,
				  sizeof(struct rx_desc_s));

		/* Point to the next descriptor */
		port_info->rx_desc_cur++;
		if (port_info->rx_desc_cur >=
		    port_info->rx_desc_base + NUM_RX_DESC)
			port_info->rx_desc_cur = port_info->rx_desc_base;
	}

	/* Restart the receiver if disabled */
	if (!(sh_eth_read(eth, EDRRR) & EDRRR_R))
		sh_eth_write(eth, EDRRR_R, EDRRR);

	return len;
}

static int sh_eth_reset(struct sh_eth_dev *eth)
{
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	int ret = 0, i;

	/* Start e-dmac transmitter and receiver */
	sh_eth_write(eth, EDSR_ENALL, EDSR);

	/* Perform a software reset and wait for it to complete */
	sh_eth_write(eth, EDMR_SRST, EDMR);
	for (i = 0; i < TIMEOUT_CNT; i++) {
		if (!(sh_eth_read(eth, EDMR) & EDMR_SRST))
			break;
		udelay(1000);
	}

	if (i == TIMEOUT_CNT) {
		printf(SHETHER_NAME  ": Software reset timeout\n");
		ret = -EIO;
	}

	return ret;
#else
	sh_eth_write(eth, sh_eth_read(eth, EDMR) | EDMR_SRST, EDMR);
	udelay(3000);
	sh_eth_write(eth, sh_eth_read(eth, EDMR) & ~EDMR_SRST, EDMR);

	return 0;
#endif
}

static int sh_eth_tx_desc_init(struct sh_eth_dev *eth)
{
	int port = eth->port, i, ret = 0;
	u32 alloc_desc_size = NUM_TX_DESC * sizeof(struct tx_desc_s);
	struct sh_eth_info *port_info = &eth->port_info[port];
	struct tx_desc_s *cur_tx_desc;

	/*
	 * Allocate rx descriptors. They must be aligned to size of struct
	 * tx_desc_s.
	 */
	port_info->tx_desc_alloc =
		memalign(sizeof(struct tx_desc_s), alloc_desc_size);
	if (!port_info->tx_desc_alloc) {
		printf(SHETHER_NAME ": memalign failed\n");
		ret = -ENOMEM;
		goto err;
	}

	flush_cache_wback((u32)port_info->tx_desc_alloc, alloc_desc_size);

	/* Make sure we use a P2 address (non-cacheable) */
	port_info->tx_desc_base =
		(struct tx_desc_s *)ADDR_TO_P2((u32)port_info->tx_desc_alloc);
	port_info->tx_desc_cur = port_info->tx_desc_base;

	/* Initialize all descriptors */
	for (cur_tx_desc = port_info->tx_desc_base, i = 0; i < NUM_TX_DESC;
	     cur_tx_desc++, i++) {
		cur_tx_desc->td0 = 0x00;
		cur_tx_desc->td1 = 0x00;
		cur_tx_desc->td2 = 0x00;
	}

	/* Mark the end of the descriptors */
	cur_tx_desc--;
	cur_tx_desc->td0 |= TD_TDLE;

	/* Point the controller to the tx descriptor list. Must use physical
	   addresses */
	sh_eth_write(eth, ADDR_TO_PHY(port_info->tx_desc_base), TDLAR);
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	sh_eth_write(eth, ADDR_TO_PHY(port_info->tx_desc_base), TDFAR);
	sh_eth_write(eth, ADDR_TO_PHY(cur_tx_desc), TDFXR);
	sh_eth_write(eth, 0x01, TDFFR);/* Last discriptor bit */
#endif

err:
	return ret;
}

static int sh_eth_rx_desc_init(struct sh_eth_dev *eth)
{
	int port = eth->port, i , ret = 0;
	u32 alloc_desc_size = NUM_RX_DESC * sizeof(struct rx_desc_s);
	struct sh_eth_info *port_info = &eth->port_info[port];
	struct rx_desc_s *cur_rx_desc;
	u8 *rx_buf;

	/*
	 * Allocate rx descriptors. They must be aligned to size of struct
	 * rx_desc_s.
	 */
	port_info->rx_desc_alloc =
		memalign(sizeof(struct rx_desc_s), alloc_desc_size);
	if (!port_info->rx_desc_alloc) {
		printf(SHETHER_NAME ": memalign failed\n");
		ret = -ENOMEM;
		goto err;
	}

	flush_cache_wback(port_info->rx_desc_alloc, alloc_desc_size);

	/* Make sure we use a P2 address (non-cacheable) */
	port_info->rx_desc_base =
		(struct rx_desc_s *)ADDR_TO_P2((u32)port_info->rx_desc_alloc);

	port_info->rx_desc_cur = port_info->rx_desc_base;

	/*
	 * Allocate rx data buffers. They must be RX_BUF_ALIGNE_SIZE bytes
	 * aligned and in P2 area.
	 */
	port_info->rx_buf_alloc =
		memalign(RX_BUF_ALIGNE_SIZE, NUM_RX_DESC * MAX_BUF_SIZE);
	if (!port_info->rx_buf_alloc) {
		printf(SHETHER_NAME ": alloc failed\n");
		ret = -ENOMEM;
		goto err_buf_alloc;
	}

	port_info->rx_buf_base = (u8 *)ADDR_TO_P2((u32)port_info->rx_buf_alloc);

	/* Initialize all descriptors */
	for (cur_rx_desc = port_info->rx_desc_base,
	     rx_buf = port_info->rx_buf_base, i = 0;
	     i < NUM_RX_DESC; cur_rx_desc++, rx_buf += MAX_BUF_SIZE, i++) {
		cur_rx_desc->rd0 = RD_RACT;
		cur_rx_desc->rd1 = MAX_BUF_SIZE << 16;
		cur_rx_desc->rd2 = (u32) ADDR_TO_PHY(rx_buf);
	}

	/* Mark the end of the descriptors */
	cur_rx_desc--;
	cur_rx_desc->rd0 |= RD_RDLE;

	/* Point the controller to the rx descriptor list */
	sh_eth_write(eth, ADDR_TO_PHY(port_info->rx_desc_base), RDLAR);
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	sh_eth_write(eth, ADDR_TO_PHY(port_info->rx_desc_base), RDFAR);
	sh_eth_write(eth, ADDR_TO_PHY(cur_rx_desc), RDFXR);
	sh_eth_write(eth, RDFFR_RDLF, RDFFR);
#endif

	return ret;

err_buf_alloc:
	free(port_info->rx_desc_alloc);
	port_info->rx_desc_alloc = NULL;

err:
	return ret;
}

static void sh_eth_tx_desc_free(struct sh_eth_dev *eth)
{
	int port = eth->port;
	struct sh_eth_info *port_info = &eth->port_info[port];

	if (port_info->tx_desc_alloc) {
		free(port_info->tx_desc_alloc);
		port_info->tx_desc_alloc = NULL;
	}
}

static void sh_eth_rx_desc_free(struct sh_eth_dev *eth)
{
	int port = eth->port;
	struct sh_eth_info *port_info = &eth->port_info[port];

	if (port_info->rx_desc_alloc) {
		free(port_info->rx_desc_alloc);
		port_info->rx_desc_alloc = NULL;
	}

	if (port_info->rx_buf_alloc) {
		free(port_info->rx_buf_alloc);
		port_info->rx_buf_alloc = NULL;
	}
}

static int sh_eth_desc_init(struct sh_eth_dev *eth)
{
	int ret = 0;

	ret = sh_eth_tx_desc_init(eth);
	if (ret)
		goto err_tx_init;

	ret = sh_eth_rx_desc_init(eth);
	if (ret)
		goto err_rx_init;

	return ret;
err_rx_init:
	sh_eth_tx_desc_free(eth);

err_tx_init:
	return ret;
}

static int sh_eth_phy_config(struct sh_eth_dev *eth)
{
	int port = eth->port, ret = 0;
	struct sh_eth_info *port_info = &eth->port_info[port];
	struct eth_device *dev = port_info->dev;
	struct phy_device *phydev;

	phydev = phy_connect(
			miiphy_get_dev_by_name(dev->name),
			port_info->phy_addr, dev, CONFIG_SH_ETHER_PHY_MODE);
	port_info->phydev = phydev;
	phy_config(phydev);

	return ret;
}

static int sh_eth_config(struct sh_eth_dev *eth, bd_t *bd)
{
	int port = eth->port, ret = 0;
	u32 val;
	struct sh_eth_info *port_info = &eth->port_info[port];
	struct eth_device *dev = port_info->dev;
	struct phy_device *phy;

	/* Configure e-dmac registers */
	sh_eth_write(eth, (sh_eth_read(eth, EDMR) & ~EMDR_DESC_R) |
			(EMDR_DESC | EDMR_EL), EDMR);

	sh_eth_write(eth, 0, EESIPR);
	sh_eth_write(eth, 0, TRSCER);
	sh_eth_write(eth, 0, TFTR);
	sh_eth_write(eth, (FIFO_SIZE_T | FIFO_SIZE_R), FDR);
	sh_eth_write(eth, RMCR_RST, RMCR);
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	sh_eth_write(eth, 0, RPADIR);
#endif
	sh_eth_write(eth, (FIFO_F_D_RFF | FIFO_F_D_RFD), FCFTR);

	/* Configure e-mac registers */
	sh_eth_write(eth, 0, ECSIPR);

	/* Set Mac address */
	val = dev->enetaddr[0] << 24 | dev->enetaddr[1] << 16 |
	    dev->enetaddr[2] << 8 | dev->enetaddr[3];
	sh_eth_write(eth, val, MAHR);

	val = dev->enetaddr[4] << 8 | dev->enetaddr[5];
	sh_eth_write(eth, val, MALR);

	sh_eth_write(eth, RFLR_RFL_MIN, RFLR);
#if defined(SH_ETH_TYPE_GETHER)
	sh_eth_write(eth, 0, PIPR);
#endif
#if defined(SH_ETH_TYPE_GETHER) || defined(SH_ETH_TYPE_RZ)
	sh_eth_write(eth, APR_AP, APR);
	sh_eth_write(eth, MPR_MP, MPR);
	sh_eth_write(eth, TPAUSER_TPAUSE, TPAUSER);
#endif

#if defined(CONFIG_CPU_SH7734) || defined(CONFIG_R8A7740)
	sh_eth_write(eth, CONFIG_SH_ETHER_SH7734_MII, RMII_MII);
#elif defined(CONFIG_R8A7790) || defined(CONFIG_R8A7791) || \
	defined(CONFIG_R8A7793) || defined(CONFIG_R8A7794)
	sh_eth_write(eth, sh_eth_read(eth, RMIIMR) | 0x1, RMIIMR);
#endif
	/* Configure phy */
	ret = sh_eth_phy_config(eth);
	if (ret) {
		printf(SHETHER_NAME ": phy config timeout\n");
		goto err_phy_cfg;
	}
	phy = port_info->phydev;
	ret = phy_startup(phy);
	if (ret) {
		printf(SHETHER_NAME ": phy startup failure\n");
		return ret;
	}

	val = 0;

	/* Set the transfer speed */
	if (phy->speed == 100) {
		printf(SHETHER_NAME ": 100Base/");
#if defined(SH_ETH_TYPE_GETHER)
		sh_eth_write(eth, GECMR_100B, GECMR);
#elif defined(CONFIG_CPU_SH7757) || defined(CONFIG_CPU_SH7752)
		sh_eth_write(eth, 1, RTRATE);
#elif defined(CONFIG_CPU_SH7724) || defined(CONFIG_R8A7790) || \
		defined(CONFIG_R8A7791) || defined(CONFIG_R8A7793) || \
		defined(CONFIG_R8A7794)
		val = ECMR_RTM;
#endif
	} else if (phy->speed == 10) {
		printf(SHETHER_NAME ": 10Base/");
#if defined(SH_ETH_TYPE_GETHER)
		sh_eth_write(eth, GECMR_10B, GECMR);
#elif defined(CONFIG_CPU_SH7757) || defined(CONFIG_CPU_SH7752)
		sh_eth_write(eth, 0, RTRATE);
#endif
	}
#if defined(SH_ETH_TYPE_GETHER)
	else if (phy->speed == 1000) {
		printf(SHETHER_NAME ": 1000Base/");
		sh_eth_write(eth, GECMR_1000B, GECMR);
	}
#endif

	/* Check if full duplex mode is supported by the phy */
	if (phy->duplex) {
		printf("Full\n");
		sh_eth_write(eth, val | (ECMR_CHG_DM|ECMR_RE|ECMR_TE|ECMR_DM),
			     ECMR);
	} else {
		printf("Half\n");
		sh_eth_write(eth, val | (ECMR_CHG_DM|ECMR_RE|ECMR_TE), ECMR);
	}

	return ret;

err_phy_cfg:
	return ret;
}

static void sh_eth_start(struct sh_eth_dev *eth)
{
	/*
	 * Enable the e-dmac receiver only. The transmitter will be enabled when
	 * we have something to transmit
	 */
	sh_eth_write(eth, EDRRR_R, EDRRR);
}

static void sh_eth_stop(struct sh_eth_dev *eth)
{
	sh_eth_write(eth, ~EDRRR_R, EDRRR);
}

int sh_eth_init(struct eth_device *dev, bd_t *bd)
{
	int ret = 0;
	struct sh_eth_dev *eth = dev->priv;

	ret = sh_eth_reset(eth);
	if (ret)
		goto err;

	ret = sh_eth_desc_init(eth);
	if (ret)
		goto err;

	ret = sh_eth_config(eth, bd);
	if (ret)
		goto err_config;

	sh_eth_start(eth);

	return ret;

err_config:
	sh_eth_tx_desc_free(eth);
	sh_eth_rx_desc_free(eth);

err:
	return ret;
}

void sh_eth_halt(struct eth_device *dev)
{
	struct sh_eth_dev *eth = dev->priv;
	sh_eth_stop(eth);
}

int sh_eth_initialize(bd_t *bd)
{
	int ret = 0;
	struct sh_eth_dev *eth = NULL;
	struct eth_device *dev = NULL;

	eth = (struct sh_eth_dev *)malloc(sizeof(struct sh_eth_dev));
	if (!eth) {
		printf(SHETHER_NAME ": %s: malloc failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (!dev) {
		printf(SHETHER_NAME ": %s: malloc failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	memset(dev, 0, sizeof(struct eth_device));
	memset(eth, 0, sizeof(struct sh_eth_dev));

	eth->port = CONFIG_SH_ETHER_USE_PORT;
	eth->port_info[eth->port].phy_addr = CONFIG_SH_ETHER_PHY_ADDR;

	dev->priv = (void *)eth;
	dev->iobase = 0;
	dev->init = sh_eth_init;
	dev->halt = sh_eth_halt;
	dev->send = sh_eth_send;
	dev->recv = sh_eth_recv;
	eth->port_info[eth->port].dev = dev;

	strcpy(dev->name, SHETHER_NAME);

	/* Register Device to EtherNet subsystem  */
	eth_register(dev);

	bb_miiphy_buses[0].priv = eth;
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = bb_miiphy_read;
	mdiodev->write = bb_miiphy_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;

	if (!eth_getenv_enetaddr("ethaddr", dev->enetaddr))
		puts("Please set MAC address\n");

	return ret;

err:
	if (dev)
		free(dev);

	if (eth)
		free(eth);

	printf(SHETHER_NAME ": Failed\n");
	return ret;
}

/******* for bb_miiphy *******/
static int sh_eth_bb_init(struct bb_miiphy_bus *bus)
{
	return 0;
}

static int sh_eth_bb_mdio_active(struct bb_miiphy_bus *bus)
{
	struct sh_eth_dev *eth = bus->priv;

	sh_eth_write(eth, sh_eth_read(eth, PIR) | PIR_MMD, PIR);

	return 0;
}

static int sh_eth_bb_mdio_tristate(struct bb_miiphy_bus *bus)
{
	struct sh_eth_dev *eth = bus->priv;

	sh_eth_write(eth, sh_eth_read(eth, PIR) & ~PIR_MMD, PIR);

	return 0;
}

static int sh_eth_bb_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	struct sh_eth_dev *eth = bus->priv;

	if (v)
		sh_eth_write(eth, sh_eth_read(eth, PIR) | PIR_MDO, PIR);
	else
		sh_eth_write(eth, sh_eth_read(eth, PIR) & ~PIR_MDO, PIR);

	return 0;
}

static int sh_eth_bb_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	struct sh_eth_dev *eth = bus->priv;

	*v = (sh_eth_read(eth, PIR) & PIR_MDI) >> 3;

	return 0;
}

static int sh_eth_bb_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	struct sh_eth_dev *eth = bus->priv;

	if (v)
		sh_eth_write(eth, sh_eth_read(eth, PIR) | PIR_MDC, PIR);
	else
		sh_eth_write(eth, sh_eth_read(eth, PIR) & ~PIR_MDC, PIR);

	return 0;
}

static int sh_eth_bb_delay(struct bb_miiphy_bus *bus)
{
	udelay(10);

	return 0;
}

struct bb_miiphy_bus bb_miiphy_buses[] = {
	{
		.name		= "sh_eth",
		.init		= sh_eth_bb_init,
		.mdio_active	= sh_eth_bb_mdio_active,
		.mdio_tristate	= sh_eth_bb_mdio_tristate,
		.set_mdio	= sh_eth_bb_set_mdio,
		.get_mdio	= sh_eth_bb_get_mdio,
		.set_mdc	= sh_eth_bb_set_mdc,
		.delay		= sh_eth_bb_delay,
	}
};
int bb_miiphy_buses_num = ARRAY_SIZE(bb_miiphy_buses);
