/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Tegra SoC common clock control functions */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/timer.h>
#include <div64.h>
#include <fdtdec.h>

/*
 * This is our record of the current clock rate of each clock. We don't
 * fill all of these in since we are only really interested in clocks which
 * we use as parents.
 */
static unsigned pll_rate[CLOCK_ID_COUNT];

/*
 * The oscillator frequency is fixed to one of four set values. Based on this
 * the other clocks are set up appropriately.
 */
static unsigned osc_freq[CLOCK_OSC_FREQ_COUNT] = {
	13000000,
	19200000,
	12000000,
	26000000,
};

/* return 1 if a peripheral ID is in range */
#define clock_type_id_isvalid(id) ((id) >= 0 && \
		(id) < CLOCK_TYPE_COUNT)

char pllp_valid = 1;	/* PLLP is set up correctly */

/* return 1 if a periphc_internal_id is in range */
#define periphc_internal_id_isvalid(id) ((id) >= 0 && \
		(id) < PERIPHC_COUNT)

/* number of clock outputs of a PLL */
static const u8 pll_num_clkouts[] = {
	1,	/* PLLC */
	1,	/* PLLM */
	4,	/* PLLP */
	1,	/* PLLA */
	0,	/* PLLU */
	0,	/* PLLD */
};

int clock_get_osc_bypass(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	reg = readl(&clkrst->crc_osc_ctrl);
	return (reg & OSC_XOBP_MASK) >> OSC_XOBP_SHIFT;
}

/* Returns a pointer to the registers of the given pll */
static struct clk_pll *get_pll(enum clock_id clkid)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;

	assert(clock_id_is_pll(clkid));
	return &clkrst->crc_pll[clkid];
}

int clock_ll_read_pll(enum clock_id clkid, u32 *divm, u32 *divn,
		u32 *divp, u32 *cpcon, u32 *lfcon)
{
	struct clk_pll *pll = get_pll(clkid);
	u32 data;

	assert(clkid != CLOCK_ID_USB);

	/* Safety check, adds to code size but is small */
	if (!clock_id_is_pll(clkid) || clkid == CLOCK_ID_USB)
		return -1;
	data = readl(&pll->pll_base);
	*divm = (data & PLL_DIVM_MASK) >> PLL_DIVM_SHIFT;
	*divn = (data & PLL_DIVN_MASK) >> PLL_DIVN_SHIFT;
	*divp = (data & PLL_DIVP_MASK) >> PLL_DIVP_SHIFT;
	data = readl(&pll->pll_misc);
	*cpcon = (data & PLL_CPCON_MASK) >> PLL_CPCON_SHIFT;
	*lfcon = (data & PLL_LFCON_MASK) >> PLL_LFCON_SHIFT;

	return 0;
}

unsigned long clock_start_pll(enum clock_id clkid, u32 divm, u32 divn,
		u32 divp, u32 cpcon, u32 lfcon)
{
	struct clk_pll *pll = get_pll(clkid);
	u32 data;

	/*
	 * We cheat by treating all PLL (except PLLU) in the same fashion.
	 * This works only because:
	 * - same fields are always mapped at same offsets, except DCCON
	 * - DCCON is always 0, doesn't conflict
	 * - M,N, P of PLLP values are ignored for PLLP
	 */
	data = (cpcon << PLL_CPCON_SHIFT) | (lfcon << PLL_LFCON_SHIFT);
	writel(data, &pll->pll_misc);

	data = (divm << PLL_DIVM_SHIFT) | (divn << PLL_DIVN_SHIFT) |
			(0 << PLL_BYPASS_SHIFT) | (1 << PLL_ENABLE_SHIFT);

	if (clkid == CLOCK_ID_USB)
		data |= divp << PLLU_VCO_FREQ_SHIFT;
	else
		data |= divp << PLL_DIVP_SHIFT;
	writel(data, &pll->pll_base);

	/* calculate the stable time */
	return timer_get_us() + CLOCK_PLL_STABLE_DELAY_US;
}

void clock_ll_set_source_divisor(enum periph_id periph_id, unsigned source,
			unsigned divisor)
{
	u32 *reg = get_periph_source_reg(periph_id);
	u32 value;

	value = readl(reg);

	value &= ~OUT_CLK_SOURCE_MASK;
	value |= source << OUT_CLK_SOURCE_SHIFT;

	value &= ~OUT_CLK_DIVISOR_MASK;
	value |= divisor << OUT_CLK_DIVISOR_SHIFT;

	writel(value, reg);
}

void clock_ll_set_source(enum periph_id periph_id, unsigned source)
{
	u32 *reg = get_periph_source_reg(periph_id);

	clrsetbits_le32(reg, OUT_CLK_SOURCE_MASK,
			source << OUT_CLK_SOURCE_SHIFT);
}

/**
 * Given the parent's rate and the required rate for the children, this works
 * out the peripheral clock divider to use, in 7.1 binary format.
 *
 * @param divider_bits	number of divider bits (8 or 16)
 * @param parent_rate	clock rate of parent clock in Hz
 * @param rate		required clock rate for this clock
 * @return divider which should be used
 */
static int clk_get_divider(unsigned divider_bits, unsigned long parent_rate,
			   unsigned long rate)
{
	u64 divider = parent_rate * 2;
	unsigned max_divider = 1 << divider_bits;

	divider += rate - 1;
	do_div(divider, rate);

	if ((s64)divider - 2 < 0)
		return 0;

	if ((s64)divider - 2 >= max_divider)
		return -1;

	return divider - 2;
}

int clock_set_pllout(enum clock_id clkid, enum pll_out_id pllout, unsigned rate)
{
	struct clk_pll *pll = get_pll(clkid);
	int data = 0, div = 0, offset = 0;

	if (!clock_id_is_pll(clkid))
		return -1;

	if (pllout + 1 > pll_num_clkouts[clkid])
		return -1;

	div = clk_get_divider(8, pll_rate[clkid], rate);

	if (div < 0)
		return -1;

	/* out2 and out4 are in the high part of the register */
	if (pllout == PLL_OUT2 || pllout == PLL_OUT4)
		offset = 16;

	data = (div << PLL_OUT_RATIO_SHIFT) |
			PLL_OUT_OVRRIDE | PLL_OUT_CLKEN | PLL_OUT_RSTN;
	clrsetbits_le32(&pll->pll_out[pllout >> 1],
			PLL_OUT_RATIO_MASK << offset, data << offset);

	return 0;
}

/**
 * Given the parent's rate and the divider in 7.1 format, this works out the
 * resulting peripheral clock rate.
 *
 * @param parent_rate	clock rate of parent clock in Hz
 * @param divider which should be used in 7.1 format
 * @return effective clock rate of peripheral
 */
static unsigned long get_rate_from_divider(unsigned long parent_rate,
					   int divider)
{
	u64 rate;

	rate = (u64)parent_rate * 2;
	do_div(rate, divider + 2);
	return rate;
}

unsigned long clock_get_periph_rate(enum periph_id periph_id,
		enum clock_id parent)
{
	u32 *reg = get_periph_source_reg(periph_id);

	return get_rate_from_divider(pll_rate[parent],
		(readl(reg) & OUT_CLK_DIVISOR_MASK) >> OUT_CLK_DIVISOR_SHIFT);
}

/**
 * Find the best available 7.1 format divisor given a parent clock rate and
 * required child clock rate. This function assumes that a second-stage
 * divisor is available which can divide by powers of 2 from 1 to 256.
 *
 * @param divider_bits	number of divider bits (8 or 16)
 * @param parent_rate	clock rate of parent clock in Hz
 * @param rate		required clock rate for this clock
 * @param extra_div	value for the second-stage divisor (not set if this
 *			function returns -1.
 * @return divider which should be used, or -1 if nothing is valid
 *
 */
static int find_best_divider(unsigned divider_bits, unsigned long parent_rate,
				unsigned long rate, int *extra_div)
{
	int shift;
	int best_divider = -1;
	int best_error = rate;

	/* try dividers from 1 to 256 and find closest match */
	for (shift = 0; shift <= 8 && best_error > 0; shift++) {
		unsigned divided_parent = parent_rate >> shift;
		int divider = clk_get_divider(divider_bits, divided_parent,
						rate);
		unsigned effective_rate = get_rate_from_divider(divided_parent,
						divider);
		int error = rate - effective_rate;

		/* Given a valid divider, look for the lowest error */
		if (divider != -1 && error < best_error) {
			best_error = error;
			*extra_div = 1 << shift;
			best_divider = divider;
		}
	}

	/* return what we found - *extra_div will already be set */
	return best_divider;
}

/**
 * Adjust peripheral PLL to use the given divider and source.
 *
 * @param periph_id	peripheral to adjust
 * @param source	Source number (0-3 or 0-7)
 * @param mux_bits	Number of mux bits (2 or 4)
 * @param divider	Required divider in 7.1 or 15.1 format
 * @return 0 if ok, -1 on error (requesting a parent clock which is not valid
 *		for this peripheral)
 */
static int adjust_periph_pll(enum periph_id periph_id, int source,
				int mux_bits, unsigned divider)
{
	u32 *reg = get_periph_source_reg(periph_id);

	clrsetbits_le32(reg, OUT_CLK_DIVISOR_MASK,
			divider << OUT_CLK_DIVISOR_SHIFT);
	udelay(1);

	/* work out the source clock and set it */
	if (source < 0)
		return -1;
	if (mux_bits == 4) {
		clrsetbits_le32(reg, OUT_CLK_SOURCE4_MASK,
			source << OUT_CLK_SOURCE4_SHIFT);
	} else {
		clrsetbits_le32(reg, OUT_CLK_SOURCE_MASK,
			source << OUT_CLK_SOURCE_SHIFT);
	}
	udelay(2);
	return 0;
}

unsigned clock_adjust_periph_pll_div(enum periph_id periph_id,
		enum clock_id parent, unsigned rate, int *extra_div)
{
	unsigned effective_rate;
	int mux_bits, divider_bits, source;
	int divider;
	int xdiv = 0;

	/* work out the source clock and set it */
	source = get_periph_clock_source(periph_id, parent, &mux_bits,
					 &divider_bits);

	divider = find_best_divider(divider_bits, pll_rate[parent],
				    rate, &xdiv);
	if (extra_div)
		*extra_div = xdiv;

	assert(divider >= 0);
	if (adjust_periph_pll(periph_id, source, mux_bits, divider))
		return -1U;
	debug("periph %d, rate=%d, reg=%p = %x\n", periph_id, rate,
		get_periph_source_reg(periph_id),
		readl(get_periph_source_reg(periph_id)));

	/* Check what we ended up with. This shouldn't matter though */
	effective_rate = clock_get_periph_rate(periph_id, parent);
	if (extra_div)
		effective_rate /= *extra_div;
	if (rate != effective_rate)
		debug("Requested clock rate %u not honored (got %u)\n",
			rate, effective_rate);
	return effective_rate;
}

unsigned clock_start_periph_pll(enum periph_id periph_id,
		enum clock_id parent, unsigned rate)
{
	unsigned effective_rate;

	reset_set_enable(periph_id, 1);
	clock_enable(periph_id);

	effective_rate = clock_adjust_periph_pll_div(periph_id, parent, rate,
						 NULL);

	reset_set_enable(periph_id, 0);
	return effective_rate;
}

void clock_enable(enum periph_id clkid)
{
	clock_set_enable(clkid, 1);
}

void clock_disable(enum periph_id clkid)
{
	clock_set_enable(clkid, 0);
}

void reset_periph(enum periph_id periph_id, int us_delay)
{
	/* Put peripheral into reset */
	reset_set_enable(periph_id, 1);
	udelay(us_delay);

	/* Remove reset */
	reset_set_enable(periph_id, 0);

	udelay(us_delay);
}

void reset_cmplx_set_enable(int cpu, int which, int reset)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 mask;

	/* Form the mask, which depends on the cpu chosen (2 or 4) */
	assert(cpu >= 0 && cpu < MAX_NUM_CPU);
	mask = which << cpu;

	/* either enable or disable those reset for that CPU */
	if (reset)
		writel(mask, &clkrst->crc_cpu_cmplx_set);
	else
		writel(mask, &clkrst->crc_cpu_cmplx_clr);
}

unsigned clock_get_rate(enum clock_id clkid)
{
	struct clk_pll *pll;
	u32 base;
	u32 divm;
	u64 parent_rate;
	u64 rate;

	parent_rate = osc_freq[clock_get_osc_freq()];
	if (clkid == CLOCK_ID_OSC)
		return parent_rate;

	pll = get_pll(clkid);
	base = readl(&pll->pll_base);

	/* Oh for bf_unpack()... */
	rate = parent_rate * ((base & PLL_DIVN_MASK) >> PLL_DIVN_SHIFT);
	divm = (base & PLL_DIVM_MASK) >> PLL_DIVM_SHIFT;
	if (clkid == CLOCK_ID_USB)
		divm <<= (base & PLLU_VCO_FREQ_MASK) >> PLLU_VCO_FREQ_SHIFT;
	else
		divm <<= (base & PLL_DIVP_MASK) >> PLL_DIVP_SHIFT;
	do_div(rate, divm);
	return rate;
}

/**
 * Set the output frequency you want for each PLL clock.
 * PLL output frequencies are programmed by setting their N, M and P values.
 * The governing equations are:
 *     VCO = (Fi / m) * n, Fo = VCO / (2^p)
 *     where Fo is the output frequency from the PLL.
 * Example: Set the output frequency to 216Mhz(Fo) with 12Mhz OSC(Fi)
 *     216Mhz = ((12Mhz / m) * n) / (2^p) so n=432,m=12,p=1
 * Please see Tegra TRM section 5.3 to get the detail for PLL Programming
 *
 * @param n PLL feedback divider(DIVN)
 * @param m PLL input divider(DIVN)
 * @param p post divider(DIVP)
 * @param cpcon base PLL charge pump(CPCON)
 * @return 0 if ok, -1 on error (the requested PLL is incorrect and cannot
 *		be overriden), 1 if PLL is already correct
 */
int clock_set_rate(enum clock_id clkid, u32 n, u32 m, u32 p, u32 cpcon)
{
	u32 base_reg;
	u32 misc_reg;
	struct clk_pll *pll;

	pll = get_pll(clkid);

	base_reg = readl(&pll->pll_base);

	/* Set BYPASS, m, n and p to PLL_BASE */
	base_reg &= ~PLL_DIVM_MASK;
	base_reg |= m << PLL_DIVM_SHIFT;

	base_reg &= ~PLL_DIVN_MASK;
	base_reg |= n << PLL_DIVN_SHIFT;

	base_reg &= ~PLL_DIVP_MASK;
	base_reg |= p << PLL_DIVP_SHIFT;

	if (clkid == CLOCK_ID_PERIPH) {
		/*
		 * If the PLL is already set up, check that it is correct
		 * and record this info for clock_verify() to check.
		 */
		if (base_reg & PLL_BASE_OVRRIDE_MASK) {
			base_reg |= PLL_ENABLE_MASK;
			if (base_reg != readl(&pll->pll_base))
				pllp_valid = 0;
			return pllp_valid ? 1 : -1;
		}
		base_reg |= PLL_BASE_OVRRIDE_MASK;
	}

	base_reg |= PLL_BYPASS_MASK;
	writel(base_reg, &pll->pll_base);

	/* Set cpcon to PLL_MISC */
	misc_reg = readl(&pll->pll_misc);
	misc_reg &= ~PLL_CPCON_MASK;
	misc_reg |= cpcon << PLL_CPCON_SHIFT;
	writel(misc_reg, &pll->pll_misc);

	/* Enable PLL */
	base_reg |= PLL_ENABLE_MASK;
	writel(base_reg, &pll->pll_base);

	/* Disable BYPASS */
	base_reg &= ~PLL_BYPASS_MASK;
	writel(base_reg, &pll->pll_base);

	return 0;
}

void clock_ll_start_uart(enum periph_id periph_id)
{
	/* Assert UART reset and enable clock */
	reset_set_enable(periph_id, 1);
	clock_enable(periph_id);
	clock_ll_set_source(periph_id, 0); /* UARTx_CLK_SRC = 00, PLLP_OUT0 */

	/* wait for 2us */
	udelay(2);

	/* De-assert reset to UART */
	reset_set_enable(periph_id, 0);
}

#ifdef CONFIG_OF_CONTROL
int clock_decode_periph_id(const void *blob, int node)
{
	enum periph_id id;
	u32 cell[2];
	int err;

	err = fdtdec_get_int_array(blob, node, "clocks", cell,
				   ARRAY_SIZE(cell));
	if (err)
		return -1;
	id = clk_id_to_periph_id(cell[1]);
	assert(clock_periph_id_isvalid(id));
	return id;
}
#endif /* CONFIG_OF_CONTROL */

int clock_verify(void)
{
	struct clk_pll *pll = get_pll(CLOCK_ID_PERIPH);
	u32 reg = readl(&pll->pll_base);

	if (!pllp_valid) {
		printf("Warning: PLLP %x is not correct\n", reg);
		return -1;
	}
	debug("PLLP %x is correct\n", reg);
	return 0;
}

void clock_init(void)
{
	pll_rate[CLOCK_ID_MEMORY] = clock_get_rate(CLOCK_ID_MEMORY);
	pll_rate[CLOCK_ID_PERIPH] = clock_get_rate(CLOCK_ID_PERIPH);
	pll_rate[CLOCK_ID_CGENERAL] = clock_get_rate(CLOCK_ID_CGENERAL);
	pll_rate[CLOCK_ID_OSC] = clock_get_rate(CLOCK_ID_OSC);
	pll_rate[CLOCK_ID_SFROM32KHZ] = 32768;
	pll_rate[CLOCK_ID_XCPU] = clock_get_rate(CLOCK_ID_XCPU);
	debug("Osc = %d\n", pll_rate[CLOCK_ID_OSC]);
	debug("PLLM = %d\n", pll_rate[CLOCK_ID_MEMORY]);
	debug("PLLP = %d\n", pll_rate[CLOCK_ID_PERIPH]);
	debug("PLLC = %d\n", pll_rate[CLOCK_ID_CGENERAL]);
	debug("PLLX = %d\n", pll_rate[CLOCK_ID_XCPU]);

	/* Do any special system timer/TSC setup */
	arch_timer_init();
}
