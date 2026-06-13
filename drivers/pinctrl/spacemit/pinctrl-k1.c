// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 Yixun Lan <dlan@gentoo.org>
 * Copyright (c) 2025-2026 RISCstar Ltd.
 */

#include <clk.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/io.h>

/*
 * +---------+----------+-----------+--------+--------+----------+--------+
 * |   pull  |   drive  | schmitter |  slew  |  edge  |  strong  |   mux  |
 * | up/down | strength |  trigger  |  rate  | detect |   pull   |  mode  |
 * +---------+----------+-----------+--------+--------+----------+--------+
 *   3 bits     3 bits     2 bits     1 bit    3 bits     1 bit    3 bits
 */

#define PAD_MUX			GENMASK(2, 0)
#define PAD_STRONG_PULL		BIT(3)
#define PAD_EDGE_RISE		BIT(4)
#define PAD_EDGE_FALL		BIT(5)
#define PAD_EDGE_CLEAR		BIT(6)
#define PAD_SLEW_RATE		GENMASK(12, 11)
#define PAD_SLEW_RATE_EN	BIT(7)
#define PAD_SCHMITT		GENMASK(9, 8)
#define PAD_DRIVE		GENMASK(12, 10)
#define PAD_PULLDOWN		BIT(13)
#define PAD_PULLUP		BIT(14)
#define PAD_PULL_EN		BIT(15)

#define PIN_POWER_STATE_1V8		1800
#define PIN_POWER_STATE_3V3		3300

enum spacemit_pin_io_type {
	IO_TYPE_NONE = 0,
	IO_TYPE_1V8,
	IO_TYPE_3V3,
	IO_TYPE_EXTERNAL,
};

struct spacemit_pin_io {
	unsigned int	pin : 12;	// 0~4095
	unsigned int	io_type : 4;	// 0~15
	unsigned int	ds : 8;		// 0~255
	unsigned int	reserved : 8;
};

struct spacemit_pinctrl_data {
	struct spacemit_pin_io *io_pins;
	int nr_io_pins;

	void __iomem * (*pin_to_reg)(struct udevice *dev, unsigned int pin);
	int (*get_gpio_mux)(struct udevice *dev, unsigned int pin);
	int (*get_pins)(struct udevice *dev);
	int (*get_functions)(struct udevice *dev);
	int (*get_io_type)(struct udevice *dev, unsigned int pin);
};

struct spacemit_pinctrl_priv {
	void __iomem		*regs;
	struct spacemit_pin_io	*io_pins;
	int			nr_io_pins;
};

struct spacemit_pin_mux_config {
	const struct spacemit_pin	*pin;
	u32				config;
};

struct spacemit_pin_drv_strength {
	unsigned int	val : 8;
	unsigned int	ma : 16;
	unsigned int	reserved : 8;
};

static char pin_name[PINNAME_SIZE];

/* External: IO voltage via external source, can be 1.8V or 3.3V */
static struct spacemit_pin_io k1_io_pins[] = {
	{ 47, IO_TYPE_EXTERNAL, 0, },
	{ 48, IO_TYPE_EXTERNAL, 0, },
	{ 49, IO_TYPE_EXTERNAL, 0, },
	{ 50, IO_TYPE_EXTERNAL, 0, },
	{ 51, IO_TYPE_EXTERNAL, 0, },
	{ 52, IO_TYPE_EXTERNAL, 0, },
	{ 75, IO_TYPE_EXTERNAL, 0, },
	{ 76, IO_TYPE_EXTERNAL, 0, },
	{ 77, IO_TYPE_EXTERNAL, 0, },
	{ 78, IO_TYPE_EXTERNAL, 0, },
	{ 79, IO_TYPE_EXTERNAL, 0, },
	{ 80, IO_TYPE_EXTERNAL, 0, },
	{ 98, IO_TYPE_EXTERNAL, 0, },
	{ 99, IO_TYPE_EXTERNAL, 0, },
	{ 100, IO_TYPE_EXTERNAL, 0, },
	{ 101, IO_TYPE_EXTERNAL, 0, },
	{ 102, IO_TYPE_EXTERNAL, 0, },
	{ 103, IO_TYPE_EXTERNAL, 0, },
	{ 104, IO_TYPE_EXTERNAL, 0, },
	{ 105, IO_TYPE_EXTERNAL, 0, },
	{ 106, IO_TYPE_EXTERNAL, 0, },
	{ 107, IO_TYPE_EXTERNAL, 0, },
	{ 108, IO_TYPE_EXTERNAL, 0, },
	{ 109, IO_TYPE_EXTERNAL, 0, },
};

static inline int k1_get_pins(struct udevice *dev)
{
	return 128;
}

static inline int k1_get_functions(struct udevice *dev)
{
	return 7;
}

// The pin number equals to the gpio number.
static void __iomem *k1_pin_to_reg(struct udevice *dev, unsigned int pin)
{
	struct spacemit_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned int offset = 1;

	if (pin < 86) {
		offset += pin;
	} else if (pin < 93) {
		offset += pin + 36;
	} else if (pin < 98) {
		offset += pin + 23;
	} else if (pin == 98) {		// QSPI_DAT3
		offset += 92;
	} else if (pin == 99) {		// QSPI_DAT2
		offset += 91;
	} else if (pin == 100) {	// QSPI_DAT1
		offset += 90;
	} else if (pin == 101) {	// QSPI_DAT0
		offset += 89;
	} else if (pin == 102) {	// QSPI_CLK
		offset += 94;
	} else if (pin == 103) {	// QSPI_CS1
		offset += 93;
	} else if (pin < 111) {
		offset += pin + 5;
	} else if (pin < 128) {
		offset += pin + 19;
	} else {
		dev_err(dev, "Invalid pin (%u)\n", pin);
		return NULL;
	}
	return priv->regs + (offset << 2);
}

static int k1_get_gpio_mux(struct udevice *dev, unsigned int selector)
{
	u32 mux = 0;

	if (selector < 70) {
		mux = 0;
	} else if (selector < 74) {
		mux = 1;
	} else if (selector < 93) {
		mux = 0;
	} else if (selector < 104) {
		mux = 1;
	} else if (selector < 110) {
		mux = 4;
	} else if (selector < 128) {
		mux = 0;
	} else {
		dev_err(dev, "Invalid pin (%u)\n", selector);
		return -EINVAL;
	}
	return mux;
}

static int k1_get_io_type(struct udevice *dev, unsigned int selector)
{
	if (selector < 47)
		return IO_TYPE_1V8;
	else if (selector < 53)
		return IO_TYPE_EXTERNAL;
	else if (selector < 75)
		return IO_TYPE_1V8;
	else if (selector < 81)
		return IO_TYPE_EXTERNAL;
	else if (selector < 98)
		return IO_TYPE_1V8;
	else if (selector < 110)
		return IO_TYPE_EXTERNAL;
	else if (selector < 128)
		return IO_TYPE_1V8;
	return -EINVAL;
}

/* use IO high level output current as the table */
static struct spacemit_pin_drv_strength spacemit_ds_1v8_tbl[4] = {
	{ 0, 11 },
	{ 2, 21 },
	{ 4, 32 },
	{ 6, 42 },
};

static struct spacemit_pin_drv_strength spacemit_ds_3v3_tbl[8] = {
	{ 0,  7 },
	{ 2, 10 },
	{ 4, 13 },
	{ 6, 16 },
	{ 1, 19 },
	{ 3, 23 },
	{ 5, 26 },
	{ 7, 29 },
};

static inline u8 spacemit_get_ds_value(struct spacemit_pin_drv_strength *tbl,
				       u32 num, u32 ma)
{
	int i;

	for (i = 0; i < num; i++)
		if (ma <= tbl[i].ma)
			return tbl[i].val;

	return tbl[num - 1].val;
}

static inline u32 spacemit_get_ds_ma(struct spacemit_pin_drv_strength *tbl,
				     u32 num, u32 val)
{
	int i;

	for (i = 0; i < num; i++)
		if (val == tbl[i].val)
			return tbl[i].ma;

	return 0;
}

static inline u8 spacemit_get_drive_strength(enum spacemit_pin_io_type type,
					     u32 ma)
{
	switch (type) {
	case IO_TYPE_1V8:
		return spacemit_get_ds_value(spacemit_ds_1v8_tbl,
					     ARRAY_SIZE(spacemit_ds_1v8_tbl),
					     ma);
	case IO_TYPE_3V3:
		return spacemit_get_ds_value(spacemit_ds_3v3_tbl,
					     ARRAY_SIZE(spacemit_ds_3v3_tbl),
					     ma);
	default:
		return 0;
	}
}

static inline u32 spacemit_get_drive_strength_ma(enum spacemit_pin_io_type type,
						 u32 value)
{
	switch (type) {
	case IO_TYPE_1V8:
		return spacemit_get_ds_ma(spacemit_ds_1v8_tbl,
					  ARRAY_SIZE(spacemit_ds_1v8_tbl),
					  value & 0x6);
	case IO_TYPE_3V3:
		return spacemit_get_ds_ma(spacemit_ds_3v3_tbl,
					  ARRAY_SIZE(spacemit_ds_3v3_tbl),
					  value);
	default:
		return 0;
	}
}

static inline u16 spacemit_dt_get_pin(u32 value)
{
	return value >> 16;
}

static inline u16 spacemit_dt_get_pin_mux(u32 value)
{
	return value & GENMASK(15, 0);
}

static int spacemit_get_pins_count(struct udevice *dev)
{
	struct spacemit_pinctrl_data *data;

	data = (struct spacemit_pinctrl_data *)dev_get_driver_data(dev);
	if (!data || !data->get_pins)
		return -EINVAL;
	return data->get_pins(dev);
}

static const char *spacemit_get_pin_name(struct udevice *dev,
					 unsigned int selector)
{
	struct spacemit_pinctrl_data *data;
	unsigned int npins;

	data = (struct spacemit_pinctrl_data *)dev_get_driver_data(dev);
	if (!data || !data->get_pins)
		return NULL;
	npins = data->get_pins(dev);

	if (selector >= npins)
		snprintf(pin_name, PINNAME_SIZE, "Error");
	else
		snprintf(pin_name, PINNAME_SIZE, "PIN%u", selector);

	return pin_name;
}

static int spacemit_get_functions_count(struct udevice *dev)
{
	struct spacemit_pinctrl_data *data;

	data = (struct spacemit_pinctrl_data *)dev_get_driver_data(dev);
	if (!data || !data->get_functions)
		return -EINVAL;
	return data->get_functions(dev);
}

static int spacemit_get_pin_muxing(struct udevice *dev, unsigned int pin,
				   char *buf, int size)
{
	struct spacemit_pinctrl_data *data;
	void __iomem *addr;
	u32 mux, val;

	data = (struct spacemit_pinctrl_data *)dev_get_driver_data(dev);
	if (!data || !data->pin_to_reg)
		return -EINVAL;

	addr = data->pin_to_reg(dev, pin);
	if (!addr)
		return -EINVAL;

	val = readl(addr);
	mux = val & PAD_MUX;
	snprintf(buf, size, "[%p] 0x%08x MUX%d", addr, val, mux);
	return 0;
}

static int spacemit_pinctrl_request_gpio(struct udevice *dev,
					 unsigned int selector)
{
	struct spacemit_pinctrl_data *data;
	void __iomem *addr;
	int mux;

	data = (struct spacemit_pinctrl_data *)dev_get_driver_data(dev);
	if (!data || !data->pin_to_reg || !data->get_gpio_mux)
		return -EINVAL;
	addr = data->pin_to_reg(dev, selector);
	mux = data->get_gpio_mux(dev, selector);
	if (mux < 0) {
		dev_err(dev, "Invalid pin (%d)\n", selector);
		return -EINVAL;
	}
	clrsetbits_le32(addr, PAD_MUX, mux & PAD_MUX);
	return 0;
}

static int spacemit_pinctrl_free_gpio(struct udevice *dev,
				      unsigned int selector)
{
	return 0;
}

static int spacemit_pinmux_set(struct udevice *dev, unsigned int pin,
			       unsigned int mux)
{
	struct spacemit_pinctrl_data *data;
	void __iomem *addr;

	data = (struct spacemit_pinctrl_data *)dev_get_driver_data(dev);
	if (!data || !data->pin_to_reg)
		return -EINVAL;
	addr = data->pin_to_reg(dev, pin);
	clrsetbits_le32(addr, PAD_MUX, mux & PAD_MUX);
	return 0;
}

static int spacemit_pinmux_property_set(struct udevice *dev, u32 pinmux_group)
{
	u32 pin, mux;

	pin = spacemit_dt_get_pin(pinmux_group);
	mux = spacemit_dt_get_pin_mux(pinmux_group);
	return spacemit_pinmux_set(dev, pin, mux);
}

static const struct pinconf_param spacemit_pinconf_params[] = {
	{ "bias-disable",	PIN_CONFIG_BIAS_DISABLE,	0 },
	{ "bias-pull-down",	PIN_CONFIG_BIAS_PULL_DOWN,	1 },
	{ "bias-pull-up",	PIN_CONFIG_BIAS_PULL_UP,	1 },
	{ "drive-strength",	PIN_CONFIG_DRIVE_STRENGTH,	U32_MAX },
	{ "power-source",	PIN_CONFIG_POWER_SOURCE,	U32_MAX },
};

static int spacemit_pinconf_set(struct udevice *dev, unsigned int pin_selector,
				unsigned int param, unsigned int argument)
{
	struct spacemit_pinctrl_data *data;
	struct spacemit_pinctrl_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u32 mask = 0;
	unsigned int io_type;
	u8 ds;
	bool found;
	int i;

	data = (struct spacemit_pinctrl_data *)dev_get_driver_data(dev);
	if (!data || !data->pin_to_reg)
		return -EINVAL;
	addr = data->pin_to_reg(dev, pin_selector);
	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		clrbits_le32(addr, PAD_PULLUP | PAD_PULLDOWN | PAD_PULL_EN);
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		mask = PAD_PULLDOWN | PAD_PULLUP | PAD_PULL_EN;
		clrsetbits_le32(addr, mask, PAD_PULLDOWN | PAD_PULL_EN);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		mask = PAD_PULLDOWN | PAD_PULLUP | PAD_PULL_EN;
		clrsetbits_le32(addr, mask, PAD_PULLUP | PAD_PULL_EN);
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		io_type = IO_TYPE_1V8;
		for (i = 0; i < priv->nr_io_pins; i++) {
			if (priv->io_pins[i].pin != pin_selector)
				continue;
			io_type = priv->io_pins[i].io_type;
			break;
		}
		if (io_type != IO_TYPE_3V3 && io_type != IO_TYPE_1V8) {
			dev_err(dev, "Invalid IO type (%d)\n", io_type);
			return -EINVAL;
		}
		ds = spacemit_get_drive_strength(io_type, argument);
		clrsetbits_le32(addr, PAD_DRIVE, ds);
		break;
	case PIN_CONFIG_POWER_SOURCE:
		for (i = 0, found = false; i < priv->nr_io_pins; i++) {
			if (priv->io_pins[i].pin != pin_selector)
				continue;
			if (argument == PIN_POWER_STATE_3V3) {
				priv->io_pins[i].io_type = IO_TYPE_3V3;
				found = true;
			} else if (argument == PIN_POWER_STATE_1V8) {
				priv->io_pins[i].io_type = IO_TYPE_1V8;
				found = true;
			}
			break;
		}
		if (!found && argument != PIN_POWER_STATE_1V8) {
			dev_err(dev, "Invalid power source (%d)\n", argument);
			return -EINVAL;
		}
		break;
	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

static int spacemit_pinctrl_probe(struct udevice *dev)
{
	struct spacemit_pinctrl_data *data;
	struct spacemit_pinctrl_priv *priv;
	struct clk_bulk clks;
	size_t size;
	int ret;

	data = (struct spacemit_pinctrl_data *)dev_get_driver_data(dev);
	priv = dev_get_priv(dev);
	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs) {
		dev_err(dev, "Fail to get base address\n");
		return -EINVAL;
	}
	priv->nr_io_pins = data->nr_io_pins;
	size = priv->nr_io_pins * sizeof(struct spacemit_pin_io);
	priv->io_pins = (struct spacemit_pin_io *)memdup(data->io_pins, size);
	if (!priv->io_pins) {
		dev_err(dev, "Fail to allocate memory\n");
		return -ENOMEM;
	}

	ret = clk_get_bulk(dev, &clks);
	if (ret) {
		dev_err(dev, "Fail to get bulk clks\n");
		goto out_get_clk;
	}
	ret = clk_enable_bulk(&clks);
	if (ret) {
		dev_err(dev, "Fail to enable bulk clks\n");
		goto out_clks;
	}
	return 0;
out_clks:
	clk_release_bulk(&clks);
out_get_clk:
	free(priv->io_pins);
	return ret;
}

static const struct spacemit_pinctrl_data k1_pinctrl_data = {
	.io_pins	= k1_io_pins,
	.nr_io_pins	= ARRAY_SIZE(k1_io_pins),
	.pin_to_reg	= k1_pin_to_reg,
	.get_gpio_mux	= k1_get_gpio_mux,
	.get_pins	= k1_get_pins,
	.get_functions	= k1_get_functions,
	.get_io_type	= k1_get_io_type,
};

static const struct udevice_id spacemit_pinctrl_ids[] = {
	{
		.compatible = "spacemit,k1-pinctrl",
		.data = (uintptr_t)&k1_pinctrl_data,
	}, { /* sentinel */ }
};

static const struct pinctrl_ops spacemit_pinctrl_ops = {
	.get_pins_count		= spacemit_get_pins_count,
	.get_pin_name		= spacemit_get_pin_name,
	.get_functions_count	= spacemit_get_functions_count,
	.get_pin_muxing		= spacemit_get_pin_muxing,
	.set_state		= pinctrl_generic_set_state,
	.gpio_request_enable	= spacemit_pinctrl_request_gpio,
	.gpio_disable_free	= spacemit_pinctrl_free_gpio,
	.pinmux_set		= spacemit_pinmux_set,
	.pinmux_property_set	= spacemit_pinmux_property_set,
	.pinconf_num_params	= ARRAY_SIZE(spacemit_pinconf_params),
	.pinconf_params		= spacemit_pinconf_params,
	.pinconf_set		= spacemit_pinconf_set,
};

U_BOOT_DRIVER(spacemit_pinctrl) = {
	.name		= "spacemit_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= spacemit_pinctrl_ids,
	.ops		= &spacemit_pinctrl_ops,
	.priv_auto	= sizeof(struct spacemit_pinctrl_priv),
	.probe		= spacemit_pinctrl_probe,
};
