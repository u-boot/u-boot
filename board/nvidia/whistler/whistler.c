/*
 *  (C) Copyright 2010-2012
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/tegra.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/gpio.h>
#include <i2c.h>

#ifdef CONFIG_TEGRA_MMC
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
void pin_mux_mmc(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/* Turn on MAX8907B LDO12 to 2.8V for J40 power */
	ret = i2c_get_chip_for_busnum(0, 0x3c, 1, &dev);
	if (ret) {
		printf("%s: Cannot find MAX8907B I2C chip\n", __func__);
		return;
	}
	val = 0x29;
	ret = dm_i2c_write(dev, 0x46, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x46 failed: %d\n", ret);
	val = 0x00;
	ret = dm_i2c_write(dev, 0x45, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x45 failed: %d\n", ret);
	val = 0x1f;
	ret = dm_i2c_write(dev, 0x44, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x44 failed: %d\n", ret);

	funcmux_select(PERIPH_ID_SDMMC3, FUNCMUX_SDMMC3_SDB_SLXA_8BIT);
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATC_ATD_8BIT);
}
#endif

/* this is a weak define that we are overriding */
void pin_mux_usb(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/*
	 * This is a hack. This should be represented in DT using the
	 * vbus-gpio property. However, U-Boot's DT support doesn't
	 * support any GPIO controller other than the Tegra's yet.
	 */

	/* Turn on TAC6416's GPIO 0+1 for USB1/3's VBUS */
	ret = i2c_get_chip_for_busnum(0, 0x20, 1, &dev);
	if (ret) {
		printf("%s: Cannot find TAC6416 I2C chip\n", __func__);
		return;
	}
	val = 0x03;
	ret = dm_i2c_write(dev, 2, &val, 1);
	if (ret)
		printf("i2c_write 0 0x20 2 failed: %d\n", ret);
	val = 0xfc;
	ret = dm_i2c_write(dev, 6, &val, 1);
	if (ret)
		printf("i2c_write 0 0x20 6 failed: %d\n", ret);
}
