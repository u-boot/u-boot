// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm GPIO driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <dm.h>
#include <errno.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <mach/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/* OE */
#define GPIO_OE_DISABLE  (0x0 << 9)
#define GPIO_OE_ENABLE   (0x1 << 9)
#define GPIO_OE_MASK     (0x1 << 9)

/* GPIO_IN_OUT register shifts. */
#define GPIO_IN          0
#define GPIO_OUT         1

struct msm_gpio_bank {
	phys_addr_t base;
	const struct msm_pin_data *pin_data;
};

#define GPIO_CONFIG_REG(dev, x) \
	(qcom_pin_offset(((struct msm_gpio_bank *)dev_get_priv(dev))->pin_data->pin_offsets, x))

#define GPIO_IN_OUT_REG(dev, x) \
	(GPIO_CONFIG_REG(dev, x) + 0x4)

static void msm_gpio_direction_input_special(struct msm_gpio_bank *priv,
					     unsigned int gpio)
{
	unsigned int offset = gpio - priv->pin_data->special_pins_start;
	const struct msm_special_pin_data *data;

	if (!priv->pin_data->special_pins_data)
		return;

	data = &priv->pin_data->special_pins_data[offset];

	if (!data->ctl_reg || data->oe_bit >= 31)
		return;

	/* switch direction */
	clrsetbits_le32(priv->base + data->ctl_reg,
			BIT(data->oe_bit), 0);
}

static void msm_gpio_direction_input(struct udevice *dev, unsigned int gpio)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	if (qcom_is_special_pin(priv->pin_data, gpio))
		msm_gpio_direction_input_special(priv, gpio);

	/* Disable OE bit */
	clrsetbits_le32(priv->base + GPIO_CONFIG_REG(dev, gpio),
			GPIO_OE_MASK, GPIO_OE_DISABLE);

	return;
}

static int msm_gpio_set_value_special(struct msm_gpio_bank *priv,
				      unsigned int gpio, int value)
{
	unsigned int offset = gpio - priv->pin_data->special_pins_start;
	const struct msm_special_pin_data *data;

	if (!priv->pin_data->special_pins_data)
		return 0;

	data = &priv->pin_data->special_pins_data[offset];

	if (!data->io_reg || data->out_bit >= 31)
		return 0;

	value = !!value;
	/* set value */
	writel(value << data->out_bit, priv->base + data->io_reg);

	return 0;
}

static int msm_gpio_set_value(struct udevice *dev, unsigned int gpio, int value)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	if (qcom_is_special_pin(priv->pin_data, gpio))
		return msm_gpio_set_value_special(priv, gpio, value);

	value = !!value;
	/* set value */
	writel(value << GPIO_OUT, priv->base + GPIO_IN_OUT_REG(dev, gpio));

	return 0;
}

static int msm_gpio_direction_output_special(struct msm_gpio_bank *priv,
					     unsigned int gpio,
					     int value)
{
	unsigned int offset = gpio - priv->pin_data->special_pins_start;
	const struct msm_special_pin_data *data;

	if (!priv->pin_data->special_pins_data)
		return 0;

	data = &priv->pin_data->special_pins_data[offset];

	if (!data->io_reg || data->out_bit >= 31)
		return 0;

	value = !!value;
	/* set value */
	writel(value << data->out_bit, priv->base + data->io_reg);

	if (!data->ctl_reg || data->oe_bit >= 31)
		return 0;

	/* switch direction */
	clrsetbits_le32(priv->base + data->ctl_reg,
			BIT(data->oe_bit), BIT(data->oe_bit));

	return 0;
}

static int msm_gpio_direction_output(struct udevice *dev, unsigned int gpio,
				     int value)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	if (qcom_is_special_pin(priv->pin_data, gpio))
		return msm_gpio_direction_output_special(priv, gpio, value);

	value = !!value;
	/* set value */
	writel(value << GPIO_OUT, priv->base + GPIO_IN_OUT_REG(dev, gpio));
	/* switch direction */
	clrsetbits_le32(priv->base + GPIO_CONFIG_REG(dev, gpio),
			GPIO_OE_MASK, GPIO_OE_ENABLE);

	return 0;
}

static int msm_gpio_set_flags(struct udevice *dev, unsigned int gpio, ulong flags)
{
	if (msm_pinctrl_is_reserved(dev_get_parent(dev), gpio))
		return -EPERM;

	if (flags & GPIOD_IS_OUT_ACTIVE) {
		return msm_gpio_direction_output(dev, gpio, 1);
	} else if (flags & GPIOD_IS_OUT) {
		return msm_gpio_direction_output(dev, gpio, 0);
	} else if (flags & GPIOD_IS_IN) {
		msm_gpio_direction_input(dev, gpio);
		if (flags & GPIOD_PULL_UP)
			return msm_gpio_set_value(dev, gpio, 1);
		else if (flags & GPIOD_PULL_DOWN)
			return msm_gpio_set_value(dev, gpio, 0);
	}

	return 0;
}

static int msm_gpio_get_value_special(struct msm_gpio_bank *priv, unsigned int gpio)
{
	unsigned int offset = gpio - priv->pin_data->special_pins_start;
	const struct msm_special_pin_data *data;

	if (!priv->pin_data->special_pins_data)
		return -EINVAL;

	data = &priv->pin_data->special_pins_data[offset];

	if (!data->io_reg)
		return -EINVAL;

	if (data->in_bit >= 31) {
		if (data->out_bit >= 31)
			return -EINVAL;

		return !!(readl(priv->base + data->io_reg) >> data->out_bit);
	}

	return !!(readl(priv->base + data->io_reg) >> data->in_bit);
}

static int msm_gpio_get_value(struct udevice *dev, unsigned int gpio)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	if (msm_pinctrl_is_reserved(dev_get_parent(dev), gpio))
		return -EPERM;

	if (qcom_is_special_pin(priv->pin_data, gpio))
		return msm_gpio_get_value_special(priv, gpio);

	return !!(readl(priv->base + GPIO_IN_OUT_REG(dev, gpio)) & BIT(GPIO_IN));
}

static int msm_gpio_get_function_special(struct msm_gpio_bank *priv,
					 unsigned int gpio)
{
	unsigned int offset = gpio - priv->pin_data->special_pins_start;
	const struct msm_special_pin_data *data;

	if (!priv->pin_data->special_pins_data)
		return GPIOF_UNKNOWN;

	data = &priv->pin_data->special_pins_data[offset];

	/* No I/O fields, cannot control/read the I/O value */
	if (!data->io_reg || (data->out_bit >= 31 && data->in_bit >= 31))
		return GPIOF_FUNC;

	/* No Output-Enable register, cannot control I/O direction */
	if (!data->ctl_reg || data->oe_bit >= 31) {
		if (data->out_bit >= 31)
			return GPIOF_INPUT;
		else
			return GPIOF_OUTPUT;
	}

	if (readl(priv->base + data->ctl_reg) & BIT(data->oe_bit))
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static int msm_gpio_get_function(struct udevice *dev, unsigned int gpio)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	if (msm_pinctrl_is_reserved(dev_get_parent(dev), gpio))
		return GPIOF_UNKNOWN;

	/* Always NOP for special pins, assume they're in the correct state */
	if (qcom_is_special_pin(priv->pin_data, gpio))
		return msm_gpio_get_function_special(priv, gpio);

	if (readl(priv->base + GPIO_CONFIG_REG(dev, gpio)) & GPIO_OE_ENABLE)
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_msm_ops = {
	.set_flags		= msm_gpio_set_flags,
	.get_value		= msm_gpio_get_value,
	.get_function		= msm_gpio_get_function,
};

static int msm_gpio_probe(struct udevice *dev)
{
	struct msm_gpio_bank *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	priv->pin_data = (struct msm_pin_data *)dev_get_driver_data(dev);

	return priv->base == FDT_ADDR_T_NONE ? -EINVAL : 0;
}

static int msm_gpio_of_to_plat(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	const struct msm_pin_data *pin_data = (struct msm_pin_data *)dev_get_driver_data(dev);

	/* Get the pin count from the pinctrl driver */
	uc_priv->gpio_count = pin_data->pin_count;
	uc_priv->bank_name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
					 "gpio-bank-name", NULL);
	if (uc_priv->bank_name == NULL)
		uc_priv->bank_name = "soc";

	return 0;
}

U_BOOT_DRIVER(gpio_msm) = {
	.name	= "gpio_msm",
	.id	= UCLASS_GPIO,
	.of_to_plat = msm_gpio_of_to_plat,
	.probe	= msm_gpio_probe,
	.ops	= &gpio_msm_ops,
	.flags	= DM_UC_FLAG_SEQ_ALIAS,
	.priv_auto	= sizeof(struct msm_gpio_bank),
};
