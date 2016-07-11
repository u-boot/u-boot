/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-uniphier.h"

static const unsigned emmc_pins[] = {18, 19, 20, 21, 22, 23, 24, 25};
static const unsigned emmc_muxvals[] = {0, 0, 0, 0, 0, 0, 0, 0};
static const unsigned emmc_dat8_pins[] = {26, 27, 28, 29};
static const unsigned emmc_dat8_muxvals[] = {0, 0, 0, 0};
static const unsigned i2c0_pins[] = {63, 64};
static const unsigned i2c0_muxvals[] = {0, 0};
static const unsigned i2c1_pins[] = {65, 66};
static const unsigned i2c1_muxvals[] = {0, 0};
static const unsigned i2c3_pins[] = {67, 68};
static const unsigned i2c3_muxvals[] = {1, 1};
static const unsigned i2c4_pins[] = {61, 62};
static const unsigned i2c4_muxvals[] = {1, 1};
static const unsigned nand_pins[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
				     15, 16, 17};
static const unsigned nand_muxvals[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
					2, 2, 2};
static const unsigned nand_cs1_pins[] = {};
static const unsigned nand_cs1_muxvals[] = {};
static const unsigned sd_pins[] = {10, 11, 12, 13, 14, 15, 16, 17};
static const unsigned sd_muxvals[] = {3, 3, 3, 3, 3, 3, 3, 3};  /* No SDVOLC */
static const unsigned uart0_pins[] = {54, 55};
static const unsigned uart0_muxvals[] = {0, 0};
static const unsigned uart1_pins[] = {58, 59};
static const unsigned uart1_muxvals[] = {1, 1};
static const unsigned uart2_pins[] = {90, 91};
static const unsigned uart2_muxvals[] = {1, 1};
static const unsigned uart3_pins[] = {94, 95};
static const unsigned uart3_muxvals[] = {1, 1};
static const unsigned usb0_pins[] = {46, 47};
static const unsigned usb0_muxvals[] = {0, 0};
static const unsigned usb1_pins[] = {48, 49};
static const unsigned usb1_muxvals[] = {0, 0};
static const unsigned usb2_pins[] = {50, 51};
static const unsigned usb2_muxvals[] = {0, 0};
static const unsigned usb3_pins[] = {52, 53};
static const unsigned usb3_muxvals[] = {0, 0};

static const struct uniphier_pinctrl_group uniphier_ld20_groups[] = {
	UNIPHIER_PINCTRL_GROUP(emmc),
	UNIPHIER_PINCTRL_GROUP(emmc_dat8),
	UNIPHIER_PINCTRL_GROUP(i2c0),
	UNIPHIER_PINCTRL_GROUP(i2c1),
	UNIPHIER_PINCTRL_GROUP(i2c3),
	UNIPHIER_PINCTRL_GROUP(i2c4),
	UNIPHIER_PINCTRL_GROUP(nand),
	UNIPHIER_PINCTRL_GROUP(nand_cs1),
	UNIPHIER_PINCTRL_GROUP(sd),	/* SD does not exist for LD11 */
	UNIPHIER_PINCTRL_GROUP(uart0),
	UNIPHIER_PINCTRL_GROUP(uart1),
	UNIPHIER_PINCTRL_GROUP(uart2),
	UNIPHIER_PINCTRL_GROUP(uart3),
	UNIPHIER_PINCTRL_GROUP(usb0),
	UNIPHIER_PINCTRL_GROUP(usb1),
	UNIPHIER_PINCTRL_GROUP(usb2),
	UNIPHIER_PINCTRL_GROUP(usb3),	/* USB3 does not exist for LD11 */
};

static const char * const uniphier_ld20_functions[] = {
	"emmc",
	"i2c0",
	"i2c1",
	"i2c3",
	"i2c4",
	"nand",
	"sd",		/* SD does not exist for LD11 */
	"uart0",
	"uart1",
	"uart2",
	"uart3",
	"usb0",
	"usb1",
	"usb2",
	"usb3",		/* USB3 does not exist for LD11 */
};

static struct uniphier_pinctrl_socdata uniphier_ld20_pinctrl_socdata = {
	.groups = uniphier_ld20_groups,
	.groups_count = ARRAY_SIZE(uniphier_ld20_groups),
	.functions = uniphier_ld20_functions,
	.functions_count = ARRAY_SIZE(uniphier_ld20_functions),
	.caps = UNIPHIER_PINCTRL_CAPS_PERPIN_IECTRL,
};

static int uniphier_ld20_pinctrl_probe(struct udevice *dev)
{
	return uniphier_pinctrl_probe(dev, &uniphier_ld20_pinctrl_socdata);
}

static const struct udevice_id uniphier_ld20_pinctrl_match[] = {
	{ .compatible = "socionext,ph1-ld11-pinctrl" },
	{ .compatible = "socionext,ph1-ld20-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_ld20_pinctrl) = {
	.name = "uniphier-ld20-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(uniphier_ld20_pinctrl_match),
	.probe = uniphier_ld20_pinctrl_probe,
	.remove = uniphier_pinctrl_remove,
	.priv_auto_alloc_size = sizeof(struct uniphier_pinctrl_priv),
	.ops = &uniphier_pinctrl_ops,
};
