// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) EETS GmbH, 2017, Felix Brack <f.brack@eets.ch>
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/libfdt.h>
#include <asm/io.h>

/**
 * struct single_pdata - platform data
 * @base: first configuration register
 * @offset: index of last configuration register
 * @mask: configuration-value mask bits
 * @width: configuration register bit width
 * @bits_per_mux: true if one register controls more than one pin
 */
struct single_pdata {
	fdt_addr_t base;
	int offset;
	u32 mask;
	u32 width;
	bool bits_per_mux;
};

/**
 * struct single_priv - private data
 * @bits_per_pin: number of bits per pin
 * @npins: number of selectable pins
 * @pin_name: temporary buffer to store the pin name
 */
struct single_priv {
	unsigned int bits_per_pin;
	unsigned int npins;
	char pin_name[PINNAME_SIZE];
};

/**
 * struct single_fdt_pin_cfg - pin configuration
 *
 * This structure is used for the pin configuration parameters in case
 * the register controls only one pin.
 *
 * @reg: configuration register offset
 * @val: configuration register value
 */
struct single_fdt_pin_cfg {
	fdt32_t reg;
	fdt32_t val;
};

/**
 * struct single_fdt_bits_cfg - pin configuration
 *
 * This structure is used for the pin configuration parameters in case
 * the register controls more than one pin.
 *
 * @reg: configuration register offset
 * @val: configuration register value
 * @mask: configuration register mask
 */
struct single_fdt_bits_cfg {
	fdt32_t reg;
	fdt32_t val;
	fdt32_t mask;
};

static unsigned int single_read(struct udevice *dev, fdt_addr_t reg)
{
	struct single_pdata *pdata = dev_get_plat(dev);

	switch (pdata->width) {
	case 8:
		return readb(reg);
	case 16:
		return readw(reg);
	default: /* 32 bits */
		return readl(reg);
	}

	return readb(reg);
}

static void single_write(struct udevice *dev, unsigned int val, fdt_addr_t reg)
{
	struct single_pdata *pdata = dev_get_plat(dev);

	switch (pdata->width) {
	case 8:
		writeb(val, reg);
		break;
	case 16:
		writew(val, reg);
		break;
	default: /* 32 bits */
		writel(val, reg);
	}
}

/**
 * single_configure_pins() - Configure pins based on FDT data
 *
 * @dev: Pointer to single pin configuration device which is the parent of
 *       the pins node holding the pin configuration data.
 * @pins: Pointer to the first element of an array of register/value pairs
 *        of type 'struct single_fdt_pin_cfg'. Each such pair describes the
 *        the pin to be configured and the value to be used for configuration.
 *        This pointer points to a 'pinctrl-single,pins' property in the
 *        device-tree.
 * @size: Size of the 'pins' array in bytes.
 *        The number of register/value pairs in the 'pins' array therefore
 *        equals to 'size / sizeof(struct single_fdt_pin_cfg)'.
 */
static int single_configure_pins(struct udevice *dev,
				 const struct single_fdt_pin_cfg *pins,
				 int size)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	int n, count = size / sizeof(struct single_fdt_pin_cfg);
	phys_addr_t reg;
	u32 offset, val;

	/* If function mask is null, needn't enable it. */
	if (!pdata->mask)
		return 0;

	for (n = 0; n < count; n++, pins++) {
		offset = fdt32_to_cpu(pins->reg);
		if (offset < 0 || offset > pdata->offset) {
			dev_dbg(dev, "  invalid register offset 0x%x\n",
				offset);
			continue;
		}

		reg = pdata->base + offset;
		val = fdt32_to_cpu(pins->val) & pdata->mask;
		single_write(dev, (single_read(dev, reg) & ~pdata->mask) | val,
			     reg);
		dev_dbg(dev, "  reg/val %pa/0x%08x\n", &reg, val);

	}
	return 0;
}

static int single_configure_bits(struct udevice *dev,
				 const struct single_fdt_bits_cfg *pins,
				 int size)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	int n, count = size / sizeof(struct single_fdt_bits_cfg);
	phys_addr_t reg;
	u32 offset, val, mask;

	for (n = 0; n < count; n++, pins++) {
		offset = fdt32_to_cpu(pins->reg);
		if (offset < 0 || offset > pdata->offset) {
			dev_dbg(dev, "  invalid register offset 0x%x\n",
				offset);
			continue;
		}

		reg = pdata->base + offset;

		mask = fdt32_to_cpu(pins->mask);
		val = fdt32_to_cpu(pins->val) & mask;
		single_write(dev, (single_read(dev, reg) & ~mask) | val, reg);
		dev_dbg(dev, "  reg/val %pa/0x%08x\n", &reg, val);
	}
	return 0;
}
static int single_set_state(struct udevice *dev,
			    struct udevice *config)
{
	const struct single_fdt_pin_cfg *prop;
	const struct single_fdt_bits_cfg *prop_bits;
	int len;

	prop = dev_read_prop(config, "pinctrl-single,pins", &len);

	if (prop) {
		dev_dbg(dev, "configuring pins for %s\n", config->name);
		if (len % sizeof(struct single_fdt_pin_cfg)) {
			dev_dbg(dev, "  invalid pin configuration in fdt\n");
			return -FDT_ERR_BADSTRUCTURE;
		}
		single_configure_pins(dev, prop, len);
		return 0;
	}

	/* pinctrl-single,pins not found so check for pinctrl-single,bits */
	prop_bits = dev_read_prop(config, "pinctrl-single,bits", &len);
	if (prop_bits) {
		dev_dbg(dev, "configuring pins for %s\n", config->name);
		if (len % sizeof(struct single_fdt_bits_cfg)) {
			dev_dbg(dev, "  invalid bits configuration in fdt\n");
			return -FDT_ERR_BADSTRUCTURE;
		}
		single_configure_bits(dev, prop_bits, len);
		return 0;
	}

	/* Neither 'pinctrl-single,pins' nor 'pinctrl-single,bits' were found */
	return len;
}

static const char *single_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	struct single_priv *priv = dev_get_priv(dev);

	if (selector >= priv->npins)
		snprintf(priv->pin_name, PINNAME_SIZE, "Error");
	else
		snprintf(priv->pin_name, PINNAME_SIZE, "PIN%u", selector);

	return priv->pin_name;
}

static int single_get_pins_count(struct udevice *dev)
{
	struct single_priv *priv = dev_get_priv(dev);

	return priv->npins;
}

static int single_probe(struct udevice *dev)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	struct single_priv *priv = dev_get_priv(dev);
	u32 size;

	size = pdata->offset + pdata->width / BITS_PER_BYTE;
	priv->npins = size / (pdata->width / BITS_PER_BYTE);
	if (pdata->bits_per_mux) {
		priv->bits_per_pin = fls(pdata->mask);
		priv->npins *= (pdata->width / priv->bits_per_pin);
	}

	dev_dbg(dev, "%d pins\n", priv->npins);
	return 0;
}

static int single_of_to_plat(struct udevice *dev)
{
	fdt_addr_t addr;
	fdt_size_t size;
	struct single_pdata *pdata = dev_get_plat(dev);
	int ret;

	ret = dev_read_u32(dev, "pinctrl-single,register-width", &pdata->width);
	if (ret) {
		dev_err(dev, "missing register width\n");
		return ret;
	}

	switch (pdata->width) {
	case 8:
	case 16:
	case 32:
		break;
	default:
		dev_err(dev, "wrong register width\n");
		return -EINVAL;
	}

	addr = dev_read_addr_size(dev, "reg", &size);
	if (addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "failed to get base register size\n");
		return -EINVAL;
	}

	pdata->offset = size - pdata->width / BITS_PER_BYTE;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE) {
		dev_dbg(dev, "no valid base register address\n");
		return -EINVAL;
	}
	pdata->base = addr;

	ret = dev_read_u32(dev, "pinctrl-single,function-mask", &pdata->mask);
	if (ret) {
		pdata->mask = 0;
		dev_warn(dev, "missing function register mask\n");
	}

	pdata->bits_per_mux = dev_read_bool(dev, "pinctrl-single,bit-per-mux");

	return 0;
}

const struct pinctrl_ops single_pinctrl_ops = {
	.get_pins_count	= single_get_pins_count,
	.get_pin_name = single_get_pin_name,
	.set_state = single_set_state,
};

static const struct udevice_id single_pinctrl_match[] = {
	{ .compatible = "pinctrl-single" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(single_pinctrl) = {
	.name = "single-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = single_pinctrl_match,
	.ops = &single_pinctrl_ops,
	.plat_auto	= sizeof(struct single_pdata),
	.priv_auto = sizeof(struct single_priv),
	.of_to_plat = single_of_to_plat,
	.probe = single_probe,
};
