// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */
#include <kendryte/clk.h>

#include <common.h>
#include <dt-bindings/clock/k210-sysctl.h>
#include <dt-bindings/mfd/k210-sysctl.h>
#include <dm.h>
#include <log.h>
#include <mapmem.h>

#include <kendryte/bypass.h>
#include <kendryte/pll.h>

/*
 * All parameters for different sub-clocks are collected into parameter arrays.
 * These parameters are then initialized by the clock which uses them during
 * probe. To save space, ids are automatically generated for each sub-clock by
 * using an enum. Instead of storing a parameter struct for each clock, even for
 * those clocks which don't use a particular type of sub-clock, we can just
 * store the parameters for the clocks which need them.
 *
 * So why do it like this? Arranging all the sub-clocks together makes it very
 * easy to find bugs in the code.
 */

/**
 * enum k210_clk_div_type - The type of divider
 * @K210_DIV_ONE: freq = parent / (reg + 1)
 * @K210_DIV_EVEN: freq = parent / 2 / (reg + 1)
 * @K210_DIV_POWER: freq = parent / (2 << reg)
 * @K210_DIV_FIXED: freq = parent / factor
 */
enum k210_clk_div_type {
	K210_DIV_ONE,
	K210_DIV_EVEN,
	K210_DIV_POWER,
	K210_DIV_FIXED,
};

/**
 * struct k210_div_params - Parameters for dividing clocks
 * @type: An &enum k210_clk_div_type specifying the dividing formula
 * @off: The offset of the divider from the sysctl base address
 * @shift: The offset of the LSB of the divider
 * @width: The number of bits in the divider
 * @div: The fixed divisor for this divider
 */
struct k210_div_params {
	u8 type;
	union {
		struct {
			u8 off;
			u8 shift;
			u8 width;
		};
		u8 div;
	};
};

#define DIV_LIST \
	DIV(K210_CLK_ACLK,   K210_SYSCTL_SEL0,  1,  2, K210_DIV_POWER) \
	DIV(K210_CLK_APB0,   K210_SYSCTL_SEL0,  3,  3, K210_DIV_ONE) \
	DIV(K210_CLK_APB1,   K210_SYSCTL_SEL0,  6,  3, K210_DIV_ONE) \
	DIV(K210_CLK_APB2,   K210_SYSCTL_SEL0,  9,  3, K210_DIV_ONE) \
	DIV(K210_CLK_SRAM0,  K210_SYSCTL_THR0,  0,  4, K210_DIV_ONE) \
	DIV(K210_CLK_SRAM1,  K210_SYSCTL_THR0,  4,  4, K210_DIV_ONE) \
	DIV(K210_CLK_AI,     K210_SYSCTL_THR0,  8,  4, K210_DIV_ONE) \
	DIV(K210_CLK_DVP,    K210_SYSCTL_THR0, 12,  4, K210_DIV_ONE) \
	DIV(K210_CLK_ROM,    K210_SYSCTL_THR0, 16,  4, K210_DIV_ONE) \
	DIV(K210_CLK_SPI0,   K210_SYSCTL_THR1,  0,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_SPI1,   K210_SYSCTL_THR1,  8,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_SPI2,   K210_SYSCTL_THR1, 16,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_SPI3,   K210_SYSCTL_THR1, 24,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_TIMER0, K210_SYSCTL_THR2,  0,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_TIMER1, K210_SYSCTL_THR2,  8,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_TIMER2, K210_SYSCTL_THR2, 16,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S0,   K210_SYSCTL_THR3,  0, 16, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S1,   K210_SYSCTL_THR3, 16, 16, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S2,   K210_SYSCTL_THR4,  0, 16, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S0_M, K210_SYSCTL_THR4, 16,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S1_M, K210_SYSCTL_THR4, 24,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2S2_M, K210_SYSCTL_THR4,  0,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2C0,   K210_SYSCTL_THR5,  8,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2C1,   K210_SYSCTL_THR5, 16,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_I2C2,   K210_SYSCTL_THR5, 24,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_WDT0,   K210_SYSCTL_THR6,  0,  8, K210_DIV_EVEN) \
	DIV(K210_CLK_WDT1,   K210_SYSCTL_THR6,  8,  8, K210_DIV_EVEN) \
	DIV_FIXED(K210_CLK_CLINT, 50) \

#define _DIVIFY(id) K210_CLK_DIV_##id
#define DIVIFY(id) _DIVIFY(id)

enum k210_div_id {
#define DIV(id, ...) DIVIFY(id),
#define DIV_FIXED DIV
	DIV_LIST
#undef DIV
#undef DIV_FIXED
	K210_CLK_DIV_NONE,
};

static const struct k210_div_params k210_divs[] = {
#define DIV(id, _off, _shift, _width, _type) \
	[DIVIFY(id)] = { \
		.type = (_type), \
		.off = (_off), \
		.shift = (_shift), \
		.width = (_width), \
	},
#define DIV_FIXED(id, _div) \
	[DIVIFY(id)] = { \
		.type = K210_DIV_FIXED, \
		.div = (_div) \
	},
	DIV_LIST
#undef DIV
#undef DIV_FIXED
};

#undef DIV
#undef DIV_LIST

/**
 * struct k210_gate_params - Parameters for gated clocks
 * @off: The offset of the gate from the sysctl base address
 * @bit_idx: The index of the bit within the register
 */
struct k210_gate_params {
	u8 off;
	u8 bit_idx;
};

#define GATE_LIST \
	GATE(K210_CLK_CPU,    K210_SYSCTL_EN_CENT,  0) \
	GATE(K210_CLK_SRAM0,  K210_SYSCTL_EN_CENT,  1) \
	GATE(K210_CLK_SRAM1,  K210_SYSCTL_EN_CENT,  2) \
	GATE(K210_CLK_APB0,   K210_SYSCTL_EN_CENT,  3) \
	GATE(K210_CLK_APB1,   K210_SYSCTL_EN_CENT,  4) \
	GATE(K210_CLK_APB2,   K210_SYSCTL_EN_CENT,  5) \
	GATE(K210_CLK_ROM,    K210_SYSCTL_EN_PERI,  0) \
	GATE(K210_CLK_DMA,    K210_SYSCTL_EN_PERI,  1) \
	GATE(K210_CLK_AI,     K210_SYSCTL_EN_PERI,  2) \
	GATE(K210_CLK_DVP,    K210_SYSCTL_EN_PERI,  3) \
	GATE(K210_CLK_FFT,    K210_SYSCTL_EN_PERI,  4) \
	GATE(K210_CLK_GPIO,   K210_SYSCTL_EN_PERI,  5) \
	GATE(K210_CLK_SPI0,   K210_SYSCTL_EN_PERI,  6) \
	GATE(K210_CLK_SPI1,   K210_SYSCTL_EN_PERI,  7) \
	GATE(K210_CLK_SPI2,   K210_SYSCTL_EN_PERI,  8) \
	GATE(K210_CLK_SPI3,   K210_SYSCTL_EN_PERI,  9) \
	GATE(K210_CLK_I2S0,   K210_SYSCTL_EN_PERI, 10) \
	GATE(K210_CLK_I2S1,   K210_SYSCTL_EN_PERI, 11) \
	GATE(K210_CLK_I2S2,   K210_SYSCTL_EN_PERI, 12) \
	GATE(K210_CLK_I2C0,   K210_SYSCTL_EN_PERI, 13) \
	GATE(K210_CLK_I2C1,   K210_SYSCTL_EN_PERI, 14) \
	GATE(K210_CLK_I2C2,   K210_SYSCTL_EN_PERI, 15) \
	GATE(K210_CLK_UART1,  K210_SYSCTL_EN_PERI, 16) \
	GATE(K210_CLK_UART2,  K210_SYSCTL_EN_PERI, 17) \
	GATE(K210_CLK_UART3,  K210_SYSCTL_EN_PERI, 18) \
	GATE(K210_CLK_AES,    K210_SYSCTL_EN_PERI, 19) \
	GATE(K210_CLK_FPIOA,  K210_SYSCTL_EN_PERI, 20) \
	GATE(K210_CLK_TIMER0, K210_SYSCTL_EN_PERI, 21) \
	GATE(K210_CLK_TIMER1, K210_SYSCTL_EN_PERI, 22) \
	GATE(K210_CLK_TIMER2, K210_SYSCTL_EN_PERI, 23) \
	GATE(K210_CLK_WDT0,   K210_SYSCTL_EN_PERI, 24) \
	GATE(K210_CLK_WDT1,   K210_SYSCTL_EN_PERI, 25) \
	GATE(K210_CLK_SHA,    K210_SYSCTL_EN_PERI, 26) \
	GATE(K210_CLK_OTP,    K210_SYSCTL_EN_PERI, 27) \
	GATE(K210_CLK_RTC,    K210_SYSCTL_EN_PERI, 29)

#define _GATEIFY(id) K210_CLK_GATE_##id
#define GATEIFY(id) _GATEIFY(id)

enum k210_gate_id {
#define GATE(id, ...) GATEIFY(id),
	GATE_LIST
#undef GATE
	K210_CLK_GATE_NONE,
};

static const struct k210_gate_params k210_gates[] = {
#define GATE(id, _off, _idx) \
	[GATEIFY(id)] = { \
		.off = (_off), \
		.bit_idx = (_idx), \
	},
	GATE_LIST
#undef GATE
};

#undef GATE_LIST

/* The most parents is PLL2 */
#define K210_CLK_MAX_PARENTS 3

/**
 * struct k210_mux_params - Parameters for muxed clocks
 * @parents: A list of parent clock ids
 * @num_parents: The number of parent clocks
 * @off: The offset of the mux from the base sysctl address
 * @shift: The offset of the LSB of the mux selector
 * @width: The number of bits in the mux selector
 */
struct k210_mux_params {
	u8 parents[K210_CLK_MAX_PARENTS];
	u8 num_parents;
	u8 off;
	u8 shift;
	u8 width;
};

#define MUX(id, reg, shift, width) \
	MUX_PARENTS(id, reg, shift, width, K210_CLK_IN0, K210_CLK_PLL0)
#define MUX_LIST \
	MUX_PARENTS(K210_CLK_PLL2, K210_SYSCTL_PLL2, 26, 2, \
		    K210_CLK_IN0, K210_CLK_PLL0, K210_CLK_PLL1) \
	MUX(K210_CLK_ACLK, K210_SYSCTL_SEL0, 0, 1) \
	MUX(K210_CLK_SPI3,   K210_SYSCTL_SEL0, 12, 1) \
	MUX(K210_CLK_TIMER0, K210_SYSCTL_SEL0, 13, 1) \
	MUX(K210_CLK_TIMER1, K210_SYSCTL_SEL0, 14, 1) \
	MUX(K210_CLK_TIMER2, K210_SYSCTL_SEL0, 15, 1)

#define _MUXIFY(id) K210_CLK_MUX_##id
#define MUXIFY(id) _MUXIFY(id)

enum k210_mux_id {
#define MUX_PARENTS(id, ...) MUXIFY(id),
	MUX_LIST
#undef MUX_PARENTS
	K210_CLK_MUX_NONE,
};

static const struct k210_mux_params k210_muxes[] = {
#define MUX_PARENTS(id, _off, _shift, _width, ...) \
	[MUXIFY(id)] = { \
		.parents = { __VA_ARGS__ }, \
		.num_parents = __count_args(__VA_ARGS__), \
		.off = (_off), \
		.shift = (_shift), \
		.width = (_width), \
	},
	MUX_LIST
#undef MUX_PARENTS
};

#undef MUX
#undef MUX_LIST

/**
 * enum k210_clk_flags - The type of a K210 clock
 * @K210_CLKF_MUX: This clock has a mux and not a static parent
 * @K210_CLKF_PLL: This clock is a PLL
 */
enum k210_clk_flags {
	K210_CLKF_MUX = BIT(0),
	K210_CLKF_PLL = BIT(1),
};

/**
 * struct k210_clk_params - The parameters defining a K210 clock
 * @name: The name of the clock
 * @flags: A set of &enum k210_clk_flags defining which fields are valid
 * @mux: An &enum k210_mux_id of this clock's mux
 * @parent: The clock id of this clock's parent
 * @pll: The id of the PLL (if this clock is a PLL)
 * @div: An &enum k210_div_id of this clock's divider
 * @gate: An &enum k210_gate_id of this clock's gate
 */
struct k210_clk_params {
#if CONFIG_IS_ENABLED(CMD_CLK)
	const char *name;
#endif
	u8 flags;
	union {
		u8 parent;
		u8 mux;
	};
	union {
		u8 pll;
		struct {
			u8 div;
			u8 gate;
		};
	};
};


static const struct k210_clk_params k210_clks[] = {
#if CONFIG_IS_ENABLED(CMD_CLK)
#define NAME(_name) .name = (_name),
#else
#define NAME(name)
#endif
#define CLK(id, _name, _parent, _div, _gate) \
	[id] = { \
		NAME(_name) \
		.parent = (_parent), \
		.div = (_div), \
		.gate = (_gate), \
	}
#define CLK_MUX(id, _name, _mux, _div, _gate) \
	[id] = { \
		NAME(_name) \
		.flags = K210_CLKF_MUX, \
		.mux = (_mux), \
		.div = (_div), \
		.gate = (_gate), \
	}
#define CLK_PLL(id, _pll, _parent) \
	[id] = { \
		NAME("pll" #_pll) \
		.flags = K210_CLKF_PLL, \
		.parent = (_parent), \
		.pll = (_pll), \
	}
#define CLK_FULL(id, name) \
	CLK_MUX(id, name, MUXIFY(id), DIVIFY(id), GATEIFY(id))
#define CLK_NOMUX(id, name, parent) \
	CLK(id, name, parent, DIVIFY(id), GATEIFY(id))
#define CLK_DIV(id, name, parent) \
	CLK(id, name, parent, DIVIFY(id), K210_CLK_GATE_NONE)
#define CLK_GATE(id, name, parent) \
	CLK(id, name, parent, K210_CLK_DIV_NONE, GATEIFY(id))
	CLK_PLL(K210_CLK_PLL0, 0, K210_CLK_IN0),
	CLK_PLL(K210_CLK_PLL1, 1, K210_CLK_IN0),
	[K210_CLK_PLL2] = {
		NAME("pll2")
		.flags = K210_CLKF_MUX | K210_CLKF_PLL,
		.mux = MUXIFY(K210_CLK_PLL2),
		.pll = 2,
	},
	CLK_MUX(K210_CLK_ACLK, "aclk", MUXIFY(K210_CLK_ACLK),
		DIVIFY(K210_CLK_ACLK), K210_CLK_GATE_NONE),
	CLK_FULL(K210_CLK_SPI3,   "spi3"),
	CLK_FULL(K210_CLK_TIMER0, "timer0"),
	CLK_FULL(K210_CLK_TIMER1, "timer1"),
	CLK_FULL(K210_CLK_TIMER2, "timer2"),
	CLK_NOMUX(K210_CLK_SRAM0, "sram0",  K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_SRAM1, "sram1",  K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_ROM,   "rom",    K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_DVP,   "dvp",    K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_APB0,  "apb0",   K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_APB1,  "apb1",   K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_APB2,  "apb2",   K210_CLK_ACLK),
	CLK_NOMUX(K210_CLK_AI,    "ai",     K210_CLK_PLL1),
	CLK_NOMUX(K210_CLK_I2S0,  "i2s0",   K210_CLK_PLL2),
	CLK_NOMUX(K210_CLK_I2S1,  "i2s1",   K210_CLK_PLL2),
	CLK_NOMUX(K210_CLK_I2S2,  "i2s2",   K210_CLK_PLL2),
	CLK_NOMUX(K210_CLK_WDT0,  "wdt0",   K210_CLK_IN0),
	CLK_NOMUX(K210_CLK_WDT1,  "wdt1",   K210_CLK_IN0),
	CLK_NOMUX(K210_CLK_SPI0,  "spi0",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_SPI1,  "spi1",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_SPI2,  "spi2",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_I2C0,  "i2c0",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_I2C1,  "i2c1",   K210_CLK_PLL0),
	CLK_NOMUX(K210_CLK_I2C2,  "i2c2",   K210_CLK_PLL0),
	CLK_DIV(K210_CLK_I2S0_M,  "i2s0_m", K210_CLK_PLL2),
	CLK_DIV(K210_CLK_I2S1_M,  "i2s1_m", K210_CLK_PLL2),
	CLK_DIV(K210_CLK_I2S2_M,  "i2s2_m", K210_CLK_PLL2),
	CLK_DIV(K210_CLK_CLINT,   "clint",  K210_CLK_ACLK),
	CLK_GATE(K210_CLK_CPU,    "cpu",    K210_CLK_ACLK),
	CLK_GATE(K210_CLK_DMA,    "dma",    K210_CLK_ACLK),
	CLK_GATE(K210_CLK_FFT,    "fft",    K210_CLK_ACLK),
	CLK_GATE(K210_CLK_GPIO,   "gpio",   K210_CLK_APB0),
	CLK_GATE(K210_CLK_UART1,  "uart1",  K210_CLK_APB0),
	CLK_GATE(K210_CLK_UART2,  "uart2",  K210_CLK_APB0),
	CLK_GATE(K210_CLK_UART3,  "uart3",  K210_CLK_APB0),
	CLK_GATE(K210_CLK_FPIOA,  "fpioa",  K210_CLK_APB0),
	CLK_GATE(K210_CLK_SHA,    "sha",    K210_CLK_APB0),
	CLK_GATE(K210_CLK_AES,    "aes",    K210_CLK_APB1),
	CLK_GATE(K210_CLK_OTP,    "otp",    K210_CLK_APB1),
	CLK_GATE(K210_CLK_RTC,    "rtc",    K210_CLK_IN0),
#undef NAME
#undef CLK_PLL
#undef CLK
#undef CLK_FULL
#undef CLK_NOMUX
#undef CLK_DIV
#undef CLK_GATE
#undef CLK_LIST
};

static u32 k210_clk_readl(struct k210_clk_priv *priv, u8 off, u8 shift,
			  u8 width)
{
	u32 reg = readl(priv->base + off);

	return (reg >> shift) & (BIT(width) - 1);
}

static void k210_clk_writel(struct k210_clk_priv *priv, u8 off, u8 shift,
			    u8 width, u32 val)
{
	u32 reg = readl(priv->base + off);
	u32 mask = (BIT(width) - 1) << shift;

	reg &= ~mask;
	reg |= mask & (val << shift);
	writel(reg, priv->base + off);
}

static int k210_clk_get_parent(struct k210_clk_priv *priv, int id)
{
	u32 sel;
	const struct k210_mux_params *mux;

	if (!(k210_clks[id].flags & K210_CLKF_MUX))
		return k210_clks[id].parent;
	mux = &k210_muxes[k210_clks[id].mux];

	sel = k210_clk_readl(priv, mux->off, mux->shift, mux->width);
	assert(sel < mux->num_parents);
	return mux->parents[sel];
}

static ulong do_k210_clk_get_rate(struct k210_clk_priv *priv, int id)
{
	int parent;
	u32 val;
	ulong parent_rate;
	const struct k210_div_params *div;

	if (id == K210_CLK_IN0)
		return clk_get_rate(&priv->in0);

	parent = k210_clk_get_parent(priv, id);
	parent_rate = do_k210_clk_get_rate(priv, parent);

	if (k210_clks[id].flags & K210_CLKF_PLL)
		return k210_pll_get_rate(priv, k210_clks[id].pll, parent_rate);

	if (k210_clks[id].div == K210_CLK_DIV_NONE)
		return parent_rate;
	div = &k210_divs[k210_clks[id].div];

	if (div->type == K210_DIV_FIXED)
		return parent_rate / div->div;

	val = k210_clk_readl(priv, div->off, div->shift, div->width);
	switch (div->type) {
	case K210_DIV_ONE:
		return parent_rate / (val + 1);
	case K210_DIV_EVEN:
		return parent_rate / 2 / (val + 1);
	case K210_DIV_POWER:
		/* This is ACLK, which has no divider on IN0 */
		if (parent == K210_CLK_IN0)
			return parent_rate;
		return parent_rate / (2 << val);
	default:
		assert(false);
		return -EINVAL;
	};
}

static ulong k210_clk_get_rate(struct clk *clk)
{
	return do_k210_clk_get_rate(dev_get_priv(clk->dev), clk->id);
}

static ulong k210_clk_set_rate(struct clk *clk, unsigned long rate)
{
	return -ENOSYS;
}

static int do_k210_clk_set_parent(struct k210_clk_priv *priv, int id, int new)
{
	int i;
	const struct k210_mux_params *mux;

	if (!(k210_clks[id].flags & K210_CLKF_MUX))
		return -ENOSYS;
	mux = &k210_muxes[k210_clks[id].mux];

	for (i = 0; i < mux->num_parents; i++) {
		if (mux->parents[i] == new) {
			k210_clk_writel(priv, mux->off, mux->shift, mux->width,
					i);
			return 0;
		}
	}
	return -EINVAL;
}

static int k210_clk_set_parent(struct clk *clk, struct clk *parent)
{
	return do_k210_clk_set_parent(dev_get_priv(clk->dev), clk->id,
				      parent->id);
}

static int k210_clk_endisable(struct k210_clk_priv *priv, int id, bool enable)
{
	int parent = k210_clk_get_parent(priv, id);
	const struct k210_gate_params *gate;

	if (id == K210_CLK_IN0) {
		if (enable)
			return clk_enable(&priv->in0);
		else
			return clk_disable(&priv->in0);
	}

	/* Only recursively enable clocks since we don't track refcounts */
	if (enable) {
		int ret = k210_clk_endisable(priv, parent, true);

		if (ret && ret != -ENOSYS)
			return ret;
	}

	if (k210_clks[id].flags & K210_CLKF_PLL) {
		if (enable)
			return k210_pll_enable(priv, k210_clks[id].pll);
		else
			return k210_pll_disable(priv, k210_clks[id].pll);
	}

	if (k210_clks[id].gate == K210_CLK_GATE_NONE)
		return -ENOSYS;
	gate = &k210_gates[k210_clks[id].gate];

	k210_clk_writel(priv, gate->off, gate->bit_idx, 1, enable);
	return 0;
}

static int k210_clk_enable(struct clk *clk)
{
	return k210_clk_endisable(dev_get_priv(clk->dev), clk->id, true);
}

static int k210_clk_disable(struct clk *clk)
{
	return k210_clk_endisable(dev_get_priv(clk->dev), clk->id, false);
}

static int k210_clk_request(struct clk *clk)
{
	if (clk->id >= ARRAY_SIZE(k210_clks))
		return -EINVAL;
	return 0;
}

static const struct clk_ops k210_clk_ops = {
	.request = k210_clk_request,
	.set_rate = k210_clk_set_rate,
	.get_rate = k210_clk_get_rate,
	.set_parent = k210_clk_set_parent,
	.enable = k210_clk_enable,
	.disable = k210_clk_disable,
};

static int k210_clk_probe(struct udevice *dev)
{
	int ret;
	struct k210_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev_get_parent(dev));
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->in0);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id k210_clk_ids[] = {
	{ .compatible = "kendryte,k210-clk" },
	{ },
};

U_BOOT_DRIVER(k210_clk) = {
	.name = "k210_clk",
	.id = UCLASS_CLK,
	.of_match = k210_clk_ids,
	.ops = &k210_clk_ops,
	.probe = k210_clk_probe,
	.priv_auto = sizeof(struct k210_clk_priv),
};
