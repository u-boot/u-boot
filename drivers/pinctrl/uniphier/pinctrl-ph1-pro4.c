/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const struct uniphier_pinctrl_pin ph1_pro4_pins[] = {
};

static const unsigned emmc_pins[] = {40, 41, 42, 43, 51, 52, 53};
static const unsigned emmc_muxvals[] = {1, 1, 1, 1, 1, 1, 1};
static const unsigned emmc_dat8_pins[] = {44, 45, 46, 47};
static const unsigned emmc_dat8_muxvals[] = {1, 1, 1, 1};
static const unsigned i2c0_pins[] = {142, 143};
static const unsigned i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {144, 145};
static const unsigned i2c1_muxvals[] = {0, 0};
static const unsigned i2c2_pins[] = {146, 147};
static const unsigned i2c2_muxvals[] = {0, 0};
static const unsigned i2c3_pins[] = {148, 149};
static const unsigned i2c3_muxvals[] = {0, 0};
static const unsigned i2c6_pins[] = {308, 309};
static const unsigned i2c6_muxvals[] = {6, 6};
static const unsigned nand_pins[] = {40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
				     50, 51, 52, 53, 54};
static const unsigned nand_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0};
static const unsigned nand_cs1_pins[] = {131, 132};
static const unsigned nand_cs1_muxvals[] = {1, 1};
static const unsigned sd_pins[] = {150, 151, 152, 153, 154, 155, 156, 157, 158};
static const unsigned sd_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned sd1_pins[] = {319, 320, 321, 322, 323, 324, 325, 326,
				    327};
static const unsigned sd1_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned uart0_pins[] = {127, 128};
static const unsigned uart0_muxvals[] = {0, 0};
static const unsigned uart1_pins[] = {129, 130};
static const unsigned uart1_muxvals[] = {0, 0};
static const unsigned uart2_pins[] = {131, 132};
static const unsigned uart2_muxvals[] = {0, 0};
static const unsigned uart3_pins[] = {88, 89};
static const unsigned uart3_muxvals[] = {2, 2};
static const unsigned usb0_pins[] = {180, 181};
static const unsigned usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {182, 183};
static const unsigned usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {184, 185};
static const unsigned usb2_muxvals[] = {0, 0};
static const unsigned usb3_pins[] = {186, 187};
static const unsigned usb3_muxvals[] = {0, 0};

static const struct uniphier_pinctrl_group ph1_pro4_groups[] = {
	UNIPHIER_PINCTRL_GROUP(emmc),
	UNIPHIER_PINCTRL_GROUP(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(i2c1),
	UNIPHIER_PINCTRL_GROUP(i2c2),
	UNIPHIER_PINCTRL_GROUP(i2c3),
	UNIPHIER_PINCTRL_GROUP(i2c6),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(nand_cs1),
	UNIPHIER_PINCTRL_GROUP(sd),
	UNIPHIER_PINCTRL_GROUP(sd1),
	UNIPHIER_PINCTRL_GROUP(uart0),
	UNIPHIER_PINCTRL_GROUP(uart1),
	UNIPHIER_PINCTRL_GROUP(uart2),
	UNIPHIER_PINCTRL_GROUP(uart3),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
	UNIPHIER_PINCTRL_GROUP(usb3),
};

static const char * const ph1_pro4_functions[] = {
	"emmc",
	"i2c0",
	"i2c1",
	"i2c2",
	"i2c3",
	"i2c6",
	"nand",
	"sd",
	"sd1",
	"uart0",
	"uart1",
	"uart2",
	"uart3",
	"usb0",
	"usb1",
	"usb2",
	"usb3",
};

static struct uniphier_pinctrl_socdata ph1_pro4_pinctrl_socdata = {
	.pins = ph1_pro4_pins,
	.pins_count = ARRAY_SIZE(ph1_pro4_pins),
	.groups = ph1_pro4_groups,
	.groups_count = ARRAY_SIZE(ph1_pro4_groups),
	.functions = ph1_pro4_functions,
	.functions_count = ARRAY_SIZE(ph1_pro4_functions),
	.mux_bits = 4,
	.reg_stride = 8,
	.load_pinctrl = true,
};

static int ph1_pro4_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &ph1_pro4_pinctrl_socdata);
}

static const struct udevice_id ph1_pro4_pinctrl_match[] = {
	{ .compatible = "socionext,ph1-pro4-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(ph1_pro4_pinctrl) = {
	.name = "ph1-pro4-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(ph1_pro4_pinctrl_match),
	.probe = ph1_pro4_pinctrl_probe,
	.remove = uniphier_pinctrl_remove,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
