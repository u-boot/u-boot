// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Hisilicon Fast Ethernet MAC Driver
 * Adapted from linux
 *
 * Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
 * Copyright (c) 2023 Yang Xiwen <forbidden405@outlook.com>
 */

#include <dm.h>
#include <clk.h>
#include <cpu_func.h>
#include <miiphy.h>
#include <net.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <linux/bitfield.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/kernel.h>

/* MAC control register list */
#define MAC_PORTSEL			0x0200
#define MAC_PORTSEL_STAT_CPU		BIT(0)
#define MAC_PORTSEL_RMII		BIT(1)
#define MAC_PORTSET			0x0208
#define MAC_PORTSET_DUPLEX_FULL		BIT(0)
#define MAC_PORTSET_LINKED		BIT(1)
#define MAC_PORTSET_SPEED_100M		BIT(2)
#define MAC_SET				0x0210
#define MAX_FRAME_SIZE			1600
#define MAX_FRAME_SIZE_MASK		GENMASK(10, 0)
#define BIT_PAUSE_EN			BIT(18)
#define RX_COALESCE_SET			0x0340
#define RX_COALESCED_FRAME_OFFSET	24
#define RX_COALESCED_FRAMES		8
#define RX_COALESCED_TIMER		0x74
#define QLEN_SET			0x0344
#define RX_DEPTH_OFFSET			8
#define MAX_HW_FIFO_DEPTH		64
#define HW_TX_FIFO_DEPTH		1
#define MAX_HW_RX_FIFO_DEPTH		(MAX_HW_FIFO_DEPTH - HW_TX_FIFO_DEPTH)
#define HW_RX_FIFO_DEPTH		min(PKTBUFSRX, MAX_HW_RX_FIFO_DEPTH)
#define IQFRM_DES			0x0354
#define RX_FRAME_LEN_MASK		GENMASK(11, 0)
#define RX_FRAME_IN_INDEX_MASK		GENMASK(17, 12)
#define IQ_ADDR				0x0358
#define EQ_ADDR				0x0360
#define EQFRM_LEN			0x0364
#define ADDRQ_STAT			0x036C
#define TX_CNT_INUSE_MASK		GENMASK(5, 0)
#define BIT_TX_READY			BIT(24)
#define BIT_RX_READY			BIT(25)
/* global control register list */
#define GLB_HOSTMAC_L32			0x0000
#define GLB_HOSTMAC_H16			0x0004
#define GLB_SOFT_RESET			0x0008
#define SOFT_RESET_ALL			BIT(0)
#define GLB_FWCTRL			0x0010
#define FWCTRL_VLAN_ENABLE		BIT(0)
#define FWCTRL_FW2CPU_ENA		BIT(5)
#define FWCTRL_FWALL2CPU		BIT(7)
#define GLB_MACTCTRL			0x0014
#define MACTCTRL_UNI2CPU		BIT(1)
#define MACTCTRL_MULTI2CPU		BIT(3)
#define MACTCTRL_BROAD2CPU		BIT(5)
#define MACTCTRL_MACT_ENA		BIT(7)
#define GLB_IRQ_STAT			0x0030
#define GLB_IRQ_ENA			0x0034
#define IRQ_ENA_PORT0_MASK		GENMASK(7, 0)
#define IRQ_ENA_PORT0			BIT(18)
#define IRQ_ENA_ALL			BIT(19)
#define GLB_IRQ_RAW			0x0038
#define IRQ_INT_RX_RDY			BIT(0)
#define IRQ_INT_TX_PER_PACKET		BIT(1)
#define IRQ_INT_TX_FIFO_EMPTY		BIT(6)
#define IRQ_INT_MULTI_RXRDY		BIT(7)
#define DEF_INT_MASK			(IRQ_INT_MULTI_RXRDY | \
					IRQ_INT_TX_PER_PACKET | \
					IRQ_INT_TX_FIFO_EMPTY)
#define GLB_MAC_L32_BASE		0x0100
#define GLB_MAC_H16_BASE		0x0104
#define MACFLT_HI16_MASK		GENMASK(15, 0)
#define BIT_MACFLT_ENA			BIT(17)
#define BIT_MACFLT_FW2CPU		BIT(21)
#define GLB_MAC_H16(reg)		(GLB_MAC_H16_BASE + ((reg) * 0x8))
#define GLB_MAC_L32(reg)		(GLB_MAC_L32_BASE + ((reg) * 0x8))
#define MAX_MAC_FILTER_NUM		8
#define MAX_UNICAST_ADDRESSES		2
#define MAX_MULTICAST_ADDRESSES		(MAX_MAC_FILTER_NUM - \
					MAX_UNICAST_ADDRESSES)
/* software tx and rx queue number, should be power of 2 */
#define TXQ_NUM				64
#define RXQ_NUM				128

#define PHY_RESET_DELAYS_PROPERTY	"hisilicon,phy-reset-delays-us"
#define MAC_RESET_DELAY_PROPERTY	"hisilicon,mac-reset-delay-us"
#define MAC_RESET_ASSERT_PERIOD		200000

enum phy_reset_delays {
	PRE_DELAY,
	PULSE,
	POST_DELAY,
	DELAYS_NUM,
};

enum clk_type {
	CLK_MAC,
	CLK_BUS,
	CLK_PHY,
	CLK_NUM,
};

struct hisi_femac_priv {
	void __iomem *port_base;
	void __iomem *glb_base;
	struct clk *clks[CLK_NUM];
	struct reset_ctl *mac_rst;
	struct reset_ctl *phy_rst;
	u32 phy_reset_delays[DELAYS_NUM];
	u32 mac_reset_delay;

	struct phy_device *phy;

	u32 link_status;
};

struct hisi_femac_stat_entry {
	const char *name;
	u32 offset;
	u32 mask;
};

/* please refer to the datasheet for the description of these entries */
static const struct hisi_femac_stat_entry hisi_femac_stats_table[] = {
	{ "rxsof_cnt",		0x584,	GENMASK(31, 28) },
	{ "rxeof_cnt",		0x584,	GENMASK(27, 24) },
	{ "rxcrcok_cnt",	0x584,	GENMASK(23, 20) },
	{ "rxcrcbad_cnt",	0x584,	GENMASK(19, 16) },
	{ "txsof_cnt",		0x584,	GENMASK(15, 12) },
	{ "txeof_cnt",		0x584,	GENMASK(11, 8) },
	{ "txcrcok_cnt",	0x584,	GENMASK(7, 4) },
	{ "txcrcbad_cnt",	0x584,	GENMASK(3, 0) },
	{ "pkts_cpu",		0x5a0,	GENMASK(15, 0) },
	{ "addr_cpu",		0x5a4,	GENMASK(15, 0) },
	{ "pkts_port",		0x5a8,	GENMASK(15, 0) },
	{ "pkts_cpu2tx",	0x5ac,	GENMASK(15, 0) },
	{ "rxdvrise",		0x600,	GENMASK(31, 0) },
	{ "ifinoctets",		0x604,	GENMASK(31, 0) },
	{ "octets_rx",		0x608,	GENMASK(31, 0) },
	{ "local_mac_match",	0x60c,	GENMASK(31, 0) },
	{ "pkts",		0x610,	GENMASK(31, 0) },
	{ "broadcastpkts",	0x614,	GENMASK(31, 0) },
	{ "multicastpkts",	0x618,	GENMASK(31, 0) },
	{ "ifinucastpkts",	0x61c,	GENMASK(31, 0) },
	{ "ifinerrors",		0x620,	GENMASK(31, 0) },
	{ "crcerr",		0x624,	GENMASK(31, 0) },
	{ "abnormalsizepkts",	0x628,	GENMASK(31, 0) },
	{ "dot3alignmenterr",	0x62c,	GENMASK(31, 0) },
	{ "dot3pause",		0x630,	GENMASK(31, 0) },
	{ "dropevents",		0x634,	GENMASK(31, 0) },
	{ "flux_frame_cnt",	0x638,	GENMASK(31, 0) },
	{ "flux_drop_cnt",	0x63c,	GENMASK(31, 0) },
	{ "mac_not2cpu_pkts",	0x64c,	GENMASK(31, 0) },
	{ "pkts_tx",		0x780,	GENMASK(31, 0) },
	{ "broadcastpkts_tx",	0x784,	GENMASK(31, 0) },
	{ "multicastpkts_tx",	0x788,	GENMASK(31, 0) },
	{ "ifoutucastpkts_tx",	0x78c,	GENMASK(31, 0) },
	{ "octets_tx",		0x790,	GENMASK(31, 0) },
	{ "dot3pause",		0x794,	GENMASK(31, 0) },
	{ "retry_times_tx",	0x798,	GENMASK(31, 0) },
	{ "collisions",		0x79c,	GENMASK(31, 0) },
	{ "dot3latecol",	0x7a0,	GENMASK(31, 0) },
	{ "dot3colok",		0x7a4,	GENMASK(31, 0) },
	{ "dot3excessivecol",	0x7a8,	GENMASK(31, 0) },
	{ "dot3colcnt",		0x7ac,	GENMASK(31, 0) },
};

static void hisi_femac_irq_enable(struct hisi_femac_priv *priv, int irqs)
{
	u32 val;

	val = readl(priv->glb_base + GLB_IRQ_ENA);
	writel(val | irqs, priv->glb_base + GLB_IRQ_ENA);
}

static void hisi_femac_irq_disable(struct hisi_femac_priv *priv, int irqs)
{
	u32 val;

	val = readl(priv->glb_base + GLB_IRQ_ENA);
	writel(val & (~irqs), priv->glb_base + GLB_IRQ_ENA);
}

static void hisi_femac_port_init(struct hisi_femac_priv *priv)
{
	u32 val;

	/* MAC gets link status info and phy mode by software config */
	val = MAC_PORTSEL_STAT_CPU;
	if (priv->phy->interface == PHY_INTERFACE_MODE_RMII)
		val |= MAC_PORTSEL_RMII;
	writel(val, priv->port_base + MAC_PORTSEL);

	/*clear all interrupt status */
	writel(IRQ_ENA_PORT0_MASK, priv->glb_base + GLB_IRQ_RAW);
	hisi_femac_irq_disable(priv, IRQ_ENA_PORT0_MASK | IRQ_ENA_PORT0);

	val = readl(priv->glb_base + GLB_FWCTRL);
	val &= ~(FWCTRL_VLAN_ENABLE | FWCTRL_FWALL2CPU);
	val |= FWCTRL_FW2CPU_ENA;
	writel(val, priv->glb_base + GLB_FWCTRL);

	val = readl(priv->glb_base + GLB_MACTCTRL);
	val |= (MACTCTRL_BROAD2CPU | MACTCTRL_MACT_ENA);
	writel(val, priv->glb_base + GLB_MACTCTRL);

	val = readl(priv->port_base + MAC_SET);
	val &= ~MAX_FRAME_SIZE_MASK;
	val |= MAX_FRAME_SIZE;
	writel(val, priv->port_base + MAC_SET);

	val = RX_COALESCED_TIMER |
		(RX_COALESCED_FRAMES << RX_COALESCED_FRAME_OFFSET);
	writel(val, priv->port_base + RX_COALESCE_SET);

	val = (HW_RX_FIFO_DEPTH << RX_DEPTH_OFFSET) | HW_TX_FIFO_DEPTH;
	writel(val, priv->port_base + QLEN_SET);
}

static void hisi_femac_rx_refill(struct hisi_femac_priv *priv)
{
	int i;
	ulong addr;

	for (i = 0; i < HW_RX_FIFO_DEPTH; i++) {
		addr = (ulong)net_rx_packets[i];
		writel(addr, priv->port_base + IQ_ADDR);
	}
}

static void hisi_femac_adjust_link(struct udevice *dev)
{
	struct hisi_femac_priv *priv = dev_get_priv(dev);
	struct phy_device *phy = priv->phy;
	u32 status = 0;

	if (phy->link)
		status |= MAC_PORTSET_LINKED;
	if (phy->duplex == DUPLEX_FULL)
		status |= MAC_PORTSET_DUPLEX_FULL;
	if (phy->speed == SPEED_100)
		status |= MAC_PORTSET_SPEED_100M;

	writel(status, priv->port_base + MAC_PORTSET);
}

static int hisi_femac_port_reset(struct hisi_femac_priv *priv)
{
	u32 val;

	val = readl(priv->glb_base + GLB_SOFT_RESET);
	val |= SOFT_RESET_ALL;
	writel(val, priv->glb_base + GLB_SOFT_RESET);

	udelay(800);

	val &= ~SOFT_RESET_ALL;
	writel(val, priv->glb_base + GLB_SOFT_RESET);

	return 0;
}

static int hisi_femac_set_hw_mac_addr(struct udevice *dev)
{
	struct hisi_femac_priv *priv = dev_get_priv(dev);
	struct eth_pdata *plat = dev_get_plat(dev);
	unsigned char *mac = plat->enetaddr;
	u32 reg;

	reg = mac[1] | (mac[0] << 8);
	writel(reg, priv->glb_base + GLB_HOSTMAC_H16);

	reg = mac[5] | (mac[4] << 8) | (mac[3] << 16) | (mac[2] << 24);
	writel(reg, priv->glb_base + GLB_HOSTMAC_L32);

	return 0;
}

static int hisi_femac_start(struct udevice *dev)
{
	int ret;
	struct hisi_femac_priv *priv = dev_get_priv(dev);

	hisi_femac_port_reset(priv);
	hisi_femac_set_hw_mac_addr(dev);
	hisi_femac_rx_refill(priv);

	ret = phy_startup(priv->phy);
	if (ret) {
		dev_err(dev, "Failed to startup phy: %d\n", ret);
		return log_msg_ret("phy", ret);
	}

	if (!priv->phy->link) {
		debug("%s: link down\n", __func__);
		return -ENODEV;
	}

	hisi_femac_adjust_link(dev);

	writel(IRQ_ENA_PORT0_MASK, priv->glb_base + GLB_IRQ_RAW);
	hisi_femac_irq_enable(priv, IRQ_ENA_ALL | IRQ_ENA_PORT0 | DEF_INT_MASK);

	return 0;
}

static int hisi_femac_send(struct udevice *dev, void *packet, int length)
{
	struct hisi_femac_priv *priv = dev_get_priv(dev);
	ulong addr = (ulong)packet;
	int ret;

	// clear previous irq
	writel(IRQ_INT_TX_PER_PACKET, priv->glb_base + GLB_IRQ_RAW);

	// flush cache
	flush_cache(addr, length + ETH_FCS_LEN);

	// write packet address
	writel(addr, priv->port_base + EQ_ADDR);

	// write packet length (and send it)
	writel(length + ETH_FCS_LEN, priv->port_base + EQFRM_LEN);

	// wait until FIFO is empty
	ret = wait_for_bit_le32(priv->glb_base + GLB_IRQ_RAW, IRQ_INT_TX_PER_PACKET, true, 50, false);
	if (ret == -ETIMEDOUT) {
		dev_err(dev, "FIFO timeout\n");
		return log_msg_ret("net", ret);
	}

	return 0;
}

static int hisi_femac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct hisi_femac_priv *priv = dev_get_priv(dev);
	int val, index, length;

	val = readl(priv->glb_base + GLB_IRQ_RAW);
	if (!(val & IRQ_INT_RX_RDY))
		return -EAGAIN;

	val = readl(priv->port_base + IQFRM_DES);
	index = (val & RX_FRAME_IN_INDEX_MASK) >> 12;
	length = val & RX_FRAME_LEN_MASK;

	// invalidate cache
	invalidate_dcache_range((ulong)net_rx_packets[index], (ulong)net_rx_packets[index] + length);
	*packetp = net_rx_packets[index];

	// Tell hardware we will process the packet
	writel(IRQ_INT_RX_RDY, priv->glb_base + GLB_IRQ_RAW);

	return length;
}

static int hisi_femac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct hisi_femac_priv *priv = dev_get_priv(dev);
	ulong addr = (ulong)packet;

	// Tell hardware the packet can be reused
	writel(addr, priv->port_base + IQ_ADDR);

	return 0;
}

static void hisi_femac_stop(struct udevice *dev)
{
	struct hisi_femac_priv *priv = dev_get_priv(dev);

	// assert internal reset
	writel(SOFT_RESET_ALL, priv->glb_base + GLB_SOFT_RESET);
}

static int hisi_femac_get_sset_count(struct udevice *dev)
{
	return ARRAY_SIZE(hisi_femac_stats_table);
}

static void hisi_femac_get_strings(struct udevice *dev, u8 *data)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hisi_femac_stats_table); i++)
		strcpy(data + i * ETH_GSTRING_LEN, hisi_femac_stats_table[i].name);
}

/* Non-constant mask variant of FIELD_GET/FIELD_PREP */
#define field_get(_mask, _reg) (((_reg) & (_mask)) >> (ffs(_mask) - 1))

static void hisi_femac_get_stats(struct udevice *dev, u64 *data)
{
	int i;
	u32 mask, reg;
	struct hisi_femac_priv *priv = dev_get_priv(dev);
	void __iomem *port_base = priv->port_base;

	for (i = 0; i < ARRAY_SIZE(hisi_femac_stats_table); i++) {
		mask = hisi_femac_stats_table[i].mask;
		reg = readl(port_base + hisi_femac_stats_table[i].offset);

		data[i] = field_get(mask, reg);
	}
}

static int hisi_femac_of_to_plat(struct udevice *dev)
{
	int ret, i;
	struct hisi_femac_priv *priv = dev_get_priv(dev);
	ofnode mdio_node;
	bool mdio_registered = false;
	static const char * const clk_strs[] = {
		[CLK_MAC] = "mac",
		[CLK_BUS] = "bus",
		[CLK_PHY] = "phy",
	};

	priv->port_base = dev_remap_addr_name(dev, "port");
	if (!priv->port_base) {
		dev_err(dev, "Failed to remap port address space\n");
		return log_msg_ret("net", -EINVAL);
	}

	priv->glb_base = dev_remap_addr_name(dev, "glb");
	if (IS_ERR(priv->glb_base)) {
		dev_err(dev, "Failed to remap global address space\n");
		return log_msg_ret("net", -EINVAL);
	}

	for (i = 0; i < ARRAY_SIZE(clk_strs); i++) {
		priv->clks[i] = devm_clk_get(dev, clk_strs[i]);
		if (IS_ERR(priv->clks[i])) {
			dev_err(dev, "Error getting clock %s\n", clk_strs[i]);
			return log_msg_ret("clk", PTR_ERR(priv->clks[i]));
		}
	}

	priv->mac_rst = devm_reset_control_get(dev, "mac");
	if (IS_ERR(priv->mac_rst)) {
		dev_err(dev, "Failed to get MAC reset %ld\n", PTR_ERR(priv->mac_rst));
		return log_msg_ret("rst", PTR_ERR(priv->mac_rst));
	}

	priv->phy_rst = devm_reset_control_get(dev, "phy");
	if (IS_ERR(priv->phy_rst)) {
		dev_err(dev, "Failed to get PHY reset %ld\n", PTR_ERR(priv->phy_rst));
		return log_msg_ret("rst", PTR_ERR(priv->phy_rst));
	}

	ret = dev_read_u32_array(dev,
				 PHY_RESET_DELAYS_PROPERTY,
				 priv->phy_reset_delays,
				 DELAYS_NUM);
	if (ret < 0) {
		dev_err(dev, "Failed to get PHY reset delays %d\n", ret);
		return log_msg_ret("rst", ret);
	}

	priv->mac_reset_delay = dev_read_u32_default(dev,
						     MAC_RESET_DELAY_PROPERTY,
						     MAC_RESET_ASSERT_PERIOD);

	/* Create MDIO bus */
	ofnode_for_each_subnode(mdio_node, dev_ofnode(dev)) {
		const char *subnode_name = ofnode_get_name(mdio_node);
		struct udevice *mdiodev;

		// Skip subnodes not starting with "mdio"
		if (strncmp(subnode_name, "mdio", 4))
			continue;

		ret = device_bind_driver_to_node(dev, "hisi-femac-mdio",
						 subnode_name, mdio_node, &mdiodev);
		if (ret) {
			dev_err(dev, "Failed to register MDIO bus device %d\n", ret);
			return log_msg_ret("net", ret);
		}

		mdio_registered = true;
		break;
	}

	if (!mdio_registered) {
		dev_err(dev, "No MDIO subnode is found!\n");
		return log_msg_ret("mdio", -ENODATA);
	}

	return 0;
}

static int hisi_femac_phy_reset(struct hisi_femac_priv *priv)
{
	struct reset_ctl *rst = priv->phy_rst;
	u32 *delays = priv->phy_reset_delays;
	int ret;

	// Disable MAC clk before phy reset
	ret = clk_disable(priv->clks[CLK_MAC]);
	if (ret < 0) {
		pr_err("%s: Failed to disable MAC clock %d\n", __func__, ret);
		return log_msg_ret("clk", ret);
	}
	ret = clk_disable(priv->clks[CLK_BUS]);
	if (ret < 0) {
		pr_err("%s: Failed to disable bus clock %d\n", __func__, ret);
		return log_msg_ret("clk", ret);
	}

	udelay(delays[PRE_DELAY]);

	ret = reset_assert(rst);
	if (ret < 0) {
		pr_err("%s: Failed to assert reset %d\n", __func__, ret);
		return log_msg_ret("rst", ret);
	}

	udelay(delays[PULSE]);

	ret = reset_deassert(rst);
	if (ret < 0) {
		pr_err("%s: Failed to deassert reset %d\n", __func__, ret);
		return log_msg_ret("rst", ret);
	}

	udelay(delays[POST_DELAY]);

	ret = clk_enable(priv->clks[CLK_MAC]);
	if (ret < 0) {
		pr_err("%s: Failed to enable MAC clock %d\n", __func__, ret);
		return log_msg_ret("clk", ret);
	}
	ret = clk_enable(priv->clks[CLK_BUS]);
	if (ret < 0) {
		pr_err("%s: Failed to enable MAC bus clock %d\n", __func__, ret);
		return log_msg_ret("clk", ret);
	}

	return 0;
}

static int hisi_femac_probe(struct udevice *dev)
{
	struct hisi_femac_priv *priv = dev_get_priv(dev);
	int ret, i;

	// Enable clocks
	for (i = 0; i < CLK_NUM; i++) {
		ret = clk_prepare_enable(priv->clks[i]);
		if (ret < 0) {
			dev_err(dev, "Failed to enable clk %d: %d\n", i, ret);
			return log_msg_ret("clk", ret);
		}
	}

	// Reset MAC
	ret = reset_assert(priv->mac_rst);
	if (ret < 0) {
		dev_err(dev, "Failed to assert MAC reset: %d\n", ret);
		return log_msg_ret("net", ret);
	}

	udelay(priv->mac_reset_delay);

	ret = reset_deassert(priv->mac_rst);
	if (ret < 0) {
		dev_err(dev, "Failed to deassert MAC reset: %d\n", ret);
		return log_msg_ret("net", ret);
	}

	// Reset PHY
	ret = hisi_femac_phy_reset(priv);
	if (ret < 0) {
		dev_err(dev, "Failed to reset PHY: %d\n", ret);
		return log_msg_ret("net", ret);
	}

	// Connect to PHY
	priv->phy = dm_eth_phy_connect(dev);
	if (!priv->phy) {
		dev_err(dev, "Failed to connect to phy\n");
		return log_msg_ret("phy", -EINVAL);
	}

	hisi_femac_port_init(priv);
	return 0;
}

static const struct eth_ops hisi_femac_ops = {
	.start		= hisi_femac_start,
	.send		= hisi_femac_send,
	.recv		= hisi_femac_recv,
	.free_pkt	= hisi_femac_free_pkt,
	.stop		= hisi_femac_stop,
	.write_hwaddr	= hisi_femac_set_hw_mac_addr,
	.get_sset_count	= hisi_femac_get_sset_count,
	.get_strings	= hisi_femac_get_strings,
	.get_stats	= hisi_femac_get_stats,
};

static const struct udevice_id hisi_femac_ids[] = {
	{.compatible = "hisilicon,hisi-femac-v1",},
	{.compatible = "hisilicon,hisi-femac-v2",},
	{.compatible = "hisilicon,hi3516cv300-femac",},
	{.compatible = "hisilicon,hi3798mv200-femac",},
	{},
};

U_BOOT_DRIVER(hisi_femac_driver) = {
	.name = "eth_hisi_femac",
	.id = UCLASS_ETH,
	.of_match = of_match_ptr(hisi_femac_ids),
	.of_to_plat = hisi_femac_of_to_plat,
	.ops = &hisi_femac_ops,
	.probe = hisi_femac_probe,
	.plat_auto = sizeof(struct eth_pdata),
	.priv_auto = sizeof(struct hisi_femac_priv),
};
