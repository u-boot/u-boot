/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 SpacemiT Technology Co. Ltd
 * Copyright (c) 2024-2025 Haylen Chu <heylenay@4d2.org>
 * Copyright (c) 2025 Junhui Liu <junhui.liu@pigmoral.tech>
 * Copyright (c) 2025-2026 RISCstar Ltd.
 *
 *  Authors: Haylen Chu <heylenay@4d2.org>
 */

#ifndef _CLK_COMMON_H_
#define _CLK_COMMON_H_

#include <linux/clk-provider.h>

struct ccu_common;

typedef int (*ccu_init_fn)(struct ccu_common *common);

struct ccu_common {
	struct regmap *regmap;
	struct regmap *lock_regmap;
	const char *name;
	const char * const *parents;
	size_t num_parents;
	ccu_init_fn init;

	union {
		/* For DDN and MIX */
		struct {
			u32 reg_ctrl;
			u32 reg_fc;
			u32 mask_fc;
		};

		/* For PLL */
		struct {
			u32 reg_swcr1;
			u32 reg_swcr3;
		};
	};

	struct clk clk;
};

#define CCU_COMMON(_id, _name, _parent, _init, _flags)			\
	.name		= #_name,					\
	.parents	= (const char *[]) { _parent },			\
	.num_parents	= 1,						\
	.init		= _init,					\
	.clk		= { .flags = _flags, .id = _id, }		\

#define CCU_COMMON_PARENTS(_id, _name, _parents, _num_p, _init, _flags)	\
	.name		= #_name,					\
	.parents	= _parents,					\
	.num_parents	= _num_p,					\
	.init		= _init,					\
	.clk		= { .flags = _flags, .id = _id, }		\

static inline struct ccu_common *clk_to_ccu_common(struct clk *clk)
{
	return container_of(clk, struct ccu_common, clk);
}

#define ccu_read(c, reg)						\
	({								\
		struct ccu_common * const __ccu = (c);			\
		u32 tmp;						\
		regmap_read(__ccu->regmap, __ccu->reg_##reg, &tmp);	\
		tmp;							\
	 })
#define ccu_update(c, reg, mask, val) \
	({								\
		struct ccu_common * const __ccu = (c);			\
		regmap_update_bits(__ccu->regmap, __ccu->reg_##reg,	\
				   mask, val);				\
	})

#endif /* _CLK_COMMON_H_ */
