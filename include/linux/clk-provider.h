/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (c) 2010-2011 Jeremy Kerr <jeremy.kerr@canonical.com>
 * Copyright (C) 2011-2012 Linaro Ltd <mturquette@linaro.org>
 */
#ifndef __LINUX_CLK_PROVIDER_H
#define __LINUX_CLK_PROVIDER_H
#include <clk-uclass.h>

static inline void clk_dm(ulong id, struct clk *clk)
{
	if (!IS_ERR(clk))
		clk->id = id;
}

/*
 * flags used across common struct clk.  these flags should only affect the
 * top-level framework.  custom flags for dealing with hardware specifics
 * belong in struct clk_foo
 *
 * Please update clk_flags[] in drivers/clk/clk.c when making changes here!
 */
#define CLK_SET_RATE_GATE	BIT(0) /* must be gated across rate change */
#define CLK_SET_PARENT_GATE	BIT(1) /* must be gated across re-parent */
#define CLK_SET_RATE_PARENT	BIT(2) /* propagate rate change up one level */
#define CLK_IGNORE_UNUSED	BIT(3) /* do not gate even if unused */
				/* unused */
#define CLK_IS_BASIC		BIT(5) /* Basic clk, can't do a to_clk_foo() */
#define CLK_GET_RATE_NOCACHE	BIT(6) /* do not use the cached clk rate */
#define CLK_SET_RATE_NO_REPARENT BIT(7) /* don't re-parent on rate change */
#define CLK_GET_ACCURACY_NOCACHE BIT(8) /* do not use the cached clk accuracy */
#define CLK_RECALC_NEW_RATES	BIT(9) /* recalc rates after notifications */
#define CLK_SET_RATE_UNGATE	BIT(10) /* clock needs to run to set rate */
#define CLK_IS_CRITICAL		BIT(11) /* do not gate, ever */
/* parents need enable during gate/ungate, set rate and re-parent */
#define CLK_OPS_PARENT_ENABLE	BIT(12)
/* duty cycle call may be forwarded to the parent clock */
#define CLK_DUTY_CYCLE_PARENT	BIT(13)

#define CLK_MUX_INDEX_ONE		BIT(0)
#define CLK_MUX_INDEX_BIT		BIT(1)
#define CLK_MUX_HIWORD_MASK		BIT(2)
#define CLK_MUX_READ_ONLY		BIT(3) /* mux can't be changed */
#define CLK_MUX_ROUND_CLOSEST		BIT(4)

struct clk_mux {
	struct clk	clk;
	void __iomem	*reg;
	u32		*table;
	u32		mask;
	u8		shift;
	u8		flags;

	/*
	 * Fields from struct clk_init_data - this struct has been
	 * omitted to avoid too deep level of CCF for bootloader
	 */
	const char	* const *parent_names;
	u8		num_parents;
#if CONFIG_IS_ENABLED(SANDBOX_CLK_CCF)
	u32             io_mux_val;
#endif

};

#define to_clk_mux(_clk) container_of(_clk, struct clk_mux, clk)
extern const struct clk_ops clk_mux_ops;
u8 clk_mux_get_parent(struct clk *clk);

struct clk_gate {
	struct clk	clk;
	void __iomem	*reg;
	u8		bit_idx;
	u8		flags;
#if CONFIG_IS_ENABLED(SANDBOX_CLK_CCF)
	u32		io_gate_val;
#endif
};

#define to_clk_gate(_clk) container_of(_clk, struct clk_gate, clk)

#define CLK_GATE_SET_TO_DISABLE		BIT(0)
#define CLK_GATE_HIWORD_MASK		BIT(1)

extern const struct clk_ops clk_gate_ops;
struct clk *clk_register_gate(struct device *dev, const char *name,
			      const char *parent_name, unsigned long flags,
			      void __iomem *reg, u8 bit_idx,
			      u8 clk_gate_flags, spinlock_t *lock);

struct clk_div_table {
	unsigned int	val;
	unsigned int	div;
};

struct clk_divider {
	struct clk	clk;
	void __iomem	*reg;
	u8		shift;
	u8		width;
	u8		flags;
	const struct clk_div_table	*table;
#if CONFIG_IS_ENABLED(SANDBOX_CLK_CCF)
	u32             io_divider_val;
#endif
};

#define clk_div_mask(width)	((1 << (width)) - 1)
#define to_clk_divider(_clk) container_of(_clk, struct clk_divider, clk)

#define CLK_DIVIDER_ONE_BASED		BIT(0)
#define CLK_DIVIDER_POWER_OF_TWO	BIT(1)
#define CLK_DIVIDER_ALLOW_ZERO		BIT(2)
#define CLK_DIVIDER_HIWORD_MASK		BIT(3)
#define CLK_DIVIDER_ROUND_CLOSEST	BIT(4)
#define CLK_DIVIDER_READ_ONLY		BIT(5)
#define CLK_DIVIDER_MAX_AT_ZERO		BIT(6)
extern const struct clk_ops clk_divider_ops;
unsigned long divider_recalc_rate(struct clk *hw, unsigned long parent_rate,
				  unsigned int val,
				  const struct clk_div_table *table,
				  unsigned long flags, unsigned long width);

struct clk_fixed_factor {
	struct clk	clk;
	unsigned int	mult;
	unsigned int	div;
};

#define to_clk_fixed_factor(_clk) container_of(_clk, struct clk_fixed_factor,\
					       clk)

struct clk_fixed_rate {
	struct clk clk;
	unsigned long fixed_rate;
};

#define to_clk_fixed_rate(dev)	((struct clk_fixed_rate *)dev_get_platdata(dev))

struct clk_composite {
	struct clk	clk;
	struct clk_ops	ops;

	struct clk	*mux;
	struct clk	*rate;
	struct clk	*gate;

	const struct clk_ops	*mux_ops;
	const struct clk_ops	*rate_ops;
	const struct clk_ops	*gate_ops;
};

#define to_clk_composite(_clk) container_of(_clk, struct clk_composite, clk)

struct clk *clk_register_composite(struct device *dev, const char *name,
		const char * const *parent_names, int num_parents,
		struct clk *mux_clk, const struct clk_ops *mux_ops,
		struct clk *rate_clk, const struct clk_ops *rate_ops,
		struct clk *gate_clk, const struct clk_ops *gate_ops,
		unsigned long flags);

int clk_register(struct clk *clk, const char *drv_name, const char *name,
		 const char *parent_name);

struct clk *clk_register_fixed_factor(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned int mult, unsigned int div);

struct clk *clk_register_divider(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_divider_flags);

struct clk *clk_register_mux(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_mux_flags);

const char *clk_hw_get_name(const struct clk *hw);
ulong clk_generic_get_rate(struct clk *clk);

static inline struct clk *dev_get_clk_ptr(struct udevice *dev)
{
	return (struct clk *)dev_get_uclass_priv(dev);
}
#endif /* __LINUX_CLK_PROVIDER_H */
