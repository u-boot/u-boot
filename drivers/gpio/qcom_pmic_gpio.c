// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm generic pmic gpio driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <log.h>
#include <power/pmic.h>
#include <spmi/spmi.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/gpio.h>
#include <linux/bitops.h>

/* Register offset for each gpio */
#define REG_OFFSET(x)          ((x) * 0x100)

/* Register maps */

/* Type and subtype are shared for all PMIC peripherals */
#define REG_TYPE               0x4
#define REG_SUBTYPE            0x5

/* GPIO peripheral type and subtype out_values */
#define REG_TYPE_VAL		0x10
#define REG_SUBTYPE_GPIO_4CH	0x1
#define REG_SUBTYPE_GPIOC_4CH	0x5
#define REG_SUBTYPE_GPIO_8CH	0x9
#define REG_SUBTYPE_GPIOC_8CH	0xd
#define REG_SUBTYPE_GPIO_LV	0x10
#define REG_SUBTYPE_GPIO_MV	0x11
#define REG_SUBTYPE_GPIO_LV_VIN2          0x12
#define REG_SUBTYPE_GPIO_MV_VIN3          0x13

#define REG_STATUS             0x08
#define REG_STATUS_VAL_MASK    0x1

/* MODE_CTL */
#define REG_CTL		0x40
#define REG_CTL_MODE_MASK       0x70
#define REG_CTL_MODE_INPUT      0x00
#define REG_CTL_MODE_INOUT      0x20
#define REG_CTL_MODE_OUTPUT     0x10
#define REG_CTL_OUTPUT_MASK     0x0F
#define REG_CTL_LV_MV_MODE_MASK		0x3
#define REG_CTL_LV_MV_MODE_INPUT	0x0
#define REG_CTL_LV_MV_MODE_INOUT	0x2
#define REG_CTL_LV_MV_MODE_OUTPUT	0x1

#define REG_DIG_VIN_CTL        0x41
#define REG_DIG_VIN_VIN0       0

#define REG_DIG_PULL_CTL       0x42
#define REG_DIG_PULL_NO_PU     0x5

#define REG_LV_MV_OUTPUT_CTL	0x44
#define REG_LV_MV_OUTPUT_CTL_MASK	0x80
#define REG_LV_MV_OUTPUT_CTL_SHIFT	7

#define REG_DIG_OUT_CTL        0x45
#define REG_DIG_OUT_CTL_CMOS   (0x0 << 4)
#define REG_DIG_OUT_CTL_DRIVE_L 0x1

#define REG_EN_CTL             0x46
#define REG_EN_CTL_ENABLE      (1 << 7)

/**
 * pmic_gpio_match_data - platform specific configuration
 *
 * @PMIC_MATCH_READONLY: treat all GPIOs as readonly, don't attempt to configure them.
 * This is a workaround for an unknown bug on some platforms where trying to write the
 * GPIO configuration registers causes the board to hang.
 */
enum pmic_gpio_quirks {
	QCOM_PMIC_QUIRK_READONLY = (1 << 0),
};

struct qcom_pmic_gpio_data {
	uint32_t pid; /* Peripheral ID on SPMI bus */
	bool     lv_mv_type; /* If subtype is GPIO_LV(0x10) or GPIO_MV(0x11) */
	u32 pin_count;
	struct udevice *pmic; /* Reference to pmic device for read/write */
};

/* dev can be the GPIO or pinctrl device */
static int _qcom_gpio_set_direction(struct udevice *dev, u32 offset, bool input, int value)
{
	struct qcom_pmic_gpio_data *plat = dev_get_plat(dev);
	u32 gpio_base = plat->pid + REG_OFFSET(offset);
	u32 reg_ctl_val;
	int ret = 0;

	/* Select the mode and output */
	if (plat->lv_mv_type) {
		if (input)
			reg_ctl_val = REG_CTL_LV_MV_MODE_INPUT;
		else
			reg_ctl_val = REG_CTL_LV_MV_MODE_INOUT;
	} else {
		if (input)
			reg_ctl_val = REG_CTL_MODE_INPUT;
		else
			reg_ctl_val = REG_CTL_MODE_INOUT | !!value;
	}

	ret = pmic_reg_write(plat->pmic, gpio_base + REG_CTL, reg_ctl_val);
	if (ret < 0)
		return ret;

	if (plat->lv_mv_type && !input) {
		ret = pmic_reg_write(plat->pmic,
				     gpio_base + REG_LV_MV_OUTPUT_CTL,
				     !!value << REG_LV_MV_OUTPUT_CTL_SHIFT);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int qcom_gpio_set_direction(struct udevice *dev, unsigned int offset,
				   bool input, int value)
{
	struct qcom_pmic_gpio_data *plat = dev_get_plat(dev);
	uint32_t gpio_base = plat->pid + REG_OFFSET(offset);
	ulong quirks = dev_get_driver_data(dev);
	int ret = 0;

	/* Some PMICs don't like their GPIOs being configured */
	if (quirks & QCOM_PMIC_QUIRK_READONLY)
		return 0;

	/* Disable the GPIO */
	ret = pmic_clrsetbits(dev->parent, gpio_base + REG_EN_CTL,
			      REG_EN_CTL_ENABLE, 0);
	if (ret < 0)
		return ret;

	_qcom_gpio_set_direction(dev, offset, input, value);

	/* Set the right pull (no pull) */
	ret = pmic_reg_write(plat->pmic, gpio_base + REG_DIG_PULL_CTL,
			     REG_DIG_PULL_NO_PU);
	if (ret < 0)
		return ret;

	/* Configure output pin drivers if needed */
	if (!input) {
		/* Select the VIN - VIN0, pin is input so it doesn't matter */
		ret = pmic_reg_write(plat->pmic, gpio_base + REG_DIG_VIN_CTL,
				     REG_DIG_VIN_VIN0);
		if (ret < 0)
			return ret;

		/* Set the right dig out control */
		ret = pmic_reg_write(plat->pmic, gpio_base + REG_DIG_OUT_CTL,
				     REG_DIG_OUT_CTL_CMOS |
				     REG_DIG_OUT_CTL_DRIVE_L);
		if (ret < 0)
			return ret;
	}

	/* Enable the GPIO */
	return pmic_clrsetbits(dev->parent, gpio_base + REG_EN_CTL, 0,
			       REG_EN_CTL_ENABLE);
}

static int qcom_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	return qcom_gpio_set_direction(dev, offset, true, 0);
}

static int qcom_gpio_direction_output(struct udevice *dev, unsigned offset,
				      int value)
{
	return qcom_gpio_set_direction(dev, offset, false, value);
}

static int qcom_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct qcom_pmic_gpio_data *plat = dev_get_plat(dev);
	uint32_t gpio_base = plat->pid + REG_OFFSET(offset);
	int reg;

	reg = pmic_reg_read(plat->pmic, gpio_base + REG_CTL);
	if (reg < 0)
		return reg;

	if (plat->lv_mv_type) {
		switch (reg & REG_CTL_LV_MV_MODE_MASK) {
		case REG_CTL_LV_MV_MODE_INPUT:
			return GPIOF_INPUT;
		case REG_CTL_LV_MV_MODE_INOUT: /* Fallthrough */
		case REG_CTL_LV_MV_MODE_OUTPUT:
			return GPIOF_OUTPUT;
		default:
			return GPIOF_UNKNOWN;
		}
	} else {
		switch (reg & REG_CTL_MODE_MASK) {
		case REG_CTL_MODE_INPUT:
			return GPIOF_INPUT;
		case REG_CTL_MODE_INOUT: /* Fallthrough */
		case REG_CTL_MODE_OUTPUT:
			return GPIOF_OUTPUT;
		default:
			return GPIOF_UNKNOWN;
		}
	}
}

static int qcom_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct qcom_pmic_gpio_data *plat = dev_get_plat(dev);
	uint32_t gpio_base = plat->pid + REG_OFFSET(offset);
	int reg;

	reg = pmic_reg_read(plat->pmic, gpio_base + REG_STATUS);
	if (reg < 0)
		return reg;

	return !!(reg & REG_STATUS_VAL_MASK);
}

static int qcom_gpio_set_value(struct udevice *dev, unsigned offset,
			       int value)
{
	struct qcom_pmic_gpio_data *plat = dev_get_plat(dev);
	uint32_t gpio_base = plat->pid + REG_OFFSET(offset);

	/* Set the output value of the gpio */
	if (plat->lv_mv_type)
		return pmic_clrsetbits(dev->parent,
				       gpio_base + REG_LV_MV_OUTPUT_CTL,
				       REG_LV_MV_OUTPUT_CTL_MASK,
				       !!value << REG_LV_MV_OUTPUT_CTL_SHIFT);
	else
		return pmic_clrsetbits(dev->parent, gpio_base + REG_CTL,
				       REG_CTL_OUTPUT_MASK, !!value);
}

static int qcom_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			   struct ofnode_phandle_args *args)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (args->args_count < 1)
		return -EINVAL;

	/* GPIOs in DT are 1-based */
	desc->offset = args->args[0] - 1;
	if (desc->offset >= uc_priv->gpio_count)
		return -EINVAL;

	if (args->args_count < 2)
		return 0;

	desc->flags = gpio_flags_xlate(args->args[1]);

	return 0;
}

static const struct dm_gpio_ops qcom_gpio_ops = {
	.direction_input	= qcom_gpio_direction_input,
	.direction_output	= qcom_gpio_direction_output,
	.get_value		= qcom_gpio_get_value,
	.set_value		= qcom_gpio_set_value,
	.get_function		= qcom_gpio_get_function,
	.xlate			= qcom_gpio_xlate,
};

static int qcom_gpio_bind(struct udevice *dev)
{

	struct qcom_pmic_gpio_data *plat = dev_get_plat(dev);
	ulong quirks = dev_get_driver_data(dev);
	struct udevice *child;
	struct driver *drv;
	int ret;

	drv = lists_driver_lookup_name("qcom_pmic_pinctrl");
	if (!drv) {
		log_warning("Cannot find driver '%s'\n", "qcom_pmic_pinctrl");
		return -ENOENT;
	}

	/* Bind the GPIO driver as a child of the PMIC. */
	ret = device_bind_with_driver_data(dev, drv,
					   dev->name,
					   quirks, dev_ofnode(dev), &child);
	if (ret)
		return log_msg_ret("bind", ret);

	dev_set_plat(child, plat);

	return 0;
}

static int qcom_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct qcom_pmic_gpio_data *plat = dev_get_plat(dev);
	struct ofnode_phandle_args args;
	int val, ret;
	u64 pid;

	plat->pmic = dev->parent;

	pid = dev_read_addr(dev);
	if (pid == FDT_ADDR_T_NONE)
		return log_msg_ret("bad address", -EINVAL);

	plat->pid = pid;

	/* Do a sanity check */
	val = pmic_reg_read(plat->pmic, plat->pid + REG_TYPE);
	if (val != REG_TYPE_VAL)
		return log_msg_ret("bad type", -ENXIO);

	val = pmic_reg_read(plat->pmic, plat->pid + REG_SUBTYPE);
	switch (val) {
	case REG_SUBTYPE_GPIO_4CH:
	case REG_SUBTYPE_GPIOC_4CH:
		plat->lv_mv_type = false;
		break;
	case REG_SUBTYPE_GPIO_LV:
	case REG_SUBTYPE_GPIO_MV:
	case REG_SUBTYPE_GPIO_LV_VIN2:
	case REG_SUBTYPE_GPIO_MV_VIN3:
		plat->lv_mv_type = true;
		break;
	default:
		return log_msg_ret("bad subtype", -ENXIO);
	}

	plat->lv_mv_type = val == REG_SUBTYPE_GPIO_LV ||
			   val == REG_SUBTYPE_GPIO_MV;

	/*
	 * Parse basic GPIO count specified via the gpio-ranges property
	 * as specified in upstream devicetrees
	 */
	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "gpio-ranges",
					     NULL, 3, 0, &args);
	if (ret)
		return log_msg_ret("gpio-ranges", ret);

	plat->pin_count = args.args[2];

	uc_priv->gpio_count = plat->pin_count;
	uc_priv->bank_name = "pmic";

	return 0;
}

static const struct udevice_id qcom_gpio_ids[] = {
	{ .compatible = "qcom,pm8916-gpio" },
	{ .compatible = "qcom,pm8994-gpio" },	/* 22 GPIO's */
	{ .compatible = "qcom,pm8998-gpio", .data = QCOM_PMIC_QUIRK_READONLY },
	{ .compatible = "qcom,pms405-gpio" },
	{ .compatible = "qcom,pm8550-gpio", .data = QCOM_PMIC_QUIRK_READONLY },
	{ }
};

U_BOOT_DRIVER(qcom_pmic_gpio) = {
	.name	= "qcom_pmic_gpio",
	.id	= UCLASS_GPIO,
	.of_match = qcom_gpio_ids,
	.bind	= qcom_gpio_bind,
	.probe = qcom_gpio_probe,
	.ops	= &qcom_gpio_ops,
	.plat_auto = sizeof(struct qcom_pmic_gpio_data),
	.flags = DM_FLAG_ALLOC_PDATA,
};

static const struct pinconf_param qcom_pmic_pinctrl_conf_params[] = {
	{ "output-high", PIN_CONFIG_OUTPUT_ENABLE, 1 },
	{ "output-low", PIN_CONFIG_OUTPUT, 0 },
};

static int qcom_pmic_pinctrl_get_pins_count(struct udevice *dev)
{
	struct qcom_pmic_gpio_data *plat = dev_get_plat(dev);

	return plat->pin_count;
}

static const char *qcom_pmic_pinctrl_get_pin_name(struct udevice *dev, unsigned int selector)
{
	static char name[8];

	/* DT indexes from 1 */
	snprintf(name, sizeof(name), "gpio%u", selector + 1);

	return name;
}

static int qcom_pmic_pinctrl_pinconf_set(struct udevice *dev, unsigned int selector,
					 unsigned int param, unsigned int arg)
{
	/* We only support configuring the pin as an output, either low or high */
	return _qcom_gpio_set_direction(dev, selector, false,
					param == PIN_CONFIG_OUTPUT_ENABLE);
}

static const char *qcom_pmic_pinctrl_get_function_name(struct udevice *dev, unsigned int selector)
{
	if (!selector)
		return "normal";
	return NULL;
}

static int qcom_pmic_pinctrl_generic_get_functions_count(struct udevice *dev)
{
	return 1;
}

static int qcom_pmic_pinctrl_generic_pinmux_set_mux(struct udevice *dev, unsigned int selector,
						    unsigned int func_selector)
{
	return 0;
}

struct pinctrl_ops qcom_pmic_pinctrl_ops = {
	.get_pins_count = qcom_pmic_pinctrl_get_pins_count,
	.get_pin_name = qcom_pmic_pinctrl_get_pin_name,
	.set_state = pinctrl_generic_set_state,
	.pinconf_num_params = ARRAY_SIZE(qcom_pmic_pinctrl_conf_params),
	.pinconf_params = qcom_pmic_pinctrl_conf_params,
	.pinconf_set = qcom_pmic_pinctrl_pinconf_set,
	.get_function_name = qcom_pmic_pinctrl_get_function_name,
	.get_functions_count = qcom_pmic_pinctrl_generic_get_functions_count,
	.pinmux_set = qcom_pmic_pinctrl_generic_pinmux_set_mux,
};

U_BOOT_DRIVER(qcom_pmic_pinctrl) = {
	.name	= "qcom_pmic_pinctrl",
	.id	= UCLASS_PINCTRL,
	.ops	= &qcom_pmic_pinctrl_ops,
};
