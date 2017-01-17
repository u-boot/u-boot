/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm/device.h>
#include <dm/pinctrl.h>
#include <fdt_support.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>

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
	fdt_addr_t addr;
	int node, gpio = -1, len;
	int na, ns;

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
		debug("mux not found\n");
		return -EINVAL;
	}

	priv->reg_mux = (void __iomem *)addr;
	priv->data = (struct meson_pinctrl_data *)dev_get_driver_data(dev);

	return 0;
}
