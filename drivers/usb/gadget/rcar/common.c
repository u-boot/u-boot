// SPDX-License-Identifier: GPL-1.0+
/*
 * Renesas USB driver
 *
 * Copyright (C) 2011 Renesas Solutions Corp.
 * Copyright (C) 2019 Renesas Electronics Corporation
 * Kuninori Morimoto <kuninori.morimoto.gx@renesas.com>
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <linux/err.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <usb.h>

#include "common.h"

/*
 *		image of renesas_usbhs
 *
 * ex) gadget case

 * mod.c
 * mod_gadget.c
 * mod_host.c		pipe.c		fifo.c
 *
 *			+-------+	+-----------+
 *			| pipe0 |------>| fifo pio  |
 * +------------+	+-------+	+-----------+
 * | mod_gadget |=====> | pipe1 |--+
 * +------------+	+-------+  |	+-----------+
 *			| pipe2 |  |  +-| fifo dma0 |
 * +------------+	+-------+  |  |	+-----------+
 * | mod_host   |	| pipe3 |<-|--+
 * +------------+	+-------+  |	+-----------+
 *			| ....  |  +--->| fifo dma1 |
 *			| ....  |	+-----------+
 */

/*
 *		common functions
 */
u16 usbhs_read(struct usbhs_priv *priv, u32 reg)
{
	return ioread16(priv->base + reg);
}

void usbhs_write(struct usbhs_priv *priv, u32 reg, u16 data)
{
	iowrite16(data, priv->base + reg);
}

void usbhs_bset(struct usbhs_priv *priv, u32 reg, u16 mask, u16 data)
{
	u16 val = usbhs_read(priv, reg);

	val &= ~mask;
	val |= data & mask;

	usbhs_write(priv, reg, val);
}

/*
 *		syscfg functions
 */
static void usbhs_sys_clock_ctrl(struct usbhs_priv *priv, int enable)
{
	usbhs_bset(priv, SYSCFG, SCKE, enable ? SCKE : 0);
}

void usbhs_sys_host_ctrl(struct usbhs_priv *priv, int enable)
{
	u16 mask = DCFM | DRPD | DPRPU | HSE | USBE;
	u16 val  = DCFM | DRPD | HSE | USBE;

	/*
	 * if enable
	 *
	 * - select Host mode
	 * - D+ Line/D- Line Pull-down
	 */
	usbhs_bset(priv, SYSCFG, mask, enable ? val : 0);
}

void usbhs_sys_function_ctrl(struct usbhs_priv *priv, int enable)
{
	u16 mask = DCFM | DRPD | DPRPU | HSE | USBE;
	u16 val  = HSE | USBE;

	/*
	 * if enable
	 *
	 * - select Function mode
	 * - D+ Line Pull-up is disabled
	 *      When D+ Line Pull-up is enabled,
	 *      calling usbhs_sys_function_pullup(,1)
	 */
	usbhs_bset(priv, SYSCFG, mask, enable ? val : 0);
}

void usbhs_sys_function_pullup(struct usbhs_priv *priv, int enable)
{
	usbhs_bset(priv, SYSCFG, DPRPU, enable ? DPRPU : 0);
}

void usbhs_sys_set_test_mode(struct usbhs_priv *priv, u16 mode)
{
	usbhs_write(priv, TESTMODE, mode);
}

/*
 *		frame functions
 */
int usbhs_frame_get_num(struct usbhs_priv *priv)
{
	return usbhs_read(priv, FRMNUM) & FRNM_MASK;
}

/*
 *		usb request functions
 */
void usbhs_usbreq_get_val(struct usbhs_priv *priv, struct usb_ctrlrequest *req)
{
	u16 val;

	val = usbhs_read(priv, USBREQ);
	req->bRequest		= (val >> 8) & 0xFF;
	req->bRequestType	= (val >> 0) & 0xFF;

	req->wValue	= cpu_to_le16(usbhs_read(priv, USBVAL));
	req->wIndex	= cpu_to_le16(usbhs_read(priv, USBINDX));
	req->wLength	= cpu_to_le16(usbhs_read(priv, USBLENG));
}

void usbhs_usbreq_set_val(struct usbhs_priv *priv, struct usb_ctrlrequest *req)
{
	usbhs_write(priv, USBREQ,  (req->bRequest << 8) | req->bRequestType);
	usbhs_write(priv, USBVAL,  le16_to_cpu(req->wValue));
	usbhs_write(priv, USBINDX, le16_to_cpu(req->wIndex));
	usbhs_write(priv, USBLENG, le16_to_cpu(req->wLength));

	usbhs_bset(priv, DCPCTR, SUREQ, SUREQ);
}

/*
 *		bus/vbus functions
 */
void usbhs_bus_send_sof_enable(struct usbhs_priv *priv)
{
	u16 status = usbhs_read(priv, DVSTCTR) & (USBRST | UACT);

	if (status != USBRST) {
		struct device *dev = usbhs_priv_to_dev(priv);
		dev_err(dev, "usbhs should be reset\n");
	}

	usbhs_bset(priv, DVSTCTR, (USBRST | UACT), UACT);
}

void usbhs_bus_send_reset(struct usbhs_priv *priv)
{
	usbhs_bset(priv, DVSTCTR, (USBRST | UACT), USBRST);
}

int usbhs_bus_get_speed(struct usbhs_priv *priv)
{
	u16 dvstctr = usbhs_read(priv, DVSTCTR);

	switch (RHST & dvstctr) {
	case RHST_LOW_SPEED:
		return USB_SPEED_LOW;
	case RHST_FULL_SPEED:
		return USB_SPEED_FULL;
	case RHST_HIGH_SPEED:
		return USB_SPEED_HIGH;
	}

	return USB_SPEED_UNKNOWN;
}

static void usbhsc_bus_init(struct usbhs_priv *priv)
{
	usbhs_write(priv, DVSTCTR, 0);
}

/*
 *		device configuration
 */
int usbhs_set_device_config(struct usbhs_priv *priv, int devnum,
			   u16 upphub, u16 hubport, u16 speed)
{
	struct device *dev = usbhs_priv_to_dev(priv);
	u16 usbspd = 0;
	u32 reg = DEVADD0 + (2 * devnum);

	if (devnum > 10) {
		dev_err(dev, "cannot set speed to unknown device %d\n", devnum);
		return -EIO;
	}

	if (upphub > 0xA) {
		dev_err(dev, "unsupported hub number %d\n", upphub);
		return -EIO;
	}

	switch (speed) {
	case USB_SPEED_LOW:
		usbspd = USBSPD_SPEED_LOW;
		break;
	case USB_SPEED_FULL:
		usbspd = USBSPD_SPEED_FULL;
		break;
	case USB_SPEED_HIGH:
		usbspd = USBSPD_SPEED_HIGH;
		break;
	default:
		dev_err(dev, "unsupported speed %d\n", speed);
		return -EIO;
	}

	usbhs_write(priv, reg,	UPPHUB(upphub)	|
				HUBPORT(hubport)|
				USBSPD(usbspd));

	return 0;
}

/*
 *		interrupt functions
 */
void usbhs_xxxsts_clear(struct usbhs_priv *priv, u16 sts_reg, u16 bit)
{
	u16 pipe_mask = (u16)GENMASK(usbhs_get_dparam(priv, pipe_size), 0);

	usbhs_write(priv, sts_reg, ~(1 << bit) & pipe_mask);
}

/*
 *		local functions
 */
static void usbhsc_set_buswait(struct usbhs_priv *priv)
{
	int wait = usbhs_get_dparam(priv, buswait_bwait);

	/* set bus wait if platform have */
	if (wait)
		usbhs_bset(priv, BUSWAIT, 0x000F, wait);
}

/*
 *		platform default param
 */

/* commonly used on newer SH-Mobile and R-Car SoCs */
static struct renesas_usbhs_driver_pipe_config usbhsc_new_pipe[] = {
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_CONTROL, 64, 0x00, false),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_ISOC, 1024, 0x08, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_ISOC, 1024, 0x28, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0x48, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0x58, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0x68, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_INT, 64, 0x04, false),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_INT, 64, 0x05, false),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_INT, 64, 0x06, false),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0x78, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0x88, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0x98, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0xa8, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0xb8, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0xc8, true),
	RENESAS_USBHS_PIPE(USB_ENDPOINT_XFER_BULK, 512, 0xd8, true),
};

#define LPSTS			0x102
#define LPSTS_SUSPM		BIT(14)

#define UGCTRL2			0x184
#define UGCTRL2_RESERVED_3	BIT(0)
#define UGCTRL2_USB0SEL_EHCI	0x10
#define UGCTRL2_USB0SEL_HSUSB	0x20
#define UGCTRL2_USB0SEL_OTG	0x30
#define UGCTRL2_USB0SEL_MASK	0x30
#define UGCTRL2_VBUSSEL		BIT(10)

struct usbhs_priv_otg_data {
	void __iomem		*base;
	void __iomem		*phybase;

	struct platform_device	usbhs_dev;
	struct usbhs_priv	usbhs_priv;

	struct phy		phy;
};

static int usbhs_rcar3_power_ctrl(struct usbhs_priv *priv, bool enable)
{
	if (enable) {
		writel(UGCTRL2_USB0SEL_OTG | UGCTRL2_VBUSSEL | UGCTRL2_RESERVED_3,
		       priv->base + UGCTRL2);

		usbhs_bset(priv, LPSTS, LPSTS_SUSPM, LPSTS_SUSPM);
		/* The controller on R-Car Gen3 needs to wait up to 90 usec */
		udelay(90);

		usbhs_sys_clock_ctrl(priv, enable);
	} else {
		usbhs_sys_clock_ctrl(priv, enable);

		usbhs_bset(priv, LPSTS, LPSTS_SUSPM, 0);
	}

	return 0;
}

void usbhsc_hotplug(struct usbhs_priv *priv)
{
	int ret;

	ret = usbhs_mod_change(priv, USBHS_GADGET);
	if (ret < 0)
		return;

	usbhs_rcar3_power_ctrl(priv, true);

	/* bus init */
	usbhsc_set_buswait(priv);
	usbhsc_bus_init(priv);

	/* module start */
	usbhs_mod_call(priv, start, priv);
}

#define USB2_OBINTSTA		0x604
#define USB2_OBINT_SESSVLDCHG		BIT(12)
#define USB2_OBINT_IDDIGCHG		BIT(11)

static int usbhs_udc_otg_gadget_handle_interrupts(struct udevice *dev)
{
	struct usbhs_priv_otg_data *priv = dev_get_priv(dev);
	const u32 status = readl(priv->phybase + USB2_OBINTSTA);

	/* We don't have a good way to forward IRQ to PHY yet */
	if (status & (USB2_OBINT_SESSVLDCHG | USB2_OBINT_IDDIGCHG)) {
		writel(USB2_OBINT_SESSVLDCHG | USB2_OBINT_IDDIGCHG,
		       priv->phybase + USB2_OBINTSTA);
		generic_phy_set_mode(&priv->phy, PHY_MODE_USB_OTG, 0);
	}

	usbhs_interrupt(0, &priv->usbhs_priv);

	return 0;
}

static int usbhs_probe(struct usbhs_priv *priv)
{
	int ret;

	priv->dparam.type = USBHS_TYPE_RCAR_GEN3;
	priv->dparam.pio_dma_border = 64;
	priv->dparam.pipe_configs = usbhsc_new_pipe;
	priv->dparam.pipe_size = ARRAY_SIZE(usbhsc_new_pipe);

	/* call pipe and module init */
	ret = usbhs_pipe_probe(priv);
	if (ret < 0)
		return ret;

	ret = usbhs_fifo_probe(priv);
	if (ret < 0)
		goto probe_end_pipe_exit;

	ret = usbhs_mod_probe(priv);
	if (ret < 0)
		goto probe_end_fifo_exit;

	usbhs_sys_clock_ctrl(priv, 0);

	usbhs_rcar3_power_ctrl(priv, true);
	usbhs_mod_autonomy_mode(priv);
	usbhsc_hotplug(priv);

	return ret;

probe_end_fifo_exit:
	usbhs_fifo_remove(priv);
probe_end_pipe_exit:
	usbhs_pipe_remove(priv);
	return ret;
}

static int usbhs_udc_otg_probe(struct udevice *dev)
{
	struct usbhs_priv_otg_data *priv = dev_get_priv(dev);
	struct usb_gadget *gadget;
	struct clk_bulk clk_bulk;
	int ret = -EINVAL;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_bulk(dev, &clk_bulk);
	if (ret)
		return ret;

	ret = clk_enable_bulk(&clk_bulk);
	if (ret)
		return ret;

	clrsetbits_le32(priv->base + UGCTRL2, UGCTRL2_USB0SEL_MASK, UGCTRL2_USB0SEL_EHCI);
	clrsetbits_le16(priv->base + LPSTS, LPSTS_SUSPM, LPSTS_SUSPM);

	ret = generic_setup_phy(dev, &priv->phy, 0, PHY_MODE_USB_OTG, 1);
	if (ret)
		goto err_clk;

	priv->phybase = dev_read_addr_ptr(priv->phy.dev);

	priv->usbhs_priv.pdev = &priv->usbhs_dev;
	priv->usbhs_priv.base = priv->base;
	priv->usbhs_dev.dev.driver_data = &priv->usbhs_priv;
	ret = usbhs_probe(&priv->usbhs_priv);
	if (ret < 0)
		goto err_phy;

	gadget = usbhsg_get_gadget(&priv->usbhs_priv);
	gadget->is_dualspeed = 1;
	gadget->is_otg = 0;
	gadget->is_a_peripheral = 0;
	gadget->b_hnp_enable = 0;
	gadget->a_hnp_support = 0;
	gadget->a_alt_hnp_support = 0;

	return usb_add_gadget_udc((struct device *)dev, gadget);

err_phy:
	generic_shutdown_phy(&priv->phy);
err_clk:
	clk_disable_bulk(&clk_bulk);
	return ret;
}

static int usbhs_udc_otg_remove(struct udevice *dev)
{
	struct usbhs_priv_otg_data *priv = dev_get_priv(dev);

	usbhs_rcar3_power_ctrl(&priv->usbhs_priv, false);
	usbhs_mod_remove(&priv->usbhs_priv);
	usbhs_fifo_remove(&priv->usbhs_priv);
	usbhs_pipe_remove(&priv->usbhs_priv);

	generic_shutdown_phy(&priv->phy);

	return dm_scan_fdt_dev(dev);
}

static const struct udevice_id usbhs_udc_otg_ids[] = {
	{ .compatible = "renesas,rcar-gen3-usbhs" },
	{},
};

static const struct usb_gadget_generic_ops usbhs_udc_otg_ops = {
	.handle_interrupts = usbhs_udc_otg_gadget_handle_interrupts,
};

U_BOOT_DRIVER(usbhs_udc_otg) = {
	.name		= "usbhs-udc-otg",
	.id		= UCLASS_USB_GADGET_GENERIC,
	.ops		= &usbhs_udc_otg_ops,
	.of_match	= usbhs_udc_otg_ids,
	.probe		= usbhs_udc_otg_probe,
	.remove		= usbhs_udc_otg_remove,
	.priv_auto	= sizeof(struct usbhs_priv_otg_data),
};
