/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
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

#ifndef _PINMUX_H_
#define _PINMUX_H_

/*
 * Pin groups which we adjust. There are three basic attributes of each pin
 * group which use this enum:
 *
 *	- function
 *	- pullup / pulldown
 *	- tristate or normal
 */
enum pmux_pingrp {
	/* APB_MISC_PP_TRISTATE_REG_A_0 */
	PINGRP_ATA,
	PINGRP_ATB,
	PINGRP_ATC,
	PINGRP_ATD,
	PINGRP_CDEV1,
	PINGRP_CDEV2,
	PINGRP_CSUS,
	PINGRP_DAP1,

	PINGRP_DAP2,
	PINGRP_DAP3,
	PINGRP_DAP4,
	PINGRP_DTA,
	PINGRP_DTB,
	PINGRP_DTC,
	PINGRP_DTD,
	PINGRP_DTE,

	PINGRP_GPU,
	PINGRP_GPV,
	PINGRP_I2CP,
	PINGRP_IRTX,
	PINGRP_IRRX,
	PINGRP_KBCB,
	PINGRP_KBCA,
	PINGRP_PMC,

	PINGRP_PTA,
	PINGRP_RM,
	PINGRP_KBCE,
	PINGRP_KBCF,
	PINGRP_GMA,
	PINGRP_GMC,
	PINGRP_SDIO1,
	PINGRP_OWC,

	/* 32: APB_MISC_PP_TRISTATE_REG_B_0 */
	PINGRP_GME,
	PINGRP_SDC,
	PINGRP_SDD,
	PINGRP_RESERVED0,
	PINGRP_SLXA,
	PINGRP_SLXC,
	PINGRP_SLXD,
	PINGRP_SLXK,

	PINGRP_SPDI,
	PINGRP_SPDO,
	PINGRP_SPIA,
	PINGRP_SPIB,
	PINGRP_SPIC,
	PINGRP_SPID,
	PINGRP_SPIE,
	PINGRP_SPIF,

	PINGRP_SPIG,
	PINGRP_SPIH,
	PINGRP_UAA,
	PINGRP_UAB,
	PINGRP_UAC,
	PINGRP_UAD,
	PINGRP_UCA,
	PINGRP_UCB,

	PINGRP_RESERVED1,
	PINGRP_ATE,
	PINGRP_KBCC,
	PINGRP_RESERVED2,
	PINGRP_RESERVED3,
	PINGRP_GMB,
	PINGRP_GMD,
	PINGRP_DDC,

	/* 64: APB_MISC_PP_TRISTATE_REG_C_0 */
	PINGRP_LD0,
	PINGRP_LD1,
	PINGRP_LD2,
	PINGRP_LD3,
	PINGRP_LD4,
	PINGRP_LD5,
	PINGRP_LD6,
	PINGRP_LD7,

	PINGRP_LD8,
	PINGRP_LD9,
	PINGRP_LD10,
	PINGRP_LD11,
	PINGRP_LD12,
	PINGRP_LD13,
	PINGRP_LD14,
	PINGRP_LD15,

	PINGRP_LD16,
	PINGRP_LD17,
	PINGRP_LHP0,
	PINGRP_LHP1,
	PINGRP_LHP2,
	PINGRP_LVP0,
	PINGRP_LVP1,
	PINGRP_HDINT,

	PINGRP_LM0,
	PINGRP_LM1,
	PINGRP_LVS,
	PINGRP_LSC0,
	PINGRP_LSC1,
	PINGRP_LSCK,
	PINGRP_LDC,
	PINGRP_LCSN,

	/* 96: APB_MISC_PP_TRISTATE_REG_D_0 */
	PINGRP_LSPI,
	PINGRP_LSDA,
	PINGRP_LSDI,
	PINGRP_LPW0,
	PINGRP_LPW1,
	PINGRP_LPW2,
	PINGRP_LDI,
	PINGRP_LHS,

	PINGRP_LPP,
	PINGRP_RESERVED4,
	PINGRP_KBCD,
	PINGRP_GPU7,
	PINGRP_DTF,
	PINGRP_UDA,
	PINGRP_CRTP,
	PINGRP_SDB,

	/* these pin groups only have pullup and pull down control */
	PINGRP_FIRST_NO_MUX,
	PINGRP_CK32 = PINGRP_FIRST_NO_MUX,
	PINGRP_DDRC,
	PINGRP_PMCA,
	PINGRP_PMCB,
	PINGRP_PMCC,
	PINGRP_PMCD,
	PINGRP_PMCE,
	PINGRP_XM2C,
	PINGRP_XM2D,

	PINGRP_COUNT,
};

/*
 * Functions which can be assigned to each of the pin groups. The values here
 * bear no relation to the values programmed into pinmux registers and are
 * purely a convenience. The translation is done through a table search.
 */
enum pmux_func {
	PMUX_FUNC_AHB_CLK,
	PMUX_FUNC_APB_CLK,
	PMUX_FUNC_AUDIO_SYNC,
	PMUX_FUNC_CRT,
	PMUX_FUNC_DAP1,
	PMUX_FUNC_DAP2,
	PMUX_FUNC_DAP3,
	PMUX_FUNC_DAP4,
	PMUX_FUNC_DAP5,
	PMUX_FUNC_DISPA,
	PMUX_FUNC_DISPB,
	PMUX_FUNC_EMC_TEST0_DLL,
	PMUX_FUNC_EMC_TEST1_DLL,
	PMUX_FUNC_GMI,
	PMUX_FUNC_GMI_INT,
	PMUX_FUNC_HDMI,
	PMUX_FUNC_I2C,
	PMUX_FUNC_I2C2,
	PMUX_FUNC_I2C3,
	PMUX_FUNC_IDE,
	PMUX_FUNC_IRDA,
	PMUX_FUNC_KBC,
	PMUX_FUNC_MIO,
	PMUX_FUNC_MIPI_HS,
	PMUX_FUNC_NAND,
	PMUX_FUNC_OSC,
	PMUX_FUNC_OWR,
	PMUX_FUNC_PCIE,
	PMUX_FUNC_PLLA_OUT,
	PMUX_FUNC_PLLC_OUT1,
	PMUX_FUNC_PLLM_OUT1,
	PMUX_FUNC_PLLP_OUT2,
	PMUX_FUNC_PLLP_OUT3,
	PMUX_FUNC_PLLP_OUT4,
	PMUX_FUNC_PWM,
	PMUX_FUNC_PWR_INTR,
	PMUX_FUNC_PWR_ON,
	PMUX_FUNC_RTCK,
	PMUX_FUNC_SDIO1,
	PMUX_FUNC_SDIO2,
	PMUX_FUNC_SDIO3,
	PMUX_FUNC_SDIO4,
	PMUX_FUNC_SFLASH,
	PMUX_FUNC_SPDIF,
	PMUX_FUNC_SPI1,
	PMUX_FUNC_SPI2,
	PMUX_FUNC_SPI2_ALT,
	PMUX_FUNC_SPI3,
	PMUX_FUNC_SPI4,
	PMUX_FUNC_TRACE,
	PMUX_FUNC_TWC,
	PMUX_FUNC_UARTA,
	PMUX_FUNC_UARTB,
	PMUX_FUNC_UARTC,
	PMUX_FUNC_UARTD,
	PMUX_FUNC_UARTE,
	PMUX_FUNC_ULPI,
	PMUX_FUNC_VI,
	PMUX_FUNC_VI_SENSOR_CLK,
	PMUX_FUNC_XIO,
	PMUX_FUNC_SAFE,

	/* These don't have a name, but can be used in the table */
	PMUX_FUNC_RSVD1,
	PMUX_FUNC_RSVD2,
	PMUX_FUNC_RSVD3,
	PMUX_FUNC_RSVD4,
	PMUX_FUNC_RSVD,	/* Not valid and should not be used */

	PMUX_FUNC_COUNT,

	PMUX_FUNC_NONE = -1,
};

/* return 1 if a pmux_func is in range */
#define pmux_func_isvalid(func) ((func) >= 0 && (func) < PMUX_FUNC_COUNT && \
		(func) != PMUX_FUNC_RSVD)

/* The pullup/pulldown state of a pin group */
enum pmux_pull {
	PMUX_PULL_NORMAL = 0,
	PMUX_PULL_DOWN,
	PMUX_PULL_UP,
};

/* Defines whether a pin group is tristated or in normal operation */
enum pmux_tristate {
	PMUX_TRI_NORMAL = 0,
	PMUX_TRI_TRISTATE = 1,
};

/* Available power domains used by pin groups */
enum pmux_vddio {
	PMUX_VDDIO_BB = 0,
	PMUX_VDDIO_LCD,
	PMUX_VDDIO_VI,
	PMUX_VDDIO_UART,
	PMUX_VDDIO_DDR,
	PMUX_VDDIO_NAND,
	PMUX_VDDIO_SYS,
	PMUX_VDDIO_AUDIO,
	PMUX_VDDIO_SD,

	PMUX_VDDIO_NONE
};

enum {
	PMUX_TRISTATE_REGS	= 4,
	PMUX_MUX_REGS		= 7,
	PMUX_PULL_REGS		= 5,
};

/* APB MISC Pin Mux and Tristate (APB_MISC_PP_) registers */
struct pmux_tri_ctlr {
	uint pmt_reserved0;		/* ABP_MISC_PP_ reserved offset 00 */
	uint pmt_reserved1;		/* ABP_MISC_PP_ reserved offset 04 */
	uint pmt_strap_opt_a;		/* _STRAPPING_OPT_A_0, offset 08   */
	uint pmt_reserved2;		/* ABP_MISC_PP_ reserved offset 0C */
	uint pmt_reserved3;		/* ABP_MISC_PP_ reserved offset 10 */
	uint pmt_tri[PMUX_TRISTATE_REGS];/* _TRI_STATE_REG_A/B/C/D_0 14-20 */
	uint pmt_cfg_ctl;		/* _CONFIG_CTL_0, offset 24        */

	uint pmt_reserved[22];		/* ABP_MISC_PP_ reserved offs 28-7C */

	uint pmt_ctl[PMUX_MUX_REGS];	/* _PIN_MUX_CTL_A-G_0, offset 80   */
	uint pmt_reserved4;		/* ABP_MISC_PP_ reserved offset 9c */
	uint pmt_pull[PMUX_PULL_REGS];	/* APB_MISC_PP_PULLUPDOWN_REG_A-E  */
};

/*
 * This defines the configuration for a pin, including the function assigned,
 * pull up/down settings and tristate settings. Having set up one of these
 * you can call pinmux_config_pingroup() to configure a pin in one step. Also
 * available is pinmux_config_table() to configure a list of pins.
 */
struct pingroup_config {
	enum pmux_pingrp pingroup;	/* pin group PINGRP_...             */
	enum pmux_func func;		/* function to assign FUNC_...      */
	enum pmux_pull pull;		/* pull up/down/normal PMUX_PULL_...*/
	enum pmux_tristate tristate;	/* tristate or normal PMUX_TRI_...  */
};

/* Set a pin group to tristate */
void pinmux_tristate_enable(enum pmux_pingrp pin);

/* Set a pin group to normal (non tristate) */
void pinmux_tristate_disable(enum pmux_pingrp pin);

/* Set the pull up/down feature for a pin group */
void pinmux_set_pullupdown(enum pmux_pingrp pin, enum pmux_pull pupd);

/* Set the mux function for a pin group */
void pinmux_set_func(enum pmux_pingrp pin, enum pmux_func func);

/* Set the complete configuration for a pin group */
void pinmux_config_pingroup(const struct pingroup_config *config);

void pinmux_set_tristate(enum pmux_pingrp pin, int enable);

/**
 * Configuure a list of pin groups
 *
 * @param config	List of config items
 * @param len		Number of config items in list
 */
void pinmux_config_table(const struct pingroup_config *config, int len);

#endif	/* PINMUX_H */
