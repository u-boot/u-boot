// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
 * NXP NETC Blocks Control Driver
 *
 * Copyright 2024 NXP
 *
 * This driver is used for pre-initialization of NETC, such as PCS and MII
 * protocols, LDID, warm reset, etc. Therefore, all NETC device drivers can
 * only be probed after the netc-blk-crtl driver has completed initialization.
 * In addition, when the system enters suspend mode, IERB, PRB, and NETCMIX
 * will be powered off, except for WOL. Therefore, when the system resumes,
 * these blocks need to be reinitialized.
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/iopoll.h>
#include <phy_interface.h>

/* NETCMIX registers */
#define IMX95_CFG_LINK_IO_VAR		0x0
#define  IO_VAR_16FF_16G_SERDES		0x1
#define  IO_VAR(port, var)		(((var) & 0xf) << ((port) << 2))

#define IMX95_CFG_LINK_MII_PROT		0x4
#define CFG_LINK_MII_PORT_0		GENMASK(3, 0)
#define CFG_LINK_MII_PORT_1		GENMASK(7, 4)
#define  MII_PROT_MII			0x0
#define  MII_PROT_RMII			0x1
#define  MII_PROT_RGMII			0x2
#define  MII_PROT_SERIAL		0x3
#define  MII_PROT(port, prot)		(((prot) & 0xf) << ((port) << 2))

#define IMX95_CFG_LINK_PCS_PROT(a)	(0x8 + (a) * 4)
#define PCS_PROT_1G_SGMII		BIT(0)
#define PCS_PROT_2500M_SGMII		BIT(1)
#define PCS_PROT_XFI			BIT(3)
#define PCS_PROT_SFI			BIT(4)
#define PCS_PROT_10G_SXGMII		BIT(6)

#define IMX94_MISC_SOC_CONTROL		0x0
#define SEL_XPCS_1			BIT(1)
#define IMX94_XPCS_PORT_0		0x0
#define IMX94_XPCS_PORT_1		0x1

#define IMX94_EXT_PIN_CONTROL		0x10
#define MAC2_MAC3_SEL			BIT(1)

#define IMX94_NETC_LINK_CFG(a)		(0x4c + (a) * 4)
#define NETC_LINK_CFG_MII_PROT		GENMASK(3, 0)
#define NETC_LINK_CFG_IO_VAR		GENMASK(19, 16)

/* NETC privileged register block register */
#define PRB_NETCRR			0x100
#define  NETCRR_SR			BIT(0)
#define  NETCRR_LOCK			BIT(1)

#define PRB_NETCSR			0x104
#define  NETCSR_ERROR			BIT(0)
#define  NETCSR_STATE			BIT(1)

/* NETC integrated endpoint register block register */
#define IERB_EMDIOFAUXR			0x344
#define IERB_T0FAUXR			0x444
#define IERB_ETBCR(a)			(0x300c + 0x100 * (a))
#define IERB_EFAUXR(a)			(0x3044 + 0x100 * (a))
#define IERB_VFAUXR(a)			(0x4004 + 0x40 * (a))
#define FAUXR_LDID			GENMASK(3, 0)

/* Platform information */
#define IMX95_ENETC0_BUS_DEVFN		0x0
#define IMX95_ENETC1_BUS_DEVFN		0x40
#define IMX95_ENETC2_BUS_DEVFN		0x80

#define IMX94_ENETC3_BUS_DEVFN		0x0
#define IMX94_TIMER0_BUS_DEVFN		0x1
#define IMX94_SWITCH_BUS_DEVFN		0x2
#define IMX94_ENETC0_BUS_DEVFN		0x100
#define IMX94_TIMER1_BUS_DEVFN		0x101
#define IMX94_ENETC1_BUS_DEVFN		0x140
#define IMX94_ENETC2_BUS_DEVFN		0x180
#define IMX94_TIMER2_BUS_DEVFN		0x181
#define IMX94_ENETC0_LINK		3
#define IMX94_ENETC1_LINK		4
#define IMX94_ENETC2_LINK		5
#define IMX94_ENETC0_OFFSET		0
#define IMX94_ENETC1_OFFSET		1
#define IMX94_ENETC2_OFFSET		2
#define IMX94_SWITCH_PORT2		2
#define IMX94_SWITCH_CPU_PORT		3
#define IMX94_TIMER0_ID			0
#define IMX94_TIMER1_ID			1
#define IMX94_TIMER2_ID			2

/* Flags for different platforms */
#define NETC_HAS_NETCMIX		BIT(0)

struct netc_blk_ctrl {
	void __iomem *prb;
	void __iomem *ierb;
	void __iomem *netcmix;
};

struct netc_devinfo {
	int (*netcmix_init)(struct udevice *dev);
	int (*ierb_init)(struct udevice *dev);
	void (*xpcs_port_init)(struct netc_blk_ctrl *priv, int port);
};

static struct netc_blk_ctrl *netc_bc;
static struct netc_devinfo *netc_di;

static void netc_reg_write(void __iomem *base, u32 offset, u32 val)
{
	writel(val, base + offset);
}

static u32 netc_reg_read(void __iomem *base, u32 offset)
{
	return readl(base + offset);
}

static int netc_of_pci_get_bus_devfn(ofnode node)
{
	u32 reg[5];
	int error;

	error = ofnode_read_u32_array(node, "reg", reg, ARRAY_SIZE(reg));
	if (error)
		return error;

	return (reg[0] >> 8) & 0xffff;
}

static int netc_get_link_mii_protocol(phy_interface_t interface)
{
	switch (interface) {
	case PHY_INTERFACE_MODE_MII:
		return MII_PROT_MII;
	case PHY_INTERFACE_MODE_RMII:
		return MII_PROT_RMII;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		return MII_PROT_RGMII;
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_XGMII:
	case PHY_INTERFACE_MODE_USXGMII:
		return MII_PROT_SERIAL;
	default:
		return -EINVAL;
	}
}

static int imx95_netcmix_init(struct udevice *dev)
{
	struct netc_blk_ctrl *priv = dev_get_priv(dev);
	ofnode child, gchild;
	phy_interface_t interface;
	int bus_devfn, mii_proto;
	u32 val;

	/* Default setting of MII protocol */
	val = MII_PROT(0, MII_PROT_RGMII) | MII_PROT(1, MII_PROT_RGMII) |
	      MII_PROT(2, MII_PROT_SERIAL);

	/* Update the link MII protocol through parsing phy-mode */
	dev_for_each_subnode(child, dev) {
		if (!ofnode_is_enabled(child))
			continue;

		ofnode_for_each_subnode(gchild, child) {
			if (!ofnode_is_enabled(gchild))
				continue;

			if (!ofnode_device_is_compatible(gchild, "pci1131,e101"))
				continue;

			bus_devfn = netc_of_pci_get_bus_devfn(gchild);
			if (bus_devfn < 0)
				return -EINVAL;

			if (bus_devfn == IMX95_ENETC2_BUS_DEVFN)
				continue;

			interface = ofnode_read_phy_mode(gchild);
			if (interface == -1)
				continue;

			mii_proto = netc_get_link_mii_protocol(interface);
			if (mii_proto < 0)
				return -EINVAL;

			switch (bus_devfn) {
			case IMX95_ENETC0_BUS_DEVFN:
				val &= ~CFG_LINK_MII_PORT_0;
				val |= FIELD_PREP(CFG_LINK_MII_PORT_0, mii_proto);
				break;
			case IMX95_ENETC1_BUS_DEVFN:
				val &= ~CFG_LINK_MII_PORT_1;
				val |= FIELD_PREP(CFG_LINK_MII_PORT_1, mii_proto);
				break;
			default:
				return -EINVAL;
			}
		}
	}

	/* Configure Link I/O variant */
	netc_reg_write(priv->netcmix, IMX95_CFG_LINK_IO_VAR,
		       IO_VAR(2, IO_VAR_16FF_16G_SERDES));
	/* Configure Link 2 PCS protocol */
	netc_reg_write(priv->netcmix, IMX95_CFG_LINK_PCS_PROT(2),
		       PCS_PROT_10G_SXGMII);
	netc_reg_write(priv->netcmix, IMX95_CFG_LINK_MII_PROT, val);

	return 0;
}

static int imx94_enetc_get_link_num(ofnode np)
{
	int bus_devfn;

	bus_devfn = netc_of_pci_get_bus_devfn(np);
	if (bus_devfn < 0)
		return -EINVAL;

	/* Parse ENETC link number */
	switch (bus_devfn) {
	case IMX94_ENETC0_BUS_DEVFN:
		return IMX94_ENETC0_LINK;
	case IMX94_ENETC1_BUS_DEVFN:
		return IMX94_ENETC1_LINK;
	case IMX94_ENETC2_BUS_DEVFN:
		return IMX94_ENETC2_LINK;
	default:
		return -EINVAL;
	}
}

static int imx94_link_config(struct netc_blk_ctrl *priv,
			     ofnode np, int link_id)
{
	phy_interface_t interface;
	int mii_proto;
	u32 val;

	interface = ofnode_read_phy_mode(np);
	if (interface == -1)
		return -EINVAL;

	mii_proto = netc_get_link_mii_protocol(interface);
	if (mii_proto < 0)
		return -EINVAL;

	val = mii_proto & NETC_LINK_CFG_MII_PROT;
	if (mii_proto == MII_PROT_SERIAL)
		val = u32_replace_bits(val, IO_VAR_16FF_16G_SERDES,
				       NETC_LINK_CFG_IO_VAR);

	netc_reg_write(priv->netcmix, IMX94_NETC_LINK_CFG(link_id), val);

	if (link_id == IMX94_ENETC0_LINK) {
		val = netc_reg_read(priv->netcmix, IMX94_EXT_PIN_CONTROL);
		val |= MAC2_MAC3_SEL;
		netc_reg_write(priv->netcmix, IMX94_EXT_PIN_CONTROL, val);
	}

	return 0;
}

static int imx94_enetc_link_config(struct netc_blk_ctrl *priv,
				   ofnode np, bool *enetc0_en)
{
	int link_id;

	link_id = imx94_enetc_get_link_num(np);
	if (link_id < 0)
		return -EINVAL;

	if (link_id == IMX94_ENETC0_LINK)
		*enetc0_en = true;

	return imx94_link_config(priv, np, link_id);
}

static int imx94_switch_link_config(struct netc_blk_ctrl *priv,
				    ofnode np, bool *swp2_en)
{
	ofnode ports, child;
	int port_id, err = 0;

	ports = ofnode_find_subnode(np, "ports");
	if (!ofnode_valid(ports))
		ports = ofnode_find_subnode(np, "ethernet-ports");
	if (!ofnode_valid(ports))
		return -ENODEV;

	ofnode_for_each_subnode(child, ports) {
		if (ofnode_read_u32(child, "reg", &port_id) < 0) {
			err = -ENODEV;
			goto end;
		}

		if (port_id == IMX94_SWITCH_CPU_PORT)
			continue;

		if (port_id == IMX94_SWITCH_PORT2)
			*swp2_en = true;

		err = imx94_link_config(priv, child, port_id);
		if (err)
			goto end;
	}

end:
	return err;
}

static int imx94_netcmix_init(struct udevice *dev)
{
	struct netc_blk_ctrl *priv = dev_get_priv(dev);
	ofnode child, gchild;
	bool enetc0_en = false, swp2_en = false;
	int err;

	dev_for_each_subnode(child, dev) {
		if (!ofnode_is_enabled(child))
			continue;

		ofnode_for_each_subnode(gchild, child) {
			if (!ofnode_is_enabled(gchild))
				continue;

			if (ofnode_device_is_compatible(gchild, "pci1131,e101")) {
				err = imx94_enetc_link_config(priv, gchild, &enetc0_en);
				if (err)
					return err;
			} else if (ofnode_device_is_compatible(gchild, "pci1131,eef2")) {
				err = imx94_switch_link_config(priv, gchild, &swp2_en);
				if (err)
					return err;
			}
		}
	}

	if (enetc0_en && swp2_en) {
		dev_err(dev, "Cannot enable swp2 and enetc0 at the same time\n");

		return -EINVAL;
	}

	return 0;
}

static bool netc_ierb_is_locked(struct netc_blk_ctrl *priv)
{
	return !!(netc_reg_read(priv->prb, PRB_NETCRR) & NETCRR_LOCK);
}

static int netc_lock_ierb(struct netc_blk_ctrl *priv)
{
	u32 val;

	netc_reg_write(priv->prb, PRB_NETCRR, NETCRR_LOCK);

	return readl_poll_timeout(priv->prb + PRB_NETCSR, val,
				  !(val & NETCSR_STATE), 2000);
}

static int netc_unlock_ierb_with_warm_reset(struct netc_blk_ctrl *priv)
{
	u32 val;

	netc_reg_write(priv->prb, PRB_NETCRR, 0);

	return readl_poll_timeout(priv->prb + PRB_NETCRR, val,
				  !(val & NETCRR_LOCK), 100000);
}

static int imx95_ierb_init(struct udevice *dev)
{
	struct netc_blk_ctrl *priv = dev_get_priv(dev);

	/* EMDIO : No MSI-X intterupt */
	netc_reg_write(priv->ierb, IERB_EMDIOFAUXR, 0);
	/* ENETC0 PF */
	netc_reg_write(priv->ierb, IERB_EFAUXR(0), 0);
	/* ENETC0 VF0 */
	netc_reg_write(priv->ierb, IERB_VFAUXR(0), 1);
	/* ENETC0 VF1 */
	netc_reg_write(priv->ierb, IERB_VFAUXR(1), 2);
	/* ENETC1 PF */
	netc_reg_write(priv->ierb, IERB_EFAUXR(1), 3);
	/* ENETC1 VF0 */
	netc_reg_write(priv->ierb, IERB_VFAUXR(2), 5);
	/* ENETC1 VF1 */
	netc_reg_write(priv->ierb, IERB_VFAUXR(3), 6);
	/* ENETC2 PF */
	netc_reg_write(priv->ierb, IERB_EFAUXR(2), 4);
	/* ENETC2 VF0 */
	netc_reg_write(priv->ierb, IERB_VFAUXR(4), 5);
	/* ENETC2 VF1 */
	netc_reg_write(priv->ierb, IERB_VFAUXR(5), 6);
	/* NETC TIMER */
	netc_reg_write(priv->ierb, IERB_T0FAUXR, 7);

	return 0;
}

static int imx94_enetc_get_enetc_offset(ofnode np)
{
	int bus_devfn;

	bus_devfn = netc_of_pci_get_bus_devfn(np);
	if (bus_devfn < 0)
		return -EINVAL;

	/* Parse ENETC offset */
	switch (bus_devfn) {
	case IMX94_ENETC0_BUS_DEVFN:
		return IMX94_ENETC0_OFFSET;
	case IMX94_ENETC1_BUS_DEVFN:
		return IMX94_ENETC1_OFFSET;
	case IMX94_ENETC2_BUS_DEVFN:
		return IMX94_ENETC2_OFFSET;
	default:
		return -EINVAL;
	}
}

static int imx94_enetc_get_timer_id(ofnode np)
{
	int bus_devfn;

	bus_devfn = netc_of_pci_get_bus_devfn(np);
	if (bus_devfn < 0)
		return -EINVAL;

	/* Parse ENETC PTP timer ID */
	switch (bus_devfn) {
	case IMX94_TIMER0_BUS_DEVFN:
		return IMX94_TIMER0_ID;
	case IMX94_TIMER1_BUS_DEVFN:
		return IMX94_TIMER1_ID;
	case IMX94_TIMER2_BUS_DEVFN:
		return IMX94_TIMER2_ID;
	default:
		return -EINVAL;
	}
}

static int imx94_enetc_update_tid(struct netc_blk_ctrl *priv, ofnode pf_np)
{
	ofnode timer_np;
	int offset, tid;

	offset = imx94_enetc_get_enetc_offset(pf_np);
	if (offset < 0) {
		printf("Find unknown PF node.\n");
		return offset;
	}

	timer_np = ofnode_parse_phandle(pf_np, "nxp,ptp-timer", 0);
	if (!ofnode_valid(timer_np)) {
		/*
		 * If nxp,ptp-timer is not set, the first timer of the bus
		 * where enetc is located will be used as the default timer.
		 */
		tid = IMX94_TIMER1_ID;
		goto update_reg;
	}

	tid = imx94_enetc_get_timer_id(timer_np);
	if (tid < 0) {
		printf("Incorrect bus/devfn of ptp-timer.\n");
		return tid;
	}

update_reg:
	netc_reg_write(priv->ierb, IERB_ETBCR(offset), tid);

	return 0;
}

static int imx94_ierb_init(struct udevice *dev)
{
	struct netc_blk_ctrl *priv = dev_get_priv(dev);
	ofnode bus_np, pf_np;
	int ret = 0;

	dev_for_each_subnode(bus_np, dev)
		ofnode_for_each_subnode(pf_np, bus_np)
			if (ofnode_device_is_compatible(pf_np, "pci1131,e101"))
				ret = imx94_enetc_update_tid(priv, pf_np);

	return ret;
}

static int netc_ierb_init(struct udevice *dev)
{
	struct netc_blk_ctrl *priv = dev_get_priv(dev);
	struct netc_devinfo *devinfo = (struct netc_devinfo *)dev_get_driver_data(dev);
	int err;

	if (netc_ierb_is_locked(priv)) {
		err = netc_unlock_ierb_with_warm_reset(priv);
		if (err) {
			dev_err(dev, "Unlock IERB failed.\n");
			return err;
		}
	}

	if (devinfo->ierb_init) {
		err = devinfo->ierb_init(dev);
		if (err)
			return err;
	}

	err = netc_lock_ierb(priv);
	if (err) {
		dev_err(dev, "Lock IERB failed.\n");
		return err;
	}

	return 0;
}

static void imx94_netc_xpcs_port_init(struct netc_blk_ctrl *priv, int port)
{
	u32 val;

	val = netc_reg_read(priv->netcmix, IMX94_MISC_SOC_CONTROL);
	if (port == IMX94_XPCS_PORT_1)
		val |= SEL_XPCS_1;
	else
		val &= ~SEL_XPCS_1;
	netc_reg_write(priv->netcmix, IMX94_MISC_SOC_CONTROL, val);
}

void netc_xpcs_port_init(int port)
{
	struct netc_blk_ctrl *priv = netc_bc;
	struct netc_devinfo *devinfo;

	if (!priv)
		return;

	devinfo = netc_di;
	if (devinfo->xpcs_port_init)
		devinfo->xpcs_port_init(priv, port);
}

static int netc_prb_check_error(struct netc_blk_ctrl *priv)
{
	if (netc_reg_read(priv->prb, PRB_NETCSR) & NETCSR_ERROR)
		return -1;

	return 0;
}

static const struct netc_devinfo imx95_devinfo = {
	.netcmix_init = imx95_netcmix_init,
	.ierb_init = imx95_ierb_init,
};

static const struct netc_devinfo imx94_devinfo = {
	.netcmix_init = imx94_netcmix_init,
	.ierb_init = imx94_ierb_init,
	.xpcs_port_init = imx94_netc_xpcs_port_init,
};

static const struct udevice_id netc_blk_ctrl_match[] = {
	{ .compatible = "nxp,imx95-netc-blk-ctrl", .data = (ulong)&imx95_devinfo },
	{ .compatible = "nxp,imx94-netc-blk-ctrl", .data = (ulong)&imx94_devinfo },
	{},
};

static int netc_blk_ctrl_probe(struct udevice *dev)
{
	struct netc_blk_ctrl *priv = dev_get_priv(dev);
	struct netc_devinfo *devinfo = (struct netc_devinfo *)dev_get_driver_data(dev);
	struct clk *ipg_clk;
	fdt_addr_t regs;
	int err;

	ipg_clk = devm_clk_get_optional(dev, "ipg");
	if (IS_ERR(ipg_clk)) {
		dev_err(dev, "Set ipg clock failed\n");
		return PTR_ERR(ipg_clk);
	}

	err = clk_prepare_enable(ipg_clk);
	if (err) {
		dev_err(dev, "Enable ipg clock failed\n");
		return err;
	}

	regs = dev_read_addr_name(dev, "ierb");
	if (regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "Missing IERB resource\n");
		return -EINVAL;
	}

	priv->ierb = (void __iomem *)regs;
	regs = dev_read_addr_name(dev, "prb");
	if (regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "Missing PRB resource\n");
		return -EINVAL;
	}

	priv->prb = (void __iomem *)regs;
	regs = dev_read_addr_name(dev, "netcmix");
	if (regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "Missing NETCMIX resource\n");
		return -EINVAL;
	}

	priv->netcmix = (void __iomem *)regs;

	if (devinfo->netcmix_init) {
		err = devinfo->netcmix_init(dev);
		if (err) {
			dev_err(dev, "Initializing NETCMIX failed\n");
			return err;
		}
	}

	err = netc_ierb_init(dev);
	if (err) {
		dev_err(dev, "Initializing IERB failed\n");
		return err;
	}

	if (netc_prb_check_error(priv) < 0)
		dev_warn(dev, "The current IERB configuration is invalid\n");

	netc_bc = priv;
	netc_di = devinfo;

	return 0;
}

U_BOOT_DRIVER(netc_blk_ctrl_drv) = {
	.name		= "netc_blk_ctrl",
	.id		= UCLASS_SIMPLE_BUS,
	.of_match	= netc_blk_ctrl_match,
	.probe		= netc_blk_ctrl_probe,
	.priv_auto	= sizeof(struct netc_blk_ctrl),
	.flags		= DM_FLAG_PRE_RELOC,
};
