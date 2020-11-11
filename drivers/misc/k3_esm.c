// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 Error Signalling Module driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 *      Tero Kristo <t-kristo@ti.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>

#define ESM_SFT_RST			0x0c
#define ESM_SFT_RST_KEY			0x0f

#define ESM_STS(i)			(0x404 + (i) / 32 * 0x20)
#define ESM_PIN_EN_SET_OFFSET(i)	(0x414 + (i) / 32 * 0x20)
#define ESM_PIN_MASK(i)			BIT((i) & 0x1f)

static void esm_pin_enable(void __iomem *base, int pin)
{
	/* Enable event */
	writel(ESM_PIN_MASK(pin), base + ESM_PIN_EN_SET_OFFSET(pin));
}

/**
 * k3_esm_probe: configures ESM based on DT data
 *
 * Parses ESM info from device tree, and configures the module accordingly.
 */
static int k3_esm_probe(struct udevice *dev)
{
	int ret;
	void __iomem *base;
	int num_pins;
	u32 *pins;
	int i;

	base = dev_remap_addr_index(dev, 0);
	if (!base)
		return -ENODEV;

	num_pins = dev_read_size(dev, "ti,esm-pins");
	if (num_pins < 0) {
		dev_err(dev, "ti,esm-pins property missing or invalid: %d\n",
			num_pins);
		return num_pins;
	}

	num_pins /= sizeof(u32);

	pins = kmalloc(num_pins * sizeof(u32), __GFP_ZERO);
	if (!pins)
		return -ENOMEM;

	ret = dev_read_u32_array(dev, "ti,esm-pins", pins, num_pins);
	if (ret < 0) {
		dev_err(dev, "failed to read ti,esm-pins property: %d\n",
			ret);
		goto free_pins;
	}

	/* Clear any pending events */
	writel(ESM_SFT_RST_KEY, base + ESM_SFT_RST);

	for (i = 0; i < num_pins; i++)
		esm_pin_enable(base, pins[i]);

free_pins:
	kfree(pins);
	return ret;
}

static const struct udevice_id k3_esm_ids[] = {
	{ .compatible = "ti,j721e-esm" },
	{}
};

U_BOOT_DRIVER(k3_esm) = {
	.name = "k3_esm",
	.of_match = k3_esm_ids,
	.id = UCLASS_MISC,
	.probe = k3_esm_probe,
};
