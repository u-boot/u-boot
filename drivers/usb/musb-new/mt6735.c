// SPDX-License-Identifier: GPL-2.0
/*
 * Mediatek "glue layer"
 *
 * Copyright (C) 2019-2021 by Mediatek
 * Based on the AllWinner SUNXI "glue layer" code.
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of the Inventra Controller Driver for Linux.
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/pinctrl.h>
#include <linux/delay.h>
#include <linux/usb/musb.h>
#include <usb.h>
#include <power/regulator.h>
#include <generic-phy.h>
#include "linux-compat.h"
#include "musb_core.h"
#include "musb_uboot.h"
#include <linux/usb/gadget.h>
#define DBG_I(fmt, ...) \
	pr_info(fmt, ##__VA_ARGS__)

struct mtk_musb_config {
	struct musb_hdrc_config *config;
};

struct mtk_musb_glue {
	struct musb_host_data mdata;
	struct clk usbclk;
	struct mtk_musb_config *cfg;
	struct device dev;
	struct udevice *vusb33_supply;
	struct phy phys;
};

#define to_mtk_musb_glue(d)	container_of(d, struct mtk_musb_glue, dev)

/******************************************************************************
 * phy settings
 ******************************************************************************/
#define USB20_PHY_BASE			0x11210800
#define USBPHY_READ8(offset)	 \
	readb((void *)(USB20_PHY_BASE + (offset)))
#define USBPHY_WRITE8(offset, value)	\
	writeb(value, (void *)(USB20_PHY_BASE + (offset)))
#define USBPHY_SET8(offset, mask)	\
	USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)	\
	USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~(mask)))

/******************************************************************************
 * MUSB Glue code
 ******************************************************************************/
static irqreturn_t mtk_musb_interrupt(int irq, void *__hci)
{
	struct musb		*musb = __hci;
	irqreturn_t		retval = IRQ_NONE;

	/* read and flush interrupts */
	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB);
//	last_int_usb = musb->int_usb;
	if (musb->int_usb)
		musb_writeb(musb->mregs, MUSB_INTRUSB, musb->int_usb);
	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX);
	if (musb->int_tx)
		musb_writew(musb->mregs, MUSB_INTRTX, musb->int_tx);
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX);
	if (musb->int_rx)
		musb_writew(musb->mregs, MUSB_INTRRX, musb->int_rx);

	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval |= musb_interrupt(musb);

	return retval;
}

/* musb_core does not call enable / disable in a balanced manner <sigh> */
static bool enabled;

static int mtk_musb_enable(struct musb *musb)
{
	struct mtk_musb_glue *glue = to_mtk_musb_glue(musb->controller);

	DBG_I("%s():\n", __func__);

	musb_ep_select(musb->mregs, 0);
	musb_writeb(musb->mregs, MUSB_FADDR, 0);

	if (enabled)
		return 0;

	regulator_set_enable(glue->vusb33_supply, true);
	generic_phy_power_on(&glue->phys);
	enabled = true;

	return 0;
}

static void mtk_musb_disable(struct musb *musb)
{
	struct mtk_musb_glue *glue = to_mtk_musb_glue(musb->controller);
	int ret;

	DBG_I("%s():\n", __func__);

	if (!enabled)
		return;

	regulator_set_enable(glue->vusb33_supply, false);
	generic_phy_power_off(&glue->phys);

	enabled = false;
}

static int mtk_musb_init(struct musb *musb)
{
	struct mtk_musb_glue *glue = to_mtk_musb_glue(musb->controller);
	int ret;
	DBG_I("%s():\n", __func__);

	ret = clk_enable(&glue->usbclk);
	if (ret) {
		dev_err(musb->controller, "failed to enable usb clock\n");
		return ret;
	}
	musb->isr = mtk_musb_interrupt;

	return 0;
}

static int mtk_musb_exit(struct musb *musb)
{
	struct mtk_musb_glue *glue = to_mtk_musb_glue(musb->controller);

	clk_disable(&glue->usbclk);
	regulator_set_enable(glue->vusb33_supply, false);
	generic_phy_power_off(&glue->phys);
	generic_phy_exit(&glue->phys);
	return 0;
}

static const struct musb_platform_ops mtk_musb_ops = {
	.init		= mtk_musb_init,
	.exit		= mtk_musb_exit,
	.enable		= mtk_musb_enable,
	.disable	= mtk_musb_disable,
};

/* MTK OTG supports up to 7 endpoints */
#define MTK_MUSB_MAX_EP_NUM		8
#define MTK_MUSB_RAM_BITS		16

static struct musb_fifo_cfg mtk_musb_mode_cfg[] = {
	MUSB_EP_FIFO_SINGLE(1, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(1, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(2, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(2, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(3, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(3, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(4, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(4, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(5, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(5, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(6, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(6, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(7, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(7, FIFO_RX, 512),
};

static struct musb_hdrc_config musb_config = {
	.fifo_cfg       = mtk_musb_mode_cfg,
	.fifo_cfg_size  = ARRAY_SIZE(mtk_musb_mode_cfg),
	.multipoint	= true,
	.dyn_fifo	= true,
	.num_eps	= MTK_MUSB_MAX_EP_NUM,
	.ram_bits	= MTK_MUSB_RAM_BITS,
};

static int musb_usb_probe(struct udevice *dev)
{
	struct mtk_musb_glue *glue = dev_get_priv(dev);
	struct musb_host_data *host = &glue->mdata;
	struct musb_hdrc_platform_data pdata;
	void *base = dev_read_addr_ptr(dev);
	int ret;

	DBG_I("%s():\n", __func__);

#ifdef CONFIG_USB_MUSB_HOST
	struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
#endif

	if (!base)
		return -EINVAL;

	glue->cfg = (struct mtk_musb_config *)dev_get_driver_data(dev);
	if (!glue->cfg)
		return -EINVAL;

	ret = clk_get_by_name(dev, "usb", &glue->usbclk);
	if (ret) {
		dev_err(dev, "failed to get usb clock\n");
		return ret;
	}

	memset(&pdata, 0, sizeof(pdata));
	pdata.power = (u8)400;
	pdata.platform_ops = &mtk_musb_ops;
	pdata.config = glue->cfg->config;

	ret = device_get_supply_regulator(dev, "vusb33-supply",
					  &glue->vusb33_supply);
	if (ret)	/* optional, ignore error */
		dev_warn(dev, "can't get optional vusb33 %d\n", ret);

	ret = generic_phy_get_by_name(dev, "usb", &glue->phys);
	if (ret)
		return ret;

	ret = generic_phy_init(&glue->phys);
	if (ret) {
		pr_err("phy %d\n", ret);
		return ret;
	}

#ifdef CONFIG_USB_MUSB_HOST
	priv->desc_before_addr = true;

	pdata.mode = MUSB_HOST;
	host->host = musb_init_controller(&pdata, &glue->dev, base);
	if (!host->host)
		return -EIO;
	ret = musb_lowlevel_init(host);
	if (!ret)
		printf("MTK MUSB OTG (Host)\n");
#else
	pdata.mode = MUSB_PERIPHERAL;
	host->host = musb_register(&pdata, &glue->dev, base);
	if (!host->host)
		return -EIO;
	pinctrl_select_state(dev, "otg");

	printf("MTK MUSB OTG (Peripheral)\n");
#endif
	generic_phy_power_on(&glue->phys);

	return ret;
}

static int musb_usb_remove(struct udevice *dev)
{
	struct mtk_musb_glue *glue = dev_get_priv(dev);
	struct musb_host_data *host = &glue->mdata;

	musb_stop(host->host);
	free(host->host);
	host->host = NULL;

	return 0;
}

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct mtk_musb_glue *glue = dev_get_priv(dev);

	mtk_musb_interrupt(0, glue);

	return 0;
}

static const struct mtk_musb_config mt6735_cfg = {
	.config = &musb_config,
};

static const struct udevice_id mtk_musb_ids[] = {
	{ .compatible = "mediatek,mt6735-musb",
	  .data = (ulong)&mt6735_cfg },
	{ }
};

U_BOOT_DRIVER(mtk_musb) = {
	.name		= "mtk_musb",
#ifdef CONFIG_USB_MUSB_HOST
	.id		= UCLASS_USB,
#else
	.id		= UCLASS_USB_GADGET_GENERIC,
#endif
	.of_match	= mtk_musb_ids,
	.probe		= musb_usb_probe,
	.remove		= musb_usb_remove,
#ifdef CONFIG_USB_MUSB_HOST
	.ops		= &mtk_musb_ops,
#endif
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct mtk_musb_glue),
};
