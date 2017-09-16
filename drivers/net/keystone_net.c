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

#include <dm.h>
#include <dm/lists.h>

#include <net.h>
#include <phy.h>
#include <errno.h>
#include <miiphy.h>
#include <malloc.h>
#include <asm/ti-common/keystone_nav.h>
#include <asm/ti-common/keystone_net.h>
#include <asm/ti-common/keystone_serdes.h>
#include <asm/arch/psc_defs.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_DM_ETH
unsigned int emac_open;
static struct mii_dev *mdio_bus;
static unsigned int sys_has_mdio = 1;
#endif

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

#ifndef CONFIG_DM_ETH
struct rx_buff_desc net_rx_buffs = {
	.buff_ptr	= rx_buffs,
	.num_buffs	= RX_BUFF_NUMS,
	.buff_len	= RX_BUFF_LEN,
	.rx_flow	= 22,
};
#endif

#ifdef CONFIG_DM_ETH

enum link_type {
	LINK_TYPE_SGMII_MAC_TO_MAC_AUTO		= 0,
	LINK_TYPE_SGMII_MAC_TO_PHY_MODE		= 1,
	LINK_TYPE_SGMII_MAC_TO_MAC_FORCED_MODE	= 2,
	LINK_TYPE_SGMII_MAC_TO_FIBRE_MODE	= 3,
	LINK_TYPE_SGMII_MAC_TO_PHY_NO_MDIO_MODE	= 4,
	LINK_TYPE_RGMII_LINK_MAC_PHY		= 5,
	LINK_TYPE_RGMII_LINK_MAC_MAC_FORCED	= 6,
	LINK_TYPE_RGMII_LINK_MAC_PHY_NO_MDIO	= 7,
	LINK_TYPE_10G_MAC_TO_PHY_MODE		= 10,
	LINK_TYPE_10G_MAC_TO_MAC_FORCED_MODE	= 11,
};

#define mac_hi(mac)     (((mac)[0] << 0) | ((mac)[1] << 8) |    \
			 ((mac)[2] << 16) | ((mac)[3] << 24))
#define mac_lo(mac)     (((mac)[4] << 0) | ((mac)[5] << 8))

#ifdef CONFIG_KSNET_NETCP_V1_0

#define EMAC_EMACSW_BASE_OFS		0x90800
#define EMAC_EMACSW_PORT_BASE_OFS	(EMAC_EMACSW_BASE_OFS + 0x60)

/* CPSW Switch slave registers */
#define CPGMACSL_REG_SA_LO		0x10
#define CPGMACSL_REG_SA_HI		0x14

#define DEVICE_EMACSW_BASE(base, x)	((base) + EMAC_EMACSW_PORT_BASE_OFS +  \
					 (x) * 0x30)

#elif defined CONFIG_KSNET_NETCP_V1_5

#define EMAC_EMACSW_PORT_BASE_OFS	0x222000

/* CPSW Switch slave registers */
#define CPGMACSL_REG_SA_LO		0x308
#define CPGMACSL_REG_SA_HI		0x30c

#define DEVICE_EMACSW_BASE(base, x)	((base) + EMAC_EMACSW_PORT_BASE_OFS +  \
					 (x) * 0x1000)

#endif


struct ks2_eth_priv {
	struct udevice			*dev;
	struct phy_device		*phydev;
	struct mii_dev			*mdio_bus;
	int				phy_addr;
	phy_interface_t			phy_if;
	int				sgmii_link_type;
	void				*mdio_base;
	struct rx_buff_desc		net_rx_buffs;
	struct pktdma_cfg		*netcp_pktdma;
	void				*hd;
	int				slave_port;
	enum link_type			link_type;
	bool				emac_open;
	bool				has_mdio;
};
#endif

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

#ifndef CONFIG_DM_ETH
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
#else
static void  __attribute__((unused))
	keystone2_eth_gigabit_enable(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	u_int16_t data;

	if (priv->has_mdio) {
		data = keystone2_mdio_read(priv->mdio_bus, priv->phy_addr,
					   MDIO_DEVAD_NONE, 0);
		/* speed selection MSB */
		if (!(data & (1 << 6)))
			return;
	}

	/*
	 * Check if link detected is giga-bit
	 * If Gigabit mode detected, enable gigbit in MAC
	 */
	writel(readl(DEVICE_EMACSL_BASE(priv->slave_port - 1) +
		     CPGMACSL_REG_CTL) |
	       EMAC_MACCONTROL_GIGFORCE | EMAC_MACCONTROL_GIGABIT_ENABLE,
	       DEVICE_EMACSL_BASE(priv->slave_port - 1) + CPGMACSL_REG_CTL);
}
#endif

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

#ifndef CONFIG_DM_ETH

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
	struct mdio_regs *adap_mdio = (struct mdio_regs *)EMAC_MDIO_BASE_ADDR;

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
		strcpy(mdio_bus->name, "ethernet-mdio");

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

#else

static int ks2_eth_start(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

#ifdef CONFIG_SOC_K2G
	keystone_rgmii_config(priv->phydev);
#else
	keystone_sgmii_config(priv->phydev, priv->slave_port - 1,
			      priv->sgmii_link_type);
#endif

	udelay(10000);

	/* On chip switch configuration */
	ethss_config(target_get_switch_ctl(), SWITCH_MAX_PKT_SIZE);

	qm_init();

	if (ksnav_init(priv->netcp_pktdma, &priv->net_rx_buffs)) {
		pr_err("ksnav_init failed\n");
		goto err_knav_init;
	}

	/*
	 * Streaming switch configuration. If not present this
	 * statement is defined to void in target.h.
	 * If present this is usually defined to a series of register writes
	 */
	hw_config_streaming_switch();

	if (priv->has_mdio) {
		keystone2_mdio_reset(priv->mdio_bus);

		phy_startup(priv->phydev);
		if (priv->phydev->link == 0) {
			pr_err("phy startup failed\n");
			goto err_phy_start;
		}
	}

	emac_gigabit_enable(dev);

	ethss_start();

	priv->emac_open = true;

	return 0;

err_phy_start:
	ksnav_close(priv->netcp_pktdma);
err_knav_init:
	qm_close();

	return -EFAULT;
}

static int ks2_eth_send(struct udevice *dev, void *packet, int length)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	genphy_update_link(priv->phydev);
	if (priv->phydev->link == 0)
		return -1;

	if (length < EMAC_MIN_ETHERNET_PKT_SIZE)
		length = EMAC_MIN_ETHERNET_PKT_SIZE;

	return ksnav_send(priv->netcp_pktdma, (u32 *)packet,
			  length, (priv->slave_port) << 16);
}

static int ks2_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	int  pkt_size;
	u32 *pkt = NULL;

	priv->hd = ksnav_recv(priv->netcp_pktdma, &pkt, &pkt_size);
	if (priv->hd == NULL)
		return -EAGAIN;

	*packetp = (uchar *)pkt;

	return pkt_size;
}

static int ks2_eth_free_pkt(struct udevice *dev, uchar *packet,
				   int length)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	ksnav_release_rxhd(priv->netcp_pktdma, priv->hd);

	return 0;
}

static void ks2_eth_stop(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	if (!priv->emac_open)
		return;
	ethss_stop();

	ksnav_close(priv->netcp_pktdma);
	qm_close();
	phy_shutdown(priv->phydev);
	priv->emac_open = false;
}

int ks2_eth_read_rom_hwaddr(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	u32 maca = 0;
	u32 macb = 0;

	/* Read the e-fuse mac address */
	if (priv->slave_port == 1) {
		maca = __raw_readl(MAC_ID_BASE_ADDR);
		macb = __raw_readl(MAC_ID_BASE_ADDR + 4);
	}

	pdata->enetaddr[0] = (macb >>  8) & 0xff;
	pdata->enetaddr[1] = (macb >>  0) & 0xff;
	pdata->enetaddr[2] = (maca >> 24) & 0xff;
	pdata->enetaddr[3] = (maca >> 16) & 0xff;
	pdata->enetaddr[4] = (maca >>  8) & 0xff;
	pdata->enetaddr[5] = (maca >>  0) & 0xff;

	return 0;
}

int ks2_eth_write_hwaddr(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	writel(mac_hi(pdata->enetaddr),
	       DEVICE_EMACSW_BASE(pdata->iobase, priv->slave_port - 1) +
				  CPGMACSL_REG_SA_HI);
	writel(mac_lo(pdata->enetaddr),
	       DEVICE_EMACSW_BASE(pdata->iobase, priv->slave_port - 1) +
				  CPGMACSL_REG_SA_LO);

	return 0;
}

static int ks2_eth_probe(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct mii_dev *mdio_bus;
	int ret;

	priv->dev = dev;

	/* These clock enables has to be moved to common location */
	if (cpu_is_k2g())
		writel(KS2_ETHERNET_RGMII, KS2_ETHERNET_CFG);

	/* By default, select PA PLL clock as PA clock source */
#ifndef CONFIG_SOC_K2G
	if (psc_enable_module(KS2_LPSC_PA))
		return -EACCES;
#endif
	if (psc_enable_module(KS2_LPSC_CPGMAC))
		return -EACCES;
	if (psc_enable_module(KS2_LPSC_CRYPTO))
		return -EACCES;

	if (cpu_is_k2e() || cpu_is_k2l())
		pll_pa_clk_sel();


	priv->net_rx_buffs.buff_ptr = rx_buffs;
	priv->net_rx_buffs.num_buffs = RX_BUFF_NUMS;
	priv->net_rx_buffs.buff_len = RX_BUFF_LEN;

	if (priv->slave_port == 1) {
		/*
		 * Register MDIO bus for slave 0 only, other slave have
		 * to re-use the same
		 */
		mdio_bus = mdio_alloc();
		if (!mdio_bus) {
			pr_err("MDIO alloc failed\n");
			return -ENOMEM;
		}
		priv->mdio_bus = mdio_bus;
		mdio_bus->read	= keystone2_mdio_read;
		mdio_bus->write	= keystone2_mdio_write;
		mdio_bus->reset	= keystone2_mdio_reset;
		mdio_bus->priv	= priv->mdio_base;
		sprintf(mdio_bus->name, "ethernet-mdio");

		ret = mdio_register(mdio_bus);
		if (ret) {
			pr_err("MDIO bus register failed\n");
			return ret;
		}
	} else {
		/* Get the MDIO bus from slave 0 device */
		struct ks2_eth_priv *parent_priv;

		parent_priv = dev_get_priv(dev->parent);
		priv->mdio_bus = parent_priv->mdio_bus;
	}

#ifndef CONFIG_SOC_K2G
	keystone2_net_serdes_setup();
#endif

	priv->netcp_pktdma = &netcp_pktdma;

	if (priv->has_mdio) {
		priv->phydev = phy_connect(priv->mdio_bus, priv->phy_addr,
					   dev, priv->phy_if);
		phy_config(priv->phydev);
	}

	return 0;
}

int ks2_eth_remove(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->mdio_bus);
	mdio_free(priv->mdio_bus);

	return 0;
}

static const struct eth_ops ks2_eth_ops = {
	.start			= ks2_eth_start,
	.send			= ks2_eth_send,
	.recv			= ks2_eth_recv,
	.free_pkt		= ks2_eth_free_pkt,
	.stop			= ks2_eth_stop,
	.read_rom_hwaddr	= ks2_eth_read_rom_hwaddr,
	.write_hwaddr		= ks2_eth_write_hwaddr,
};

static int ks2_eth_bind_slaves(struct udevice *dev, int gbe, int *gbe_0)
{
	const void *fdt = gd->fdt_blob;
	struct udevice *sl_dev;
	int interfaces;
	int sec_slave;
	int slave;
	int ret;
	char *slave_name;

	interfaces = fdt_subnode_offset(fdt, gbe, "interfaces");
	fdt_for_each_subnode(slave, fdt, interfaces) {
		int slave_no;

		slave_no = fdtdec_get_int(fdt, slave, "slave-port", -ENOENT);
		if (slave_no == -ENOENT)
			continue;

		if (slave_no == 0) {
			/* This is the current eth device */
			*gbe_0 = slave;
		} else {
			/* Slave devices to be registered */
			slave_name = malloc(20);
			snprintf(slave_name, 20, "netcp@slave-%d", slave_no);
			ret = device_bind_driver_to_node(dev, "eth_ks2_sl",
					slave_name, offset_to_ofnode(slave),
					&sl_dev);
			if (ret) {
				pr_err("ks2_net - not able to bind slave interfaces\n");
				return ret;
			}
		}
	}

	sec_slave = fdt_subnode_offset(fdt, gbe, "secondary-slave-ports");
	fdt_for_each_subnode(slave, fdt, sec_slave) {
		int slave_no;

		slave_no = fdtdec_get_int(fdt, slave, "slave-port", -ENOENT);
		if (slave_no == -ENOENT)
			continue;

		/* Slave devices to be registered */
		slave_name = malloc(20);
		snprintf(slave_name, 20, "netcp@slave-%d", slave_no);
		ret = device_bind_driver_to_node(dev, "eth_ks2_sl", slave_name,
					offset_to_ofnode(slave), &sl_dev);
		if (ret) {
			pr_err("ks2_net - not able to bind slave interfaces\n");
			return ret;
		}
	}

	return 0;
}

static int ks2_eth_parse_slave_interface(int netcp, int slave,
					 struct ks2_eth_priv *priv,
					 struct eth_pdata *pdata)
{
	const void *fdt = gd->fdt_blob;
	int mdio;
	int phy;
	int dma_count;
	u32 dma_channel[8];

	priv->slave_port = fdtdec_get_int(fdt, slave, "slave-port", -1);
	priv->net_rx_buffs.rx_flow = priv->slave_port * 8;

	/* U-Boot slave port number starts with 1 instead of 0 */
	priv->slave_port += 1;

	dma_count = fdtdec_get_int_array_count(fdt, netcp,
					       "ti,navigator-dmas",
					       dma_channel, 8);

	if (dma_count > (2 * priv->slave_port)) {
		int dma_idx;

		dma_idx = priv->slave_port * 2 - 1;
		priv->net_rx_buffs.rx_flow = dma_channel[dma_idx];
	}

	priv->link_type = fdtdec_get_int(fdt, slave, "link-interface", -1);

	phy = fdtdec_lookup_phandle(fdt, slave, "phy-handle");
	if (phy >= 0) {
		priv->phy_addr = fdtdec_get_int(fdt, phy, "reg", -1);

		mdio = fdt_parent_offset(fdt, phy);
		if (mdio < 0) {
			pr_err("mdio dt not found\n");
			return -ENODEV;
		}
		priv->mdio_base = (void *)fdtdec_get_addr(fdt, mdio, "reg");
	}

	if (priv->link_type == LINK_TYPE_SGMII_MAC_TO_PHY_MODE) {
		priv->phy_if = PHY_INTERFACE_MODE_SGMII;
		pdata->phy_interface = priv->phy_if;
		priv->sgmii_link_type = SGMII_LINK_MAC_PHY;
		priv->has_mdio = true;
	} else if (priv->link_type == LINK_TYPE_RGMII_LINK_MAC_PHY) {
		priv->phy_if = PHY_INTERFACE_MODE_RGMII;
		pdata->phy_interface = priv->phy_if;
		priv->has_mdio = true;
	}

	return 0;
}

static int ks2_sl_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	int slave = dev_of_offset(dev);
	int interfaces;
	int gbe;
	int netcp_devices;
	int netcp;

	interfaces = fdt_parent_offset(fdt, slave);
	gbe = fdt_parent_offset(fdt, interfaces);
	netcp_devices = fdt_parent_offset(fdt, gbe);
	netcp = fdt_parent_offset(fdt, netcp_devices);

	ks2_eth_parse_slave_interface(netcp, slave, priv, pdata);

	pdata->iobase = fdtdec_get_addr(fdt, netcp, "reg");

	return 0;
}

static int ks2_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	int gbe_0 = -ENODEV;
	int netcp_devices;
	int gbe;

	netcp_devices = fdt_subnode_offset(fdt, dev_of_offset(dev),
					   "netcp-devices");
	gbe = fdt_subnode_offset(fdt, netcp_devices, "gbe");

	ks2_eth_bind_slaves(dev, gbe, &gbe_0);

	ks2_eth_parse_slave_interface(dev_of_offset(dev), gbe_0, priv, pdata);

	pdata->iobase = devfdt_get_addr(dev);

	return 0;
}

static const struct udevice_id ks2_eth_ids[] = {
	{ .compatible = "ti,netcp-1.0" },
	{ }
};

U_BOOT_DRIVER(eth_ks2_slave) = {
	.name	= "eth_ks2_sl",
	.id	= UCLASS_ETH,
	.ofdata_to_platdata = ks2_sl_eth_ofdata_to_platdata,
	.probe	= ks2_eth_probe,
	.remove	= ks2_eth_remove,
	.ops	= &ks2_eth_ops,
	.priv_auto_alloc_size = sizeof(struct ks2_eth_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};

U_BOOT_DRIVER(eth_ks2) = {
	.name	= "eth_ks2",
	.id	= UCLASS_ETH,
	.of_match = ks2_eth_ids,
	.ofdata_to_platdata = ks2_eth_ofdata_to_platdata,
	.probe	= ks2_eth_probe,
	.remove	= ks2_eth_remove,
	.ops	= &ks2_eth_ops,
	.priv_auto_alloc_size = sizeof(struct ks2_eth_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
