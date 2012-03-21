/*
 * sh_eth.c - Driver for Renesas SH7763's ethernet controler.
 *
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (c) 2008 Nobuhiro Iwamatsu
 * Copyright (c) 2007 Carlos Munoz <carlos@kenati.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <asm/errno.h>
#include <asm/io.h>

#include "sh_eth.h"

#ifndef CONFIG_SH_ETHER_USE_PORT
# error "Please define CONFIG_SH_ETHER_USE_PORT"
#endif
#ifndef CONFIG_SH_ETHER_PHY_ADDR
# error "Please define CONFIG_SH_ETHER_PHY_ADDR"
#endif
#ifdef CONFIG_SH_ETHER_CACHE_WRITEBACK
#define flush_cache_wback(addr, len)	\
			dcache_wback_range((u32)addr, (u32)(addr + len - 1))
#else
#define flush_cache_wback(...)
#endif

#define SH_ETH_PHY_DELAY 50000

/*
 * Bits are written to the PHY serially using the
 * PIR register, just like a bit banger.
 */
static void sh_eth_mii_write_phy_bits(int port, u32 val, int len)
{
	int i;
	u32 pir;

	/* Bit positions is 1 less than the number of bits */
	for (i = len - 1; i >= 0; i--) {
		/* Write direction, bit to write, clock is low */
		pir = 2 | ((val & 1 << i) ? 1 << 2 : 0);
		outl(pir, PIR(port));
		udelay(1);
		/* Write direction, bit to write, clock is high */
		pir = 3 | ((val & 1 << i) ? 1 << 2 : 0);
		outl(pir, PIR(port));
		udelay(1);
		/* Write direction, bit to write, clock is low */
		pir = 2 | ((val & 1 << i) ? 1 << 2 : 0);
		outl(pir, PIR(port));
		udelay(1);
	}
}

static void sh_eth_mii_bus_release(int port)
{
	/* Read direction, clock is low */
	outl(0, PIR(port));
	udelay(1);
	/* Read direction, clock is high */
	outl(1, PIR(port));
	udelay(1);
	/* Read direction, clock is low */
	outl(0, PIR(port));
	udelay(1);
}

static void sh_eth_mii_ind_bus_release(int port)
{
	/* Read direction, clock is low */
	outl(0, PIR(port));
	udelay(1);
}

static void sh_eth_mii_read_phy_bits(int port, u32 *val, int len)
{
	int i;
	u32 pir;

	*val = 0;
	for (i = len - 1; i >= 0; i--) {
		/* Read direction, clock is high */
		outl(1, PIR(port));
		udelay(1);
		/* Read bit */
		pir = inl(PIR(port));
		*val |= (pir & 8) ? 1 << i : 0;
		/* Read direction, clock is low */
		outl(0, PIR(port));
		udelay(1);
	}
}

#define PHY_INIT	0xFFFFFFFF
#define PHY_READ	0x02
#define PHY_WRITE	0x01
/*
 * To read a phy register, mii managements frames are sent to the phy.
 * The frames look like this:
 * pre (32 bits):	0xffff ffff
 * st (2 bits):		01
 * op (2bits):		10: read 01: write
 * phyad (5 bits):	xxxxx
 * regad (5 bits):	xxxxx
 * ta (Bus release):
 * data (16 bits):	read data
 */
static u32 sh_eth_mii_read_phy_reg(int port, u8 phy_addr, int reg)
{
	u32 val;

	/* Sent mii management frame */
	/* pre */
	sh_eth_mii_write_phy_bits(port, PHY_INIT, 32);
	/* st (start of frame) */
	sh_eth_mii_write_phy_bits(port, 0x1, 2);
	/* op (code) */
	sh_eth_mii_write_phy_bits(port, PHY_READ, 2);
	/* phy address */
	sh_eth_mii_write_phy_bits(port, phy_addr, 5);
	/* Register to read */
	sh_eth_mii_write_phy_bits(port, reg, 5);

	/* Bus release */
	sh_eth_mii_bus_release(port);

	/* Read register */
	sh_eth_mii_read_phy_bits(port, &val, 16);

	return val;
}

/*
 * To write a phy register, mii managements frames are sent to the phy.
 * The frames look like this:
 * pre (32 bits):	0xffff ffff
 * st (2 bits):		01
 * op (2bits):		10: read 01: write
 * phyad (5 bits):	xxxxx
 * regad (5 bits):	xxxxx
 * ta (2 bits):		10
 * data (16 bits):	write data
 * idle (Independent bus release)
 */
static void sh_eth_mii_write_phy_reg(int port, u8 phy_addr, int reg, u16 val)
{
	/* Sent mii management frame */
	/* pre */
	sh_eth_mii_write_phy_bits(port, PHY_INIT, 32);
	/* st (start of frame) */
	sh_eth_mii_write_phy_bits(port, 0x1, 2);
	/* op (code) */
	sh_eth_mii_write_phy_bits(port, PHY_WRITE, 2);
	/* phy address */
	sh_eth_mii_write_phy_bits(port, phy_addr, 5);
	/* Register to read */
	sh_eth_mii_write_phy_bits(port, reg, 5);
	/* ta */
	sh_eth_mii_write_phy_bits(port, PHY_READ, 2);
	/* Write register data */
	sh_eth_mii_write_phy_bits(port, val, 16);

	/* Independent bus release */
	sh_eth_mii_ind_bus_release(port);
}

int sh_eth_send(struct eth_device *dev, volatile void *packet, int len)
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
	if ((int)packet & (4 - 1)) {
		printf(SHETHER_NAME ": %s: packet not 4 byte alligned\n", __func__);
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

	/* Restart the transmitter if disabled */
	if (!(inl(EDTRR(port)) & EDTRR_TRNS))
		outl(EDTRR_TRNS, EDTRR(port));

	/* Wait until packet is transmitted */
	timeout = 1000;
	while (port_info->tx_desc_cur->td0 & TD_TACT && timeout--)
		udelay(100);

	if (timeout < 0) {
		printf(SHETHER_NAME ": transmit timeout\n");
		ret = -ETIMEDOUT;
		goto err;
	}

	port_info->tx_desc_cur++;
	if (port_info->tx_desc_cur >= port_info->tx_desc_base + NUM_TX_DESC)
		port_info->tx_desc_cur = port_info->tx_desc_base;

	return ret;
err:
	return ret;
}

int sh_eth_recv(struct eth_device *dev)
{
	struct sh_eth_dev *eth = dev->priv;
	int port = eth->port, len = 0;
	struct sh_eth_info *port_info = &eth->port_info[port];
	volatile u8 *packet;

	/* Check if the rx descriptor is ready */
	if (!(port_info->rx_desc_cur->rd0 & RD_RACT)) {
		/* Check for errors */
		if (!(port_info->rx_desc_cur->rd0 & RD_RFE)) {
			len = port_info->rx_desc_cur->rd1 & 0xffff;
			packet = (volatile u8 *)
			    ADDR_TO_P2(port_info->rx_desc_cur->rd2);
			NetReceive(packet, len);
		}

		/* Make current descriptor available again */
		if (port_info->rx_desc_cur->rd0 & RD_RDLE)
			port_info->rx_desc_cur->rd0 = RD_RACT | RD_RDLE;
		else
			port_info->rx_desc_cur->rd0 = RD_RACT;

		/* Point to the next descriptor */
		port_info->rx_desc_cur++;
		if (port_info->rx_desc_cur >=
		    port_info->rx_desc_base + NUM_RX_DESC)
			port_info->rx_desc_cur = port_info->rx_desc_base;
	}

	/* Restart the receiver if disabled */
	if (!(inl(EDRRR(port)) & EDRRR_R))
		outl(EDRRR_R, EDRRR(port));

	return len;
}

#define EDMR_INIT_CNT 1000
static int sh_eth_reset(struct sh_eth_dev *eth)
{
	int port = eth->port;
#if defined(CONFIG_CPU_SH7763)
	int ret = 0, i;

	/* Start e-dmac transmitter and receiver */
	outl(EDSR_ENALL, EDSR(port));

	/* Perform a software reset and wait for it to complete */
	outl(EDMR_SRST, EDMR(port));
	for (i = 0; i < EDMR_INIT_CNT; i++) {
		if (!(inl(EDMR(port)) & EDMR_SRST))
			break;
		udelay(1000);
	}

	if (i == EDMR_INIT_CNT) {
		printf(SHETHER_NAME  ": Software reset timeout\n");
		ret = -EIO;
	}

	return ret;
#else
	outl(inl(EDMR(port)) | EDMR_SRST, EDMR(port));
	udelay(3000);
	outl(inl(EDMR(port)) & ~EDMR_SRST, EDMR(port));

	return 0;
#endif
}

static int sh_eth_tx_desc_init(struct sh_eth_dev *eth)
{
	int port = eth->port, i, ret = 0;
	u32 tmp_addr;
	struct sh_eth_info *port_info = &eth->port_info[port];
	struct tx_desc_s *cur_tx_desc;

	/*
	 * Allocate tx descriptors. They must be TX_DESC_SIZE bytes aligned
	 */
	port_info->tx_desc_malloc = malloc(NUM_TX_DESC *
						 sizeof(struct tx_desc_s) +
						 TX_DESC_SIZE - 1);
	if (!port_info->tx_desc_malloc) {
		printf(SHETHER_NAME ": malloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	tmp_addr = (u32) (((int)port_info->tx_desc_malloc + TX_DESC_SIZE - 1) &
			  ~(TX_DESC_SIZE - 1));
	flush_cache_wback(tmp_addr, NUM_TX_DESC * sizeof(struct tx_desc_s));
	/* Make sure we use a P2 address (non-cacheable) */
	port_info->tx_desc_base = (struct tx_desc_s *)ADDR_TO_P2(tmp_addr);
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
	outl(ADDR_TO_PHY(port_info->tx_desc_base), TDLAR(port));
#if defined(CONFIG_CPU_SH7763)
	outl(ADDR_TO_PHY(port_info->tx_desc_base), TDFAR(port));
	outl(ADDR_TO_PHY(cur_tx_desc), TDFXR(port));
	outl(0x01, TDFFR(port));/* Last discriptor bit */
#endif

err:
	return ret;
}

static int sh_eth_rx_desc_init(struct sh_eth_dev *eth)
{
	int port = eth->port, i , ret = 0;
	struct sh_eth_info *port_info = &eth->port_info[port];
	struct rx_desc_s *cur_rx_desc;
	u32 tmp_addr;
	u8 *rx_buf;

	/*
	 * Allocate rx descriptors. They must be RX_DESC_SIZE bytes aligned
	 */
	port_info->rx_desc_malloc = malloc(NUM_RX_DESC *
						 sizeof(struct rx_desc_s) +
						 RX_DESC_SIZE - 1);
	if (!port_info->rx_desc_malloc) {
		printf(SHETHER_NAME ": malloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	tmp_addr = (u32) (((int)port_info->rx_desc_malloc + RX_DESC_SIZE - 1) &
			  ~(RX_DESC_SIZE - 1));
	flush_cache_wback(tmp_addr, NUM_RX_DESC * sizeof(struct rx_desc_s));
	/* Make sure we use a P2 address (non-cacheable) */
	port_info->rx_desc_base = (struct rx_desc_s *)ADDR_TO_P2(tmp_addr);

	port_info->rx_desc_cur = port_info->rx_desc_base;

	/*
	 * Allocate rx data buffers. They must be 32 bytes aligned  and in
	 * P2 area
	 */
	port_info->rx_buf_malloc = malloc(NUM_RX_DESC * MAX_BUF_SIZE + 31);
	if (!port_info->rx_buf_malloc) {
		printf(SHETHER_NAME ": malloc failed\n");
		ret = -ENOMEM;
		goto err_buf_malloc;
	}

	tmp_addr = (u32)(((int)port_info->rx_buf_malloc + (32 - 1)) &
			  ~(32 - 1));
	port_info->rx_buf_base = (u8 *)ADDR_TO_P2(tmp_addr);

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
	outl(ADDR_TO_PHY(port_info->rx_desc_base), RDLAR(port));
#if defined(CONFIG_CPU_SH7763)
	outl(ADDR_TO_PHY(port_info->rx_desc_base), RDFAR(port));
	outl(ADDR_TO_PHY(cur_rx_desc), RDFXR(port));
	outl(RDFFR_RDLF, RDFFR(port));
#endif

	return ret;

err_buf_malloc:
	free(port_info->rx_desc_malloc);
	port_info->rx_desc_malloc = NULL;

err:
	return ret;
}

static void sh_eth_tx_desc_free(struct sh_eth_dev *eth)
{
	int port = eth->port;
	struct sh_eth_info *port_info = &eth->port_info[port];

	if (port_info->tx_desc_malloc) {
		free(port_info->tx_desc_malloc);
		port_info->tx_desc_malloc = NULL;
	}
}

static void sh_eth_rx_desc_free(struct sh_eth_dev *eth)
{
	int port = eth->port;
	struct sh_eth_info *port_info = &eth->port_info[port];

	if (port_info->rx_desc_malloc) {
		free(port_info->rx_desc_malloc);
		port_info->rx_desc_malloc = NULL;
	}

	if (port_info->rx_buf_malloc) {
		free(port_info->rx_buf_malloc);
		port_info->rx_buf_malloc = NULL;
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
	int port = eth->port, timeout, ret = 0;
	struct sh_eth_info *port_info = &eth->port_info[port];
	u32 val;

	/* Reset phy */
	sh_eth_mii_write_phy_reg
		(port, port_info->phy_addr, PHY_CTRL, PHY_C_RESET);
	timeout = 10;
	while (timeout--) {
		val = sh_eth_mii_read_phy_reg(port,
				port_info->phy_addr, PHY_CTRL);
		if (!(val & PHY_C_RESET))
			break;
		udelay(SH_ETH_PHY_DELAY);
	}

	if (timeout < 0) {
		printf(SHETHER_NAME ": phy reset timeout\n");
		ret = -EIO;
		goto err_tout;
	}

	/* Advertise 100/10 baseT full/half duplex */
	sh_eth_mii_write_phy_reg(port, port_info->phy_addr, PHY_ANA,
		(PHY_A_FDX|PHY_A_HDX|PHY_A_10FDX|PHY_A_10HDX|PHY_A_EXT));
	/* Autonegotiation, normal operation, full duplex, enable tx */
	sh_eth_mii_write_phy_reg(port, port_info->phy_addr, PHY_CTRL,
		(PHY_C_ANEGEN|PHY_C_RANEG));
	/* Wait for autonegotiation to complete */
	timeout = 100;
	while (timeout--) {
		val = sh_eth_mii_read_phy_reg(port, port_info->phy_addr, 1);
		if (val & PHY_S_ANEGC)
			break;

		udelay(SH_ETH_PHY_DELAY);
	}

	if (timeout < 0) {
		printf(SHETHER_NAME ": phy auto-negotiation failed\n");
		ret = -ETIMEDOUT;
		goto err_tout;
	}

	return ret;

err_tout:
	return ret;
}

static int sh_eth_config(struct sh_eth_dev *eth, bd_t *bd)
{
	int port = eth->port, ret = 0;
	u32 val,  phy_status;
	struct sh_eth_info *port_info = &eth->port_info[port];
	struct eth_device *dev = port_info->dev;

	/* Configure e-dmac registers */
	outl((inl(EDMR(port)) & ~EMDR_DESC_R) | EDMR_EL, EDMR(port));
	outl(0, EESIPR(port));
	outl(0, TRSCER(port));
	outl(0, TFTR(port));
	outl((FIFO_SIZE_T | FIFO_SIZE_R), FDR(port));
	outl(RMCR_RST, RMCR(port));
#ifndef CONFIG_CPU_SH7757
	outl(0, RPADIR(port));
#endif
	outl((FIFO_F_D_RFF | FIFO_F_D_RFD), FCFTR(port));

	/* Configure e-mac registers */
#if defined(CONFIG_CPU_SH7757)
	outl(ECSIPR_BRCRXIP | ECSIPR_PSRTOIP | ECSIPR_LCHNGIP |
		ECSIPR_MPDIP | ECSIPR_ICDIP, ECSIPR(port));
#else
	outl(0, ECSIPR(port));
#endif

	/* Set Mac address */
	val = dev->enetaddr[0] << 24 | dev->enetaddr[1] << 16 |
	    dev->enetaddr[2] << 8 | dev->enetaddr[3];
	outl(val, MAHR(port));

	val = dev->enetaddr[4] << 8 | dev->enetaddr[5];
	outl(val, MALR(port));

	outl(RFLR_RFL_MIN, RFLR(port));
#ifndef CONFIG_CPU_SH7757
	outl(0, PIPR(port));
#endif
	outl(APR_AP, APR(port));
	outl(MPR_MP, MPR(port));
#ifdef CONFIG_CPU_SH7757
	outl(TPAUSER_UNLIMITED, TPAUSER(port));
#else
	outl(TPAUSER_TPAUSE, TPAUSER(port));
#endif
	/* Configure phy */
	ret = sh_eth_phy_config(eth);
	if (ret) {
		printf(SHETHER_NAME ": phy config timeout\n");
		goto err_phy_cfg;
	}
	/* Read phy status to finish configuring the e-mac */
	phy_status = sh_eth_mii_read_phy_reg(port, port_info->phy_addr, 1);

	/* Set the transfer speed */
#ifdef CONFIG_CPU_SH7763
	if (phy_status & (PHY_S_100X_F|PHY_S_100X_H)) {
		printf(SHETHER_NAME ": 100Base/");
		outl(GECMR_100B, GECMR(port));
	} else {
		printf(SHETHER_NAME ": 10Base/");
		outl(GECMR_10B, GECMR(port));
	}
#endif
#if defined(CONFIG_CPU_SH7757)
	if (phy_status & (PHY_S_100X_F|PHY_S_100X_H)) {
		printf("100Base/");
		outl(1, RTRATE(port));
	} else {
		printf("10Base/");
		outl(0, RTRATE(port));
	}
#endif

	/* Check if full duplex mode is supported by the phy */
	if (phy_status & (PHY_S_100X_F|PHY_S_10T_F)) {
		printf("Full\n");
		outl((ECMR_CHG_DM|ECMR_RE|ECMR_TE|ECMR_DM), ECMR(port));
	} else {
		printf("Half\n");
		outl((ECMR_CHG_DM|ECMR_RE|ECMR_TE),  ECMR(port));
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
	outl(EDRRR_R, EDRRR(eth->port));
}

static void sh_eth_stop(struct sh_eth_dev *eth)
{
	outl(~EDRRR_R, EDRRR(eth->port));
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

	sprintf(dev->name, SHETHER_NAME);

    /* Register Device to EtherNet subsystem  */
    eth_register(dev);

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
