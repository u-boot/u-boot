// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021, Collabora Ltd.
 * Copyright (C) 2021, General Electric Company
 * Author(s): Sebastian Reichel <sebastian.reichel@collabora.com>
 */

#define LOG_CATEGORY UCLASS_GPIO

#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <spi.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <dt-bindings/gpio/gpio.h>
#include <linux/delay.h>

enum mcp230xx_type {
	UNKNOWN = 0,
	MCP23008,
	MCP23017,
	MCP23018,
	MCP23S08,
	MCP23S17,
	MCP23S18,
};

struct mcp230xx_info {
	uint dev_addr;
};

#define MCP230XX_IODIR 0x00
#define MCP230XX_GPPU 0x06
#define MCP230XX_GPIO 0x09
#define MCP230XX_OLAT 0x0a

#define BANKSIZE 8

#define MCP230XX_ADDR 0x20

static int mcp230xx_read_spi(struct udevice *dev, uint reg_addr)
{
	struct mcp230xx_info *info = dev_get_plat(dev);
	uint dev_addr, value = 0;
	int ret;

	/* set R/W bit for reading */
	dev_addr = (info->dev_addr << 1) | 1;

	ret = dm_spi_claim_bus(dev);
	if (ret)
		return ret;

	ret = dm_spi_xfer(dev, 0, NULL, NULL, SPI_XFER_BEGIN);
	if (ret < 0)
		goto fail;
	udelay(1);

	ret = dm_spi_xfer(dev, 8, &dev_addr, NULL, 0);
	if (ret < 0)
		goto fail;

	ret = dm_spi_xfer(dev, 8, &reg_addr, NULL, 0);
	if (ret < 0)
		goto fail;

	ret = dm_spi_xfer(dev, 8, NULL, &value, 0);

fail:
	dm_spi_xfer(dev, 0, NULL, NULL,  SPI_XFER_END);
	dm_spi_release_bus(dev);
	if (ret < 0)
		return ret;
	return value;
}

static int mcp230xx_read(struct udevice *dev, uint reg, uint offset)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int bank = offset / BANKSIZE;
	int mask = 1 << (offset % BANKSIZE);
	int shift = (uc_priv->gpio_count / BANKSIZE) - 1;
	int reg_addr = (reg << shift) | bank;
	int ret = 0;

	switch (dev_get_driver_data(dev)) {
	case MCP23008:
	case MCP23017:
	case MCP23018:
		ret = dm_i2c_reg_read(dev, reg_addr);
		break;
	case MCP23S08:
	case MCP23S17:
	case MCP23S18:
		ret = mcp230xx_read_spi(dev, reg_addr);
		break;
	default:
		return -ENODEV;
	}

	if (ret < 0)
		return ret;

	return !!(ret & mask);
}

static int mcp230xx_clrset_spi(struct udevice *dev, uint reg_addr, uint clr, uint set)
{
	struct mcp230xx_info *info = dev_get_plat(dev);
	int dev_addr, value;
	int ret;

	/* R/W bit = 0 for writing */
	dev_addr = (info->dev_addr << 1);

	ret = mcp230xx_read_spi(dev, reg_addr);
	if (ret < 0)
		return ret;

	value = ret;
	value &= ~clr;
	value |= set;

	ret = dm_spi_claim_bus(dev);
	if (ret)
		return ret;

	ret = dm_spi_xfer(dev, 0, NULL, NULL, SPI_XFER_BEGIN);
	if (ret < 0)
		goto fail;
	udelay(1);

	ret = dm_spi_xfer(dev, 8, &dev_addr, NULL, 0);
	if (ret < 0)
		goto fail;

	ret = dm_spi_xfer(dev, 8, &reg_addr, NULL, 0);
	if (ret < 0)
		goto fail;

	ret = dm_spi_xfer(dev, 8, &value, NULL,  0);

fail:
	dm_spi_xfer(dev, 0, NULL, NULL,  SPI_XFER_END);
	dm_spi_release_bus(dev);
	return ret;
}

static int mcp230xx_write(struct udevice *dev, uint reg, uint offset, bool val)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int bank = offset / BANKSIZE;
	int mask = 1 << (offset % BANKSIZE);
	int shift = (uc_priv->gpio_count / BANKSIZE) - 1;
	int reg_addr = (reg << shift) | bank;

	switch (dev_get_driver_data(dev)) {
	case MCP23008:
	case MCP23017:
	case MCP23018:
		return dm_i2c_reg_clrset(dev, reg_addr, mask, val ? mask : 0);
	case MCP23S08:
	case MCP23S17:
	case MCP23S18:
		return mcp230xx_clrset_spi(dev, reg_addr, mask, val ? mask : 0);
	default:
		return -ENODEV;
	}
}

static int mcp230xx_get_value(struct udevice *dev, uint offset)
{
	int ret;

	ret = mcp230xx_read(dev, MCP230XX_GPIO, offset);
	if (ret < 0) {
		dev_err(dev, "%s error: %d\n", __func__, ret);
		return ret;
	}

	return ret;
}

static int mcp230xx_set_value(struct udevice *dev, uint offset, int val)
{
	int ret;

	ret = mcp230xx_write(dev, MCP230XX_GPIO, offset, val);
	if (ret < 0) {
		dev_err(dev, "%s error: %d\n", __func__, ret);
		return ret;
	}

	return ret;
}

static int mcp230xx_get_flags(struct udevice *dev, unsigned int offset,
				  ulong *flags)
{
	int direction, pullup;

	pullup = mcp230xx_read(dev, MCP230XX_GPPU, offset);
	if (pullup < 0) {
		dev_err(dev, "%s error: %d\n", __func__, pullup);
		return pullup;
	}

	direction = mcp230xx_read(dev, MCP230XX_IODIR, offset);
	if (direction < 0) {
		dev_err(dev, "%s error: %d\n", __func__, direction);
		return direction;
	}

	*flags = direction ? GPIOD_IS_IN : GPIOD_IS_OUT;

	if (pullup)
		*flags |= GPIOD_PULL_UP;

	return 0;
}

static int mcp230xx_set_flags(struct udevice *dev, uint offset, ulong flags)
{
	bool input = !(flags & (GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE));
	bool pullup = flags & GPIOD_PULL_UP;
	ulong supported_mask;
	int ret;

	/* Note: active-low is ignored (handled by core) */
	supported_mask = GPIOD_ACTIVE_LOW | GPIOD_MASK_DIR | GPIOD_PULL_UP;
	if (flags & ~supported_mask) {
		dev_err(dev, "%s unsupported flag(s): %lx\n", __func__, flags);
		return -EINVAL;
	}

	ret = mcp230xx_write(dev, MCP230XX_OLAT, offset, !!(flags & GPIOD_IS_OUT_ACTIVE));
	if (ret) {
		dev_err(dev, "%s failed to setup output latch: %d\n", __func__, ret);
		return ret;
	}

	ret = mcp230xx_write(dev, MCP230XX_GPPU, offset, pullup);
	if (ret) {
		dev_err(dev, "%s failed to setup pull-up: %d\n", __func__, ret);
		return ret;
	}

	ret = mcp230xx_write(dev, MCP230XX_IODIR, offset, input);
	if (ret) {
		dev_err(dev, "%s failed to setup direction: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int mcp230xx_direction_input(struct udevice *dev, uint offset)
{
	return mcp230xx_set_flags(dev, offset, GPIOD_IS_IN);
}

static int mcp230xx_direction_output(struct udevice *dev, uint offset, int val)
{
	int ret = mcp230xx_set_value(dev, offset, val);
	if (ret < 0) {
		dev_err(dev, "%s error: %d\n", __func__, ret);
		return ret;
	}
	return mcp230xx_set_flags(dev, offset, GPIOD_IS_OUT);
}

static int mcp230xx_get_function(struct udevice *dev, uint offset)
{
	int ret;

	ret = mcp230xx_read(dev, MCP230XX_IODIR, offset);
	if (ret < 0) {
		dev_err(dev, "%s error: %d\n", __func__, ret);
		return ret;
	}

	return ret ? GPIOF_INPUT : GPIOF_OUTPUT;
}

static const struct dm_gpio_ops mcp230xx_ops = {
	.direction_input	= mcp230xx_direction_input,
	.direction_output	= mcp230xx_direction_output,
	.get_value		= mcp230xx_get_value,
	.set_value		= mcp230xx_set_value,
	.get_function		= mcp230xx_get_function,
	.set_flags		= mcp230xx_set_flags,
	.get_flags		= mcp230xx_get_flags,
};

static int mcp230xx_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mcp230xx_info *info = dev_get_plat(dev);
	char name[32], label[32], *str;
	int addr, gpio_count, size;
	const u8 *tmp;

	switch (dev_get_driver_data(dev)) {
	case MCP23008:
	case MCP23S08:
		gpio_count = 8;
		break;
	case MCP23017:
	case MCP23018:
	case MCP23S17:
	case MCP23S18:
		gpio_count = 16;
		break;
	default:
		return -ENODEV;
	}

	switch (dev_get_driver_data(dev)) {
	case MCP23S08:
	case MCP23S17:
	case MCP23S18:
		info->dev_addr = dev_read_u32_default(dev, "addr", MCP230XX_ADDR);
		break;
	default:
		info->dev_addr = 0;
		break;
	}

	addr = dev_read_addr(dev);
	tmp = dev_read_prop(dev, "label", &size);
	if (tmp) {
		memcpy(label, tmp, sizeof(label) - 1);
		label[sizeof(label) - 1] = '\0';
		snprintf(name, sizeof(name), "%s@%x_", label, addr);
	} else {
		snprintf(name, sizeof(name), "gpio@%x_", addr);
	}

	str = strdup(name);
	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = gpio_count;

	dev_dbg(dev, "%s is ready\n", str);

	return 0;
}

static const struct udevice_id mcp230xx_ids[] = {
	/* i2c interface */
	{ .compatible = "microchip,mcp23008", .data = MCP23008, },
	{ .compatible = "microchip,mcp23017", .data = MCP23017, },
	{ .compatible = "microchip,mcp23018", .data = MCP23018, },
	/* spi interface */
	{ .compatible = "microchip,mcp23s08", .data = MCP23S08, },
	{ .compatible = "microchip,mcp23s17", .data = MCP23S17, },
	{ .compatible = "microchip,mcp23s18", .data = MCP23S18, },
	{ }
};

U_BOOT_DRIVER(mcp230xx) = {
	.name		= "mcp230xx",
	.id		= UCLASS_GPIO,
	.ops		= &mcp230xx_ops,
	.probe		= mcp230xx_probe,
	.plat_auto	= sizeof(struct mcp230xx_info),
	.of_match	= mcp230xx_ids,
};
