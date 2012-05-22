/*
 *  (C) Copyright 2010-2012
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/mmc.h>
#include <asm/gpio.h>
#ifdef CONFIG_TEGRA_MMC
#include <mmc.h>
#endif

/*
 * Routine: gpio_config_uart
 * Description: Does nothing on Whistler - no UART-related GPIOs.
 */
void gpio_config_uart(void)
{
}

/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
static void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC3, FUNCMUX_SDMMC3_SDB_SLXA_8BIT);
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATC_ATD_8BIT);
}

/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	uchar val;
	int ret;

	debug("board_mmc_init called\n");

	/* Turn on MAX8907B LDO12 to 2.8V for J40 power */
	ret = i2c_set_bus_num(0);
	if (ret)
		printf("i2c_set_bus_num failed: %d\n", ret);
	val = 0x29;
	ret = i2c_write(0x3c, 0x46, 1, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x46 failed: %d\n", ret);
	val = 0x00;
	ret = i2c_write(0x3c, 0x45, 1, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x45 failed: %d\n", ret);
	val = 0x1f;
	ret = i2c_write(0x3c, 0x44, 1, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x44 failed: %d\n", ret);

	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();

	/* init dev 0 (SDMMC4), (J29 "HSMMC") with 8-bit bus */
	tegra2_mmc_init(0, 8, -1, -1);

	/* init dev 1 (SDMMC3), (J40 "SDIO3") with 8-bit bus */
	tegra2_mmc_init(1, 8, -1, -1);

	return 0;
}

/* this is a weak define that we are overriding */
void pin_mux_usb(void)
{
	uchar val;
	int ret;

	/*
	 * This is a hack. This should be represented in DT using the
	 * vbus-gpio property. However, U-Boot's DT support doesn't
	 * support any GPIO controller other than the Tegra's yet.
	 */

	/* Turn on TAC6416's GPIO 0+1 for USB1/3's VBUS */
	ret = i2c_set_bus_num(0);
	if (ret)
		printf("i2c_set_bus_num failed: %d\n", ret);
	val = 0x03;
	ret = i2c_write(0x20, 2, 1, &val, 1);
	if (ret)
		printf("i2c_write 0 0x20 2 failed: %d\n", ret);
	val = 0xfc;
	ret = i2c_write(0x20, 6, 1, &val, 1);
	if (ret)
		printf("i2c_write 0 0x20 6 failed: %d\n", ret);
}
