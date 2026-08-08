// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 */

#include <dm/lists.h>
#include <linux/iopoll.h>

#include "mtu3.h"
#include "mtu3_dr.h"

void ssusb_set_force_mode(struct ssusb_mtk *ssusb,
			  enum mtu3_dr_force_mode mode)
{
	u32 value;

	value = mtu3_readl(ssusb->ippc_base, SSUSB_U2_CTRL(0));
	switch (mode) {
	case MTU3_DR_FORCE_DEVICE:
		value |= SSUSB_U2_PORT_FORCE_IDDIG | SSUSB_U2_PORT_RG_IDDIG;
		break;
	case MTU3_DR_FORCE_HOST:
		value |= SSUSB_U2_PORT_FORCE_IDDIG;
		value &= ~SSUSB_U2_PORT_RG_IDDIG;
		break;
	case MTU3_DR_FORCE_NONE:
		value &= ~(SSUSB_U2_PORT_FORCE_IDDIG | SSUSB_U2_PORT_RG_IDDIG);
		break;
	default:
		return;
	}
	mtu3_writel(ssusb->ippc_base, SSUSB_U2_CTRL(0), value);
}

/* u2-port0 should be powered on and enabled; */
int ssusb_check_clocks(struct ssusb_mtk *ssusb, u32 ex_clks)
{
	void __iomem *ibase = ssusb->ippc_base;
	u32 value, check_val;
	int ret;

	check_val = ex_clks | SSUSB_SYS125_RST_B_STS | SSUSB_SYSPLL_STABLE |
			SSUSB_REF_RST_B_STS;

	ret = readl_poll_timeout(ibase + U3D_SSUSB_IP_PW_STS1, value,
				 ((value & check_val) == check_val), 10000);
	if (ret) {
		dev_err(ssusb->dev, "clks of sts1 are not stable!\n");
		return ret;
	}

	ret = readl_poll_timeout(ibase + U3D_SSUSB_IP_PW_STS2, value,
				 (value & SSUSB_U2_MAC_SYS_RST_B_STS), 10000);
	if (ret) {
		dev_err(ssusb->dev, "mac2 clock is not stable\n");
		return ret;
	}

	return 0;
}

int ssusb_phy_setup(struct ssusb_mtk *ssusb)
{
	struct udevice *dev = ssusb->dev;
	struct phy_bulk *phys = &ssusb->phys;
	int ret;

	ret = generic_phy_get_bulk(dev, phys);
	if (ret)
		return ret;

	ret = generic_phy_init_bulk(phys);
	if (ret)
		return ret;

	ret = generic_phy_power_on_bulk(phys);
	if (ret)
		generic_phy_exit_bulk(phys);

	return ret;
}

void ssusb_phy_shutdown(struct ssusb_mtk *ssusb)
{
	generic_phy_power_off_bulk(&ssusb->phys);
	generic_phy_exit_bulk(&ssusb->phys);
}

static int ssusb_rscs_init(struct ssusb_mtk *ssusb)
{
	int ret = 0;

	ret = regulator_set_enable(ssusb->vusb33_supply, true);
	if (ret < 0 && ret != -ENOSYS) {
		dev_err(ssusb->dev, "failed to enable vusb33\n");
		goto vusb33_err;
	}

	ret = clk_enable_bulk(&ssusb->clks);
	if (ret)
		goto clks_err;

	ret = ssusb_phy_setup(ssusb);
	if (ret) {
		dev_err(ssusb->dev, "failed to setup phy\n");
		goto phy_err;
	}

	return 0;

phy_err:
	clk_disable_bulk(&ssusb->clks);
clks_err:
	regulator_set_enable(ssusb->vusb33_supply, false);
vusb33_err:
	return ret;
}

static void ssusb_rscs_exit(struct ssusb_mtk *ssusb)
{
	clk_disable_bulk(&ssusb->clks);
	regulator_set_enable(ssusb->vusb33_supply, false);
	ssusb_phy_shutdown(ssusb);
}

static void ssusb_ip_sw_reset(struct ssusb_mtk *ssusb)
{
	/* reset whole ip (xhci & u3d) */
	mtu3_setbits(ssusb->ippc_base, U3D_SSUSB_IP_PW_CTRL0, SSUSB_IP_SW_RST);
	udelay(1);
	mtu3_clrbits(ssusb->ippc_base, U3D_SSUSB_IP_PW_CTRL0, SSUSB_IP_SW_RST);
}

static int get_ssusb_rscs(struct udevice *dev, struct ssusb_mtk *ssusb)
{
	int ret;

	ret = device_get_supply_regulator(dev, "vusb33-supply",
					  &ssusb->vusb33_supply);
	if (ret)	/* optional, ignore error */
		dev_warn(dev, "can't get optional vusb33 %d\n", ret);

	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &ssusb->vbus_supply);
	if (ret)	/* optional, ignore error */
		dev_warn(dev, "can't get optional vbus regulator %d!\n", ret);

	ret = clk_get_bulk(dev, &ssusb->clks);
	if (ret) {
		dev_err(dev, "failed to get clocks %d!\n", ret);
		return ret;
	}

	ssusb->ippc_base = dev_remap_addr_name(dev, "ippc");
	if (!ssusb->ippc_base) {
		dev_err(dev, "error mapping memory for ippc\n");
		return -ENODEV;
	}

	ssusb->mac_base = dev_remap_addr_name(dev, "mac");
	if (!ssusb->mac_base) {
		dev_err(dev, "error mapping memory for mac\n");
		return -ENODEV;
	}

	dev_info(dev, "ippc: 0x%p, mac: 0x%p\n",
		 ssusb->ippc_base, ssusb->mac_base);

	return 0;
}

static int mtu3_probe(struct udevice *dev)
{
	struct ssusb_mtk *ssusb = dev_get_priv(dev);
	int ret = -ENOMEM;

	ssusb->dev = dev;

	ret = get_ssusb_rscs(dev, ssusb);
	if (ret)
		return ret;

	ret = ssusb_rscs_init(ssusb);
	if (ret)
		return ret;

	ssusb_ip_sw_reset(ssusb);

	return 0;
}

static int mtu3_remove(struct udevice *dev)
{
	struct ssusb_mtk *ssusb = dev_to_ssusb(dev);

	ssusb_rscs_exit(ssusb);
	return 0;
}

#if CONFIG_IS_ENABLED(DM_USB_GADGET)
static int mtu3_gadget_probe(struct udevice *dev)
{
	struct ssusb_mtk *ssusb = dev_to_ssusb(dev->parent);
	struct mtu3 *mtu = dev_get_priv(dev);

	mtu->dev = dev;
	ssusb->u3d = mtu;
	return ssusb_gadget_init(ssusb);
}

static int mtu3_gadget_remove(struct udevice *dev)
{
	struct mtu3 *mtu = dev_get_priv(dev);

	ssusb_gadget_exit(mtu->ssusb);
	return 0;
}

static int mtu3_gadget_handle_interrupts(struct udevice *dev)
{
	struct mtu3 *mtu = dev_get_priv(dev);

	mtu3_irq(0, mtu);

	return 0;
}

static const struct usb_gadget_generic_ops mtu3_gadget_ops = {
	.handle_interrupts	= mtu3_gadget_handle_interrupts,
};

U_BOOT_DRIVER(mtu3_peripheral) = {
	.name = "mtu3-peripheral",
	.id = UCLASS_USB_GADGET_GENERIC,
	.ops = &mtu3_gadget_ops,
	.probe = mtu3_gadget_probe,
	.remove = mtu3_gadget_remove,
	.priv_auto	= sizeof(struct mtu3),
};

static int mtu3_bind_gadget(struct udevice *parent)
{
	struct udevice *dev;
	int ret;

	/* Node-less device: name it after the controller for diagnostics. */
	ret = device_bind_driver(parent, "mtu3-peripheral",
				 ofnode_get_name(dev_ofnode(parent)), &dev);
	if (ret)
		dev_err(parent, "failed to bind peripheral mode: %d\n", ret);

	return ret;
}
#else
static int mtu3_bind_gadget(struct udevice *parent)
{
	return -ENODEV;
}
#endif

#if defined(CONFIG_SPL_USB_HOST) || \
	(!defined(CONFIG_XPL_BUILD) && defined(CONFIG_USB_HOST))
static int mtu3_host_rscs_init(struct mtu3_host *u3h)
{
	struct ssusb_mtk *ssusb = u3h->ssusb;
	int ret;

	ret = clk_get_bulk(u3h->dev, &u3h->clks);
	if (ret) {
		dev_err(u3h->dev, "failed to get clocks %d!\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(u3h->dev, "vusb33-supply",
					  &u3h->vusb33_supply);
	if (ret)
		dev_dbg(u3h->dev, "can't get optional vusb33 %d\n", ret);

	ret = device_get_supply_regulator(u3h->dev, "vbus-supply",
					  &u3h->vbus_supply);
	if (ret) {
		dev_dbg(u3h->dev, "can't get optional vbus regulator %d\n",
			ret);
		u3h->vbus_supply = ssusb->vbus_supply;
	}

	ret = regulator_set_enable_if_allowed(u3h->vusb33_supply, true);
	if (ret && ret != -ENOSYS) {
		dev_err(u3h->dev, "failed to enable vusb33 %d!\n", ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(u3h->vbus_supply, true);
	if (ret && ret != -ENOSYS) {
		dev_err(u3h->dev, "failed to enable vbus %d!\n", ret);
		goto vbus_err;
	}

	ret = clk_enable_bulk(&u3h->clks);
	if (ret)
		goto clks_err;

	return 0;

clks_err:
	regulator_set_enable_if_allowed(u3h->vbus_supply, false);
vbus_err:
	regulator_set_enable_if_allowed(u3h->vusb33_supply, false);
	return ret;
}

static void mtu3_host_rscs_exit(struct mtu3_host *u3h)
{
	clk_disable_bulk(&u3h->clks);
	regulator_set_enable_if_allowed(u3h->vbus_supply, false);
	regulator_set_enable_if_allowed(u3h->vusb33_supply, false);
}

static int mtu3_host_probe(struct udevice *dev)
{
	struct ssusb_mtk *ssusb = dev_to_ssusb(dev->parent);
	struct mtu3_host *u3h = dev_get_priv(dev);
	struct xhci_hcor *hcor;
	int rc;

	u3h->dev = dev;
	ssusb->u3h = u3h;
	u3h->ssusb = ssusb;
	u3h->hcd = dev_remap_addr_name(dev, "mac");
	if (!u3h->hcd) {
		dev_err(dev, "error mapping memory for xHCI mac\n");
		return -ENODEV;
	}

	rc = mtu3_host_rscs_init(u3h);
	if (rc)
		return rc;

	rc = ssusb_host_init(ssusb);
	if (rc)
		goto rscs_err;

	u3h->ctrl.quirks = XHCI_MTK_HOST;
	hcor = (struct xhci_hcor *)((uintptr_t)u3h->hcd +
			HC_LENGTH(xhci_readl(&u3h->hcd->cr_capbase)));

	rc = xhci_register(dev, u3h->hcd, hcor);
	if (rc)
		goto host_err;

	return 0;

host_err:
	ssusb_host_exit(ssusb);
rscs_err:
	mtu3_host_rscs_exit(u3h);
	return rc;
}

static int mtu3_host_remove(struct udevice *dev)
{
	struct mtu3_host *u3h = dev_get_priv(dev);

	xhci_deregister(dev);
	ssusb_host_exit(u3h->ssusb);
	mtu3_host_rscs_exit(u3h);
	return 0;
}

U_BOOT_DRIVER(mtu3_host) = {
	.name = "mtu3-host",
	.id = UCLASS_USB,
	.probe = mtu3_host_probe,
	.remove = mtu3_host_remove,
	.priv_auto	= sizeof(struct mtu3_host),
	.ops = &xhci_usb_ops,
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};

static int mtu3_bind_host(struct udevice *parent)
{
	struct udevice *dev;
	const char *name;
	ofnode node;
	int ret;

	ofnode_for_each_subnode(node, dev_ofnode(parent)) {
		if (ofnode_is_enabled(node) &&
		    ofnode_device_is_compatible(node, "mediatek,mtk-xhci"))
			break;
	}
	if (!ofnode_valid(node)) {
		dev_err(parent, "failed to find enabled xHCI child node\n");
		return -ENODEV;
	}

	name = ofnode_get_name(node);
	ret = device_bind_driver_to_node(parent, "mtu3-host", name, node,
					 &dev);
	if (ret)
		dev_err(parent, "failed to bind host mode: %d\n", ret);

	return ret;
}
#else
static int mtu3_bind_host(struct udevice *parent)
{
	return -ENODEV;
}
#endif

static int mtu3_glue_bind(struct udevice *parent)
{
	enum usb_dr_mode dr_mode;
	ofnode node;

	/* Reject the old layout before misinterpreting its register offsets. */
	ofnode_for_each_subnode(node, dev_ofnode(parent)) {
		if (ofnode_device_is_compatible(node, "mediatek,ssusb")) {
			dev_err(parent,
				"legacy mediatek,ssusb child is unsupported\n");
			/* Only -ENODEV keeps the failure contained to USB. */
			return -ENODEV;
		}
	}

	dr_mode = usb_get_dr_mode(dev_ofnode(parent));
	if (dr_mode == USB_DR_MODE_UNKNOWN) {
		/* Absent defaults to otg, an unparsable value is an error. */
		if (dev_read_string(parent, "dr_mode")) {
			dev_err(parent, "invalid dr_mode\n");
			return -ENODEV;
		}

		dr_mode = USB_DR_MODE_OTG;
	}

	switch (dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
		if (IS_ENABLED(CONFIG_USB_MTU3_GADGET))
			return mtu3_bind_gadget(parent);
		break;
	case USB_DR_MODE_HOST:
		if (IS_ENABLED(CONFIG_USB_MTU3_HOST))
			return mtu3_bind_host(parent);
		break;
	case USB_DR_MODE_OTG:
		if (IS_ENABLED(CONFIG_USB_MTU3_GADGET))
			return mtu3_bind_gadget(parent);
		if (IS_ENABLED(CONFIG_USB_MTU3_HOST))
			return mtu3_bind_host(parent);
		break;
	default:
		break;
	}

	dev_err(parent, "dr_mode %d is unsupported by this build\n", dr_mode);
	return -ENODEV;
}

static const struct udevice_id mtu3_of_match[] = {
	{.compatible = "mediatek,mtu3",},
	{},
};

U_BOOT_DRIVER(mtu3) = {
	.name = "mtu3",
	.id = UCLASS_NOP,
	.of_match = mtu3_of_match,
	.bind = mtu3_glue_bind,
	.probe = mtu3_probe,
	.remove = mtu3_remove,
	.priv_auto	= sizeof(struct ssusb_mtk),
};
