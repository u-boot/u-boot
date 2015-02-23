/**
 * dwc3-omap.c - OMAP Specific Glue layer
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Taken from Linux Kernel v3.19-rc1 (drivers/usb/dwc3/dwc3-omap.c) and ported
 * to uboot.
 *
 * commit 7ee2566ff5 : usb: dwc3: dwc3-omap: get rid of ->prepare()/->complete()
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/usb/dwc3-omap.h>
#include <linux/ioport.h>

#include <linux/usb/otg.h>
#include <linux/compat.h>

#include "linux-compat.h"

/*
 * All these registers belong to OMAP's Wrapper around the
 * DesignWare USB3 Core.
 */

#define USBOTGSS_REVISION			0x0000
#define USBOTGSS_SYSCONFIG			0x0010
#define USBOTGSS_IRQ_EOI			0x0020
#define USBOTGSS_EOI_OFFSET			0x0008
#define USBOTGSS_IRQSTATUS_RAW_0		0x0024
#define USBOTGSS_IRQSTATUS_0			0x0028
#define USBOTGSS_IRQENABLE_SET_0		0x002c
#define USBOTGSS_IRQENABLE_CLR_0		0x0030
#define USBOTGSS_IRQ0_OFFSET			0x0004
#define USBOTGSS_IRQSTATUS_RAW_1		0x0030
#define USBOTGSS_IRQSTATUS_1			0x0034
#define USBOTGSS_IRQENABLE_SET_1		0x0038
#define USBOTGSS_IRQENABLE_CLR_1		0x003c
#define USBOTGSS_IRQSTATUS_RAW_2		0x0040
#define USBOTGSS_IRQSTATUS_2			0x0044
#define USBOTGSS_IRQENABLE_SET_2		0x0048
#define USBOTGSS_IRQENABLE_CLR_2		0x004c
#define USBOTGSS_IRQSTATUS_RAW_3		0x0050
#define USBOTGSS_IRQSTATUS_3			0x0054
#define USBOTGSS_IRQENABLE_SET_3		0x0058
#define USBOTGSS_IRQENABLE_CLR_3		0x005c
#define USBOTGSS_IRQSTATUS_EOI_MISC		0x0030
#define USBOTGSS_IRQSTATUS_RAW_MISC		0x0034
#define USBOTGSS_IRQSTATUS_MISC			0x0038
#define USBOTGSS_IRQENABLE_SET_MISC		0x003c
#define USBOTGSS_IRQENABLE_CLR_MISC		0x0040
#define USBOTGSS_IRQMISC_OFFSET			0x03fc
#define USBOTGSS_UTMI_OTG_CTRL			0x0080
#define USBOTGSS_UTMI_OTG_STATUS		0x0084
#define USBOTGSS_UTMI_OTG_OFFSET		0x0480
#define USBOTGSS_TXFIFO_DEPTH			0x0508
#define USBOTGSS_RXFIFO_DEPTH			0x050c
#define USBOTGSS_MMRAM_OFFSET			0x0100
#define USBOTGSS_FLADJ				0x0104
#define USBOTGSS_DEBUG_CFG			0x0108
#define USBOTGSS_DEBUG_DATA			0x010c
#define USBOTGSS_DEV_EBC_EN			0x0110
#define USBOTGSS_DEBUG_OFFSET			0x0600

/* SYSCONFIG REGISTER */
#define USBOTGSS_SYSCONFIG_DMADISABLE		(1 << 16)

/* IRQ_EOI REGISTER */
#define USBOTGSS_IRQ_EOI_LINE_NUMBER		(1 << 0)

/* IRQS0 BITS */
#define USBOTGSS_IRQO_COREIRQ_ST		(1 << 0)

/* IRQMISC BITS */
#define USBOTGSS_IRQMISC_DMADISABLECLR		(1 << 17)
#define USBOTGSS_IRQMISC_OEVT			(1 << 16)
#define USBOTGSS_IRQMISC_DRVVBUS_RISE		(1 << 13)
#define USBOTGSS_IRQMISC_CHRGVBUS_RISE		(1 << 12)
#define USBOTGSS_IRQMISC_DISCHRGVBUS_RISE	(1 << 11)
#define USBOTGSS_IRQMISC_IDPULLUP_RISE		(1 << 8)
#define USBOTGSS_IRQMISC_DRVVBUS_FALL		(1 << 5)
#define USBOTGSS_IRQMISC_CHRGVBUS_FALL		(1 << 4)
#define USBOTGSS_IRQMISC_DISCHRGVBUS_FALL		(1 << 3)
#define USBOTGSS_IRQMISC_IDPULLUP_FALL		(1 << 0)

/* UTMI_OTG_CTRL REGISTER */
#define USBOTGSS_UTMI_OTG_CTRL_DRVVBUS		(1 << 5)
#define USBOTGSS_UTMI_OTG_CTRL_CHRGVBUS		(1 << 4)
#define USBOTGSS_UTMI_OTG_CTRL_DISCHRGVBUS	(1 << 3)
#define USBOTGSS_UTMI_OTG_CTRL_IDPULLUP		(1 << 0)

/* UTMI_OTG_STATUS REGISTER */
#define USBOTGSS_UTMI_OTG_STATUS_SW_MODE	(1 << 31)
#define USBOTGSS_UTMI_OTG_STATUS_POWERPRESENT	(1 << 9)
#define USBOTGSS_UTMI_OTG_STATUS_TXBITSTUFFENABLE (1 << 8)
#define USBOTGSS_UTMI_OTG_STATUS_IDDIG		(1 << 4)
#define USBOTGSS_UTMI_OTG_STATUS_SESSEND	(1 << 3)
#define USBOTGSS_UTMI_OTG_STATUS_SESSVALID	(1 << 2)
#define USBOTGSS_UTMI_OTG_STATUS_VBUSVALID	(1 << 1)

struct dwc3_omap {
	struct device		*dev;

	void __iomem		*base;

	u32			utmi_otg_status;
	u32			utmi_otg_offset;
	u32			irqmisc_offset;
	u32			irq_eoi_offset;
	u32			debug_offset;
	u32			irq0_offset;

	u32			dma_status:1;
};

static inline u32 dwc3_omap_readl(void __iomem *base, u32 offset)
{
	return readl(base + offset);
}

static inline void dwc3_omap_writel(void __iomem *base, u32 offset, u32 value)
{
	writel(value, base + offset);
}

static u32 dwc3_omap_read_utmi_status(struct dwc3_omap *omap)
{
	return dwc3_omap_readl(omap->base, USBOTGSS_UTMI_OTG_STATUS +
							omap->utmi_otg_offset);
}

static void dwc3_omap_write_utmi_status(struct dwc3_omap *omap, u32 value)
{
	dwc3_omap_writel(omap->base, USBOTGSS_UTMI_OTG_STATUS +
					omap->utmi_otg_offset, value);

}

static u32 dwc3_omap_read_irq0_status(struct dwc3_omap *omap)
{
	return dwc3_omap_readl(omap->base, USBOTGSS_IRQSTATUS_0 -
						omap->irq0_offset);
}

static void dwc3_omap_write_irq0_status(struct dwc3_omap *omap, u32 value)
{
	dwc3_omap_writel(omap->base, USBOTGSS_IRQSTATUS_0 -
						omap->irq0_offset, value);

}

static u32 dwc3_omap_read_irqmisc_status(struct dwc3_omap *omap)
{
	return dwc3_omap_readl(omap->base, USBOTGSS_IRQSTATUS_MISC +
						omap->irqmisc_offset);
}

static void dwc3_omap_write_irqmisc_status(struct dwc3_omap *omap, u32 value)
{
	dwc3_omap_writel(omap->base, USBOTGSS_IRQSTATUS_MISC +
					omap->irqmisc_offset, value);

}

static void dwc3_omap_write_irqmisc_set(struct dwc3_omap *omap, u32 value)
{
	dwc3_omap_writel(omap->base, USBOTGSS_IRQENABLE_SET_MISC +
						omap->irqmisc_offset, value);

}

static void dwc3_omap_write_irq0_set(struct dwc3_omap *omap, u32 value)
{
	dwc3_omap_writel(omap->base, USBOTGSS_IRQENABLE_SET_0 -
						omap->irq0_offset, value);
}

static void dwc3_omap_set_mailbox(struct dwc3_omap *omap,
	enum omap_dwc3_vbus_id_status status)
{
	u32	val;

	switch (status) {
	case OMAP_DWC3_ID_GROUND:
		dev_dbg(omap->dev, "ID GND\n");

		val = dwc3_omap_read_utmi_status(omap);
		val &= ~(USBOTGSS_UTMI_OTG_STATUS_IDDIG
				| USBOTGSS_UTMI_OTG_STATUS_VBUSVALID
				| USBOTGSS_UTMI_OTG_STATUS_SESSEND);
		val |= USBOTGSS_UTMI_OTG_STATUS_SESSVALID
				| USBOTGSS_UTMI_OTG_STATUS_POWERPRESENT;
		dwc3_omap_write_utmi_status(omap, val);
		break;

	case OMAP_DWC3_VBUS_VALID:
		dev_dbg(omap->dev, "VBUS Connect\n");

		val = dwc3_omap_read_utmi_status(omap);
		val &= ~USBOTGSS_UTMI_OTG_STATUS_SESSEND;
		val |= USBOTGSS_UTMI_OTG_STATUS_IDDIG
				| USBOTGSS_UTMI_OTG_STATUS_VBUSVALID
				| USBOTGSS_UTMI_OTG_STATUS_SESSVALID
				| USBOTGSS_UTMI_OTG_STATUS_POWERPRESENT;
		dwc3_omap_write_utmi_status(omap, val);
		break;

	case OMAP_DWC3_ID_FLOAT:
	case OMAP_DWC3_VBUS_OFF:
		dev_dbg(omap->dev, "VBUS Disconnect\n");

		val = dwc3_omap_read_utmi_status(omap);
		val &= ~(USBOTGSS_UTMI_OTG_STATUS_SESSVALID
				| USBOTGSS_UTMI_OTG_STATUS_VBUSVALID
				| USBOTGSS_UTMI_OTG_STATUS_POWERPRESENT);
		val |= USBOTGSS_UTMI_OTG_STATUS_SESSEND
				| USBOTGSS_UTMI_OTG_STATUS_IDDIG;
		dwc3_omap_write_utmi_status(omap, val);
		break;

	default:
		dev_dbg(omap->dev, "invalid state\n");
	}
}

static irqreturn_t dwc3_omap_interrupt(int irq, void *_omap)
{
	struct dwc3_omap	*omap = _omap;
	u32			reg;

	reg = dwc3_omap_read_irqmisc_status(omap);

	if (reg & USBOTGSS_IRQMISC_DMADISABLECLR) {
		dev_dbg(omap->dev, "DMA Disable was Cleared\n");
		omap->dma_status = false;
	}

	if (reg & USBOTGSS_IRQMISC_OEVT)
		dev_dbg(omap->dev, "OTG Event\n");

	if (reg & USBOTGSS_IRQMISC_DRVVBUS_RISE)
		dev_dbg(omap->dev, "DRVVBUS Rise\n");

	if (reg & USBOTGSS_IRQMISC_CHRGVBUS_RISE)
		dev_dbg(omap->dev, "CHRGVBUS Rise\n");

	if (reg & USBOTGSS_IRQMISC_DISCHRGVBUS_RISE)
		dev_dbg(omap->dev, "DISCHRGVBUS Rise\n");

	if (reg & USBOTGSS_IRQMISC_IDPULLUP_RISE)
		dev_dbg(omap->dev, "IDPULLUP Rise\n");

	if (reg & USBOTGSS_IRQMISC_DRVVBUS_FALL)
		dev_dbg(omap->dev, "DRVVBUS Fall\n");

	if (reg & USBOTGSS_IRQMISC_CHRGVBUS_FALL)
		dev_dbg(omap->dev, "CHRGVBUS Fall\n");

	if (reg & USBOTGSS_IRQMISC_DISCHRGVBUS_FALL)
		dev_dbg(omap->dev, "DISCHRGVBUS Fall\n");

	if (reg & USBOTGSS_IRQMISC_IDPULLUP_FALL)
		dev_dbg(omap->dev, "IDPULLUP Fall\n");

	dwc3_omap_write_irqmisc_status(omap, reg);

	reg = dwc3_omap_read_irq0_status(omap);

	dwc3_omap_write_irq0_status(omap, reg);

	return IRQ_HANDLED;
}

static int dwc3_omap_remove_core(struct device *dev, void *c)
{
	struct platform_device *pdev = to_platform_device(dev);

	of_device_unregister(pdev);

	return 0;
}

static void dwc3_omap_enable_irqs(struct dwc3_omap *omap)
{
	u32			reg;

	/* enable all IRQs */
	reg = USBOTGSS_IRQO_COREIRQ_ST;
	dwc3_omap_write_irq0_set(omap, reg);

	reg = (USBOTGSS_IRQMISC_OEVT |
			USBOTGSS_IRQMISC_DRVVBUS_RISE |
			USBOTGSS_IRQMISC_CHRGVBUS_RISE |
			USBOTGSS_IRQMISC_DISCHRGVBUS_RISE |
			USBOTGSS_IRQMISC_IDPULLUP_RISE |
			USBOTGSS_IRQMISC_DRVVBUS_FALL |
			USBOTGSS_IRQMISC_CHRGVBUS_FALL |
			USBOTGSS_IRQMISC_DISCHRGVBUS_FALL |
			USBOTGSS_IRQMISC_IDPULLUP_FALL);

	dwc3_omap_write_irqmisc_set(omap, reg);
}

static void dwc3_omap_disable_irqs(struct dwc3_omap *omap)
{
	/* disable all IRQs */
	dwc3_omap_write_irqmisc_set(omap, 0x00);
	dwc3_omap_write_irq0_set(omap, 0x00);
}

static u64 dwc3_omap_dma_mask = DMA_BIT_MASK(32);

static void dwc3_omap_map_offset(struct dwc3_omap *omap)
{
	struct device_node	*node = omap->dev->of_node;

	/*
	 * Differentiate between OMAP5 and AM437x.
	 *
	 * For OMAP5(ES2.0) and AM437x wrapper revision is same, even
	 * though there are changes in wrapper register offsets.
	 *
	 * Using dt compatible to differentiate AM437x.
	 */
	if (of_device_is_compatible(node, "ti,am437x-dwc3")) {
		omap->irq_eoi_offset = USBOTGSS_EOI_OFFSET;
		omap->irq0_offset = USBOTGSS_IRQ0_OFFSET;
		omap->irqmisc_offset = USBOTGSS_IRQMISC_OFFSET;
		omap->utmi_otg_offset = USBOTGSS_UTMI_OTG_OFFSET;
		omap->debug_offset = USBOTGSS_DEBUG_OFFSET;
	}
}

static void dwc3_omap_set_utmi_mode(struct dwc3_omap *omap)
{
	u32			reg;
	struct device_node	*node = omap->dev->of_node;
	int			utmi_mode = 0;

	reg = dwc3_omap_read_utmi_status(omap);

	of_property_read_u32(node, "utmi-mode", &utmi_mode);

	switch (utmi_mode) {
	case DWC3_OMAP_UTMI_MODE_SW:
		reg |= USBOTGSS_UTMI_OTG_STATUS_SW_MODE;
		break;
	case DWC3_OMAP_UTMI_MODE_HW:
		reg &= ~USBOTGSS_UTMI_OTG_STATUS_SW_MODE;
		break;
	default:
		dev_dbg(omap->dev, "UNKNOWN utmi mode %d\n", utmi_mode);
	}

	dwc3_omap_write_utmi_status(omap, reg);
}

static int dwc3_omap_probe(struct platform_device *pdev)
{
	struct device_node	*node = pdev->dev.of_node;

	struct dwc3_omap	*omap;
	struct resource		*res;
	struct device		*dev = &pdev->dev;

	int			ret;
	u32			reg;

	void __iomem		*base;

	if (!node) {
		dev_err(dev, "device node not found\n");
		return -EINVAL;
	}

	omap = devm_kzalloc(dev, sizeof(*omap), GFP_KERNEL);
	if (!omap)
		return -ENOMEM;

	platform_set_drvdata(pdev, omap);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	omap->dev	= dev;
	omap->base	= base;
	dev->dma_mask	= &dwc3_omap_dma_mask;

	dwc3_omap_map_offset(omap);
	dwc3_omap_set_utmi_mode(omap);

	/* check the DMA Status */
	reg = dwc3_omap_readl(omap->base, USBOTGSS_SYSCONFIG);
	omap->dma_status = !!(reg & USBOTGSS_SYSCONFIG_DMADISABLE);

	dwc3_omap_enable_irqs(omap);

	ret = of_platform_populate(node, NULL, NULL, dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to create dwc3 core\n");
		goto err1;
	}

	return 0;

err1:
	dwc3_omap_disable_irqs(omap);

err0:
	return ret;
}

static int dwc3_omap_remove(struct platform_device *pdev)
{
	struct dwc3_omap	*omap = platform_get_drvdata(pdev);

	dwc3_omap_disable_irqs(omap);
	device_for_each_child(&pdev->dev, NULL, dwc3_omap_remove_core);

	return 0;
}

static const struct of_device_id of_dwc3_match[] = {
	{
		.compatible =	"ti,dwc3"
	},
	{
		.compatible =	"ti,am437x-dwc3"
	},
	{ },
};
MODULE_DEVICE_TABLE(of, of_dwc3_match);

static struct platform_driver dwc3_omap_driver = {
	.probe		= dwc3_omap_probe,
	.remove		= dwc3_omap_remove,
	.driver		= {
		.name	= "omap-dwc3",
		.of_match_table	= of_dwc3_match,
	},
};

module_platform_driver(dwc3_omap_driver);

MODULE_ALIAS("platform:omap-dwc3");
MODULE_AUTHOR("Felipe Balbi <balbi@ti.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DesignWare USB3 OMAP Glue Layer");
