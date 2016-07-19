/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const struct uniphier_pinctrl_pin uniphier_ld4_pins[] = {
	UNIPHIER_PINCTRL_PIN(53, 0),
	UNIPHIER_PINCTRL_PIN(54, 0),
	UNIPHIER_PINCTRL_PIN(55, 0),
	UNIPHIER_PINCTRL_PIN(56, 0),
	UNIPHIER_PINCTRL_PIN(67, 0),
	UNIPHIER_PINCTRL_PIN(68, 0),
	UNIPHIER_PINCTRL_PIN(69, 0),
	UNIPHIER_PINCTRL_PIN(70, 0),
	UNIPHIER_PINCTRL_PIN(85, 0),
	UNIPHIER_PINCTRL_PIN(88, 0),
	UNIPHIER_PINCTRL_PIN(156, 0),
};

static const unsigned emmc_pins[] = {21, 22, 23, 24, 25, 26, 27};
static const int emmc_muxvals[] = {0, 1, 1, 1, 1, 1, 1};
static const unsigned emmc_dat8_pins[] = {28, 29, 30, 31};
static const int emmc_dat8_muxvals[] = {1, 1, 1, 1};
static const unsigned ether_mii_pins[] = {32, 33, 34, 35, 36, 37, 38, 39, 40,
					  41, 42, 43, 136, 137, 138, 139, 140,
					  141, 142};
static const int ether_mii_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					4, 4, 4, 4, 4, 4, 4};
static const unsigned ether_rmii_pins[] = {32, 33, 34, 35, 36, 37, 38, 39, 40,
					   41, 42, 43};
static const int ether_rmii_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned i2c0_pins[] = {102, 103};
static const int i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {104, 105};
static const int i2c1_muxvals[] = {0, 0};
static const unsigned i2c2_pins[] = {108, 109};
static const int i2c2_muxvals[] = {2, 2};
static const unsigned i2c3_pins[] = {108, 109};
static const int i2c3_muxvals[] = {3, 3};
static const unsigned nand_pins[] = {24, 25, 26, 27, 28, 29, 30, 31, 158, 159,
				     160, 161, 162, 163, 164};
static const int nand_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned nand_cs1_pins[] = {22, 23};
static const int nand_cs1_muxvals[] = {0, 0};
static const unsigned sd_pins[] = {44, 45, 46, 47, 48, 49, 50, 51, 52};
static const int sd_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned uart0_pins[] = {85, 88};
static const int uart0_muxvals[] = {1, 1};
static const unsigned uart1_pins[] = {155, 156};
static const int uart1_muxvals[] = {13, 13};
static const unsigned uart1b_pins[] = {69, 70};
static const int uart1b_muxvals[] = {23, 23};
static const unsigned uart2_pins[] = {128, 129};
static const int uart2_muxvals[] = {13, 13};
static const unsigned uart3_pins[] = {110, 111};
static const int uart3_muxvals[] = {1, 1};
static const unsigned usb0_pins[] = {53, 54};
static const int usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {55, 56};
static const int usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {155, 156};
static const int usb2_muxvals[] = {4, 4};
static const unsigned usb2b_pins[] = {67, 68};
static const int usb2b_muxvals[] = {23, 23};

static const struct uniphier_pinctrl_group uniphier_ld4_groups[] = {
	UNIPHIER_PINCTRL_GROUP_SPL(emmc),
	UNIPHIER_PINCTRL_GROUP_SPL(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(ether_mii),
	UNIPHIER_PINCTRL_GROUP(ether_rmii),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(i2c1),
	UNIPHIER_PINCTRL_GROUP(i2c2),
	UNIPHIER_PINCTRL_GROUP(i2c3),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(nand_cs1),
	UNIPHIER_PINCTRL_GROUP(sd),
	UNIPHIER_PINCTRL_GROUP_SPL(uart0),
	UNIPHIER_PINCTRL_GROUP_SPL(uart1),
	UNIPHIER_PINCTRL_GROUP_SPL(uart1b),
	UNIPHIER_PINCTRL_GROUP_SPL(uart2),
	UNIPHIER_PINCTRL_GROUP_SPL(uart3),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
	UNIPHIER_PINCTRL_GROUP(usb2b),
};

static const char * const uniphier_ld4_functions[] = {
	UNIPHIER_PINMUX_FUNCTION_SPL(emmc),
	UNIPHIER_PINMUX_FUNCTION(ether_mii),
	UNIPHIER_PINMUX_FUNCTION(ether_rmii),
	UNIPHIER_PINMUX_FUNCTION(i2c0),
	UNIPHIER_PINMUX_FUNCTION(i2c1),
	UNIPHIER_PINMUX_FUNCTION(i2c2),
	UNIPHIER_PINMUX_FUNCTION(i2c3),
	UNIPHIER_PINMUX_FUNCTION(nand),
	UNIPHIER_PINMUX_FUNCTION(sd),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart0),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart1),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart2),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart3),
	UNIPHIER_PINMUX_FUNCTION(usb0),
	UNIPHIER_PINMUX_FUNCTION(usb1),
	UNIPHIER_PINMUX_FUNCTION(usb2),
};

static struct uniphier_pinctrl_socdata uniphier_ld4_pinctrl_socdata = {
	.pins = uniphier_ld4_pins,
	.pins_count = ARRAY_SIZE(uniphier_ld4_pins),
	.groups = uniphier_ld4_groups,
	.groups_count = ARRAY_SIZE(uniphier_ld4_groups),
	.functions = uniphier_ld4_functions,
	.functions_count = ARRAY_SIZE(uniphier_ld4_functions),
};

static int uniphier_ld4_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_ld4_pinctrl_socdata);
}

static const struct udevice_id uniphier_ld4_pinctrl_match[] = {
	{ .compatible = "socionext,uniphier-ld4-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_ld4_pinctrl) = {
	.name = "uniphier-ld4-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_ld4_pinctrl_match),
	.probe = uniphier_ld4_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
};
