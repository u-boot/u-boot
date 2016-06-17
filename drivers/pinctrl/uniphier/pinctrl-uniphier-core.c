/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mapmem.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

DECLARE_GLOBAL_DATA_PTR;

static int uniphier_pinctrl_get_groups_count(struct udevice *dev)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->socdata->groups_count;
}

static const char *uniphier_pinctrl_get_group_name(struct udevice *dev,
						   unsigned selector)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->socdata->groups[selector].name;
}

static int uniphier_pinmux_get_functions_count(struct udevice *dev)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->socdata->functions_count;
}

static const char *uniphier_pinmux_get_function_name(struct udevice *dev,
						     unsigned selector)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->socdata->functions[selector];
}

static void uniphier_pinconf_input_enable(struct udevice *dev, unsigned pin)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	int pins_count = priv->socdata->pins_count;
	const struct uniphier_pinctrl_pin *pins = priv->socdata->pins;
	int i;

	for (i = 0; i < pins_count; i++) {
		if (pins[i].number == pin) {
			unsigned int iectrl;
			u32 tmp;

			iectrl = uniphier_pin_get_iectrl(pins[i].data);
			tmp = readl(priv->base + UNIPHIER_PINCTRL_IECTRL);
			tmp |= 1 << iectrl;
			writel(tmp, priv->base + UNIPHIER_PINCTRL_IECTRL);
		}
	}
}

static void uniphier_pinmux_set_one(struct udevice *dev, unsigned pin,
				    unsigned muxval)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned mux_bits = priv->socdata->mux_bits;
	unsigned reg_stride = priv->socdata->reg_stride;
	unsigned reg, reg_end, shift, mask;
	u32 tmp;

	/* some pins need input-enabling */
	uniphier_pinconf_input_enable(dev, pin);

	reg = UNIPHIER_PINCTRL_PINMUX_BASE + pin * mux_bits / 32 * reg_stride;
	reg_end = reg + reg_stride;
	shift = pin * mux_bits % 32;
	mask = (1U << mux_bits) - 1;

	/*
	 * If reg_stride is greater than 4, the MSB of each pinsel shall be
	 * stored in the offset+4.
	 */
	for (; reg < reg_end; reg += 4) {
		tmp = readl(priv->base + reg);
		tmp &= ~(mask << shift);
		tmp |= (mask & muxval) << shift;
		writel(tmp, priv->base + reg);

		muxval >>= mux_bits;
	}

	if (priv->socdata->load_pinctrl)
		writel(1, priv->base + UNIPHIER_PINCTRL_LOAD_PINMUX);
}

static int uniphier_pinmux_group_set(struct udevice *dev,
				     unsigned group_selector,
				     unsigned func_selector)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	const struct uniphier_pinctrl_group *grp =
					&priv->socdata->groups[group_selector];
	int i;

	for (i = 0; i < grp->num_pins; i++)
		uniphier_pinmux_set_one(dev, grp->pins[i], grp->muxvals[i]);

	return 0;
}

const struct pinctrl_ops uniphier_pinctrl_ops = {
	.get_groups_count = uniphier_pinctrl_get_groups_count,
	.get_group_name = uniphier_pinctrl_get_group_name,
	.get_functions_count = uniphier_pinmux_get_functions_count,
	.get_function_name = uniphier_pinmux_get_function_name,
	.pinmux_group_set = uniphier_pinmux_group_set,
	.set_state = pinctrl_generic_set_state,
};

int uniphier_pinctrl_probe(struct udevice *dev,
			   struct uniphier_pinctrl_socdata *socdata)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	fdt_size_t size;

	addr = fdtdec_get_addr_size(gd->fdt_blob, dev->of_offset, "reg",
				    &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = map_sysmem(addr, size);
	if (!priv->base)
		return -ENOMEM;

	priv->socdata = socdata;

	return 0;
}

int uniphier_pinctrl_remove(struct udevice *dev)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	unmap_sysmem(priv->base);

	return 0;
}
