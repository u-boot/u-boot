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

/* Tegra20 pin multiplexing functions */

#include <asm/io.h>
#include <asm/arch/tegra20.h>
#include <asm/arch/pinmux.h>
#include <common.h>


/*
 * This defines the order of the pin mux control bits in the registers. For
 * some reason there is no correspendence between the tristate, pin mux and
 * pullup/pulldown registers.
 */
enum pmux_ctlid {
	/* 0: APB_MISC_PP_PIN_MUX_CTL_A_0 */
	MUXCTL_UAA,
	MUXCTL_UAB,
	MUXCTL_UAC,
	MUXCTL_UAD,
	MUXCTL_UDA,
	MUXCTL_RESERVED5,
	MUXCTL_ATE,
	MUXCTL_RM,

	MUXCTL_ATB,
	MUXCTL_RESERVED9,
	MUXCTL_ATD,
	MUXCTL_ATC,
	MUXCTL_ATA,
	MUXCTL_KBCF,
	MUXCTL_KBCE,
	MUXCTL_SDMMC1,

	/* 16: APB_MISC_PP_PIN_MUX_CTL_B_0 */
	MUXCTL_GMA,
	MUXCTL_GMC,
	MUXCTL_HDINT,
	MUXCTL_SLXA,
	MUXCTL_OWC,
	MUXCTL_SLXC,
	MUXCTL_SLXD,
	MUXCTL_SLXK,

	MUXCTL_UCA,
	MUXCTL_UCB,
	MUXCTL_DTA,
	MUXCTL_DTB,
	MUXCTL_RESERVED28,
	MUXCTL_DTC,
	MUXCTL_DTD,
	MUXCTL_DTE,

	/* 32: APB_MISC_PP_PIN_MUX_CTL_C_0 */
	MUXCTL_DDC,
	MUXCTL_CDEV1,
	MUXCTL_CDEV2,
	MUXCTL_CSUS,
	MUXCTL_I2CP,
	MUXCTL_KBCA,
	MUXCTL_KBCB,
	MUXCTL_KBCC,

	MUXCTL_IRTX,
	MUXCTL_IRRX,
	MUXCTL_DAP1,
	MUXCTL_DAP2,
	MUXCTL_DAP3,
	MUXCTL_DAP4,
	MUXCTL_GMB,
	MUXCTL_GMD,

	/* 48: APB_MISC_PP_PIN_MUX_CTL_D_0 */
	MUXCTL_GME,
	MUXCTL_GPV,
	MUXCTL_GPU,
	MUXCTL_SPDO,
	MUXCTL_SPDI,
	MUXCTL_SDB,
	MUXCTL_SDC,
	MUXCTL_SDD,

	MUXCTL_SPIH,
	MUXCTL_SPIG,
	MUXCTL_SPIF,
	MUXCTL_SPIE,
	MUXCTL_SPID,
	MUXCTL_SPIC,
	MUXCTL_SPIB,
	MUXCTL_SPIA,

	/* 64: APB_MISC_PP_PIN_MUX_CTL_E_0 */
	MUXCTL_LPW0,
	MUXCTL_LPW1,
	MUXCTL_LPW2,
	MUXCTL_LSDI,
	MUXCTL_LSDA,
	MUXCTL_LSPI,
	MUXCTL_LCSN,
	MUXCTL_LDC,

	MUXCTL_LSCK,
	MUXCTL_LSC0,
	MUXCTL_LSC1,
	MUXCTL_LHS,
	MUXCTL_LVS,
	MUXCTL_LM0,
	MUXCTL_LM1,
	MUXCTL_LVP0,

	/* 80: APB_MISC_PP_PIN_MUX_CTL_F_0 */
	MUXCTL_LD0,
	MUXCTL_LD1,
	MUXCTL_LD2,
	MUXCTL_LD3,
	MUXCTL_LD4,
	MUXCTL_LD5,
	MUXCTL_LD6,
	MUXCTL_LD7,

	MUXCTL_LD8,
	MUXCTL_LD9,
	MUXCTL_LD10,
	MUXCTL_LD11,
	MUXCTL_LD12,
	MUXCTL_LD13,
	MUXCTL_LD14,
	MUXCTL_LD15,

	/* 96: APB_MISC_PP_PIN_MUX_CTL_G_0 */
	MUXCTL_LD16,
	MUXCTL_LD17,
	MUXCTL_LHP1,
	MUXCTL_LHP2,
	MUXCTL_LVP1,
	MUXCTL_LHP0,
	MUXCTL_RESERVED102,
	MUXCTL_LPP,

	MUXCTL_LDI,
	MUXCTL_PMC,
	MUXCTL_CRTP,
	MUXCTL_PTA,
	MUXCTL_RESERVED108,
	MUXCTL_KBCD,
	MUXCTL_GPU7,
	MUXCTL_DTF,

	MUXCTL_NONE = -1,
};

/*
 * And this defines the order of the pullup/pulldown controls which are again
 * in a different order
 */
enum pmux_pullid {
	/* 0: APB_MISC_PP_PULLUPDOWN_REG_A_0 */
	PUCTL_ATA,
	PUCTL_ATB,
	PUCTL_ATC,
	PUCTL_ATD,
	PUCTL_ATE,
	PUCTL_DAP1,
	PUCTL_DAP2,
	PUCTL_DAP3,

	PUCTL_DAP4,
	PUCTL_DTA,
	PUCTL_DTB,
	PUCTL_DTC,
	PUCTL_DTD,
	PUCTL_DTE,
	PUCTL_DTF,
	PUCTL_GPV,

	/* 16: APB_MISC_PP_PULLUPDOWN_REG_B_0 */
	PUCTL_RM,
	PUCTL_I2CP,
	PUCTL_PTA,
	PUCTL_GPU7,
	PUCTL_KBCA,
	PUCTL_KBCB,
	PUCTL_KBCC,
	PUCTL_KBCD,

	PUCTL_SPDI,
	PUCTL_SPDO,
	PUCTL_GPSLXAU,
	PUCTL_CRTP,
	PUCTL_SLXC,
	PUCTL_SLXD,
	PUCTL_SLXK,

	/* 32: APB_MISC_PP_PULLUPDOWN_REG_C_0 */
	PUCTL_CDEV1,
	PUCTL_CDEV2,
	PUCTL_SPIA,
	PUCTL_SPIB,
	PUCTL_SPIC,
	PUCTL_SPID,
	PUCTL_SPIE,
	PUCTL_SPIF,

	PUCTL_SPIG,
	PUCTL_SPIH,
	PUCTL_IRTX,
	PUCTL_IRRX,
	PUCTL_GME,
	PUCTL_RESERVED45,
	PUCTL_XM2D,
	PUCTL_XM2C,

	/* 48: APB_MISC_PP_PULLUPDOWN_REG_D_0 */
	PUCTL_UAA,
	PUCTL_UAB,
	PUCTL_UAC,
	PUCTL_UAD,
	PUCTL_UCA,
	PUCTL_UCB,
	PUCTL_LD17,
	PUCTL_LD19_18,

	PUCTL_LD21_20,
	PUCTL_LD23_22,
	PUCTL_LS,
	PUCTL_LC,
	PUCTL_CSUS,
	PUCTL_DDRC,
	PUCTL_SDC,
	PUCTL_SDD,

	/* 64: APB_MISC_PP_PULLUPDOWN_REG_E_0 */
	PUCTL_KBCF,
	PUCTL_KBCE,
	PUCTL_PMCA,
	PUCTL_PMCB,
	PUCTL_PMCC,
	PUCTL_PMCD,
	PUCTL_PMCE,
	PUCTL_CK32,

	PUCTL_UDA,
	PUCTL_SDMMC1,
	PUCTL_GMA,
	PUCTL_GMB,
	PUCTL_GMC,
	PUCTL_GMD,
	PUCTL_DDC,
	PUCTL_OWC,

	PUCTL_NONE = -1
};

struct tegra_pingroup_desc {
	const char *name;
	enum pmux_func funcs[4];
	enum pmux_func func_safe;
	enum pmux_vddio vddio;
	enum pmux_ctlid ctl_id;
	enum pmux_pullid pull_id;
};


/* Converts a pmux_pingrp number to a tristate register: 0=A, 1=B, 2=C, 3=D */
#define TRISTATE_REG(pmux_pingrp) ((pmux_pingrp) >> 5)

/* Mask value for a tristate (within TRISTATE_REG(id)) */
#define TRISTATE_MASK(pmux_pingrp) (1 << ((pmux_pingrp) & 0x1f))

/* Converts a PUCTL id to a pull register: 0=A, 1=B...4=E */
#define PULL_REG(pmux_pullid) ((pmux_pullid) >> 4)

/* Converts a PUCTL id to a shift position */
#define PULL_SHIFT(pmux_pullid) ((pmux_pullid << 1) & 0x1f)

/* Converts a MUXCTL id to a ctl register: 0=A, 1=B...6=G */
#define MUXCTL_REG(pmux_ctlid) ((pmux_ctlid) >> 4)

/* Converts a MUXCTL id to a shift position */
#define MUXCTL_SHIFT(pmux_ctlid) ((pmux_ctlid << 1) & 0x1f)

/* Convenient macro for defining pin group properties */
#define PINALL(pg_name, vdd, f0, f1, f2, f3, f_safe, mux, pupd)		\
	{						\
		.vddio = PMUX_VDDIO_ ## vdd,		\
		.funcs = {				\
			PMUX_FUNC_ ## f0,			\
			PMUX_FUNC_ ## f1,			\
			PMUX_FUNC_ ## f2,			\
			PMUX_FUNC_ ## f3,			\
		},					\
		.func_safe = PMUX_FUNC_ ## f_safe,		\
		.ctl_id = mux,				\
		.pull_id = pupd				\
	}

/* A normal pin group where the mux name and pull-up name match */
#define PIN(pg_name, vdd, f0, f1, f2, f3, f_safe)		\
		PINALL(pg_name, vdd, f0, f1, f2, f3, f_safe,	\
			MUXCTL_ ## pg_name, PUCTL_ ## pg_name)

/* A pin group where the pull-up name doesn't have a 1-1 mapping */
#define PINP(pg_name, vdd, f0, f1, f2, f3, f_safe, pupd)		\
		PINALL(pg_name, vdd, f0, f1, f2, f3, f_safe,	\
			MUXCTL_ ## pg_name, PUCTL_ ## pupd)

/* A pin group number which is not used */
#define PIN_RESERVED \
	PIN(NONE, NONE, NONE, NONE, NONE, NONE, NONE)

const struct tegra_pingroup_desc tegra_soc_pingroups[PINGRP_COUNT] = {
	PIN(ATA,  NAND,  IDE,    NAND,   GMI,       RSVD,        IDE),
	PIN(ATB,  NAND,  IDE,    NAND,   GMI,       SDIO4,       IDE),
	PIN(ATC,  NAND,  IDE,    NAND,   GMI,       SDIO4,       IDE),
	PIN(ATD,  NAND,  IDE,    NAND,   GMI,       SDIO4,       IDE),
	PIN(CDEV1, AUDIO, OSC,   PLLA_OUT, PLLM_OUT1, AUDIO_SYNC, OSC),
	PIN(CDEV2, AUDIO, OSC,   AHB_CLK, APB_CLK, PLLP_OUT4,    OSC),
	PIN(CSUS, VI, PLLC_OUT1, PLLP_OUT2, PLLP_OUT3, VI_SENSOR_CLK,
		PLLC_OUT1),
	PIN(DAP1, AUDIO, DAP1,   RSVD,   GMI,       SDIO2,       DAP1),

	PIN(DAP2, AUDIO, DAP2,   TWC,    RSVD,      GMI,         DAP2),
	PIN(DAP3, BB,    DAP3,   RSVD,   RSVD,      RSVD,        DAP3),
	PIN(DAP4, UART,  DAP4,   RSVD,   GMI,       RSVD,        DAP4),
	PIN(DTA,  VI,    RSVD,   SDIO2,  VI,        RSVD,        RSVD4),
	PIN(DTB,  VI,    RSVD,   RSVD,   VI,        SPI1,        RSVD1),
	PIN(DTC,  VI,    RSVD,   RSVD,   VI,        RSVD,        RSVD1),
	PIN(DTD,  VI,    RSVD,   SDIO2,  VI,        RSVD,        RSVD1),
	PIN(DTE,  VI,    RSVD,   RSVD,   VI,        SPI1,        RSVD1),

	PINP(GPU, UART,  PWM,    UARTA,  GMI,       RSVD,        RSVD4,
		GPSLXAU),
	PIN(GPV,  SD,    PCIE,   RSVD,   RSVD,      RSVD,        PCIE),
	PIN(I2CP, SYS,   I2C,    RSVD,   RSVD,      RSVD,        RSVD4),
	PIN(IRTX, UART,  UARTA,  UARTB,  GMI,       SPI4,        UARTB),
	PIN(IRRX, UART,  UARTA,  UARTB,  GMI,       SPI4,        UARTB),
	PIN(KBCB, SYS,   KBC,    NAND,   SDIO2,     MIO,         KBC),
	PIN(KBCA, SYS,   KBC,    NAND,   SDIO2,     EMC_TEST0_DLL, KBC),
	PINP(PMC, SYS,   PWR_ON, PWR_INTR, RSVD,    RSVD,        PWR_ON, NONE),

	PIN(PTA,  NAND,  I2C2,   HDMI,   GMI,       RSVD,        RSVD4),
	PIN(RM,   UART,  I2C,    RSVD,   RSVD,      RSVD,        RSVD4),
	PIN(KBCE, SYS,   KBC,    NAND,   OWR,       RSVD,        KBC),
	PIN(KBCF, SYS,   KBC,    NAND,   TRACE,     MIO,         KBC),
	PIN(GMA,  NAND,  UARTE,  SPI3,   GMI,       SDIO4,       SPI3),
	PIN(GMC,  NAND,  UARTD,  SPI4,   GMI,       SFLASH,      SPI4),
	PIN(SDMMC1, BB,  SDIO1,  RSVD,   UARTE,     UARTA,       RSVD2),
	PIN(OWC,  SYS,   OWR,    RSVD,   RSVD,      RSVD,        OWR),

	PIN(GME,  NAND,  RSVD,   DAP5,   GMI,       SDIO4,       GMI),
	PIN(SDC,  SD,    PWM,    TWC,    SDIO3,     SPI3,        TWC),
	PIN(SDD,  SD,    UARTA,  PWM,    SDIO3,     SPI3,        PWM),
	PIN_RESERVED,
	PINP(SLXA, SD,   PCIE,   SPI4,   SDIO3,     SPI2,        PCIE, CRTP),
	PIN(SLXC, SD,    SPDIF,  SPI4,   SDIO3,     SPI2,        SPI4),
	PIN(SLXD, SD,    SPDIF,  SPI4,   SDIO3,     SPI2,        SPI4),
	PIN(SLXK, SD,    PCIE,   SPI4,   SDIO3,     SPI2,        PCIE),

	PIN(SPDI, AUDIO, SPDIF,  RSVD,   I2C,       SDIO2,       RSVD2),
	PIN(SPDO, AUDIO, SPDIF,  RSVD,   I2C,       SDIO2,       RSVD2),
	PIN(SPIA, AUDIO, SPI1,   SPI2,   SPI3,      GMI,         GMI),
	PIN(SPIB, AUDIO, SPI1,   SPI2,   SPI3,      GMI,         GMI),
	PIN(SPIC, AUDIO, SPI1,   SPI2,   SPI3,      GMI,         GMI),
	PIN(SPID, AUDIO, SPI2,   SPI1,   SPI2_ALT,  GMI,         GMI),
	PIN(SPIE, AUDIO, SPI2,   SPI1,   SPI2_ALT,  GMI,         GMI),
	PIN(SPIF, AUDIO, SPI3,   SPI1,   SPI2,      RSVD,        RSVD4),

	PIN(SPIG, AUDIO, SPI3,   SPI2,   SPI2_ALT,  I2C,         SPI2_ALT),
	PIN(SPIH, AUDIO, SPI3,   SPI2,   SPI2_ALT,  I2C,         SPI2_ALT),
	PIN(UAA,  BB,    SPI3,   MIPI_HS, UARTA,    ULPI,        MIPI_HS),
	PIN(UAB,  BB,    SPI2,   MIPI_HS, UARTA,    ULPI,        MIPI_HS),
	PIN(UAC,  BB,    OWR,    RSVD,   RSVD,      RSVD,        RSVD4),
	PIN(UAD,  UART,  IRDA,   SPDIF,  UARTA,     SPI4,        SPDIF),
	PIN(UCA,  UART,  UARTC,  RSVD,   GMI,       RSVD,        RSVD4),
	PIN(UCB,  UART,  UARTC,  PWM,    GMI,       RSVD,        RSVD4),

	PIN_RESERVED,
	PIN(ATE,  NAND,  IDE,    NAND,   GMI,       RSVD,        IDE),
	PIN(KBCC, SYS,   KBC,    NAND,   TRACE,     EMC_TEST1_DLL, KBC),
	PIN_RESERVED,
	PIN_RESERVED,
	PIN(GMB,  NAND,  IDE,    NAND,   GMI,       GMI_INT,     GMI),
	PIN(GMD,  NAND,  RSVD,   NAND,   GMI,       SFLASH,      GMI),
	PIN(DDC,  LCD,   I2C2,   RSVD,   RSVD,      RSVD,        RSVD4),

	/* 64 */
	PINP(LD0,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD1,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD2,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD3,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD4,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD5,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD6,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD7,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),

	PINP(LD8,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD9,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD10, LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD11, LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD12, LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD13, LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD14, LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD15, LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),

	PINP(LD16, LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LD17),
	PINP(LD17, LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LD17),
	PINP(LHP0, LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LD21_20),
	PINP(LHP1, LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LD19_18),
	PINP(LHP2, LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LD19_18),
	PINP(LVP0, LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LC),
	PINP(LVP1, LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LD21_20),
	PINP(HDINT, LCD, HDMI,   RSVD,   RSVD,      RSVD,     HDMI , LC),

	PINP(LM0,  LCD,  DISPA,  DISPB,  SPI3,      RSVD,     RSVD4, LC),
	PINP(LM1,  LCD,  DISPA,  DISPB,  RSVD,      CRT,      RSVD3, LC),
	PINP(LVS,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LC),
	PINP(LSC0, LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LC),
	PINP(LSC1, LCD,  DISPA,  DISPB,  SPI3,      HDMI,     DISPA, LS),
	PINP(LSCK, LCD,  DISPA,  DISPB,  SPI3,      HDMI,     DISPA, LS),
	PINP(LDC,  LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LS),
	PINP(LCSN, LCD,  DISPA,  DISPB,  SPI3,      RSVD,     RSVD4, LS),

	/* 96 */
	PINP(LSPI, LCD,  DISPA,  DISPB,  XIO,       HDMI,     DISPA, LC),
	PINP(LSDA, LCD,  DISPA,  DISPB,  SPI3,      HDMI,     DISPA, LS),
	PINP(LSDI, LCD,  DISPA,  DISPB,  SPI3,      RSVD,     DISPA, LS),
	PINP(LPW0, LCD,  DISPA,  DISPB,  SPI3,      HDMI,     DISPA, LS),
	PINP(LPW1, LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LS),
	PINP(LPW2, LCD,  DISPA,  DISPB,  SPI3,      HDMI,     DISPA, LS),
	PINP(LDI,  LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LD23_22),
	PINP(LHS,  LCD,  DISPA,  DISPB,  XIO,       RSVD,     RSVD4, LC),

	PINP(LPP,  LCD,  DISPA,  DISPB,  RSVD,      RSVD,     RSVD4, LD23_22),
	PIN_RESERVED,
	PIN(KBCD,  SYS,  KBC,    NAND,   SDIO2,     MIO,      KBC),
	PIN(GPU7,  SYS,  RTCK,   RSVD,   RSVD,      RSVD,     RTCK),
	PIN(DTF,   VI,   I2C3,   RSVD,   VI,        RSVD,     RSVD4),
	PIN(UDA,   BB,   SPI1,   RSVD,   UARTD,     ULPI,     RSVD2),
	PIN(CRTP,  LCD,  CRT,    RSVD,   RSVD,      RSVD,     RSVD),
	PINP(SDB,  SD,   UARTA,  PWM,    SDIO3,     SPI2,     PWM,   NONE),

	/* these pin groups only have pullup and pull down control */
	PINALL(CK32,  SYS,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
	PINALL(DDRC,  DDR,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
	PINALL(PMCA,  SYS,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
	PINALL(PMCB,  SYS,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
	PINALL(PMCC,  SYS,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
	PINALL(PMCD,  SYS,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
	PINALL(PMCE,  SYS,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
	PINALL(XM2C,  DDR,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
	PINALL(XM2D,  DDR,   RSVD, RSVD, RSVD, RSVD,  RSVD, MUXCTL_NONE,
		PUCTL_NONE),
};

void pinmux_set_tristate(enum pmux_pingrp pin, int enable)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *tri = &pmt->pmt_tri[TRISTATE_REG(pin)];
	u32 reg;

	reg = readl(tri);
	if (enable)
		reg |= TRISTATE_MASK(pin);
	else
		reg &= ~TRISTATE_MASK(pin);
	writel(reg, tri);
}

void pinmux_tristate_enable(enum pmux_pingrp pin)
{
	pinmux_set_tristate(pin, 1);
}

void pinmux_tristate_disable(enum pmux_pingrp pin)
{
	pinmux_set_tristate(pin, 0);
}

void pinmux_set_pullupdown(enum pmux_pingrp pin, enum pmux_pull pupd)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	enum pmux_pullid pull_id = tegra_soc_pingroups[pin].pull_id;
	u32 *pull = &pmt->pmt_pull[PULL_REG(pull_id)];
	u32 mask_bit;
	u32 reg;
	mask_bit = PULL_SHIFT(pull_id);

	reg = readl(pull);
	reg &= ~(0x3 << mask_bit);
	reg |= pupd << mask_bit;
	writel(reg, pull);
}

void pinmux_set_func(enum pmux_pingrp pin, enum pmux_func func)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	enum pmux_ctlid mux_id = tegra_soc_pingroups[pin].ctl_id;
	u32 *muxctl = &pmt->pmt_ctl[MUXCTL_REG(mux_id)];
	u32 mask_bit;
	int i, mux = -1;
	u32 reg;

	assert(pmux_func_isvalid(func));

	/* Handle special values */
	if (func >= PMUX_FUNC_RSVD1) {
		mux = (func - PMUX_FUNC_RSVD1) & 0x3;
	} else {
		/* Search for the appropriate function */
		for (i = 0; i < 4; i++) {
			if (tegra_soc_pingroups[pin].funcs[i] == func) {
				mux = i;
				break;
			}
		}
	}
	assert(mux != -1);

	mask_bit = MUXCTL_SHIFT(mux_id);
	reg = readl(muxctl);
	reg &= ~(0x3 << mask_bit);
	reg |= mux << mask_bit;
	writel(reg, muxctl);
}

void pinmux_config_pingroup(struct pingroup_config *config)
{
	enum pmux_pingrp pin = config->pingroup;

	pinmux_set_func(pin, config->func);
	pinmux_set_pullupdown(pin, config->pull);
	pinmux_set_tristate(pin, config->tristate);
}

void pinmux_config_table(struct pingroup_config *config, int len)
{
	int i;

	for (i = 0; i < len; i++)
		pinmux_config_pingroup(&config[i]);
}
