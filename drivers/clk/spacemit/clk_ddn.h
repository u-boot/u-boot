/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 SpacemiT Technology Co. Ltd
 * Copyright (c) 2024-2025 Haylen Chu <heylenay@4d2.org>
 * Copyright (c) 2025 Junhui Liu <junhui.liu@pigmoral.tech>
 * Copyright (c) 2025-2026 RISCstar Ltd.
 *
 *  Authors: Haylen Chu <heylenay@4d2.org>
 */

#ifndef _CLK_DDN_H_
#define _CLK_DDN_H_

#include <linux/clk-provider.h>

#include "clk_common.h"

struct ccu_ddn {
	struct ccu_common common;
	unsigned int num_mask;
	unsigned int num_shift;
	unsigned int den_mask;
	unsigned int den_shift;
	unsigned int pre_div;
};

#define CCU_DDN_MASK(_num_shift, _num_width)					\
	GENMASK((_num_shift) + (_num_width) - 1, _num_shift)

#define CCU_DDN_DEFINE(_id, _var, _name, _parent, _reg_ctrl, _num_mask,		\
		       _num_shift, _den_mask, _den_shift, _pre_div, _flags)	\
static struct ccu_ddn _var = {							\
	.common = {								\
		.reg_ctrl	= _reg_ctrl,					\
		CCU_COMMON(_id, _name, _parent, spacemit_ddn_init, _flags)	\
	},									\
	.num_mask	= _num_mask,						\
	.num_shift	= _num_shift,						\
	.den_mask	= _den_mask,						\
	.den_shift	= _den_shift,						\
	.pre_div	= _pre_div,						\
}

static inline struct ccu_ddn *clk_to_ccu_ddn(struct clk *clk)
{
	struct ccu_common *common = clk_to_ccu_common(clk);

	return container_of(common, struct ccu_ddn, common);
}

int spacemit_ddn_init(struct ccu_common *common);

#endif /* _CLK_DDN_H_ */
