// SPDX-License-Identifier: GPL-2.0+
/*
 * Legacy IPU clock management for i.MX5/6 without Common Clock Framework
 *
 * (C) Copyright 2026
 * Brian Ruley, GE HealthCare, brian.ruley@gehealthcare.com
 */

#include "ipu.h"
#include "ipu_regs.h"
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <div64.h>
#include <dm/devres.h>
#include <linux/err.h>
#include <log.h>

extern struct mxc_ccm_reg *mxc_ccm;

void clk_enable(struct clk *clk)
{
	if (clk) {
		if (clk->usecount++ == 0)
			clk->enable(clk);
	}
}

void clk_disable(struct clk *clk)
{
	if (clk) {
		if (!(--clk->usecount)) {
			if (clk->disable)
				clk->disable(clk);
		}
	}
}

int clk_get_usecount(struct clk *clk)
{
	if (clk == NULL)
		return 0;

	return clk->usecount;
}

u32 clk_get_rate(struct clk *clk)
{
	if (!clk)
		return 0;

	return clk->rate;
}

struct clk *clk_get_parent(struct clk *clk)
{
	if (!clk)
		return 0;

	return clk->parent;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	if (!clk)
		return 0;

	if (clk->set_rate)
		clk->set_rate(clk, rate);

	return clk->rate;
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (clk == NULL || !clk->round_rate)
		return 0;

	return clk->round_rate(clk, rate);
}

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	clk->parent = parent;
	if (clk->set_parent)
		return clk->set_parent(clk, parent);
	return 0;
}

static int clk_ipu_enable(struct clk *clk)
{
	u32 reg;

	reg = __raw_readl(clk->enable_reg);
	reg |= MXC_CCM_CCGR_CG_MASK << clk->enable_shift;
	__raw_writel(reg, clk->enable_reg);

#if CONFIG_IS_ENABLED(MX51) || CONFIG_IS_ENABLED(MX53)
	reg = __raw_readl(&mxc_ccm->ccdr);
	reg &= ~MXC_CCM_CCDR_IPU_HS_MASK;
	__raw_writel(reg, &mxc_ccm->ccdr);

	reg = __raw_readl(&mxc_ccm->clpcr);
	reg &= ~MXC_CCM_CLPCR_BYPASS_IPU_LPM_HS;
	__raw_writel(reg, &mxc_ccm->clpcr);
#endif
	return 0;
}

static void clk_ipu_disable(struct clk *clk)
{
	u32 reg;

	reg = __raw_readl(clk->enable_reg);
	reg &= ~(MXC_CCM_CCGR_CG_MASK << clk->enable_shift);
	__raw_writel(reg, clk->enable_reg);

#if CONFIG_IS_ENABLED(MX51) || CONFIG_IS_ENABLED(MX53)
	reg = __raw_readl(&mxc_ccm->ccdr);
	reg |= MXC_CCM_CCDR_IPU_HS_MASK;
	__raw_writel(reg, &mxc_ccm->ccdr);

	reg = __raw_readl(&mxc_ccm->clpcr);
	reg |= MXC_CCM_CLPCR_BYPASS_IPU_LPM_HS;
	__raw_writel(reg, &mxc_ccm->clpcr);
#endif
}

static void ipu_pixel_clk_recalc(struct clk *clk)
{
	u32 div;
	u64 final_rate = (unsigned long long)clk->parent->rate * 16;

	div = __raw_readl(DI_BS_CLKGEN0(clk->id));
	debug("read BS_CLKGEN0 div:%d, final_rate:%lld, prate:%ld\n", div,
	      final_rate, clk->parent->rate);

	clk->rate = 0;
	if (div != 0) {
		do_div(final_rate, div);
		clk->rate = final_rate;
	}
}

static unsigned long ipu_pixel_clk_round_rate(struct clk *clk,
					      unsigned long rate)
{
	u64 div, final_rate;
	u32 remainder;
	u64 parent_rate = (unsigned long long)clk->parent->rate * 16;

	div = parent_rate;
	remainder = do_div(div, rate);
	if (remainder > (rate / 2))
		div++;
	if (div < 0x10)
		div = 0x10;
	if (div & ~0xFEF)
		div &= 0xFF8;
	else {
		if ((div & 0xC) == 0xC) {
			div += 0x10;
			div &= ~0xF;
		}
	}
	final_rate = parent_rate;
	do_div(final_rate, div);

	return final_rate;
}

static int ipu_pixel_clk_set_rate(struct clk *clk, unsigned long rate)
{
	u64 div, parent_rate;
	u32 remainder;

	parent_rate = (unsigned long long)clk->parent->rate * 16;
	div = parent_rate;
	remainder = do_div(div, rate);
	if (remainder > (rate / 2))
		div++;

	if ((div & 0xC) == 0xC) {
		div += 0x10;
		div &= ~0xF;
	}
	if (div > 0x1000)
		debug("Overflow, DI_BS_CLKGEN0 div:0x%x\n", (u32)div);

	__raw_writel(div, DI_BS_CLKGEN0(clk->id));
	__raw_writel((div / 16) << 16, DI_BS_CLKGEN1(clk->id));

	do_div(parent_rate, div);
	clk->rate = parent_rate;

	return 0;
}

static int ipu_pixel_clk_enable(struct clk *clk)
{
	u32 disp_gen = __raw_readl(IPU_DISP_GEN);
	disp_gen |= clk->id ? DI1_COUNTER_RELEASE : DI0_COUNTER_RELEASE;
	__raw_writel(disp_gen, IPU_DISP_GEN);

	return 0;
}

static void ipu_pixel_clk_disable(struct clk *clk)
{
	u32 disp_gen = __raw_readl(IPU_DISP_GEN);
	disp_gen &= clk->id ? ~DI1_COUNTER_RELEASE : ~DI0_COUNTER_RELEASE;
	__raw_writel(disp_gen, IPU_DISP_GEN);
}

static int ipu_pixel_clk_set_parent(struct clk *clk, struct clk *parent)
{
	u32 di_gen = __raw_readl(DI_GENERAL(clk->id));
	struct ipu_ctx *ctx = clk->ctx;

	if (parent == ctx->ipu_clk)
		di_gen &= ~DI_GEN_DI_CLK_EXT;
	else if (!IS_ERR(ctx->di_clk[clk->id]) && parent == ctx->ldb_clk)
		di_gen |= DI_GEN_DI_CLK_EXT;
	else
		return -EINVAL;

	__raw_writel(di_gen, DI_GENERAL(clk->id));
	ipu_pixel_clk_recalc(clk);
	return 0;
}

int ipu_pixel_clk_init_legacy(struct ipu_ctx *ctx, int id)
{
	struct clk *pixel_clk;

	pixel_clk = devm_kzalloc(ctx->dev, sizeof(*pixel_clk), GFP_KERNEL);
	if (!pixel_clk)
		return -ENOMEM;

	pixel_clk->name = "pixel_clk";
	pixel_clk->id = id;
	pixel_clk->ctx = ctx;
	pixel_clk->recalc = ipu_pixel_clk_recalc;
	pixel_clk->set_rate = ipu_pixel_clk_set_rate;
	pixel_clk->round_rate = ipu_pixel_clk_round_rate;
	pixel_clk->set_parent = ipu_pixel_clk_set_parent;
	pixel_clk->enable = ipu_pixel_clk_enable;
	pixel_clk->disable = ipu_pixel_clk_disable;
	pixel_clk->usecount = 0;

	ctx->pixel_clk[id] = pixel_clk;
	return 0;
}

int ipu_clk_init_legacy(struct ipu_ctx *ctx)
{
	struct clk *ipu_clk;

	ipu_clk = devm_kzalloc(ctx->dev, sizeof(*ipu_clk), GFP_KERNEL);
	if (!ipu_clk)
		return -ENOMEM;

	ipu_clk->name = "ipu_clk";
	ipu_clk->ctx = ctx;
#if CONFIG_IS_ENABLED(MX51) || CONFIG_IS_ENABLED(MX53)
	ipu_clk->enable_reg =
		(u32 *)(CCM_BASE_ADDR + offsetof(struct mxc_ccm_reg, CCGR5));
	ipu_clk->enable_shift = MXC_CCM_CCGR5_IPU_OFFSET;
#else
	ipu_clk->enable_reg =
		(u32 *)(CCM_BASE_ADDR + offsetof(struct mxc_ccm_reg, CCGR3));
	ipu_clk->enable_shift = MXC_CCM_CCGR3_IPU1_IPU_DI0_OFFSET;
#endif

	ipu_clk->enable = clk_ipu_enable;
	ipu_clk->disable = clk_ipu_disable;
	ipu_clk->usecount = 0;

#if CONFIG_IS_ENABLED(MX51)
	ipu_clk->rate = IPUV3_CLK_MX51;
#elif CONFIG_IS_ENABLED(MX53)
	ipu_clk->rate = IPUV3_CLK_MX53;
#else
	ipu_clk->rate = is_mx6sdl() ? IPUV3_CLK_MX6DL : IPUV3_CLK_MX6Q;
#endif

	ctx->ipu_clk = ipu_clk;
	return 0;
}

#if !defined CFG_SYS_LDB_CLOCK
#define CFG_SYS_LDB_CLOCK 65000000
#endif

int ipu_ldb_clk_init_legacy(struct ipu_ctx *ctx)
{
	struct clk *ldb_clk;

	ldb_clk = devm_kzalloc(ctx->dev, sizeof(*ldb_clk), GFP_KERNEL);
	if (!ldb_clk)
		return -ENOMEM;

	ldb_clk->name = "ldb_clk";
	ldb_clk->ctx = ctx;
	ldb_clk->rate = CFG_SYS_LDB_CLOCK;
	ldb_clk->usecount = 0;

	ctx->ldb_clk = ldb_clk;
	return 0;
}
