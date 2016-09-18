/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const struct uniphier_pinctrl_pin uniphier_sld8_pins[] = {
	UNIPHIER_PINCTRL_PIN(32, 8),
	UNIPHIER_PINCTRL_PIN(33, 8),
	UNIPHIER_PINCTRL_PIN(34, 8),
	UNIPHIER_PINCTRL_PIN(35, 8),
	UNIPHIER_PINCTRL_PIN(36, 8),
	UNIPHIER_PINCTRL_PIN(37, 8),
	UNIPHIER_PINCTRL_PIN(38, 8),
	UNIPHIER_PINCTRL_PIN(39, 8),
	UNIPHIER_PINCTRL_PIN(40, 9),
	UNIPHIER_PINCTRL_PIN(41, 0),
	UNIPHIER_PINCTRL_PIN(42, 0),
	UNIPHIER_PINCTRL_PIN(43, 0),
	UNIPHIER_PINCTRL_PIN(44, 0),
	UNIPHIER_PINCTRL_PIN(70, 0),
	UNIPHIER_PINCTRL_PIN(71, 0),
	UNIPHIER_PINCTRL_PIN(102, 10),
	UNIPHIER_PINCTRL_PIN(103, 10),
	UNIPHIER_PINCTRL_PIN(104, 11),
	UNIPHIER_PINCTRL_PIN(105, 11),
	UNIPHIER_PINCTRL_PIN(108, 13),
	UNIPHIER_PINCTRL_PIN(109, 13),
	UNIPHIER_PINCTRL_PIN(112, 0),
	UNIPHIER_PINCTRL_PIN(113, 0),
	UNIPHIER_PINCTRL_PIN(114, 0),
	UNIPHIER_PINCTRL_PIN(115, 0),
};

static const unsigned emmc_pins[] = {21, 22, 23, 24, 25, 26, 27};
static const int emmc_muxvals[] = {1, 1, 1, 1, 1, 1, 1};
static const unsigned emmc_dat8_pins[] = {28, 29, 30, 31};
static const int emmc_dat8_muxvals[] = {1, 1, 1, 1};
static const unsigned ether_mii_pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 13, 14,
					  61, 63, 64, 65, 66, 67, 68};
static const int ether_mii_muxvals[] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
					13, 13, 27, 27, 27, 27, 27, 27, 27};
static const unsigned ether_rmii_pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 13,
					   14};
static const int ether_rmii_muxvals[] = {13, 13, 13, 13, 13, 13, 13, 13, 13,
					 13, 13, 13};
static const unsigned i2c0_pins[] = {102, 103};
static const int i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {104, 105};
static const int i2c1_muxvals[] = {0, 0};
static const unsigned i2c2_pins[] = {108, 109};
static const int i2c2_muxvals[] = {2, 2};
static const unsigned i2c3_pins[] = {108, 109};
static const int i2c3_muxvals[] = {3, 3};
static const unsigned nand_pins[] = {15, 16, 17, 18, 19, 20, 21, 24, 25, 26,
				     27, 28, 29, 30, 31};
static const int nand_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned nand_cs1_pins[] = {22, 23};
static const int nand_cs1_muxvals[] = {0, 0};
static const unsigned sd_pins[] = {32, 33, 34, 35, 36, 37, 38, 39, 40};
static const int sd_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned system_bus_pins[] = {136, 137, 138, 139, 140, 141, 142,
					   143, 144, 145, 146, 147, 148, 149};
static const int system_bus_muxvals[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1,
					 -1, -1, -1, -1, -1};
static const unsigned system_bus_cs1_pins[] = {150};
static const int system_bus_cs1_muxvals[] = {-1};
static const unsigned system_bus_cs2_pins[] = {10};
static const int system_bus_cs2_muxvals[] = {1};
static const unsigned system_bus_cs3_pins[] = {11};
static const int system_bus_cs3_muxvals[] = {1};
static const unsigned system_bus_cs4_pins[] = {12};
static const int system_bus_cs4_muxvals[] = {1};
static const unsigned system_bus_cs5_pins[] = {13};
static const int system_bus_cs5_muxvals[] = {1};
static const unsigned uart0_pins[] = {70, 71};
static const int uart0_muxvals[] = {3, 3};
static const unsigned uart1_pins[] = {114, 115};
static const int uart1_muxvals[] = {0, 0};
static const unsigned uart2_pins[] = {112, 113};
static const int uart2_muxvals[] = {1, 1};
static const unsigned uart3_pins[] = {110, 111};
static const int uart3_muxvals[] = {1, 1};
static const unsigned usb0_pins[] = {41, 42};
static const int usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {43, 44};
static const int usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {114, 115};
static const int usb2_muxvals[] = {1, 1};

static const struct uniphier_pinctrl_group uniphier_sld8_groups[] = {
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
	UNIPHIER_PINCTRL_GROUP(system_bus),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs1),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs2),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs3),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs4),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs5),
	UNIPHIER_PINCTRL_GROUP_SPL(uart0),
	UNIPHIER_PINCTRL_GROUP_SPL(uart1),
	UNIPHIER_PINCTRL_GROUP_SPL(uart2),
	UNIPHIER_PINCTRL_GROUP_SPL(uart3),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
};

static const char * const uniphier_sld8_functions[] = {
	UNIPHIER_PINMUX_FUNCTION_SPL(emmc),
	UNIPHIER_PINMUX_FUNCTION(ether_mii),
	UNIPHIER_PINMUX_FUNCTION(ether_rmii),
	UNIPHIER_PINMUX_FUNCTION(i2c0),
	UNIPHIER_PINMUX_FUNCTION(i2c1),
	UNIPHIER_PINMUX_FUNCTION(i2c2),
	UNIPHIER_PINMUX_FUNCTION(i2c3),
	UNIPHIER_PINMUX_FUNCTION(nand),
	UNIPHIER_PINMUX_FUNCTION(sd),
	UNIPHIER_PINMUX_FUNCTION(system_bus),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart0),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart1),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart2),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart3),
	UNIPHIER_PINMUX_FUNCTION(usb0),
	UNIPHIER_PINMUX_FUNCTION(usb1),
	UNIPHIER_PINMUX_FUNCTION(usb2),
};

static struct uniphier_pinctrl_socdata uniphier_sld8_pinctrl_socdata = {
	.pins = uniphier_sld8_pins,
	.pins_count = ARRAY_SIZE(uniphier_sld8_pins),
	.groups = uniphier_sld8_groups,
	.groups_count = ARRAY_SIZE(uniphier_sld8_groups),
	.functions = uniphier_sld8_functions,
	.functions_count = ARRAY_SIZE(uniphier_sld8_functions),
};

static int uniphier_sld8_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_sld8_pinctrl_socdata);
}

static const struct udevice_id uniphier_sld8_pinctrl_match[] = {
	{ .compatible = "socionext,uniphier-sld8-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_sld8_pinctrl) = {
	.name = "uniphier-sld8-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_sld8_pinctrl_match),
	.probe = uniphier_sld8_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
};
