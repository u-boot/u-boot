// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Author: Greg Malysa <greg.malysa@timesys.com>
 * Additional Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 */

#include <dm.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/io.h>

#define ADSP_PORT_MMIO_SIZE		0x80
#define ADSP_PORT_PIN_SIZE		16

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

struct adsp_gpio_priv {
	void __iomem *base;
	int ngpio;
};

static u32 get_port(unsigned int pin)
{
	return pin / ADSP_PORT_PIN_SIZE;
}

static u32 get_offset(unsigned int pin)
{
	return pin % ADSP_PORT_PIN_SIZE;
}

static int adsp_gpio_input(struct udevice *udev, unsigned int pin)
{
	struct adsp_gpio_priv *priv = dev_get_priv(udev);
	u32 port, offset;
	void __iomem *portbase;

	if (pin < priv->ngpio) {
		port = get_port(pin);
		offset = get_offset(pin);
		portbase = priv->base + port * ADSP_PORT_MMIO_SIZE;

		iowrite16(BIT(offset), portbase + ADSP_PORT_REG_FER_CLEAR);
		iowrite16(BIT(offset), portbase + ADSP_PORT_REG_DIR_CLEAR);
		iowrite16(BIT(offset), portbase + ADSP_PORT_REG_INEN_SET);
		return 0;
	}

	return -EINVAL;
}

static int adsp_gpio_output(struct udevice *udev, unsigned int pin, int value)
{
	struct adsp_gpio_priv *priv = dev_get_priv(udev);
	u32 port, offset;
	void __iomem *portbase;

	if (pin < priv->ngpio) {
		port = get_port(pin);
		offset = get_offset(pin);
		portbase = priv->base + port * ADSP_PORT_MMIO_SIZE;

		iowrite16(BIT(offset), portbase + ADSP_PORT_REG_FER_CLEAR);

		if (value)
			iowrite16(BIT(offset), portbase + ADSP_PORT_REG_DATA_SET);
		else
			iowrite16(BIT(offset), portbase + ADSP_PORT_REG_DATA_CLEAR);

		iowrite16(BIT(offset), portbase + ADSP_PORT_REG_DIR_SET);
		iowrite16(BIT(offset), portbase + ADSP_PORT_REG_INEN_CLEAR);
		return 0;
	}

	return -EINVAL;
}

static int adsp_gpio_get_value(struct udevice *udev, unsigned int pin)
{
	struct adsp_gpio_priv *priv = dev_get_priv(udev);
	u32 port, offset;
	u16 val;
	void __iomem *portbase;

	if (pin < priv->ngpio) {
		port = get_port(pin);
		offset = get_offset(pin);
		portbase = priv->base + port * ADSP_PORT_MMIO_SIZE;

		val = ioread16(portbase + ADSP_PORT_REG_DATA);
		return !!(val & BIT(offset));
	}

	return 0;
}

static int adsp_gpio_set_value(struct udevice *udev, unsigned int pin, int value)
{
	struct adsp_gpio_priv *priv = dev_get_priv(udev);
	u32 port, offset;
	void __iomem *portbase;

	if (pin < priv->ngpio) {
		port = get_port(pin);
		offset = get_offset(pin);
		portbase = priv->base + port * ADSP_PORT_MMIO_SIZE;

		if (value)
			iowrite16(BIT(offset), portbase + ADSP_PORT_REG_DATA_SET);
		else
			iowrite16(BIT(offset), portbase + ADSP_PORT_REG_DATA_CLEAR);
	}

	return 0;
}

static const struct dm_gpio_ops adsp_gpio_ops = {
	.direction_input = adsp_gpio_input,
	.direction_output = adsp_gpio_output,
	.get_value = adsp_gpio_get_value,
	.set_value = adsp_gpio_set_value,
};

static int adsp_gpio_probe(struct udevice *udev)
{
	struct adsp_gpio_priv *priv = dev_get_priv(udev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	uc_priv->bank_name = "adsp gpio";
	uc_priv->gpio_count = dev_read_u32_default(udev, "adi,ngpios", 0);

	if (!uc_priv->gpio_count) {
		dev_err(udev, "Missing adi,ngpios property!\n");
		return -ENOENT;
	}

	priv->base = dev_read_addr_ptr(udev);
	priv->ngpio = uc_priv->gpio_count;

	return 0;
}

static const struct udevice_id adsp_gpio_match[] = {
	{ .compatible = "adi,adsp-gpio" },
	{ },
};

U_BOOT_DRIVER(adi_adsp_gpio) = {
	.name	= "adi_adsp_gpio",
	.id	= UCLASS_GPIO,
	.ops	= &adsp_gpio_ops,
	.probe	= adsp_gpio_probe,
	.priv_auto = sizeof(struct adsp_gpio_priv),
	.of_match = adsp_gpio_match,
	.flags = DM_FLAG_PRE_RELOC,
};
