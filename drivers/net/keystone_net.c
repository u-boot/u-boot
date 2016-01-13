/*
 * Ethernet driver for TI K2HK EVM.
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <command.h>
#include <console.h>

#include <net.h>
#include <phy.h>
#include <errno.h>
#include <miiphy.h>
#include <malloc.h>
#include <asm/ti-common/keystone_nav.h>
#include <asm/ti-common/keystone_net.h>
#include <asm/ti-common/keystone_serdes.h>

unsigned int emac_open;
static struct mii_dev *mdio_bus;
static unsigned int sys_has_mdio = 1;

#ifdef KEYSTONE2_EMAC_GIG_ENABLE
#define emac_gigabit_enable(x)	keystone2_eth_gigabit_enable(x)
#else
#define emac_gigabit_enable(x)	/* no gigabit to enable */
#endif

#define RX_BUFF_NUMS	24
#define RX_BUFF_LEN	1520
#define MAX_SIZE_STREAM_BUFFER RX_BUFF_LEN
#define SGMII_ANEG_TIMEOUT		4000

static u8 rx_buffs[RX_BUFF_NUMS * RX_BUFF_LEN] __aligned(16);

struct rx_buff_desc net_rx_buffs = {
	.buff_ptr	= rx_buffs,
	.num_buffs	= RX_BUFF_NUMS,
	.buff_len	= RX_BUFF_LEN,
	.rx_flow	= 22,
};

#ifndef CONFIG_SOC_K2G
static void keystone2_net_serdes_setup(void);
#endif

int keystone2_eth_read_mac_addr(struct eth_device *dev)
{
	struct eth_priv_t *eth_priv;
	u32 maca = 0;
	u32 macb = 0;

	eth_priv = (struct eth_priv_t *)dev->priv;

	/* Read the e-fuse mac address */
	if (eth_priv->slave_port == 1) {
		maca = __raw_readl(MAC_ID_BASE_ADDR);
		macb = __raw_readl(MAC_ID_BASE_ADDR + 4);
	}

	dev->enetaddr[0] = (macb >>  8) & 0xff;
	dev->enetaddr[1] = (macb >>  0) & 0xff;
	dev->enetaddr[2] = (maca >> 24) & 0xff;
	dev->enetaddr[3] = (maca >> 16) & 0xff;
	dev->enetaddr[4] = (maca >>  8) & 0xff;
	dev->enetaddr[5] = (maca >>  0) & 0xff;

	return 0;
}

/* MDIO */

static int keystone2_mdio_reset(struct mii_dev *bus)
{
	u_int32_t clkdiv;
	struct mdio_regs *adap_mdio = bus->priv;

	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;

	writel((clkdiv & 0xffff) | MDIO_CONTROL_ENABLE |
	       MDIO_CONTROL_FAULT | MDIO_CONTROL_FAULT_ENABLE,
	       &adap_mdio->control);

	while (readl(&adap_mdio->control) & MDIO_CONTROL_IDLE)
		;

	return 0;
}

/**
 * keystone2_mdio_read - read a PHY register via MDIO interface.
 * Blocks until operation is complete.
 */
static int keystone2_mdio_read(struct mii_dev *bus,
			       int addr, int devad, int reg)
{
	int tmp;
	struct mdio_regs *adap_mdio = bus->priv;

	while (readl(&adap_mdio->useraccess0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO | MDIO_USERACCESS0_WRITE_READ |
	       ((reg & 0x1f) << 21) | ((addr & 0x1f) << 16),
	       &adap_mdio->useraccess0);

	/* Wait for command to complete */
	while ((tmp = readl(&adap_mdio->useraccess0)) & MDIO_USERACCESS0_GO)
		;

	if (tmp & MDIO_USERACCESS0_ACK)
		return tmp & 0xffff;

	return -1;
}

/**
 * keystone2_mdio_write - write to a PHY register via MDIO interface.
 * Blocks until operation is complete.
 */
static int keystone2_mdio_write(struct mii_dev *bus,
				int addr, int devad, int reg, u16 val)
{
	struct mdio_regs *adap_mdio = bus->priv;

	while (readl(&adap_mdio->useraccess0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO | MDIO_USERACCESS0_WRITE_WRITE |
	       ((reg & 0x1f) << 21) | ((addr & 0x1f) << 16) |
	       (val & 0xffff), &adap_mdio->useraccess0);

	/* Wait for command to complete */
	while (readl(&adap_mdio->useraccess0) & MDIO_USERACCESS0_GO)
		;

	return 0;
}

static void  __attribute__((unused))
	keystone2_eth_gigabit_enable(struct eth_device *dev)
{
	u_int16_t data;
	struct eth_priv_t *eth_priv = (struct eth_priv_t *)dev->priv;

	if (sys_has_mdio) {
		data = keystone2_mdio_read(mdio_bus, eth_priv->phy_addr,
					   MDIO_DEVAD_NONE, 0);
		/* speed selection MSB */
		if (!(data & (1 << 6)))
			return;
	}

	/*
	 * Check if link detected is giga-bit
	 * If Gigabit mode detected, enable gigbit in MAC
	 */
	writel(readl(DEVICE_EMACSL_BASE(eth_priv->slave_port - 1) +
		     CPGMACSL_REG_CTL) |
	       EMAC_MACCONTROL_GIGFORCE | EMAC_MACCONTROL_GIGABIT_ENABLE,
	       DEVICE_EMACSL_BASE(eth_priv->slave_port - 1) + CPGMACSL_REG_CTL);
}

#ifdef CONFIG_SOC_K2G
int keystone_rgmii_config(struct phy_device *phy_dev)
{
	unsigned int i, status;

	i = 0;
	do {
		if (i > SGMII_ANEG_TIMEOUT) {
			puts(" TIMEOUT !\n");
			phy_dev->link = 0;
			return 0;
		}

		if (ctrlc()) {
			puts("user interrupt!\n");
			phy_dev->link = 0;
			return -EINTR;
		}

		if ((i++ % 500) == 0)
			printf(".");

		udelay(1000);   /* 1 ms */
		status = readl(RGMII_STATUS_REG);
	} while (!(status & RGMII_REG_STATUS_LINK));

	puts(" done\n");

	return 0;
}
#else
int keystone_sgmii_config(struct phy_device *phy_dev, int port, int interface)
{
	unsigned int i, status, mask;
	unsigned int mr_adv_ability, control;

	switch (interface) {
	case SGMII_LINK_MAC_MAC_AUTONEG:
		mr_adv_ability	= (SGMII_REG_MR_ADV_ENABLE |
				   SGMII_REG_MR_ADV_LINK |
				   SGMII_REG_MR_ADV_FULL_DUPLEX |
				   SGMII_REG_MR_ADV_GIG_MODE);
		control		= (SGMII_REG_CONTROL_MASTER |
				   SGMII_REG_CONTROL_AUTONEG);

		break;
	case SGMII_LINK_MAC_PHY:
	case SGMII_LINK_MAC_PHY_FORCED:
		mr_adv_ability	= SGMII_REG_MR_ADV_ENABLE;
		control		= SGMII_REG_CONTROL_AUTONEG;

		break;
	case SGMII_LINK_MAC_MAC_FORCED:
		mr_adv_ability	= (SGMII_REG_MR_ADV_ENABLE |
				   SGMII_REG_MR_ADV_LINK |
				   SGMII_REG_MR_ADV_FULL_DUPLEX |
				   SGMII_REG_MR_ADV_GIG_MODE);
		control		= SGMII_REG_CONTROL_MASTER;

		break;
	case SGMII_LINK_MAC_FIBER:
		mr_adv_ability	= 0x20;
		control		= SGMII_REG_CONTROL_AUTONEG;

		break;
	default:
		mr_adv_ability	= SGMII_REG_MR_ADV_ENABLE;
		control		= SGMII_REG_CONTROL_AUTONEG;
	}

	__raw_writel(0, SGMII_CTL_REG(port));

	/*
	 * Wait for the SerDes pll to lock,
	 * but don't trap if lock is never read
	 */
	for (i = 0; i < 1000; i++)  {
		udelay(2000);
		status = __raw_readl(SGMII_STATUS_REG(port));
		if ((status & SGMII_REG_STATUS_LOCK) != 0)
			break;
	}

	__raw_writel(mr_adv_ability, SGMII_MRADV_REG(port));
	__raw_writel(control, SGMII_CTL_REG(port));


	mask = SGMII_REG_STATUS_LINK;

	if (control & SGMII_REG_CONTROL_AUTONEG)
		mask |= SGMII_REG_STATUS_AUTONEG;

	status = __raw_readl(SGMII_STATUS_REG(port));
	if ((status & mask) == mask)
		return 0;

	printf("\n%s Waiting for SGMII auto negotiation to complete",
	       phy_dev->dev->name);
	while ((status & mask) != mask) {
		/*
		 * Timeout reached ?
		 */
		if (i > SGMII_ANEG_TIMEOUT) {
			puts(" TIMEOUT !\n");
			phy_dev->link = 0;
			return 0;
		}

		if (ctrlc()) {
			puts("user interrupt!\n");
			phy_dev->link = 0;
			return -EINTR;
		}

		if ((i++ % 500) == 0)
			printf(".");

		udelay(1000);   /* 1 ms */
		status = __raw_readl(SGMII_STATUS_REG(port));
	}
	puts(" done\n");

	return 0;
}
#endif

int mac_sl_reset(u32 port)
{
	u32 i, v;

	if (port >= DEVICE_N_GMACSL_PORTS)
		return GMACSL_RET_INVALID_PORT;

	/* Set the soft reset bit */
	writel(CPGMAC_REG_RESET_VAL_RESET,
	       DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_RESET);

	/* Wait for the bit to clear */
	for (i = 0; i < DEVICE_EMACSL_RESET_POLL_COUNT; i++) {
		v = readl(DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_RESET);
		if ((v & CPGMAC_REG_RESET_VAL_RESET_MASK) !=
		    CPGMAC_REG_RESET_VAL_RESET)
			return GMACSL_RET_OK;
	}

	/* Timeout on the reset */
	return GMACSL_RET_WARN_RESET_INCOMPLETE;
}

int mac_sl_config(u_int16_t port, struct mac_sl_cfg *cfg)
{
	u32 v, i;
	int ret = GMACSL_RET_OK;

	if (port >= DEVICE_N_GMACSL_PORTS)
		return GMACSL_RET_INVALID_PORT;

	if (cfg->max_rx_len > CPGMAC_REG_MAXLEN_LEN) {
		cfg->max_rx_len = CPGMAC_REG_MAXLEN_LEN;
		ret = GMACSL_RET_WARN_MAXLEN_TOO_BIG;
	}

	/* Must wait if the device is undergoing reset */
	for (i = 0; i < DEVICE_EMACSL_RESET_POLL_COUNT; i++) {
		v = readl(DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_RESET);
		if ((v & CPGMAC_REG_RESET_VAL_RESET_MASK) !=
		    CPGMAC_REG_RESET_VAL_RESET)
			break;
	}

	if (i == DEVICE_EMACSL_RESET_POLL_COUNT)
		return GMACSL_RET_CONFIG_FAIL_RESET_ACTIVE;

	writel(cfg->max_rx_len, DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_MAXLEN);
	writel(cfg->ctl, DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_CTL);

#ifndef CONFIG_SOC_K2HK
	/* Map RX packet flow priority to 0 */
	writel(0, DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_RX_PRI_MAP);
#endif

	return ret;
}

int ethss_config(u32 ctl, u32 max_pkt_size)
{
	u32 i;

	/* Max length register */
	writel(max_pkt_size, DEVICE_CPSW_BASE + CPSW_REG_MAXLEN);

	/* Control register */
	writel(ctl, DEVICE_CPSW_BASE + CPSW_REG_CTL);

	/* All statistics enabled by default */
	writel(CPSW_REG_VAL_STAT_ENABLE_ALL,
	       DEVICE_CPSW_BASE + CPSW_REG_STAT_PORT_EN);

	/* Reset and enable the ALE */
	writel(CPSW_REG_VAL_ALE_CTL_RESET_AND_ENABLE |
	       CPSW_REG_VAL_ALE_CTL_BYPASS,
	       DEVICE_CPSW_BASE + CPSW_REG_ALE_CONTROL);

	/* All ports put into forward mode */
	for (i = 0; i < DEVICE_CPSW_NUM_PORTS; i++)
		writel(CPSW_REG_VAL_PORTCTL_FORWARD_MODE,
		       DEVICE_CPSW_BASE + CPSW_REG_ALE_PORTCTL(i));

	return 0;
}

int ethss_start(void)
{
	int i;
	struct mac_sl_cfg cfg;

	cfg.max_rx_len	= MAX_SIZE_STREAM_BUFFER;
	cfg.ctl		= GMACSL_ENABLE | GMACSL_RX_ENABLE_EXT_CTL;

	for (i = 0; i < DEVICE_N_GMACSL_PORTS; i++) {
		mac_sl_reset(i);
		mac_sl_config(i, &cfg);
	}

	return 0;
}

int ethss_stop(void)
{
	int i;

	for (i = 0; i < DEVICE_N_GMACSL_PORTS; i++)
		mac_sl_reset(i);

	return 0;
}

int32_t cpmac_drv_send(u32 *buffer, int num_bytes, int slave_port_num)
{
	if (num_bytes < EMAC_MIN_ETHERNET_PKT_SIZE)
		num_bytes = EMAC_MIN_ETHERNET_PKT_SIZE;

	return ksnav_send(&netcp_pktdma, buffer,
			  num_bytes, (slave_port_num) << 16);
}

/* Eth device open */
static int keystone2_eth_open(struct eth_device *dev, bd_t *bis)
{
	struct eth_priv_t *eth_priv = (struct eth_priv_t *)dev->priv;
	struct phy_device *phy_dev = eth_priv->phy_dev;

	debug("+ emac_open\n");

	net_rx_buffs.rx_flow	= eth_priv->rx_flow;

	sys_has_mdio =
		(eth_priv->sgmii_link_type == SGMII_LINK_MAC_PHY) ? 1 : 0;

	if (sys_has_mdio)
		keystone2_mdio_reset(mdio_bus);

#ifdef CONFIG_SOC_K2G
	keystone_rgmii_config(phy_dev);
#else
	keystone_sgmii_config(phy_dev, eth_priv->slave_port - 1,
			      eth_priv->sgmii_link_type);
#endif

	udelay(10000);

	/* On chip switch configuration */
	ethss_config(target_get_switch_ctl(), SWITCH_MAX_PKT_SIZE);

	/* TODO: add error handling code */
	if (qm_init()) {
		printf("ERROR: qm_init()\n");
		return -1;
	}
	if (ksnav_init(&netcp_pktdma, &net_rx_buffs)) {
		qm_close();
		printf("ERROR: netcp_init()\n");
		return -1;
	}

	/*
	 * Streaming switch configuration. If not present this
	 * statement is defined to void in target.h.
	 * If present this is usually defined to a series of register writes
	 */
	hw_config_streaming_switch();

	if (sys_has_mdio) {
		keystone2_mdio_reset(mdio_bus);

		phy_startup(phy_dev);
		if (phy_dev->link == 0) {
			ksnav_close(&netcp_pktdma);
			qm_close();
			return -1;
		}
	}

	emac_gigabit_enable(dev);

	ethss_start();

	debug("- emac_open\n");

	emac_open = 1;

	return 0;
}

/* Eth device close */
void keystone2_eth_close(struct eth_device *dev)
{
	struct eth_priv_t *eth_priv = (struct eth_priv_t *)dev->priv;
	struct phy_device *phy_dev = eth_priv->phy_dev;

	debug("+ emac_close\n");

	if (!emac_open)
		return;

	ethss_stop();

	ksnav_close(&netcp_pktdma);
	qm_close();
	phy_shutdown(phy_dev);

	emac_open = 0;

	debug("- emac_close\n");
}

/*
 * This function sends a single packet on the network and returns
 * positive number (number of bytes transmitted) or negative for error
 */
static int keystone2_eth_send_packet(struct eth_device *dev,
					void *packet, int length)
{
	int ret_status = -1;
	struct eth_priv_t *eth_priv = (struct eth_priv_t *)dev->priv;
	struct phy_device *phy_dev = eth_priv->phy_dev;

	genphy_update_link(phy_dev);
	if (phy_dev->link == 0)
		return -1;

	if (cpmac_drv_send((u32 *)packet, length, eth_priv->slave_port) != 0)
		return ret_status;

	return length;
}

/*
 * This function handles receipt of a packet from the network
 */
static int keystone2_eth_rcv_packet(struct eth_device *dev)
{
	void *hd;
	int  pkt_size;
	u32  *pkt;

	hd = ksnav_recv(&netcp_pktdma, &pkt, &pkt_size);
	if (hd == NULL)
		return 0;

	net_process_received_packet((uchar *)pkt, pkt_size);

	ksnav_release_rxhd(&netcp_pktdma, hd);

	return pkt_size;
}

#ifdef CONFIG_MCAST_TFTP
static int keystone2_eth_bcast_addr(struct eth_device *dev, u32 ip, u8 set)
{
	return 0;
}
#endif

/*
 * This function initializes the EMAC hardware.
 */
int keystone2_emac_initialize(struct eth_priv_t *eth_priv)
{
	int res;
	struct eth_device *dev;
	struct phy_device *phy_dev;

	dev = malloc(sizeof(struct eth_device));
	if (dev == NULL)
		return -1;

	memset(dev, 0, sizeof(struct eth_device));

	strcpy(dev->name, eth_priv->int_name);
	dev->priv = eth_priv;

	keystone2_eth_read_mac_addr(dev);

	dev->iobase		= 0;
	dev->init		= keystone2_eth_open;
	dev->halt		= keystone2_eth_close;
	dev->send		= keystone2_eth_send_packet;
	dev->recv		= keystone2_eth_rcv_packet;
#ifdef CONFIG_MCAST_TFTP
	dev->mcast		= keystone2_eth_bcast_addr;
#endif

	eth_register(dev);

	/* Register MDIO bus if it's not registered yet */
	if (!mdio_bus) {
		mdio_bus	= mdio_alloc();
		mdio_bus->read	= keystone2_mdio_read;
		mdio_bus->write	= keystone2_mdio_write;
		mdio_bus->reset	= keystone2_mdio_reset;
		mdio_bus->priv	= (void *)EMAC_MDIO_BASE_ADDR;
		sprintf(mdio_bus->name, "ethernet-mdio");

		res = mdio_register(mdio_bus);
		if (res)
			return res;
	}

#ifndef CONFIG_SOC_K2G
	keystone2_net_serdes_setup();
#endif

	/* Create phy device and bind it with driver */
#ifdef CONFIG_KSNET_MDIO_PHY_CONFIG_ENABLE
	phy_dev = phy_connect(mdio_bus, eth_priv->phy_addr,
			      dev, eth_priv->phy_if);
	phy_config(phy_dev);
#else
	phy_dev = phy_find_by_mask(mdio_bus, 1 << eth_priv->phy_addr,
				   eth_priv->phy_if);
	phy_dev->dev = dev;
#endif
	eth_priv->phy_dev = phy_dev;

	return 0;
}

struct ks2_serdes ks2_serdes_sgmii_156p25mhz = {
	.clk = SERDES_CLOCK_156P25M,
	.rate = SERDES_RATE_5G,
	.rate_mode = SERDES_QUARTER_RATE,
	.intf = SERDES_PHY_SGMII,
	.loopback = 0,
};

#ifndef CONFIG_SOC_K2G
static void keystone2_net_serdes_setup(void)
{
	ks2_serdes_init(CONFIG_KSNET_SERDES_SGMII_BASE,
			&ks2_serdes_sgmii_156p25mhz,
			CONFIG_KSNET_SERDES_LANES_PER_SGMII);

#if defined(CONFIG_SOC_K2E) || defined(CONFIG_SOC_K2L)
	ks2_serdes_init(CONFIG_KSNET_SERDES_SGMII2_BASE,
			&ks2_serdes_sgmii_156p25mhz,
			CONFIG_KSNET_SERDES_LANES_PER_SGMII);
#endif

	/* wait till setup */
	udelay(5000);
}
#endif
