// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * Sunxi PMIC bus access helpers
 *
 * The axp152 & axp209 use an i2c bus, the axp221 uses the p2wi bus and the
 * axp223 uses the rsb bus, these functions abstract this.
 */

#include <axp_pmic.h>
#include <dm.h>
#include <asm/arch/p2wi.h>
#include <asm/arch/rsb.h>
#include <i2c.h>
#include <power/pmic.h>
#include <asm/arch/pmic_bus.h>

#define AXP221_CHIP_ADDR		0x68

#if CONFIG_IS_ENABLED(PMIC_AXP)
static struct udevice *pmic;
#endif

int pmic_bus_init(void)
{
	/* This cannot be 0 because it is used in SPL before BSS is ready */
	static int needs_init = 1;
	int ret = 0;

	if (!needs_init)
		return 0;

#if CONFIG_IS_ENABLED(PMIC_AXP)
	ret = uclass_get_device_by_driver(UCLASS_PMIC, DM_DRIVER_GET(axp_pmic),
					  &pmic);
#else
	if (IS_ENABLED(CONFIG_SYS_I2C_SUN6I_P2WI)) {
		p2wi_init();
		ret = p2wi_change_to_p2wi_mode(AXP221_CHIP_ADDR,
					       AXP_PMIC_MODE_REG,
					       AXP_PMIC_MODE_P2WI);
	} else if (IS_ENABLED(CONFIG_SYS_I2C_SUN8I_RSB)) {
		ret = rsb_init();
		if (ret)
			return ret;

		ret = rsb_set_device_address(AXP_PMIC_PRI_DEVICE_ADDR,
					     AXP_PMIC_PRI_RUNTIME_ADDR);
	}
#endif

	needs_init = ret;

	return ret;
}

int pmic_bus_read(u8 reg, u8 *data)
{
#if CONFIG_IS_ENABLED(PMIC_AXP)
	return pmic_read(pmic, reg, data, 1);
#else
	if (IS_ENABLED(CONFIG_SYS_I2C_SUN6I_P2WI))
		return p2wi_read(reg, data);
	if (IS_ENABLED(CONFIG_SYS_I2C_SUN8I_RSB))
		return rsb_read(AXP_PMIC_PRI_RUNTIME_ADDR, reg, data);

	return i2c_read(CONFIG_AXP_I2C_ADDRESS, reg, 1, data, 1);
#endif
}

int pmic_bus_write(u8 reg, u8 data)
{
#if CONFIG_IS_ENABLED(PMIC_AXP)
	return pmic_write(pmic, reg, &data, 1);
#else
	if (IS_ENABLED(CONFIG_SYS_I2C_SUN6I_P2WI))
		return p2wi_write(reg, data);
	if (IS_ENABLED(CONFIG_SYS_I2C_SUN8I_RSB))
		return rsb_write(AXP_PMIC_PRI_RUNTIME_ADDR, reg, data);

	return i2c_write(CONFIG_AXP_I2C_ADDRESS, reg, 1, &data, 1);
#endif
}

int pmic_bus_setbits(u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(reg, &val);
	if (ret)
		return ret;

	if ((val & bits) == bits)
		return 0;

	val |= bits;
	return pmic_bus_write(reg, val);
}

int pmic_bus_clrbits(u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(reg, &val);
	if (ret)
		return ret;

	if (!(val & bits))
		return 0;

	val &= ~bits;
	return pmic_bus_write(reg, val);
}
