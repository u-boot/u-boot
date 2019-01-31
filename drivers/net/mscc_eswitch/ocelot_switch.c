// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
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

#define MIIM_STATUS			0x0
#define		MIIM_STAT_BUSY			BIT(3)
#define MIIM_CMD			0x8
#define		MIIM_CMD_SCAN		BIT(0)
#define		MIIM_CMD_OPR_WRITE	BIT(1)
#define		MIIM_CMD_OPR_READ	BIT(2)
#define		MIIM_CMD_SINGLE_SCAN	BIT(3)
#define		MIIM_CMD_WRDATA(x)	((x) << 4)
#define		MIIM_CMD_REGAD(x)	((x) << 20)
#define		MIIM_CMD_PHYAD(x)	((x) << 25)
#define		MIIM_CMD_VLD		BIT(31)
#define MIIM_DATA			0xC
#define		MIIM_DATA_ERROR		(0x2 << 16)

#define PHY_CFG				0x0
#define PHY_CFG_ENA				0xF
#define PHY_CFG_COMMON_RST			BIT(4)
#define PHY_CFG_RST				(0xF << 5)
#define PHY_STAT			0x4
#define PHY_STAT_SUPERVISOR_COMPLETE		BIT(0)

#define ANA_PORT_VLAN_CFG(x)		(0x7000 + 0x100 * (x))
#define		ANA_PORT_VLAN_CFG_AWARE_ENA	BIT(20)
#define		ANA_PORT_VLAN_CFG_POP_CNT(x)	((x) << 18)
#define ANA_PORT_PORT_CFG(x)		(0x7070 + 0x100 * (x))
#define		ANA_PORT_PORT_CFG_RECV_ENA	BIT(6)
#define	ANA_TABLES_MACHDATA		0x8b34
#define	ANA_TABLES_MACLDATA		0x8b38
#define ANA_TABLES_MACACCESS		0x8b3c
#define		ANA_TABLES_MACACCESS_VALID	BIT(11)
#define		ANA_TABLES_MACACCESS_ENTRYTYPE(x)   ((x) << 9)
#define		ANA_TABLES_MACACCESS_DEST_IDX(x)    ((x) << 3)
#define		ANA_TABLES_MACACCESS_MAC_TABLE_CMD(x)	(x)
#define		ANA_TABLES_MACACCESS_MAC_TABLE_CMD_M	GENMASK(2, 0)
#define		MACACCESS_CMD_IDLE                     0
#define		MACACCESS_CMD_LEARN                    1
#define		MACACCESS_CMD_GET_NEXT                 4
#define ANA_PGID(x)			(0x8c00 + 4 * (x))

#define SYS_FRM_AGING			0x574
#define		SYS_FRM_AGING_ENA		BIT(20)

#define SYS_SYSTEM_RST_CFG		0x508
#define		SYS_SYSTEM_RST_MEM_INIT		BIT(0)
#define		SYS_SYSTEM_RST_MEM_ENA		BIT(1)
#define		SYS_SYSTEM_RST_CORE_ENA		BIT(2)
#define SYS_PORT_MODE(x)		(0x514 + 0x4 * (x))
#define		SYS_PORT_MODE_INCL_INJ_HDR(x)	((x) << 3)
#define		SYS_PORT_MODE_INCL_INJ_HDR_M	GENMASK(4, 3)
#define		SYS_PORT_MODE_INCL_XTR_HDR(x)	((x) << 1)
#define		SYS_PORT_MODE_INCL_XTR_HDR_M	GENMASK(2, 1)
#define	SYS_PAUSE_CFG(x)		(0x608 + 0x4 * (x))
#define		SYS_PAUSE_CFG_PAUSE_ENA		BIT(0)

#define QSYS_SWITCH_PORT_MODE(x)	(0x11234 + 0x4 * (x))
#define		QSYS_SWITCH_PORT_MODE_PORT_ENA	BIT(14)
#define	QSYS_QMAP			0x112d8
#define	QSYS_EGR_NO_SHARING		0x1129c

/* Port registers */
#define DEV_CLOCK_CFG			0x0
#define DEV_CLOCK_CFG_LINK_SPEED_1000		1
#define DEV_MAC_ENA_CFG			0x1c
#define		DEV_MAC_ENA_CFG_RX_ENA		BIT(4)
#define		DEV_MAC_ENA_CFG_TX_ENA		BIT(0)

#define DEV_MAC_IFG_CFG			0x30
#define		DEV_MAC_IFG_CFG_TX_IFG(x)	((x) << 8)
#define		DEV_MAC_IFG_CFG_RX_IFG2(x)	((x) << 4)
#define		DEV_MAC_IFG_CFG_RX_IFG1(x)	(x)

#define PCS1G_CFG			0x48
#define		PCS1G_MODE_CFG_SGMII_MODE_ENA	BIT(0)
#define PCS1G_MODE_CFG			0x4c
#define		PCS1G_MODE_CFG_UNIDIR_MODE_ENA	BIT(4)
#define		PCS1G_MODE_CFG_SGMII_MODE_ENA	BIT(0)
#define PCS1G_SD_CFG			0x50
#define PCS1G_ANEG_CFG			0x54
#define		PCS1G_ANEG_CFG_ADV_ABILITY(x)	((x) << 16)

#define QS_XTR_GRP_CFG(x)		(4 * (x))
#define QS_XTR_GRP_CFG_MODE(x)			((x) << 2)
#define		QS_XTR_GRP_CFG_STATUS_WORD_POS	BIT(1)
#define		QS_XTR_GRP_CFG_BYTE_SWAP	BIT(0)
#define QS_XTR_RD(x)			(0x8 + 4 * (x))
#define QS_XTR_FLUSH			0x18
#define		QS_XTR_FLUSH_FLUSH		GENMASK(1, 0)
#define QS_XTR_DATA_PRESENT		0x1c
#define QS_INJ_GRP_CFG(x)		(0x24 + (x) * 4)
#define		QS_INJ_GRP_CFG_MODE(x)		((x) << 2)
#define		QS_INJ_GRP_CFG_BYTE_SWAP	BIT(0)
#define QS_INJ_WR(x)			(0x2c + 4 * (x))
#define QS_INJ_CTRL(x)			(0x34 + 4 * (x))
#define		QS_INJ_CTRL_GAP_SIZE(x)		((x) << 21)
#define		QS_INJ_CTRL_EOF			BIT(19)
#define		QS_INJ_CTRL_SOF			BIT(18)
#define		QS_INJ_CTRL_VLD_BYTES(x)	((x) << 16)

#define XTR_EOF_0     ntohl(0x80000000u)
#define XTR_EOF_1     ntohl(0x80000001u)
#define XTR_EOF_2     ntohl(0x80000002u)
#define XTR_EOF_3     ntohl(0x80000003u)
#define XTR_PRUNED    ntohl(0x80000004u)
#define XTR_ABORT     ntohl(0x80000005u)
#define XTR_ESCAPE    ntohl(0x80000006u)
#define XTR_NOT_READY ntohl(0x80000007u)

#define IFH_INJ_BYPASS		BIT(31)
#define	IFH_TAG_TYPE_C		0
#define XTR_VALID_BYTES(x)	(4 - ((x) & 3))
#define	MAC_VID			1
#define CPU_PORT		11
#define INTERNAL_PORT_MSK	0xF
#define IFH_LEN			4
#define OCELOT_BUF_CELL_SZ	60
#define ETH_ALEN		6
#define	PGID_BROADCAST		13
#define	PGID_UNICAST		14
#define	PGID_SRC		80

enum ocelot_target {
	ANA,
	QS,
	QSYS,
	REW,
	SYS,
	HSIO,
	PORT0,
	PORT1,
	PORT2,
	PORT3,
	TARGET_MAX,
};

#define MAX_PORT (PORT3 - PORT0)

/* MAC table entry types.
 * ENTRYTYPE_NORMAL is subject to aging.
 * ENTRYTYPE_LOCKED is not subject to aging.
 * ENTRYTYPE_MACv4 is not subject to aging. For IPv4 multicast.
 * ENTRYTYPE_MACv6 is not subject to aging. For IPv6 multicast.
 */
enum macaccess_entry_type {
	ENTRYTYPE_NORMAL = 0,
	ENTRYTYPE_LOCKED,
	ENTRYTYPE_MACv4,
	ENTRYTYPE_MACv6,
};

enum ocelot_mdio_target {
	MIIM,
	PHY,
	TARGET_MDIO_MAX,
};

enum ocelot_phy_id {
	INTERNAL,
	EXTERNAL,
	NUM_PHY,
};

struct ocelot_private {
	void __iomem *regs[TARGET_MAX];

	struct mii_dev *bus[NUM_PHY];
	struct phy_device *phydev;
	int phy_mode;
	int max_speed;

	int rx_pos;
	int rx_siz;
	int rx_off;
	int tx_num;

	u8 tx_adj_packetbuf[PKTSIZE_ALIGN + PKTALIGN];
	void *tx_adj_buf;
};

struct mscc_miim_dev {
	void __iomem *regs;
	void __iomem *phy_regs;
};

struct mscc_miim_dev miim[NUM_PHY];

static int mscc_miim_wait_ready(struct mscc_miim_dev *miim)
{
	return wait_for_bit_le32(miim->regs + MIIM_STATUS, MIIM_STAT_BUSY,
				 false, 250, false);
}

static int mscc_miim_reset(struct mii_dev *bus)
{
	struct mscc_miim_dev *miim = (struct mscc_miim_dev *)bus->priv;

	if (miim->phy_regs) {
		writel(0, miim->phy_regs + PHY_CFG);
		writel(PHY_CFG_RST | PHY_CFG_COMMON_RST
		       | PHY_CFG_ENA, miim->phy_regs + PHY_CFG);
		mdelay(500);
	}

	return 0;
}

static int mscc_miim_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mscc_miim_dev *miim = (struct mscc_miim_dev *)bus->priv;
	u32 val;
	int ret;

	ret = mscc_miim_wait_ready(miim);
	if (ret)
		goto out;

	writel(MIIM_CMD_VLD | MIIM_CMD_PHYAD(addr) |
	       MIIM_CMD_REGAD(reg) | MIIM_CMD_OPR_READ,
	       miim->regs + MIIM_CMD);

	ret = mscc_miim_wait_ready(miim);
	if (ret)
		goto out;

	val = readl(miim->regs + MIIM_DATA);
	if (val & MIIM_DATA_ERROR) {
		ret = -EIO;
		goto out;
	}

	ret = val & 0xFFFF;
 out:
	return ret;
}

static int mscc_miim_write(struct mii_dev *bus, int addr, int devad, int reg,
			   u16 val)
{
	struct mscc_miim_dev *miim = (struct mscc_miim_dev *)bus->priv;
	int ret;

	ret = mscc_miim_wait_ready(miim);
	if (ret < 0)
		goto out;

	writel(MIIM_CMD_VLD | MIIM_CMD_PHYAD(addr) |
	       MIIM_CMD_REGAD(reg) | MIIM_CMD_WRDATA(val) |
	       MIIM_CMD_OPR_WRITE, miim->regs + MIIM_CMD);
 out:
	return ret;
}

/* For now only setup the internal mdio bus */
static struct mii_dev *ocelot_mdiobus_init(struct udevice *dev)
{
	unsigned long phy_size[TARGET_MAX];
	phys_addr_t phy_base[TARGET_MAX];
	struct ofnode_phandle_args phandle;
	ofnode eth_node, node, mdio_node;
	struct resource res;
	struct mii_dev *bus;
	fdt32_t faddr;
	int i;

	bus = mdio_alloc();

	if (!bus)
		return NULL;

	/* gathered only the first mdio bus */
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
	miim[INTERNAL].phy_regs = ioremap(phy_base[PHY], phy_size[PHY]);
	miim[INTERNAL].regs = ioremap(phy_base[MIIM], phy_size[MIIM]);
	bus->priv = &miim[INTERNAL];
	bus->reset = mscc_miim_reset;
	bus->read = mscc_miim_read;
	bus->write = mscc_miim_write;

	if (mdio_register(bus))
		return NULL;
	else
		return bus;
}

__weak void mscc_switch_reset(void)
{
}

static void ocelot_stop(struct udevice *dev)
{
	struct ocelot_private *priv = dev_get_priv(dev);
	int i;

	mscc_switch_reset();
	for (i = 0; i < NUM_PHY; i++)
		if (priv->bus[i])
			mscc_miim_reset(priv->bus[i]);
}

static void ocelot_cpu_capture_setup(struct ocelot_private *priv)
{
	int i;

	/* map the 8 CPU extraction queues to CPU port 11 */
	writel(0, priv->regs[QSYS] + QSYS_QMAP);

	for (i = 0; i <= 1; i++) {
		/*
		 * Do byte-swap and expect status after last data word
		 * Extraction: Mode: manual extraction) | Byte_swap
		 */
		writel(QS_XTR_GRP_CFG_MODE(1) | QS_XTR_GRP_CFG_BYTE_SWAP,
		       priv->regs[QS] + QS_XTR_GRP_CFG(i));
		/*
		 * Injection: Mode: manual extraction | Byte_swap
		 */
		writel(QS_INJ_GRP_CFG_MODE(1) | QS_INJ_GRP_CFG_BYTE_SWAP,
		       priv->regs[QS] + QS_INJ_GRP_CFG(i));
	}

	for (i = 0; i <= 1; i++)
		/* Enable IFH insertion/parsing on CPU ports */
		writel(SYS_PORT_MODE_INCL_INJ_HDR(1) |
		       SYS_PORT_MODE_INCL_XTR_HDR(1),
		       priv->regs[SYS] + SYS_PORT_MODE(CPU_PORT + i));
	/*
	 * Setup the CPU port as VLAN aware to support switching frames
	 * based on tags
	 */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA | ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID, priv->regs[ANA] + ANA_PORT_VLAN_CFG(CPU_PORT));

	/* Disable learning (only RECV_ENA must be set) */
	writel(ANA_PORT_PORT_CFG_RECV_ENA,
	       priv->regs[ANA] + ANA_PORT_PORT_CFG(CPU_PORT));

	/* Enable switching to/from cpu port */
	setbits_le32(priv->regs[QSYS] + QSYS_SWITCH_PORT_MODE(CPU_PORT),
		     QSYS_SWITCH_PORT_MODE_PORT_ENA);

	/* No pause on CPU port - not needed (off by default) */
	clrbits_le32(priv->regs[SYS] + SYS_PAUSE_CFG(CPU_PORT),
		     SYS_PAUSE_CFG_PAUSE_ENA);

	setbits_le32(priv->regs[QSYS] + QSYS_EGR_NO_SHARING, BIT(CPU_PORT));
}

static void ocelot_port_init(struct ocelot_private *priv, int port)
{
	void __iomem *regs = priv->regs[port];

	/* Enable PCS */
	writel(PCS1G_MODE_CFG_SGMII_MODE_ENA, regs + PCS1G_CFG);

	/* Disable Signal Detect */
	writel(0, regs + PCS1G_SD_CFG);

	/* Enable MAC RX and TX */
	writel(DEV_MAC_ENA_CFG_RX_ENA | DEV_MAC_ENA_CFG_TX_ENA,
	       regs + DEV_MAC_ENA_CFG);

	/* Clear sgmii_mode_ena */
	writel(0, regs + PCS1G_MODE_CFG);

	/*
	 * Clear sw_resolve_ena(bit 0) and set adv_ability to
	 * something meaningful just in case
	 */
	writel(PCS1G_ANEG_CFG_ADV_ABILITY(0x20), regs + PCS1G_ANEG_CFG);

	/* Set MAC IFG Gaps */
	writel(DEV_MAC_IFG_CFG_TX_IFG(5) | DEV_MAC_IFG_CFG_RX_IFG1(5) |
	       DEV_MAC_IFG_CFG_RX_IFG2(1), regs + DEV_MAC_IFG_CFG);

	/* Set link speed and release all resets */
	writel(DEV_CLOCK_CFG_LINK_SPEED_1000, regs + DEV_CLOCK_CFG);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA | ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID, priv->regs[ANA] + ANA_PORT_VLAN_CFG(port - PORT0));

	/* Enable the port in the core */
	setbits_le32(priv->regs[QSYS] + QSYS_SWITCH_PORT_MODE(port - PORT0),
		     QSYS_SWITCH_PORT_MODE_PORT_ENA);
}

static int ocelot_switch_init(struct ocelot_private *priv)
{
	/* Reset switch & memories */
	writel(SYS_SYSTEM_RST_MEM_ENA | SYS_SYSTEM_RST_MEM_INIT,
	       priv->regs[SYS] + SYS_SYSTEM_RST_CFG);

	/* Wait to complete */
	if (wait_for_bit_le32(priv->regs[SYS] + SYS_SYSTEM_RST_CFG,
			      SYS_SYSTEM_RST_MEM_INIT, false, 2000, false)) {
		pr_err("Timeout in memory reset\n");
		return -EIO;
	}

	/* Enable switch core */
	setbits_le32(priv->regs[SYS] + SYS_SYSTEM_RST_CFG,
		     SYS_SYSTEM_RST_CORE_ENA);

	return 0;
}

static void ocelot_switch_flush(struct ocelot_private *priv)
{
	/* All Queues flush */
	setbits_le32(priv->regs[QS] + QS_XTR_FLUSH, QS_XTR_FLUSH_FLUSH);
	/* Allow to drain */
	mdelay(1);
	/* All Queues normal */
	clrbits_le32(priv->regs[QS] + QS_XTR_FLUSH, QS_XTR_FLUSH_FLUSH);
}

static int ocelot_initialize(struct ocelot_private *priv)
{
	int ret, i;

	/* Initialize switch memories, enable core */
	ret = ocelot_switch_init(priv);
	if (ret)
		return ret;
	/*
	 * Disable port-to-port by switching
	 * Put fron ports in "port isolation modes" - i.e. they cant send
	 * to other ports - via the PGID sorce masks.
	 */
	for (i = 0; i <= MAX_PORT; i++)
		writel(0, priv->regs[ANA] + ANA_PGID(PGID_SRC + i));

	/* Flush queues */
	ocelot_switch_flush(priv);

	/* Setup frame ageing - "2 sec" - The unit is 6.5us on Ocelot */
	writel(SYS_FRM_AGING_ENA | (20000000 / 65),
	       priv->regs[SYS] + SYS_FRM_AGING);

	for (i = PORT0; i <= PORT3; i++)
		ocelot_port_init(priv, i);

	ocelot_cpu_capture_setup(priv);

	debug("Ports enabled\n");

	return 0;
}

static inline int ocelot_vlant_wait_for_completion(struct ocelot_private *priv)
{
	unsigned int val, timeout = 10;

	/* Wait for the issued mac table command to be completed, or timeout.
	 * When the command read from ANA_TABLES_MACACCESS is
	 * MACACCESS_CMD_IDLE, the issued command completed successfully.
	 */
	do {
		val = readl(priv->regs[ANA] + ANA_TABLES_MACACCESS);
		val &= ANA_TABLES_MACACCESS_MAC_TABLE_CMD_M;
	} while (val != MACACCESS_CMD_IDLE && timeout--);

	if (!timeout)
		return -ETIMEDOUT;

	return 0;
}

static int ocelot_mac_table_add(struct ocelot_private *priv,
				const unsigned char mac[ETH_ALEN], int pgid)
{
	u32 macl = 0, mach = 0;
	int ret;

	/* Set the MAC address to handle and the vlan associated in a format
	 * understood by the hardware.
	 */
	mach |= MAC_VID << 16;
	mach |= ((u32)mac[0]) << 8;
	mach |= ((u32)mac[1]) << 0;
	macl |= ((u32)mac[2]) << 24;
	macl |= ((u32)mac[3]) << 16;
	macl |= ((u32)mac[4]) << 8;
	macl |= ((u32)mac[5]) << 0;

	writel(macl, priv->regs[ANA] + ANA_TABLES_MACLDATA);
	writel(mach, priv->regs[ANA] + ANA_TABLES_MACHDATA);

	writel(ANA_TABLES_MACACCESS_VALID |
	       ANA_TABLES_MACACCESS_DEST_IDX(pgid) |
	       ANA_TABLES_MACACCESS_ENTRYTYPE(ENTRYTYPE_LOCKED) |
	       ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MACACCESS_CMD_LEARN),
	       priv->regs[ANA] + ANA_TABLES_MACACCESS);

	ret = ocelot_vlant_wait_for_completion(priv);

	return ret;
}

static int ocelot_write_hwaddr(struct udevice *dev)
{
	struct ocelot_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	ocelot_mac_table_add(priv, pdata->enetaddr, PGID_UNICAST);

	writel(BIT(CPU_PORT), priv->regs[ANA] + ANA_PGID(PGID_UNICAST));

	return 0;
}

static int ocelot_start(struct udevice *dev)
{
	struct ocelot_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const unsigned char mac[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff,
					      0xff };
	int ret;

	ret = ocelot_initialize(priv);
	if (ret)
		return ret;

	/* Set MAC address tables entries for CPU redirection */
	ocelot_mac_table_add(priv, mac, PGID_BROADCAST);

	writel(BIT(CPU_PORT) | INTERNAL_PORT_MSK,
	       priv->regs[ANA] + ANA_PGID(PGID_BROADCAST));

	/* It should be setup latter in ocelot_write_hwaddr */
	ocelot_mac_table_add(priv, pdata->enetaddr, PGID_UNICAST);

	writel(BIT(CPU_PORT), priv->regs[ANA] + ANA_PGID(PGID_UNICAST));

	return 0;
}

static int ocelot_send(struct udevice *dev, void *packet, int length)
{
	struct ocelot_private *priv = dev_get_priv(dev);
	u32 ifh[IFH_LEN];
	int port = BIT(0);	/* use port 0 */
	u8 grp = 0;		/* Send everything on CPU group 0 */
	int i, count = (length + 3) / 4, last = length % 4;
	u32 *buf = packet;

	writel(QS_INJ_CTRL_GAP_SIZE(1) | QS_INJ_CTRL_SOF,
	       priv->regs[QS] + QS_INJ_CTRL(grp));

	/*
	 * Generate the IFH for frame injection
	 *
	 * The IFH is a 128bit-value
	 * bit 127: bypass the analyzer processing
	 * bit 56-67: destination mask
	 * bit 28-29: pop_cnt: 3 disables all rewriting of the frame
	 * bit 20-27: cpu extraction queue mask
	 * bit 16: tag type 0: C-tag, 1: S-tag
	 * bit 0-11: VID
	 */
	ifh[0] = IFH_INJ_BYPASS;
	ifh[1] = (0xf00 & port) >> 8;
	ifh[2] = (0xff & port) << 24;
	ifh[3] = (IFH_TAG_TYPE_C << 16);

	for (i = 0; i < IFH_LEN; i++)
		writel(ifh[i], priv->regs[QS] + QS_INJ_WR(grp));

	for (i = 0; i < count; i++)
		writel(buf[i], priv->regs[QS] + QS_INJ_WR(grp));

	/* Add padding */
	while (i < (OCELOT_BUF_CELL_SZ / 4)) {
		writel(0, priv->regs[QS] + QS_INJ_WR(grp));
		i++;
	}

	/* Indicate EOF and valid bytes in last word */
	writel(QS_INJ_CTRL_GAP_SIZE(1) |
	       QS_INJ_CTRL_VLD_BYTES(length < OCELOT_BUF_CELL_SZ ? 0 : last) |
	       QS_INJ_CTRL_EOF, priv->regs[QS] + QS_INJ_CTRL(grp));

	/* Add dummy CRC */
	writel(0, priv->regs[QS] + QS_INJ_WR(grp));

	return 0;
}

static int ocelot_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ocelot_private *priv = dev_get_priv(dev);
	u8 grp = 0;		/* Send everything on CPU group 0 */
	u32 *rxbuf = (u32 *)net_rx_packets[0];
	int i, byte_cnt = 0;
	bool eof_flag = false, pruned_flag = false, abort_flag = false;

	if (!(readl(priv->regs[QS] + QS_XTR_DATA_PRESENT) & BIT(grp)))
		return -EAGAIN;

	/* skip IFH */
	for (i = 0; i < IFH_LEN; i++)
		readl(priv->regs[QS] + QS_XTR_RD(grp));

	while (!eof_flag) {
		u32 val = readl(priv->regs[QS] + QS_XTR_RD(grp));

		switch (val) {
		case XTR_NOT_READY:
			debug("%d NOT_READY...?\n", byte_cnt);
			break;
		case XTR_ABORT:
			/* really nedeed?? not done in linux */
			*rxbuf = readl(priv->regs[QS] + QS_XTR_RD(grp));
			abort_flag = true;
			eof_flag = true;
			debug("XTR_ABORT\n");
			break;
		case XTR_EOF_0:
		case XTR_EOF_1:
		case XTR_EOF_2:
		case XTR_EOF_3:
			byte_cnt += XTR_VALID_BYTES(val);
			*rxbuf = readl(priv->regs[QS] + QS_XTR_RD(grp));
			eof_flag = true;
			debug("EOF\n");
			break;
		case XTR_PRUNED:
			/* But get the last 4 bytes as well */
			eof_flag = true;
			pruned_flag = true;
			debug("PRUNED\n");
			/* fallthrough */
		case XTR_ESCAPE:
			*rxbuf = readl(priv->regs[QS] + QS_XTR_RD(grp));
			byte_cnt += 4;
			rxbuf++;
			debug("ESCAPED\n");
			break;
		default:
			*rxbuf = val;
			byte_cnt += 4;
			rxbuf++;
		}
	}

	if (abort_flag || pruned_flag || !eof_flag) {
		debug("Discarded frame: abort:%d pruned:%d eof:%d\n",
		      abort_flag, pruned_flag, eof_flag);
		return -EAGAIN;
	}

	*packetp = net_rx_packets[0];

	return byte_cnt;
}

static int ocelot_probe(struct udevice *dev)
{
	struct ocelot_private *priv = dev_get_priv(dev);
	int ret, i;

	struct {
		enum ocelot_target id;
		char *name;
	} reg[] = {
		{ SYS, "sys" },
		{ REW, "rew" },
		{ QSYS, "qsys" },
		{ ANA, "ana" },
		{ QS, "qs" },
		{ HSIO, "hsio" },
		{ PORT0, "port0" },
		{ PORT1, "port1" },
		{ PORT2, "port2" },
		{ PORT3, "port3" },
	};

	for (i = 0; i < ARRAY_SIZE(reg); i++) {
		priv->regs[reg[i].id] = dev_remap_addr_name(dev, reg[i].name);
		if (!priv->regs[reg[i].id]) {
			pr_err
			    ("Error %d: can't get regs base addresses for %s\n",
			     ret, reg[i].name);
			return -ENOMEM;
		}
	}

	priv->bus[INTERNAL] = ocelot_mdiobus_init(dev);

	for (i = 0; i < 4; i++) {
		phy_connect(priv->bus[INTERNAL], i, dev,
			    PHY_INTERFACE_MODE_NONE);
	}

	return 0;
}

static int ocelot_remove(struct udevice *dev)
{
	struct ocelot_private *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < NUM_PHY; i++) {
		mdio_unregister(priv->bus[i]);
		mdio_free(priv->bus[i]);
	}

	return 0;
}

static const struct eth_ops ocelot_ops = {
	.start        = ocelot_start,
	.stop         = ocelot_stop,
	.send         = ocelot_send,
	.recv         = ocelot_recv,
	.write_hwaddr = ocelot_write_hwaddr,
};

static const struct udevice_id mscc_ocelot_ids[] = {
	{.compatible = "mscc,vsc7514-switch"},
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(ocelot) = {
	.name     = "ocelot-switch",
	.id       = UCLASS_ETH,
	.of_match = mscc_ocelot_ids,
	.probe	  = ocelot_probe,
	.remove	  = ocelot_remove,
	.ops	  = &ocelot_ops,
	.priv_auto_alloc_size = sizeof(struct ocelot_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
