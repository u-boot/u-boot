/*
 * ZynqMP clock driver
 *
 * Copyright (C) 2016 Xilinx, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <linux/bitops.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <clk.h>

#define ZYNQMP_GEM0_REF_CTRL		0xFF5E0050
#define ZYNQMP_IOPLL_CTRL		0xFF5E0020
#define ZYNQMP_RPLL_CTRL		0xFF5E0030
#define ZYNQMP_DPLL_CTRL		0xFD1A002C
#define ZYNQMP_SIP_SVC_MMIO_WRITE	0xC2000013
#define ZYNQMP_SIP_SVC_MMIO_WRITE	0xC2000013
#define ZYNQMP_SIP_SVC_MMIO_WRITE	0xC2000013
#define ZYNQMP_SIP_SVC_MMIO_READ	0xC2000014
#define ZYNQMP_DIV_MAX_VAL		0x3F
#define ZYNQMP_DIV1_SHFT		8
#define ZYNQMP_DIV1_SHFT		8
#define ZYNQMP_DIV2_SHFT		16
#define ZYNQMP_DIV_MASK			0x3F
#define ZYNQMP_PLL_CTRL_FBDIV_MASK	0x7F
#define ZYNQMP_PLL_CTRL_FBDIV_SHFT	8
#define ZYNQMP_GEM_REF_CTRL_SRC_MASK	0x7
#define ZYNQMP_GEM0_CLK_ID		45
#define ZYNQMP_GEM1_CLK_ID		46
#define ZYNQMP_GEM2_CLK_ID		47
#define ZYNQMP_GEM3_CLK_ID		48

static unsigned long pss_ref_clk;

static int zynqmp_calculate_divisors(unsigned long req_rate,
				     unsigned long parent_rate,
				     u32 *div1, u32 *div2)
{
	u32 req_div = 1;
	u32 i;

	/*
	 * calculate two divisors to get
	 * required rate and each divisor
	 * should be less than 63
	 */
	req_div = DIV_ROUND_UP(parent_rate, req_rate);

	for (i = 1; i <= req_div; i++) {
		if ((req_div % i) == 0) {
			*div1 = req_div / i;
			*div2 = i;
			if ((*div1 < ZYNQMP_DIV_MAX_VAL) &&
			    (*div2 < ZYNQMP_DIV_MAX_VAL))
				return 0;
		}
	}

	return -1;
}

static int zynqmp_get_periph_id(unsigned long id)
{
	int periph_id;

	switch (id) {
	case ZYNQMP_GEM0_CLK_ID:
		periph_id = 0;
		break;
	case ZYNQMP_GEM1_CLK_ID:
		periph_id = 1;
		break;
	case ZYNQMP_GEM2_CLK_ID:
		periph_id = 2;
		break;
	case ZYNQMP_GEM3_CLK_ID:
		periph_id = 3;
		break;
	default:
		printf("%s, Invalid clock id:%ld\n", __func__, id);
		return -EINVAL;
	}

	return periph_id;
}

static int zynqmp_set_clk(unsigned long id, u32 div1, u32 div2)
{
	struct pt_regs regs;
	ulong reg;
	u32 mask, value;

	id = zynqmp_get_periph_id(id);
	if (id < 0)
		return -EINVAL;

	reg = (ulong)((u32 *)ZYNQMP_GEM0_REF_CTRL + id);
	mask = (ZYNQMP_DIV_MASK << ZYNQMP_DIV1_SHFT) |
	       (ZYNQMP_DIV_MASK << ZYNQMP_DIV2_SHFT);
	value = (div1 << ZYNQMP_DIV1_SHFT) | (div2 << ZYNQMP_DIV2_SHFT);

	debug("%s: reg:0x%lx, mask:0x%x, value:0x%x\n", __func__, reg, mask,
	      value);

	regs.regs[0] = ZYNQMP_SIP_SVC_MMIO_WRITE;
	regs.regs[1] = ((u64)mask << 32) | reg;
	regs.regs[2] = value;
	regs.regs[3] = 0;

	smc_call(&regs);

	return regs.regs[0];
}

static unsigned long zynqmp_clk_get_rate(struct clk *clk)
{
	struct pt_regs regs;
	ulong reg;
	unsigned long value;
	int id;

	id = zynqmp_get_periph_id(clk->id);
	if (id < 0)
		return -EINVAL;

	reg = (ulong)((u32 *)ZYNQMP_GEM0_REF_CTRL + id);

	regs.regs[0] = ZYNQMP_SIP_SVC_MMIO_READ;
	regs.regs[1] = reg;
	regs.regs[2] = 0;
	regs.regs[3] = 0;

	smc_call(&regs);

	value = upper_32_bits(regs.regs[0]);

	value &= ZYNQMP_GEM_REF_CTRL_SRC_MASK;

	switch (value) {
	case 0:
		regs.regs[1] = ZYNQMP_IOPLL_CTRL;
		break;
	case 2:
		regs.regs[1] = ZYNQMP_RPLL_CTRL;
		break;
	case 3:
		regs.regs[1] = ZYNQMP_DPLL_CTRL;
		break;
	default:
		return -EINVAL;
	}

	regs.regs[0] = ZYNQMP_SIP_SVC_MMIO_READ;
	regs.regs[2] = 0;
	regs.regs[3] = 0;

	smc_call(&regs);

	value = upper_32_bits(regs.regs[0]) &
		 (ZYNQMP_PLL_CTRL_FBDIV_MASK <<
		 ZYNQMP_PLL_CTRL_FBDIV_SHFT);
	value >>= ZYNQMP_PLL_CTRL_FBDIV_SHFT;
	value *= pss_ref_clk;

	return value;
}

static ulong zynqmp_clk_set_rate(struct clk *clk, unsigned long clk_rate)
{
	int ret;
	u32 div1 = 0;
	u32 div2 = 0;
	unsigned long input_clk;

	input_clk = zynqmp_clk_get_rate(clk);
	if (IS_ERR_VALUE(input_clk)) {
		dev_err(dev, "failed to get input_clk\n");
		return -EINVAL;
	}

	debug("%s: i/p CLK %ld, clk_rate:0x%ld\n", __func__, input_clk,
	      clk_rate);

	ret = zynqmp_calculate_divisors(clk_rate, input_clk, &div1, &div2);
	if (ret) {
		dev_err(dev, "failed to proper divisors\n");
		return -EINVAL;
	}

	debug("%s: Div1:%d, Div2:%d\n", __func__, div1, div2);

	ret = zynqmp_set_clk(clk->id, div1, div2);
	if (ret) {
		dev_err(dev, "failed to set gem clk\n");
		return -EINVAL;
	}

	return 0;
}

static int zynqmp_clk_probe(struct udevice *dev)
{
	struct clk clk;
	int ret;

	debug("%s\n", __func__);
	ret = clk_get_by_name(dev, "pss_ref_clk", &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get pss_ref_clk\n");
		return ret;
	}

	pss_ref_clk = clk_get_rate(&clk);
	if (IS_ERR_VALUE(pss_ref_clk)) {
		dev_err(dev, "failed to get rate pss_ref_clk\n");
		return -EINVAL;
	}

	return 0;
}

static struct clk_ops zynqmp_clk_ops = {
	.set_rate = zynqmp_clk_set_rate,
	.get_rate = zynqmp_clk_get_rate,
};

static const struct udevice_id zynqmp_clk_ids[] = {
	{ .compatible = "xlnx,zynqmp-clkc" },
	{ }
};

U_BOOT_DRIVER(zynqmp_clk) = {
	.name = "zynqmp-clk",
	.id = UCLASS_CLK,
	.of_match = zynqmp_clk_ids,
	.probe = zynqmp_clk_probe,
	.ops = &zynqmp_clk_ops,
};
