// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments K3 AM65 PRU Ethernet Driver
 *
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#include <asm/io.h>
#include <asm/processor.h>
#include <clk.h>
#include <dm/lists.h>
#include <dm/device.h>
#include <dma-uclass.h>
#include <dm/of_access.h>
#include <dm/pinctrl.h>
#include <fs_loader.h>
#include <miiphy.h>
#include <net.h>
#include <phy.h>
#include <power-domain.h>
#include <linux/soc/ti/ti-udma.h>
#include <regmap.h>
#include <remoteproc.h>
#include <syscon.h>
#include <soc.h>
#include <linux/pruss_driver.h>
#include <dm/device_compat.h>

#include "icssg_prueth.h"
#include "icss_mii_rt.h"

#define ICSS_SLICE0     0
#define ICSS_SLICE1     1

#ifdef PKTSIZE_ALIGN
#define UDMA_RX_BUF_SIZE PKTSIZE_ALIGN
#else
#define UDMA_RX_BUF_SIZE ALIGN(PKTSIZE, ARCH_DMA_MINALIGN)
#endif

#ifdef PKTBUFSRX
#define UDMA_RX_DESC_NUM PKTBUFSRX
#else
#define UDMA_RX_DESC_NUM 4
#endif

/* Config region lies in shared RAM */
#define ICSS_CONFIG_OFFSET_SLICE0	0
#define ICSS_CONFIG_OFFSET_SLICE1	0x8000

/* Firmware flags */
#define ICSS_SET_RUN_FLAG_VLAN_ENABLE		BIT(0)	/* switch only */
#define ICSS_SET_RUN_FLAG_FLOOD_UNICAST		BIT(1)	/* switch only */
#define ICSS_SET_RUN_FLAG_PROMISC		BIT(2)	/* MAC only */
#define ICSS_SET_RUN_FLAG_MULTICAST_PROMISC	BIT(3)	/* MAC only */

/* CTRLMMR_ICSSG_RGMII_CTRL register bits */
#define ICSSG_CTRL_RGMII_ID_MODE		BIT(24)

/* Management packet type */
#define PRUETH_PKT_TYPE_CMD		0x10

/* Number of PRU Cores per Slice */
#define ICSSG_NUM_PRU_CORES		3
#define ICSSG_NUM_FIRMWARES		6

static int icssg_gmii_select(struct prueth_priv *priv)
{
	struct phy_device *phydev = priv->phydev;

	if (phydev->interface != PHY_INTERFACE_MODE_MII &&
	    phydev->interface < PHY_INTERFACE_MODE_RGMII &&
	    phydev->interface > PHY_INTERFACE_MODE_RGMII_TXID) {
		dev_err(priv->dev, "PHY mode unsupported %s\n",
			phy_string_for_interface(phydev->interface));
		return -EINVAL;
	}

	/* AM65 SR2.0 has TX Internal delay always enabled by hardware
	 * and it is not possible to disable TX Internal delay. The below
	 * switch case block describes how we handle different phy modes
	 * based on hardware restriction.
	 */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII_ID:
		phydev->interface = PHY_INTERFACE_MODE_RGMII_RXID;
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		phydev->interface = PHY_INTERFACE_MODE_RGMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_RXID:
		dev_err(priv->dev, "RGMII mode without TX delay is not supported");
		return -EINVAL;
	default:
		break;
	}

	return 0;
}

static int icssg_phy_init(struct udevice *dev)
{
	struct prueth_priv *priv = dev_get_priv(dev);
	struct phy_device *phydev;
	u32 supported = PHY_GBIT_FEATURES;
	int ret;

	phydev = dm_eth_phy_connect(dev);
	if (!phydev) {
		dev_err(dev, "phy_connect() failed\n");
		return -ENODEV;
	}

	/* disable unsupported features */
	supported &= ~(PHY_10BT_FEATURES |
			SUPPORTED_100baseT_Half |
			SUPPORTED_1000baseT_Half |
			SUPPORTED_Pause |
			SUPPORTED_Asym_Pause);

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;
	priv->phydev = phydev;

	ret = icssg_gmii_select(priv);
	if (ret)
		goto out;

	ret = phy_config(phydev);
	if (ret < 0)
		dev_err(dev, "phy_config() failed: %d", ret);
out:
	return ret;
}

static void icssg_config_set_speed(struct prueth_priv *priv, int speed)
{
	struct prueth *prueth = priv->prueth;
	u8 fw_speed;

	switch (speed) {
	case SPEED_1000:
		fw_speed = FW_LINK_SPEED_1G;
		break;
	case SPEED_100:
		fw_speed = FW_LINK_SPEED_100M;
		break;
	case SPEED_10:
		fw_speed = FW_LINK_SPEED_10M;
		break;
	default:
		/* Other links speeds not supported */
		dev_err(priv->dev, "Unsupported link speed\n");
		return;
	}

	writeb(fw_speed, prueth->dram[priv->port_id].pa + PORT_LINK_SPEED_OFFSET);
}

static int icssg_update_link(struct prueth_priv *priv)
{
	struct phy_device *phy = priv->phydev;
	struct prueth *prueth = priv->prueth;
	bool gig_en = false, full_duplex = false;

	if (phy->link) { /* link up */
		if (phy->speed == SPEED_1000)
			gig_en = true;
		if (phy->duplex == DUPLEX_FULL)
			full_duplex = true;
		/* Set the RGMII cfg for gig en and full duplex */
		icssg_update_rgmii_cfg(prueth->miig_rt, phy->speed, full_duplex,
				       priv->port_id, priv);
		/* update the Tx IPG based on 100M/1G speed */
		icssg_config_ipg(priv, phy->speed, priv->port_id);

		/* Send command to firmware to update Speed setting */
		icssg_config_set_speed(priv, phy->speed);

		/* Enable PORT FORWARDING */
		emac_set_port_state(priv, ICSSG_EMAC_PORT_FORWARD);

		printf("link up on port %d, speed %d, %s duplex\n",
		       priv->port_id, phy->speed,
		       (phy->duplex == DUPLEX_FULL) ? "full" : "half");
	} else {
		emac_set_port_state(priv, ICSSG_EMAC_PORT_DISABLE);
		printf("link down on port %d\n", priv->port_id);
	}

	return phy->link;
}

static int icssg_start_pru_cores(struct udevice *dev)
{
	struct prueth_priv *priv = dev_get_priv(dev);
	struct prueth *prueth = priv->prueth;
	struct icssg_firmwares *firmwares;
	struct udevice *rproc_dev = NULL;
	int ret, slice;
	u32 phandle;
	u8 index;

	slice = priv->port_id;
	index = slice * ICSSG_NUM_PRU_CORES;
	firmwares = prueth->firmwares;

	ofnode_read_u32_index(dev_ofnode(prueth->dev), "ti,prus", index, &phandle);
	ret = uclass_get_device_by_phandle_id(UCLASS_REMOTEPROC, phandle, &rproc_dev);
	if (ret) {
		dev_err(dev, "Unknown remote processor with phandle '0x%x' requested(%d)\n",
			phandle, ret);
		return ret;
	}

	prueth->pru_core_id = dev_seq(rproc_dev);
	ret = rproc_set_firmware(rproc_dev, firmwares[slice].pru);
	if (ret)
		return ret;

	ret = rproc_boot(rproc_dev);
	if (ret) {
		dev_err(dev, "failed to boot PRU%d: %d\n", slice, ret);
		return -EINVAL;
	}

	ofnode_read_u32_index(dev_ofnode(prueth->dev), "ti,prus", index + 1, &phandle);
	ret = uclass_get_device_by_phandle_id(UCLASS_REMOTEPROC, phandle, &rproc_dev);
	if (ret) {
		dev_err(dev, "Unknown remote processor with phandle '0x%x' requested(%d)\n",
			phandle, ret);
		goto halt_pru;
	}

	prueth->rtu_core_id = dev_seq(rproc_dev);
	ret = rproc_set_firmware(rproc_dev, firmwares[slice].rtu);
	if (ret)
		goto halt_pru;

	ret = rproc_boot(rproc_dev);
	if (ret) {
		dev_err(dev, "failed to boot RTU%d: %d\n", slice, ret);
		goto halt_pru;
	}

	ofnode_read_u32_index(dev_ofnode(prueth->dev), "ti,prus", index + 2, &phandle);
	ret = uclass_get_device_by_phandle_id(UCLASS_REMOTEPROC, phandle, &rproc_dev);
	if (ret) {
		dev_err(dev, "Unknown remote processor with phandle '0x%x' requested(%d)\n",
			phandle, ret);
		goto halt_rtu;
	}

	prueth->txpru_core_id = dev_seq(rproc_dev);
	ret = rproc_set_firmware(rproc_dev, firmwares[slice].txpru);
	if (ret)
		goto halt_rtu;

	ret = rproc_boot(rproc_dev);
	if (ret) {
		dev_err(dev, "failed to boot TXPRU%d: %d\n", slice, ret);
		goto halt_rtu;
	}

	return 0;

halt_rtu:
	rproc_stop(prueth->rtu_core_id);

halt_pru:
	rproc_stop(prueth->pru_core_id);
	return ret;
}

static int icssg_stop_pru_cores(struct udevice *dev)
{
	struct prueth_priv *priv = dev_get_priv(dev);
	struct prueth *prueth = priv->prueth;

	rproc_stop(prueth->pru_core_id);
	rproc_stop(prueth->rtu_core_id);
	rproc_stop(prueth->txpru_core_id);

	return 0;
}

static int prueth_start(struct udevice *dev)
{
	struct ti_udma_drv_chan_cfg_data *dma_rx_cfg_data;
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct prueth_priv *priv = dev_get_priv(dev);
	struct prueth *prueth = priv->prueth;
	struct icssg_flow_cfg *flow_cfg;
	u8 *hwaddr = pdata->enetaddr;
	char chn_name[16];
	void *config;
	int ret, i;

	icssg_class_set_mac_addr(prueth->miig_rt, priv->port_id, hwaddr);
	icssg_ft1_set_mac_addr(prueth->miig_rt, priv->port_id, hwaddr);
	icssg_class_default(prueth->miig_rt, priv->port_id, 0);

	/* Set Load time configuration */
	icssg_config(priv);

	ret = icssg_start_pru_cores(dev);
	if (ret)
		return ret;

	/* To differentiate channels for SLICE0 vs SLICE1 */
	snprintf(chn_name, sizeof(chn_name), "tx%d-0", priv->port_id);

	ret = dma_get_by_name(prueth->dev, chn_name, &prueth->dma_tx);
	if (ret)
		dev_err(dev, "TX dma get failed %d\n", ret);

	snprintf(chn_name, sizeof(chn_name), "rx%d", priv->port_id);
	ret = dma_get_by_name(prueth->dev, chn_name, &prueth->dma_rx);
	if (ret)
		dev_err(dev, "RX dma get failed %d\n", ret);

	for (i = 0; i < UDMA_RX_DESC_NUM; i++) {
		ret = dma_prepare_rcv_buf(&prueth->dma_rx,
					  net_rx_packets[i],
					  UDMA_RX_BUF_SIZE);
		if (ret)
			dev_err(dev, "RX dma add buf failed %d\n", ret);
	}

	ret = dma_enable(&prueth->dma_tx);
	if (ret) {
		dev_err(dev, "TX dma_enable failed %d\n", ret);
		goto tx_fail;
	}

	ret = dma_enable(&prueth->dma_rx);
	if (ret) {
		dev_err(dev, "RX dma_enable failed %d\n", ret);
		goto rx_fail;
	}

	/* check if the rx_flow_id of dma_rx is as expected since
	 * driver hardcode that value in config struct to firmware
	 * in probe. Just add this sanity check to catch any change
	 * to rx channel assignment in the future.
	 */
	dma_get_cfg(&prueth->dma_rx, 0, (void **)&dma_rx_cfg_data);
	config = (void *)(prueth->dram[priv->port_id].pa + ICSSG_CONFIG_OFFSET);

	flow_cfg = config + PSI_L_REGULAR_FLOW_ID_BASE_OFFSET;
	writew(dma_rx_cfg_data->flow_id_base, &flow_cfg->rx_base_flow);
	writew(0, &flow_cfg->mgm_base_flow);

	dev_info(dev, "K3 ICSSG: rflow_id_base: %u, chn_name = %s\n",
		 dma_rx_cfg_data->flow_id_base, chn_name);

	ret = emac_fdb_flow_id_updated(priv);
	if (ret) {
		dev_err(dev, "Failed to update Rx Flow ID %d", ret);
		goto phy_fail;
	}

	ret = phy_startup(priv->phydev);
	if (ret) {
		dev_err(dev, "phy_startup failed\n");
		goto phy_fail;
	}

	ret = icssg_update_link(priv);
	if (!ret) {
		ret = -ENODEV;
		goto phy_shut;
	}

	return 0;

phy_shut:
	phy_shutdown(priv->phydev);
phy_fail:
	dma_disable(&prueth->dma_rx);
	dma_free(&prueth->dma_rx);
rx_fail:
	dma_disable(&prueth->dma_tx);
	dma_free(&prueth->dma_tx);

tx_fail:
	icssg_class_disable(prueth->miig_rt, priv->port_id);

	return ret;
}

static int prueth_send(struct udevice *dev, void *packet, int length)
{
	struct prueth_priv *priv = dev_get_priv(dev);
	struct prueth *prueth = priv->prueth;
	int ret;

	ret = dma_send(&prueth->dma_tx, packet, length, NULL);

	return ret;
}

static int prueth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct prueth_priv *priv = dev_get_priv(dev);
	struct prueth *prueth = priv->prueth;
	int ret;

	/* try to receive a new packet */
	ret = dma_receive(&prueth->dma_rx, (void **)packetp, NULL);

	return ret;
}

static int prueth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct prueth_priv *priv = dev_get_priv(dev);
	struct prueth *prueth = priv->prueth;
	int ret = 0;

	if (length > 0) {
		u32 pkt = prueth->rx_next % UDMA_RX_DESC_NUM;

		dev_dbg(dev, "%s length:%d pkt:%u\n", __func__, length, pkt);

		ret = dma_prepare_rcv_buf(&prueth->dma_rx,
					  net_rx_packets[pkt],
					  UDMA_RX_BUF_SIZE);
		prueth->rx_next++;
	}

	return ret;
}

static void prueth_stop(struct udevice *dev)
{
	struct prueth_priv *priv = dev_get_priv(dev);
	struct prueth *prueth = priv->prueth;

	phy_shutdown(priv->phydev);

	dma_disable(&prueth->dma_tx);
	dma_disable(&prueth->dma_rx);

	icssg_stop_pru_cores(dev);

	dma_free(&prueth->dma_tx);
	dma_free(&prueth->dma_rx);
}

static const struct eth_ops prueth_ops = {
	.start		= prueth_start,
	.send		= prueth_send,
	.recv		= prueth_recv,
	.free_pkt	= prueth_free_pkt,
	.stop		= prueth_stop,
};

static char *prepend_fw_path(const char *fw_name)
{
	static const char fw_dir[] = "/lib/firmware/";
	char *result;
	int len;

	if (!fw_name)
		return NULL;

	len = strlen(fw_dir) + strlen(fw_name) + 1;
	result = malloc(len);
	if (!result)
		return NULL;

	sprintf(result, "%s%s", fw_dir, fw_name);
	return result;
}

static int icssg_ofdata_parse_phy(struct udevice *dev)
{
	struct prueth_priv *priv = dev_get_priv(dev);

	dev_read_u32(dev, "reg", &priv->port_id);
	priv->phy_interface = dev_read_phy_mode(dev);
	if (priv->phy_interface == PHY_INTERFACE_MODE_NA) {
		dev_err(dev, "Invalid PHY mode '%s', port %u\n",
			phy_string_for_interface(priv->phy_interface),
			priv->port_id);
		return -EINVAL;
	}

	return 0;
}

static int prueth_port_probe(struct udevice *dev)
{
	struct prueth_priv *priv = dev_get_priv(dev);
	struct prueth *prueth;
	char portname[15];
	int ret;

	priv->dev = dev;
	prueth = dev_get_priv(dev->parent);
	priv->prueth = prueth;

	sprintf(portname, "%s-%s", dev->parent->name, dev->name);

	device_set_name(dev, portname);

	ret = icssg_ofdata_parse_phy(dev);
	if (ret)
		goto out;

	ret = icssg_phy_init(dev);
	if (ret)
		goto out;

	ret = pruss_request_mem_region(prueth->pruss,
				       priv->port_id ? PRUSS_MEM_DRAM1 : PRUSS_MEM_DRAM0,
				       &prueth->dram[priv->port_id]);
	if (ret) {
		dev_err(dev, "could not request DRAM%d region\n", priv->port_id);
		return ret;
	}
out:
	return ret;
}

static int prueth_probe(struct udevice *dev)
{
	ofnode node, pruss_node, mdio_node, sram_node, curr_sram_node;
	struct prueth *prueth = dev_get_priv(dev);
	u32 phandle, err, sp, prev_end_addr;
	struct udevice **prussdev = NULL;
	ofnode eth_ports_node, eth_node;
	struct udevice *port_dev;
	const char **fw_names;
	int fw_count, i;
	int ret = 0;

	prueth->dev = dev;

	err = ofnode_read_u32(dev_ofnode(dev), "ti,prus", &phandle);
	if (err)
		return err;

	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node))
		return -EINVAL;

	pruss_node = ofnode_get_parent(node);
	ret = device_get_global_by_ofnode(pruss_node, prussdev);
	if (ret)
		dev_err(dev, "error getting the pruss dev\n");
	prueth->pruss = *prussdev;

	ret = pruss_request_mem_region(*prussdev, PRUSS_MEM_SHRD_RAM2,
				       &prueth->shram);
	if (ret)
		return ret;

	ret = pruss_request_tm_region(*prussdev, &prueth->tmaddr);
	if (ret)
		return ret;

	prueth->miig_rt = syscon_regmap_lookup_by_phandle(dev, "ti,mii-g-rt");
	if (!prueth->miig_rt) {
		dev_err(dev, "couldn't get mii-g-rt syscon regmap\n");
		return -ENODEV;
	}

	prueth->mii_rt = syscon_regmap_lookup_by_phandle(dev, "ti,mii-rt");
	if (!prueth->mii_rt) {
		dev_err(dev, "couldn't get mii-rt syscon regmap\n");
		return -ENODEV;
	}

	ret = ofnode_read_u32(dev_ofnode(dev), "sram", &sp);
	if (ret) {
		dev_err(dev, "sram node fetch failed %d\n", ret);
		return ret;
	}

	sram_node = ofnode_get_by_phandle(sp);
	if (!ofnode_valid(sram_node))
		return -EINVAL;

	prev_end_addr = ofnode_get_addr(sram_node);

	ofnode_for_each_subnode(curr_sram_node, sram_node) {
		u32 start_addr, size, end_addr, avail;
		const char *name;

		name = ofnode_get_name(curr_sram_node);
		start_addr = ofnode_get_addr(curr_sram_node);
		size = ofnode_get_size(curr_sram_node);
		end_addr = start_addr + size;
		avail = start_addr - prev_end_addr;

		if (avail > MSMC_RAM_SIZE)
			break;

		prev_end_addr = end_addr;
	}

	prueth->sram_pa = prev_end_addr;
	if (prueth->sram_pa % SZ_64K != 0) {
		/* This is constraint for SR2.0 firmware */
		dev_err(dev, "sram address needs to be 64KB aligned\n");
		return -EINVAL;
	}
	dev_dbg(dev, "sram: addr %x size %x\n", prueth->sram_pa, MSMC_RAM_SIZE);

	mdio_node = ofnode_find_subnode(pruss_node, "mdio");
	prueth->mdio_base = ofnode_get_addr(mdio_node);
	ofnode_read_u32(mdio_node, "bus_freq", &prueth->mdio_freq);

	ret = clk_get_by_name_nodev(mdio_node, "fck", &prueth->mdiofck);
	if (ret) {
		dev_err(dev, "failed to get clock %d\n", ret);
		return ret;
	}

	ret = clk_enable(&prueth->mdiofck);
	if (ret) {
		dev_err(dev, "clk_enable failed %d\n", ret);
		return ret;
	}

	eth_ports_node = dev_read_subnode(dev, "ethernet-ports");
	if (!ofnode_valid(eth_ports_node))
		return -ENOENT;

	ofnode_for_each_subnode(eth_node, eth_ports_node) {
		const char *node_name;
		u32 port_id;
		bool disabled;

		node_name = ofnode_get_name(eth_node);
		disabled = !ofnode_is_enabled(eth_node);
		ret = ofnode_read_u32(eth_node, "reg", &port_id);
		if (ret)
			dev_err(dev, "%s: error reading port_id (%d)\n", node_name, ret);

		if (port_id >= PRUETH_NUM_MACS) {
			dev_err(dev, "%s: invalid port_id (%d)\n", node_name, port_id);
			return -EINVAL;
		}

		if (port_id < 0)
			continue;
		if (disabled)
			continue;

		ret = device_bind_driver_to_node(dev, "prueth_port",
						 ofnode_get_name(eth_node),
						 eth_node, &port_dev);
		if (ret) {
			dev_err(dev, "Failed to bind to %s node\n", ofnode_get_name(eth_node));
			goto out;
		}
	}

	/* Parse firmware-name property from DT */
	fw_count = dev_read_string_list(dev, "firmware-name", &fw_names);
	if (fw_count != ICSSG_NUM_FIRMWARES) {
		dev_err(dev, "Expected %d firmware names, got %d\n", ICSSG_NUM_FIRMWARES, fw_count);
		return -EINVAL;
	}
	for (i = 0; i < 2; i++) {
		prueth->firmwares[i].pru   = prepend_fw_path(fw_names[i * 3 + 0]);
		prueth->firmwares[i].rtu   = prepend_fw_path(fw_names[i * 3 + 1]);
		prueth->firmwares[i].txpru = prepend_fw_path(fw_names[i * 3 + 2]);
	}

	return 0;
out:
	clk_disable(&prueth->mdiofck);

	return ret;
}

static const struct udevice_id prueth_ids[] = {
	{ .compatible = "ti,am654-icssg-prueth" },
	{ .compatible = "ti,am642-icssg-prueth" },
	{ }
};

U_BOOT_DRIVER(prueth) = {
	.name	= "prueth",
	.id	= UCLASS_MISC,
	.of_match = prueth_ids,
	.probe	= prueth_probe,
	.priv_auto = sizeof(struct prueth),
};

U_BOOT_DRIVER(prueth_port) = {
	.name	= "prueth_port",
	.id	= UCLASS_ETH,
	.probe	= prueth_port_probe,
	.ops	= &prueth_ops,
	.priv_auto = sizeof(struct prueth_priv),
	.plat_auto = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
