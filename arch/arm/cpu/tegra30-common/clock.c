/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
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

/* Tegra30 Clock control functions */

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

/*
 * Clock types that we can use as a source. The Tegra3 has muxes for the
 * peripheral clocks, and in most cases there are four options for the clock
 * source. This gives us a clock 'type' and exploits what commonality exists
 * in the device.
 *
 * Letters are obvious, except for T which means CLK_M, and S which means the
 * clock derived from 32KHz. Beware that CLK_M (also called OSC in the
 * datasheet) and PLL_M are different things. The former is the basic
 * clock supplied to the SOC from an external oscillator. The latter is the
 * memory clock PLL.
 *
 * See definitions in clock_id in the header file.
 */
enum clock_type_id {
	CLOCK_TYPE_AXPT,	/* PLL_A, PLL_X, PLL_P, CLK_M */
	CLOCK_TYPE_MCPA,	/* and so on */
	CLOCK_TYPE_MCPT,
	CLOCK_TYPE_PCM,
	CLOCK_TYPE_PCMT,
	CLOCK_TYPE_PCMT16,
	CLOCK_TYPE_PDCT,
	CLOCK_TYPE_ACPT,
	CLOCK_TYPE_ASPTE,
	CLOCK_TYPE_PMDACD2T,
	CLOCK_TYPE_PCST,

	CLOCK_TYPE_COUNT,
	CLOCK_TYPE_NONE = -1,	/* invalid clock type */
};

/* return 1 if a peripheral ID is in range */
#define clock_type_id_isvalid(id) ((id) >= 0 && \
		(id) < CLOCK_TYPE_COUNT)

char pllp_valid = 1;	/* PLLP is set up correctly */

enum {
	CLOCK_MAX_MUX	= 8	/* number of source options for each clock */
};

enum {
	MASK_BITS_31_30	= 2,	/* num of bits used to specify clock source */
	MASK_BITS_31_29,
	MASK_BITS_29_28,
};

/*
 * Clock source mux for each clock type. This just converts our enum into
 * a list of mux sources for use by the code.
 *
 * Note:
 *  The extra column in each clock source array is used to store the mask
 *  bits in its register for the source.
 */
#define CLK(x) CLOCK_ID_ ## x
static enum clock_id clock_source[CLOCK_TYPE_COUNT][CLOCK_MAX_MUX+1] = {
	{ CLK(AUDIO),	CLK(XCPU),	CLK(PERIPH),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(MEMORY),	CLK(CGENERAL),	CLK(PERIPH),	CLK(AUDIO),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(MEMORY),	CLK(CGENERAL),	CLK(PERIPH),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(MEMORY),	CLK(NONE),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(MEMORY),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(MEMORY),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),	CLK(DISPLAY),	CLK(CGENERAL),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(AUDIO),	CLK(CGENERAL),	CLK(PERIPH),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(AUDIO),	CLK(SFROM32KHZ),	CLK(PERIPH),	CLK(OSC),
		CLK(EPCI),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_31_29},
	{ CLK(PERIPH),	CLK(MEMORY),	CLK(DISPLAY),	CLK(AUDIO),
		CLK(CGENERAL),	CLK(DISPLAY2),	CLK(OSC),	CLK(NONE),
		MASK_BITS_31_29},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(SFROM32KHZ),	CLK(OSC),
		CLK(NONE),	CLK(NONE),	CLK(NONE),	CLK(NONE),
		MASK_BITS_29_28}
};

/* return 1 if a periphc_internal_id is in range */
#define periphc_internal_id_isvalid(id) ((id) >= 0 && \
		(id) < PERIPHC_COUNT)

/*
 * Clock type for each peripheral clock source. We put the name in each
 * record just so it is easy to match things up
 */
#define TYPE(name, type) type
static enum clock_type_id clock_periph_type[PERIPHC_COUNT] = {
	/* 0x00 */
	TYPE(PERIPHC_I2S1,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2S2,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_SPDIF_OUT,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_SPDIF_IN,	CLOCK_TYPE_PCM),
	TYPE(PERIPHC_PWM,	CLOCK_TYPE_PCST),  /* only PWM uses b29:28 */
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC2,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SBC3,	CLOCK_TYPE_PCMT),

	/* 0x08 */
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_I2C1,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_DVC_I2C,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC1,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_DISP1,	CLOCK_TYPE_PMDACD2T),
	TYPE(PERIPHC_DISP2,	CLOCK_TYPE_PMDACD2T),

	/* 0x10 */
	TYPE(PERIPHC_CVE,	CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VI,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SDMMC1,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SDMMC2,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_G3D,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_G2D,	CLOCK_TYPE_MCPA),

	/* 0x18 */
	TYPE(PERIPHC_NDFLASH,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SDMMC4,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_VFIR,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_EPP,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_MPE,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_MIPI,	CLOCK_TYPE_PCMT),	/* MIPI base-band HSI */
	TYPE(PERIPHC_UART1,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_UART2,	CLOCK_TYPE_PCMT),

	/* 0x20 */
	TYPE(PERIPHC_HOST1X,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_TVO,	CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_HDMI,	CLOCK_TYPE_PMDACD2T),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_TVDAC,	CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_I2C2,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_EMC,	CLOCK_TYPE_MCPT),

	/* 0x28 */
	TYPE(PERIPHC_UART3,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VI,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC4,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_I2C3,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_SDMMC3,	CLOCK_TYPE_PCMT),

	/* 0x30 */
	TYPE(PERIPHC_UART4,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_UART5,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_VDE,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_OWR,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NOR,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_CSITE,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_I2S0,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),

	/* 0x38h */		/* Jumps to reg offset 0x3B0h - new for T30 */
	TYPE(PERIPHC_G3D2,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_MSELECT,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_TSENSOR,	CLOCK_TYPE_PCST),	/* s/b PCTS */
	TYPE(PERIPHC_I2S3,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2S4,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2C4,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_SBC5,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SBC6,	CLOCK_TYPE_PCMT),

	/* 0x40 */
	TYPE(PERIPHC_AUDIO,	CLOCK_TYPE_ACPT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_DAM0,	CLOCK_TYPE_ACPT),
	TYPE(PERIPHC_DAM1,	CLOCK_TYPE_ACPT),
	TYPE(PERIPHC_DAM2,	CLOCK_TYPE_ACPT),
	TYPE(PERIPHC_HDA2CODEC2X, CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_ACTMON,	CLOCK_TYPE_PCST),	/* MASK 31:30 */
	TYPE(PERIPHC_EXTPERIPH1, CLOCK_TYPE_ASPTE),

	/* 0x48 */
	TYPE(PERIPHC_EXTPERIPH2, CLOCK_TYPE_ASPTE),
	TYPE(PERIPHC_EXTPERIPH3, CLOCK_TYPE_ASPTE),
	TYPE(PERIPHC_NANDSPEED,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_I2CSLOW,	CLOCK_TYPE_PCST),	/* MASK 31:30 */
	TYPE(PERIPHC_SYS,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SPEEDO,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),

	/* 0x50 */
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SATAOOB,	CLOCK_TYPE_PCMT),	/* offset 0x420h */
	TYPE(PERIPHC_SATA,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_HDA,	CLOCK_TYPE_PCMT),
};

/*
 * This array translates a periph_id to a periphc_internal_id
 *
 * Not present/matched up:
 *	uint vi_sensor;	 _VI_SENSOR_0,		0x1A8
 *	SPDIF - which is both 0x08 and 0x0c
 *
 */
#define NONE(name) (-1)
#define OFFSET(name, value) PERIPHC_ ## name
static s8 periph_id_to_internal_id[PERIPH_ID_COUNT] = {
	/* Low word: 31:0 */
	NONE(CPU),
	NONE(COP),
	NONE(TRIGSYS),
	NONE(RESERVED3),
	NONE(RESERVED4),
	NONE(TMR),
	PERIPHC_UART1,
	PERIPHC_UART2,	/* and vfir 0x68 */

	/* 8 */
	NONE(GPIO),
	PERIPHC_SDMMC2,
	NONE(SPDIF),		/* 0x08 and 0x0c, unclear which to use */
	PERIPHC_I2S1,
	PERIPHC_I2C1,
	PERIPHC_NDFLASH,
	PERIPHC_SDMMC1,
	PERIPHC_SDMMC4,

	/* 16 */
	NONE(RESERVED16),
	PERIPHC_PWM,
	PERIPHC_I2S2,
	PERIPHC_EPP,
	PERIPHC_VI,
	PERIPHC_G2D,
	NONE(USBD),
	NONE(ISP),

	/* 24 */
	PERIPHC_G3D,
	NONE(RESERVED25),
	PERIPHC_DISP2,
	PERIPHC_DISP1,
	PERIPHC_HOST1X,
	NONE(VCP),
	PERIPHC_I2S0,
	NONE(CACHE2),

	/* Middle word: 63:32 */
	NONE(MEM),
	NONE(AHBDMA),
	NONE(APBDMA),
	NONE(RESERVED35),
	NONE(RESERVED36),
	NONE(STAT_MON),
	NONE(RESERVED38),
	NONE(RESERVED39),

	/* 40 */
	NONE(KFUSE),
	NONE(SBC1),	/* SBC1, 0x34, is this SPI1? */
	PERIPHC_NOR,
	NONE(RESERVED43),
	PERIPHC_SBC2,
	NONE(RESERVED45),
	PERIPHC_SBC3,
	PERIPHC_DVC_I2C,

	/* 48 */
	NONE(DSI),
	PERIPHC_TVO,	/* also CVE 0x40 */
	PERIPHC_MIPI,
	PERIPHC_HDMI,
	NONE(CSI),
	PERIPHC_TVDAC,
	PERIPHC_I2C2,
	PERIPHC_UART3,

	/* 56 */
	NONE(RESERVED56),
	PERIPHC_EMC,
	NONE(USB2),
	NONE(USB3),
	PERIPHC_MPE,
	PERIPHC_VDE,
	NONE(BSEA),
	NONE(BSEV),

	/* Upper word 95:64 */
	PERIPHC_SPEEDO,
	PERIPHC_UART4,
	PERIPHC_UART5,
	PERIPHC_I2C3,
	PERIPHC_SBC4,
	PERIPHC_SDMMC3,
	NONE(PCIE),
	PERIPHC_OWR,

	/* 72 */
	NONE(AFI),
	PERIPHC_CSITE,
	NONE(PCIEXCLK),
	NONE(AVPUCQ),
	NONE(RESERVED76),
	NONE(RESERVED77),
	NONE(RESERVED78),
	NONE(DTV),

	/* 80 */
	PERIPHC_NANDSPEED,
	PERIPHC_I2CSLOW,
	NONE(DSIB),
	NONE(RESERVED83),
	NONE(IRAMA),
	NONE(IRAMB),
	NONE(IRAMC),
	NONE(IRAMD),

	/* 88 */
	NONE(CRAM2),
	NONE(RESERVED89),
	NONE(MDOUBLER),
	NONE(RESERVED91),
	NONE(SUSOUT),
	NONE(RESERVED93),
	NONE(RESERVED94),
	NONE(RESERVED95),

	/* V word: 31:0 */
	NONE(CPUG),
	NONE(CPULP),
	PERIPHC_G3D2,
	PERIPHC_MSELECT,
	PERIPHC_TSENSOR,
	PERIPHC_I2S3,
	PERIPHC_I2S4,
	PERIPHC_I2C4,

	/* 08 */
	PERIPHC_SBC5,
	PERIPHC_SBC6,
	PERIPHC_AUDIO,
	NONE(APBIF),
	PERIPHC_DAM0,
	PERIPHC_DAM1,
	PERIPHC_DAM2,
	PERIPHC_HDA2CODEC2X,

	/* 16 */
	NONE(ATOMICS),
	NONE(RESERVED17),
	NONE(RESERVED18),
	NONE(RESERVED19),
	NONE(RESERVED20),
	NONE(RESERVED21),
	NONE(RESERVED22),
	PERIPHC_ACTMON,

	/* 24 */
	NONE(RESERVED24),
	NONE(RESERVED25),
	NONE(RESERVED26),
	NONE(RESERVED27),
	PERIPHC_SATA,
	PERIPHC_HDA,
	NONE(RESERVED30),
	NONE(RESERVED31),

	/* W word: 31:0 */
	NONE(HDA2HDMICODEC),
	NONE(SATACOLD),
	NONE(RESERVED0_PCIERX0),
	NONE(RESERVED1_PCIERX1),
	NONE(RESERVED2_PCIERX2),
	NONE(RESERVED3_PCIERX3),
	NONE(RESERVED4_PCIERX4),
	NONE(RESERVED5_PCIERX5),

	/* 40 */
	NONE(CEC),
	NONE(RESERVED6_PCIE2),
	NONE(RESERVED7_EMC),
	NONE(RESERVED8_HDMI),
	NONE(RESERVED9_SATA),
	NONE(RESERVED10_MIPI),
	NONE(EX_RESERVED46),
	NONE(EX_RESERVED47),
};

/*
 * Get the oscillator frequency, from the corresponding hardware configuration
 * field.
 */
enum clock_osc_freq clock_get_osc_freq(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	reg = readl(&clkrst->crc_osc_ctrl);
	return (reg & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;
}

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

/* Returns a pointer to the clock source register for a peripheral */
static u32 *get_periph_source_reg(enum periph_id periph_id)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	enum periphc_internal_id internal_id;

	/* Coresight is a special case */
	if (periph_id == PERIPH_ID_CSI)
		return &clkrst->crc_clk_src[PERIPH_ID_CSI+1];

	assert(periph_id >= PERIPH_ID_FIRST && periph_id < PERIPH_ID_COUNT);
	internal_id = periph_id_to_internal_id[periph_id];
	assert(internal_id != -1);
	if (internal_id >= PERIPHC_VW_FIRST) {
		internal_id -= PERIPHC_VW_FIRST;
		return &clkrst->crc_clk_src_vw[internal_id];
	} else
		return &clkrst->crc_clk_src[internal_id];
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
 * Given a peripheral ID and the required source clock, this returns which
 * value should be programmed into the source mux for that peripheral.
 *
 * There is special code here to handle the one source type with 5 sources.
 *
 * @param periph_id	peripheral to start
 * @param source	PLL id of required parent clock
 * @param mux_bits	Set to number of bits in mux register: 2 or 4
 * @param divider_bits	Set to number of divider bits (8 or 16)
 * @return mux value (0-4, or -1 if not found)
 */
static int get_periph_clock_source(enum periph_id periph_id,
		enum clock_id parent, int *mux_bits, int *divider_bits)
{
	enum clock_type_id type;
	enum periphc_internal_id internal_id;
	int mux;

	assert(clock_periph_id_isvalid(periph_id));

	internal_id = periph_id_to_internal_id[periph_id];
	assert(periphc_internal_id_isvalid(internal_id));

	type = clock_periph_type[internal_id];
	assert(clock_type_id_isvalid(type));

	*mux_bits = clock_source[type][CLOCK_MAX_MUX];

	if (type == CLOCK_TYPE_PCMT16)
		*divider_bits = 16;
	else
		*divider_bits = 8;

	for (mux = 0; mux < CLOCK_MAX_MUX; mux++)
		if (clock_source[type][mux] == parent)
			return mux;

	/* if we get here, either us or the caller has made a mistake */
	printf("Caller requested bad clock: periph=%d, parent=%d\n", periph_id,
		parent);
	return -1;
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
	int mux_bits, source;
	int divider, divider_bits = 0;

	/* work out the source clock and set it */
	source = get_periph_clock_source(periph_id, parent, &mux_bits,
					 &divider_bits);

	if (extra_div)
		divider = find_best_divider(divider_bits, pll_rate[parent],
					    rate, extra_div);
	else
		divider = clk_get_divider(divider_bits, pll_rate[parent],
					  rate);
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

void clock_set_enable(enum periph_id periph_id, int enable)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 *clk;
	u32 reg;

	/* Enable/disable the clock to this peripheral */
	assert(clock_periph_id_isvalid(periph_id));
	if ((int)periph_id < (int)PERIPH_ID_VW_FIRST)
		clk = &clkrst->crc_clk_out_enb[PERIPH_REG(periph_id)];
	else
		clk = &clkrst->crc_clk_out_enb_vw[PERIPH_REG(periph_id)];
	reg = readl(clk);
	if (enable)
		reg |= PERIPH_MASK(periph_id);
	else
		reg &= ~PERIPH_MASK(periph_id);
	writel(reg, clk);
}

void clock_enable(enum periph_id clkid)
{
	clock_set_enable(clkid, 1);
}

void clock_disable(enum periph_id clkid)
{
	clock_set_enable(clkid, 0);
}

void reset_set_enable(enum periph_id periph_id, int enable)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 *reset;
	u32 reg;

	/* Enable/disable reset to the peripheral */
	assert(clock_periph_id_isvalid(periph_id));
	if (periph_id < PERIPH_ID_VW_FIRST)
		reset = &clkrst->crc_rst_dev[PERIPH_REG(periph_id)];
	else
		reset = &clkrst->crc_rst_dev_vw[PERIPH_REG(periph_id)];
	reg = readl(reset);
	if (enable)
		reg |= PERIPH_MASK(periph_id);
	else
		reg &= ~PERIPH_MASK(periph_id);
	writel(reg, reset);
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

	/* Form the mask, which depends on the cpu chosen. Tegra3 has 4 */
	assert(cpu >= 0 && cpu < 4);
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
static int clock_set_rate(enum clock_id clkid, u32 n, u32 m, u32 p, u32 cpcon)
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
/*
 * Convert a device tree clock ID to our peripheral ID. They are mostly
 * the same but we are very cautious so we check that a valid clock ID is
 * provided.
 *
 * @param clk_id	Clock ID according to tegra30 device tree binding
 * @return peripheral ID, or PERIPH_ID_NONE if the clock ID is invalid
 */
static enum periph_id clk_id_to_periph_id(int clk_id)
{
	if (clk_id > PERIPH_ID_COUNT)
		return PERIPH_ID_NONE;

	switch (clk_id) {
	case PERIPH_ID_RESERVED3:
	case PERIPH_ID_RESERVED4:
	case PERIPH_ID_RESERVED16:
	case PERIPH_ID_RESERVED24:
	case PERIPH_ID_RESERVED35:
	case PERIPH_ID_RESERVED43:
	case PERIPH_ID_RESERVED45:
	case PERIPH_ID_RESERVED56:
	case PERIPH_ID_RESERVED76:
	case PERIPH_ID_RESERVED77:
	case PERIPH_ID_RESERVED78:
	case PERIPH_ID_RESERVED83:
	case PERIPH_ID_RESERVED89:
	case PERIPH_ID_RESERVED91:
	case PERIPH_ID_RESERVED93:
	case PERIPH_ID_RESERVED94:
	case PERIPH_ID_RESERVED95:
		return PERIPH_ID_NONE;
	default:
		return clk_id;
	}
}

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

void clock_early_init(void)
{
	/*
	 * PLLP output frequency set to 408Mhz
	 * PLLC output frequency set to 228Mhz
	 */
	switch (clock_get_osc_freq()) {
	case CLOCK_OSC_FREQ_12_0: /* OSC is 12Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 408, 12, 0, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 456, 12, 1, 8);
		break;

	case CLOCK_OSC_FREQ_26_0: /* OSC is 26Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 408, 26, 0, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 26, 0, 8);
		break;

	case CLOCK_OSC_FREQ_13_0: /* OSC is 13Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 408, 13, 0, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 13, 0, 8);
		break;
	case CLOCK_OSC_FREQ_19_2:
	default:
		/*
		 * These are not supported. It is too early to print a
		 * message and the UART likely won't work anyway due to the
		 * oscillator being wrong.
		 */
		break;
	}
}

void clock_init(void)
{
	pll_rate[CLOCK_ID_MEMORY] = clock_get_rate(CLOCK_ID_MEMORY);
	pll_rate[CLOCK_ID_PERIPH] = clock_get_rate(CLOCK_ID_PERIPH);
	pll_rate[CLOCK_ID_CGENERAL] = clock_get_rate(CLOCK_ID_CGENERAL);
	pll_rate[CLOCK_ID_OSC] = clock_get_rate(CLOCK_ID_OSC);
	pll_rate[CLOCK_ID_SFROM32KHZ] = 32768;
	debug("Osc = %d\n", pll_rate[CLOCK_ID_OSC]);
	debug("PLLM = %d\n", pll_rate[CLOCK_ID_MEMORY]);
	debug("PLLP = %d\n", pll_rate[CLOCK_ID_PERIPH]);
}
