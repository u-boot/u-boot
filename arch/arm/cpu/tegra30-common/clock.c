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
 * Clock types that we can use as a source. The Tegra30 has muxes for the
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
	CLOCK_TYPE_NONE = -1,   /* invalid clock type */
};

enum {
	CLOCK_MAX_MUX   = 8     /* number of source options for each clock */
};

enum {
	MASK_BITS_31_30 = 2,    /* num of bits used to specify clock source */
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
	{ CLK(AUDIO),   CLK(XCPU),      CLK(PERIPH),    CLK(OSC),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(MEMORY),  CLK(CGENERAL),  CLK(PERIPH),    CLK(AUDIO),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(MEMORY),  CLK(CGENERAL),  CLK(PERIPH),    CLK(OSC),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),  CLK(CGENERAL),  CLK(MEMORY),    CLK(NONE),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),  CLK(CGENERAL),  CLK(MEMORY),    CLK(OSC),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),  CLK(CGENERAL),  CLK(MEMORY),    CLK(OSC),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(PERIPH),  CLK(DISPLAY),   CLK(CGENERAL),  CLK(OSC),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(AUDIO),   CLK(CGENERAL),  CLK(PERIPH),    CLK(OSC),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_30},
	{ CLK(AUDIO),   CLK(SFROM32KHZ),	CLK(PERIPH),   CLK(OSC),
		CLK(EPCI),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_31_29},
	{ CLK(PERIPH),  CLK(MEMORY),    CLK(DISPLAY),   CLK(AUDIO),
		CLK(CGENERAL),  CLK(DISPLAY2),  CLK(OSC),       CLK(NONE),
		MASK_BITS_31_29},
	{ CLK(PERIPH),  CLK(CGENERAL),  CLK(SFROM32KHZ), CLK(OSC),
		CLK(NONE),      CLK(NONE),      CLK(NONE),      CLK(NONE),
		MASK_BITS_29_28}
};

/*
 * Clock type for each peripheral clock source. We put the name in each
 * record just so it is easy to match things up
 */
#define TYPE(name, type) type
static enum clock_type_id clock_periph_type[PERIPHC_COUNT] = {
	/* 0x00 */
	TYPE(PERIPHC_I2S1,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2S2,      CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_SPDIF_OUT, CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_SPDIF_IN,  CLOCK_TYPE_PCM),
	TYPE(PERIPHC_PWM,       CLOCK_TYPE_PCST),  /* only PWM uses b29:28 */
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC2,      CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SBC3,      CLOCK_TYPE_PCMT),

	/* 0x08 */
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_I2C1,      CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_DVC_I2C,   CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC1,      CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_DISP1,     CLOCK_TYPE_PMDACD2T),
	TYPE(PERIPHC_DISP2,     CLOCK_TYPE_PMDACD2T),

	/* 0x10 */
	TYPE(PERIPHC_CVE,       CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VI,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SDMMC1,    CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SDMMC2,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_G3D,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_G2D,	CLOCK_TYPE_MCPA),

	/* 0x18 */
	TYPE(PERIPHC_NDFLASH,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SDMMC4,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_VFIR,      CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_EPP,       CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_MPE,       CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_MIPI,      CLOCK_TYPE_PCMT),       /* MIPI base-band HSI */
	TYPE(PERIPHC_UART1,     CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_UART2,     CLOCK_TYPE_PCMT),

	/* 0x20 */
	TYPE(PERIPHC_HOST1X,    CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_TVO,       CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_HDMI,      CLOCK_TYPE_PMDACD2T),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_TVDAC,     CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_I2C2,      CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_EMC,	CLOCK_TYPE_MCPT),

	/* 0x28 */
	TYPE(PERIPHC_UART3,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VI,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SBC4,      CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_I2C3,      CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_SDMMC3,    CLOCK_TYPE_PCMT),

	/* 0x30 */
	TYPE(PERIPHC_UART4,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_UART5,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_VDE,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_OWR,       CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NOR,       CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_CSITE,     CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_I2S0,      CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),

	/* 0x38h */	     /* Jumps to reg offset 0x3B0h - new for T30 */
	TYPE(PERIPHC_G3D2,      CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_MSELECT,   CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_TSENSOR,   CLOCK_TYPE_PCST),       /* s/b PCTS */
	TYPE(PERIPHC_I2S3,      CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2S4,      CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2C4,      CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_SBC5,      CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SBC6,      CLOCK_TYPE_PCMT),

	/* 0x40 */
	TYPE(PERIPHC_AUDIO,     CLOCK_TYPE_ACPT),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_DAM0,      CLOCK_TYPE_ACPT),
	TYPE(PERIPHC_DAM1,      CLOCK_TYPE_ACPT),
	TYPE(PERIPHC_DAM2,      CLOCK_TYPE_ACPT),
	TYPE(PERIPHC_HDA2CODEC2X, CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_ACTMON,    CLOCK_TYPE_PCST),       /* MASK 31:30 */
	TYPE(PERIPHC_EXTPERIPH1, CLOCK_TYPE_ASPTE),

	/* 0x48 */
	TYPE(PERIPHC_EXTPERIPH2, CLOCK_TYPE_ASPTE),
	TYPE(PERIPHC_EXTPERIPH3, CLOCK_TYPE_ASPTE),
	TYPE(PERIPHC_NANDSPEED, CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_I2CSLOW,   CLOCK_TYPE_PCST),       /* MASK 31:30 */
	TYPE(PERIPHC_SYS,       CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SPEEDO,    CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),

	/* 0x50 */
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,      CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SATAOOB,   CLOCK_TYPE_PCMT),       /* offset 0x420h */
	TYPE(PERIPHC_SATA,      CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_HDA,       CLOCK_TYPE_PCMT),
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
	PERIPHC_UART2,  /* and vfir 0x68 */

	/* 8 */
	NONE(GPIO),
	PERIPHC_SDMMC2,
	NONE(SPDIF),	    /* 0x08 and 0x0c, unclear which to use */
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
	PERIPHC_SBC1,
	PERIPHC_NOR,
	NONE(RESERVED43),
	PERIPHC_SBC2,
	NONE(RESERVED45),
	PERIPHC_SBC3,
	PERIPHC_DVC_I2C,

	/* 48 */
	NONE(DSI),
	PERIPHC_TVO,    /* also CVE 0x40 */
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
 * field. Note that T30 supports 3 new higher freqs, but we map back
 * to the old T20 freqs. Support for the higher oscillators is TBD.
 */
enum clock_osc_freq clock_get_osc_freq(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	reg = readl(&clkrst->crc_osc_ctrl);
	reg = (reg & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;

	if (reg & 1)			/* one of the newer freqs */
		printf("Warning: OSC_FREQ is unsupported! (%d)\n", reg);

	return reg >> 2;	/* Map to most common (T20) freqs */
}

/* Returns a pointer to the clock source register for a peripheral */
u32 *get_periph_source_reg(enum periph_id periph_id)
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

/**
 * Given a peripheral ID and the required source clock, this returns which
 * value should be programmed into the source mux for that peripheral.
 *
 * There is special code here to handle the one source type with 5 sources.
 *
 * @param periph_id	peripheral to start
 * @param source	PLL id of required parent clock
 * @param mux_bits	Set to number of bits in mux register: 2 or 4
 * @param divider_bits  Set to number of divider bits (8 or 16)
 * @return mux value (0-4, or -1 if not found)
 */
int get_periph_clock_source(enum periph_id periph_id,
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

#ifdef CONFIG_OF_CONTROL
/*
 * Convert a device tree clock ID to our peripheral ID. They are mostly
 * the same but we are very cautious so we check that a valid clock ID is
 * provided.
 *
 * @param clk_id	Clock ID according to tegra30 device tree binding
 * @return peripheral ID, or PERIPH_ID_NONE if the clock ID is invalid
 */
enum periph_id clk_id_to_periph_id(int clk_id)
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
#endif /* CONFIG_OF_CONTROL */

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

void arch_timer_init(void)
{
}
