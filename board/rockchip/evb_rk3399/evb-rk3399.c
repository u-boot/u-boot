/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <ram.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <asm/arch/periph.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	struct udevice *pinctrl, *regulator;
	int ret;

	/*
	 * The PWM do not have decicated interrupt number in dts and can
	 * not get periph_id by pinctrl framework, so let's init them here.
	 * The PWM2 and PWM3 are for pwm regulater.
	 */
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		goto out;
	}

	/* Enable pwm0 for panel backlight */
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM0);
	if (ret) {
		debug("%s PWM0 pinctrl init fail! (ret=%d)\n", __func__, ret);
		goto out;
	}

	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM2);
	if (ret) {
		debug("%s PWM2 pinctrl init fail!\n", __func__);
		goto out;
	}

	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_PWM3);
	if (ret) {
		debug("%s PWM3 pinctrl init fail!\n", __func__);
		goto out;
	}

	ret = regulators_enable_boot_on(false);
	if (ret)
		debug("%s: Cannot enable boot on regulator\n", __func__);

	ret = regulator_get_by_platname("vcc5v0_host", &regulator);
	if (ret) {
		debug("%s vcc5v0_host init fail! ret %d\n", __func__, ret);
		goto out;
	}

	ret = regulator_set_enable(regulator, true);
	if (ret) {
		debug("%s vcc5v0-host-en set fail!\n", __func__);
		goto out;
	}

out:
	return 0;
}

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return ret;
	}
	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("Cannot get DRAM size: %d\n", ret);
		return ret;
	}
	debug("SDRAM base=%llx, size=%x\n", ram.base, (unsigned int)ram.size);
	gd->ram_size = ram.size;

	return 0;
}

int dram_init_banksize(void)
{
	/* Reserve 0x200000 for ATF bl31 */
	gd->bd->bi_dram[0].start = 0x200000;
	gd->bd->bi_dram[0].size = 0x7e000000;

	return 0;
}
