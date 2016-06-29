/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PINCTRL_UNIPHIER_H__
#define __PINCTRL_UNIPHIER_H__

#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/kernel.h>
#include <linux/types.h>

#define UNIPHIER_PINCTRL_PINMUX_BASE	0x1000
#define UNIPHIER_PINCTRL_LOAD_PINMUX	0x1700
#define UNIPHIER_PINCTRL_IECTRL		0x1d00

#define UNIPHIER_PIN_ATTR_PACKED(iectrl)	(iectrl)

static inline unsigned int uniphier_pin_get_iectrl(unsigned long data)
{
	return data;
}

/**
 * struct uniphier_pinctrl_pin - pin data for UniPhier SoC
 *
 * @number: pin number
 * @data: additional per-pin data
 */
struct uniphier_pinctrl_pin {
	unsigned number;
	unsigned long data;
};

/**
 * struct uniphier_pinctrl_group - pin group data for UniPhier SoC
 *
 * @name: pin group name
 * @pins: array of pins that belong to the group
 * @num_pins: number of pins in the group
 * @muxvals: array of values to be set to pinmux registers
 */
struct uniphier_pinctrl_group {
	const char *name;
	const unsigned *pins;
	unsigned num_pins;
	const int *muxvals;
};

/**
 * struct uniphier_pinctrl_socdata - SoC data for UniPhier pin controller
 *
 * @pins: array of pin data
 * @pins_count: number of pin data
 * @groups: array of pin group data
 * @groups_count: number of pin group data
 * @functions: array of pinmux function names
 * @functions_count: number of pinmux functions
 * @mux_bits: bit width of each pinmux register
 * @reg_stride: stride of pinmux register address
 * @caps: SoC-specific capability flag
 */
struct uniphier_pinctrl_socdata {
	const struct uniphier_pinctrl_pin *pins;
	int pins_count;
	const struct uniphier_pinctrl_group *groups;
	int groups_count;
	const char * const *functions;
	int functions_count;
	unsigned caps;
#define UNIPHIER_PINCTRL_CAPS_PERPIN_IECTRL	BIT(1)
#define UNIPHIER_PINCTRL_CAPS_DBGMUX_SEPARATE	BIT(0)
};

#define UNIPHIER_PINCTRL_PIN(a, b)					\
{									\
	.number = a,							\
	.data = UNIPHIER_PIN_ATTR_PACKED(b),				\
}

#define __UNIPHIER_PINCTRL_GROUP(grp)					\
	{								\
		.name = #grp,						\
		.pins = grp##_pins,					\
		.num_pins = ARRAY_SIZE(grp##_pins),			\
		.muxvals = grp##_muxvals +				\
			BUILD_BUG_ON_ZERO(ARRAY_SIZE(grp##_pins) !=	\
					  ARRAY_SIZE(grp##_muxvals)),	\
	}

#define __UNIPHIER_PINMUX_FUNCTION(func)	#func

#ifdef CONFIG_SPL_BUILD
#define UNIPHIER_PINCTRL_GROUP(grp)		{ .name = NULL }
#define UNIPHIER_PINMUX_FUNCTION(func)		NULL
#else
#define UNIPHIER_PINCTRL_GROUP(grp)		__UNIPHIER_PINCTRL_GROUP(grp)
#define UNIPHIER_PINMUX_FUNCTION(func)		__UNIPHIER_PINMUX_FUNCTION(func)
#endif

#define UNIPHIER_PINCTRL_GROUP_SPL(grp)		__UNIPHIER_PINCTRL_GROUP(grp)
#define UNIPHIER_PINMUX_FUNCTION_SPL(func)	__UNIPHIER_PINMUX_FUNCTION(func)

/**
 * struct uniphier_pinctrl_priv - private data for UniPhier pinctrl driver
 *
 * @base: base address of the pinctrl device
 * @socdata: SoC specific data
 */
struct uniphier_pinctrl_priv {
	void __iomem *base;
	struct uniphier_pinctrl_socdata *socdata;
};

extern const struct pinctrl_ops uniphier_pinctrl_ops;

int uniphier_pinctrl_probe(struct udevice *dev,
			   struct uniphier_pinctrl_socdata *socdata);

int uniphier_pinctrl_remove(struct udevice *dev);

#endif /* __PINCTRL_UNIPHIER_H__ */
