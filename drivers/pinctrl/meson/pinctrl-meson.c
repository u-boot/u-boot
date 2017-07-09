/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <fdt_support.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/gpio.h>

#include "pinctrl-meson.h"

DECLARE_GLOBAL_DATA_PTR;

static const char *meson_pinctrl_dummy_name = "_dummy";

static int meson_pinctrl_get_groups_count(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	return priv->data->num_groups;
}

static const char *meson_pinctrl_get_group_name(struct udevice *dev,
						unsigned selector)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	if (!priv->data->groups[selector].name)
		return meson_pinctrl_dummy_name;

	return priv->data->groups[selector].name;
}

static int meson_pinmux_get_functions_count(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	return priv->data->num_funcs;
}

static const char *meson_pinmux_get_function_name(struct udevice *dev,
						  unsigned selector)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);

	return priv->data->funcs[selector].name;
}

static void meson_pinmux_disable_other_groups(struct meson_pinctrl *priv,
					      unsigned int pin, int sel_group)
{
	struct meson_pmx_group *group;
	void __iomem *addr;
	int i, j;

	for (i = 0; i < priv->data->num_groups; i++) {
		group = &priv->data->groups[i];
		if (group->is_gpio || i == sel_group)
			continue;

		for (j = 0; j < group->num_pins; j++) {
			if (group->pins[j] == pin) {
				/* We have found a group using the pin */
				debug("pinmux: disabling %s\n", group->name);
				addr = priv->reg_mux + group->reg * 4;
				writel(readl(addr) & ~BIT(group->bit), addr);
			}
		}
	}
}

static int meson_pinmux_group_set(struct udevice *dev,
				  unsigned group_selector,
				  unsigned func_selector)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	const struct meson_pmx_group *group;
	const struct meson_pmx_func *func;
	void __iomem *addr;
	int i;

	group = &priv->data->groups[group_selector];
	func = &priv->data->funcs[func_selector];

	debug("pinmux: set group %s func %s\n", group->name, func->name);

	/*
	 * Disable groups using the same pins.
	 * The selected group is not disabled to avoid glitches.
	 */
	for (i = 0; i < group->num_pins; i++) {
		meson_pinmux_disable_other_groups(priv,
						  group->pins[i],
						  group_selector);
	}

	/* Function 0 (GPIO) doesn't need any additional setting */
	if (func_selector) {
		addr = priv->reg_mux + group->reg * 4;
		writel(readl(addr) | BIT(group->bit), addr);
	}

	return 0;
}

const struct pinctrl_ops meson_pinctrl_ops = {
	.get_groups_count = meson_pinctrl_get_groups_count,
	.get_group_name = meson_pinctrl_get_group_name,
	.get_functions_count = meson_pinmux_get_functions_count,
	.get_function_name = meson_pinmux_get_function_name,
	.pinmux_group_set = meson_pinmux_group_set,
	.set_state = pinctrl_generic_set_state,
};

static int meson_gpio_calc_reg_and_bit(struct udevice *dev, unsigned int offset,
				       enum meson_reg_type reg_type,
				       unsigned int *reg, unsigned int *bit)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	struct meson_bank *bank = NULL;
	struct meson_reg_desc *desc;
	unsigned int pin;
	int i;

	pin = priv->data->pin_base + offset;

	for (i = 0; i < priv->data->num_banks; i++) {
		if (pin >= priv->data->banks[i].first &&
		    pin <= priv->data->banks[i].last) {
			bank = &priv->data->banks[i];
			break;
		}
	}

	if (!bank)
		return -EINVAL;

	desc = &bank->regs[reg_type];
	*reg = desc->reg * 4;
	*bit = desc->bit + pin - bank->first;

	return 0;
}

static int meson_gpio_get(struct udevice *dev, unsigned int offset)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_IN, &reg, &bit);
	if (ret)
		return ret;

	return !!(readl(priv->reg_gpio + reg) & BIT(bit));
}

static int meson_gpio_set(struct udevice *dev, unsigned int offset, int value)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_OUT, &reg, &bit);
	if (ret)
		return ret;

	clrsetbits_le32(priv->reg_gpio + reg, BIT(bit), value ? BIT(bit) : 0);

	return 0;
}

static int meson_gpio_get_direction(struct udevice *dev, unsigned int offset)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit, val;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_DIR, &reg, &bit);
	if (ret)
		return ret;

	val = readl(priv->reg_gpio + reg);

	return (val & BIT(bit)) ? GPIOF_INPUT : GPIOF_OUTPUT;
}

static int meson_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_DIR, &reg, &bit);
	if (ret)
		return ret;

	clrsetbits_le32(priv->reg_gpio + reg, BIT(bit), 1);

	return 0;
}

static int meson_gpio_direction_output(struct udevice *dev,
				       unsigned int offset, int value)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	unsigned int reg, bit;
	int ret;

	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_DIR, &reg, &bit);
	if (ret)
		return ret;

	clrsetbits_le32(priv->reg_gpio + reg, BIT(bit), 0);

	ret = meson_gpio_calc_reg_and_bit(dev, offset, REG_OUT, &reg, &bit);
	if (ret)
		return ret;

	clrsetbits_le32(priv->reg_gpio + reg, BIT(bit), value ? BIT(bit) : 0);

	return 0;
}

static int meson_gpio_probe(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev->parent);
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = priv->data->name;
	uc_priv->gpio_count = priv->data->num_pins;

	return 0;
}

static const struct dm_gpio_ops meson_gpio_ops = {
	.set_value = meson_gpio_set,
	.get_value = meson_gpio_get,
	.get_function = meson_gpio_get_direction,
	.direction_input = meson_gpio_direction_input,
	.direction_output = meson_gpio_direction_output,
};

static struct driver meson_gpio_driver = {
	.name	= "meson-gpio",
	.id	= UCLASS_GPIO,
	.probe	= meson_gpio_probe,
	.ops	= &meson_gpio_ops,
};

static fdt_addr_t parse_address(int offset, const char *name, int na, int ns)
{
	int index, len = 0;
	const fdt32_t *reg;

	index = fdt_stringlist_search(gd->fdt_blob, offset, "reg-names", name);
	if (index < 0)
		return FDT_ADDR_T_NONE;

	reg = fdt_getprop(gd->fdt_blob, offset, "reg", &len);
	if (!reg || (len <= (index * sizeof(fdt32_t) * (na + ns))))
		return FDT_ADDR_T_NONE;

	reg += index * (na + ns);

	return fdt_translate_address((void *)gd->fdt_blob, offset, reg);
}

int meson_pinctrl_probe(struct udevice *dev)
{
	struct meson_pinctrl *priv = dev_get_priv(dev);
	struct uclass_driver *drv;
	struct udevice *gpio_dev;
	fdt_addr_t addr;
	int node, gpio = -1, len;
	int na, ns;
	char *name;

	na = fdt_address_cells(gd->fdt_blob, dev_of_offset(dev->parent));
	if (na < 1) {
		debug("bad #address-cells\n");
		return -EINVAL;
	}

	ns = fdt_size_cells(gd->fdt_blob, dev_of_offset(dev->parent));
	if (ns < 1) {
		debug("bad #size-cells\n");
		return -EINVAL;
	}

	fdt_for_each_subnode(node, gd->fdt_blob, dev_of_offset(dev)) {
		if (fdt_getprop(gd->fdt_blob, node, "gpio-controller", &len)) {
			gpio = node;
			break;
		}
	}

	if (!gpio) {
		debug("gpio node not found\n");
		return -EINVAL;
	}

	addr = parse_address(gpio, "mux", na, ns);
	if (addr == FDT_ADDR_T_NONE) {
		debug("mux address not found\n");
		return -EINVAL;
	}
	priv->reg_mux = (void __iomem *)addr;

	addr = parse_address(gpio, "gpio", na, ns);
	if (addr == FDT_ADDR_T_NONE) {
		debug("gpio address not found\n");
		return -EINVAL;
	}
	priv->reg_gpio = (void __iomem *)addr;
	priv->data = (struct meson_pinctrl_data *)dev_get_driver_data(dev);

	/* Lookup GPIO driver */
	drv = lists_uclass_lookup(UCLASS_GPIO);
	if (!drv) {
		puts("Cannot find GPIO driver\n");
		return -ENOENT;
	}

	name = calloc(1, 32);
	sprintf(name, "meson-gpio");

	/* Create child device UCLASS_GPIO and bind it */
	device_bind(dev, &meson_gpio_driver, name, NULL, gpio, &gpio_dev);
	dev_set_of_offset(gpio_dev, gpio);

	return 0;
}
