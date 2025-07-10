// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023 Jisheng Zhang <jszhang@kernel.org>
 * Copyright (C) 2023 Vivo Communication Technology Co. Ltd.
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 *  Authors: Yangtao Li <frank.li@vivo.com>
 */

#include <asm/io.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/clk-provider.h>

#include <dt-bindings/clock/thead,th1520-clk-ap.h>

#define TH1520_PLL_POSTDIV2	GENMASK(26, 24)
#define TH1520_PLL_POSTDIV1	GENMASK(22, 20)
#define TH1520_PLL_FBDIV	GENMASK(19, 8)
#define TH1520_PLL_REFDIV	GENMASK(5, 0)
#define TH1520_PLL_BYPASS	BIT(30)
#define TH1520_PLL_DSMPD	BIT(24)
#define TH1520_PLL_FRAC		GENMASK(23, 0)
#define TH1520_PLL_FRAC_BITS    24

static const char ccu_osc_name_to_be_filled[] = "TO BE FILLED";

struct ccu_internal {
	u8 shift;
	u8 width;
};

struct ccu_div_internal {
	u8 shift;
	u8 width;
	unsigned long flags;
};

struct ccu_common {
	void __iomem *reg;
	const char *name;
	struct clk clk;
	int clkid;
	u16 cfg0;
	u16 cfg1;
};

struct ccu_mux {
	struct ccu_common common;
	struct ccu_internal mux;
	const char **parents;
	size_t num_parents;
};

struct ccu_gate {
	struct ccu_common common;
	const char *parent;
	u32 enable;
};

struct ccu_div {
	struct ccu_div_internal div;
	struct ccu_common common;
	struct ccu_internal mux;
	const char **parents;
	size_t num_parents;
	u32 enable;
};

struct ccu_pll {
	struct ccu_common	common;
};

#define TH_CCU_ARG(_shift, _width)					\
	{								\
		.shift	= _shift,					\
		.width	= _width,					\
	}

#define TH_CCU_DIV_FLAGS(_shift, _width, _flags)			\
	{								\
		.shift	= _shift,					\
		.width	= _width,					\
		.flags	= _flags,					\
	}

#define CCU_GATE(_clkid, _struct, _name, _parent, _reg, _gate, _flags)	\
	struct ccu_gate _struct = {					\
		.parent = _parent,					\
		.enable	= _gate,					\
		.common	= {						\
			.clkid		= _clkid,			\
			.cfg0		= _reg,				\
			.name		= _name,			\
		}							\
	}

static inline struct ccu_common *clk_to_ccu_common(struct clk *clk)
{
	return container_of(clk, struct ccu_common, clk);
}

static inline struct ccu_mux *clk_to_ccu_mux(struct clk *clk)
{
	struct ccu_common *common = clk_to_ccu_common(clk);

	return container_of(common, struct ccu_mux, common);
}

static inline struct ccu_pll *clk_to_ccu_pll(struct clk *clk)
{
	struct ccu_common *common = clk_to_ccu_common(clk);

	return container_of(common, struct ccu_pll, common);
}

static inline struct ccu_div *clk_to_ccu_div(struct clk *clk)
{
	struct ccu_common *common = clk_to_ccu_common(clk);

	return container_of(common, struct ccu_div, common);
}

static inline struct ccu_gate *clk_to_ccu_gate(struct clk *clk)
{
	struct ccu_common *common = clk_to_ccu_common(clk);

	return container_of(common, struct ccu_gate, common);
}

static int ccu_set_parent_helper(struct ccu_common *common,
				 struct ccu_internal *mux,
				 u8 index)
{
	clrsetbits_le32(common->reg + common->cfg0,
			GENMASK(mux->width - 1, 0) << mux->shift,
			index << mux->shift);

	return 0;
}

static void ccu_disable_helper(struct ccu_common *common, u32 gate)
{
	if (!gate)
		return;

	clrsetbits_le32(common->reg + common->cfg0,
			gate, ~gate);
}

static int ccu_enable_helper(struct ccu_common *common, u32 gate)
{
	u32 val;

	if (!gate)
		return 0;

	clrsetbits_le32(common->reg + common->cfg0, gate, gate);
	val = readl(common->reg + common->cfg0);

	return 0;
}

static int ccu_get_parent_index_helper(const char * const *parents,
				       int num_parents, struct clk *parent)
{
	const char *parent_name = parent->dev->name;
	unsigned int index;

	for (index = 0; index < num_parents; index++) {
		if (!strcmp(parents[index], parent_name))
			return index;
	}

	return -ENOENT;
}

static unsigned long ccu_div_get_rate(struct clk *clk)
{
	struct ccu_div *cd = clk_to_ccu_div(clk);
	unsigned long rate;
	unsigned int val;

	val = readl(cd->common.reg + cd->common.cfg0);
	val = val >> cd->div.shift;
	val &= GENMASK(cd->div.width - 1, 0);
	rate = divider_recalc_rate(clk, clk_get_parent_rate(clk), val, NULL,
				   cd->div.flags, cd->div.width);

	return rate;
}

static int ccu_div_get_parent(struct ccu_div *cd)
{
	u32 val = readl(cd->common.reg + cd->common.cfg0);

	return (val >> cd->mux.shift) & GENMASK(cd->mux.width - 1, 0);
}

static int ccu_div_set_parent(struct clk *clk, struct clk *parent)
{
	struct ccu_div *cd = clk_to_ccu_div(clk);
	u8 id;

	id = ccu_get_parent_index_helper(cd->parents, cd->num_parents, parent);
	if (id < 0)
		return id;

	return ccu_set_parent_helper(&cd->common, &cd->mux, id);
}

static int ccu_div_disable(struct clk *clk)
{
	struct ccu_div *cd = clk_to_ccu_div(clk);

	ccu_disable_helper(&cd->common, cd->enable);

	return 0;
}

static int ccu_div_enable(struct clk *clk)
{
	struct ccu_div *cd = clk_to_ccu_div(clk);

	return ccu_enable_helper(&cd->common, cd->enable);
}

static const struct clk_ops ccu_div_ops = {
	.disable	= ccu_div_disable,
	.enable		= ccu_div_enable,
	.set_parent	= ccu_div_set_parent,
	.get_rate	= ccu_div_get_rate,
};

U_BOOT_DRIVER(th1520_clk_div) = {
	.name	= "th1520_clk_div",
	.id	= UCLASS_CLK,
	.ops	= &ccu_div_ops,
};

static unsigned long th1520_pll_vco_recalc_rate(struct clk *clk,
						unsigned long parent_rate)
{
	struct ccu_pll *pll = clk_to_ccu_pll(clk);
	unsigned long div, mul, frac;
	unsigned int cfg0, cfg1;
	u64 rate = parent_rate;

	cfg0 = readl(pll->common.reg + pll->common.cfg0);
	cfg1 = readl(pll->common.reg + pll->common.cfg1);

	mul = FIELD_GET(TH1520_PLL_FBDIV, cfg0);
	div = FIELD_GET(TH1520_PLL_REFDIV, cfg0);
	if (!(cfg1 & TH1520_PLL_DSMPD)) {
		mul <<= TH1520_PLL_FRAC_BITS;
		frac = FIELD_GET(TH1520_PLL_FRAC, cfg1);
		mul += frac;
		div <<= TH1520_PLL_FRAC_BITS;
	}

	rate = parent_rate * mul;
	rate = rate / div;

	return rate;
}

static unsigned long th1520_pll_postdiv_recalc_rate(struct clk *clk,
						    unsigned long parent_rate)
{
	struct ccu_pll *pll = clk_to_ccu_pll(clk);
	unsigned long div, rate = parent_rate;
	unsigned int cfg0, cfg1;

	cfg0 = readl(pll->common.reg + pll->common.cfg0);
	cfg1 = readl(pll->common.reg + pll->common.cfg1);

	if (cfg1 & TH1520_PLL_BYPASS)
		return rate;

	div = FIELD_GET(TH1520_PLL_POSTDIV1, cfg0) *
	      FIELD_GET(TH1520_PLL_POSTDIV2, cfg0);

	rate = rate / div;

	return rate;
}

static unsigned long ccu_pll_get_rate(struct clk *clk)
{
	unsigned long rate = clk_get_parent_rate(clk);

	rate = th1520_pll_vco_recalc_rate(clk, rate);
	rate = th1520_pll_postdiv_recalc_rate(clk, rate);

	return rate;
}

static const struct clk_ops clk_pll_ops = {
	.get_rate	= ccu_pll_get_rate,
};

U_BOOT_DRIVER(th1520_clk_pll) = {
	.name	= "th1520_clk_pll",
	.id	= UCLASS_CLK,
	.ops	= &clk_pll_ops,
};

static struct ccu_pll cpu_pll0_clk = {
	.common		= {
		.clkid		= CLK_CPU_PLL0,
		.cfg0		= 0x000,
		.cfg1		= 0x004,
		.name		= "cpu-pll0",
	},
};

static struct ccu_pll cpu_pll1_clk = {
	.common		= {
		.clkid		= CLK_CPU_PLL1,
		.cfg0		= 0x010,
		.cfg1		= 0x014,
		.name		= "cpu-pll1",
	},
};

static struct ccu_pll gmac_pll_clk = {
	.common		= {
		.clkid		= CLK_GMAC_PLL,
		.cfg0		= 0x020,
		.cfg1		= 0x024,
		.name		= "gmac-pll",
	},
};

static const char *gmac_pll_clk_parent[] = {
	"gmac-pll",
};

static struct ccu_pll video_pll_clk = {
	.common		= {
		.clkid		= CLK_VIDEO_PLL,
		.cfg0		= 0x030,
		.cfg1		= 0x034,
		.name		= "video-pll",
	},
};

static const char *video_pll_clk_parent[] = {
	"video-pll",
};

static struct ccu_pll dpu0_pll_clk = {
	.common		= {
		.clkid		= CLK_DPU0_PLL,
		.cfg0		= 0x040,
		.cfg1		= 0x044,
		.name		= "dpu0-pll",
	},
};

static const char *dpu0_pll_clk_parent[] = {
	"dpu0-pll",
};

static struct ccu_pll dpu1_pll_clk = {
	.common		= {
		.clkid		= CLK_DPU1_PLL,
		.cfg0		= 0x050,
		.cfg1		= 0x054,
		.name		= "dpu1-pll",
	},
};

static const char *dpu1_pll_clk_parent[] = {
	"dpu1-pll",
};

static struct ccu_pll tee_pll_clk = {
	.common		= {
		.clkid		= CLK_TEE_PLL,
		.cfg0		= 0x060,
		.cfg1		= 0x064,
		.name		= "tee-pll",
	},
};

static const char *c910_i0_parents[] = {
	"cpu-pll0", ccu_osc_name_to_be_filled,
};

static struct ccu_mux c910_i0_clk = {
	.parents	= c910_i0_parents,
	.num_parents	= ARRAY_SIZE(c910_i0_parents),
	.mux		= TH_CCU_ARG(1, 1),
	.common		= {
		.clkid		= CLK_C910_I0,
		.cfg0		= 0x100,
		.name		= "c910-i0",
	}
};

static const char *c910_parents[] = {
	"c910-i0", "cpu-pll1",
};

static struct ccu_mux c910_clk = {
	.parents	= c910_parents,
	.num_parents	= ARRAY_SIZE(c910_parents),
	.mux		= TH_CCU_ARG(0, 1),
	.common		= {
		.clkid		= CLK_C910,
		.cfg0		= 0x100,
		.name		= "c910",
	}
};

static const char *ahb2_cpusys_parents[] = {
	"gmac-pll", ccu_osc_name_to_be_filled,
};

static struct ccu_div ahb2_cpusys_hclk = {
	.parents	= ahb2_cpusys_parents,
	.num_parents	= ARRAY_SIZE(ahb2_cpusys_parents),
	.div		= TH_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_ONE_BASED),
	.mux		= TH_CCU_ARG(5, 1),
	.common		= {
		.clkid          = CLK_AHB2_CPUSYS_HCLK,
		.cfg0		= 0x120,
		.name		= "ahb2-cpusys-hclk",
	},
};

static const char *ahb2_cpusys_hclk_parents[] = {
	"ahb2-cpusys-hclk",
};

static struct ccu_div apb3_cpusys_pclk = {
	.parents	= ahb2_cpusys_hclk_parents,
	.num_parents	= ARRAY_SIZE(ahb2_cpusys_hclk_parents),
	.div		= TH_CCU_ARG(0, 3),
	.common		= {
		.clkid          = CLK_APB3_CPUSYS_PCLK,
		.cfg0		= 0x130,
		.name		= "apb3-cpusys-pclk",
	},
};

static struct ccu_div axi4_cpusys2_aclk = {
	.parents	= gmac_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(gmac_pll_clk_parent),
	.div		= TH_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_AXI4_CPUSYS2_ACLK,
		.cfg0		= 0x134,
		.name		= "axi4-cpusys2-aclk",
	},
};

static const char *axi_parents[] = {
	"video-pll", ccu_osc_name_to_be_filled,
};

static struct ccu_div axi_aclk = {
	.parents	= axi_parents,
	.num_parents	= ARRAY_SIZE(axi_parents),
	.div		= TH_CCU_DIV_FLAGS(0, 4, CLK_DIVIDER_ONE_BASED),
	.mux		= TH_CCU_ARG(5, 1),
	.common		= {
		.clkid          = CLK_AXI_ACLK,
		.cfg0		= 0x138,
		.name		= "axi-aclk",
	},
};

static const char *perisys_ahb_hclk_parents[] = {
	"gmac-pll", ccu_osc_name_to_be_filled,
};

static struct ccu_div perisys_ahb_hclk = {
	.parents	= perisys_ahb_hclk_parents,
	.num_parents	= ARRAY_SIZE(perisys_ahb_hclk_parents),
	.enable		= BIT(6),
	.div		= TH_CCU_DIV_FLAGS(0, 4, CLK_DIVIDER_ONE_BASED),
	.mux		= TH_CCU_ARG(5, 1),
	.common		= {
		.clkid          = CLK_PERI_AHB_HCLK,
		.cfg0		= 0x140,
		.name		= "perisys-ahb-hclk",
	},
};

static const char *perisys_ahb_hclk_parent[] = {
	"perisys-ahb-hclk",
};

static struct ccu_div perisys_apb_pclk = {
	.parents	= perisys_ahb_hclk_parent,
	.num_parents	= ARRAY_SIZE(perisys_ahb_hclk_parent),
	.div		= TH_CCU_ARG(0, 3),
	.common		= {
		.clkid          = CLK_PERI_APB_PCLK,
		.cfg0		= 0x150,
		.name		= "perisys-apb-pclk",
	},
};

static struct ccu_div peri2sys_apb_pclk = {
	.parents	= gmac_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(gmac_pll_clk_parent),
	.div		= TH_CCU_DIV_FLAGS(4, 3, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_PERI2APB_PCLK,
		.cfg0		= 0x150,
		.name		= "peri2sys-apb-pclk",
	},
};

static const char *apb_parents[] = {
	"gmac-pll", ccu_osc_name_to_be_filled,
};

static struct ccu_div apb_pclk = {
	.parents	= apb_parents,
	.num_parents	= ARRAY_SIZE(apb_parents),
	.enable		= BIT(5),
	.div		= TH_CCU_DIV_FLAGS(0, 4, CLK_DIVIDER_ONE_BASED),
	.mux		= TH_CCU_ARG(7, 1),
	.common		= {
		.clkid          = CLK_APB_PCLK,
		.cfg0		= 0x1c4,
		.name		= "apb-pclk",
	},
};

static const char *npu_parents[] = {
	"gmac-pll", "video-pll",
};

static struct ccu_div npu_clk = {
	.parents	= npu_parents,
	.num_parents	= ARRAY_SIZE(npu_parents),
	.enable		= BIT(4),
	.div		= TH_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_ONE_BASED),
	.mux		= TH_CCU_ARG(6, 1),
	.common		= {
		.clkid          = CLK_NPU,
		.cfg0		= 0x1c8,
		.name		= "npu",
	},
};

static struct ccu_div vi_clk = {
	.parents	= video_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(video_pll_clk_parent),
	.div		= TH_CCU_DIV_FLAGS(16, 4, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_VI,
		.cfg0		= 0x1d0,
		.name		= "vi",
	},
};

static struct ccu_div vi_ahb_clk = {
	.parents	= video_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(video_pll_clk_parent),
	.div		= TH_CCU_DIV_FLAGS(0, 4, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_VI_AHB,
		.cfg0		= 0x1d0,
		.name		= "vi-ahb",
	},
};

static struct ccu_div vo_axi_clk = {
	.parents	= video_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(video_pll_clk_parent),
	.enable		= BIT(5),
	.div		= TH_CCU_DIV_FLAGS(0, 4, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_VO_AXI,
		.cfg0		= 0x1dc,
		.name		= "vo-axi",
	},
};

static struct ccu_div vp_apb_clk = {
	.parents	= gmac_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(gmac_pll_clk_parent),
	.div		= TH_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_VP_APB,
		.cfg0		= 0x1e0,
		.name		= "vp-apb",
	},
};

static struct ccu_div vp_axi_clk = {
	.parents	= video_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(video_pll_clk_parent),
	.enable		= BIT(15),
	.div		= TH_CCU_DIV_FLAGS(8, 4, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_VP_AXI,
		.cfg0		= 0x1e0,
		.name		= "vp-axi",
	},
};

static struct ccu_div venc_clk = {
	.parents	= gmac_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(gmac_pll_clk_parent),
	.enable		= BIT(5),
	.div		= TH_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_VENC,
		.cfg0		= 0x1e4,
		.name		= "venc",
	},
};

static struct ccu_div dpu0_clk = {
	.parents	= dpu0_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(dpu0_pll_clk_parent),
	.div		= TH_CCU_DIV_FLAGS(0, 8, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_DPU0,
		.cfg0		= 0x1e8,
		.name		= "dpu0",
	},
};

static struct ccu_div dpu1_clk = {
	.parents	= dpu1_pll_clk_parent,
	.num_parents	= ARRAY_SIZE(dpu1_pll_clk_parent),
	.div		= TH_CCU_DIV_FLAGS(0, 8, CLK_DIVIDER_ONE_BASED),
	.common		= {
		.clkid          = CLK_DPU1,
		.cfg0		= 0x1ec,
		.name		= "dpu1",
	},
};

static CCU_GATE(CLK_BROM, brom_clk, "brom", "ahb2-cpusys-hclk", 0x100, BIT(4), 0);
static CCU_GATE(CLK_BMU, bmu_clk, "bmu", "axi4-cpusys2-aclk", 0x100, BIT(5), 0);
static CCU_GATE(CLK_AON2CPU_A2X, aon2cpu_a2x_clk, "aon2cpu-a2x", "axi4-cpusys2-aclk",
		0x134, BIT(8), 0);
static CCU_GATE(CLK_X2X_CPUSYS, x2x_cpusys_clk, "x2x-cpusys", "axi4-cpusys2-aclk",
		0x134, BIT(7), 0);
static CCU_GATE(CLK_CPU2AON_X2H, cpu2aon_x2h_clk, "cpu2aon-x2h", "axi-aclk", 0x138, BIT(8), 0);
static CCU_GATE(CLK_CPU2PERI_X2H, cpu2peri_x2h_clk, "cpu2peri-x2h", "axi4-cpusys2-aclk",
		0x140, BIT(9), CLK_IGNORE_UNUSED);
static CCU_GATE(CLK_PERISYS_APB1_HCLK, perisys_apb1_hclk, "perisys-apb1-hclk", "perisys-ahb-hclk",
		0x150, BIT(9), 0);
static CCU_GATE(CLK_PERISYS_APB2_HCLK, perisys_apb2_hclk, "perisys-apb2-hclk", "perisys-ahb-hclk",
		0x150, BIT(10), CLK_IGNORE_UNUSED);
static CCU_GATE(CLK_PERISYS_APB3_HCLK, perisys_apb3_hclk, "perisys-apb3-hclk", "perisys-ahb-hclk",
		0x150, BIT(11), CLK_IGNORE_UNUSED);
static CCU_GATE(CLK_PERISYS_APB4_HCLK, perisys_apb4_hclk, "perisys-apb4-hclk", "perisys-ahb-hclk",
		0x150, BIT(12), 0);
static CCU_GATE(CLK_NPU_AXI, npu_axi_clk, "npu-axi", "axi-aclk", 0x1c8, BIT(5), 0);
static CCU_GATE(CLK_CPU2VP, cpu2vp_clk, "cpu2vp", "axi-aclk", 0x1e0, BIT(13), 0);
static CCU_GATE(CLK_EMMC_SDIO, emmc_sdio_clk, "emmc-sdio", "emmc-sdio-ref", 0x204, BIT(30), 0);
static CCU_GATE(CLK_GMAC1, gmac1_clk, "gmac1", "gmac-pll", 0x204, BIT(26), 0);
static CCU_GATE(CLK_PADCTRL1, padctrl1_clk, "padctrl1", "perisys-apb-pclk", 0x204, BIT(24), 0);
static CCU_GATE(CLK_DSMART, dsmart_clk, "dsmart", "perisys-apb-pclk", 0x204, BIT(23), 0);
static CCU_GATE(CLK_PADCTRL0, padctrl0_clk, "padctrl0", "perisys-apb-pclk", 0x204, BIT(22), 0);
static CCU_GATE(CLK_GMAC_AXI, gmac_axi_clk, "gmac-axi", "axi4-cpusys2-aclk", 0x204, BIT(21), 0);
static CCU_GATE(CLK_GPIO3, gpio3_clk, "gpio3-clk", "peri2sys-apb-pclk", 0x204, BIT(20), 0);
static CCU_GATE(CLK_GMAC0, gmac0_clk, "gmac0", "gmac-pll", 0x204, BIT(19), 0);
static CCU_GATE(CLK_PWM, pwm_clk, "pwm", "perisys-apb-pclk", 0x204, BIT(18), 0);
static CCU_GATE(CLK_QSPI0, qspi0_clk, "qspi0", "video-pll", 0x204, BIT(17), 0);
static CCU_GATE(CLK_QSPI1, qspi1_clk, "qspi1", "video-pll", 0x204, BIT(16), 0);
static CCU_GATE(CLK_SPI, spi_clk, "spi", "video-pll", 0x204, BIT(15), 0);
static CCU_GATE(CLK_UART0_PCLK, uart0_pclk, "uart0-pclk", "perisys-apb-pclk", 0x204, BIT(14), 0);
static CCU_GATE(CLK_UART1_PCLK, uart1_pclk, "uart1-pclk", "perisys-apb-pclk", 0x204, BIT(13), 0);
static CCU_GATE(CLK_UART2_PCLK, uart2_pclk, "uart2-pclk", "perisys-apb-pclk", 0x204, BIT(12), 0);
static CCU_GATE(CLK_UART3_PCLK, uart3_pclk, "uart3-pclk", "perisys-apb-pclk", 0x204, BIT(11), 0);
static CCU_GATE(CLK_UART4_PCLK, uart4_pclk, "uart4-pclk", "perisys-apb-pclk", 0x204, BIT(10), 0);
static CCU_GATE(CLK_UART5_PCLK, uart5_pclk, "uart5-pclk", "perisys-apb-pclk", 0x204, BIT(9), 0);
static CCU_GATE(CLK_GPIO0, gpio0_clk, "gpio0-clk", "perisys-apb-pclk", 0x204, BIT(8), 0);
static CCU_GATE(CLK_GPIO1, gpio1_clk, "gpio1-clk", "perisys-apb-pclk", 0x204, BIT(7), 0);
static CCU_GATE(CLK_GPIO2, gpio2_clk, "gpio2-clk", "peri2sys-apb-pclk", 0x204, BIT(6), 0);
static CCU_GATE(CLK_I2C0, i2c0_clk, "i2c0", "perisys-apb-pclk", 0x204, BIT(5), 0);
static CCU_GATE(CLK_I2C1, i2c1_clk, "i2c1", "perisys-apb-pclk", 0x204, BIT(4), 0);
static CCU_GATE(CLK_I2C2, i2c2_clk, "i2c2", "perisys-apb-pclk", 0x204, BIT(3), 0);
static CCU_GATE(CLK_I2C3, i2c3_clk, "i2c3", "perisys-apb-pclk", 0x204, BIT(2), 0);
static CCU_GATE(CLK_I2C4, i2c4_clk, "i2c4", "perisys-apb-pclk", 0x204, BIT(1), 0);
static CCU_GATE(CLK_I2C5, i2c5_clk, "i2c5", "perisys-apb-pclk", 0x204, BIT(0), 0);
static CCU_GATE(CLK_SPINLOCK, spinlock_clk, "spinlock", "ahb2-cpusys-hclk", 0x208, BIT(10), 0);
static CCU_GATE(CLK_DMA, dma_clk, "dma", "axi4-cpusys2-aclk", 0x208, BIT(8), 0);
static CCU_GATE(CLK_MBOX0, mbox0_clk, "mbox0", "apb3-cpusys-pclk", 0x208, BIT(7), 0);
static CCU_GATE(CLK_MBOX1, mbox1_clk, "mbox1", "apb3-cpusys-pclk", 0x208, BIT(6), 0);
static CCU_GATE(CLK_MBOX2, mbox2_clk, "mbox2", "apb3-cpusys-pclk", 0x208, BIT(5), 0);
static CCU_GATE(CLK_MBOX3, mbox3_clk, "mbox3", "apb3-cpusys-pclk", 0x208, BIT(4), 0);
static CCU_GATE(CLK_WDT0, wdt0_clk, "wdt0", "apb3-cpusys-pclk", 0x208, BIT(3), 0);
static CCU_GATE(CLK_WDT1, wdt1_clk, "wdt1", "apb3-cpusys-pclk", 0x208, BIT(2), 0);
static CCU_GATE(CLK_TIMER0, timer0_clk, "timer0", "apb3-cpusys-pclk", 0x208, BIT(1), 0);
static CCU_GATE(CLK_TIMER1, timer1_clk, "timer1", "apb3-cpusys-pclk", 0x208, BIT(0), 0);
static CCU_GATE(CLK_SRAM0, sram0_clk, "sram0", "axi-aclk", 0x20c, BIT(4), 0);
static CCU_GATE(CLK_SRAM1, sram1_clk, "sram1", "axi-aclk", 0x20c, BIT(3), 0);
static CCU_GATE(CLK_SRAM2, sram2_clk, "sram2", "axi-aclk", 0x20c, BIT(2), 0);
static CCU_GATE(CLK_SRAM3, sram3_clk, "sram3", "axi-aclk", 0x20c, BIT(1), 0);

static const char *uart_sclk_parents[] = {
	"gmac-pll-clk-100m", ccu_osc_name_to_be_filled,
};

static struct ccu_mux uart_sclk = {
	.parents	= uart_sclk_parents,
	.num_parents	= ARRAY_SIZE(uart_sclk_parents),
	.mux		= TH_CCU_ARG(0, 1),
	.common		= {
		.clkid          = CLK_UART_SCLK,
		.cfg0		= 0x210,
		.name		= "uart-sclk",
	}
};

static struct ccu_common *th1520_pll_clks[] = {
	&cpu_pll0_clk.common,
	&cpu_pll1_clk.common,
	&gmac_pll_clk.common,
	&video_pll_clk.common,
	&dpu0_pll_clk.common,
	&dpu1_pll_clk.common,
	&tee_pll_clk.common,
};

static struct ccu_common *th1520_div_clks[] = {
	&ahb2_cpusys_hclk.common,
	&apb3_cpusys_pclk.common,
	&axi4_cpusys2_aclk.common,
	&perisys_ahb_hclk.common,
	&perisys_apb_pclk.common,
	&axi_aclk.common,
	&peri2sys_apb_pclk.common,
	&apb_pclk.common,
	&npu_clk.common,
	&vi_clk.common,
	&vi_ahb_clk.common,
	&vo_axi_clk.common,
	&vp_apb_clk.common,
	&vp_axi_clk.common,
	&venc_clk.common,
	&dpu0_clk.common,
	&dpu1_clk.common,
};

static struct ccu_common *th1520_mux_clks[] = {
	&c910_i0_clk.common,
	&c910_clk.common,
	&uart_sclk.common,
};

static struct ccu_common *th1520_gate_clks[] = {
	&emmc_sdio_clk.common,
	&aon2cpu_a2x_clk.common,
	&x2x_cpusys_clk.common,
	&brom_clk.common,
	&bmu_clk.common,
	&cpu2aon_x2h_clk.common,
	&cpu2peri_x2h_clk.common,
	&cpu2vp_clk.common,
	&perisys_apb1_hclk.common,
	&perisys_apb2_hclk.common,
	&perisys_apb3_hclk.common,
	&perisys_apb4_hclk.common,
	&npu_axi_clk.common,
	&gmac1_clk.common,
	&padctrl1_clk.common,
	&dsmart_clk.common,
	&padctrl0_clk.common,
	&gmac_axi_clk.common,
	&gpio3_clk.common,
	&gmac0_clk.common,
	&pwm_clk.common,
	&qspi0_clk.common,
	&qspi1_clk.common,
	&spi_clk.common,
	&uart0_pclk.common,
	&uart1_pclk.common,
	&uart2_pclk.common,
	&uart3_pclk.common,
	&uart4_pclk.common,
	&uart5_pclk.common,
	&gpio0_clk.common,
	&gpio1_clk.common,
	&gpio2_clk.common,
	&i2c0_clk.common,
	&i2c1_clk.common,
	&i2c2_clk.common,
	&i2c3_clk.common,
	&i2c4_clk.common,
	&i2c5_clk.common,
	&spinlock_clk.common,
	&dma_clk.common,
	&mbox0_clk.common,
	&mbox1_clk.common,
	&mbox2_clk.common,
	&mbox3_clk.common,
	&wdt0_clk.common,
	&wdt1_clk.common,
	&timer0_clk.common,
	&timer1_clk.common,
	&sram0_clk.common,
	&sram1_clk.common,
	&sram2_clk.common,
	&sram3_clk.common,
};

static void th1520_clk_fill_osc_name(const char **names, size_t name_num,
				     const char *osc_name)
{
	size_t i;

	for (i = 0; i < name_num; i++) {
		if (names[i] == ccu_osc_name_to_be_filled)
			names[i] = osc_name;
	}
}

static int th1520_clk_probe(struct udevice *dev)
{
	struct clk *clk, osc_clk;
	const char *osc_name;
	void __iomem *base;
	fdt_addr_t addr;
	int ret, i;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	base = (void __iomem *)addr;

	ret = clk_get_by_index(dev, 0, &osc_clk);
	if (ret) {
		pr_err("failed to get osc clock: %d\n", ret);
		return ret;
	}

	osc_name = clk_hw_get_name(&osc_clk);

	for (i = 0; i < ARRAY_SIZE(th1520_pll_clks); i++) {
		struct ccu_common *common = th1520_pll_clks[i];

		common->reg = base;

		ret = clk_register(&common->clk, "th1520_clk_pll",
				   common->name, osc_name);
		if (ret) {
			pr_err("failed to register PLL %s: %d\n",
			       common->name, ret);
			return ret;
		}

		common->clk.id = common->clkid;
	}

	for (i = 0; i < ARRAY_SIZE(th1520_div_clks); i++) {
		struct ccu_div *cd = container_of(th1520_div_clks[i],
						  struct ccu_div, common);
		const char *current_parent;

		cd->common.reg = base;
		th1520_clk_fill_osc_name(cd->parents, cd->num_parents,
					 osc_name);

		if (cd->num_parents > 1)
			current_parent = cd->parents[ccu_div_get_parent(cd)];
		else
			current_parent = cd->parents[0];

		ret = clk_register(&cd->common.clk, "th1520_clk_div",
				   cd->common.name,
				   current_parent);

		if (ret) {
			pr_err("failed to register div clock %s: %d\n",
			       cd->common.name, ret);
			return ret;
		}

		cd->common.clk.id = cd->common.clkid;
	}

	clk = clk_register_fixed_factor(dev, "gmac-pll-clk-100m", "gmac-pll",
					0, 1, 10);
	if (IS_ERR(clk)) {
		pr_err("failed to register gmac-pll-clk-100m: %d\n",
		       (int)PTR_ERR(clk));
		return PTR_ERR(clk);
	}
	clk->id = CLK_PLL_GMAC_100M;

	clk = clk_register_fixed_factor(dev, "emmc-sdio-ref", "video-pll",
					0, 1, 4);
	if (IS_ERR(clk)) {
		pr_err("failed to register emmc-sdio-ref: %d\n",
		       (int)PTR_ERR(clk));
		return PTR_ERR(clk);
	}

	for (i = 0; i < ARRAY_SIZE(th1520_mux_clks); i++) {
		struct ccu_mux *cm = container_of(th1520_mux_clks[i],
						  struct ccu_mux, common);

		th1520_clk_fill_osc_name(cm->parents, cm->num_parents,
					 osc_name);

		clk = clk_register_mux(dev, cm->common.name,
				       cm->parents, cm->num_parents,
				       0,
				       base + cm->common.cfg0,
				       cm->mux.shift, cm->mux.width,
				       0);
		if (IS_ERR(clk)) {
			pr_err("failed to register mux clock %s: %d\n",
			       cm->common.name, (int)PTR_ERR(clk));
			return PTR_ERR(clk);
		}

		clk->id = cm->common.clkid;
	}

	for (i = 0; i < ARRAY_SIZE(th1520_gate_clks); i++) {
		struct ccu_gate *cg = container_of(th1520_gate_clks[i],
						   struct ccu_gate, common);

		th1520_clk_fill_osc_name(&cg->parent, 1, osc_name);

		clk = clk_register_gate(dev, cg->common.name,
					cg->parent,
					0,
					base + cg->common.cfg0,
					ffs(cg->enable) - 1, 0, NULL);
		if (IS_ERR(clk)) {
			pr_err("failed to register gate clock %s: %d\n",
			       cg->common.name, (int)PTR_ERR(clk));
			return PTR_ERR(clk);
		}

		clk->id = cg->common.clkid;
	}

	return 0;
}

static const struct udevice_id th1520_clk_match[] = {
	{
		.compatible = "thead,th1520-clk-ap",
	},
	{ /* sentinel */ },
};

static int th1520_clk_enable(struct clk *clk)
{
	struct clk *c;
	int ret;

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	return clk_enable(c);
}

static int th1520_clk_disable(struct clk *clk)
{
	struct clk *c;
	int ret;

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	return clk_disable(c);
}

static ulong th1520_clk_get_rate(struct clk *clk)
{
	struct clk *c;
	int ret;

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	return clk_get_rate(c);
}

static ulong th1520_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk *c;
	int ret;

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	return clk_set_rate(c, rate);
}

static int th1520_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk *c, *p;
	int ret;

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	ret = clk_get_by_id(parent->id, &p);
	if (ret)
		return ret;

	return clk_set_parent(c, p);
}

static const struct clk_ops th1520_clk_ops = {
	.enable		= th1520_clk_enable,
	.disable	= th1520_clk_disable,
	.get_rate	= th1520_clk_get_rate,
	.set_rate	= th1520_clk_set_rate,
	.set_parent	= th1520_clk_set_parent,
};

U_BOOT_DRIVER(th1520_clk) = {
	.name		= "th1520-clk",
	.id		= UCLASS_CLK,
	.of_match	= th1520_clk_match,
	.probe		= th1520_clk_probe,
	.ops		= &th1520_clk_ops,
};
