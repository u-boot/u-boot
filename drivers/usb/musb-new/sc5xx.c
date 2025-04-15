// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * ADI SC5XX MUSB "glue layer"
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Loosely ported from Linux driver:
 * Author: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 *
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/usb/musb.h>
#include "linux-compat.h"
#include "musb_core.h"
#include "musb_uboot.h"

#define MUSB_SOFTRST		0x7f
#define  MUSB_SOFTRST_NRST	BIT(0)
#define  MUSB_SOFTRST_NRSTX	BIT(1)

#define REG_USB_VBUS_CTL	0x380
#define REG_USB_ID_CTL		0x382
#define REG_USB_PHY_CTL		0x394
#define REG_USB_PLL_OSC		0x398
#define REG_USB_UTMI_CTL	0x39c

/* controller data */
struct sc5xx_musb_data {
	struct musb_host_data mdata;
	struct device dev;
};

#define to_sc5xx_musb_data(d)	\
	container_of(d, struct sc5xx_musb_data, dev)

static void sc5xx_musb_disable(struct musb *musb)
{
	/* no way to shut the controller */
}

static int sc5xx_musb_enable(struct musb *musb)
{
	/* soft reset by NRSTx */
	musb_writeb(musb->mregs, MUSB_SOFTRST, MUSB_SOFTRST_NRSTX);
	/* set mode */
	musb_platform_set_mode(musb, musb->board_mode);

	return 0;
}

static irqreturn_t sc5xx_interrupt(int irq, void *hci)
{
	struct musb  *musb = hci;
	irqreturn_t ret = IRQ_NONE;
	u8 devctl;

	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB);
	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX);
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX);

	if (musb->int_usb & MUSB_INTR_VBUSERROR) {
		musb->int_usb &= ~MUSB_INTR_VBUSERROR;
		devctl = musb_readw(musb->mregs, MUSB_DEVCTL);
		devctl |= MUSB_DEVCTL_SESSION;
		musb_writeb(musb->mregs, MUSB_DEVCTL, devctl);
	}
	if (musb->int_usb || musb->int_tx || musb->int_rx) {
		musb_writeb(musb->mregs, MUSB_INTRUSB, musb->int_usb);
		musb_writew(musb->mregs, MUSB_INTRTX, musb->int_tx);
		musb_writew(musb->mregs, MUSB_INTRRX, musb->int_rx);
		ret = musb_interrupt(musb);
	}

	if (musb->int_usb & MUSB_INTR_DISCONNECT && is_host_active(musb))
		musb_writeb(musb->mregs, REG_USB_VBUS_CTL, 0x0);

	return ret;
}

static int sc5xx_musb_set_mode(struct musb *musb, u8 mode)
{
	struct device *dev = musb->controller;
	struct sc5xx_musb_data *pdata = to_sc5xx_musb_data(dev);

	switch (mode) {
	case MUSB_HOST:
		musb_writeb(musb->mregs, REG_USB_ID_CTL, 0x1);
		break;
	case MUSB_PERIPHERAL:
		musb_writeb(musb->mregs, REG_USB_ID_CTL, 0x3);
		break;
	case MUSB_OTG:
		musb_writeb(musb->mregs, REG_USB_ID_CTL, 0x0);
		break;
	default:
		dev_err(dev, "unsupported mode %d\n", mode);
		return -EINVAL;
	}

	return 0;
}

static int sc5xx_musb_init(struct musb *musb)
{
	struct sc5xx_musb_data *pdata = to_sc5xx_musb_data(musb->controller);

	musb->isr = sc5xx_interrupt;

	musb_writel(musb->mregs, REG_USB_PLL_OSC, 20 << 1);
	musb_writeb(musb->mregs, REG_USB_VBUS_CTL, 0x0);
	musb_writeb(musb->mregs, REG_USB_PHY_CTL, 0x80);
	musb_writel(musb->mregs, REG_USB_UTMI_CTL,
		    0x40 | musb_readl(musb->mregs, REG_USB_UTMI_CTL));

	return 0;
}

const struct musb_platform_ops sc5xx_musb_ops = {
	.init		= sc5xx_musb_init,
	.set_mode	= sc5xx_musb_set_mode,
	.disable	= sc5xx_musb_disable,
	.enable		= sc5xx_musb_enable,
};

static struct musb_hdrc_config sc5xx_musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

/* has one MUSB controller which can be host or gadget */
static struct musb_hdrc_platform_data sc5xx_musb_plat = {
	.mode           = MUSB_HOST,
	.config         = &sc5xx_musb_config,
	.power          = 100,
	.platform_ops	= &sc5xx_musb_ops,
};

static int musb_usb_probe(struct udevice *dev)
{
	struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
	struct sc5xx_musb_data *pdata = dev_get_priv(dev);
	struct musb_host_data *mdata = &pdata->mdata;
	void __iomem *mregs;
	int ret;

	priv->desc_before_addr = true;

	mregs = dev_remap_addr(dev);
	if (!mregs)
		return -EINVAL;

	/* init controller */
	if (IS_ENABLED(CONFIG_USB_MUSB_HOST)) {
		mdata->host = musb_init_controller(&sc5xx_musb_plat,
						   &pdata->dev, mregs);
		if (!mdata->host)
			return -EIO;

		ret = musb_lowlevel_init(mdata);
	} else {
		sc5xx_musb_plat.mode = MUSB_PERIPHERAL;
		mdata->host = musb_register(&sc5xx_musb_plat, &pdata->dev, mregs);
		if (!mdata->host)
			return -EIO;
	}
	return ret;
}

static int musb_usb_remove(struct udevice *dev)
{
	struct sc5xx_musb_data *pdata = dev_get_priv(dev);

	musb_stop(pdata->mdata.host);

	return 0;
}

static const struct udevice_id sc5xx_musb_ids[] = {
	{ .compatible = "adi,sc5xx-musb" },
	{ }
};

U_BOOT_DRIVER(usb_musb) = {
	.name		= "sc5xx-musb",
	.id		= UCLASS_USB,
	.of_match	= sc5xx_musb_ids,
	.probe		= musb_usb_probe,
	.remove		= musb_usb_remove,
#ifdef CONFIG_USB_MUSB_HOST
	.ops		= &musb_usb_ops,
#endif
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct sc5xx_musb_data),
};
