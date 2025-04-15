/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 *
 */

#ifndef __clk_SOPHGO_PLL_H__
#define __clk_SOPHGO_PLL_H__

#include <clk.h>

#include "clk-common.h"

struct cv1800b_clk_synthesizer {
	struct cv1800b_clk_regbit en;
	struct cv1800b_clk_regbit clk_half;
	u32 ctrl;
	u32 set;
};

struct cv1800b_clk_ipll {
	struct clk clk;
	const char *name;
	const char *parent_name;
	void __iomem *base;
	u32 pll_reg;
	struct cv1800b_clk_regbit pll_pwd;
	struct cv1800b_clk_regbit pll_status;
};

struct cv1800b_clk_fpll {
	struct cv1800b_clk_ipll ipll;
	struct cv1800b_clk_synthesizer syn;
};

#define CV1800B_IPLL(_id, _name, _parent_name, _pll_reg, _pll_pwd_offset,	\
		     _pll_pwd_shift, _pll_status_offset, _pll_status_shift,	\
		     _flags)							\
	{									\
		.clk = {							\
			.id = CV1800B_CLK_ID_TRANSFORM(_id),			\
			.flags = _flags,					\
		},								\
		.name = _name,							\
		.parent_name = _parent_name,					\
		.pll_reg = _pll_reg,						\
		.pll_pwd = CV1800B_CLK_REGBIT(_pll_pwd_offset, _pll_pwd_shift),	\
		.pll_status = CV1800B_CLK_REGBIT(_pll_status_offset,		\
						 _pll_status_shift),		\
	}

#define CV1800B_FPLL(_id, _name, _parent_name, _pll_reg, _pll_pwd_offset,	\
		     _pll_pwd_shift, _pll_status_offset, _pll_status_shift,	\
		     _syn_en_offset, _syn_en_shift, _syn_clk_half_offset,	\
		     _syn_clk_half_shift, _syn_ctrl_offset, _syn_set_offset,	\
		     _flags)							\
	{									\
		.ipll = CV1800B_IPLL(_id, _name, _parent_name, _pll_reg,	\
				     _pll_pwd_offset, _pll_pwd_shift,		\
				     _pll_status_offset, _pll_status_shift,	\
				     _flags),					\
		.syn = {							\
			.en = CV1800B_CLK_REGBIT(_syn_en_offset, _syn_en_shift),\
			.clk_half = CV1800B_CLK_REGBIT(_syn_clk_half_offset,	\
						       _syn_clk_half_shift),	\
			.ctrl = _syn_ctrl_offset,				\
			.set = _syn_set_offset,					\
		},								\
	}

extern const struct clk_ops cv1800b_ipll_ops;
extern const struct clk_ops cv1800b_fpll_ops;

#endif /* __clk_SOPHGO_PLL_H__ */
