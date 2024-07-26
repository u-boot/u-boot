// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 * Copyright (c) 2021 Toco Technologies FZE <contact@toco.ae>
 * Copyright (c) 2021 Gabor Juhos <j4g8y7@gmail.com>
 *
 * Qualcomm ESS EDMA ethernet driver
 */

#include <asm/io.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <log.h>
#include <miiphy.h>
#include <net.h>
#include <reset.h>

#include "essedma.h"

#define EDMA_MAX_PKT_SIZE	(PKTSIZE_ALIGN + PKTALIGN)

#define EDMA_RXQ_ID	0
#define EDMA_TXQ_ID	0

/* descriptor ring */
struct edma_ring {
	u16 count; /* number of descriptors in the ring */
	void *hw_desc; /* descriptor ring virtual address */
	unsigned int hw_size; /* hw descriptor ring length in bytes */
	dma_addr_t dma; /* descriptor ring physical address */
	u16 head; /* next Tx descriptor to fill */
	u16 tail; /* next Tx descriptor to clean */
};

struct ess_switch {
	phys_addr_t base;
	struct phy_device *phydev[ESS_PORTS_NUM];
	u32 phy_mask;
	ofnode ports_node;
	phy_interface_t port_wrapper_mode;
	int num_phy;
};

struct essedma_priv {
	phys_addr_t base;
	struct udevice *dev;
	struct clk ess_clk;
	struct reset_ctl ess_rst;
	struct udevice *mdio_dev;
	struct ess_switch esw;
	phys_addr_t psgmii_base;
	struct edma_ring tpd_ring;
	struct edma_ring rfd_ring;
};

static void esw_port_loopback_set(struct ess_switch *esw, int port,
				  bool enable)
{
	u32 t;

	t = readl(esw->base + ESS_PORT_LOOKUP_CTRL(port));
	if (enable)
		t |= ESS_PORT_LOOP_BACK_EN;
	else
		t &= ~ESS_PORT_LOOP_BACK_EN;
	writel(t, esw->base + ESS_PORT_LOOKUP_CTRL(port));
}

static void esw_port_loopback_set_all(struct ess_switch *esw, bool enable)
{
	int i;

	for (i = 1; i < ESS_PORTS_NUM; i++)
		esw_port_loopback_set(esw, i, enable);
}

static void ess_reset(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);

	reset_assert(&priv->ess_rst);
	mdelay(10);

	reset_deassert(&priv->ess_rst);
	mdelay(10);
}

void qca8075_ess_reset(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	struct phy_device *psgmii_phy;
	int i, val;

	/* Find the PSGMII PHY */
	psgmii_phy = priv->esw.phydev[priv->esw.num_phy - 1];

	/* Fix phy psgmii RX 20bit */
	phy_write(psgmii_phy, MDIO_DEVAD_NONE, MII_BMCR, 0x005b);

	/* Reset phy psgmii */
	phy_write(psgmii_phy, MDIO_DEVAD_NONE, MII_BMCR, 0x001b);

	/* Release reset phy psgmii */
	phy_write(psgmii_phy, MDIO_DEVAD_NONE, MII_BMCR, 0x005b);
	for (i = 0; i < 100; i++) {
		val = phy_read_mmd(psgmii_phy, MDIO_MMD_PMAPMD, 0x28);
		if (val & 0x1)
			break;
		mdelay(1);
	}
	if (i >= 100)
		printf("QCA807x PSGMII PLL_VCO_CALIB Not Ready\n");

	/*
	 * Check qca8075 psgmii calibration done end.
	 * Freeze phy psgmii RX CDR
	 */
	phy_write(psgmii_phy, MDIO_DEVAD_NONE, 0x1a, 0x2230);

	ess_reset(dev);

	/* Check ipq psgmii calibration done start */
	for (i = 0; i < 100; i++) {
		val = readl(priv->psgmii_base + PSGMIIPHY_VCO_CALIBRATION_CTRL_REGISTER_2);
		if (val & 0x1)
			break;
		mdelay(1);
	}
	if (i >= 100)
		printf("PSGMII PLL_VCO_CALIB Not Ready\n");

	/*
	 * Check ipq psgmii calibration done end.
	 * Relesae phy psgmii RX CDR
	 */
	phy_write(psgmii_phy, MDIO_DEVAD_NONE, 0x1a, 0x3230);

	/* Release phy psgmii RX 20bit */
	phy_write(psgmii_phy, MDIO_DEVAD_NONE, MII_BMCR, 0x005f);
}

#define PSGMII_ST_NUM_RETRIES		20
#define PSGMII_ST_PKT_COUNT		(4 * 1024)
#define PSGMII_ST_PKT_SIZE		1504

/*
 * Transmitting one byte over a 1000Mbps link requires 8 ns.
 * Additionally, use + 1 ns for safety to compensate latencies
 * and such.
 */
#define PSGMII_ST_TRAFFIC_TIMEOUT_NS	\
	(PSGMII_ST_PKT_COUNT * PSGMII_ST_PKT_SIZE * (8 + 1))

#define PSGMII_ST_TRAFFIC_TIMEOUT	\
	DIV_ROUND_UP(PSGMII_ST_TRAFFIC_TIMEOUT_NS, 1000000)

static bool psgmii_self_test_repeat;

static void psgmii_st_phy_power_down(struct phy_device *phydev)
{
	int val;

	val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	val |= QCA807X_POWER_DOWN;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, val);
}

static void psgmii_st_phy_prepare(struct phy_device *phydev)
{
	int val;

	/* check phydev combo port */
	val = phy_read(phydev, MDIO_DEVAD_NONE,
		       QCA807X_CHIP_CONFIGURATION);
	if (val) {
		/* Select copper page */
		val |= QCA807X_MEDIA_PAGE_SELECT;
		phy_write(phydev, MDIO_DEVAD_NONE,
			  QCA807X_CHIP_CONFIGURATION, val);
	}

	/* Force no link by power down */
	psgmii_st_phy_power_down(phydev);

	/* Packet number (Non documented) */
	phy_write_mmd(phydev, MDIO_MMD_AN, 0x8021, PSGMII_ST_PKT_COUNT);
	phy_write_mmd(phydev, MDIO_MMD_AN, 0x8062, PSGMII_ST_PKT_SIZE);

	/* Fix MDI status */
	val = phy_read(phydev, MDIO_DEVAD_NONE, QCA807X_FUNCTION_CONTROL);
	val &= ~QCA807X_MDI_CROSSOVER_MODE_MASK;
	val |= FIELD_PREP(QCA807X_MDI_CROSSOVER_MODE_MASK,
			  QCA807X_MDI_CROSSOVER_MODE_MANUAL_MDI);
	val &= ~QCA807X_POLARITY_REVERSAL;
	phy_write(phydev, MDIO_DEVAD_NONE, QCA807X_FUNCTION_CONTROL, val);
}

static void psgmii_st_phy_recover(struct phy_device *phydev)
{
	int val;

	/* Packet number (Non documented) */
	phy_write_mmd(phydev, MDIO_MMD_AN, 0x8021, 0x0);

	/* Disable CRC checker and packet counter */
	val = phy_read_mmd(phydev, MDIO_MMD_AN, QCA807X_MMD7_CRC_PACKET_COUNTER);
	val &= ~QCA807X_MMD7_PACKET_COUNTER_SELFCLR;
	val &= ~QCA807X_MMD7_CRC_PACKET_COUNTER_EN;
	phy_write_mmd(phydev, MDIO_MMD_AN, QCA807X_MMD7_CRC_PACKET_COUNTER, val);

	/* Disable traffic (Undocumented) */
	phy_write_mmd(phydev, MDIO_MMD_AN, 0x8020, 0x0);
}

static void psgmii_st_phy_start_traffic(struct phy_device *phydev)
{
	int val;

	/* Enable CRC checker and packet counter */
	val = phy_read_mmd(phydev, MDIO_MMD_AN, QCA807X_MMD7_CRC_PACKET_COUNTER);
	val |= QCA807X_MMD7_CRC_PACKET_COUNTER_EN;
	phy_write_mmd(phydev, MDIO_MMD_AN, QCA807X_MMD7_CRC_PACKET_COUNTER, val);

	/* Start traffic (Undocumented) */
	phy_write_mmd(phydev, MDIO_MMD_AN, 0x8020, 0xa000);
}

static bool psgmii_st_phy_check_counters(struct phy_device *phydev)
{
	u32 tx_ok;

	/*
	 * The number of test packets is limited to 65535 so
	 * only read the lower 16 bits of the counter.
	 */
	tx_ok = phy_read_mmd(phydev, MDIO_MMD_AN,
			     QCA807X_MMD7_VALID_EGRESS_COUNTER_2);

	return (tx_ok == PSGMII_ST_PKT_COUNT);
}

static void psgmii_st_phy_reset_loopback(struct phy_device *phydev)
{
	/* reset the PHY */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, 0x9000);

	/* enable loopback mode */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, 0x4140);
}

static inline bool psgmii_st_phy_link_is_up(struct phy_device *phydev)
{
	int val;

	val = phy_read(phydev, MDIO_DEVAD_NONE, QCA807X_PHY_SPECIFIC);
	return !!(val & QCA807X_PHY_SPECIFIC_LINK);
}

static bool psgmii_st_phy_wait(struct ess_switch *esw, u32 mask,
			       int retries, int delay,
			       bool (*check)(struct phy_device *))
{
	int i;

	for (i = 0; i < retries; i++) {
		int phy;

		for (phy = 0; phy < esw->num_phy - 1; phy++) {
			u32 phybit = BIT(phy);

			if (!(mask & phybit))
				continue;

			if (check(esw->phydev[phy]))
				mask &= ~phybit;
		}

		if (!mask)
			break;

		mdelay(delay);
	}

	return (!mask);
}

static bool psgmii_st_phy_wait_link(struct ess_switch *esw, u32 mask)
{
	return psgmii_st_phy_wait(esw, mask, 100, 10,
				  psgmii_st_phy_link_is_up);
}

static bool psgmii_st_phy_wait_tx_complete(struct ess_switch *esw, u32 mask)
{
	return psgmii_st_phy_wait(esw, mask, PSGMII_ST_TRAFFIC_TIMEOUT, 1,
				  psgmii_st_phy_check_counters);
}

static bool psgmii_st_run_test_serial(struct ess_switch *esw)
{
	bool result = true;
	int i;

	for (i = 0; i < esw->num_phy - 1; i++) {
		struct phy_device *phydev = esw->phydev[i];

		psgmii_st_phy_reset_loopback(phydev);

		psgmii_st_phy_wait_link(esw, BIT(i));

		psgmii_st_phy_start_traffic(phydev);

		/* wait for the traffic to complete */
		result &= psgmii_st_phy_wait_tx_complete(esw, BIT(i));

		/* Power down */
		psgmii_st_phy_power_down(phydev);

		if (!result)
			break;
	}

	return result;
}

static bool psgmii_st_run_test_parallel(struct ess_switch *esw)
{
	bool result;
	int i;

	/* enable loopback mode on all PHYs */
	for (i = 0; i < esw->num_phy - 1; i++)
		psgmii_st_phy_reset_loopback(esw->phydev[i]);

	psgmii_st_phy_wait_link(esw, esw->phy_mask);

	/* start traffic on all PHYs parallely */
	for (i = 0; i < esw->num_phy - 1; i++)
		psgmii_st_phy_start_traffic(esw->phydev[i]);

	/* wait for the traffic to complete on all PHYs */
	result = psgmii_st_phy_wait_tx_complete(esw, esw->phy_mask);

	/* Power down all PHYs */
	for (i = 0; i < esw->num_phy - 1; i++)
		psgmii_st_phy_power_down(esw->phydev[i]);

	return result;
}

struct psgmii_st_stats {
	int succeed;
	int failed;
	int failed_max;
	int failed_cont;
};

static void psgmii_st_update_stats(struct psgmii_st_stats *stats,
				   bool success)
{
	if (success) {
		stats->succeed++;
		stats->failed_cont = 0;
		return;
	}

	stats->failed++;
	stats->failed_cont++;
	if (stats->failed_max < stats->failed_cont)
		stats->failed_max = stats->failed_cont;
}

static void psgmii_self_test(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	struct ess_switch *esw = &priv->esw;
	struct psgmii_st_stats stats;
	bool result = false;
	unsigned long tm;
	int i;

	memset(&stats, 0, sizeof(stats));

	tm = get_timer(0);

	for (i = 0; i < esw->num_phy - 1; i++)
		psgmii_st_phy_prepare(esw->phydev[i]);

	for (i = 0; i < PSGMII_ST_NUM_RETRIES; i++) {
		qca8075_ess_reset(dev);

		/* enable loopback mode on the switch's ports */
		esw_port_loopback_set_all(esw, true);

		/* run test on each PHYs individually after each other */
		result = psgmii_st_run_test_serial(esw);

		if (result) {
			/* run test on each PHYs parallely */
			result = psgmii_st_run_test_parallel(esw);
		}

		psgmii_st_update_stats(&stats, result);

		if (psgmii_self_test_repeat)
			continue;

		if (result)
			break;
	}

	for (i = 0; i < esw->num_phy - 1; i++) {
		/* Configuration recover */
		psgmii_st_phy_recover(esw->phydev[i]);

		/* Disable loopback */
		phy_write(esw->phydev[i], MDIO_DEVAD_NONE,
			  QCA807X_FUNCTION_CONTROL, 0x6860);
		phy_write(esw->phydev[i], MDIO_DEVAD_NONE, MII_BMCR, 0x9040);
	}

	/* disable loopback mode on the switch's ports */
	esw_port_loopback_set_all(esw, false);

	tm = get_timer(tm);
	dev_dbg(priv->dev, "\nPSGMII self-test: succeed %d, failed %d (max %d), duration %lu.%03lu secs\n",
	      stats.succeed, stats.failed, stats.failed_max,
	      tm / 1000, tm % 1000);
}

static int ess_switch_disable_lookup(struct ess_switch *esw)
{
	int val;
	int i;

	/* Disable port lookup for all ports*/
	for (i = 0; i < ESS_PORTS_NUM; i++) {
		int ess_port_vid;

		val = readl(esw->base + ESS_PORT_LOOKUP_CTRL(i));
		val &= ~ESS_PORT_VID_MEM_MASK;

		switch (i) {
		case 0:
			fallthrough;
		case 5:
			/* CPU,WAN port -> nothing */
			ess_port_vid = 0;
			break;
		case 1 ... 4:
			/* LAN ports -> all other LAN ports */
			ess_port_vid = GENMASK(4, 1);
			ess_port_vid &= ~BIT(i);
			break;
		default:
			return -EINVAL;
		}

		val |= FIELD_PREP(ESS_PORT_VID_MEM_MASK, ess_port_vid);

		writel(val, esw->base + ESS_PORT_LOOKUP_CTRL(i));
	}

	/* Set magic value for the global forwarding register 1 */
	writel(0x3e3e3e, esw->base + ESS_GLOBAL_FW_CTRL1);

	return 0;
}

static int ess_switch_enable_lookup(struct ess_switch *esw)
{
	int val;
	int i;

	/* Enable port lookup for all ports*/
	for (i = 0; i < ESS_PORTS_NUM; i++) {
		int ess_port_vid;

		val = readl(esw->base + ESS_PORT_LOOKUP_CTRL(i));
		val &= ~ESS_PORT_VID_MEM_MASK;

		switch (i) {
		case 0:
			/* CPU port -> all other ports */
			ess_port_vid = GENMASK(5, 1);
			break;
		case 1 ... 4:
			/* LAN ports -> CPU and all other LAN ports */
			ess_port_vid = GENMASK(4, 0);
			ess_port_vid &= ~BIT(i);
			break;
		case 5:
			/* WAN port -> CPU port only */
			ess_port_vid = BIT(0);
			break;
		default:
			return -EINVAL;
		}

		val |= FIELD_PREP(ESS_PORT_VID_MEM_MASK, ess_port_vid);

		writel(val, esw->base + ESS_PORT_LOOKUP_CTRL(i));
	}

	/* Set magic value for the global forwarding register 1 */
	writel(0x3f3f3f, esw->base + ESS_GLOBAL_FW_CTRL1);

	return 0;
}

static void ess_switch_init(struct ess_switch *esw)
{
	int val = 0;
	int i;

	/* Set magic value for the global forwarding register 1 */
	writel(0x3e3e3e, esw->base + ESS_GLOBAL_FW_CTRL1);

	/* Set 1000M speed, full duplex and RX/TX flow control for the CPU port*/
	val &= ~ESS_PORT_SPEED_MASK;
	val |= FIELD_PREP(ESS_PORT_SPEED_MASK, ESS_PORT_SPEED_1000);
	val |= ESS_PORT_DUPLEX_MODE;
	val |= ESS_PORT_TX_FLOW_EN;
	val |= ESS_PORT_RX_FLOW_EN;

	writel(val, esw->base + ESS_PORT0_STATUS);

	/* Disable port lookup for all ports*/
	for (i = 0; i < ESS_PORTS_NUM; i++) {
		val = readl(esw->base + ESS_PORT_LOOKUP_CTRL(i));
		val &= ~ESS_PORT_VID_MEM_MASK;

		writel(val, esw->base + ESS_PORT_LOOKUP_CTRL(i));
	}

	/* Set HOL settings for all ports*/
	for (i = 0; i < ESS_PORTS_NUM; i++) {
		val = 0;

		val |= FIELD_PREP(EG_PORT_QUEUE_NUM_MASK, 30);
		if (i == 0 || i == 5) {
			val |= FIELD_PREP(EG_PRI5_QUEUE_NUM_MASK, 4);
			val |= FIELD_PREP(EG_PRI4_QUEUE_NUM_MASK, 4);
		}
		val |= FIELD_PREP(EG_PRI3_QUEUE_NUM_MASK, 4);
		val |= FIELD_PREP(EG_PRI2_QUEUE_NUM_MASK, 4);
		val |= FIELD_PREP(EG_PRI1_QUEUE_NUM_MASK, 4);
		val |= FIELD_PREP(EG_PRI0_QUEUE_NUM_MASK, 4);

		writel(val, esw->base + ESS_PORT_HOL_CTRL0(i));

		val = readl(esw->base + ESS_PORT_HOL_CTRL1(i));
		val &= ~ESS_ING_BUF_NUM_0_MASK;
		val |= FIELD_PREP(ESS_ING_BUF_NUM_0_MASK, 6);

		writel(val, esw->base + ESS_PORT_HOL_CTRL1(i));
	}

	/* Give switch some time */
	mdelay(1);

	/* Enable RX and TX MAC-s */
	val = readl(esw->base + ESS_PORT0_STATUS);
	val |= ESS_PORT_TXMAC_EN;
	val |= ESS_PORT_RXMAC_EN;

	writel(val, esw->base + ESS_PORT0_STATUS);

	/* Set magic value for the global forwarding register 1 */
	writel(0x7f7f7f, esw->base + ESS_GLOBAL_FW_CTRL1);
}

static int essedma_of_phy(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	struct ess_switch *esw = &priv->esw;
	int num_phy = 0, ret = 0;
	ofnode node;
	int i;

	ofnode_for_each_subnode(node, esw->ports_node) {
		struct ofnode_phandle_args phandle_args;
		struct phy_device *phydev;
		u32 phy_addr;

		if (ofnode_is_enabled(node)) {
			if (ofnode_parse_phandle_with_args(node, "phy-handle", NULL, 0, 0,
					       &phandle_args)) {
				dev_dbg(priv->dev, "Failed to find phy-handle\n");
				return -ENODEV;
			}

			ret = ofnode_read_u32(phandle_args.node, "reg", &phy_addr);
			if (ret) {
				dev_dbg(priv->dev, "Missing reg property in PHY node %s\n",
				      ofnode_get_name(phandle_args.node));
				return ret;
			}

			phydev = dm_mdio_phy_connect(priv->mdio_dev, phy_addr,
						     dev, priv->esw.port_wrapper_mode);
			if (!phydev) {
				dev_dbg(priv->dev, "Failed to find phy on addr %d\n", phy_addr);
				return -ENODEV;
			}

			phydev->node = phandle_args.node;
			ret = phy_config(phydev);

			esw->phydev[num_phy] = phydev;

			num_phy++;
		}
	}

	esw->num_phy = num_phy;

	for (i = 0; i < esw->num_phy - 1; i++)
		esw->phy_mask |= BIT(i);

	return ret;
}

static int essedma_of_switch(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	int port_wrapper_mode = -1;

	priv->esw.ports_node = ofnode_find_subnode(dev_ofnode(dev), "ports");
	if (!ofnode_valid(priv->esw.ports_node)) {
		printf("Failed to find ports node\n");
		return -EINVAL;
	}

	port_wrapper_mode = ofnode_read_phy_mode(priv->esw.ports_node);
	if (port_wrapper_mode == -1)
		return -EINVAL;

	priv->esw.port_wrapper_mode = port_wrapper_mode;

	return essedma_of_phy(dev);
}

static void ipq40xx_edma_start_rx_tx(struct essedma_priv *priv)
{
	volatile u32 data;

	/* enable RX queues */
	data = readl(priv->base + EDMA_REG_RXQ_CTRL);
	data |= EDMA_RXQ_CTRL_EN;
	writel(data, priv->base + EDMA_REG_RXQ_CTRL);

	/* enable TX queues */
	data = readl(priv->base + EDMA_REG_TXQ_CTRL);
	data |= EDMA_TXQ_CTRL_TXQ_EN;
	writel(data, priv->base + EDMA_REG_TXQ_CTRL);
}

/*
 * ipq40xx_edma_init_desc()
 * Update descriptor ring size,
 * Update buffer and producer/consumer index
 */
static void ipq40xx_edma_init_desc(struct essedma_priv *priv)
{
	struct edma_ring *rfd_ring;
	struct edma_ring *etdr;
	volatile u32 data = 0;
	u16 hw_cons_idx = 0;

	/* Set the base address of every TPD ring. */
	etdr = &priv->tpd_ring;

	/* Update TX descriptor ring base address. */
	writel((u32)(etdr->dma & 0xffffffff),
	       priv->base + EDMA_REG_TPD_BASE_ADDR_Q(EDMA_TXQ_ID));
	data = readl(priv->base + EDMA_REG_TPD_IDX_Q(EDMA_TXQ_ID));

	/* Calculate hardware consumer index for Tx. */
	hw_cons_idx = FIELD_GET(EDMA_TPD_CONS_IDX_MASK, data);
	etdr->head = hw_cons_idx;
	etdr->tail = hw_cons_idx;
	data &= ~EDMA_TPD_PROD_IDX_MASK;
	data |= hw_cons_idx;

	/* Update producer index for Tx. */
	writel(data, priv->base + EDMA_REG_TPD_IDX_Q(EDMA_TXQ_ID));

	/* Update SW consumer index register for Tx. */
	writel(hw_cons_idx,
	       priv->base + EDMA_REG_TX_SW_CONS_IDX_Q(EDMA_TXQ_ID));

	/* Set TPD ring size. */
	writel((u32)(etdr->count & EDMA_TPD_RING_SIZE_MASK),
	       priv->base + EDMA_REG_TPD_RING_SIZE);

	/* Configure Rx ring. */
	rfd_ring = &priv->rfd_ring;

	/* Update Receive Free descriptor ring base address. */
	writel((u32)(rfd_ring->dma & 0xffffffff),
	       priv->base + EDMA_REG_RFD_BASE_ADDR_Q(EDMA_RXQ_ID));
	data = readl(priv->base + EDMA_REG_RFD_BASE_ADDR_Q(EDMA_RXQ_ID));

	/* Update RFD ring size and RX buffer size. */
	data = (rfd_ring->count & EDMA_RFD_RING_SIZE_MASK)
				<< EDMA_RFD_RING_SIZE_SHIFT;
	data |= (EDMA_MAX_PKT_SIZE & EDMA_RX_BUF_SIZE_MASK)
				<< EDMA_RX_BUF_SIZE_SHIFT;
	writel(data, priv->base + EDMA_REG_RX_DESC0);

	/* Disable TX FIFO low watermark and high watermark */
	writel(0, priv->base + EDMA_REG_TXF_WATER_MARK);

	/* Load all of base address above */
	data = readl(priv->base + EDMA_REG_TX_SRAM_PART);
	data |= 1 << EDMA_LOAD_PTR_SHIFT;
	writel(data, priv->base + EDMA_REG_TX_SRAM_PART);
}

static void ipq40xx_edma_init_rfd_ring(struct essedma_priv *priv)
{
	struct edma_ring *erdr = &priv->rfd_ring;
	struct edma_rfd *rfds = erdr->hw_desc;
	int i;

	for (i = 0; i < erdr->count; i++)
		rfds[i].buffer_addr = virt_to_phys(net_rx_packets[i]);

	flush_dcache_range(erdr->dma, erdr->dma + erdr->hw_size);

	/* setup producer index */
	erdr->head = erdr->count - 1;
	writel(erdr->head, priv->base + EDMA_REG_RFD_IDX_Q(EDMA_RXQ_ID));
}

static void ipq40xx_edma_configure(struct essedma_priv *priv)
{
	u32 tmp;
	int i;

	/* Set RSS type */
	writel(IPQ40XX_EDMA_RSS_TYPE_NONE, priv->base + EDMA_REG_RSS_TYPE);

	/* Configure RSS indirection table.
	 * 128 hash will be configured in the following
	 * pattern: hash{0,1,2,3} = {Q0,Q2,Q4,Q6} respectively
	 * and so on
	 */
	for (i = 0; i < EDMA_NUM_IDT; i++)
		writel(EDMA_RSS_IDT_VALUE, priv->base + EDMA_REG_RSS_IDT(i));

	/* Set RFD burst number */
	tmp = (EDMA_RFD_BURST << EDMA_RXQ_RFD_BURST_NUM_SHIFT);

	/* Set RFD prefetch threshold */
	tmp |= (EDMA_RFD_THR << EDMA_RXQ_RFD_PF_THRESH_SHIFT);

	/* Set RFD in host ring low threshold to generte interrupt */
	tmp |= (EDMA_RFD_LTHR << EDMA_RXQ_RFD_LOW_THRESH_SHIFT);
	writel(tmp, priv->base + EDMA_REG_RX_DESC1);

	/* configure reception control data. */

	/* Set Rx FIFO threshold to start to DMA data to host */
	tmp = EDMA_FIFO_THRESH_128_BYTE;

	/* Set RX remove vlan bit */
	tmp |= EDMA_RXQ_CTRL_RMV_VLAN;
	writel(tmp, priv->base + EDMA_REG_RXQ_CTRL);

	/* Configure transmission control data */
	tmp = (EDMA_TPD_BURST << EDMA_TXQ_NUM_TPD_BURST_SHIFT);
	tmp |= EDMA_TXQ_CTRL_TPD_BURST_EN;
	tmp |= (EDMA_TXF_BURST << EDMA_TXQ_TXF_BURST_NUM_SHIFT);
	writel(tmp, priv->base + EDMA_REG_TXQ_CTRL);
}

static void ipq40xx_edma_stop_rx_tx(struct essedma_priv *priv)
{
	volatile u32 data;

	data = readl(priv->base + EDMA_REG_RXQ_CTRL);
	data &= ~EDMA_RXQ_CTRL_EN;
	writel(data, priv->base + EDMA_REG_RXQ_CTRL);
	data = readl(priv->base + EDMA_REG_TXQ_CTRL);
	data &= ~EDMA_TXQ_CTRL_TXQ_EN;
	writel(data, priv->base + EDMA_REG_TXQ_CTRL);
}

static int ipq40xx_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	struct edma_ring *erdr = &priv->rfd_ring;
	struct edma_rrd *rrd;
	u32 hw_tail;
	u8 *rx_pkt;

	hw_tail = readl(priv->base + EDMA_REG_RFD_IDX_Q(EDMA_RXQ_ID));
	hw_tail = FIELD_GET(EDMA_RFD_CONS_IDX_MASK, hw_tail);

	if (hw_tail == erdr->tail)
		return -EAGAIN;

	rx_pkt = net_rx_packets[erdr->tail];
	invalidate_dcache_range((unsigned long)rx_pkt,
				(unsigned long)(rx_pkt + EDMA_MAX_PKT_SIZE));

	rrd = (struct edma_rrd *)rx_pkt;

	/* Check if RRD is valid */
	if (!(rrd->rrd7 & EDMA_RRD7_DESC_VALID))
		return 0;

	*packetp = rx_pkt + EDMA_RRD_SIZE;

	/* get the packet size */
	return rrd->rrd6;
}

static int ipq40xx_eth_free_pkt(struct udevice *dev, uchar *packet,
				int length)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	struct edma_ring *erdr;

	erdr = &priv->rfd_ring;

	/* Update the producer index */
	writel(erdr->head, priv->base + EDMA_REG_RFD_IDX_Q(EDMA_RXQ_ID));

	erdr->head++;
	if (erdr->head == erdr->count)
		erdr->head = 0;

	/* Update the consumer index */
	erdr->tail++;
	if (erdr->tail == erdr->count)
		erdr->tail = 0;

	writel(erdr->tail,
	       priv->base + EDMA_REG_RX_SW_CONS_IDX_Q(EDMA_RXQ_ID));

	return 0;
}

static int ipq40xx_eth_start(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);

	ipq40xx_edma_init_rfd_ring(priv);

	ipq40xx_edma_start_rx_tx(priv);
	ess_switch_enable_lookup(&priv->esw);

	return 0;
}

/*
 * One TPD would be enough for sending a packet, however because the
 * minimal cache line size is larger than the size of a TPD it is not
 * possible to flush only one at once. To overcome this limitation
 * multiple TPDs are used for sending a single packet.
 */
#define EDMA_TPDS_PER_PACKET	4
#define EDMA_TPD_MIN_BYTES	4
#define EDMA_MIN_PKT_SIZE	(EDMA_TPDS_PER_PACKET * EDMA_TPD_MIN_BYTES)

#define EDMA_TX_COMPLETE_TIMEOUT	1000000

static int ipq40xx_eth_send(struct udevice *dev, void *packet, int length)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	struct edma_tpd *first_tpd;
	struct edma_tpd *tpds;
	int i;

	if (length < EDMA_MIN_PKT_SIZE)
		return 0;

	flush_dcache_range((unsigned long)(packet),
			   (unsigned long)(packet) +
			   roundup(length, ARCH_DMA_MINALIGN));

	tpds = priv->tpd_ring.hw_desc;
	for (i = 0; i < EDMA_TPDS_PER_PACKET; i++) {
		struct edma_tpd *tpd;
		void *frag;

		frag = packet + (i * EDMA_TPD_MIN_BYTES);

		/* get the next TPD */
		tpd = &tpds[priv->tpd_ring.head];
		if (i == 0)
			first_tpd = tpd;

		/* update the software index */
		priv->tpd_ring.head++;
		if (priv->tpd_ring.head == priv->tpd_ring.count)
			priv->tpd_ring.head = 0;

		tpd->svlan_tag = 0;
		tpd->addr = virt_to_phys(frag);
		tpd->word3 = EDMA_PORT_ENABLE_ALL << EDMA_TPD_PORT_BITMAP_SHIFT;

		if (i < (EDMA_TPDS_PER_PACKET - 1)) {
			tpd->len = EDMA_TPD_MIN_BYTES;
			tpd->word1 = 0;
		} else {
			tpd->len = length;
			tpd->word1 = 1 << EDMA_TPD_EOP_SHIFT;
		}

		length -= EDMA_TPD_MIN_BYTES;
	}

	/* make sure that memory writing completes */
	wmb();

	flush_dcache_range((unsigned long)first_tpd,
			   (unsigned long)first_tpd +
			   EDMA_TPDS_PER_PACKET * sizeof(struct edma_tpd));

	/* update the TX producer index */
	writel(priv->tpd_ring.head,
	       priv->base + EDMA_REG_TPD_IDX_Q(EDMA_TXQ_ID));

	/* Wait for TX DMA completion */
	for (i = 0; i < EDMA_TX_COMPLETE_TIMEOUT; i++) {
		u32 r, prod, cons;

		r = readl(priv->base + EDMA_REG_TPD_IDX_Q(EDMA_TXQ_ID));
		prod = FIELD_GET(EDMA_TPD_PROD_IDX_MASK, r);
		cons = FIELD_GET(EDMA_TPD_CONS_IDX_MASK, r);

		if (cons == prod)
			break;

		udelay(1);
	}

	if (i == EDMA_TX_COMPLETE_TIMEOUT)
		printf("TX timeout: packet not sent!\n");

	/* update the software TX consumer index register */
	writel(priv->tpd_ring.head,
	       priv->base + EDMA_REG_TX_SW_CONS_IDX_Q(EDMA_TXQ_ID));

	return 0;
}

static void ipq40xx_eth_stop(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);

	ess_switch_disable_lookup(&priv->esw);
	ipq40xx_edma_stop_rx_tx(priv);
}

static void ipq40xx_edma_free_ring(struct edma_ring *ring)
{
	free(ring->hw_desc);
}

/*
 * Free Tx and Rx rings
 */
static void ipq40xx_edma_free_rings(struct essedma_priv *priv)
{
	ipq40xx_edma_free_ring(&priv->tpd_ring);
	ipq40xx_edma_free_ring(&priv->rfd_ring);
}

/*
 * ipq40xx_edma_alloc_ring()
 * allocate edma ring descriptor.
 */
static int ipq40xx_edma_alloc_ring(struct edma_ring *erd,
				   unsigned int desc_size)
{
	erd->head = 0;
	erd->tail = 0;

	 /* Alloc HW descriptors */
	erd->hw_size = roundup(desc_size * erd->count,
			       ARCH_DMA_MINALIGN);

	erd->hw_desc = memalign(CONFIG_SYS_CACHELINE_SIZE, erd->hw_size);
	if (!erd->hw_desc)
		 return -ENOMEM;

	memset(erd->hw_desc, 0, erd->hw_size);
	erd->dma = virt_to_phys(erd->hw_desc);

	return 0;

}

/*
 * ipq40xx_allocate_tx_rx_rings()
 */
static int ipq40xx_edma_alloc_tx_rx_rings(struct essedma_priv *priv)
{
	int ret;

	ret = ipq40xx_edma_alloc_ring(&priv->tpd_ring,
				      sizeof(struct edma_tpd));
	if (ret)
		return ret;

	ret = ipq40xx_edma_alloc_ring(&priv->rfd_ring,
				      sizeof(struct edma_rfd));
	if (ret)
		goto err_free_tpd;

	return 0;

err_free_tpd:
	ipq40xx_edma_free_ring(&priv->tpd_ring);
	return ret;
}

static int ipq40xx_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct essedma_priv *priv = dev_get_priv(dev);
	unsigned char *mac = pdata->enetaddr;
	u32 mac_lo, mac_hi;

	mac_hi = ((u32)mac[0]) << 8 | (u32)mac[1];
	mac_lo = ((u32)mac[2]) << 24 | ((u32)mac[3]) << 16 |
		 ((u32)mac[4]) << 8 | (u32)mac[5];

	writel(mac_lo, priv->base + REG_MAC_CTRL0);
	writel(mac_hi, priv->base + REG_MAC_CTRL1);

	return 0;
}

static int edma_init(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	int ret;

	priv->tpd_ring.count = IPQ40XX_EDMA_TX_RING_SIZE;
	priv->rfd_ring.count = PKTBUFSRX;

	ret = ipq40xx_edma_alloc_tx_rx_rings(priv);
	if (ret)
		return -ENOMEM;

	ipq40xx_edma_stop_rx_tx(priv);

	/* Configure EDMA. */
	ipq40xx_edma_configure(priv);

	/* Configure descriptor Ring */
	ipq40xx_edma_init_desc(priv);

	ess_switch_disable_lookup(&priv->esw);

	return 0;
}

static int essedma_probe(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);
	int ret;

	priv->dev = dev;

	priv->base = dev_read_addr_name(dev, "edma");
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->psgmii_base = dev_read_addr_name(dev, "psgmii_phy");
	if (priv->psgmii_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->esw.base = dev_read_addr_name(dev, "base");
	if (priv->esw.base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = clk_get_by_name(dev, "ess", &priv->ess_clk);
	if (ret)
		return ret;

	ret = reset_get_by_name(dev, "ess", &priv->ess_rst);
	if (ret)
		return ret;

	ret = clk_enable(&priv->ess_clk);
	if (ret)
		return ret;

	ess_reset(dev);

	ret = uclass_get_device_by_driver(UCLASS_MDIO,
					  DM_DRIVER_GET(ipq4019_mdio),
					  &priv->mdio_dev);
	if (ret) {
		dev_dbg(dev, "Cant find IPQ4019 MDIO: %d\n", ret);
		goto err;
	}

	/* OF switch and PHY parsing and configuration */
	ret = essedma_of_switch(dev);
	if (ret)
		goto err;

	switch (priv->esw.port_wrapper_mode) {
	case PHY_INTERFACE_MODE_PSGMII:
		writel(PSGMIIPHY_PLL_VCO_VAL,
		       priv->psgmii_base + PSGMIIPHY_PLL_VCO_RELATED_CTRL);
		writel(PSGMIIPHY_VCO_VAL, priv->psgmii_base +
		       PSGMIIPHY_VCO_CALIBRATION_CTRL_REGISTER_1);
		/* wait for 10ms */
		mdelay(10);
		writel(PSGMIIPHY_VCO_RST_VAL, priv->psgmii_base +
		       PSGMIIPHY_VCO_CALIBRATION_CTRL_REGISTER_1);
		break;
	case PHY_INTERFACE_MODE_RGMII:
		writel(0x1, RGMII_TCSR_ESS_CFG);
		writel(0x400, priv->esw.base + ESS_RGMII_CTRL);
		break;
	default:
		printf("Unknown MII interface\n");
	}

	if (priv->esw.port_wrapper_mode == PHY_INTERFACE_MODE_PSGMII)
		psgmii_self_test(dev);

	ess_switch_init(&priv->esw);

	ret = edma_init(dev);
	if (ret)
		goto err;

	return 0;

err:
	reset_assert(&priv->ess_rst);
	clk_disable(&priv->ess_clk);
	return ret;
}

static int essedma_remove(struct udevice *dev)
{
	struct essedma_priv *priv = dev_get_priv(dev);

	ipq40xx_edma_free_rings(priv);

	clk_disable(&priv->ess_clk);
	reset_assert(&priv->ess_rst);

	return 0;
}

static const struct eth_ops essedma_eth_ops = {
	.start		= ipq40xx_eth_start,
	.send		= ipq40xx_eth_send,
	.recv		= ipq40xx_eth_recv,
	.free_pkt	= ipq40xx_eth_free_pkt,
	.stop		= ipq40xx_eth_stop,
	.write_hwaddr	= ipq40xx_eth_write_hwaddr,
};

static const struct udevice_id essedma_ids[] = {
	{ .compatible = "qcom,ipq4019-ess", },
	{ }
};

U_BOOT_DRIVER(essedma) = {
	.name		= "essedma",
	.id		= UCLASS_ETH,
	.of_match	= essedma_ids,
	.probe		= essedma_probe,
	.remove		= essedma_remove,
	.priv_auto	= sizeof(struct essedma_priv),
	.plat_auto 	= sizeof(struct eth_pdata),
	.ops		= &essedma_eth_ops,
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
