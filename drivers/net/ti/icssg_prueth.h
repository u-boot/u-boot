/* SPDX-License-Identifier: GPL-2.0 */
/* Texas Instruments K3 AM65 Ethernet Switch SubSystem Driver
 *
 * Copyright (C) 2018-2024 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#ifndef __NET_TI_ICSSG_PRUETH_H
#define __NET_TI_ICSSG_PRUETH_H

#include <asm/io.h>
#include <clk.h>
#include <dm/lists.h>
#include <dm/ofnode.h>
#include <dm/device.h>
#include <dma-uclass.h>
#include <regmap.h>
#include <linux/sizes.h>
#include <linux/pruss_driver.h>
#include "icssg_config.h"
#include "icssg_switch_map.h"

void icssg_class_set_mac_addr(struct regmap *miig_rt, int slice, u8 *mac);
void icssg_class_set_host_mac_addr(struct regmap *miig_rt, u8 *mac);
void icssg_class_disable(struct regmap *miig_rt, int slice);
void icssg_class_default(struct regmap *miig_rt, int slice, bool allmulti);
void icssg_ft1_set_mac_addr(struct regmap *miig_rt, int slice, u8 *mac_addr);

enum prueth_mac {
	PRUETH_MAC0 = 0,
	PRUETH_MAC1,
	PRUETH_NUM_MACS,
};

enum prueth_port {
	PRUETH_PORT_HOST = 0,	/* host side port */
	PRUETH_PORT_MII0,	/* physical port MII 0 */
	PRUETH_PORT_MII1,	/* physical port MII 1 */
};

struct icssg_firmwares {
	char *pru;
	char *rtu;
	char *txpru;
};

struct prueth {
	struct udevice		*dev;
	struct udevice		*pruss;
	struct regmap		*miig_rt;
	struct regmap		*mii_rt;
	fdt_addr_t		mdio_base;
	struct pruss_mem_region shram;
	struct pruss_mem_region dram[PRUETH_NUM_MACS];
	phys_addr_t		tmaddr;
	struct mii_dev		*bus;
	u32			sram_pa;
	ofnode			eth_node[PRUETH_NUM_MACS];
	u32			mdio_freq;
	int			phy_interface;
	struct			clk mdiofck;
	struct dma		dma_tx;
	struct dma		dma_rx;
	struct dma		dma_rx_mgm;
	u32			rx_next;
	u32			rx_pend;
	int			slice;
	bool			mdio_manual_mode;
	int			speed;
	int			duplex;
	u8			pru_core_id;
	u8			rtu_core_id;
	u8			txpru_core_id;
	u8			icssg_hwcmdseq;
	struct icssg_firmwares  firmwares[PRUETH_NUM_MACS];
};

struct prueth_priv {
	struct udevice		*dev;
	struct prueth		*prueth;
	u32			port_id;
	struct phy_device	*phydev;
	bool			has_phy;
	ofnode			phy_node;
	u32			phy_addr;
	int			phy_interface;
};

/* config helpers */
void icssg_config_ipg(struct prueth_priv *priv, int speed, int mii);
int icssg_config(struct prueth_priv *priv);
int emac_set_port_state(struct prueth_priv *priv, enum icssg_port_state_cmd cmd);

/* Buffer queue helpers */
int icssg_queue_pop(struct prueth *prueth, u8 queue);
void icssg_queue_push(struct prueth *prueth, int queue, u16 addr);
u32 icssg_queue_level(struct prueth *prueth, int queue);

/* FDB helpers */
int icssg_send_fdb_msg(struct prueth_priv *priv, struct mgmt_cmd *cmd,
		       struct mgmt_cmd_rsp *rsp);
int emac_fdb_flow_id_updated(struct prueth_priv *priv);

#endif /* __NET_TI_ICSSG_PRUETH_H */
