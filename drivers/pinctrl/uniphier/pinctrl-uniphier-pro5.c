/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const struct uniphier_pinctrl_pin uniphier_pro5_pins[] = {
	UNIPHIER_PINCTRL_PIN(47, 0),
	UNIPHIER_PINCTRL_PIN(48, 0),
	UNIPHIER_PINCTRL_PIN(49, 0),
	UNIPHIER_PINCTRL_PIN(50, 0),
	UNIPHIER_PINCTRL_PIN(53, 0),
	UNIPHIER_PINCTRL_PIN(54, 0),
	UNIPHIER_PINCTRL_PIN(87, 0),
	UNIPHIER_PINCTRL_PIN(88, 0),
	UNIPHIER_PINCTRL_PIN(101, 0),
	UNIPHIER_PINCTRL_PIN(102, 0),
};

static const unsigned emmc_pins[] = {36, 37, 38, 39, 40, 41, 42};
static const unsigned emmc_muxvals[] = {0, 0, 0, 0, 0, 0, 0};
static const unsigned emmc_dat8_pins[] = {43, 44, 45, 46};
static const unsigned emmc_dat8_muxvals[] = {0, 0, 0, 0};
static const unsigned i2c0_pins[] = {112, 113};
static const unsigned i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {114, 115};
static const unsigned i2c1_muxvals[] = {0, 0};
static const unsigned i2c2_pins[] = {116, 117};
static const unsigned i2c2_muxvals[] = {0, 0};
static const unsigned i2c3_pins[] = {118, 119};
static const unsigned i2c3_muxvals[] = {0, 0};
static const unsigned i2c5_pins[] = {87, 88};
static const unsigned i2c5_muxvals[] = {2, 2};
static const unsigned i2c5b_pins[] = {196, 197};
static const unsigned i2c5b_muxvals[] = {2, 2};
static const unsigned i2c5c_pins[] = {215, 216};
static const unsigned i2c5c_muxvals[] = {2, 2};
static const unsigned i2c6_pins[] = {101, 102};
static const unsigned i2c6_muxvals[] = {2, 2};
static const unsigned nand_pins[] = {19, 20, 21, 22, 23, 24, 25, 28, 29, 30,
				     31, 32, 33, 34, 35};
static const unsigned nand_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0};
static const unsigned nand_cs1_pins[] = {26, 27};
static const unsigned nand_cs1_muxvals[] = {0, 0};
static const unsigned sd_pins[] = {250, 251, 252, 253, 254, 255, 256, 257, 258};
static const unsigned sd_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned uart0_pins[] = {47, 48};
static const unsigned uart0_muxvals[] = {0, 0};
static const unsigned uart0b_pins[] = {227, 228};
static const unsigned uart0b_muxvals[] = {3, 3};
static const unsigned uart1_pins[] = {49, 50};
static const unsigned uart1_muxvals[] = {0, 0};
static const unsigned uart2_pins[] = {51, 52};
static const unsigned uart2_muxvals[] = {0, 0};
static const unsigned uart3_pins[] = {53, 54};
static const unsigned uart3_muxvals[] = {0, 0};
static const unsigned usb0_pins[] = {124, 125};
static const unsigned usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {126, 127};
static const unsigned usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {128, 129};
static const unsigned usb2_muxvals[] = {0, 0};

static const struct uniphier_pinctrl_group uniphier_pro5_groups[] = {
	UNIPHIER_PINCTRL_GROUP(emmc),
	UNIPHIER_PINCTRL_GROUP(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(i2c1),
	UNIPHIER_PINCTRL_GROUP(i2c2),
	UNIPHIER_PINCTRL_GROUP(i2c3),
	UNIPHIER_PINCTRL_GROUP(i2c5),
	UNIPHIER_PINCTRL_GROUP(i2c5b),
	UNIPHIER_PINCTRL_GROUP(i2c5c),
	UNIPHIER_PINCTRL_GROUP(i2c6),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(nand_cs1),
	UNIPHIER_PINCTRL_GROUP(sd),
	UNIPHIER_PINCTRL_GROUP(uart0),
	UNIPHIER_PINCTRL_GROUP(uart0b),
	UNIPHIER_PINCTRL_GROUP(uart1),
	UNIPHIER_PINCTRL_GROUP(uart2),
	UNIPHIER_PINCTRL_GROUP(uart3),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
};

static const char * const uniphier_pro5_functions[] = {
	"emmc",
	"i2c0",
	"i2c1",
	"i2c2",
	"i2c3",
	"i2c5",
	"i2c6",
	"nand",
	"sd",
	"uart0",
	"uart1",
	"uart2",
	"uart3",
	"usb0",
	"usb1",
	"usb2",
};

static struct uniphier_pinctrl_socdata uniphier_pro5_pinctrl_socdata = {
	.pins = uniphier_pro5_pins,
	.pins_count = ARRAY_SIZE(uniphier_pro5_pins),
	.groups = uniphier_pro5_groups,
	.groups_count = ARRAY_SIZE(uniphier_pro5_groups),
	.functions = uniphier_pro5_functions,
	.functions_count = ARRAY_SIZE(uniphier_pro5_functions),
	.caps = UNIPHIER_PINCTRL_CAPS_DBGMUX_SEPARATE,
};

static int uniphier_pro5_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_pro5_pinctrl_socdata);
}

static const struct udevice_id uniphier_pro5_pinctrl_match[] = {
	{ .compatible = "socionext,ph1-pro5-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_pro5_pinctrl) = {
	.name = "uniphier-pro5-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_pro5_pinctrl_match),
	.probe = uniphier_pro5_pinctrl_probe,
	.remove = uniphier_pinctrl_remove,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
