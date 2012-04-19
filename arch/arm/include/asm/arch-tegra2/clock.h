/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* Tegra2 clock control functions */

#ifndef _CLOCK_H
#define _CLOCK_H

/* Set of oscillator frequencies supported in the internal API. */
enum clock_osc_freq {
	/* All in MHz, so 13_0 is 13.0MHz */
	CLOCK_OSC_FREQ_13_0,
	CLOCK_OSC_FREQ_19_2,
	CLOCK_OSC_FREQ_12_0,
	CLOCK_OSC_FREQ_26_0,

	CLOCK_OSC_FREQ_COUNT,
};

/* The PLLs supported by the hardware */
enum clock_id {
	CLOCK_ID_FIRST,
	CLOCK_ID_CGENERAL = CLOCK_ID_FIRST,
	CLOCK_ID_MEMORY,
	CLOCK_ID_PERIPH,
	CLOCK_ID_AUDIO,
	CLOCK_ID_USB,
	CLOCK_ID_DISPLAY,

	/* now the simple ones */
	CLOCK_ID_FIRST_SIMPLE,
	CLOCK_ID_XCPU = CLOCK_ID_FIRST_SIMPLE,
	CLOCK_ID_EPCI,
	CLOCK_ID_SFROM32KHZ,

	/* These are the base clocks (inputs to the Tegra SOC) */
	CLOCK_ID_32KHZ,
	CLOCK_ID_OSC,

	CLOCK_ID_COUNT,	/* number of clocks */
	CLOCK_ID_NONE = -1,
};

/* The clocks supported by the hardware */
enum periph_id {
	PERIPH_ID_FIRST,

	/* Low word: 31:0 */
	PERIPH_ID_CPU = PERIPH_ID_FIRST,
	PERIPH_ID_RESERVED1,
	PERIPH_ID_RESERVED2,
	PERIPH_ID_AC97,
	PERIPH_ID_RTC,
	PERIPH_ID_TMR,
	PERIPH_ID_UART1,
	PERIPH_ID_UART2,

	/* 8 */
	PERIPH_ID_GPIO,
	PERIPH_ID_SDMMC2,
	PERIPH_ID_SPDIF,
	PERIPH_ID_I2S1,
	PERIPH_ID_I2C1,
	PERIPH_ID_NDFLASH,
	PERIPH_ID_SDMMC1,
	PERIPH_ID_SDMMC4,

	/* 16 */
	PERIPH_ID_TWC,
	PERIPH_ID_PWM,
	PERIPH_ID_I2S2,
	PERIPH_ID_EPP,
	PERIPH_ID_VI,
	PERIPH_ID_2D,
	PERIPH_ID_USBD,
	PERIPH_ID_ISP,

	/* 24 */
	PERIPH_ID_3D,
	PERIPH_ID_IDE,
	PERIPH_ID_DISP2,
	PERIPH_ID_DISP1,
	PERIPH_ID_HOST1X,
	PERIPH_ID_VCP,
	PERIPH_ID_RESERVED30,
	PERIPH_ID_CACHE2,

	/* Middle word: 63:32 */
	PERIPH_ID_MEM,
	PERIPH_ID_AHBDMA,
	PERIPH_ID_APBDMA,
	PERIPH_ID_RESERVED35,
	PERIPH_ID_KBC,
	PERIPH_ID_STAT_MON,
	PERIPH_ID_PMC,
	PERIPH_ID_FUSE,

	/* 40 */
	PERIPH_ID_KFUSE,
	PERIPH_ID_SBC1,
	PERIPH_ID_SNOR,
	PERIPH_ID_SPI1,
	PERIPH_ID_SBC2,
	PERIPH_ID_XIO,
	PERIPH_ID_SBC3,
	PERIPH_ID_DVC_I2C,

	/* 48 */
	PERIPH_ID_DSI,
	PERIPH_ID_TVO,
	PERIPH_ID_MIPI,
	PERIPH_ID_HDMI,
	PERIPH_ID_CSI,
	PERIPH_ID_TVDAC,
	PERIPH_ID_I2C2,
	PERIPH_ID_UART3,

	/* 56 */
	PERIPH_ID_RESERVED56,
	PERIPH_ID_EMC,
	PERIPH_ID_USB2,
	PERIPH_ID_USB3,
	PERIPH_ID_MPE,
	PERIPH_ID_VDE,
	PERIPH_ID_BSEA,
	PERIPH_ID_BSEV,

	/* Upper word 95:64 */
	PERIPH_ID_SPEEDO,
	PERIPH_ID_UART4,
	PERIPH_ID_UART5,
	PERIPH_ID_I2C3,
	PERIPH_ID_SBC4,
	PERIPH_ID_SDMMC3,
	PERIPH_ID_PCIE,
	PERIPH_ID_OWR,

	/* 72 */
	PERIPH_ID_AFI,
	PERIPH_ID_CORESIGHT,
	PERIPH_ID_RESERVED74,
	PERIPH_ID_AVPUCQ,
	PERIPH_ID_RESERVED76,
	PERIPH_ID_RESERVED77,
	PERIPH_ID_RESERVED78,
	PERIPH_ID_RESERVED79,

	/* 80 */
	PERIPH_ID_RESERVED80,
	PERIPH_ID_RESERVED81,
	PERIPH_ID_RESERVED82,
	PERIPH_ID_RESERVED83,
	PERIPH_ID_IRAMA,
	PERIPH_ID_IRAMB,
	PERIPH_ID_IRAMC,
	PERIPH_ID_IRAMD,

	/* 88 */
	PERIPH_ID_CRAM2,

	PERIPH_ID_COUNT,
	PERIPH_ID_NONE = -1,
};

/* Converts a clock number to a clock register: 0=L, 1=H, 2=U */
#define PERIPH_REG(id) ((id) >> 5)

/* Mask value for a clock (within PERIPH_REG(id)) */
#define PERIPH_MASK(id) (1 << ((id) & 0x1f))

/* return 1 if a PLL ID is in range, and not a simple PLL */
#define clock_id_is_pll(id) ((id) >= CLOCK_ID_FIRST && \
		(id) < CLOCK_ID_FIRST_SIMPLE)

/* PLL stabilization delay in usec */
#define CLOCK_PLL_STABLE_DELAY_US 300

/* return the current oscillator clock frequency */
enum clock_osc_freq clock_get_osc_freq(void);

/**
 * Start PLL using the provided configuration parameters.
 *
 * @param id	clock id
 * @param divm	input divider
 * @param divn	feedback divider
 * @param divp	post divider 2^n
 * @param cpcon	charge pump setup control
 * @param lfcon	loop filter setup control
 *
 * @returns monotonic time in us that the PLL will be stable
 */
unsigned long clock_start_pll(enum clock_id id, u32 divm, u32 divn,
		u32 divp, u32 cpcon, u32 lfcon);

/**
 * Read low-level parameters of a PLL.
 *
 * @param id	clock id to read (note: USB is not supported)
 * @param divm	returns input divider
 * @param divn	returns feedback divider
 * @param divp	returns post divider 2^n
 * @param cpcon	returns charge pump setup control
 * @param lfcon	returns loop filter setup control
 *
 * @returns 0 if ok, -1 on error (invalid clock id)
 */
int clock_ll_read_pll(enum clock_id clkid, u32 *divm, u32 *divn,
		      u32 *divp, u32 *cpcon, u32 *lfcon);

/*
 * Enable a clock
 *
 * @param id	clock id
 */
void clock_enable(enum periph_id clkid);

/*
 * Disable a clock
 *
 * @param id	clock id
 */
void clock_disable(enum periph_id clkid);

/*
 * Set whether a clock is enabled or disabled.
 *
 * @param id		clock id
 * @param enable	1 to enable, 0 to disable
 */
void clock_set_enable(enum periph_id clkid, int enable);

/**
 * Reset a peripheral. This puts it in reset, waits for a delay, then takes
 * it out of reset and waits for th delay again.
 *
 * @param periph_id	peripheral to reset
 * @param us_delay	time to delay in microseconds
 */
void reset_periph(enum periph_id periph_id, int us_delay);

/**
 * Put a peripheral into or out of reset.
 *
 * @param periph_id	peripheral to reset
 * @param enable	1 to put into reset, 0 to take out of reset
 */
void reset_set_enable(enum periph_id periph_id, int enable);


/* CLK_RST_CONTROLLER_RST_CPU_CMPLX_SET/CLR_0 */
enum crc_reset_id {
	/* Things we can hold in reset for each CPU */
	crc_rst_cpu = 1,
	crc_rst_de = 1 << 2,	/* What is de? */
	crc_rst_watchdog = 1 << 3,
	crc_rst_debug = 1 << 4,
};

/**
 * Put parts of the CPU complex into or out of reset.\
 *
 * @param cpu		cpu number (0 or 1 on Tegra2)
 * @param which		which parts of the complex to affect (OR of crc_reset_id)
 * @param reset		1 to assert reset, 0 to de-assert
 */
void reset_cmplx_set_enable(int cpu, int which, int reset);

/**
 * Set the source for a peripheral clock. This plus the divisor sets the
 * clock rate. You need to look up the datasheet to see the meaning of the
 * source parameter as it changes for each peripheral.
 *
 * Warning: This function is only for use pre-relocation. Please use
 * clock_start_periph_pll() instead.
 *
 * @param periph_id	peripheral to adjust
 * @param source	source clock (0, 1, 2 or 3)
 */
void clock_ll_set_source(enum periph_id periph_id, unsigned source);

/**
 * Set the source and divisor for a peripheral clock. This sets the
 * clock rate. You need to look up the datasheet to see the meaning of the
 * source parameter as it changes for each peripheral.
 *
 * Warning: This function is only for use pre-relocation. Please use
 * clock_start_periph_pll() instead.
 *
 * @param periph_id	peripheral to adjust
 * @param source	source clock (0, 1, 2 or 3)
 * @param divisor	divisor value to use
 */
void clock_ll_set_source_divisor(enum periph_id periph_id, unsigned source,
		unsigned divisor);

/**
 * Start a peripheral PLL clock at the given rate. This also resets the
 * peripheral.
 *
 * @param periph_id	peripheral to start
 * @param parent	PLL id of required parent clock
 * @param rate		Required clock rate in Hz
 * @return rate selected in Hz, or -1U if something went wrong
 */
unsigned clock_start_periph_pll(enum periph_id periph_id,
		enum clock_id parent, unsigned rate);

/**
 * Returns the rate of a peripheral clock in Hz. Since the caller almost
 * certainly knows the parent clock (having just set it) we require that
 * this be passed in so we don't need to work it out.
 *
 * @param periph_id	peripheral to start
 * @param parent	PLL id of parent clock (used to calculate rate, you
 *			must know this!)
 * @return clock rate of peripheral in Hz
 */
unsigned long clock_get_periph_rate(enum periph_id periph_id,
		enum clock_id parent);

/**
 * Adjust peripheral PLL clock to the given rate. This does not reset the
 * peripheral. If a second stage divisor is not available, pass NULL for
 * extra_div. If it is available, then this parameter will return the
 * divisor selected (which will be a power of 2 from 1 to 256).
 *
 * @param periph_id	peripheral to start
 * @param parent	PLL id of required parent clock
 * @param rate		Required clock rate in Hz
 * @param extra_div	value for the second-stage divisor (NULL if one is
			not available)
 * @return rate selected in Hz, or -1U if something went wrong
 */
unsigned clock_adjust_periph_pll_div(enum periph_id periph_id,
		enum clock_id parent, unsigned rate, int *extra_div);

/**
 * Returns the clock rate of a specified clock, in Hz.
 *
 * @param parent	PLL id of clock to check
 * @return rate of clock in Hz
 */
unsigned clock_get_rate(enum clock_id clkid);

/**
 * Start up a UART using low-level calls
 *
 * Prior to relocation clock_start_periph_pll() cannot be called. This
 * function provides a way to set up a UART using low-level calls which
 * do not require BSS.
 *
 * @param periph_id	Peripheral ID of UART to enable (e,g, PERIPH_ID_UART1)
 */
void clock_ll_start_uart(enum periph_id periph_id);

/**
 * Decode a peripheral ID from a device tree node.
 *
 * This works by looking up the peripheral's 'clocks' node and reading out
 * the second cell, which is the clock number / peripheral ID.
 *
 * @param blob		FDT blob to use
 * @param node		Node to look at
 * @return peripheral ID, or PERIPH_ID_NONE if none
 */
enum periph_id clock_decode_periph_id(const void *blob, int node);

/**
 * Checks if the oscillator bypass is enabled (XOBP bit)
 *
 * @return 1 if bypass is enabled, 0 if not
 */
int clock_get_osc_bypass(void);

/*
 * Checks that clocks are valid and prints a warning if not
 *
 * @return 0 if ok, -1 on error
 */
int clock_verify(void);

/* Initialize the clocks */
void clock_init(void);

/* Initialize the PLLs */
void clock_early_init(void);

#endif
