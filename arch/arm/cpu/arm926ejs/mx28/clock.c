/*
 * Freescale i.MX28 clock setup code
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>

/* The PLL frequency is always 480MHz, see section 10.2 in iMX28 datasheet. */
#define	PLL_FREQ_KHZ	480000
#define	PLL_FREQ_COEF	18
/* The XTAL frequency is always 24MHz, see section 10.2 in iMX28 datasheet. */
#define	XTAL_FREQ_KHZ	24000

#define	PLL_FREQ_MHZ	(PLL_FREQ_KHZ / 1000)
#define	XTAL_FREQ_MHZ	(XTAL_FREQ_KHZ / 1000)

static uint32_t mx28_get_pclk(void)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;

	uint32_t clkctrl, clkseq, div;
	uint8_t clkfrac, frac;

	clkctrl = readl(&clkctrl_regs->hw_clkctrl_cpu);

	/* No support of fractional divider calculation */
	if (clkctrl &
		(CLKCTRL_CPU_DIV_XTAL_FRAC_EN | CLKCTRL_CPU_DIV_CPU_FRAC_EN)) {
		return 0;
	}

	clkseq = readl(&clkctrl_regs->hw_clkctrl_clkseq);

	/* XTAL Path */
	if (clkseq & CLKCTRL_CLKSEQ_BYPASS_CPU) {
		div = (clkctrl & CLKCTRL_CPU_DIV_XTAL_MASK) >>
			CLKCTRL_CPU_DIV_XTAL_OFFSET;
		return XTAL_FREQ_MHZ / div;
	}

	/* REF Path */
	clkfrac = readb(&clkctrl_regs->hw_clkctrl_frac0[CLKCTRL_FRAC0_CPU]);
	frac = clkfrac & CLKCTRL_FRAC_FRAC_MASK;
	div = clkctrl & CLKCTRL_CPU_DIV_CPU_MASK;
	return (PLL_FREQ_MHZ * PLL_FREQ_COEF / frac) / div;
}

static uint32_t mx28_get_hclk(void)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;

	uint32_t div;
	uint32_t clkctrl;

	clkctrl = readl(&clkctrl_regs->hw_clkctrl_hbus);

	/* No support of fractional divider calculation */
	if (clkctrl & CLKCTRL_HBUS_DIV_FRAC_EN)
		return 0;

	div = clkctrl & CLKCTRL_HBUS_DIV_MASK;
	return mx28_get_pclk() / div;
}

static uint32_t mx28_get_emiclk(void)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;

	uint32_t clkctrl, clkseq, div;
	uint8_t clkfrac, frac;

	clkseq = readl(&clkctrl_regs->hw_clkctrl_clkseq);
	clkctrl = readl(&clkctrl_regs->hw_clkctrl_emi);

	/* XTAL Path */
	if (clkseq & CLKCTRL_CLKSEQ_BYPASS_EMI) {
		div = (clkctrl & CLKCTRL_EMI_DIV_XTAL_MASK) >>
			CLKCTRL_EMI_DIV_XTAL_OFFSET;
		return XTAL_FREQ_MHZ / div;
	}

	/* REF Path */
	clkfrac = readb(&clkctrl_regs->hw_clkctrl_frac0[CLKCTRL_FRAC0_EMI]);
	frac = clkfrac & CLKCTRL_FRAC_FRAC_MASK;
	div = clkctrl & CLKCTRL_EMI_DIV_EMI_MASK;
	return (PLL_FREQ_MHZ * PLL_FREQ_COEF / frac) / div;
}

static uint32_t mx28_get_gpmiclk(void)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;

	uint32_t clkctrl, clkseq, div;
	uint8_t clkfrac, frac;

	clkseq = readl(&clkctrl_regs->hw_clkctrl_clkseq);
	clkctrl = readl(&clkctrl_regs->hw_clkctrl_gpmi);

	/* XTAL Path */
	if (clkseq & CLKCTRL_CLKSEQ_BYPASS_GPMI) {
		div = clkctrl & CLKCTRL_GPMI_DIV_MASK;
		return XTAL_FREQ_MHZ / div;
	}

	/* REF Path */
	clkfrac = readb(&clkctrl_regs->hw_clkctrl_frac1[CLKCTRL_FRAC1_GPMI]);
	frac = clkfrac & CLKCTRL_FRAC_FRAC_MASK;
	div = clkctrl & CLKCTRL_GPMI_DIV_MASK;
	return (PLL_FREQ_MHZ * PLL_FREQ_COEF / frac) / div;
}

/*
 * Set IO clock frequency, in kHz
 */
void mx28_set_ioclk(enum mxs_ioclock io, uint32_t freq)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint32_t div;
	int io_reg;

	if (freq == 0)
		return;

	if ((io < MXC_IOCLK0) || (io > MXC_IOCLK1))
		return;

	div = (PLL_FREQ_KHZ * PLL_FREQ_COEF) / freq;

	if (div < 18)
		div = 18;

	if (div > 35)
		div = 35;

	io_reg = CLKCTRL_FRAC0_IO0 - io;	/* Register order is reversed */
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac0_set[io_reg]);
	writeb(CLKCTRL_FRAC_CLKGATE | (div & CLKCTRL_FRAC_FRAC_MASK),
		&clkctrl_regs->hw_clkctrl_frac0[io_reg]);
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac0_clr[io_reg]);
}

/*
 * Get IO clock, returns IO clock in kHz
 */
static uint32_t mx28_get_ioclk(enum mxs_ioclock io)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint8_t ret;
	int io_reg;

	if ((io < MXC_IOCLK0) || (io > MXC_IOCLK1))
		return 0;

	io_reg = CLKCTRL_FRAC0_IO0 - io;	/* Register order is reversed */

	ret = readb(&clkctrl_regs->hw_clkctrl_frac0[io_reg]) &
		CLKCTRL_FRAC_FRAC_MASK;

	return (PLL_FREQ_KHZ * PLL_FREQ_COEF) / ret;
}

/*
 * Configure SSP clock frequency, in kHz
 */
void mx28_set_sspclk(enum mxs_sspclock ssp, uint32_t freq, int xtal)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint32_t clk, clkreg;

	if (ssp > MXC_SSPCLK3)
		return;

	clkreg = (uint32_t)(&clkctrl_regs->hw_clkctrl_ssp0) +
			(ssp * sizeof(struct mx28_register_32));

	clrbits_le32(clkreg, CLKCTRL_SSP_CLKGATE);
	while (readl(clkreg) & CLKCTRL_SSP_CLKGATE)
		;

	if (xtal)
		clk = XTAL_FREQ_KHZ;
	else
		clk = mx28_get_ioclk(ssp >> 1);

	if (freq > clk)
		return;

	/* Calculate the divider and cap it if necessary */
	clk /= freq;
	if (clk > CLKCTRL_SSP_DIV_MASK)
		clk = CLKCTRL_SSP_DIV_MASK;

	clrsetbits_le32(clkreg, CLKCTRL_SSP_DIV_MASK, clk);
	while (readl(clkreg) & CLKCTRL_SSP_BUSY)
		;

	if (xtal)
		writel(CLKCTRL_CLKSEQ_BYPASS_SSP0 << ssp,
			&clkctrl_regs->hw_clkctrl_clkseq_set);
	else
		writel(CLKCTRL_CLKSEQ_BYPASS_SSP0 << ssp,
			&clkctrl_regs->hw_clkctrl_clkseq_clr);
}

/*
 * Return SSP frequency, in kHz
 */
static uint32_t mx28_get_sspclk(enum mxs_sspclock ssp)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint32_t clkreg;
	uint32_t clk, tmp;

	if (ssp > MXC_SSPCLK3)
		return 0;

	tmp = readl(&clkctrl_regs->hw_clkctrl_clkseq);
	if (tmp & (CLKCTRL_CLKSEQ_BYPASS_SSP0 << ssp))
		return XTAL_FREQ_KHZ;

	clkreg = (uint32_t)(&clkctrl_regs->hw_clkctrl_ssp0) +
			(ssp * sizeof(struct mx28_register_32));

	tmp = readl(clkreg) & CLKCTRL_SSP_DIV_MASK;

	if (tmp == 0)
		return 0;

	clk = mx28_get_ioclk(ssp >> 1);

	return clk / tmp;
}

/*
 * Set SSP/MMC bus frequency, in kHz)
 */
void mx28_set_ssp_busclock(unsigned int bus, uint32_t freq)
{
	struct mx28_ssp_regs *ssp_regs;
	const uint32_t sspclk = mx28_get_sspclk(bus);
	uint32_t reg;
	uint32_t divide, rate, tgtclk;

	ssp_regs = (struct mx28_ssp_regs *)(MXS_SSP0_BASE + (bus * 0x2000));

	/*
	 * SSP bit rate = SSPCLK / (CLOCK_DIVIDE * (1 + CLOCK_RATE)),
	 * CLOCK_DIVIDE has to be an even value from 2 to 254, and
	 * CLOCK_RATE could be any integer from 0 to 255.
	 */
	for (divide = 2; divide < 254; divide += 2) {
		rate = sspclk / freq / divide;
		if (rate <= 256)
			break;
	}

	tgtclk = sspclk / divide / rate;
	while (tgtclk > freq) {
		rate++;
		tgtclk = sspclk / divide / rate;
	}
	if (rate > 256)
		rate = 256;

	/* Always set timeout the maximum */
	reg = SSP_TIMING_TIMEOUT_MASK |
		(divide << SSP_TIMING_CLOCK_DIVIDE_OFFSET) |
		((rate - 1) << SSP_TIMING_CLOCK_RATE_OFFSET);
	writel(reg, &ssp_regs->hw_ssp_timing);

	debug("SPI%d: Set freq rate to %d KHz (requested %d KHz)\n",
		bus, tgtclk, freq);
}

uint32_t mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return mx28_get_pclk() * 1000000;
	case MXC_GPMI_CLK:
		return mx28_get_gpmiclk() * 1000000;
	case MXC_AHB_CLK:
	case MXC_IPG_CLK:
		return mx28_get_hclk() * 1000000;
	case MXC_EMI_CLK:
		return mx28_get_emiclk();
	case MXC_IO0_CLK:
		return mx28_get_ioclk(MXC_IOCLK0);
	case MXC_IO1_CLK:
		return mx28_get_ioclk(MXC_IOCLK1);
	case MXC_SSP0_CLK:
		return mx28_get_sspclk(MXC_SSPCLK0);
	case MXC_SSP1_CLK:
		return mx28_get_sspclk(MXC_SSPCLK1);
	case MXC_SSP2_CLK:
		return mx28_get_sspclk(MXC_SSPCLK2);
	case MXC_SSP3_CLK:
		return mx28_get_sspclk(MXC_SSPCLK3);
	}

	return 0;
}
