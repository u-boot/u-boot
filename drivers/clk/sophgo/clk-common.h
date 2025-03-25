/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 *
 */

#ifndef __CLK_SOPHGO_COMMON_H__
#define __CLK_SOPHGO_COMMON_H__

#include <linux/bitops.h>
#include <linux/io.h>

#define CV1800B_CLK_OSC 1
#define CV1800B_CLK_BYPASS 2
#define CV1800B_CLK_ID_TRANSFORM(_id) ((_id) + 3)

struct cv1800b_clk_regbit {
	u32 offset;
	u8 shift;
};

struct cv1800b_clk_regfield {
	u32 offset;
	u8 shift;
	u8 width;
};

#define CV1800B_CLK_REGBIT(_offset, _shift)	\
	{					\
		.offset = _offset,		\
		.shift = _shift,		\
	}

#define CV1800B_CLK_REGFIELD(_offset, _shift, _width)	\
	{						\
		.offset = _offset,			\
		.shift = _shift,			\
		.width = _width,			\
	}

static inline u32 cv1800b_clk_getbit(void *base, struct cv1800b_clk_regbit *bit)
{
	return readl(base + bit->offset) & (BIT(bit->shift));
}

static inline u32 cv1800b_clk_setbit(void *base, struct cv1800b_clk_regbit *bit)
{
	setbits_le32(base + bit->offset, BIT(bit->shift));
	return 0;
}

static inline u32 cv1800b_clk_clrbit(void *base, struct cv1800b_clk_regbit *bit)
{
	clrbits_le32(base + bit->offset, BIT(bit->shift));
	return 0;
}

static inline u32 cv1800b_clk_getfield(void *base,
				       struct cv1800b_clk_regfield *field)
{
	u32 mask = GENMASK(field->shift + field->width - 1, field->shift);

	return (readl(base + field->offset) & mask) >> field->shift;
}

static inline void
cv1800b_clk_setfield(void *base, struct cv1800b_clk_regfield *field, u32 val)
{
	u32 mask = GENMASK(field->shift + field->width - 1, field->shift);
	u32 new_val = (readl(base + field->offset) & ~mask) |
		      ((val << field->shift) & mask);

	return writel(new_val, base + field->offset);
}

#endif /* __CLK_SOPHGO_COMMON_H__ */
