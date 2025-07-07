// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, Exfo Inc - All Rights Reserved
 *
 * Author: Anis CHALI <anis.chali@exfo.com>
 * inspired and adapted from linux driver of sx150x written by Gregory Bean
 * <gbean@codeaurora.org>
 */

#include <asm/gpio.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dt-bindings/gpio/gpio.h>
#include <i2c.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <log.h>
#include <power/regulator.h>
#include <regmap.h>

#define err(format, arg...) printf("ERR:" format "\n", ##arg)
#define dbg(format, arg...) printf("DBG:" format "\n", ##arg)

#define SX150X_PIN(_pin, _name) { .pin = _pin, .name = _name }

/* The chip models of sx150x */
enum {
	SX150X_123 = 0,
	SX150X_456,
	SX150X_789,
};

enum {
	SX150X_789_REG_MISC_AUTOCLEAR_OFF = 1 << 0,
	SX150X_MAX_REGISTER = 0xad,
	SX150X_IRQ_TYPE_EDGE_RISING = 0x1,
	SX150X_IRQ_TYPE_EDGE_FALLING = 0x2,
	SX150X_789_RESET_KEY1 = 0x12,
	SX150X_789_RESET_KEY2 = 0x34,
};

struct sx150x_123_pri {
	u8 reg_pld_mode;
	u8 reg_pld_table0;
	u8 reg_pld_table1;
	u8 reg_pld_table2;
	u8 reg_pld_table3;
	u8 reg_pld_table4;
	u8 reg_advanced;
};

struct sx150x_456_pri {
	u8 reg_pld_mode;
	u8 reg_pld_table0;
	u8 reg_pld_table1;
	u8 reg_pld_table2;
	u8 reg_pld_table3;
	u8 reg_pld_table4;
	u8 reg_advanced;
};

struct sx150x_789_pri {
	u8 reg_drain;
	u8 reg_polarity;
	u8 reg_clock;
	u8 reg_misc;
	u8 reg_reset;
	u8 ngpios;
};

struct sx150x_pin_desc {
	u32 pin;
	u8 *name;
};

struct sx150x_device_data {
	u8 model;
	u8 reg_pullup;
	u8 reg_pulldn;
	u8 reg_dir;
	u8 reg_data;
	u8 reg_irq_mask;
	u8 reg_irq_src;
	u8 reg_sense;
	u8 ngpios;
	union {
		struct sx150x_123_pri x123;
		struct sx150x_456_pri x456;
		struct sx150x_789_pri x789;
	} pri;
	const struct sx150x_pin_desc *pins;
	unsigned int npins;
};

struct sx150x_pinctrl_priv {
	char name[32];
	struct udevice *gpio;
	struct udevice *i2c;
	const struct sx150x_device_data *data;
};

static const struct sx150x_pin_desc sx150x_4_pins[] = {
	SX150X_PIN(0, "gpio0"), SX150X_PIN(1, "gpio1"), SX150X_PIN(2, "gpio2"),
	SX150X_PIN(3, "gpio3"), SX150X_PIN(4, "oscio"),
};

static const struct sx150x_pin_desc sx150x_8_pins[] = {
	SX150X_PIN(0, "gpio0"), SX150X_PIN(1, "gpio1"), SX150X_PIN(2, "gpio2"),
	SX150X_PIN(3, "gpio3"), SX150X_PIN(4, "gpio4"), SX150X_PIN(5, "gpio5"),
	SX150X_PIN(6, "gpio6"), SX150X_PIN(7, "gpio7"), SX150X_PIN(8, "oscio"),
};

static const struct sx150x_pin_desc sx150x_16_pins[] = {
	SX150X_PIN(0, "gpio0"),	  SX150X_PIN(1, "gpio1"),
	SX150X_PIN(2, "gpio2"),	  SX150X_PIN(3, "gpio3"),
	SX150X_PIN(4, "gpio4"),	  SX150X_PIN(5, "gpio5"),
	SX150X_PIN(6, "gpio6"),	  SX150X_PIN(7, "gpio7"),
	SX150X_PIN(8, "gpio8"),	  SX150X_PIN(9, "gpio9"),
	SX150X_PIN(10, "gpio10"), SX150X_PIN(11, "gpio11"),
	SX150X_PIN(12, "gpio12"), SX150X_PIN(13, "gpio13"),
	SX150X_PIN(14, "gpio14"), SX150X_PIN(15, "gpio15"),
	SX150X_PIN(16, "oscio"),
};

static const struct sx150x_device_data sx1501q_device_data = {
	.model = SX150X_123,
	.reg_pullup = 0x02,
	.reg_pulldn = 0x03,
	.reg_dir = 0x01,
	.reg_data = 0x00,
	.reg_irq_mask = 0x05,
	.reg_irq_src = 0x08,
	.reg_sense = 0x07,
	.pri.x123 = {
			.reg_pld_mode = 0x10,
			.reg_pld_table0 = 0x11,
			.reg_pld_table2 = 0x13,
			.reg_advanced = 0xad,
		},
	.ngpios = 4,
	.pins = sx150x_4_pins,
	.npins = 4, /* oscio not available */
};

static const struct sx150x_device_data sx1502q_device_data = {
	.model = SX150X_123,
	.reg_pullup = 0x02,
	.reg_pulldn = 0x03,
	.reg_dir = 0x01,
	.reg_data = 0x00,
	.reg_irq_mask = 0x05,
	.reg_irq_src = 0x08,
	.reg_sense = 0x06,
	.pri.x123 = {
			.reg_pld_mode = 0x10,
			.reg_pld_table0 = 0x11,
			.reg_pld_table1 = 0x12,
			.reg_pld_table2 = 0x13,
			.reg_pld_table3 = 0x14,
			.reg_pld_table4 = 0x15,
			.reg_advanced = 0xad,
		},
	.ngpios = 8,
	.pins = sx150x_8_pins,
	.npins = 8, /* oscio not available */
};

static const struct sx150x_device_data sx1503q_device_data = {
	.model = SX150X_123,
	.reg_pullup = 0x04,
	.reg_pulldn = 0x06,
	.reg_dir = 0x02,
	.reg_data = 0x00,
	.reg_irq_mask = 0x08,
	.reg_irq_src = 0x0e,
	.reg_sense = 0x0a,
	.pri.x123 = {
			.reg_pld_mode = 0x20,
			.reg_pld_table0 = 0x22,
			.reg_pld_table1 = 0x24,
			.reg_pld_table2 = 0x26,
			.reg_pld_table3 = 0x28,
			.reg_pld_table4 = 0x2a,
			.reg_advanced = 0xad,
		},
	.ngpios = 16,
	.pins = sx150x_16_pins,
	.npins = 16, /* oscio not available */
};

static const struct sx150x_device_data sx1504q_device_data = {
	.model = SX150X_456,
	.reg_pullup = 0x02,
	.reg_pulldn = 0x03,
	.reg_dir = 0x01,
	.reg_data = 0x00,
	.reg_irq_mask = 0x05,
	.reg_irq_src = 0x08,
	.reg_sense = 0x07,
	.pri.x456 = {
			.reg_pld_mode = 0x10,
			.reg_pld_table0 = 0x11,
			.reg_pld_table2 = 0x13,
		},
	.ngpios = 4,
	.pins = sx150x_4_pins,
	.npins = 4, /* oscio not available */
};

static const struct sx150x_device_data sx1505q_device_data = {
	.model = SX150X_456,
	.reg_pullup = 0x02,
	.reg_pulldn = 0x03,
	.reg_dir = 0x01,
	.reg_data = 0x00,
	.reg_irq_mask = 0x05,
	.reg_irq_src = 0x08,
	.reg_sense = 0x06,
	.pri.x456 = {
			.reg_pld_mode = 0x10,
			.reg_pld_table0 = 0x11,
			.reg_pld_table1 = 0x12,
			.reg_pld_table2 = 0x13,
			.reg_pld_table3 = 0x14,
			.reg_pld_table4 = 0x15,
		},
	.ngpios = 8,
	.pins = sx150x_8_pins,
	.npins = 8, /* oscio not available */
};

static const struct sx150x_device_data sx1506q_device_data = {
	.model = SX150X_456,
	.reg_pullup = 0x04,
	.reg_pulldn = 0x06,
	.reg_dir = 0x02,
	.reg_data = 0x00,
	.reg_irq_mask = 0x08,
	.reg_irq_src = 0x0e,
	.reg_sense = 0x0a,
	.pri.x456 = {
			.reg_pld_mode = 0x20,
			.reg_pld_table0 = 0x22,
			.reg_pld_table1 = 0x24,
			.reg_pld_table2 = 0x26,
			.reg_pld_table3 = 0x28,
			.reg_pld_table4 = 0x2a,
			.reg_advanced = 0xad,
		},
	.ngpios = 16,
	.pins = sx150x_16_pins,
	.npins = 16, /* oscio not available */
};

static const struct sx150x_device_data sx1507q_device_data = {
	.model = SX150X_789,
	.reg_pullup = 0x03,
	.reg_pulldn = 0x04,
	.reg_dir = 0x07,
	.reg_data = 0x08,
	.reg_irq_mask = 0x09,
	.reg_irq_src = 0x0b,
	.reg_sense = 0x0a,
	.pri.x789 = {
			.reg_drain = 0x05,
			.reg_polarity = 0x06,
			.reg_clock = 0x0d,
			.reg_misc = 0x0e,
			.reg_reset = 0x7d,
		},
	.ngpios = 4,
	.pins = sx150x_4_pins,
	.npins = ARRAY_SIZE(sx150x_4_pins),
};

static const struct sx150x_device_data sx1508q_device_data = {
	.model = SX150X_789,
	.reg_pullup = 0x03,
	.reg_pulldn = 0x04,
	.reg_dir = 0x07,
	.reg_data = 0x08,
	.reg_irq_mask = 0x09,
	.reg_irq_src = 0x0c,
	.reg_sense = 0x0a,
	.pri.x789 = {
			.reg_drain = 0x05,
			.reg_polarity = 0x06,
			.reg_clock = 0x0f,
			.reg_misc = 0x10,
			.reg_reset = 0x7d,
		},
	.ngpios = 8,
	.pins = sx150x_8_pins,
	.npins = ARRAY_SIZE(sx150x_8_pins),
};

static const struct sx150x_device_data sx1509q_device_data = {
	.model = SX150X_789,
	.reg_pullup = 0x06,
	.reg_pulldn = 0x08,
	.reg_dir = 0x0e,
	.reg_data = 0x10,
	.reg_irq_mask = 0x12,
	.reg_irq_src = 0x18,
	.reg_sense = 0x14,
	.pri.x789 = {
			.reg_drain = 0x0a,
			.reg_polarity = 0x0c,
			.reg_clock = 0x1e,
			.reg_misc = 0x1f,
			.reg_reset = 0x7d,
		},
	.ngpios = 16,
	.pins = sx150x_16_pins,
	.npins = ARRAY_SIZE(sx150x_16_pins),
};

static bool sx150x_pin_is_oscio(struct sx150x_pinctrl_priv *pctl,
				unsigned int pin)
{
	if (pin >= pctl->data->npins)
		return false;

	/* OSCIO pin is only present in 789 devices */
	if (pctl->data->model != SX150X_789)
		return false;

	return !strcmp(pctl->data->pins[pin].name, "oscio");
}

static int sx150x_reg_width(struct sx150x_pinctrl_priv *pctl, unsigned int reg)
{
	const struct sx150x_device_data *data = pctl->data;

	if (reg == data->reg_sense) {
		/*
		 * RegSense packs two bits of configuration per GPIO,
		 * so we'd need to read twice as many bits as there
		 * are GPIO in our chip
		 */
		return 2 * data->ngpios;
	} else if ((data->model == SX150X_789 &&
		    (reg == data->pri.x789.reg_misc ||
		     reg == data->pri.x789.reg_clock ||
		     reg == data->pri.x789.reg_reset)) ||
		   (data->model == SX150X_123 &&
		    reg == data->pri.x123.reg_advanced) ||
		   (data->model == SX150X_456 && data->pri.x456.reg_advanced &&
		    reg == data->pri.x456.reg_advanced)) {
		return 8;
	} else {
		return data->ngpios;
	}
}

static unsigned int sx150x_maybe_swizzle(struct sx150x_pinctrl_priv *pctl,
					 unsigned int reg, unsigned int val)
{
	unsigned int a, b;
	const struct sx150x_device_data *data = pctl->data;

	/*
	 * Whereas SX1509 presents RegSense in a simple layout as such:
	 *	reg     [ f f e e d d c c ]
	 *	reg  1 [ b b a a 9 9 8 8 ]
	 *	reg  2 [ 7 7 6 6 5 5 4 4 ]
	 *	reg  3 [ 3 3 2 2 1 1 0 0 ]
	 *
	 * SX1503 and SX1506 deviate from that data layout, instead storing
	 * their contents as follows:
	 *
	 *	reg     [ f f e e d d c c ]
	 *	reg  1 [ 7 7 6 6 5 5 4 4 ]
	 *	reg  2 [ b b a a 9 9 8 8 ]
	 *	reg  3 [ 3 3 2 2 1 1 0 0 ]
	 *
	 * so, taking that into account, we swap two
	 * inner bytes of a 4-byte result
	 */

	if (reg == data->reg_sense && data->ngpios == 16 &&
	    (data->model == SX150X_123 || data->model == SX150X_456)) {
		a = val & 0x00ff0000;
		b = val & 0x0000ff00;

		val &= 0xff0000ff;
		val |= b << 8;
		val |= a >> 8;
	}

	return val;
}

/*
 * In order to mask the differences between 16 and 8 bit expander
 * devices we set up a sligthly ficticious regmap that pretends to be
 * a set of 32-bit (to accommodate RegSenseLow/RegSenseHigh
 * pair/quartet) registers and transparently reconstructs those
 * registers via multiple I2C/SMBus reads
 *
 * This way the rest of the driver code, interfacing with the chip via
 * regmap API, can work assuming that each GPIO pin is represented by
 * a group of bits at an offset proportional to GPIO number within a
 * given register.
 */
static int sx150x_reg_read(struct sx150x_pinctrl_priv *pctl, unsigned int reg,
			   unsigned int *result)
{
	int ret, n;
	const int width = sx150x_reg_width(pctl, reg);
	unsigned int idx, val;

	/*
	 * There are four potential cases covered by this function:
	 *
	 * 1) 8-pin chip, single configuration bit register
	 *
	 *	This is trivial the code below just needs to read:
	 *		reg  [ 7 6 5 4 3 2 1 0 ]
	 *
	 * 2) 8-pin chip, double configuration bit register (RegSense)
	 *
	 *	The read will be done as follows:
	 *		reg      [ 7 7 6 6 5 5 4 4 ]
	 *		reg  1  [ 3 3 2 2 1 1 0 0 ]
	 *
	 * 3) 16-pin chip, single configuration bit register
	 *
	 *	The read will be done as follows:
	 *		reg     [ f e d c b a 9 8 ]
	 *		reg  1 [ 7 6 5 4 3 2 1 0 ]
	 *
	 * 4) 16-pin chip, double configuration bit register (RegSense)
	 *
	 *	The read will be done as follows:
	 *		reg     [ f f e e d d c c ]
	 *		reg  1 [ b b a a 9 9 8 8 ]
	 *		reg  2 [ 7 7 6 6 5 5 4 4 ]
	 *		reg  3 [ 3 3 2 2 1 1 0 0 ]
	 */

	for (n = width, val = 0, idx = reg; n > 0; n -= 8, idx) {
		val <<= 8;

		ret = dm_i2c_reg_read(pctl->i2c, idx);
		if (ret < 0)
			return ret;

		val |= ret;
	}

	*result = sx150x_maybe_swizzle(pctl, reg, val);

	return 0;
}

static int sx150x_reg_write(struct sx150x_pinctrl_priv *pctl, unsigned int reg,
			    unsigned int val)
{
	int ret, n;
	const int width = sx150x_reg_width(pctl, reg);

	val = sx150x_maybe_swizzle(pctl, reg, val);

	n = (width - 1) & ~7;
	do {
		const u8 byte = (val >> n) & 0xff;

		ret = dm_i2c_reg_write(pctl->i2c, reg, byte);
		if (ret < 0)
			return ret;

		reg;
		n -= 8;
	} while (n >= 0);

	return 0;
}

static unsigned int sx150x_read(struct sx150x_pinctrl_priv *pctl, uint reg)
{
	int ret;
	unsigned int res;

	ret = sx150x_reg_read(pctl, reg, &res);
	if (ret) {
		err("%s: failed to read reg(%x) with %d", pctl->name, reg, ret);
		return ret;
	}

	return res;
}

static int sx150x_write(struct sx150x_pinctrl_priv *pctl, uint reg, uint val)
{
	return sx150x_reg_write(pctl, reg, val);
}

static int sx150x_write_bits(struct sx150x_pinctrl_priv *pctl, uint reg,
			     uint mask, uint val)
{
	int orig, tmp;

	orig = sx150x_read(pctl, reg);
	if (orig < 0)
		return orig;

	tmp = orig & ~mask;
	tmp |= val & mask;

	return sx150x_write(pctl, reg, tmp);
}

static int sx150x_reset(struct udevice *dev)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev);
	int err;

	err = sx150x_write(pctl, pctl->data->pri.x789.reg_reset,
			   SX150X_789_RESET_KEY1);
	if (err < 0)
		return err;

	err = sx150x_write(pctl, pctl->data->pri.x789.reg_reset,
			   SX150X_789_RESET_KEY2);
	return err;
}

static int sx150x_init_misc(struct udevice *dev)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev);
	u8 reg, value;

	switch (pctl->data->model) {
	case SX150X_789:
		reg = pctl->data->pri.x789.reg_misc;
		value = 0x0;
		break;
	case SX150X_456:
		reg = pctl->data->pri.x456.reg_advanced;
		value = 0x00;

		/*
		 * Only SX1506 has RegAdvanced, SX1504/5 are expected
		 * to initialize this offset to zero
		 */
		if (!reg)
			return 0;
		break;
	case SX150X_123:
		reg = pctl->data->pri.x123.reg_advanced;
		value = 0x00;
		break;
	default:
		WARN(1, "Unknown chip model %d\n", pctl->data->model);
		return -EINVAL;
	}

	return sx150x_write(pctl, reg, value);
}

static int sx150x_init_hw(struct udevice *dev)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev);
	const u8 reg[] = {
		[SX150X_789] = pctl->data->pri.x789.reg_polarity,
		[SX150X_456] = pctl->data->pri.x456.reg_pld_mode,
		[SX150X_123] = pctl->data->pri.x123.reg_pld_mode,
	};
	int err;

	if (pctl->data->model == SX150X_789 &&
	    dev_read_bool(dev, "semtech,probe-reset")) {
		err = sx150x_reset(dev);
		if (err < 0)
			return err;
	}

	err = sx150x_init_misc(dev);
	if (err < 0)
		return err;

	/* Set all pins to work in normal mode */
	return sx150x_write(pctl, reg[pctl->data->model], 0);
}

static int sx150x_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev->parent);

	if (sx150x_pin_is_oscio(pctl, offset))
		return -EINVAL;

	int val = sx150x_read(pctl, pctl->data->reg_data);

	return !!(val & BIT(offset));
}

static int sx150x_gpio_set(struct udevice *dev, unsigned int offset, int value)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev->parent);

	return sx150x_write_bits(pctl, pctl->data->reg_data, BIT(offset),
				 value ? BIT(offset) : 0);
}

static int sx150x_gpio_oscio_set(struct udevice *dev, int value)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev->parent);

	return sx150x_write(pctl, pctl->data->pri.x789.reg_clock,
			    (value ? 0x1f : 0x10));
}

static int sx150x_gpio_set_value(struct udevice *dev, unsigned int offset,
				 int value)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev->parent);

	if (sx150x_pin_is_oscio(pctl, offset))
		sx150x_gpio_oscio_set(dev->parent, value);
	else
		sx150x_gpio_set(dev->parent, offset, value);

	return 0;
}

static int sx150x_gpio_get_direction(struct udevice *dev, unsigned int offset)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev->parent);
	int val;

	if (sx150x_pin_is_oscio(pctl, offset))
		return GPIOF_OUTPUT;

	val = sx150x_read(pctl, pctl->data->reg_data);
	if (val < 0)
		return val;

	if (val & BIT(offset))
		return GPIOF_INPUT;

	return GPIOF_OUTPUT;
}

static int sx150x_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev->parent);

	if (sx150x_pin_is_oscio(pctl, offset))
		return -EINVAL;

	return sx150x_write_bits(pctl, pctl->data->reg_dir, BIT(offset),
				 BIT(offset));
}

static int sx150x_gpio_direction_output(struct udevice *dev,
					unsigned int offset, int value)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev->parent);
	int ret;

	if (sx150x_pin_is_oscio(pctl, offset))
		return sx150x_gpio_oscio_set(dev, value);

	ret = sx150x_write_bits(pctl, pctl->data->reg_dir, BIT(offset), 0);
	if (ret < 0)
		return ret;

	return sx150x_gpio_set(dev, offset, value);
}

static int sx150x_gpio_probe(struct udevice *dev)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev->parent);
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = pctl->name;
	uc_priv->gpio_count = pctl->data->ngpios;

	return 0;
}

static struct dm_gpio_ops sx150x_gpio_ops = {
	.get_value = sx150x_gpio_get_value,
	.set_value = sx150x_gpio_set_value,
	.get_function = sx150x_gpio_get_direction,
	.direction_input = sx150x_gpio_direction_input,
	.direction_output = sx150x_gpio_direction_output,
};

static struct driver sx150x_gpio_driver = {
	.name = "sx150x-gpio",
	.id = UCLASS_GPIO,
	.probe = sx150x_gpio_probe,
	.ops = &sx150x_gpio_ops,
};

static const struct udevice_id sx150x_pinctrl_of_match[] = {
	{ .compatible = "semtech,sx1501q",
	  .data = (ulong)&sx1501q_device_data },
	{ .compatible = "semtech,sx1502q",
	  .data = (ulong)&sx1502q_device_data },
	{ .compatible = "semtech,sx1503q",
	  .data = (ulong)&sx1503q_device_data },
	{ .compatible = "semtech,sx1504q",
	  .data = (ulong)&sx1504q_device_data },
	{ .compatible = "semtech,sx1505q",
	  .data = (ulong)&sx1505q_device_data },
	{ .compatible = "semtech,sx1506q",
	  .data = (ulong)&sx1506q_device_data },
	{ .compatible = "semtech,sx1507q",
	  .data = (ulong)&sx1507q_device_data },
	{ .compatible = "semtech,sx1508q",
	  .data = (ulong)&sx1508q_device_data },
	{ .compatible = "semtech,sx1509q",
	  .data = (ulong)&sx1509q_device_data },
	{},
};

static const struct pinconf_param sx150x_conf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 0 },
	{ "drive-push-pull", PIN_CONFIG_DRIVE_PUSH_PULL, 0 },
	{ "output", PIN_CONFIG_OUTPUT, 0 },
};

static int sx150x_pinctrl_get_pins_count(struct udevice *dev)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev);

	return pctl->data->ngpios;
}

static const char *sx150x_pinctrl_get_pin_name(struct udevice *dev,
					       unsigned int selector)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev);
	static char pin_name[PINNAME_SIZE];

	snprintf(pin_name, PINNAME_SIZE, "%s", pctl->data->pins[selector].name);
	return pin_name;
}

static int sx150x_pinctrl_conf_set(struct udevice *dev, unsigned int pin,
				   unsigned int param, unsigned int arg)
{
	int ret;
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev);

	if (sx150x_pin_is_oscio(pctl, pin)) {
		if (param == PIN_CONFIG_OUTPUT) {
			ret = sx150x_gpio_direction_output(pctl->gpio, pin,
							   arg);
			if (ret < 0)
				return ret;
		} else {
			return -EOPNOTSUPP;
		}
	}

	switch (param) {
	case PIN_CONFIG_BIAS_PULL_PIN_DEFAULT:
	case PIN_CONFIG_BIAS_DISABLE:
		ret = sx150x_write_bits(pctl, pctl->data->reg_pulldn, BIT(pin),
					0);
		if (ret < 0)
			return ret;

		ret = sx150x_write_bits(pctl, pctl->data->reg_pullup, BIT(pin),
					0);
		if (ret < 0)
			return ret;
		break;

	case PIN_CONFIG_BIAS_PULL_UP:
		ret = sx150x_write_bits(pctl, pctl->data->reg_pullup, BIT(pin),
					BIT(pin));
		if (ret < 0)
			return ret;

		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		ret = sx150x_write_bits(pctl, pctl->data->reg_pulldn, BIT(pin),
					BIT(pin));
		if (ret < 0)
			return ret;
		break;

	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		if (pctl->data->model != SX150X_789 ||
		    sx150x_pin_is_oscio(pctl, pin))
			return -EOPNOTSUPP;

		ret = sx150x_write_bits(pctl, pctl->data->pri.x789.reg_drain,
					BIT(pin), BIT(pin));
		if (ret < 0)
			return ret;

		break;

	case PIN_CONFIG_DRIVE_PUSH_PULL:
		if (pctl->data->model != SX150X_789 ||
		    sx150x_pin_is_oscio(pctl, pin))
			return 0;

		ret = sx150x_write_bits(pctl, pctl->data->pri.x789.reg_drain,
					BIT(pin), 0);
		if (ret < 0)
			return ret;

		break;

	case PIN_CONFIG_OUTPUT:
		ret = sx150x_gpio_direction_output(pctl->gpio, pin, arg);
		if (ret < 0)
			return ret;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int sx150x_pinctrl_bind(struct udevice *dev)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_plat(dev);
	int ret, reg;

	if (!dev_read_bool(dev, "gpio-controller"))
		return 0;

	reg = (int)dev_read_addr_ptr(dev);

	ret = device_bind(dev, &sx150x_gpio_driver, dev_read_name(dev), NULL,
			  dev_ofnode(dev), &pctl->gpio);
	if (ret)
		return ret;

	return 0;
}

static int sx150x_pinctrl_probe(struct udevice *dev)
{
	struct sx150x_pinctrl_priv *pctl = dev_get_priv(dev);
	const struct sx150x_device_data *drv_data =
		(const struct sx150x_device_data *)dev_get_driver_data(dev);
	int ret, reg;

	if (!drv_data)
		return -ENOENT;

	pctl->data = drv_data;

	reg = (int)dev_read_addr_ptr(dev);
	ret = dm_i2c_probe(dev->parent, reg, 0, &pctl->i2c);
	if (ret) {
		err("Cannot find I2C chip %02x (%d)", reg, ret);
		return ret;
	}

	ret = sx150x_init_hw(dev);
	if (ret) {
		err("Cannot initialize GPIO expander at %02x with %d", reg,
		    ret);
		return ret;
	}

	snprintf(pctl->name, 32, "gpio-ext@%x_", reg);

	return 0;
}

static struct pinctrl_ops sx150x_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_pins_count = sx150x_pinctrl_get_pins_count,
	.get_pin_name = sx150x_pinctrl_get_pin_name,
#if CONFIG_IS_ENABLED(PINCONF)
	.pinconf_set = sx150x_pinctrl_conf_set,
	.pinconf_num_params = ARRAY_SIZE(sx150x_conf_params),
	.pinconf_params = sx150x_conf_params,
#endif
};

U_BOOT_DRIVER(sx150x_pinctrl) = {
	.name = "sx150x-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = sx150x_pinctrl_of_match,
	.priv_auto = sizeof(struct sx150x_pinctrl_priv),
	.ops = &sx150x_pinctrl_ops,
	.probe = sx150x_pinctrl_probe,
	.bind = sx150x_pinctrl_bind,
};
