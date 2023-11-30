// SPDX-License-Identifier: GPL-2.0+
/*
 * Exynos pinctrl driver common code.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#include <log.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include "pinctrl-exynos.h"

DECLARE_GLOBAL_DATA_PTR;

/**
 * exynos_pinctrl_setup_peri: setup pinctrl for a peripheral.
 * conf: soc specific pin configuration data array
 * num_conf: number of configurations in the conf array.
 * base: base address of the pin controller.
 */
void exynos_pinctrl_setup_peri(struct exynos_pinctrl_config_data *conf,
		unsigned int num_conf, unsigned long base)
{
	unsigned int idx, val;

	for (idx = 0; idx < num_conf; idx++) {
		val = readl(base + conf[idx].offset);
		val &= ~(conf[idx].mask);
		val |= conf[idx].value;
		writel(val, base + conf[idx].offset);
	}
}

static void parse_pin(const char *pin_name, u32 *pin, char *bank_name)
{
	u32 idx = 0;

	/*
	 * The format of the pin name is <bank_name name>-<pin_number>.
	 * Example: gpa0-4 (gpa0 is the bank_name name and 4 is the pin number.
	 */
	while (pin_name[idx] != '-') {
		bank_name[idx] = pin_name[idx];
		idx++;
	}
	bank_name[idx] = '\0';
	*pin = pin_name[++idx] - '0';
}

/* given a bank name, find out the pin bank structure */
static const struct samsung_pin_bank_data *get_bank(struct udevice *dev,
						    const char *bank_name)
{
	struct exynos_pinctrl_priv *priv = dev_get_priv(dev);
	const struct samsung_pin_ctrl *pin_ctrl_array = priv->pin_ctrl;
	const struct samsung_pin_bank_data *bank_data;
	u32 nr_banks, pin_ctrl_idx = 0, idx = 0;

	/* lookup the pin bank data using the pin bank name */
	while (true) {
		const struct samsung_pin_ctrl *pin_ctrl =
			&pin_ctrl_array[pin_ctrl_idx];

		nr_banks = pin_ctrl->nr_banks;
		if (!nr_banks)
			break;

		bank_data = pin_ctrl->pin_banks;
		for (idx = 0; idx < nr_banks; idx++) {
			debug("pinctrl[%d] bank_data[%d] name is: %s\n",
					pin_ctrl_idx, idx, bank_data[idx].name);
			if (!strcmp(bank_name, bank_data[idx].name))
				return &bank_data[idx];
		}
		pin_ctrl_idx++;
	}

	return NULL;
}

/**
 * exynos_pinctrl_set_state: configure a pin state.
 * dev: the pinctrl device to be configured.
 * config: the state to be configured.
 */
int exynos_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct exynos_pinctrl_priv *priv = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(config);
	unsigned int count, idx, pin_num;
	unsigned int pinfunc, pinpud, pindrv;
	unsigned long reg, value;
	const char *name;

	/*
	 * refer to the following document for the pinctrl bindings
	 * linux/Documentation/devicetree/bindings/pinctrl/samsung-pinctrl.txt
	 */
	count = fdt_stringlist_count(fdt, node, "samsung,pins");
	if (count <= 0)
		return -EINVAL;

	pinfunc = fdtdec_get_int(fdt, node, "samsung,pin-function", -1);
	pinpud = fdtdec_get_int(fdt, node, "samsung,pin-pud", -1);
	pindrv = fdtdec_get_int(fdt, node, "samsung,pin-drv", -1);

	for (idx = 0; idx < count; idx++) {
		const struct samsung_pin_bank_data *bank;
		char bank_name[10];

		name = fdt_stringlist_get(fdt, node, "samsung,pins", idx, NULL);
		if (!name)
			continue;
		parse_pin(name, &pin_num, bank_name);
		bank = get_bank(dev, bank_name);
		reg = priv->base + bank->offset;

		if (pinfunc != -1) {
			value = readl(reg + PIN_CON);
			value &= ~(0xf << (pin_num << 2));
			value |= (pinfunc << (pin_num << 2));
			writel(value, reg + PIN_CON);
		}

		if (pinpud != -1) {
			value = readl(reg + PIN_PUD);
			value &= ~(0x3 << (pin_num << 1));
			value |= (pinpud << (pin_num << 1));
			writel(value, reg + PIN_PUD);
		}

		if (pindrv != -1) {
			value = readl(reg + PIN_DRV);
			value &= ~(0x3 << (pin_num << 1));
			value |= (pindrv << (pin_num << 1));
			writel(value, reg + PIN_DRV);
		}
	}

	return 0;
}

int exynos_pinctrl_probe(struct udevice *dev)
{
	struct exynos_pinctrl_priv *priv;
	fdt_addr_t base;

	priv = dev_get_priv(dev);
	if (!priv)
		return -EINVAL;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = base;
	priv->pin_ctrl = (struct samsung_pin_ctrl *)dev_get_driver_data(dev) +
				dev_seq(dev);

	return 0;
}
