/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 *
 */

#ifndef __CLK_SOPHGO_IP_H__
#define __CLK_SOPHGO_IP_H__

#include <clk.h>

#include "clk-common.h"

struct cv1800b_mmux_parent_info {
	const char *name;
	u8 clk_sel;
	u8 index;
};

struct cv1800b_clk_gate {
	struct clk clk;
	const char *name;
	const char *parent_name;
	void __iomem *base;
	struct cv1800b_clk_regbit gate;
};

struct cv1800b_clk_div {
	struct clk clk;
	const char *name;
	const char *parent_name;
	void __iomem *base;
	struct cv1800b_clk_regbit gate;
	struct cv1800b_clk_regfield div;
	int div_init;
};

struct cv1800b_clk_bypass_div {
	struct cv1800b_clk_div div;
	struct cv1800b_clk_regbit bypass;
};

struct cv1800b_clk_fixed_div {
	struct clk clk;
	const char *name;
	const char *parent_name;
	void __iomem *base;
	struct cv1800b_clk_regbit gate;
	int div;
};

struct cv1800b_clk_bypass_fixed_div {
	struct cv1800b_clk_fixed_div div;
	struct cv1800b_clk_regbit bypass;
};

struct cv1800b_clk_mux {
	struct clk clk;
	const char *name;
	const char * const *parent_names;
	u8 num_parents;
	void __iomem *base;
	struct cv1800b_clk_regbit gate;
	struct cv1800b_clk_regfield div;
	int div_init;
	struct cv1800b_clk_regfield mux;
};

struct cv1800b_clk_bypass_mux {
	struct cv1800b_clk_mux mux;
	struct cv1800b_clk_regbit bypass;
};

struct cv1800b_clk_mmux {
	struct clk clk;
	const char *name;
	const struct cv1800b_mmux_parent_info *parent_infos;
	u8 num_parents;
	void __iomem *base;
	struct cv1800b_clk_regbit gate;
	struct cv1800b_clk_regfield div[2];
	int div_init[2];
	struct cv1800b_clk_regfield mux[2];
	struct cv1800b_clk_regbit bypass;
	struct cv1800b_clk_regbit clk_sel;
};

struct cv1800b_clk_audio {
	struct clk clk;
	const char *name;
	const char *parent_name;
	void __iomem *base;
	struct cv1800b_clk_regbit src_en;
	struct cv1800b_clk_regbit output_en;
	struct cv1800b_clk_regbit div_en;
	struct cv1800b_clk_regbit div_up;
	struct cv1800b_clk_regfield m;
	struct cv1800b_clk_regfield n;
};

#define CV1800B_GATE(_id, _name, _parent,				\
		     _gate_offset, _gate_shift,				\
		     _flags)						\
	{								\
		.clk = {						\
			.id = CV1800B_CLK_ID_TRANSFORM(_id),		\
			.flags = _flags,				\
		},							\
		.name = _name,						\
		.parent_name = _parent,					\
		.gate = CV1800B_CLK_REGBIT(_gate_offset, _gate_shift),	\
	}

#define CV1800B_DIV(_id, _name, _parent,				\
		    _gate_offset, _gate_shift,				\
		    _div_offset, _div_shift, _div_width,		\
		    _div_init, _flags)					\
	{								\
		.clk = {						\
			.id = CV1800B_CLK_ID_TRANSFORM(_id),		\
			.flags = _flags,				\
		},							\
		.name = _name,						\
		.parent_name = _parent,					\
		.gate = CV1800B_CLK_REGBIT(_gate_offset, _gate_shift),	\
		.div = CV1800B_CLK_REGFIELD(_div_offset, _div_shift,	\
					    _div_width),		\
		.div_init = _div_init,					\
	}

#define CV1800B_BYPASS_DIV(_id, _name, _parent,				\
			   _gate_offset, _gate_shift,			\
			   _div_offset, _div_shift,			\
			   _div_width, _div_init,			\
			   _bypass_offset, _bypass_shift,		\
			   _flags)					\
	{								\
		.div = CV1800B_DIV(_id, _name, _parent,			\
				   _gate_offset, _gate_shift,		\
				   _div_offset, _div_shift, _div_width,	\
				   _div_init, _flags),			\
		.bypass = CV1800B_CLK_REGBIT(_bypass_offset,		\
					     _bypass_shift),		\
	}

#define CV1800B_FIXED_DIV(_id, _name, _parent,				\
			  _gate_offset, _gate_shift,			\
			  _div, _flags)					\
	{								\
		.clk = {						\
			.id = CV1800B_CLK_ID_TRANSFORM(_id),		\
			.flags = _flags,				\
		},							\
		.name = _name,						\
		.parent_name = _parent,					\
		.gate = CV1800B_CLK_REGBIT(_gate_offset, _gate_shift),	\
		.div = _div,						\
	}

#define CV1800B_BYPASS_FIXED_DIV(_id, _name, _parent,			\
				 _gate_offset, _gate_shift,		\
				 _div,					\
				 _bypass_offset, _bypass_shift,		\
				 _flags)				\
	{								\
		.div = CV1800B_FIXED_DIV(_id, _name, _parent,		\
					 _gate_offset, _gate_shift,	\
					 _div, _flags),			\
		.bypass = CV1800B_CLK_REGBIT(_bypass_offset,		\
					     _bypass_shift)		\
	}

#define CV1800B_MUX(_id, _name, _parents,				\
		    _gate_offset, _gate_shift,				\
		    _div_offset, _div_shift, _div_width, _div_init,	\
		    _mux_offset, _mux_shift, _mux_width,		\
		    _flags)						\
	{								\
		.clk = {						\
			.id = CV1800B_CLK_ID_TRANSFORM(_id),		\
			.flags = _flags,				\
		},							\
		.name = _name,						\
		.parent_names = _parents,				\
		.num_parents = ARRAY_SIZE(_parents),			\
		.gate = CV1800B_CLK_REGBIT(_gate_offset, _gate_shift),	\
		.div = CV1800B_CLK_REGFIELD(_div_offset, _div_shift,	\
					    _div_width),		\
		.div_init = _div_init,					\
		.mux = CV1800B_CLK_REGFIELD(_mux_offset, _mux_shift,	\
					    _mux_width),		\
	}

#define CV1800B_BYPASS_MUX(_id, _name, _parents,			\
			   _gate_offset, _gate_shift,			\
			   _div_offset, _div_shift,			\
			   _div_width, _div_init,			\
			   _mux_offset, _mux_shift, _mux_width,		\
			   _bypass_offset, _bypass_shift,		\
			   _flags)					\
	{								\
		.mux = CV1800B_MUX(_id, _name, _parents,		\
				   _gate_offset, _gate_shift,		\
				   _div_offset, _div_shift,		\
				   _div_width, _div_init,		\
				   _mux_offset, _mux_shift, _mux_width,	\
				   _flags),				\
		.bypass = CV1800B_CLK_REGBIT(_bypass_offset,		\
					     _bypass_shift),		\
	}

#define CV1800B_MMUX(_id, _name, _parents,				\
		     _gate_offset, _gate_shift,				\
		     _div0_offset, _div0_shift, _div0_width, _div0_init,\
		     _div1_offset, _div1_shift, _div1_width, _div1_init,\
		     _mux0_offset, _mux0_shift, _mux0_width,		\
		     _mux1_offset, _mux1_shift, _mux1_width,		\
		     _bypass_offset, _bypass_shift,			\
		     _clk_sel_offset, _clk_sel_shift,			\
		     _flags)						\
	{								\
		.clk = {						\
			.id = CV1800B_CLK_ID_TRANSFORM(_id),		\
			.flags = _flags,				\
		},							\
		.name = _name,						\
		.parent_infos = _parents,				\
		.num_parents = ARRAY_SIZE(_parents),			\
		.gate = CV1800B_CLK_REGBIT(_gate_offset, _gate_shift),	\
		.div = {						\
			CV1800B_CLK_REGFIELD(_div0_offset, _div0_shift,	\
					     _div0_width),		\
			CV1800B_CLK_REGFIELD(_div1_offset, _div1_shift,	\
					     _div1_width),		\
		},							\
		.div_init = { _div0_init, _div1_init },			\
		.mux = {						\
			CV1800B_CLK_REGFIELD(_mux0_offset, _mux0_shift,	\
					     _mux0_width),		\
			CV1800B_CLK_REGFIELD(_mux1_offset, _mux1_shift,	\
					     _mux1_width),		\
		},							\
		.bypass = CV1800B_CLK_REGBIT(_bypass_offset,		\
					     _bypass_shift),		\
		.clk_sel = CV1800B_CLK_REGBIT(_clk_sel_offset,		\
					      _clk_sel_shift),		\
	}

#define CV1800B_AUDIO(_id, _name, _parent,				\
		      _src_en_offset, _src_en_shift,			\
		      _output_en_offset, _output_en_shift,		\
		      _div_en_offset, _div_en_shift,			\
		      _div_up_offset, _div_up_shift,			\
		      _m_offset, _m_shift, _m_width,			\
		      _n_offset, _n_shift, _n_width,			\
		      _flags)						\
	{								\
		.clk = {						\
			.id = CV1800B_CLK_ID_TRANSFORM(_id),		\
			.flags = _flags,				\
		},							\
		.name = _name,						\
		.parent_name = _parent,					\
		.src_en = CV1800B_CLK_REGBIT(_src_en_offset,		\
					     _src_en_shift),		\
		.output_en = CV1800B_CLK_REGBIT(_output_en_offset,	\
						_output_en_shift),	\
		.div_en = CV1800B_CLK_REGBIT(_div_en_offset,		\
					     _div_en_shift),		\
		.div_up = CV1800B_CLK_REGBIT(_div_up_offset,		\
					     _div_up_shift),		\
		.m = CV1800B_CLK_REGFIELD(_m_offset, _m_shift,		\
					  _m_width),			\
		.n = CV1800B_CLK_REGFIELD(_n_offset, _n_shift,		\
					  _n_width),			\
	}

extern const struct clk_ops cv1800b_clk_gate_ops;
extern const struct clk_ops cv1800b_clk_div_ops;
extern const struct clk_ops cv1800b_clk_bypass_div_ops;
extern const struct clk_ops cv1800b_clk_fixed_div_ops;
extern const struct clk_ops cv1800b_clk_bypass_fixed_div_ops;
extern const struct clk_ops cv1800b_clk_mux_ops;
extern const struct clk_ops cv1800b_clk_bypass_mux_ops;
extern const struct clk_ops cv1800b_clk_mmux_ops;
extern const struct clk_ops cv1800b_clk_audio_ops;

#endif /* __CLK_SOPHGO_IP_H__ */
