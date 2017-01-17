/*
 * MISC driver for TI MUSB Glue.
 *
 * (C) Copyright 2016
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <linux/usb/otg.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

#include <asm/io.h>
#include <asm/omap_musb.h>
#include "musb_uboot.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DM_USB

/* USB 2.0 PHY Control */
#define CM_PHY_PWRDN			(1 << 0)
#define CM_PHY_OTG_PWRDN		(1 << 1)
#define OTGVDET_EN			(1 << 19)
#define OTGSESSENDEN			(1 << 20)

#define AM335X_USB1_CTRL	0x8

struct ti_musb_platdata {
	void *base;
	void *ctrl_mod_base;
	struct musb_hdrc_platform_data plat;
	struct musb_hdrc_config musb_config;
	struct omap_musb_board_data otg_board_data;
};

static int ti_musb_get_usb_index(int node)
{
	const void *fdt = gd->fdt_blob;
	int i = 0;
	char path[64];
	const char *alias_path;
	char alias[16];

	fdt_get_path(fdt, node, path, sizeof(path));

	do {
		snprintf(alias, sizeof(alias), "usb%d", i);
		alias_path = fdt_get_alias(fdt, alias);
		if (alias_path == NULL) {
			debug("USB index not found\n");
			return -ENOENT;
		}

		if (!strcmp(path, alias_path))
			return i;

		i++;
	} while (alias_path);

	return -ENOENT;
}

static void ti_musb_set_phy_power(struct udevice *dev, u8 on)
{
	struct ti_musb_platdata *platdata = dev_get_platdata(dev);

	if (on) {
		clrsetbits_le32(platdata->ctrl_mod_base,
				CM_PHY_PWRDN | CM_PHY_OTG_PWRDN,
				OTGVDET_EN | OTGSESSENDEN);
	} else {
		clrsetbits_le32(platdata->ctrl_mod_base, 0,
				CM_PHY_PWRDN | CM_PHY_OTG_PWRDN);
	}
}

static int ti_musb_ofdata_to_platdata(struct udevice *dev)
{
	struct ti_musb_platdata *platdata = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int phys;
	int ctrl_mod;
	int usb_index;

	platdata->base = (void *)dev_get_addr_index(dev, 1);

	phys = fdtdec_lookup_phandle(fdt, node, "phys");
	ctrl_mod = fdtdec_lookup_phandle(fdt, phys, "ti,ctrl_mod");
	platdata->ctrl_mod_base = (void *)fdtdec_get_addr(fdt, ctrl_mod, "reg");
	usb_index = ti_musb_get_usb_index(node);
	switch (usb_index) {
	case 1:
		platdata->ctrl_mod_base += AM335X_USB1_CTRL;
	case 0:
	default:
		break;
	}

	platdata->musb_config.multipoint = fdtdec_get_int(fdt, node,
							  "mentor,multipoint",
							  -1);
	if (platdata->musb_config.multipoint < 0) {
		error("MUSB multipoint DT entry missing\n");
		return -ENOENT;
	}

	platdata->musb_config.dyn_fifo = 1;

	platdata->musb_config.num_eps = fdtdec_get_int(fdt, node,
						       "mentor,num-eps", -1);
	if (platdata->musb_config.num_eps < 0) {
		error("MUSB num-eps DT entry missing\n");
		return -ENOENT;
	}

	platdata->musb_config.ram_bits = fdtdec_get_int(fdt, node,
							"mentor,ram-bits", -1);
	if (platdata->musb_config.ram_bits < 0) {
		error("MUSB ram-bits DT entry missing\n");
		return -ENOENT;
	}

	platdata->otg_board_data.set_phy_power = ti_musb_set_phy_power;
	platdata->otg_board_data.dev = dev;
	platdata->plat.config = &platdata->musb_config;

	platdata->plat.power = fdtdec_get_int(fdt, node, "mentor,power", -1);
	if (platdata->plat.power < 0) {
		error("MUSB mentor,power DT entry missing\n");
		return -ENOENT;
	}

	platdata->plat.platform_ops = &musb_dsps_ops;
	platdata->plat.board_data = &platdata->otg_board_data;

	return 0;
}

static int ti_musb_host_probe(struct udevice *dev)
{
	struct musb_host_data *host = dev_get_priv(dev);
	struct ti_musb_platdata *platdata = dev_get_platdata(dev);
	struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
	struct omap_musb_board_data *otg_board_data;
	int ret;

	priv->desc_before_addr = true;

	otg_board_data = &platdata->otg_board_data;

	host->host = musb_init_controller(&platdata->plat,
					  (struct device *)otg_board_data,
					  platdata->base);
	if (!host->host)
		return -EIO;

	ret = musb_lowlevel_init(host);

	return ret;
}

static int ti_musb_host_remove(struct udevice *dev)
{
	struct musb_host_data *host = dev_get_priv(dev);

	musb_stop(host->host);

	return 0;
}

static int ti_musb_host_ofdata_to_platdata(struct udevice *dev)
{
	struct ti_musb_platdata *platdata = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	ret = ti_musb_ofdata_to_platdata(dev);
	if (ret) {
		error("platdata dt parse error\n");
		return ret;
	}

	platdata->plat.mode = MUSB_HOST;

	return 0;
}

U_BOOT_DRIVER(ti_musb_host) = {
	.name	= "ti-musb-host",
	.id	= UCLASS_USB,
	.ofdata_to_platdata = ti_musb_host_ofdata_to_platdata,
	.probe = ti_musb_host_probe,
	.remove = ti_musb_host_remove,
	.ops	= &musb_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct ti_musb_platdata),
	.priv_auto_alloc_size = sizeof(struct musb_host_data),
};

static int ti_musb_wrapper_bind(struct udevice *parent)
{
	const void *fdt = gd->fdt_blob;
	int node;
	int ret;

	for (node = fdt_first_subnode(fdt, dev_of_offset(parent)); node > 0;
	     node = fdt_next_subnode(fdt, node)) {
		struct udevice *dev;
		const char *name = fdt_get_name(fdt, node, NULL);
		enum usb_dr_mode dr_mode;
		struct driver *drv;

		if (strncmp(name, "usb@", 4))
			continue;

		dr_mode = usb_get_dr_mode(node);
		switch (dr_mode) {
		case USB_DR_MODE_PERIPHERAL:
			/* Bind MUSB device */
			break;
		case USB_DR_MODE_HOST:
			/* Bind MUSB host */
			ret = device_bind_driver_to_node(parent, "ti-musb-host",
							 name, node, &dev);
			if (ret) {
				error("musb - not able to bind usb host node\n");
				return ret;
			}
			break;
		default:
			break;
		};
	}
	return 0;
}

static const struct udevice_id ti_musb_ids[] = {
	{ .compatible = "ti,am33xx-usb" },
	{ }
};

U_BOOT_DRIVER(ti_musb_wrapper) = {
	.name	= "ti-musb-wrapper",
	.id	= UCLASS_MISC,
	.of_match = ti_musb_ids,
	.bind = ti_musb_wrapper_bind,
};

#endif /* CONFIG_DM_USB */
