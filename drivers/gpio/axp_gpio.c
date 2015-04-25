/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * X-Powers AXP Power Management ICs gpio driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pmic_bus.h>
#include <errno.h>

#ifdef CONFIG_AXP152_POWER
#include <axp152.h>
#elif defined CONFIG_AXP209_POWER
#include <axp209.h>
#elif defined CONFIG_AXP221_POWER
#include <axp221.h>
#else
#error Unknown AXP model
#endif

static u8 axp_get_gpio_ctrl_reg(unsigned pin)
{
	switch (pin) {
	case 0: return AXP_GPIO0_CTRL;
	case 1: return AXP_GPIO1_CTRL;
#ifdef AXP_GPIO2_CTRL
	case 2: return AXP_GPIO2_CTRL;
#endif
#ifdef AXP_GPIO3_CTRL
	case 3: return AXP_GPIO3_CTRL;
#endif
	}
	return 0;
}

int axp_gpio_direction_input(struct udevice *dev, unsigned pin)
{
	u8 reg;

	switch (pin) {
#ifndef CONFIG_AXP152_POWER /* NA on axp152 */
	case SUNXI_GPIO_AXP0_VBUS_DETECT:
		return 0;
#endif
	default:
		reg = axp_get_gpio_ctrl_reg(pin);
		if (reg == 0)
			return -EINVAL;

		return pmic_bus_write(reg, AXP_GPIO_CTRL_INPUT);
	}
}

int axp_gpio_direction_output(struct udevice *dev, unsigned pin, int val)
{
	__maybe_unused int ret;
	u8 reg;

	switch (pin) {
#ifdef CONFIG_AXP221_POWER /* Only available on axp221/axp223 */
	case SUNXI_GPIO_AXP0_VBUS_ENABLE:
		ret = pmic_bus_clrbits(AXP221_MISC_CTRL,
				       AXP221_MISC_CTRL_N_VBUSEN_FUNC);
		if (ret)
			return ret;

		return axp_gpio_set_value(dev, pin, val);
#endif
	default:
		reg = axp_get_gpio_ctrl_reg(pin);
		if (reg == 0)
			return -EINVAL;

		return pmic_bus_write(reg, val ? AXP_GPIO_CTRL_OUTPUT_HIGH :
						 AXP_GPIO_CTRL_OUTPUT_LOW);
	}
}

int axp_gpio_get_value(struct udevice *dev, unsigned pin)
{
	u8 reg, val, mask;
	int ret;

	switch (pin) {
#ifndef CONFIG_AXP152_POWER /* NA on axp152 */
	case SUNXI_GPIO_AXP0_VBUS_DETECT:
		ret = pmic_bus_read(AXP_POWER_STATUS, &val);
		mask = AXP_POWER_STATUS_VBUS_PRESENT;
		break;
#endif
#ifdef CONFIG_AXP221_POWER /* Only available on axp221/axp223 */
	case SUNXI_GPIO_AXP0_VBUS_ENABLE:
		ret = pmic_bus_read(AXP221_VBUS_IPSOUT, &val);
		mask = AXP221_VBUS_IPSOUT_DRIVEBUS;
		break;
#endif
	default:
		reg = axp_get_gpio_ctrl_reg(pin);
		if (reg == 0)
			return -EINVAL;

		ret = pmic_bus_read(AXP_GPIO_STATE, &val);
		mask = 1 << (pin + AXP_GPIO_STATE_OFFSET);
	}
	if (ret)
		return ret;

	return (val & mask) ? 1 : 0;
}

int axp_gpio_set_value(struct udevice *dev, unsigned pin, int val)
{
	u8 reg;

	switch (pin) {
#ifdef CONFIG_AXP221_POWER /* Only available on axp221/axp223 */
	case SUNXI_GPIO_AXP0_VBUS_ENABLE:
		if (val)
			return pmic_bus_setbits(AXP221_VBUS_IPSOUT,
						AXP221_VBUS_IPSOUT_DRIVEBUS);
		else
			return pmic_bus_clrbits(AXP221_VBUS_IPSOUT,
						AXP221_VBUS_IPSOUT_DRIVEBUS);
#endif
	default:
		reg = axp_get_gpio_ctrl_reg(pin);
		if (reg == 0)
			return -EINVAL;

		return pmic_bus_write(reg, val ? AXP_GPIO_CTRL_OUTPUT_HIGH :
						 AXP_GPIO_CTRL_OUTPUT_LOW);
	}
}

int axp_gpio_init(void)
{
	int ret;

	ret = pmic_bus_init();
	if (ret)
		return ret;

	return 0;
}
