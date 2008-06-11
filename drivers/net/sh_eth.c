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
#include <asm/errno.h>
#include <asm/io.h>

#include "sh_eth.h"

#ifndef CONFIG_SH_ETHER_USE_PORT
# error "Please define CONFIG_SH_ETHER_USE_PORT"
#endif
#ifndef CONFIG_SH_ETHER_PHY_ADDR
# error "Please define CONFIG_SH_ETHER_PHY_ADDR"
#endif

extern int eth_init(bd_t *bd);
extern void eth_halt(void);
extern int eth_rx(void);
extern int eth_send(volatile void *packet, int length);

static struct dev_info_s *dev;

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

static int sh_eth_mii_read_phy_bits(int port, u32 * val, int len)
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

	return 0;
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

void eth_halt(void)
{
}

int eth_send(volatile void *packet, int len)
{
	int port = dev->port;
	struct port_info_s *port_info = &dev->port_info[port];
	int timeout;
	int rc = 0;

	if (!packet || len > 0xffff) {
		printf("eth_send: Invalid argument\n");
		return -EINVAL;
	}

	/* packet must be a 4 byte boundary */
	if ((int)packet & (4 - 1)) {
		printf("eth_send: packet not 4 byte alligned\n");
		return -EFAULT;
	}

	/* Update tx descriptor */
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
		printf("eth_send: transmit timeout\n");
		rc = -1;
		goto err;
	}

err:
	port_info->tx_desc_cur++;
	if (port_info->tx_desc_cur >= port_info->tx_desc_base + NUM_TX_DESC)
		port_info->tx_desc_cur = port_info->tx_desc_base;

	return rc;
}

int eth_rx(void)
{
	int port = dev->port;
	struct port_info_s *port_info = &dev->port_info[port];
	int len = 0;
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
static int sh_eth_reset(struct dev_info_s *dev)
{
	int port = dev->port;
	int i;

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
		printf("Error: Software reset timeout\n");
		return -1;
	}
	return 0;
}

static int sh_eth_tx_desc_init(struct dev_info_s *dev)
{
	int port = dev->port;
	struct port_info_s *port_info = &dev->port_info[port];
	u32 tmp_addr;
	struct tx_desc_s *cur_tx_desc;
	int i;

	/* Allocate tx descriptors. They must be TX_DESC_SIZE bytes
	   aligned */
	if (!(port_info->tx_desc_malloc = malloc(NUM_TX_DESC *
						 sizeof(struct tx_desc_s) +
						 TX_DESC_SIZE - 1))) {
		printf("Error: malloc failed\n");
		return -ENOMEM;
	}
	tmp_addr = (u32) (((int)port_info->tx_desc_malloc + TX_DESC_SIZE - 1) &
			  ~(TX_DESC_SIZE - 1));
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
	outl(ADDR_TO_PHY(port_info->tx_desc_base), TDFAR(port));
	outl(ADDR_TO_PHY(cur_tx_desc), TDFXR(port));
	outl(0x01, TDFFR(port));/* Last discriptor bit */

	return 0;
}

static int sh_eth_rx_desc_init(struct dev_info_s *dev)
{
	int port = dev->port;
	struct port_info_s *port_info = &dev->port_info[port];
	u32 tmp_addr;
	struct rx_desc_s *cur_rx_desc;
	u8 *rx_buf;
	int i;

	/* Allocate rx descriptors. They must be RX_DESC_SIZE bytes
	   aligned */
	if (!(port_info->rx_desc_malloc = malloc(NUM_RX_DESC *
						 sizeof(struct rx_desc_s) +
						 RX_DESC_SIZE - 1))) {
		printf("Error: malloc failed\n");
		return -ENOMEM;
	}
	tmp_addr = (u32) (((int)port_info->rx_desc_malloc + RX_DESC_SIZE - 1) &
			  ~(RX_DESC_SIZE - 1));
	/* Make sure we use a P2 address (non-cacheable) */
	port_info->rx_desc_base = (struct rx_desc_s *)ADDR_TO_P2(tmp_addr);

	port_info->rx_desc_cur = port_info->rx_desc_base;

	/* Allocate rx data buffers. They must be 32 bytes aligned  and in
	   P2 area */
	if (!(port_info->rx_buf_malloc = malloc(NUM_RX_DESC * MAX_BUF_SIZE +
						31))) {
		printf("Error: malloc failed\n");
		free(port_info->rx_desc_malloc);
		port_info->rx_desc_malloc = NULL;
		return -ENOMEM;
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
	outl(ADDR_TO_PHY(port_info->rx_desc_base), RDFAR(port));
	outl(ADDR_TO_PHY(cur_rx_desc), RDFXR(port));
	outl(RDFFR_RDLF, RDFFR(port));

	return 0;
}

static void sh_eth_desc_free(struct dev_info_s *dev)
{
	int port = dev->port;
	struct port_info_s *port_info = &dev->port_info[port];

	if (port_info->tx_desc_malloc) {
		free(port_info->tx_desc_malloc);
		port_info->tx_desc_malloc = NULL;
	}

	if (port_info->rx_desc_malloc) {
		free(port_info->rx_desc_malloc);
		port_info->rx_desc_malloc = NULL;
	}

	if (port_info->rx_buf_malloc) {
		free(port_info->rx_buf_malloc);
		port_info->rx_buf_malloc = NULL;
	}
}

static int sh_eth_desc_init(struct dev_info_s *dev)
{
	int rc;

	if ((rc = sh_eth_tx_desc_init(dev)) || (rc = sh_eth_rx_desc_init(dev))) {
		sh_eth_desc_free(dev);
		return rc;
	}

	return 0;
}

static int sh_eth_phy_config(struct dev_info_s *dev)
{
	int port = dev->port;
	struct port_info_s *port_info = &dev->port_info[port];
	int timeout;
	u32 val;
	/* Reset phy */
	sh_eth_mii_write_phy_reg(port, port_info->phy_addr, PHY_CTRL, PHY_C_RESET);
	timeout = 10;
	while (timeout--) {
		val = sh_eth_mii_read_phy_reg(port, port_info->phy_addr, PHY_CTRL);
		if (!(val & PHY_C_RESET))
			break;
		udelay(50000);
	}
	if (timeout < 0) {
		printf("%s phy reset timeout\n", __func__);
		return -1;
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
		udelay(50000);
	}
	if (timeout < 0) {
		printf("sh_eth_phy_config() phy auto-negotiation failed\n");
		return -1;
	}

	return 0;
}

static int sh_eth_config(struct dev_info_s *dev, bd_t * bd)
{
	int port = dev->port;
	struct port_info_s *port_info = &dev->port_info[port];
	u32 val;
	u32 phy_status;
	int rc;

	/* Configure e-dmac registers */
	outl((inl(EDMR(port)) & ~EMDR_DESC_R) | EDMR_EL, EDMR(port));
	outl(0, EESIPR(port));
	outl(0, TRSCER(port));
	outl(0, TFTR(port));
	outl((FIFO_SIZE_T | FIFO_SIZE_R), FDR(port));
	outl(RMCR_RST, RMCR(port));
	outl(0, RPADIR(port));
	outl((FIFO_F_D_RFF | FIFO_F_D_RFD), FCFTR(port));

	/* Configure e-mac registers */
	outl(0, ECSIPR(port));

	/* Set Mac address */
	val = bd->bi_enetaddr[0] << 24 | bd->bi_enetaddr[1] << 16 |
	    bd->bi_enetaddr[2] << 8 | bd->bi_enetaddr[3];
	outl(val, MAHR(port));

	val = bd->bi_enetaddr[4] << 8 | bd->bi_enetaddr[5];
	outl(val, MALR(port));

	outl(RFLR_RFL_MIN, RFLR(port));
	outl(0, PIPR(port));
	outl(APR_AP, APR(port));
	outl(MPR_MP, MPR(port));
	outl(TPAUSER_TPAUSE, TPAUSER(port));

	/* Configure phy */
	if ((rc = sh_eth_phy_config(dev)))
		return rc;

	/* Read phy status to finish configuring the e-mac */
	phy_status = sh_eth_mii_read_phy_reg(dev->port,
					     dev->port_info[dev->port].phy_addr,
					     1);

	/* Set the transfer speed */
	if (phy_status & (PHY_S_100X_F|PHY_S_100X_H)) {
		printf("100Base/");
		outl(GECMR_100B, GECMR(port));
	} else {
		printf("10Base/");
		outl(GECMR_10B, GECMR(port));
	}

	/* Check if full duplex mode is supported by the phy */
	if (phy_status & (PHY_S_100X_F|PHY_S_10T_F)) {
		printf("Full\n");
		outl((ECMR_CHG_DM|ECMR_RE|ECMR_TE|ECMR_DM), ECMR(port));
	} else {
		printf("Half\n");
		outl((ECMR_CHG_DM|ECMR_RE|ECMR_TE),  ECMR(port));
	}
	return 0;
}

static int sh_eth_start(struct dev_info_s *dev)
{
	/*
	 * Enable the e-dmac receiver only. The transmitter will be enabled when
	 * we have something to transmit
	 */
	outl(EDRRR_R, EDRRR(dev->port));

	return 0;
}

static int sh_eth_get_mac(bd_t *bd)
{
	char *s, *e;
	int i;

	s = getenv("ethaddr");
	if (s != NULL) {
		for (i = 0; i < 6; ++i) {
			bd->bi_enetaddr[i] = s ? simple_strtoul(s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}
	} else {
		puts("Please set MAC address\n");
	}
	return 0;
}

int eth_init(bd_t *bd)
{
	int rc;
	/* Allocate main device information structure */
	if (!(dev = malloc(sizeof(*dev)))) {
		printf("eth_init: malloc failed\n");
		return -ENOMEM;
	}

	memset(dev, 0, sizeof(*dev));

	dev->port = CONFIG_SH_ETHER_USE_PORT;
	dev->port_info[dev->port].phy_addr = CONFIG_SH_ETHER_PHY_ADDR;

	sh_eth_get_mac(bd);

	if ((rc = sh_eth_reset(dev)) || (rc = sh_eth_desc_init(dev)))
		goto err;

	if ((rc = sh_eth_config(dev, bd)) || (rc = sh_eth_start(dev)))
		goto err_desc;

	return 0;

err_desc:
	sh_eth_desc_free(dev);
err:
	free(dev);
	printf("eth_init: Failed\n");
	return rc;
}
