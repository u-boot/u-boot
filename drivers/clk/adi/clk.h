/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Author: Greg Malysa <greg.malysa@timesys.com>
 *
 * Ported from Linux: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 */

#ifndef CLK_ADI_CLK_H
#define CLK_ADI_CLK_H

#include <linux/compiler_types.h>
#include <linux/types.h>
#include <linux/clk-provider.h>

#define CGU_CTL         0x00
#define CGU_PLLCTL      0x04
#define CGU_STAT        0x08
#define CGU_DIV         0x0C
#define CGU_CLKOUTSEL   0x10
#define CGU_OSCWDCTL    0x14
#define CGU_TSCTL       0x18
#define CGU_TSVALUE0    0x1C
#define CGU_TSVALUE1    0x20
#define CGU_TSCOUNT0    0x24
#define CGU_TSCOUNT1    0x28
#define CGU_CCBF_DIS    0x2C
#define CGU_CCBF_STAT   0x30
#define CGU_SCBF_DIS    0x38
#define CGU_SCBF_STAT   0x3C
#define CGU_DIVEX       0x40
#define CGU_REVID       0x48

#define CDU_CFG0     0x00
#define CDU_CFG1     0x04
#define CDU_CFG2     0x08
#define CDU_CFG3     0x0C
#define CDU_CFG4     0x10
#define CDU_CFG5     0x14
#define CDU_CFG6     0x18
#define CDU_CFG7     0x1C
#define CDU_CFG8     0x20
#define CDU_CFG9     0x24
#define CDU_CFG10    0x28
#define CDU_CFG11    0x2C
#define CDU_CFG12    0x30
#define CDU_CFG13    0x34
#define CDU_CFG14    0x38

#define PLL3_OFFSET 0x2c

#define CDU_CLKINSEL 0x44

#define CGU_MSEL_SHIFT 8
#define CGU_MSEL_WIDTH 7

#define PLL3_MSEL_SHIFT 4
#define PLL3_MSEL_WIDTH 7

#define CDU_MUX_SIZE 4
#define CDU_MUX_SHIFT 1
#define CDU_MUX_WIDTH 2
#define CDU_EN_BIT 0

extern const struct clk_ops adi_clk_ops;

struct clk *sc5xx_cgu_pll(const char *name, const char *parent_name,
			  void __iomem *base, u8 shift, u8 width, u32 m_offset, bool half_m);

/**
 * All CDU clock muxes are the same size
 */
static inline struct clk *cdu_mux(const char *name, void __iomem *reg,
				  const char * const *parents)
{
	return clk_register_mux(NULL, name, parents, CDU_MUX_SIZE,
		CLK_SET_RATE_PARENT, reg, CDU_MUX_SHIFT, CDU_MUX_WIDTH, 0);
}

static inline struct clk *cgu_divider(const char *name, const char *parent,
				      void __iomem *reg, u8 shift, u8 width, u8 extra_flags)
{
	return clk_register_divider(NULL, name, parent, CLK_SET_RATE_PARENT,
		reg, shift, width, CLK_DIVIDER_MAX_AT_ZERO | extra_flags);
}

static inline struct clk *cdu_gate(const char *name, const char *parent,
				   void __iomem *reg, u32 flags)
{
	return clk_register_gate(NULL, name, parent, CLK_SET_RATE_PARENT | flags,
		reg, CDU_EN_BIT, 0, NULL);
}

static inline struct clk *cgu_gate(const char *name, const char *parent,
				   void __iomem *reg, u8 bit)
{
	return clk_register_gate(NULL, name, parent, CLK_SET_RATE_PARENT, reg, bit,
		CLK_GATE_SET_TO_DISABLE, NULL);
}

static inline int cdu_check_clocks(struct clk *clks[], size_t count)
{
	size_t i;

	for (i = 0; i < count; ++i) {
		if (clks[i]) {
			if (IS_ERR(clks[i])) {
				pr_err("Clock %zu failed to register: %ld\n", i, PTR_ERR(clks[i]));
				return PTR_ERR(clks[i]);
			}
			clks[i]->id = i;
		} else {
			pr_err("ADI Clock framework: Null pointer detected on clock %zu\n", i);
		}
	}

	return 0;
}

#endif
