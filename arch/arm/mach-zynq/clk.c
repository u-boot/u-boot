/*
 * Copyright (C) 2013 Soren Brinkmann <soren.brinkmann@xilinx.com>
 * Copyright (C) 2013 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <clk.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clk.h>

/* Board oscillator frequency */
#ifndef CONFIG_ZYNQ_PS_CLK_FREQ
# define CONFIG_ZYNQ_PS_CLK_FREQ	33333333UL
#endif

/* Register bitfield defines */
#define PLLCTRL_FBDIV_MASK	0x7f000
#define PLLCTRL_FBDIV_SHIFT	12
#define PLLCTRL_BPFORCE_MASK	(1 << 4)
#define PLLCTRL_PWRDWN_MASK	2
#define PLLCTRL_PWRDWN_SHIFT	1
#define PLLCTRL_RESET_MASK	1
#define PLLCTRL_RESET_SHIFT	0

#define ZYNQ_CLK_MAXDIV		0x3f
#define CLK_CTRL_DIV1_SHIFT	20
#define CLK_CTRL_DIV1_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV1_SHIFT)
#define CLK_CTRL_DIV0_SHIFT	8
#define CLK_CTRL_DIV0_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV0_SHIFT)
#define CLK_CTRL_SRCSEL_SHIFT	4
#define CLK_CTRL_SRCSEL_MASK	(0x3 << CLK_CTRL_SRCSEL_SHIFT)

#define CLK_CTRL_DIV2X_SHIFT	26
#define CLK_CTRL_DIV2X_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV2X_SHIFT)
#define CLK_CTRL_DIV3X_SHIFT	20
#define CLK_CTRL_DIV3X_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV3X_SHIFT)

#define ZYNQ_CLKMUX_SEL_0	0
#define ZYNQ_CLKMUX_SEL_1	1
#define ZYNQ_CLKMUX_SEL_2	2
#define ZYNQ_CLKMUX_SEL_3	3

DECLARE_GLOBAL_DATA_PTR;

struct clk;

/**
 * struct zynq_clk_ops:
 * @set_rate:	Function pointer to set_rate() implementation
 * @get_rate:	Function pointer to get_rate() implementation
 */
struct zynq_clk_ops {
	int (*set_rate)(struct clk *clk, unsigned long rate);
	unsigned long (*get_rate)(struct clk *clk);
};

/**
 * struct clk:
 * @name:	Clock name
 * @frequency:	Currenct frequency
 * @parent:	Parent clock
 * @flags:	Clock flags
 * @reg:	Clock control register
 * @ops:	Clock operations
 */
struct clk {
	char		*name;
	unsigned long	frequency;
	enum zynq_clk	parent;
	unsigned int	flags;
	u32		*reg;
	struct zynq_clk_ops	ops;
};
#define ZYNQ_CLK_FLAGS_HAS_2_DIVS	1

static struct clk clks[clk_max];

/**
 * __zynq_clk_cpu_get_parent() - Decode clock multiplexer
 * @srcsel:	Mux select value
 * Returns the clock identifier associated with the selected mux input.
 */
static int __zynq_clk_cpu_get_parent(unsigned int srcsel)
{
	unsigned int ret;

	switch (srcsel) {
	case ZYNQ_CLKMUX_SEL_0:
	case ZYNQ_CLKMUX_SEL_1:
		ret = armpll_clk;
		break;
	case ZYNQ_CLKMUX_SEL_2:
		ret = ddrpll_clk;
		break;
	case ZYNQ_CLKMUX_SEL_3:
		ret = iopll_clk;
		break;
	default:
		ret = armpll_clk;
		break;
	}

	return ret;
}

/**
 * ddr2x_get_rate() - Get clock rate of DDR2x clock
 * @clk:	Clock handle
 * Returns the current clock rate of @clk.
 */
static unsigned long ddr2x_get_rate(struct clk *clk)
{
	u32 clk_ctrl = readl(clk->reg);
	u32 div = (clk_ctrl & CLK_CTRL_DIV2X_MASK) >> CLK_CTRL_DIV2X_SHIFT;

	return DIV_ROUND_CLOSEST(zynq_clk_get_rate(clk->parent), div);
}

/**
 * ddr3x_get_rate() - Get clock rate of DDR3x clock
 * @clk:	Clock handle
 * Returns the current clock rate of @clk.
 */
static unsigned long ddr3x_get_rate(struct clk *clk)
{
	u32 clk_ctrl = readl(clk->reg);
	u32 div = (clk_ctrl & CLK_CTRL_DIV3X_MASK) >> CLK_CTRL_DIV3X_SHIFT;

	return DIV_ROUND_CLOSEST(zynq_clk_get_rate(clk->parent), div);
}

static void init_ddr_clocks(void)
{
	u32 div0, div1;
	unsigned long prate = zynq_clk_get_rate(ddrpll_clk);
	u32 clk_ctrl = readl(&slcr_base->ddr_clk_ctrl);

	/* DDR2x */
	clks[ddr2x_clk].reg = &slcr_base->ddr_clk_ctrl;
	clks[ddr2x_clk].parent = ddrpll_clk;
	clks[ddr2x_clk].name = "ddr_2x";
	clks[ddr2x_clk].frequency = ddr2x_get_rate(&clks[ddr2x_clk]);
	clks[ddr2x_clk].ops.get_rate = ddr2x_get_rate;

	/* DDR3x */
	clks[ddr3x_clk].reg = &slcr_base->ddr_clk_ctrl;
	clks[ddr3x_clk].parent = ddrpll_clk;
	clks[ddr3x_clk].name = "ddr_3x";
	clks[ddr3x_clk].frequency = ddr3x_get_rate(&clks[ddr3x_clk]);
	clks[ddr3x_clk].ops.get_rate = ddr3x_get_rate;

	/* DCI */
	clk_ctrl = readl(&slcr_base->dci_clk_ctrl);
	div0 = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	div1 = (clk_ctrl & CLK_CTRL_DIV1_MASK) >> CLK_CTRL_DIV1_SHIFT;
	clks[dci_clk].reg = &slcr_base->dci_clk_ctrl;
	clks[dci_clk].parent = ddrpll_clk;
	clks[dci_clk].frequency = DIV_ROUND_CLOSEST(
			DIV_ROUND_CLOSEST(prate, div0), div1);
	clks[dci_clk].name = "dci";

	gd->bd->bi_ddr_freq = clks[ddr3x_clk].frequency / 1000000;
}

static void init_cpu_clocks(void)
{
	int clk_621;
	u32 reg, div, srcsel;
	enum zynq_clk parent;

	reg = readl(&slcr_base->arm_clk_ctrl);
	clk_621 = readl(&slcr_base->clk_621_true) & 1;
	div = (reg & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	srcsel = (reg & CLK_CTRL_SRCSEL_MASK) >> CLK_CTRL_SRCSEL_SHIFT;
	parent = __zynq_clk_cpu_get_parent(srcsel);

	/* cpu clocks */
	clks[cpu_6or4x_clk].reg = &slcr_base->arm_clk_ctrl;
	clks[cpu_6or4x_clk].parent = parent;
	clks[cpu_6or4x_clk].frequency = DIV_ROUND_CLOSEST(
			zynq_clk_get_rate(parent), div);
	clks[cpu_6or4x_clk].name = "cpu_6or4x";

	clks[cpu_3or2x_clk].reg = &slcr_base->arm_clk_ctrl;
	clks[cpu_3or2x_clk].parent = cpu_6or4x_clk;
	clks[cpu_3or2x_clk].frequency = zynq_clk_get_rate(cpu_6or4x_clk) / 2;
	clks[cpu_3or2x_clk].name = "cpu_3or2x";

	clks[cpu_2x_clk].reg = &slcr_base->arm_clk_ctrl;
	clks[cpu_2x_clk].parent = cpu_6or4x_clk;
	clks[cpu_2x_clk].frequency = zynq_clk_get_rate(cpu_6or4x_clk) /
			(2 + clk_621);
	clks[cpu_2x_clk].name = "cpu_2x";

	clks[cpu_1x_clk].reg = &slcr_base->arm_clk_ctrl;
	clks[cpu_1x_clk].parent = cpu_6or4x_clk;
	clks[cpu_1x_clk].frequency = zynq_clk_get_rate(cpu_6or4x_clk) /
			(4 + 2 * clk_621);
	clks[cpu_1x_clk].name = "cpu_1x";
}

/**
 * periph_calc_two_divs() - Calculate clock dividers
 * @cur_rate:	Current clock rate
 * @tgt_rate:	Target clock rate
 * @prate:	Parent clock rate
 * @div0:	First divider (output)
 * @div1:	Second divider (output)
 * Returns the actual clock rate possible.
 *
 * Calculates clock dividers for clocks with two 6-bit dividers.
 */
static unsigned long periph_calc_two_divs(unsigned long cur_rate,
		unsigned long tgt_rate, unsigned long prate, u32 *div0,
		u32 *div1)
{
	long err, best_err = (long)(~0UL >> 1);
	unsigned long rate, best_rate = 0;
	u32 d0, d1;

	for (d0 = 1; d0 <= ZYNQ_CLK_MAXDIV; d0++) {
		for (d1 = 1; d1 <= ZYNQ_CLK_MAXDIV >> 1; d1++) {
			rate = DIV_ROUND_CLOSEST(DIV_ROUND_CLOSEST(prate, d0),
					d1);
			err = abs(rate - tgt_rate);

			if (err < best_err) {
				*div0 = d0;
				*div1 = d1;
				best_err = err;
				best_rate = rate;
			}
		}
	}

	return best_rate;
}

/**
 * zynq_clk_periph_set_rate() - Set clock rate
 * @clk:	Handle of the peripheral clock
 * @rate:	New clock rate
 * Sets the clock frequency of @clk to @rate. Returns zero on success.
 */
static int zynq_clk_periph_set_rate(struct clk *clk,
		unsigned long rate)
{
	u32 ctrl, div0 = 0, div1 = 0;
	unsigned long prate, new_rate, cur_rate = clk->frequency;

	ctrl = readl(clk->reg);
	prate = zynq_clk_get_rate(clk->parent);
	ctrl &= ~CLK_CTRL_DIV0_MASK;

	if (clk->flags & ZYNQ_CLK_FLAGS_HAS_2_DIVS) {
		ctrl &= ~CLK_CTRL_DIV1_MASK;
		new_rate = periph_calc_two_divs(cur_rate, rate, prate, &div0,
				&div1);
		ctrl |= div1 << CLK_CTRL_DIV1_SHIFT;
	} else {
		div0 = DIV_ROUND_CLOSEST(prate, rate);
		div0 &= ZYNQ_CLK_MAXDIV;
		new_rate = DIV_ROUND_CLOSEST(rate, div0);
	}

	/* write new divs to hardware */
	ctrl |= div0 << CLK_CTRL_DIV0_SHIFT;
	writel(ctrl, clk->reg);

	/* update frequency in clk framework */
	clk->frequency = new_rate;

	return 0;
}

/**
 * zynq_clk_periph_get_rate() - Get clock rate
 * @clk:	Handle of the peripheral clock
 * Returns the current clock rate of @clk.
 */
static unsigned long zynq_clk_periph_get_rate(struct clk *clk)
{
	u32 clk_ctrl = readl(clk->reg);
	u32 div0 = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	u32 div1 = 1;

	if (clk->flags & ZYNQ_CLK_FLAGS_HAS_2_DIVS)
		div1 = (clk_ctrl & CLK_CTRL_DIV1_MASK) >> CLK_CTRL_DIV1_SHIFT;

	/* a register value of zero == division by 1 */
	if (!div0)
		div0 = 1;
	if (!div1)
		div1 = 1;

	return
		DIV_ROUND_CLOSEST(
			DIV_ROUND_CLOSEST(zynq_clk_get_rate(clk->parent), div0),
			div1);
}

/**
 * __zynq_clk_periph_get_parent() - Decode clock multiplexer
 * @srcsel:	Mux select value
 * Returns the clock identifier associated with the selected mux input.
 */
static enum zynq_clk __zynq_clk_periph_get_parent(u32 srcsel)
{
	switch (srcsel) {
	case ZYNQ_CLKMUX_SEL_0:
	case ZYNQ_CLKMUX_SEL_1:
		return iopll_clk;
	case ZYNQ_CLKMUX_SEL_2:
		return armpll_clk;
	case ZYNQ_CLKMUX_SEL_3:
		return ddrpll_clk;
	default:
		return 0;
	}
}

/**
 * zynq_clk_periph_get_parent() - Decode clock multiplexer
 * @clk:	Clock handle
 * Returns the clock identifier associated with the selected mux input.
 */
static enum zynq_clk zynq_clk_periph_get_parent(struct clk *clk)
{
	u32 clk_ctrl = readl(clk->reg);
	u32 srcsel = (clk_ctrl & CLK_CTRL_SRCSEL_MASK) >> CLK_CTRL_SRCSEL_SHIFT;

	return __zynq_clk_periph_get_parent(srcsel);
}

/**
 * zynq_clk_register_periph_clk() - Set up a peripheral clock with the framework
 * @clk:	Pointer to struct clk for the clock
 * @ctrl:	Clock control register
 * @name:	PLL name
 * @two_divs:	Indicates whether the clock features one or two dividers
 */
static int zynq_clk_register_periph_clk(struct clk *clk, u32 *ctrl, char *name,
		bool two_divs)
{
	clk->name = name;
	clk->reg = ctrl;
	if (two_divs)
		clk->flags = ZYNQ_CLK_FLAGS_HAS_2_DIVS;
	clk->parent = zynq_clk_periph_get_parent(clk);
	clk->frequency = zynq_clk_periph_get_rate(clk);
	clk->ops.get_rate = zynq_clk_periph_get_rate;
	clk->ops.set_rate = zynq_clk_periph_set_rate;

	return 0;
}

static void init_periph_clocks(void)
{
	zynq_clk_register_periph_clk(&clks[gem0_clk], &slcr_base->gem0_clk_ctrl,
				     "gem0", 1);
	zynq_clk_register_periph_clk(&clks[gem1_clk], &slcr_base->gem1_clk_ctrl,
				     "gem1", 1);

	zynq_clk_register_periph_clk(&clks[smc_clk], &slcr_base->smc_clk_ctrl,
				     "smc", 0);

	zynq_clk_register_periph_clk(&clks[lqspi_clk],
				     &slcr_base->lqspi_clk_ctrl, "lqspi", 0);

	zynq_clk_register_periph_clk(&clks[sdio0_clk],
				     &slcr_base->sdio_clk_ctrl, "sdio0", 0);
	zynq_clk_register_periph_clk(&clks[sdio1_clk],
				     &slcr_base->sdio_clk_ctrl, "sdio1", 0);

	zynq_clk_register_periph_clk(&clks[spi0_clk], &slcr_base->spi_clk_ctrl,
				     "spi0", 0);
	zynq_clk_register_periph_clk(&clks[spi1_clk], &slcr_base->spi_clk_ctrl,
				     "spi1", 0);

	zynq_clk_register_periph_clk(&clks[uart0_clk],
				     &slcr_base->uart_clk_ctrl, "uart0", 0);
	zynq_clk_register_periph_clk(&clks[uart1_clk],
				     &slcr_base->uart_clk_ctrl, "uart1", 0);

	zynq_clk_register_periph_clk(&clks[dbg_trc_clk],
				     &slcr_base->dbg_clk_ctrl, "dbg_trc", 0);
	zynq_clk_register_periph_clk(&clks[dbg_apb_clk],
				     &slcr_base->dbg_clk_ctrl, "dbg_apb", 0);

	zynq_clk_register_periph_clk(&clks[pcap_clk],
				     &slcr_base->pcap_clk_ctrl, "pcap", 0);

	zynq_clk_register_periph_clk(&clks[fclk0_clk],
				     &slcr_base->fpga0_clk_ctrl, "fclk0", 1);
	zynq_clk_register_periph_clk(&clks[fclk1_clk],
				     &slcr_base->fpga1_clk_ctrl, "fclk1", 1);
	zynq_clk_register_periph_clk(&clks[fclk2_clk],
				     &slcr_base->fpga2_clk_ctrl, "fclk2", 1);
	zynq_clk_register_periph_clk(&clks[fclk3_clk],
				     &slcr_base->fpga3_clk_ctrl, "fclk3", 1);
}

/**
 * zynq_clk_register_aper_clk() - Set up a APER clock with the framework
 * @clk:	Pointer to struct clk for the clock
 * @ctrl:	Clock control register
 * @name:	PLL name
 */
static void zynq_clk_register_aper_clk(struct clk *clk, u32 *ctrl, char *name)
{
	clk->name = name;
	clk->reg = ctrl;
	clk->parent = cpu_1x_clk;
	clk->frequency = zynq_clk_get_rate(clk->parent);
}

static void init_aper_clocks(void)
{
	zynq_clk_register_aper_clk(&clks[usb0_aper_clk],
				   &slcr_base->aper_clk_ctrl, "usb0_aper");
	zynq_clk_register_aper_clk(&clks[usb1_aper_clk],
				   &slcr_base->aper_clk_ctrl, "usb1_aper");

	zynq_clk_register_aper_clk(&clks[gem0_aper_clk],
				   &slcr_base->aper_clk_ctrl, "gem0_aper");
	zynq_clk_register_aper_clk(&clks[gem1_aper_clk],
				   &slcr_base->aper_clk_ctrl, "gem1_aper");

	zynq_clk_register_aper_clk(&clks[sdio0_aper_clk],
				   &slcr_base->aper_clk_ctrl, "sdio0_aper");
	zynq_clk_register_aper_clk(&clks[sdio1_aper_clk],
				   &slcr_base->aper_clk_ctrl, "sdio1_aper");

	zynq_clk_register_aper_clk(&clks[spi0_aper_clk],
				   &slcr_base->aper_clk_ctrl, "spi0_aper");
	zynq_clk_register_aper_clk(&clks[spi1_aper_clk],
				   &slcr_base->aper_clk_ctrl, "spi1_aper");

	zynq_clk_register_aper_clk(&clks[can0_aper_clk],
				   &slcr_base->aper_clk_ctrl, "can0_aper");
	zynq_clk_register_aper_clk(&clks[can1_aper_clk],
				   &slcr_base->aper_clk_ctrl, "can1_aper");

	zynq_clk_register_aper_clk(&clks[i2c0_aper_clk],
				   &slcr_base->aper_clk_ctrl, "i2c0_aper");
	zynq_clk_register_aper_clk(&clks[i2c1_aper_clk],
				   &slcr_base->aper_clk_ctrl, "i2c1_aper");

	zynq_clk_register_aper_clk(&clks[uart0_aper_clk],
				   &slcr_base->aper_clk_ctrl, "uart0_aper");
	zynq_clk_register_aper_clk(&clks[uart1_aper_clk],
				   &slcr_base->aper_clk_ctrl, "uart1_aper");

	zynq_clk_register_aper_clk(&clks[gpio_aper_clk],
				   &slcr_base->aper_clk_ctrl, "gpio_aper");

	zynq_clk_register_aper_clk(&clks[lqspi_aper_clk],
				   &slcr_base->aper_clk_ctrl, "lqspi_aper");

	zynq_clk_register_aper_clk(&clks[smc_aper_clk],
				   &slcr_base->aper_clk_ctrl, "smc_aper");
}

/**
 * __zynq_clk_pll_get_rate() - Get PLL rate
 * @addr:	Address of the PLL's control register
 * Returns the current PLL output rate.
 */
static unsigned long __zynq_clk_pll_get_rate(u32 *addr)
{
	u32 reg, mul, bypass;

	reg = readl(addr);
	bypass = reg & PLLCTRL_BPFORCE_MASK;
	if (bypass)
		mul = 1;
	else
		mul = (reg & PLLCTRL_FBDIV_MASK) >> PLLCTRL_FBDIV_SHIFT;

	return CONFIG_ZYNQ_PS_CLK_FREQ * mul;
}

/**
 * zynq_clk_pll_get_rate() - Get PLL rate
 * @pll:	Handle of the PLL
 * Returns the current clock rate of @pll.
 */
static unsigned long zynq_clk_pll_get_rate(struct clk *pll)
{
	return __zynq_clk_pll_get_rate(pll->reg);
}

/**
 * zynq_clk_register_pll() - Set up a PLL with the framework
 * @clk:	Pointer to struct clk for the PLL
 * @ctrl:	PLL control register
 * @name:	PLL name
 * @prate:	PLL input clock rate
 */
static void zynq_clk_register_pll(struct clk *clk, u32 *ctrl, char *name,
		unsigned long prate)
{
	clk->name = name;
	clk->reg = ctrl;
	clk->frequency = zynq_clk_pll_get_rate(clk);
	clk->ops.get_rate = zynq_clk_pll_get_rate;
}

/**
 * clkid_2_register() - Get clock control register
 * @id:	Clock identifier of one of the PLLs
 * Returns the address of the requested PLL's control register.
 */
static u32 *clkid_2_register(enum zynq_clk id)
{
	switch (id) {
	case armpll_clk:
		return &slcr_base->arm_pll_ctrl;
	case ddrpll_clk:
		return &slcr_base->ddr_pll_ctrl;
	case iopll_clk:
		return &slcr_base->io_pll_ctrl;
	default:
		return &slcr_base->io_pll_ctrl;
	}
}

/* API */
/**
 * zynq_clk_early_init() - Early init for the clock framework
 *
 * This function is called from before relocation and sets up the CPU clock
 * frequency in the global data struct.
 */
void zynq_clk_early_init(void)
{
	u32 reg = readl(&slcr_base->arm_clk_ctrl);
	u32 div = (reg & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	u32 srcsel = (reg & CLK_CTRL_SRCSEL_MASK) >> CLK_CTRL_SRCSEL_SHIFT;
	enum zynq_clk parent = __zynq_clk_cpu_get_parent(srcsel);
	u32 *pllreg = clkid_2_register(parent);
	unsigned long prate = __zynq_clk_pll_get_rate(pllreg);

	if (!div)
		div = 1;

	gd->cpu_clk = DIV_ROUND_CLOSEST(prate, div);
}

/**
 * get_uart_clk() - Get UART input frequency
 * @dev_index:	UART ID
 * Returns UART input clock frequency in Hz.
 *
 * Compared to zynq_clk_get_rate() this function is designed to work before
 * relocation and can be called when the serial UART is set up.
 */
unsigned long get_uart_clk(int dev_index)
{
	u32 reg = readl(&slcr_base->uart_clk_ctrl);
	u32 div = (reg & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	u32 srcsel = (reg & CLK_CTRL_SRCSEL_MASK) >> CLK_CTRL_SRCSEL_SHIFT;
	enum zynq_clk parent = __zynq_clk_periph_get_parent(srcsel);
	u32 *pllreg = clkid_2_register(parent);
	unsigned long prate = __zynq_clk_pll_get_rate(pllreg);

	if (!div)
		div = 1;

	return DIV_ROUND_CLOSEST(prate, div);
}

/**
 * set_cpu_clk_info() - Initialize clock framework
 * Always returns zero.
 *
 * This function is called from common code after relocation and sets up the
 * clock framework. The framework must not be used before this function had been
 * called.
 */
int set_cpu_clk_info(void)
{
	zynq_clk_register_pll(&clks[armpll_clk], &slcr_base->arm_pll_ctrl,
			      "armpll", CONFIG_ZYNQ_PS_CLK_FREQ);
	zynq_clk_register_pll(&clks[ddrpll_clk], &slcr_base->ddr_pll_ctrl,
			      "ddrpll", CONFIG_ZYNQ_PS_CLK_FREQ);
	zynq_clk_register_pll(&clks[iopll_clk], &slcr_base->io_pll_ctrl,
			      "iopll", CONFIG_ZYNQ_PS_CLK_FREQ);

	init_ddr_clocks();
	init_cpu_clocks();
	init_periph_clocks();
	init_aper_clocks();

	gd->bd->bi_arm_freq = gd->cpu_clk / 1000000;
	gd->bd->bi_dsp_freq = 0;

	return 0;
}

/**
 * zynq_clk_get_rate() - Get clock rate
 * @clk:	Clock identifier
 * Returns the current clock rate of @clk on success or zero for an invalid
 * clock id.
 */
unsigned long zynq_clk_get_rate(enum zynq_clk clk)
{
	if (clk < 0 || clk >= clk_max)
		return 0;

	return clks[clk].frequency;
}

/**
 * zynq_clk_set_rate() - Set clock rate
 * @clk:	Clock identifier
 * @rate:	Requested clock rate
 * Passes on the return value from the clock's set_rate() function or negative
 * errno.
 */
int zynq_clk_set_rate(enum zynq_clk clk, unsigned long rate)
{
	if (clk < 0 || clk >= clk_max)
		return -ENODEV;

	if (clks[clk].ops.set_rate)
		return clks[clk].ops.set_rate(&clks[clk], rate);

	return -ENXIO;
}

/**
 * zynq_clk_get_name() - Get clock name
 * @clk:	Clock identifier
 * Returns the name of @clk.
 */
const char *zynq_clk_get_name(enum zynq_clk clk)
{
	return clks[clk].name;
}

/**
 * soc_clk_dump() - Print clock frequencies
 * Returns zero on success
 *
 * Implementation for the clk dump command.
 */
int soc_clk_dump(void)
{
	int i;

	printf("clk\t\tfrequency\n");
	for (i = 0; i < clk_max; i++) {
		const char *name = zynq_clk_get_name(i);
		if (name)
			printf("%10s%20lu\n", name, zynq_clk_get_rate(i));
	}

	return 0;
}
