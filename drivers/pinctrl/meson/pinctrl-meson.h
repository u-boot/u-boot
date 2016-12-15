/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PINCTRL_MESON_H__
#define __PINCTRL_MESON_H__

#include <linux/types.h>

struct meson_pmx_group {
	const char *name;
	const unsigned int *pins;
	unsigned int num_pins;
	bool is_gpio;
	unsigned int reg;
	unsigned int bit;
};

struct meson_pmx_func {
	const char *name;
	const char * const *groups;
	unsigned int num_groups;
};

struct meson_pinctrl_data {
	const char *name;
	struct meson_pmx_group *groups;
	struct meson_pmx_func *funcs;
	unsigned int pin_base;
	unsigned int num_pins;
	unsigned int num_groups;
	unsigned int num_funcs;
};

struct meson_pinctrl {
	struct meson_pinctrl_data *data;
	void __iomem *reg_mux;
};

#define PIN(x, b)	(b + x)

#define GROUP(grp, r, b)						\
	{								\
		.name = #grp,						\
		.pins = grp ## _pins,					\
		.num_pins = ARRAY_SIZE(grp ## _pins),			\
		.reg = r,						\
		.bit = b,						\
	 }

#define GPIO_GROUP(gpio, b)						\
	{								\
		.name = #gpio,						\
		.pins = (const unsigned int[]){ PIN(gpio, b) },		\
		.num_pins = 1,						\
		.is_gpio = true,					\
	 }

#define FUNCTION(fn)							\
	{								\
		.name = #fn,						\
		.groups = fn ## _groups,				\
		.num_groups = ARRAY_SIZE(fn ## _groups),		\
	}

#define MESON_PIN(x, b) PINCTRL_PIN(PIN(x, b), #x)

extern const struct pinctrl_ops meson_pinctrl_ops;

int meson_pinctrl_probe(struct udevice *dev);

#endif /* __PINCTRL_MESON_H__ */
