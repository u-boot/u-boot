/*
 * Copyright (c) 2012 Samsung Electronics.
 * Abhilash Kesavan <a.kesavan@samsung.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sromc.h>

static void exynos5_uart_config(int peripheral)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();
	struct s5p_gpio_bank *bank;
	int i, start, count;

	switch (peripheral) {
	case PERIPH_ID_UART0:
		bank = &gpio1->a0;
		start = 0;
		count = 4;
		break;
	case PERIPH_ID_UART1:
		bank = &gpio1->d0;
		start = 0;
		count = 4;
		break;
	case PERIPH_ID_UART2:
		bank = &gpio1->a1;
		start = 0;
		count = 4;
		break;
	case PERIPH_ID_UART3:
		bank = &gpio1->a1;
		start = 4;
		count = 2;
		break;
	}
	for (i = start; i < start + count; i++) {
		s5p_gpio_set_pull(bank, i, GPIO_PULL_NONE);
		s5p_gpio_cfg_pin(bank, i, GPIO_FUNC(0x2));
	}
}

static int exynos5_mmc_config(int peripheral, int flags)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();
	struct s5p_gpio_bank *bank, *bank_ext;
	int i, start = 0, gpio_func = 0;

	switch (peripheral) {
	case PERIPH_ID_SDMMC0:
		bank = &gpio1->c0;
		bank_ext = &gpio1->c1;
		start = 0;
		gpio_func = GPIO_FUNC(0x2);
		break;
	case PERIPH_ID_SDMMC1:
		bank = &gpio1->c2;
		bank_ext = NULL;
		break;
	case PERIPH_ID_SDMMC2:
		bank = &gpio1->c3;
		bank_ext = &gpio1->c4;
		start = 3;
		gpio_func = GPIO_FUNC(0x3);
		break;
	case PERIPH_ID_SDMMC3:
		bank = &gpio1->c4;
		bank_ext = NULL;
		break;
	}
	if ((flags & PINMUX_FLAG_8BIT_MODE) && !bank_ext) {
		debug("SDMMC device %d does not support 8bit mode",
				peripheral);
		return -1;
	}
	if (flags & PINMUX_FLAG_8BIT_MODE) {
		for (i = start; i <= (start + 3); i++) {
			s5p_gpio_cfg_pin(bank_ext, i, gpio_func);
			s5p_gpio_set_pull(bank_ext, i, GPIO_PULL_UP);
			s5p_gpio_set_drv(bank_ext, i, GPIO_DRV_4X);
		}
	}
	for (i = 0; i < 2; i++) {
		s5p_gpio_cfg_pin(bank, i, GPIO_FUNC(0x2));
		s5p_gpio_set_pull(bank, i, GPIO_PULL_NONE);
		s5p_gpio_set_drv(bank, i, GPIO_DRV_4X);
	}
	for (i = 3; i <= 6; i++) {
		s5p_gpio_cfg_pin(bank, i, GPIO_FUNC(0x2));
		s5p_gpio_set_pull(bank, i, GPIO_PULL_UP);
		s5p_gpio_set_drv(bank, i, GPIO_DRV_4X);
	}
	return 0;
}

static void exynos5_sromc_config(int flags)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();
	int i;

	/*
	 * SROM:CS1 and EBI
	 *
	 * GPY0[0]	SROM_CSn[0]
	 * GPY0[1]	SROM_CSn[1](2)
	 * GPY0[2]	SROM_CSn[2]
	 * GPY0[3]	SROM_CSn[3]
	 * GPY0[4]	EBI_OEn(2)
	 * GPY0[5]	EBI_EEn(2)
	 *
	 * GPY1[0]	EBI_BEn[0](2)
	 * GPY1[1]	EBI_BEn[1](2)
	 * GPY1[2]	SROM_WAIT(2)
	 * GPY1[3]	EBI_DATA_RDn(2)
	 */
	s5p_gpio_cfg_pin(&gpio1->y0, (flags & PINMUX_FLAG_BANK),
				GPIO_FUNC(2));
	s5p_gpio_cfg_pin(&gpio1->y0, 4, GPIO_FUNC(2));
	s5p_gpio_cfg_pin(&gpio1->y0, 5, GPIO_FUNC(2));

	for (i = 0; i < 4; i++)
		s5p_gpio_cfg_pin(&gpio1->y1, i, GPIO_FUNC(2));

	/*
	 * EBI: 8 Addrss Lines
	 *
	 * GPY3[0]	EBI_ADDR[0](2)
	 * GPY3[1]	EBI_ADDR[1](2)
	 * GPY3[2]	EBI_ADDR[2](2)
	 * GPY3[3]	EBI_ADDR[3](2)
	 * GPY3[4]	EBI_ADDR[4](2)
	 * GPY3[5]	EBI_ADDR[5](2)
	 * GPY3[6]	EBI_ADDR[6](2)
	 * GPY3[7]	EBI_ADDR[7](2)
	 *
	 * EBI: 16 Data Lines
	 *
	 * GPY5[0]	EBI_DATA[0](2)
	 * GPY5[1]	EBI_DATA[1](2)
	 * GPY5[2]	EBI_DATA[2](2)
	 * GPY5[3]	EBI_DATA[3](2)
	 * GPY5[4]	EBI_DATA[4](2)
	 * GPY5[5]	EBI_DATA[5](2)
	 * GPY5[6]	EBI_DATA[6](2)
	 * GPY5[7]	EBI_DATA[7](2)
	 *
	 * GPY6[0]	EBI_DATA[8](2)
	 * GPY6[1]	EBI_DATA[9](2)
	 * GPY6[2]	EBI_DATA[10](2)
	 * GPY6[3]	EBI_DATA[11](2)
	 * GPY6[4]	EBI_DATA[12](2)
	 * GPY6[5]	EBI_DATA[13](2)
	 * GPY6[6]	EBI_DATA[14](2)
	 * GPY6[7]	EBI_DATA[15](2)
	 */
	for (i = 0; i < 8; i++) {
		s5p_gpio_cfg_pin(&gpio1->y3, i, GPIO_FUNC(2));
		s5p_gpio_set_pull(&gpio1->y3, i, GPIO_PULL_UP);

		s5p_gpio_cfg_pin(&gpio1->y5, i, GPIO_FUNC(2));
		s5p_gpio_set_pull(&gpio1->y5, i, GPIO_PULL_UP);

		s5p_gpio_cfg_pin(&gpio1->y6, i, GPIO_FUNC(2));
		s5p_gpio_set_pull(&gpio1->y6, i, GPIO_PULL_UP);
	}
}

static void exynos5_i2c_config(int peripheral, int flags)
{

	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();

	switch (peripheral) {
	case PERIPH_ID_I2C0:
		s5p_gpio_cfg_pin(&gpio1->b3, 0, GPIO_FUNC(0x2));
		s5p_gpio_cfg_pin(&gpio1->b3, 1, GPIO_FUNC(0x2));
		break;
	case PERIPH_ID_I2C1:
		s5p_gpio_cfg_pin(&gpio1->b3, 2, GPIO_FUNC(0x2));
		s5p_gpio_cfg_pin(&gpio1->b3, 3, GPIO_FUNC(0x2));
		break;
	case PERIPH_ID_I2C2:
		s5p_gpio_cfg_pin(&gpio1->a0, 6, GPIO_FUNC(0x3));
		s5p_gpio_cfg_pin(&gpio1->a0, 7, GPIO_FUNC(0x3));
		break;
	case PERIPH_ID_I2C3:
		s5p_gpio_cfg_pin(&gpio1->a1, 2, GPIO_FUNC(0x3));
		s5p_gpio_cfg_pin(&gpio1->a1, 3, GPIO_FUNC(0x3));
		break;
	case PERIPH_ID_I2C4:
		s5p_gpio_cfg_pin(&gpio1->a2, 0, GPIO_FUNC(0x3));
		s5p_gpio_cfg_pin(&gpio1->a2, 1, GPIO_FUNC(0x3));
		break;
	case PERIPH_ID_I2C5:
		s5p_gpio_cfg_pin(&gpio1->a2, 2, GPIO_FUNC(0x3));
		s5p_gpio_cfg_pin(&gpio1->a2, 3, GPIO_FUNC(0x3));
		break;
	case PERIPH_ID_I2C6:
		s5p_gpio_cfg_pin(&gpio1->b1, 3, GPIO_FUNC(0x4));
		s5p_gpio_cfg_pin(&gpio1->b1, 4, GPIO_FUNC(0x4));
		break;
	case PERIPH_ID_I2C7:
		s5p_gpio_cfg_pin(&gpio1->b2, 2, GPIO_FUNC(0x3));
		s5p_gpio_cfg_pin(&gpio1->b2, 3, GPIO_FUNC(0x3));
		break;
	}
}

static int exynos5_pinmux_config(int peripheral, int flags)
{
	switch (peripheral) {
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
		exynos5_uart_config(peripheral);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
	case PERIPH_ID_SDMMC2:
	case PERIPH_ID_SDMMC3:
		return exynos5_mmc_config(peripheral, flags);
	case PERIPH_ID_SROMC:
		exynos5_sromc_config(flags);
		break;
	case PERIPH_ID_I2C0:
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
	case PERIPH_ID_I2C3:
	case PERIPH_ID_I2C4:
	case PERIPH_ID_I2C5:
	case PERIPH_ID_I2C6:
	case PERIPH_ID_I2C7:
		exynos5_i2c_config(peripheral, flags);
		break;
	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return -1;
	}

	return 0;
}

int exynos_pinmux_config(int peripheral, int flags)
{
	if (cpu_is_exynos5())
		return exynos5_pinmux_config(peripheral, flags);
	else {
		debug("pinmux functionality not supported\n");
		return -1;
	}
}
