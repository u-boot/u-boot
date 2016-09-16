/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const unsigned emmc_pins[] = {55, 56, 60};
static const int emmc_muxvals[] = {1, 1, 1};
static const unsigned emmc_dat8_pins[] = {57};
static const int emmc_dat8_muxvals[] = {1};
static const unsigned ether_mii_pins[] = {35, 107, 108, 109, 110, 111, 112,
					  113};
static const int ether_mii_muxvals[] = {1, 2, 2, 2, 2, 2, 2, 2};
static const unsigned ether_rmii_pins[] = {35};
static const int ether_rmii_muxvals[] = {1};
static const unsigned i2c0_pins[] = {36};
static const int i2c0_muxvals[] = {0};
static const unsigned nand_pins[] = {38, 39, 40, 58, 59};
static const int nand_muxvals[] = {1, 1, 1, 1, 1};
static const unsigned nand_cs1_pins[] = {41};
static const int nand_cs1_muxvals[] = {1};
static const unsigned sd_pins[] = {42, 43, 44, 45};
static const int sd_muxvals[] = {1, 1, 1, 1};
static const unsigned system_bus_pins[] = {46, 50, 51, 53, 54, 73, 74, 75, 76,
					   77, 78, 79, 80, 88, 89, 91, 92, 99};
static const int system_bus_muxvals[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					 1, 1, 1, 1, 1};
static const unsigned system_bus_cs0_pins[] = {93};
static const int system_bus_cs0_muxvals[] = {1};
static const unsigned system_bus_cs1_pins[] = {94};
static const int system_bus_cs1_muxvals[] = {1};
static const unsigned system_bus_cs2_pins[] = {95};
static const int system_bus_cs2_muxvals[] = {1};
static const unsigned system_bus_cs3_pins[] = {96};
static const int system_bus_cs3_muxvals[] = {1};
static const unsigned system_bus_cs4_pins[] = {81};
static const int system_bus_cs4_muxvals[] = {1};
static const unsigned system_bus_cs5_pins[] = {82};
static const int system_bus_cs5_muxvals[] = {1};
static const unsigned uart0_pins[] = {63, 64};
static const int uart0_muxvals[] = {0, 1};
static const unsigned uart1_pins[] = {65, 66};
static const int uart1_muxvals[] = {0, 1};
static const unsigned uart2_pins[] = {96, 102};
static const int uart2_muxvals[] = {2, 2};
static const unsigned usb0_pins[] = {13, 14};
static const int usb0_muxvals[] = {0, 1};
static const unsigned usb1_pins[] = {15, 16};
static const int usb1_muxvals[] = {0, 1};
static const unsigned usb2_pins[] = {17, 18};
static const int usb2_muxvals[] = {0, 1};
static const unsigned usb3_pins[] = {19, 20};
static const int usb3_muxvals[] = {0, 1};

static const struct uniphier_pinctrl_group uniphier_sld3_groups[] = {
	UNIPHIER_PINCTRL_GROUP_SPL(emmc),
	UNIPHIER_PINCTRL_GROUP_SPL(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(ether_mii),
	UNIPHIER_PINCTRL_GROUP(ether_rmii),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(nand_cs1),
	UNIPHIER_PINCTRL_GROUP(sd),
	UNIPHIER_PINCTRL_GROUP(system_bus),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs0),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs1),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs2),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs3),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs4),
	UNIPHIER_PINCTRL_GROUP(system_bus_cs5),
	UNIPHIER_PINCTRL_GROUP_SPL(uart0),
	UNIPHIER_PINCTRL_GROUP_SPL(uart1),
	UNIPHIER_PINCTRL_GROUP_SPL(uart2),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
	UNIPHIER_PINCTRL_GROUP(usb3)
};

static const char * const uniphier_sld3_functions[] = {
	UNIPHIER_PINMUX_FUNCTION_SPL(emmc),
	UNIPHIER_PINMUX_FUNCTION(ether_mii),
	UNIPHIER_PINMUX_FUNCTION(ether_rmii),
	UNIPHIER_PINMUX_FUNCTION(i2c0),
	UNIPHIER_PINMUX_FUNCTION(nand),
	UNIPHIER_PINMUX_FUNCTION(sd),
	UNIPHIER_PINMUX_FUNCTION(system_bus),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart0),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart1),
	UNIPHIER_PINMUX_FUNCTION_SPL(uart2),
	UNIPHIER_PINMUX_FUNCTION(usb0),
	UNIPHIER_PINMUX_FUNCTION(usb1),
	UNIPHIER_PINMUX_FUNCTION(usb2),
	UNIPHIER_PINMUX_FUNCTION(usb3),
};

static struct uniphier_pinctrl_socdata uniphier_sld3_pinctrl_socdata = {
	.groups = uniphier_sld3_groups,
	.groups_count = ARRAY_SIZE(uniphier_sld3_groups),
	.functions = uniphier_sld3_functions,
	.functions_count = ARRAY_SIZE(uniphier_sld3_functions),
	.caps = UNIPHIER_PINCTRL_CAPS_MUX_4BIT,
};

static int uniphier_sld3_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_sld3_pinctrl_socdata);
}

static const struct udevice_id uniphier_sld3_pinctrl_match[] = {
	{ .compatible = "socionext,uniphier-sld3-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_sld3_pinctrl) = {
	.name = "uniphier-sld3-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_sld3_pinctrl_match),
	.probe = uniphier_sld3_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
};
