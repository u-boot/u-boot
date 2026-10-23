// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Sanchayan Maity <sanchayan.maity@toradex.com>
 * Copyright (C) 2015 Toradex AG
 *
 * Based on ehci-mx6 driver
 */

#include <dm.h>
#include <log.h>
#include <usb.h>
#include <errno.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/regs-usbphy.h>
#include <linux/delay.h>
#include <usb/ehci-ci.h>
#include <fdtdec.h>
#include <linux/usb/otg.h>

#include "ehci.h"

#define USB_NC_REG_OFFSET				0x00000800

#define ANADIG_PLL_CTRL_EN_USB_CLKS		(1 << 6)

#define UCTRL_OVER_CUR_POL	(1 << 8) /* OTG Polarity of Overcurrent */
#define UCTRL_OVER_CUR_DIS	(1 << 7) /* Disable OTG Overcurrent Detection */

/* USBCMD */
#define UCMD_RUN_STOP		(1 << 0) /* controller run/stop */
#define UCMD_RESET			(1 << 1) /* controller reset */

static const unsigned phy_bases[] = {
	USB_PHY0_BASE_ADDR,
	USB_PHY1_BASE_ADDR,
};

static const unsigned nc_reg_bases[] = {
	USBC0_BASE_ADDR,
	USBC1_BASE_ADDR,
};

static void usb_internal_phy_clock_gate(int index)
{
	void __iomem *phy_reg;

	phy_reg = (void __iomem *)phy_bases[index];
	clrbits_le32(phy_reg + USBPHY_CTRL, USBPHY_CTRL_CLKGATE);
}

static void usb_power_config(int index)
{
	struct anadig_reg __iomem *anadig =
		(struct anadig_reg __iomem *)ANADIG_BASE_ADDR;
	void __iomem *pll_ctrl;

	switch (index) {
	case 0:
		pll_ctrl = &anadig->pll3_ctrl;
		clrbits_le32(pll_ctrl, ANADIG_PLL3_CTRL_BYPASS);
		setbits_le32(pll_ctrl, ANADIG_PLL3_CTRL_ENABLE
			 | ANADIG_PLL3_CTRL_POWERDOWN
			 | ANADIG_PLL_CTRL_EN_USB_CLKS);
		break;
	case 1:
		pll_ctrl = &anadig->pll7_ctrl;
		clrbits_le32(pll_ctrl, ANADIG_PLL7_CTRL_BYPASS);
		setbits_le32(pll_ctrl, ANADIG_PLL7_CTRL_ENABLE
			 | ANADIG_PLL7_CTRL_POWERDOWN
			 | ANADIG_PLL_CTRL_EN_USB_CLKS);
		break;
	default:
		return;
	}
}

static void usb_phy_enable(int index, struct usb_ehci *ehci)
{
	void __iomem *phy_reg;
	void __iomem *phy_ctrl;
	void __iomem *usb_cmd;

	phy_reg = (void __iomem *)phy_bases[index];
	phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);
	usb_cmd = (void __iomem *)&ehci->usbcmd;

	/* Stop then Reset */
	clrbits_le32(usb_cmd, UCMD_RUN_STOP);
	while (readl(usb_cmd) & UCMD_RUN_STOP)
		;

	setbits_le32(usb_cmd, UCMD_RESET);
	while (readl(usb_cmd) & UCMD_RESET)
		;

	/* Reset USBPHY module */
	setbits_le32(phy_ctrl, USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Remove CLKGATE and SFTRST */
	clrbits_le32(phy_ctrl, USBPHY_CTRL_CLKGATE | USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Power up the PHY */
	writel(0, phy_reg + USBPHY_PWD);

	/* Enable FS/LS device */
	setbits_le32(phy_ctrl, USBPHY_CTRL_ENUTMILEVEL2 |
		 USBPHY_CTRL_ENUTMILEVEL3);
}

static void usb_oc_config(int index)
{
	void __iomem *ctrl;

	ctrl = (void __iomem *)(nc_reg_bases[index] + USB_NC_REG_OFFSET);

	setbits_le32(ctrl, UCTRL_OVER_CUR_POL);
	setbits_le32(ctrl, UCTRL_OVER_CUR_DIS);
}

int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

int ehci_vf_common_init(struct usb_ehci *ehci, int index)
{
	int ret;

	/* Do board specific initialisation */
	ret = board_ehci_hcd_init(index);
	if (ret)
		return ret;

	usb_power_config(index);
	usb_oc_config(index);
	usb_internal_phy_clock_gate(index);
	usb_phy_enable(index, ehci);

	return 0;
}

struct ehci_vf_priv_data {
	struct ehci_ctrl ctrl;
	struct usb_ehci *ehci;
	struct gpio_desc cdet_gpio;
	enum usb_init_type init_type;
	enum usb_dr_mode dr_mode;
	u32 portnr;
};

static int vf_usb_of_to_plat(struct udevice *dev)
{
	struct ehci_vf_priv_data *priv = dev_get_priv(dev);
	fdt_addr_t phy_addr;
	ofnode phy_node;
	int i;

	phy_node = ofnode_parse_phandle(dev_ofnode(dev), "fsl,usbphy", 0);
	if (!ofnode_valid(phy_node))
		return -EINVAL;

	phy_addr = ofnode_get_addr(phy_node);
	for (i = 0; i < ARRAY_SIZE(phy_bases); i++) {
		if (phy_addr == phy_bases[i])
			break;
	}
	if (i == ARRAY_SIZE(phy_bases))
		return -EINVAL;

	priv->portnr = i;

	priv->ehci = dev_read_addr_ptr(dev);
	priv->dr_mode = usb_get_dr_mode(dev_ofnode(dev));
	/* Handle legacy DTs without dr_mode property, fall back to host */
	if (priv->dr_mode == USB_DR_MODE_UNKNOWN)
		priv->dr_mode = USB_DR_MODE_HOST;

	switch (priv->dr_mode) {
	case USB_DR_MODE_HOST:
		priv->init_type = USB_INIT_HOST;
		break;
	case USB_DR_MODE_PERIPHERAL:
		priv->init_type = USB_INIT_DEVICE;
		break;
	case USB_DR_MODE_OTG:
		/*
		 * We set init_type to device by default when OTG
		 * mode is requested. If a valid gpio is provided
		 * we will switch the init_type based on the state
		 * of the gpio pin.
		 */
		priv->init_type = USB_INIT_DEVICE;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int vf_init_after_reset(struct ehci_ctrl *dev)
{
	struct ehci_vf_priv_data *priv = dev->priv;
	enum usb_init_type type = priv->init_type;
	struct usb_ehci *ehci = priv->ehci;
	int ret;

	ret = ehci_vf_common_init(priv->ehci, priv->portnr);
	if (ret)
		return ret;

	if (type == USB_INIT_DEVICE)
		return 0;

	setbits_le32(&ehci->usbmode, CM_HOST);
	writel((PORT_PTS_UTMI | PORT_PTS_PTW), &ehci->portsc);
	setbits_le32(&ehci->portsc, USB_EN);

	mdelay(10);

	return 0;
}

static const struct ehci_ops vf_ehci_ops = {
	.init_after_reset = vf_init_after_reset
};

static int ehci_usb_probe(struct udevice *dev)
{
	struct usb_plat *plat = dev_get_plat(dev);
	struct ehci_vf_priv_data *priv = dev_get_priv(dev);
	struct usb_ehci *ehci = priv->ehci;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int ret;

	if (priv->dr_mode == USB_DR_MODE_OTG) {
		ret = gpio_request_by_name(dev, "fsl,cdet-gpio", 0,
					   &priv->cdet_gpio, GPIOD_IS_IN);
		if (ret && ret != -ENOENT)
			return ret;
		if (!ret) {
			ret = dm_gpio_get_value(&priv->cdet_gpio);
			if (ret < 0)
				goto err_gpio;
			priv->init_type = ret ? USB_INIT_DEVICE : USB_INIT_HOST;
		}
	}

	ret = ehci_vf_common_init(ehci, priv->portnr);
	if (ret)
		goto err_gpio;

	if (priv->init_type != plat->init_type) {
		ret = -ENODEV;
		goto err_gpio;
	}

	if (priv->init_type == USB_INIT_HOST) {
		setbits_le32(&ehci->usbmode, CM_HOST);
		writel((PORT_PTS_UTMI | PORT_PTS_PTW), &ehci->portsc);
		setbits_le32(&ehci->portsc, USB_EN);
	}

	mdelay(10);

	hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((uint32_t)hccr +
				HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	ret = ehci_register(dev, hccr, hcor, &vf_ehci_ops, 0, priv->init_type);
	if (ret)
		goto err_gpio;

	return 0;

err_gpio:
	if (dm_gpio_is_valid(&priv->cdet_gpio))
		dm_gpio_free(dev, &priv->cdet_gpio);

	return ret;
}

static const struct udevice_id vf_usb_ids[] = {
	{ .compatible = "fsl,vf610-usb" },
	{ }
};

static int ehci_usb_remove(struct udevice *dev)
{
	struct ehci_vf_priv_data *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->cdet_gpio))
		dm_gpio_free(dev, &priv->cdet_gpio);

	return ehci_deregister(dev);
}

U_BOOT_DRIVER(ehci_vf) = {
	.name = "ehci_vf",
	.id = UCLASS_USB,
	.of_match = vf_usb_ids,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops = &ehci_usb_ops,
	.of_to_plat = vf_usb_of_to_plat,
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct ehci_vf_priv_data),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
