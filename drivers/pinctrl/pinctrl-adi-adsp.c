// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Author: Greg Malysa <greg.malysa@timesys.com>
 * Additional Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 *
 * dm pinctrl implementation for ADI ADSP SoCs
 *
 */

#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/io.h>

#define ADSP_PORT_MMIO_SIZE		0x80
#define ADSP_PORT_PIN_SIZE		16

#define ADSP_PORT_PORT_MUX_BITS		2
#define ADSP_PORT_PORT_MUX_MASK		0x03
#define ADSP_PINCTRL_FUNCTION_COUNT 4

#define ADSP_PORT_REG_FER			0x00
#define ADSP_PORT_REG_FER_SET		0x04
#define ADSP_PORT_REG_FER_CLEAR		0x08
#define ADSP_PORT_REG_DATA			0x0c
#define ADSP_PORT_REG_DATA_SET		0x10
#define ADSP_PORT_REG_DATA_CLEAR	0x14
#define ADSP_PORT_REG_DIR			0x18
#define ADSP_PORT_REG_DIR_SET		0x1c
#define ADSP_PORT_REG_DIR_CLEAR		0x20
#define ADSP_PORT_REG_INEN			0x24
#define ADSP_PORT_REG_INEN_SET		0x28
#define ADSP_PORT_REG_INEN_CLEAR	0x2c
#define ADSP_PORT_REG_PORT_MUX		0x30
#define ADSP_PORT_REG_DATA_TGL		0x34
#define ADSP_PORT_REG_POLAR			0x38
#define ADSP_PORT_REG_POLAR_SET		0x3c
#define ADSP_PORT_REG_POLAR_CLEAR	0x40
#define ADSP_PORT_REG_LOCK			0x44
#define ADSP_PORT_REG_TRIG_TGL		0x48

struct adsp_pinctrl_priv {
	void __iomem *base;
	int npins;
	char pinbuf[16];
};

static u32 get_port(unsigned int pin)
{
	return pin / ADSP_PORT_PIN_SIZE;
}

static u32 get_offset(unsigned int pin)
{
	return pin % ADSP_PORT_PIN_SIZE;
}

static int adsp_pinctrl_pinmux_set(struct udevice *udev, unsigned int pin, unsigned int func)
{
	struct adsp_pinctrl_priv *priv = dev_get_priv(udev);
	void __iomem *portbase;
	u32 port, offset;
	u32 val;

	if (pin >= priv->npins)
		return -ENODEV;

	if (func >= ADSP_PINCTRL_FUNCTION_COUNT)
		return -EINVAL;

	port = get_port(pin);
	offset = get_offset(pin);
	portbase = priv->base + port * ADSP_PORT_MMIO_SIZE;

	val = ioread32(portbase + ADSP_PORT_REG_PORT_MUX);
	val &= ~(ADSP_PORT_PORT_MUX_MASK << (ADSP_PORT_PORT_MUX_BITS * offset));
	val |= func << (ADSP_PORT_PORT_MUX_BITS * offset);
	iowrite32(val, portbase + ADSP_PORT_REG_PORT_MUX);

	iowrite32(BIT(offset), portbase + ADSP_PORT_REG_FER_SET);
	return 0;
}

static int adsp_pinctrl_set_state(struct udevice *udev, struct udevice *config)
{
	const struct fdt_property *pinlist;
	int length = 0;
	int ret, i;
	u32 pin, function;

	pinlist = dev_read_prop(config, "adi,pins", &length);
	if (!pinlist) {
		dev_err(udev, "missing adi,pins property in pinctrl config node\n");
		return -EINVAL;
	}

	if (length % (sizeof(uint32_t) * 2)) {
		dev_err(udev, "adi,pins property must be a multiple of two uint32_ts\n");
		return -EINVAL;
	}

	for (i = 0; i < length / sizeof(uint32_t); i += 2) {
		ret = dev_read_u32_index(config, "adi,pins", i, &pin);
		if (ret)
			return ret;

		ret = dev_read_u32_index(config, "adi,pins", i + 1, &function);
		if (ret)
			return ret;

		ret = adsp_pinctrl_pinmux_set(udev, pin, function);
		if (ret)
			return ret;
	}

	return 0;
}

const struct pinctrl_ops adsp_pinctrl_ops = {
	.set_state = adsp_pinctrl_set_state,
};

static int adsp_pinctrl_probe(struct udevice *udev)
{
	struct adsp_pinctrl_priv *priv = dev_get_priv(udev);

	priv->base = dev_read_addr_ptr(udev);
	priv->npins = dev_read_u32_default(udev, "adi,npins", 0);

	if (!priv->base) {
		dev_err(udev, "Missing or invalid pinctrl base address\n");
		return -ENOENT;
	}

	if (!priv->npins) {
		dev_err(udev, "Missing adi,npins property!\n");
		return -ENOENT;
	}

	return 0;
}

static const struct udevice_id adsp_pinctrl_match[] = {
	{ .compatible = "adi,adsp-pinctrl" },
	{ },
};

U_BOOT_DRIVER(adi_adsp_pinctrl) = {
	.name = "adi_adsp_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = adsp_pinctrl_match,
	.probe = adsp_pinctrl_probe,
	.priv_auto = sizeof(struct adsp_pinctrl_priv),
	.ops = &adsp_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
