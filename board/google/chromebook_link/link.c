/*
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cros_ec.h>
#include <asm/gpio.h>

int arch_early_init_r(void)
{
	if (cros_ec_board_init())
		return -1;

	return 0;
}

static const struct pch_gpio_set1 pch_gpio_set1_mode = {
	.gpio0 = GPIO_MODE_GPIO,  /* NMI_DBG# */
	.gpio3 = GPIO_MODE_GPIO,  /* ALS_INT# */
	.gpio5 = GPIO_MODE_GPIO,  /* SIM_DET */
	.gpio7 = GPIO_MODE_GPIO,  /* EC_SCI# */
	.gpio8 = GPIO_MODE_GPIO,  /* EC_SMI# */
	.gpio9 = GPIO_MODE_GPIO,  /* RECOVERY# */
	.gpio10 = GPIO_MODE_GPIO, /* SPD vector D3 */
	.gpio11 = GPIO_MODE_GPIO, /* smbalert#, let's keep it initialized */
	.gpio12 = GPIO_MODE_GPIO, /* TP_INT# */
	.gpio14 = GPIO_MODE_GPIO, /* Touch_INT_L */
	.gpio15 = GPIO_MODE_GPIO, /* EC_LID_OUT# (EC_WAKE#) */
	.gpio21 = GPIO_MODE_GPIO, /* EC_IN_RW */
	.gpio24 = GPIO_MODE_GPIO, /* DDR3L_EN */
	.gpio28 = GPIO_MODE_GPIO, /* SLP_ME_CSW_DEV# */
};

static const struct pch_gpio_set1 pch_gpio_set1_direction = {
	.gpio0 = GPIO_DIR_INPUT,
	.gpio3 = GPIO_DIR_INPUT,
	.gpio5 = GPIO_DIR_INPUT,
	.gpio7 = GPIO_DIR_INPUT,
	.gpio8 = GPIO_DIR_INPUT,
	.gpio9 = GPIO_DIR_INPUT,
	.gpio10 = GPIO_DIR_INPUT,
	.gpio11 = GPIO_DIR_INPUT,
	.gpio12 = GPIO_DIR_INPUT,
	.gpio14 = GPIO_DIR_INPUT,
	.gpio15 = GPIO_DIR_INPUT,
	.gpio21 = GPIO_DIR_INPUT,
	.gpio24 = GPIO_DIR_OUTPUT,
	.gpio28 = GPIO_DIR_INPUT,
};

static const struct pch_gpio_set1 pch_gpio_set1_level = {
	.gpio1 = GPIO_LEVEL_HIGH,
	.gpio6 = GPIO_LEVEL_HIGH,
	.gpio24 = GPIO_LEVEL_LOW,
};

static const struct pch_gpio_set1 pch_gpio_set1_invert = {
	.gpio7 = GPIO_INVERT,
	.gpio8 = GPIO_INVERT,
	.gpio12 = GPIO_INVERT,
	.gpio14 = GPIO_INVERT,
	.gpio15 = GPIO_INVERT,
};

static const struct pch_gpio_set2 pch_gpio_set2_mode = {
	.gpio36 = GPIO_MODE_GPIO, /* W_DISABLE_L */
	.gpio41 = GPIO_MODE_GPIO, /* SPD vector D0 */
	.gpio42 = GPIO_MODE_GPIO, /* SPD vector D1 */
	.gpio43 = GPIO_MODE_GPIO, /* SPD vector D2 */
	.gpio57 = GPIO_MODE_GPIO, /* PCH_SPI_WP_D */
	.gpio60 = GPIO_MODE_GPIO, /* DRAMRST_CNTRL_PCH */
};

static const struct pch_gpio_set2 pch_gpio_set2_direction = {
	.gpio36 = GPIO_DIR_OUTPUT,
	.gpio41 = GPIO_DIR_INPUT,
	.gpio42 = GPIO_DIR_INPUT,
	.gpio43 = GPIO_DIR_INPUT,
	.gpio57 = GPIO_DIR_INPUT,
	.gpio60 = GPIO_DIR_OUTPUT,
};

static const struct pch_gpio_set2 pch_gpio_set2_level = {
	.gpio36 = GPIO_LEVEL_HIGH,
	.gpio60 = GPIO_LEVEL_HIGH,
};

static const struct pch_gpio_set3 pch_gpio_set3_mode = {
};

static const struct pch_gpio_set3 pch_gpio_set3_direction = {
};

static const struct pch_gpio_set3 pch_gpio_set3_level = {
};

static const struct pch_gpio_map link_gpio_map = {
	.set1 = {
		.mode      = &pch_gpio_set1_mode,
		.direction = &pch_gpio_set1_direction,
		.level     = &pch_gpio_set1_level,
		.invert    = &pch_gpio_set1_invert,
	},
	.set2 = {
		.mode      = &pch_gpio_set2_mode,
		.direction = &pch_gpio_set2_direction,
		.level     = &pch_gpio_set2_level,
	},
	.set3 = {
		.mode      = &pch_gpio_set3_mode,
		.direction = &pch_gpio_set3_direction,
		.level     = &pch_gpio_set3_level,
	},
};

int board_early_init_f(void)
{
	ich_gpio_set_gpio_map(&link_gpio_map);

	return 0;
}
