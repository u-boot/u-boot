/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018 - Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 - BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef CLK_MESON_H
#define CLK_MESON_H

/* Gate Structure */

#include <linux/bitops.h>
struct meson_gate {
	unsigned int reg;
	unsigned int bit;
};

#define MESON_GATE(id, _reg, _bit)		\
	[id] = {				\
		.reg = (_reg),			\
		.bit = (_bit),			\
	}

/* PLL Parameters */

struct parm {
	u16 reg_off;
	u8 shift;
	u8 width;
};

#define PMASK(width)                    GENMASK(width - 1, 0)
#define SETPMASK(width, shift)          GENMASK(shift + width - 1, shift)
#define CLRPMASK(width, shift)          (~SETPMASK(width, shift))

#define PARM_GET(width, shift, reg)                                     \
	(((reg) & SETPMASK(width, shift)) >> (shift))
#define PARM_SET(width, shift, reg, val)                                \
	(((reg) & CLRPMASK(width, shift)) | ((val) << (shift)))

#define SET_PARM_VALUE(_priv, _parm, _val)				\
	regmap_update_bits((_priv)->map, (_parm)->reg_off,		\
			   SETPMASK((_parm)->width, (_parm)->shift),	\
			   (_val) << (_parm)->shift)

#define GET_PARM_VALUE(_priv, _parm)					\
({									\
	uint _reg;							\
	regmap_read((_priv)->map, (_parm)->reg_off, &_reg);		\
	PARM_GET((_parm)->width, (_parm)->shift, _reg);			\
})

struct meson_clk {
	struct regmap *map;
};

/**
 * enum meson_clk_type - The type of clock
 * @MESON_CLK_ANY: Special value that matches any clock type
 * @MESON_CLK_GATE: This clock is a gate
 * @MESON_CLK_MUX: This clock is a multiplexer
 * @MESON_CLK_DIV: This clock is a configurable divider
 * @MESON_CLK_DIV2: This clock is a configurable power-of-two divider
 * @MESON_CLK_FIXED_DIV: This clock is a (fractional) fixed-factor clock
 * @MESON_CLK_EXTERNAL: This is an external clock from different clock provider
 * @MESON_CLK_PLL: This is a PLL
 */
enum meson_clk_type {
	MESON_CLK_ANY = 0,
	MESON_CLK_GATE,
	MESON_CLK_MUX,
	MESON_CLK_DIV,
	MESON_CLK_DIV2,
	MESON_CLK_FIXED_DIV,
	MESON_CLK_EXTERNAL,
	MESON_CLK_PLL,
};

/**
 * struct meson_clk_info - The parameters defining a clock
 * @name: Name of the clock
 * @parm: Register bits description for muxes and dividers
 * @div: Fixed divider value
 * @parents: List of parent clock IDs
 * @type: Clock type
 */
struct meson_clk_info {
	const char *name;
	union {
		const struct parm *parm;
		struct {
			u8 mult;
			u8 div;
		};
	};
	const unsigned int *parents;
	const enum meson_clk_type type;
};

/**
 * struct meson_clk_data - Clocks supported by clock provider
 * @num_clocks: Number of clocks
 * @clocks: Array of clock descriptions
 *
 */
struct meson_clk_data {
	const u8 num_clocks;
	const struct meson_clk_info **clocks;
};

/* Clock description initialization macros */

/* A multiplexer */
#define CLK_MUX(_name, _reg, _shift, _width, ...)			\
	(&(struct meson_clk_info){					\
		.parents = (const unsigned int[])__VA_ARGS__,		\
		.parm = &(struct parm) {				\
			.reg_off = (_reg),				\
			.shift = (_shift),				\
			.width = (_width),				\
		},							\
		.name = (_name),					\
		.type = MESON_CLK_MUX,					\
	})

#define _CLK_REG(_type, _name, _reg, _shift, _width, _parent)		\
	(&(struct meson_clk_info){					\
		.parents = (const unsigned int[]) { (_parent) },	\
		.parm = &(struct parm) {				\
			.reg_off = (_reg),				\
			.shift = (_shift),				\
			.width = (_width),				\
		},							\
		.name = (_name),					\
		.type = _type,						\
	})

/* A divider with an integral divisor */
#define CLK_DIV(name, reg, shift, width, parent)			\
	_CLK_REG(MESON_CLK_DIV, name, reg, shift, width, parent)

/* A divider with a power-of-two divisor */
#define CLK_DIV2(name, reg, shift, width, parent)			\
	_CLK_REG(MESON_CLK_DIV2, name, reg, shift, width, parent)

/* A fixed divider */
#define CLK_DIV_FIXED_FULL(_name, _mult, _div, _parent)			\
	(&(struct meson_clk_info){					\
		.parents = (const unsigned int[]) { (_parent) },	\
		.mult = (_mult),					\
		.div = (_div),						\
		.name = (_name),					\
		.type = MESON_CLK_FIXED_DIV,				\
	})
#define CLK_DIV_FIXED(name, div, parent)				\
	CLK_DIV_FIXED_FULL(name, 1, div, parent)

/* An external clock */
#define CLK_EXTERNAL(_name)						\
	(&(struct meson_clk_info){					\
		.name = (_name),					\
		.parents = (const unsigned int[]) { -ENOENT },		\
		.type = MESON_CLK_EXTERNAL,				\
	})

/* A clock gate */
#define CLK_GATE(name, reg, shift, parent)				\
	_CLK_REG(MESON_CLK_GATE, name, reg, shift, 1, parent)

/* A PLL clock */
#define CLK_PLL(_name, _parent, ...)					\
	(&(struct meson_clk_info){					\
		.name = (_name),					\
		.parents = (const unsigned int[]) { (_parent) },	\
		.parm = (const struct parm[])__VA_ARGS__,		\
		.type = MESON_CLK_PLL,					\
	})

/* MPLL Parameters */

#define SDM_DEN 16384
#define N2_MIN  4
#define N2_MAX  511

int meson_clk_enable(struct clk *clk);
int meson_clk_disable(struct clk *clk);
int meson_clk_get_parent(struct clk *clk);
ulong meson_clk_get_rate(struct clk *clk);
ulong meson_composite_set_rate(struct clk *clk, ulong rate);
ulong meson_mux_set_rate(struct clk *clk, ulong rate);
int meson_clk_set_parent(struct clk *clk, struct clk *parent);
#if IS_ENABLED(CONFIG_CMD_CLK)
void meson_clk_dump(struct udevice *dev);
#endif

#endif
