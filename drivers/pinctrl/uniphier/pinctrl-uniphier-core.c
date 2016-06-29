/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mapmem.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const char *uniphier_pinctrl_dummy_name = "_dummy";

static int uniphier_pinctrl_get_groups_count(struct udevice *dev)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->socdata->groups_count;
}

static const char *uniphier_pinctrl_get_group_name(struct udevice *dev,
						   unsigned selector)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	if (!priv->socdata->groups[selector].name)
		return uniphier_pinctrl_dummy_name;

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

	if (!priv->socdata->functions[selector])
		return uniphier_pinctrl_dummy_name;

	return priv->socdata->functions[selector];
}

static void uniphier_pinconf_input_enable_perpin(struct udevice *dev,
						 unsigned pin)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned reg;
	u32 mask, tmp;

	reg = UNIPHIER_PINCTRL_IECTRL + pin / 32 * 4;
	mask = BIT(pin % 32);

	tmp = readl(priv->base + reg);
	tmp |= mask;
	writel(tmp, priv->base + reg);
}

static void uniphier_pinconf_input_enable_legacy(struct udevice *dev,
						 unsigned pin)
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

static void uniphier_pinconf_input_enable(struct udevice *dev, unsigned pin)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);

	if (priv->socdata->caps & UNIPHIER_PINCTRL_CAPS_PERPIN_IECTRL)
		uniphier_pinconf_input_enable_perpin(dev, pin);
	else
		uniphier_pinconf_input_enable_legacy(dev, pin);
}

static void uniphier_pinmux_set_one(struct udevice *dev, unsigned pin,
				    int muxval)
{
	struct uniphier_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned mux_bits, reg_stride, reg, reg_end, shift, mask;
	bool load_pinctrl;
	u32 tmp;

	/* some pins need input-enabling */
	uniphier_pinconf_input_enable(dev, pin);

	if (muxval < 0)
		return;		/* dedicated pin; nothing to do for pin-mux */

	if (priv->socdata->caps & UNIPHIER_PINCTRL_CAPS_DBGMUX_SEPARATE) {
		/*
		 *  Mode       offset        bit
		 *  Normal     4 * n     shift+3:shift
		 *  Debug      4 * n     shift+7:shift+4
		 */
		mux_bits = 4;
		reg_stride = 8;
		load_pinctrl = true;
	} else {
		/*
		 *  Mode       offset           bit
		 *  Normal     8 * n        shift+3:shift
		 *  Debug      8 * n + 4    shift+3:shift
		 */
		mux_bits = 8;
		reg_stride = 4;
		load_pinctrl = false;
	}

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

	if (load_pinctrl)
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

	addr = dev_get_addr(dev->parent);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = map_sysmem(addr, SZ_4K);
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
