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

#include <net.h>
#include <miiphy.h>
#include <malloc.h>
#include <asm/ti-common/keystone_nav.h>
#include <asm/ti-common/keystone_net.h>
#include <asm/ti-common/keystone_serdes.h>

unsigned int emac_open;
static unsigned int sys_has_mdio = 1;

#ifdef KEYSTONE2_EMAC_GIG_ENABLE
#define emac_gigabit_enable(x)	keystone2_eth_gigabit_enable(x)
#else
#define emac_gigabit_enable(x)	/* no gigabit to enable */
#endif

#define RX_BUFF_NUMS	24
#define RX_BUFF_LEN	1520
#define MAX_SIZE_STREAM_BUFFER RX_BUFF_LEN

static u8 rx_buffs[RX_BUFF_NUMS * RX_BUFF_LEN] __aligned(16);

struct rx_buff_desc net_rx_buffs = {
	.buff_ptr	= rx_buffs,
	.num_buffs	= RX_BUFF_NUMS,
	.buff_len	= RX_BUFF_LEN,
	.rx_flow	= 22,
};

static void keystone2_eth_mdio_enable(void);
static void keystone2_net_serdes_setup(void);

static int gen_get_link_speed(int phy_addr);

/* EMAC Addresses */
static volatile struct mdio_regs	*adap_mdio =
	(struct mdio_regs *)EMAC_MDIO_BASE_ADDR;

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

static void keystone2_eth_mdio_enable(void)
{
	u_int32_t	clkdiv;

	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;

	writel((clkdiv & 0xffff) |
	       MDIO_CONTROL_ENABLE |
	       MDIO_CONTROL_FAULT |
	       MDIO_CONTROL_FAULT_ENABLE,
	       &adap_mdio->control);

	while (readl(&adap_mdio->control) & MDIO_CONTROL_IDLE)
		;
}

/* Read a PHY register via MDIO inteface. Returns 1 on success, 0 otherwise */
int keystone2_eth_phy_read(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t *data)
{
	int	tmp;

	while (readl(&adap_mdio->useraccess0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO |
	       MDIO_USERACCESS0_WRITE_READ |
	       ((reg_num & 0x1f) << 21) |
	       ((phy_addr & 0x1f) << 16),
	       &adap_mdio->useraccess0);

	/* Wait for command to complete */
	while ((tmp = readl(&adap_mdio->useraccess0)) & MDIO_USERACCESS0_GO)
		;

	if (tmp & MDIO_USERACCESS0_ACK) {
		*data = tmp & 0xffff;
		return 0;
	}

	*data = -1;
	return -1;
}

/*
 * Write to a PHY register via MDIO inteface.
 * Blocks until operation is complete.
 */
int keystone2_eth_phy_write(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t data)
{
	while (readl(&adap_mdio->useraccess0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO |
	       MDIO_USERACCESS0_WRITE_WRITE |
	       ((reg_num & 0x1f) << 21) |
	       ((phy_addr & 0x1f) << 16) |
	       (data & 0xffff),
	       &adap_mdio->useraccess0);

	/* Wait for command to complete */
	while (readl(&adap_mdio->useraccess0) & MDIO_USERACCESS0_GO)
		;

	return 0;
}

/* PHY functions for a generic PHY */
static int gen_get_link_speed(int phy_addr)
{
	u_int16_t	tmp;

	if ((!keystone2_eth_phy_read(phy_addr, MII_STATUS_REG, &tmp)) &&
	    (tmp & 0x04)) {
		return 0;
	}

	return -1;
}

static void  __attribute__((unused))
	keystone2_eth_gigabit_enable(struct eth_device *dev)
{
	u_int16_t data;
	struct eth_priv_t *eth_priv = (struct eth_priv_t *)dev->priv;

	if (sys_has_mdio) {
		if (keystone2_eth_phy_read(eth_priv->phy_addr, 0, &data) ||
		    !(data & (1 << 6))) /* speed selection MSB */
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

int keystone_sgmii_link_status(int port)
{
	u32 status = 0;

	status = __raw_readl(SGMII_STATUS_REG(port));

	return status & SGMII_REG_STATUS_LINK;
}


int keystone_get_link_status(struct eth_device *dev)
{
	struct eth_priv_t *eth_priv = (struct eth_priv_t *)dev->priv;
	int sgmii_link;
	int link_state = 0;
#if CONFIG_GET_LINK_STATUS_ATTEMPTS > 1
	int j;

	for (j = 0; (j < CONFIG_GET_LINK_STATUS_ATTEMPTS) && (link_state == 0);
	     j++) {
#endif
		sgmii_link =
			keystone_sgmii_link_status(eth_priv->slave_port - 1);

		if (sgmii_link) {
			link_state = 1;

			if (eth_priv->sgmii_link_type == SGMII_LINK_MAC_PHY)
				if (gen_get_link_speed(eth_priv->phy_addr))
					link_state = 0;
		}
#if CONFIG_GET_LINK_STATUS_ATTEMPTS > 1
	}
#endif
	return link_state;
}

int keystone_sgmii_config(int port, int interface)
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

	for (i = 0; i < 1000; i++) {
		status = __raw_readl(SGMII_STATUS_REG(port));
		if ((status & mask) == mask)
			break;
	}

	return 0;
}

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
	u_int32_t clkdiv;
	int link;
	struct eth_priv_t *eth_priv = (struct eth_priv_t *)dev->priv;

	debug("+ emac_open\n");

	net_rx_buffs.rx_flow	= eth_priv->rx_flow;

	sys_has_mdio =
		(eth_priv->sgmii_link_type == SGMII_LINK_MAC_PHY) ? 1 : 0;

	keystone2_net_serdes_setup();

	if (sys_has_mdio)
		keystone2_eth_mdio_enable();

	keystone_sgmii_config(eth_priv->slave_port - 1,
			      eth_priv->sgmii_link_type);

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
		/* Init MDIO & get link state */
		clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;
		writel((clkdiv & 0xff) | MDIO_CONTROL_ENABLE |
		       MDIO_CONTROL_FAULT, &adap_mdio->control)
			;

		/* We need to wait for MDIO to start */
		udelay(1000);

		link = keystone_get_link_status(dev);
		if (link == 0) {
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
	debug("+ emac_close\n");

	if (!emac_open)
		return;

	ethss_stop();

	ksnav_close(&netcp_pktdma);
	qm_close();

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

	if (keystone_get_link_status(dev) == 0)
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

	NetReceive((uchar *)pkt, pkt_size);

	ksnav_release_rxhd(&netcp_pktdma, hd);

	return pkt_size;
}

/*
 * This function initializes the EMAC hardware.
 */
int keystone2_emac_initialize(struct eth_priv_t *eth_priv)
{
	struct eth_device *dev;

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

	eth_register(dev);

	return 0;
}

struct ks2_serdes ks2_serdes_sgmii_156p25mhz = {
	.clk = SERDES_CLOCK_156P25M,
	.rate = SERDES_RATE_5G,
	.rate_mode = SERDES_QUARTER_RATE,
	.intf = SERDES_PHY_SGMII,
	.loopback = 0,
};

static void keystone2_net_serdes_setup(void)
{
	ks2_serdes_init(CONFIG_KSNET_SERDES_SGMII_BASE,
			&ks2_serdes_sgmii_156p25mhz,
			CONFIG_KSNET_SERDES_LANES_PER_SGMII);

	/* wait till setup */
	udelay(5000);
}
