// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Microsemi Corporation
 */

#include <common.h>
#include <config.h>
#include <dm.h>
#include <dm/of_access.h>
#include <dm/of_addr.h>
#include <fdt_support.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <miiphy.h>
#include <net.h>
#include <wait_bit.h>

#include "mscc_miim.h"
#include "mscc_xfer.h"
#include "mscc_mac_table.h"

#define ANA_PORT_VLAN_CFG(x)		(0x00 + 0x80 * (x))
#define		ANA_PORT_VLAN_CFG_AWARE_ENA	BIT(20)
#define		ANA_PORT_VLAN_CFG_POP_CNT(x)	((x) << 18)
#define ANA_PORT_CPU_FWD_CFG(x)		(0x50 + 0x80 * (x))
#define		ANA_PORT_CPU_FWD_CFG_SRC_COPY_ENA	BIT(1)
#define ANA_PORT_PORT_CFG(x)		(0x60 + 0x80 * (x))
#define		ANA_PORT_PORT_CFG_RECV_ENA	BIT(5)
#define ANA_PGID(x)			(0x1000 + 4 * (x))

#define SYS_FRM_AGING			0x8300

#define SYS_SYSTEM_RST_CFG		0x81b0
#define		SYS_SYSTEM_RST_MEM_INIT		BIT(0)
#define		SYS_SYSTEM_RST_MEM_ENA		BIT(1)
#define		SYS_SYSTEM_RST_CORE_ENA		BIT(2)
#define SYS_PORT_MODE(x)		(0x81bc + 0x4 * (x))
#define		SYS_PORT_MODE_INCL_INJ_HDR	BIT(0)
#define SYS_SWITCH_PORT_MODE(x)		(0x8294 + 0x4 * (x))
#define		SYS_SWITCH_PORT_MODE_PORT_ENA	BIT(3)
#define SYS_EGR_NO_SHARING		0x8378
#define SYS_SCH_CPU			0x85a0

#define REW_PORT_CFG(x)			(0x8 + 0x80 * (x))
#define		REW_PORT_CFG_IFH_INSERT_ENA	BIT(7)

#define GCB_DEVCPU_RST_SOFT_CHIP_RST	0x90
#define		GCB_DEVCPU_RST_SOFT_CHIP_RST_SOFT_PHY	BIT(1)
#define GCB_MISC_STAT			0x11c
#define		GCB_MISC_STAT_PHY_READY			BIT(3)

#define	QS_XTR_MAP(x)			(0x10 + 4 * (x))
#define		QS_XTR_MAP_GRP			BIT(4)
#define		QS_XTR_MAP_ENA			BIT(0)

#define HSIO_PLL5G_CFG_PLL5G_CFG2	0x8

#define HSIO_RCOMP_CFG_CFG0		0x20
#define		HSIO_RCOMP_CFG_CFG0_MODE_SEL(x)			((x) << 8)
#define		HSIO_RCOMP_CFG_CFG0_RUN_CAL			BIT(12)
#define HSIO_RCOMP_STATUS		0x24
#define		HSIO_RCOMP_STATUS_BUSY				BIT(12)
#define		HSIO_RCOMP_STATUS_RCOMP_M			GENMASK(3, 0)
#define HSIO_SERDES6G_ANA_CFG_DES_CFG	0x64
#define		HSIO_SERDES6G_ANA_CFG_DES_CFG_BW_ANA(x)		((x) << 1)
#define		HSIO_SERDES6G_ANA_CFG_DES_CFG_BW_HYST(x)	((x) << 5)
#define		HSIO_SERDES6G_ANA_CFG_DES_CFG_MBTR_CTRL(x)	((x) << 10)
#define		HSIO_SERDES6G_ANA_CFG_DES_CFG_PHS_CTRL(x)	((x) << 13)
#define HSIO_SERDES6G_ANA_CFG_IB_CFG	0x68
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_RESISTOR_CTRL(x)	(x)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_VBCOM(x)		((x) << 4)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_VBAC(x)		((x) << 7)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_RT(x)		((x) << 9)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_RF(x)		((x) << 14)
#define HSIO_SERDES6G_ANA_CFG_IB_CFG1	0x6c
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_RST		BIT(0)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_ENA_OFFSDC	BIT(2)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_ENA_OFFSAC	BIT(3)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_ANEG_MODE	BIT(6)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_CHF		BIT(7)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_C(x)		((x) << 8)
#define HSIO_SERDES6G_ANA_CFG_OB_CFG	0x70
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_SR(x)		((x) << 4)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_SR_H		BIT(8)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_POST0(x)		((x) << 23)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_POL		BIT(29)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_ENA1V_MODE	BIT(30)
#define HSIO_SERDES6G_ANA_CFG_OB_CFG1	0x74
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG1_LEV(x)		(x)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG1_ENA_CAS(x)	((x) << 6)
#define HSIO_SERDES6G_ANA_CFG_COMMON_CFG 0x7c
#define		HSIO_SERDES6G_ANA_CFG_COMMON_CFG_IF_MODE(x)	(x)
#define		HSIO_SERDES6G_ANA_CFG_COMMON_CFG_ENA_LANE	BIT(18)
#define		HSIO_SERDES6G_ANA_CFG_COMMON_CFG_SYS_RST	BIT(31)
#define HSIO_SERDES6G_ANA_CFG_PLL_CFG	0x80
#define		HSIO_SERDES6G_ANA_CFG_PLL_CFG_FSM_ENA		BIT(7)
#define		HSIO_SERDES6G_ANA_CFG_PLL_CFG_FSM_CTRL_DATA(x)	((x) << 8)
#define HSIO_SERDES6G_ANA_CFG_SER_CFG	0x84
#define HSIO_SERDES6G_DIG_CFG_MISC_CFG	0x88
#define		HSIO_SERDES6G_DIG_CFG_MISC_CFG_LANE_RST		BIT(0)
#define HSIO_MCB_SERDES6G_CFG		0xac
#define		HSIO_MCB_SERDES6G_CFG_WR_ONE_SHOT		BIT(31)
#define		HSIO_MCB_SERDES6G_CFG_ADDR(x)			(x)

#define DEV_GMII_PORT_MODE_CLK		0x0
#define		DEV_GMII_PORT_MODE_CLK_PHY_RST	BIT(0)
#define DEV_GMII_MAC_CFG_MAC_ENA	0xc
#define		DEV_GMII_MAC_CFG_MAC_ENA_RX_ENA		BIT(4)
#define		DEV_GMII_MAC_CFG_MAC_ENA_TX_ENA		BIT(0)

#define DEV_PORT_MODE_CLK		0x4
#define		DEV_PORT_MODE_CLK_PHY_RST		BIT(2)
#define		DEV_PORT_MODE_CLK_LINK_SPEED_1000	1
#define DEV_MAC_CFG_MAC_ENA		0x10
#define		DEV_MAC_CFG_MAC_ENA_RX_ENA		BIT(4)
#define		DEV_MAC_CFG_MAC_ENA_TX_ENA		BIT(0)
#define DEV_MAC_CFG_MAC_IFG		0x24
#define		DEV_MAC_CFG_MAC_IFG_TX_IFG(x)		((x) << 8)
#define		DEV_MAC_CFG_MAC_IFG_RX_IFG2(x)		((x) << 4)
#define		DEV_MAC_CFG_MAC_IFG_RX_IFG1(x)		(x)
#define DEV_PCS1G_CFG_PCS1G_CFG		0x40
#define		DEV_PCS1G_CFG_PCS1G_CFG_PCS_ENA		BIT(0)
#define DEV_PCS1G_CFG_PCS1G_MODE	0x44
#define DEV_PCS1G_CFG_PCS1G_SD		0x48
#define DEV_PCS1G_CFG_PCS1G_ANEG	0x4c
#define		DEV_PCS1G_CFG_PCS1G_ANEG_ADV_ABILITY(x)	((x) << 16)

#define IFH_INJ_BYPASS		BIT(31)
#define IFH_TAG_TYPE_C		0
#define MAC_VID			1
#define CPU_PORT		26
#define INTERNAL_PORT_MSK	0xFFFFFF
#define IFH_LEN			2
#define ETH_ALEN		6
#define PGID_BROADCAST		28
#define PGID_UNICAST		29
#define PGID_SRC		80

enum luton_target {
	PORT0,
	PORT1,
	PORT2,
	PORT3,
	PORT4,
	PORT5,
	PORT6,
	PORT7,
	PORT8,
	PORT9,
	PORT10,
	PORT11,
	PORT12,
	PORT13,
	PORT14,
	PORT15,
	PORT16,
	PORT17,
	PORT18,
	PORT19,
	PORT20,
	PORT21,
	PORT22,
	PORT23,
	SYS,
	ANA,
	REW,
	GCB,
	QS,
	HSIO,
	TARGET_MAX,
};

#define MAX_PORT (PORT23 - PORT0 + 1)

#define MIN_INT_PORT PORT0
#define MAX_INT_PORT (PORT11 - PORT0  + 1)
#define MIN_EXT_PORT PORT12
#define MAX_EXT_PORT MAX_PORT

enum luton_mdio_target {
	MIIM,
	TARGET_MDIO_MAX,
};

enum luton_phy_id {
	INTERNAL,
	EXTERNAL,
	NUM_PHY,
};

struct luton_private {
	void __iomem *regs[TARGET_MAX];
	struct mii_dev *bus[NUM_PHY];
};

static const unsigned long luton_regs_qs[] = {
	[MSCC_QS_XTR_RD] = 0x18,
	[MSCC_QS_XTR_FLUSH] = 0x28,
	[MSCC_QS_XTR_DATA_PRESENT] = 0x2c,
	[MSCC_QS_INJ_WR] = 0x3c,
	[MSCC_QS_INJ_CTRL] = 0x44,
};

static const unsigned long luton_regs_ana_table[] = {
	[MSCC_ANA_TABLES_MACHDATA] = 0x11b0,
	[MSCC_ANA_TABLES_MACLDATA] = 0x11b4,
	[MSCC_ANA_TABLES_MACACCESS] = 0x11b8,
};

static struct mscc_miim_dev miim[NUM_PHY];

static struct mii_dev *luton_mdiobus_init(struct udevice *dev,
					  int mdiobus_id)
{
	unsigned long phy_size[NUM_PHY];
	phys_addr_t phy_base[NUM_PHY];
	struct ofnode_phandle_args phandle;
	ofnode eth_node, node, mdio_node;
	struct resource res;
	struct mii_dev *bus;
	fdt32_t faddr;
	int i;

	bus = mdio_alloc();
	if (!bus)
		return NULL;

	/* gather only the first mdio bus */
	eth_node = dev_read_first_subnode(dev);
	node = ofnode_first_subnode(eth_node);
	ofnode_parse_phandle_with_args(node, "phy-handle", NULL, 0, 0,
				       &phandle);
	mdio_node = ofnode_get_parent(phandle.node);

	for (i = 0; i < TARGET_MDIO_MAX; i++) {
		if (ofnode_read_resource(mdio_node, i, &res)) {
			pr_err("%s: get OF resource failed\n", __func__);
			return NULL;
		}

		faddr = cpu_to_fdt32(res.start);
		phy_base[i] = ofnode_translate_address(mdio_node, &faddr);
		phy_size[i] = res.end - res.start;
	}

	strcpy(bus->name, "miim-internal");
	miim[mdiobus_id].regs = ioremap(phy_base[mdiobus_id],
					phy_size[mdiobus_id]);
	bus->priv = &miim[mdiobus_id];
	bus->read = mscc_miim_read;
	bus->write = mscc_miim_write;

	if (mdio_register(bus))
		return NULL;
	else
		return bus;
}

static void luton_stop(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);

	/*
	 * Switch core only reset affects VCORE-III bus and MIPS frequency
	 * and thereby also the DDR SDRAM controller. The workaround is to
	 * not to redirect any trafic to the CPU after the data transfer.
	 */
	writel(GENMASK(9, 2), priv->regs[SYS] + SYS_SCH_CPU);
}

static void luton_cpu_capture_setup(struct luton_private *priv)
{
	int i;

	/* map the 8 CPU extraction queues to CPU port 26 */
	writel(0x0, priv->regs[SYS] + SYS_SCH_CPU);

	for (i = 0; i <= 1; i++) {
		/*
		 * One to one mapping from CPU Queue number to Group extraction
		 * number
		 */
		writel(QS_XTR_MAP_ENA | (QS_XTR_MAP_GRP * i),
		       priv->regs[QS] + QS_XTR_MAP(i));

		/* Enable IFH insertion/parsing on CPU ports */
		setbits_le32(priv->regs[REW] + REW_PORT_CFG(CPU_PORT + i),
			     REW_PORT_CFG_IFH_INSERT_ENA);

		/* Enable IFH parsing on CPU port 0 and 1 */
		setbits_le32(priv->regs[SYS] + SYS_PORT_MODE(CPU_PORT + i),
			     SYS_PORT_MODE_INCL_INJ_HDR);
	}

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA] + ANA_PORT_VLAN_CFG(CPU_PORT));

	/* Disable learning (only RECV_ENA must be set) */
	writel(ANA_PORT_PORT_CFG_RECV_ENA,
	       priv->regs[ANA] + ANA_PORT_PORT_CFG(CPU_PORT));

	/* Enable switching to/from cpu port */
	setbits_le32(priv->regs[SYS] + SYS_SWITCH_PORT_MODE(CPU_PORT),
		     SYS_SWITCH_PORT_MODE_PORT_ENA);

	setbits_le32(priv->regs[SYS] + SYS_EGR_NO_SHARING, BIT(CPU_PORT));
}

static void luton_gmii_port_init(struct luton_private *priv, int port)
{
	void __iomem *regs = priv->regs[port];

	writel(0, regs + DEV_GMII_PORT_MODE_CLK);

	/* Enable MAC RX and TX */
	writel(DEV_GMII_MAC_CFG_MAC_ENA_RX_ENA |
	       DEV_GMII_MAC_CFG_MAC_ENA_TX_ENA,
	       regs + DEV_GMII_MAC_CFG_MAC_ENA);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA] + ANA_PORT_VLAN_CFG(port - PORT0));

	/* Enable switching to/from port */
	setbits_le32(priv->regs[SYS] + SYS_SWITCH_PORT_MODE(port - PORT0),
		     SYS_SWITCH_PORT_MODE_PORT_ENA);
}

static void luton_port_init(struct luton_private *priv, int port)
{
	void __iomem *regs = priv->regs[port];

	writel(0, regs + DEV_PORT_MODE_CLK);

	/* Enable MAC RX and TX */
	writel(DEV_MAC_CFG_MAC_ENA_RX_ENA |
	       DEV_MAC_CFG_MAC_ENA_TX_ENA,
	       regs + DEV_MAC_CFG_MAC_ENA);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA] + ANA_PORT_VLAN_CFG(port - PORT0));

	/* Enable switching to/from port */
	setbits_le32(priv->regs[SYS] + SYS_SWITCH_PORT_MODE(port - PORT0),
		     SYS_SWITCH_PORT_MODE_PORT_ENA);
}

static void luton_ext_port_init(struct luton_private *priv, int port)
{
	void __iomem *regs = priv->regs[port];

	/* Enable PCS */
	writel(DEV_PCS1G_CFG_PCS1G_CFG_PCS_ENA,
	       regs + DEV_PCS1G_CFG_PCS1G_CFG);

	/* Disable Signal Detect */
	writel(0, regs + DEV_PCS1G_CFG_PCS1G_SD);

	/* Enable MAC RX and TX */
	writel(DEV_MAC_CFG_MAC_ENA_RX_ENA |
	       DEV_MAC_CFG_MAC_ENA_TX_ENA,
	       regs + DEV_MAC_CFG_MAC_ENA);

	/* Clear sgmii_mode_ena */
	writel(0, regs + DEV_PCS1G_CFG_PCS1G_MODE);

	/*
	 * Clear sw_resolve_ena(bit 0) and set adv_ability to
	 * something meaningful just in case
	 */
	writel(DEV_PCS1G_CFG_PCS1G_ANEG_ADV_ABILITY(0x20),
	       regs + DEV_PCS1G_CFG_PCS1G_ANEG);

	/* Set MAC IFG Gaps */
	writel(DEV_MAC_CFG_MAC_IFG_TX_IFG(7) |
	       DEV_MAC_CFG_MAC_IFG_RX_IFG1(1) |
	       DEV_MAC_CFG_MAC_IFG_RX_IFG2(5),
	       regs + DEV_MAC_CFG_MAC_IFG);

	/* Set link speed and release all resets */
	writel(DEV_PORT_MODE_CLK_LINK_SPEED_1000,
	       regs + DEV_PORT_MODE_CLK);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA] + ANA_PORT_VLAN_CFG(port - PORT0));

	/* Enable switching to/from port */
	setbits_le32(priv->regs[SYS] + SYS_SWITCH_PORT_MODE(port - PORT0),
		     SYS_SWITCH_PORT_MODE_PORT_ENA);
}

static void serdes6g_write(struct luton_private *priv, u32 addr)
{
	u32 data;

	writel(HSIO_MCB_SERDES6G_CFG_WR_ONE_SHOT |
	       HSIO_MCB_SERDES6G_CFG_ADDR(addr),
	       priv->regs[HSIO] + HSIO_MCB_SERDES6G_CFG);

	do {
		data = readl(priv->regs[HSIO] + HSIO_MCB_SERDES6G_CFG);
	} while (data & HSIO_MCB_SERDES6G_CFG_WR_ONE_SHOT);

	mdelay(100);
}

static void serdes6g_cfg(struct luton_private *priv)
{
	writel(HSIO_RCOMP_CFG_CFG0_MODE_SEL(0x3) |
	       HSIO_RCOMP_CFG_CFG0_RUN_CAL,
	       priv->regs[HSIO] + HSIO_RCOMP_CFG_CFG0);

	while (readl(priv->regs[HSIO] + HSIO_RCOMP_STATUS) &
	       HSIO_RCOMP_STATUS_BUSY)
		;

	writel(HSIO_SERDES6G_ANA_CFG_OB_CFG_SR(0xb) |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG_SR_H |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG_POST0(0x10) |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG_POL |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG_ENA1V_MODE,
	       priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_OB_CFG);
	writel(HSIO_SERDES6G_ANA_CFG_OB_CFG1_LEV(0x18) |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG1_ENA_CAS(0x1),
	       priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_OB_CFG1);
	writel(HSIO_SERDES6G_ANA_CFG_IB_CFG_RESISTOR_CTRL(0xc) |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG_VBCOM(0x4) |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG_VBAC(0x5) |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG_RT(0xf) |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG_RF(0x4),
	       priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_IB_CFG);
	writel(HSIO_SERDES6G_ANA_CFG_IB_CFG1_RST |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_ENA_OFFSDC |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_ENA_OFFSAC |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_ANEG_MODE |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_CHF |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_C(0x4),
	       priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_IB_CFG1);
	writel(HSIO_SERDES6G_ANA_CFG_DES_CFG_BW_ANA(0x5) |
	       HSIO_SERDES6G_ANA_CFG_DES_CFG_BW_HYST(0x5) |
	       HSIO_SERDES6G_ANA_CFG_DES_CFG_MBTR_CTRL(0x2) |
	       HSIO_SERDES6G_ANA_CFG_DES_CFG_PHS_CTRL(0x6),
	       priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_DES_CFG);
	writel(HSIO_SERDES6G_ANA_CFG_PLL_CFG_FSM_ENA |
	       HSIO_SERDES6G_ANA_CFG_PLL_CFG_FSM_CTRL_DATA(0x78),
	       priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_PLL_CFG);
	writel(HSIO_SERDES6G_ANA_CFG_COMMON_CFG_IF_MODE(0x30) |
	       HSIO_SERDES6G_ANA_CFG_COMMON_CFG_ENA_LANE,
	       priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_COMMON_CFG);
	/*
	 * There are 4 serdes6g, configure all except serdes6g0, therefore
	 * the address is b1110
	 */
	serdes6g_write(priv, 0xe);

	writel(readl(priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_COMMON_CFG) |
	       HSIO_SERDES6G_ANA_CFG_COMMON_CFG_SYS_RST,
	       priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_COMMON_CFG);
	serdes6g_write(priv, 0xe);

	clrbits_le32(priv->regs[HSIO] + HSIO_SERDES6G_ANA_CFG_IB_CFG1,
		     HSIO_SERDES6G_ANA_CFG_IB_CFG1_RST);
	writel(HSIO_SERDES6G_DIG_CFG_MISC_CFG_LANE_RST,
	       priv->regs[HSIO] + HSIO_SERDES6G_DIG_CFG_MISC_CFG);
	serdes6g_write(priv, 0xe);
}

static int luton_switch_init(struct luton_private *priv)
{
	setbits_le32(priv->regs[HSIO] + HSIO_PLL5G_CFG_PLL5G_CFG2, BIT(1));
	clrbits_le32(priv->regs[HSIO] + HSIO_PLL5G_CFG_PLL5G_CFG2, BIT(1));

	/* Reset switch & memories */
	writel(SYS_SYSTEM_RST_MEM_ENA | SYS_SYSTEM_RST_MEM_INIT,
	       priv->regs[SYS] + SYS_SYSTEM_RST_CFG);

	/* Wait to complete */
	if (wait_for_bit_le32(priv->regs[SYS] + SYS_SYSTEM_RST_CFG,
			      SYS_SYSTEM_RST_MEM_INIT, false, 2000, false)) {
		printf("Timeout in memory reset\n");
	}

	/* Enable switch core */
	setbits_le32(priv->regs[SYS] + SYS_SYSTEM_RST_CFG,
		     SYS_SYSTEM_RST_CORE_ENA);

	/* Setup the Serdes6g macros */
	serdes6g_cfg(priv);

	return 0;
}

static int luton_initialize(struct luton_private *priv)
{
	int ret, i;

	/* Initialize switch memories, enable core */
	ret = luton_switch_init(priv);
	if (ret)
		return ret;

	/*
	 * Disable port-to-port by switching
	 * Put front ports in "port isolation modes" - i.e. they can't send
	 * to other ports - via the PGID sorce masks.
	 */
	for (i = 0; i < MAX_PORT; i++)
		writel(0, priv->regs[ANA] + ANA_PGID(PGID_SRC + i));

	/* Flush queues */
	mscc_flush(priv->regs[QS], luton_regs_qs);

	/* Setup frame ageing - "2 sec" - The unit is 4ns on Luton*/
	writel(2000000000 / 4,
	       priv->regs[SYS] + SYS_FRM_AGING);

	for (i = PORT0; i < MAX_PORT; i++) {
		if (i < PORT10)
			luton_gmii_port_init(priv, i);
		else
			if (i == PORT10 || i == PORT11)
				luton_port_init(priv, i);
			else
				luton_ext_port_init(priv, i);
	}

	luton_cpu_capture_setup(priv);

	return 0;
}

static int luton_write_hwaddr(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	mscc_mac_table_add(priv->regs[ANA], luton_regs_ana_table,
			   pdata->enetaddr, PGID_UNICAST);

	writel(BIT(CPU_PORT), priv->regs[ANA] + ANA_PGID(PGID_UNICAST));

	return 0;
}

static int luton_start(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const unsigned char mac[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff,
					      0xff };
	int ret;

	ret = luton_initialize(priv);
	if (ret)
		return ret;

	/* Set MAC address tables entries for CPU redirection */
	mscc_mac_table_add(priv->regs[ANA], luton_regs_ana_table,
			   mac, PGID_BROADCAST);

	writel(BIT(CPU_PORT) | INTERNAL_PORT_MSK,
	       priv->regs[ANA] + ANA_PGID(PGID_BROADCAST));

	mscc_mac_table_add(priv->regs[ANA], luton_regs_ana_table,
			   pdata->enetaddr, PGID_UNICAST);

	writel(BIT(CPU_PORT), priv->regs[ANA] + ANA_PGID(PGID_UNICAST));

	return 0;
}

static int luton_send(struct udevice *dev, void *packet, int length)
{
	struct luton_private *priv = dev_get_priv(dev);
	u32 ifh[IFH_LEN];
	int port = BIT(0);	/* use port 0 */
	u32 *buf = packet;

	ifh[0] = IFH_INJ_BYPASS | port;
	ifh[1] = (IFH_TAG_TYPE_C << 16);

	return mscc_send(priv->regs[QS], luton_regs_qs,
			 ifh, IFH_LEN, buf, length);
}

static int luton_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct luton_private *priv = dev_get_priv(dev);
	u32 *rxbuf = (u32 *)net_rx_packets[0];
	int byte_cnt = 0;

	byte_cnt = mscc_recv(priv->regs[QS], luton_regs_qs, rxbuf, IFH_LEN,
			     true);

	*packetp = net_rx_packets[0];

	return byte_cnt;
}

static int luton_probe(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);
	int i;

	struct {
		enum luton_target id;
		char *name;
	} reg[] = {
		{ PORT0, "port0" },
		{ PORT1, "port1" },
		{ PORT2, "port2" },
		{ PORT3, "port3" },
		{ PORT4, "port4" },
		{ PORT5, "port5" },
		{ PORT6, "port6" },
		{ PORT7, "port7" },
		{ PORT8, "port8" },
		{ PORT9, "port9" },
		{ PORT10, "port10" },
		{ PORT11, "port11" },
		{ PORT12, "port12" },
		{ PORT13, "port13" },
		{ PORT14, "port14" },
		{ PORT15, "port15" },
		{ PORT16, "port16" },
		{ PORT17, "port17" },
		{ PORT18, "port18" },
		{ PORT19, "port19" },
		{ PORT20, "port20" },
		{ PORT21, "port21" },
		{ PORT22, "port22" },
		{ PORT23, "port23" },
		{ SYS, "sys" },
		{ ANA, "ana" },
		{ REW, "rew" },
		{ GCB, "gcb" },
		{ QS, "qs" },
		{ HSIO, "hsio" },
	};

	if (!priv)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(reg); i++) {
		priv->regs[reg[i].id] = dev_remap_addr_name(dev, reg[i].name);
		if (!priv->regs[reg[i].id]) {
			debug
			    ("Error can't get regs base addresses for %s\n",
			     reg[i].name);
			return -ENOMEM;
		}
	}

	/* Release reset in the CU-PHY */
	writel(0, priv->regs[GCB] + GCB_DEVCPU_RST_SOFT_CHIP_RST);

	/* Ports with ext phy don't need to reset clk */
	for (i = PORT0; i < MAX_INT_PORT; i++) {
		if (i < PORT10)
			clrbits_le32(priv->regs[i] + DEV_GMII_PORT_MODE_CLK,
				     DEV_GMII_PORT_MODE_CLK_PHY_RST);
		else
			clrbits_le32(priv->regs[i] + DEV_PORT_MODE_CLK,
				     DEV_PORT_MODE_CLK_PHY_RST);
	}

	/* Wait for internal PHY to be ready */
	if (wait_for_bit_le32(priv->regs[GCB] + GCB_MISC_STAT,
			      GCB_MISC_STAT_PHY_READY, true, 500, false))
		return -EACCES;

	priv->bus[INTERNAL] = luton_mdiobus_init(dev, INTERNAL);

	for (i = 0; i < MAX_INT_PORT; i++) {
		phy_connect(priv->bus[INTERNAL], i, dev,
			    PHY_INTERFACE_MODE_NONE);
	}

	/*
	 * coma_mode is need on only one phy, because all the other phys
	 * will be affected.
	 */
	mscc_miim_write(priv->bus[INTERNAL], 0, 0, 31, 0x10);
	mscc_miim_write(priv->bus[INTERNAL], 0, 0, 14, 0x800);
	mscc_miim_write(priv->bus[INTERNAL], 0, 0, 31, 0);

	return 0;
}

static int luton_remove(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < NUM_PHY; i++) {
		mdio_unregister(priv->bus[i]);
		mdio_free(priv->bus[i]);
	}

	return 0;
}

static const struct eth_ops luton_ops = {
	.start        = luton_start,
	.stop         = luton_stop,
	.send         = luton_send,
	.recv         = luton_recv,
	.write_hwaddr = luton_write_hwaddr,
};

static const struct udevice_id mscc_luton_ids[] = {
	{.compatible = "mscc,vsc7527-switch", },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(luton) = {
	.name     = "luton-switch",
	.id       = UCLASS_ETH,
	.of_match = mscc_luton_ids,
	.probe	  = luton_probe,
	.remove	  = luton_remove,
	.ops	  = &luton_ops,
	.priv_auto_alloc_size = sizeof(struct luton_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
