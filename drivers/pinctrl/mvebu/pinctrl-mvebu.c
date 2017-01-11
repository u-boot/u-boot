/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <config.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch-armada8k/soc-info.h>
#include "pinctrl-mvebu.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * mvebu_pinctrl_set_state: configure pin functions.
 * @dev: the pinctrl device to be configured.
 * @config: the state to be configured.
 * @return: 0 in success
 */
int mvebu_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	const void *blob = gd->fdt_blob;
	int node = config->of_offset;
	struct mvebu_pinctrl_priv *priv;
	u32 pin_arr[MVEBU_MAX_PINS_PER_BANK];
	u32 function;
	int i, pin_count;

	priv = dev_get_priv(dev);

	pin_count = fdtdec_get_int_array_count(blob, node,
					       "marvell,pins",
					       pin_arr,
					       MVEBU_MAX_PINS_PER_BANK);
	if (pin_count <= 0) {
		debug("Failed reading pins array for pinconfig %s (%d)\n",
		      config->name, pin_count);
		return -EINVAL;
	}

	function = fdtdec_get_int(blob, node, "marvell,function", 0xff);

	for (i = 0; i < pin_count; i++) {
	int reg_offset;
	int field_offset;
		int pin = pin_arr[i];

		if (function > priv->max_func) {
			debug("Illegal function %d for pinconfig %s\n",
			      function, config->name);
			return -EINVAL;
		}

		/* Calculate register address and bit in register */
		reg_offset   = priv->reg_direction * 4 *
					(pin >> (PIN_REG_SHIFT));
		field_offset = (BITS_PER_PIN) * (pin & PIN_FIELD_MASK);

		clrsetbits_le32(priv->base_reg + reg_offset,
				PIN_FUNC_MASK << field_offset,
				(function & PIN_FUNC_MASK) << field_offset);
	}

	return 0;
}

/*
 * mvebu_pinctrl_set_state_all: configure the entire bank pin functions.
 * @dev: the pinctrl device to be configured.
 * @config: the state to be configured.
 * @return: 0 in success
 */
static int mvebu_pinctrl_set_state_all(struct udevice *dev,
				       struct udevice *config)
{
	const void *blob = gd->fdt_blob;
	int node = config->of_offset;
	struct mvebu_pinctrl_priv *priv;
	u32 func_arr[MVEBU_MAX_PINS_PER_BANK];
	int pin, err;

	priv = dev_get_priv(dev);

	err = fdtdec_get_int_array(blob, node, "pin-func",
				   func_arr, priv->pin_cnt);
	if (err) {
		debug("Failed reading pin functions for bank %s\n",
		      priv->bank_name);
		return -EINVAL;
	}

	for (pin = 0; pin < priv->pin_cnt; pin++) {
		int reg_offset;
		int field_offset;
		u32 func = func_arr[pin];

		/* Bypass pins with function 0xFF */
		if (func == 0xff) {
			debug("Warning: pin %d value is not modified ", pin);
			debug("(kept as default)\n");
			continue;
		} else if (func > priv->max_func) {
			debug("Illegal function %d for pin %d\n", func, pin);
			return -EINVAL;
		}

		/* Calculate register address and bit in register */
		reg_offset   = priv->reg_direction * 4 *
					(pin >> (PIN_REG_SHIFT));
		field_offset = (BITS_PER_PIN) * (pin & PIN_FIELD_MASK);

		clrsetbits_le32(priv->base_reg + reg_offset,
				PIN_FUNC_MASK << field_offset,
				(func & PIN_FUNC_MASK) << field_offset);
	}

	return 0;
}

int mvebu_pinctl_probe(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	int node = dev->of_offset;
	struct mvebu_pinctrl_priv *priv;

	priv = dev_get_priv(dev);
	if (!priv) {
		debug("%s: Failed to get private\n", __func__);
		return -EINVAL;
	}

	priv->base_reg = dev_get_addr_ptr(dev);
	if (priv->base_reg == (void *)FDT_ADDR_T_NONE) {
		debug("%s: Failed to get base address\n", __func__);
		return -EINVAL;
	}

	priv->pin_cnt   = fdtdec_get_int(blob, node, "pin-count",
					MVEBU_MAX_PINS_PER_BANK);
	priv->max_func  = fdtdec_get_int(blob, node, "max-func",
					 MVEBU_MAX_FUNC);
	priv->bank_name = fdt_getprop(blob, node, "bank-name", NULL);

	priv->reg_direction = 1;
	if (fdtdec_get_bool(blob, node, "reverse-reg"))
		priv->reg_direction = -1;

	return mvebu_pinctrl_set_state_all(dev, dev);
}

static struct pinctrl_ops mvebu_pinctrl_ops = {
	.set_state	= mvebu_pinctrl_set_state
};

static const struct udevice_id mvebu_pinctrl_ids[] = {
	{ .compatible = "marvell,mvebu-pinctrl" },
	{ .compatible = "marvell,armada-ap806-pinctrl" },
	{ .compatible = "marvell,a70x0-pinctrl" },
	{ .compatible = "marvell,a80x0-cp0-pinctrl" },
	{ .compatible = "marvell,a80x0-cp1-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_mvebu) = {
	.name		= "mvebu_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= mvebu_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct mvebu_pinctrl_priv),
	.ops		= &mvebu_pinctrl_ops,
	.probe		= mvebu_pinctl_probe
};
