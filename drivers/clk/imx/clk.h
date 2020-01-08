// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */
#ifndef __MACH_IMX_CLK_H
#define __MACH_IMX_CLK_H

#include <linux/clk-provider.h>

enum imx_pllv3_type {
	IMX_PLLV3_GENERIC,
	IMX_PLLV3_SYS,
	IMX_PLLV3_USB,
	IMX_PLLV3_USB_VF610,
	IMX_PLLV3_AV,
	IMX_PLLV3_ENET,
	IMX_PLLV3_ENET_IMX7,
	IMX_PLLV3_SYS_VF610,
	IMX_PLLV3_DDR_IMX7,
};

enum imx_pll14xx_type {
	PLL_1416X,
	PLL_1443X,
};

/* NOTE: Rate table should be kept sorted in descending order. */
struct imx_pll14xx_rate_table {
	unsigned int rate;
	unsigned int pdiv;
	unsigned int mdiv;
	unsigned int sdiv;
	unsigned int kdiv;
};

struct imx_pll14xx_clk {
	enum imx_pll14xx_type type;
	const struct imx_pll14xx_rate_table *rate_table;
	int rate_count;
	int flags;
};

struct clk *imx_clk_pll14xx(const char *name, const char *parent_name,
			    void __iomem *base,
			    const struct imx_pll14xx_clk *pll_clk);

struct clk *clk_register_gate2(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 bit_idx, u8 cgr_val,
		u8 clk_gate_flags);

struct clk *imx_clk_pllv3(enum imx_pllv3_type type, const char *name,
			  const char *parent_name, void __iomem *base,
			  u32 div_mask);

static inline struct clk *imx_clk_gate2(const char *name, const char *parent,
					void __iomem *reg, u8 shift)
{
	return clk_register_gate2(NULL, name, parent, CLK_SET_RATE_PARENT, reg,
			shift, 0x3, 0);
}

static inline struct clk *imx_clk_gate4(const char *name, const char *parent,
		void __iomem *reg, u8 shift)
{
	return clk_register_gate2(NULL, name, parent,
			CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, 0x3, 0);
}

static inline struct clk *imx_clk_gate4_flags(const char *name,
		const char *parent, void __iomem *reg, u8 shift,
		unsigned long flags)
{
	return clk_register_gate2(NULL, name, parent,
			flags | CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, 0x3, 0);
}

static inline struct clk *imx_clk_fixed_factor(const char *name,
		const char *parent, unsigned int mult, unsigned int div)
{
	return clk_register_fixed_factor(NULL, name, parent,
			CLK_SET_RATE_PARENT, mult, div);
}

static inline struct clk *imx_clk_divider(const char *name, const char *parent,
		void __iomem *reg, u8 shift, u8 width)
{
	return clk_register_divider(NULL, name, parent, CLK_SET_RATE_PARENT,
			reg, shift, width, 0);
}

static inline struct clk *
imx_clk_busy_divider(const char *name, const char *parent, void __iomem *reg,
		     u8 shift, u8 width, void __iomem *busy_reg, u8 busy_shift)
{
	return clk_register_divider(NULL, name, parent, CLK_SET_RATE_PARENT,
			reg, shift, width, 0);
}

static inline struct clk *imx_clk_divider2(const char *name, const char *parent,
		void __iomem *reg, u8 shift, u8 width)
{
	return clk_register_divider(NULL, name, parent,
			CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, width, 0);
}

struct clk *imx_clk_pfd(const char *name, const char *parent_name,
			void __iomem *reg, u8 idx);

struct clk *imx_clk_fixup_mux(const char *name, void __iomem *reg,
			      u8 shift, u8 width, const char * const *parents,
			      int num_parents, void (*fixup)(u32 *val));

static inline struct clk *imx_clk_mux_flags(const char *name,
			void __iomem *reg, u8 shift, u8 width,
			const char * const *parents, int num_parents,
			unsigned long flags)
{
	return clk_register_mux(NULL, name, parents, num_parents,
				flags | CLK_SET_RATE_NO_REPARENT, reg, shift,
				width, 0);
}

static inline struct clk *imx_clk_mux2_flags(const char *name,
		void __iomem *reg, u8 shift, u8 width,
		const char * const *parents,
		int num_parents, unsigned long flags)
{
	return clk_register_mux(NULL, name, parents, num_parents,
			flags | CLK_SET_RATE_NO_REPARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, width, 0);
}

static inline struct clk *imx_clk_mux(const char *name, void __iomem *reg,
			u8 shift, u8 width, const char * const *parents,
			int num_parents)
{
	return clk_register_mux(NULL, name, parents, num_parents,
			CLK_SET_RATE_NO_REPARENT, reg, shift,
			width, 0);
}

static inline struct clk *
imx_clk_busy_mux(const char *name, void __iomem *reg, u8 shift, u8 width,
		 void __iomem *busy_reg, u8 busy_shift,
		 const char * const *parents, int num_parents)
{
	return clk_register_mux(NULL, name, parents, num_parents,
			CLK_SET_RATE_NO_REPARENT, reg, shift,
			width, 0);
}

static inline struct clk *imx_clk_mux2(const char *name, void __iomem *reg,
			u8 shift, u8 width, const char * const *parents,
			int num_parents)
{
	return clk_register_mux(NULL, name, parents, num_parents,
			CLK_SET_RATE_NO_REPARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, width, 0);
}

static inline struct clk *imx_clk_gate(const char *name, const char *parent,
		void __iomem *reg, u8 shift)
{
	return clk_register_gate(NULL, name, parent, CLK_SET_RATE_PARENT, reg,
			shift, 0, NULL);
}

static inline struct clk *imx_clk_gate_flags(const char *name, const char *parent,
		void __iomem *reg, u8 shift, unsigned long flags)
{
	return clk_register_gate(NULL, name, parent, flags | CLK_SET_RATE_PARENT, reg,
			shift, 0, NULL);
}

static inline struct clk *imx_clk_gate3(const char *name, const char *parent,
		void __iomem *reg, u8 shift)
{
	return clk_register_gate(NULL, name, parent,
			CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
			reg, shift, 0, NULL);
}

struct clk *imx8m_clk_composite_flags(const char *name,
		const char * const *parent_names,
		int num_parents, void __iomem *reg, unsigned long flags);

#define __imx8m_clk_composite(name, parent_names, reg, flags) \
	imx8m_clk_composite_flags(name, parent_names, \
		ARRAY_SIZE(parent_names), reg, \
		flags | CLK_SET_RATE_NO_REPARENT | CLK_OPS_PARENT_ENABLE)

#define imx8m_clk_composite(name, parent_names, reg) \
	__imx8m_clk_composite(name, parent_names, reg, 0)

#define imx8m_clk_composite_critical(name, parent_names, reg) \
	__imx8m_clk_composite(name, parent_names, reg, CLK_IS_CRITICAL)

#endif /* __MACH_IMX_CLK_H */
