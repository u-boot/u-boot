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

/* Tegra30 pin multiplexing functions */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/tegra.h>
#include <asm/arch/pinmux.h>

struct tegra_pingroup_desc {
	const char *name;
	enum pmux_func funcs[4];
	enum pmux_func func_safe;
	enum pmux_vddio vddio;
	enum pmux_pin_io io;
};

#define PMUX_MUXCTL_SHIFT	0
#define PMUX_PULL_SHIFT		2
#define PMUX_TRISTATE_SHIFT	4
#define PMUX_TRISTATE_MASK	(1 << PMUX_TRISTATE_SHIFT)
#define PMUX_IO_SHIFT		5
#define PMUX_OD_SHIFT		6
#define PMUX_LOCK_SHIFT		7
#define PMUX_IO_RESET_SHIFT	8

#define PGRP_HSM_SHIFT		2
#define PGRP_SCHMT_SHIFT	3
#define PGRP_LPMD_SHIFT		4
#define PGRP_LPMD_MASK		(3 << PGRP_LPMD_SHIFT)
#define PGRP_DRVDN_SHIFT	12
#define PGRP_DRVDN_MASK		(0x7F << PGRP_DRVDN_SHIFT)
#define PGRP_DRVUP_SHIFT	20
#define PGRP_DRVUP_MASK		(0x7F << PGRP_DRVUP_SHIFT)
#define PGRP_SLWR_SHIFT		28
#define PGRP_SLWR_MASK		(3 << PGRP_SLWR_SHIFT)
#define PGRP_SLWF_SHIFT		30
#define PGRP_SLWF_MASK		(3 << PGRP_SLWF_SHIFT)

/* Convenient macro for defining pin group properties */
#define PIN(pg_name, vdd, f0, f1, f2, f3, iod)	\
	{						\
		.vddio = PMUX_VDDIO_ ## vdd,		\
		.funcs = {				\
			PMUX_FUNC_ ## f0,		\
			PMUX_FUNC_ ## f1,		\
			PMUX_FUNC_ ## f2,		\
			PMUX_FUNC_ ## f3,		\
		},					\
		.func_safe = PMUX_FUNC_RSVD1,		\
		.io = PMUX_PIN_ ## iod,			\
	}

/* Input and output pins */
#define PINI(pg_name, vdd, f0, f1, f2, f3) \
	PIN(pg_name, vdd, f0, f1, f2, f3, INPUT)
#define PINO(pg_name, vdd, f0, f1, f2, f3) \
	PIN(pg_name, vdd, f0, f1, f2, f3, OUTPUT)

const struct tegra_pingroup_desc tegra_soc_pingroups[PINGRP_COUNT] = {
	/*	NAME	  VDD	   f0		f1	   f2	    f3  */
	PINI(ULPI_DATA0,  BB,	   SPI3,	HSI,	   UARTA,   ULPI),
	PINI(ULPI_DATA1,  BB,	   SPI3,	HSI,	   UARTA,   ULPI),
	PINI(ULPI_DATA2,  BB,	   SPI3,	HSI,	   UARTA,   ULPI),
	PINI(ULPI_DATA3,  BB,	   SPI3,	HSI,	   UARTA,   ULPI),
	PINI(ULPI_DATA4,  BB,	   SPI2,	HSI,	   UARTA,   ULPI),
	PINI(ULPI_DATA5,  BB,	   SPI2,	HSI,	   UARTA,   ULPI),
	PINI(ULPI_DATA6,  BB,	   SPI2,	HSI,	   UARTA,   ULPI),
	PINI(ULPI_DATA7,  BB,	   SPI2,	HSI,	   UARTA,   ULPI),
	PINI(ULPI_CLK,	  BB,	   SPI1,	RSVD2,	   UARTD,   ULPI),
	PINI(ULPI_DIR,	  BB,	   SPI1,	RSVD2,	   UARTD,   ULPI),
	PINI(ULPI_NXT,	  BB,	   SPI1,	RSVD2,	   UARTD,   ULPI),
	PINI(ULPI_STP,	  BB,	   SPI1,	RSVD2,	   UARTD,   ULPI),
	PINI(DAP3_FS,	  BB,	   I2S2,	RSVD2,	   DISPA,   DISPB),
	PINI(DAP3_DIN,	  BB,	   I2S2,	RSVD2,	   DISPA,   DISPB),
	PINI(DAP3_DOUT,	  BB,	   I2S2,	RSVD2,	   DISPA,   DISPB),
	PINI(DAP3_SCLK,	  BB,	   I2S2,	RSVD2,	   DISPA,   DISPB),
	PINI(GPIO_PV0,	  BB,	   RSVD1,	RSVD2,	   RSVD3,   RSVD4),
	PINI(GPIO_PV1,	  BB,	   RSVD1,	RSVD2,	   RSVD3,   RSVD4),
	PINI(SDMMC1_CLK,  SDMMC1,  SDMMC1,	RSVD2,	   RSVD3,   UARTA),
	PINI(SDMMC1_CMD,  SDMMC1,  SDMMC1,	RSVD2,	   RSVD3,   UARTA),
	PINI(SDMMC1_DAT3, SDMMC1,  SDMMC1,	RSVD2,	   UARTE,   UARTA),
	PINI(SDMMC1_DAT2, SDMMC1,  SDMMC1,	RSVD2,	   UARTE,   UARTA),
	PINI(SDMMC1_DAT1, SDMMC1,  SDMMC1,	RSVD2,	   UARTE,   UARTA),
	PINI(SDMMC1_DAT0, SDMMC1,  SDMMC1,	RSVD2,	   UARTE,   UARTA),
	PINI(GPIO_PV2,	  SDMMC1,  OWR,		RSVD2,	   RSVD3,   RSVD4),
	PINI(GPIO_PV3,	  SDMMC1,  CLK_12M_OUT,	RSVD2,	   RSVD3,   RSVD4),
	PINI(CLK2_OUT,	  SDMMC1,  EXTPERIPH2,	RSVD2,     RSVD3,   RSVD4),
	PINI(CLK2_REQ,	  SDMMC1,  DAP,		RSVD2,	   RSVD3,   RSVD4),
	PINO(LCD_PWR1,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_PWR2,	  LCD,	   DISPA,	DISPB,	   SPI5,    HDCP),
	PINO(LCD_SDIN,	  LCD,	   DISPA,	DISPB,	   SPI5,    RSVD4),
	PINO(LCD_SDOUT,	  LCD,	   DISPA,	DISPB,	   SPI5,    HDCP),
	PINO(LCD_WR_N,	  LCD,	   DISPA,	DISPB,	   SPI5,    HDCP),
	PINO(LCD_CS0_N,	  LCD,	   DISPA,	DISPB,	   SPI5,    RSVD4),
	PINO(LCD_DC0,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_SCK,	  LCD,	   DISPA,	DISPB,	   SPI5,    HDCP),
	PINO(LCD_PWR0,	  LCD,	   DISPA,	DISPB,	   SPI5,    HDCP),
	PINO(LCD_PCLK,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_DE,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_HSYNC,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_VSYNC,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D0,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D1,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D2,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D3,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D4,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D5,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D6,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D7,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D8,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D9,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D10,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D11,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D12,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D13,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D14,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D15,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D16,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D17,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D18,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D19,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D20,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D21,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D22,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_D23,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_CS1_N,	  LCD,	   DISPA,	DISPB,	   SPI5,    RSVD4),
	PINO(LCD_M1,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINO(LCD_DC1,	  LCD,	   DISPA,	DISPB,	   RSVD3,   RSVD4),
	PINI(HDMI_INT,	  LCD,	   HDMI,	RSVD2,	   RSVD3,   RSVD4),
	PINI(DDC_SCL,	  LCD,	   I2C4,	RSVD2,	   RSVD3,   RSVD4),
	PINI(DDC_SDA,	  LCD,	   I2C4,	RSVD2,	   RSVD3,   RSVD4),
	PINI(CRT_HSYNC,	  LCD,	   CRT,		RSVD2,	   RSVD3,   RSVD4),
	PINI(CRT_VSYNC,	  LCD,	   CRT,		RSVD2,	   RSVD3,   RSVD4),
	PINI(VI_D0,	  VI,	   DDR,		RSVD2,	   VI,      RSVD4),
	PINI(VI_D1,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D2,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D3,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D4,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D5,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D6,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D7,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D8,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D9,	  VI,	   DDR,		SDMMC2,	   VI,      RSVD4),
	PINI(VI_D10,	  VI,	   DDR,		RSVD2,	   VI,      RSVD4),
	PINI(VI_D11,	  VI,	   DDR,		RSVD2,	   VI,      RSVD4),
	PINI(VI_PCLK,	  VI,	   RSVD1,	SDMMC2,	   VI,      RSVD4),
	PINI(VI_MCLK,	  VI,	   VI,		VI,	   VI,      VI),
	PINI(VI_VSYNC,	  VI,	   DDR,		RSVD2,	   VI,      RSVD4),
	PINI(VI_HSYNC,	  VI,	   DDR,		RSVD2,	   VI,      RSVD4),
	PINI(UART2_RXD,	  UART,	   UARTB,	SPDIF,	   UARTA,   SPI4),
	PINI(UART2_TXD,	  UART,	   UARTB,	SPDIF,	   UARTA,   SPI4),
	PINI(UART2_RTS_N, UART,	   UARTA,	UARTB,	   GMI,     SPI4),
	PINI(UART2_CTS_N, UART,	   UARTA,	UARTB,	   GMI,     SPI4),
	PINI(UART3_TXD,	  UART,	   UARTC,	RSVD2,	   GMI,     RSVD4),
	PINI(UART3_RXD,	  UART,	   UARTC,	RSVD2,	   GMI,     RSVD4),
	PINI(UART3_CTS_N, UART,	   UARTC,	RSVD2,	   GMI,     RSVD4),
	PINI(UART3_RTS_N, UART,	   UARTC,	PWM0,	   GMI,     RSVD4),
	PINI(GPIO_PU0,	  UART,	   OWR,		UARTA,	   GMI,     RSVD4),
	PINI(GPIO_PU1,	  UART,	   RSVD1,	UARTA,	   GMI,     RSVD4),
	PINI(GPIO_PU2,	  UART,	   RSVD1,	UARTA,	   GMI,     RSVD4),
	PINI(GPIO_PU3,	  UART,	   PWM0,	UARTA,	   GMI,     RSVD4),
	PINI(GPIO_PU4,	  UART,	   PWM1,	UARTA,	   GMI,     RSVD4),
	PINI(GPIO_PU5,	  UART,	   PWM2,	UARTA,	   GMI,     RSVD4),
	PINI(GPIO_PU6,	  UART,	   PWM3,	UARTA,	   GMI,     RSVD4),
	PINI(GEN1_I2C_SDA, UART,   I2C1,	RSVD2,	   RSVD3,   RSVD4),
	PINI(GEN1_I2C_SCL, UART,   I2C1,	RSVD2,	   RSVD3,   RSVD4),
	PINI(DAP4_FS,	  UART,	   I2S3,	RSVD2,	   GMI,     RSVD4),
	PINI(DAP4_DIN,	  UART,	   I2S3,	RSVD2,	   GMI,     RSVD4),
	PINI(DAP4_DOUT,	  UART,	   I2S3,	RSVD2,	   GMI,     RSVD4),
	PINI(DAP4_SCLK,	  UART,	   I2S3,	RSVD2,	   GMI,     RSVD4),
	PINI(CLK3_OUT,	  UART,	   EXTPERIPH3,	RSVD2,	   RSVD3,   RSVD4),
	PINI(CLK3_REQ,	  UART,	   DEV3,	RSVD2,	   RSVD3,   RSVD4),
	PINI(GMI_WP_N,	  GMI,	   RSVD1,	NAND,	   GMI,     GMI_ALT),
	PINI(GMI_IORDY,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_WAIT,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_ADV_N,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_CLK,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_CS0_N,	  GMI,	   RSVD1,	NAND,	   GMI,     DTV),
	PINI(GMI_CS1_N,	  GMI,	   RSVD1,	NAND,	   GMI,     DTV),
	PINI(GMI_CS2_N,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_CS3_N,	  GMI,	   RSVD1,	NAND,	   GMI,     GMI_ALT),
	PINI(GMI_CS4_N,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_CS6_N,	  GMI,	   NAND,	NAND_ALT,  GMI,     SATA),
	PINI(GMI_CS7_N,	  GMI,	   NAND,	NAND_ALT,  GMI,     GMI_ALT),
	PINI(GMI_AD0,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD1,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD2,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD3,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD4,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD5,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD6,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD7,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD8,	  GMI,	   PWM0,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD9,	  GMI,	   PWM1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD10,	  GMI,	   PWM2,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD11,	  GMI,	   PWM3,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD12,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD13,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD14,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_AD15,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_A16,	  GMI,	   UARTD,	SPI4,	   GMI,     GMI_ALT),
	PINI(GMI_A17,	  GMI,	   UARTD,	SPI4,	   GMI,     DTV),
	PINI(GMI_A18,	  GMI,	   UARTD,	SPI4,	   GMI,     DTV),
	PINI(GMI_A19,	  GMI,	   UARTD,	SPI4,	   GMI,     RSVD4),
	PINI(GMI_WR_N,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_OE_N,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_DQS,	  GMI,	   RSVD1,	NAND,	   GMI,     RSVD4),
	PINI(GMI_RST_N,	  GMI,	   NAND,	NAND_ALT,  GMI,     RSVD4),
	PINI(GEN2_I2C_SCL, GMI,	   I2C2,	HDCP,	   GMI,     RSVD4),
	PINI(GEN2_I2C_SDA, GMI,    I2C2,	HDCP,	   GMI,     RSVD4),
	PINI(SDMMC4_CLK,  SDMMC4,   RSVD1,	NAND,	   GMI,     SDMMC4),
	PINI(SDMMC4_CMD,  SDMMC4,   I2C3,	NAND,	   GMI,     SDMMC4),
	PINI(SDMMC4_DAT0, SDMMC4,   UARTE,	SPI3,	   GMI,     SDMMC4),
	PINI(SDMMC4_DAT1, SDMMC4,   UARTE,	SPI3,	   GMI,     SDMMC4),
	PINI(SDMMC4_DAT2, SDMMC4,   UARTE,	SPI3,	   GMI,     SDMMC4),
	PINI(SDMMC4_DAT3, SDMMC4,   UARTE,	SPI3,	   GMI,     SDMMC4),
	PINI(SDMMC4_DAT4, SDMMC4,   I2C3,	I2S4,	   GMI,     SDMMC4),
	PINI(SDMMC4_DAT5, SDMMC4,   VGP3,	I2S4,	   GMI,     SDMMC4),
	PINI(SDMMC4_DAT6, SDMMC4,   VGP4,	I2S4,	   GMI,     SDMMC4),
	PINI(SDMMC4_DAT7, SDMMC4,   VGP5,	I2S4,	   GMI,     SDMMC4),
	PINI(SDMMC4_RST_N, SDMMC4,  VGP6,	RSVD2,	   RSVD3,   SDMMC4),
	PINI(CAM_MCLK,	  CAM,	   VI,		RSVD2,	   VI_ALT2, SDMMC4),
	PINI(GPIO_PCC1,	  CAM,	   I2S4,	RSVD2,	   RSVD3,   SDMMC4),
	PINI(GPIO_PBB0,	  CAM,	   I2S4,	RSVD2,	   RSVD3,   SDMMC4),
	PINI(CAM_I2C_SCL, CAM,	   VGP1,	I2C3,	   RSVD3,   SDMMC4),
	PINI(CAM_I2C_SDA, CAM,	   VGP2,	I2C3,	   RSVD3,   SDMMC4),
	PINI(GPIO_PBB3,	  CAM,	   VGP3,	DISPA,	   DISPB,   SDMMC4),
	PINI(GPIO_PBB4,	  CAM,	   VGP4,	DISPA,	   DISPB,   SDMMC4),
	PINI(GPIO_PBB5,	  CAM,	   VGP5,	DISPA,	   DISPB,   SDMMC4),
	PINI(GPIO_PBB6,	  CAM,	   VGP6,	DISPA,	   DISPB,   SDMMC4),
	PINI(GPIO_PBB7,	  CAM,	   I2S4,	RSVD2,	   RSVD3,   SDMMC4),
	PINI(GPIO_PCC2,	  CAM,	   I2S4,	RSVD2,	   RSVD3,   RSVD4),
	PINI(JTAG_RTCK,	  SYS,	   RTCK,	RSVD2,	   RSVD3,   RSVD4),
	PINI(PWR_I2C_SCL, SYS,	   I2CPWR,	RSVD2,	   RSVD3,   RSVD4),
	PINI(PWR_I2C_SDA, SYS,	   I2CPWR,	RSVD2,	   RSVD3,   RSVD4),
	PINI(KB_ROW0,	  SYS,	   KBC,		NAND,	   RSVD3,   RSVD4),
	PINI(KB_ROW1,	  SYS,	   KBC,		NAND,	   RSVD3,   RSVD4),
	PINI(KB_ROW2,	  SYS,	   KBC,		NAND,	   RSVD3,   RSVD4),
	PINI(KB_ROW3,	  SYS,	   KBC,		NAND,	   RSVD3,   RSVD4),
	PINI(KB_ROW4,	  SYS,	   KBC,		NAND,	   TRACE,   RSVD4),
	PINI(KB_ROW5,	  SYS,	   KBC,		NAND,	   TRACE,   OWR),
	PINI(KB_ROW6,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW7,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW8,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW9,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW10,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW11,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW12,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW13,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW14,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_ROW15,	  SYS,	   KBC,		NAND,	   SDMMC2,  MIO),
	PINI(KB_COL0,	  SYS,	   KBC,		NAND,	   TRACE,   TEST),
	PINI(KB_COL1,	  SYS,	   KBC,		NAND,	   TRACE,   TEST),
	PINI(KB_COL2,	  SYS,	   KBC,		NAND,	   TRACE,   RSVD4),
	PINI(KB_COL3,	  SYS,	   KBC,		NAND,	   TRACE,   RSVD4),
	PINI(KB_COL4,	  SYS,	   KBC,		NAND,	   TRACE,   RSVD4),
	PINI(KB_COL5,	  SYS,	   KBC,		NAND,	   TRACE,   RSVD4),
	PINI(KB_COL6,	  SYS,	   KBC,		NAND,	   TRACE,   MIO),
	PINI(KB_COL7,	  SYS,	   KBC,		NAND,	   TRACE,   MIO),
	PINI(CLK_32K_OUT, SYS,	   BLINK,	RSVD2,	   RSVD3,   RSVD4),
	PINI(SYS_CLK_REQ, SYS,	   SYSCLK,	RSVD2,	   RSVD3,   RSVD4),
	PINI(CORE_PWR_REQ, SYS,	   CORE_PWR_REQ, RSVD2,	   RSVD3,   RSVD4),
	PINI(CPU_PWR_REQ, SYS,	   CPU_PWR_REQ,	RSVD2,	   RSVD3,   RSVD4),
	PINI(PWR_INT_N,	  SYS,	   PWR_INT_N,	RSVD2,	   RSVD3,   RSVD4),
	PINI(CLK_32K_IN,  SYS,	   CLK_32K_IN,	RSVD2,	   RSVD3,   RSVD4),
	PINI(OWR,	  SYS,	   OWR,		CEC,	   RSVD3,   RSVD4),
	PINI(DAP1_FS,	  AUDIO,   I2S0,	HDA,	   GMI,     SDMMC2),
	PINI(DAP1_DIN,	  AUDIO,   I2S0,	HDA,	   GMI,     SDMMC2),
	PINI(DAP1_DOUT,	  AUDIO,   I2S0,	HDA,	   GMI,     SDMMC2),
	PINI(DAP1_SCLK,	  AUDIO,   I2S0,	HDA,	   GMI,     SDMMC2),
	PINI(CLK1_REQ,	  AUDIO,   DAP,		HDA,	   RSVD3,   RSVD4),
	PINI(CLK1_OUT,	  AUDIO,   EXTPERIPH1,	RSVD2,	   RSVD3,   RSVD4),
	PINI(SPDIF_IN,	  AUDIO,   SPDIF,	HDA,	   I2C1,    SDMMC2),
	PINI(SPDIF_OUT,	  AUDIO,   SPDIF,	RSVD2,	   I2C1,    SDMMC2),
	PINI(DAP2_FS,	  AUDIO,   I2S1,	HDA,	   RSVD3,   GMI),
	PINI(DAP2_DIN,	  AUDIO,   I2S1,	HDA,	   RSVD3,   GMI),
	PINI(DAP2_DOUT,	  AUDIO,   I2S1,	HDA,	   RSVD3,   GMI),
	PINI(DAP2_SCLK,	  AUDIO,   I2S1,	HDA,	   RSVD3,   GMI),
	PINI(SPI2_MOSI,	  AUDIO,   SPI6,	SPI2,	   GMI,     GMI),
	PINI(SPI2_MISO,	  AUDIO,   SPI6,	SPI2,	   GMI,     GMI),
	PINI(SPI2_CS0_N,  AUDIO,   SPI6,	SPI2,	   GMI,     GMI),
	PINI(SPI2_SCK,	  AUDIO,   SPI6,	SPI2,	   GMI,     GMI),
	PINI(SPI1_MOSI,	  AUDIO,   SPI2,	SPI1,	   SPI2_ALT, GMI),
	PINI(SPI1_SCK,	  AUDIO,   SPI2,	SPI1,	   SPI2_ALT, GMI),
	PINI(SPI1_CS0_N,  AUDIO,   SPI2,	SPI1,	   SPI2_ALT, GMI),
	PINI(SPI1_MISO,	  AUDIO,   SPI3,	SPI1,	   SPI2_ALT, RSVD4),
	PINI(SPI2_CS1_N,  AUDIO,   SPI3,	SPI2,	   SPI2_ALT, I2C1),
	PINI(SPI2_CS2_N,  AUDIO,   SPI3,	SPI2,	   SPI2_ALT, I2C1),
	PINI(SDMMC3_CLK,  SDMMC3,  UARTA,	PWM2,	   SDMMC3,  SPI3),
	PINI(SDMMC3_CMD,  SDMMC3,  UARTA,	PWM3,	   SDMMC3,  SPI2),
	PINI(SDMMC3_DAT0, SDMMC3,  RSVD1,	RSVD2,	   SDMMC3,  SPI3),
	PINI(SDMMC3_DAT1, SDMMC3,  RSVD1,	RSVD2,	   SDMMC3,  SPI3),
	PINI(SDMMC3_DAT2, SDMMC3,  RSVD1,	PWM1,	   SDMMC3,  SPI3),
	PINI(SDMMC3_DAT3, SDMMC3,  RSVD1,	PWM0,	   SDMMC3,  SPI3),
	PINI(SDMMC3_DAT4, SDMMC3,  PWM1,	SPI4,	   SDMMC3,  SPI2),
	PINI(SDMMC3_DAT5, SDMMC3,  PWM0,	SPI4,	   SDMMC3,  SPI2),
	PINI(SDMMC3_DAT6, SDMMC3,  SPDIF,	SPI4,	   SDMMC3,  SPI2),
	PINI(SDMMC3_DAT7, SDMMC3,  SPDIF,	SPI4,	   SDMMC3,  SPI2),
	PINI(PEX_L0_PRSNT_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_L0_RST_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_L0_CLKREQ_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_WAKE_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_L1_PRSNT_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_L1_RST_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_L1_CLKREQ_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_L2_PRSNT_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_L2_RST_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(PEX_L2_CLKREQ_N,	PEXCTL,   PCIE,	HDA,	   RSVD3,   RSVD4),
	PINI(HDMI_CEC,		SYS,      CEC,	RSVD2,	   RSVD3,   RSVD4),
};

void pinmux_set_tristate(enum pmux_pingrp pin, int enable)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *tri = &pmt->pmt_ctl[pin];
	u32 reg;

	/* Error check on pin */
	assert(pmux_pingrp_isvalid(pin));

	reg = readl(tri);
	if (enable)
		reg |= PMUX_TRISTATE_MASK;
	else
		reg &= ~PMUX_TRISTATE_MASK;
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
	u32 *pull = &pmt->pmt_ctl[pin];
	u32 reg;

	/* Error check on pin and pupd */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_pupd_isvalid(pupd));

	reg = readl(pull);
	reg &= ~(0x3 << PMUX_PULL_SHIFT);
	reg |= (pupd << PMUX_PULL_SHIFT);
	writel(reg, pull);
}

void pinmux_set_func(enum pmux_pingrp pin, enum pmux_func func)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *muxctl = &pmt->pmt_ctl[pin];
	int i, mux = -1;
	u32 reg;

	/* Error check on pin and func */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_func_isvalid(func));

	/* Handle special values */
	if (func == PMUX_FUNC_SAFE)
		func = tegra_soc_pingroups[pin].func_safe;

	if (func & PMUX_FUNC_RSVD1) {
		mux = func & 0x3;
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

	reg = readl(muxctl);
	reg &= ~(0x3 << PMUX_MUXCTL_SHIFT);
	reg |= (mux << PMUX_MUXCTL_SHIFT);
	writel(reg, muxctl);

}

void pinmux_set_io(enum pmux_pingrp pin, enum pmux_pin_io io)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pin_io = &pmt->pmt_ctl[pin];
	u32 reg;

	/* Error check on pin and io */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_io_isvalid(io));

	reg = readl(pin_io);
	reg &= ~(0x1 << PMUX_IO_SHIFT);
	reg |= (io & 0x1) << PMUX_IO_SHIFT;
	writel(reg, pin_io);
}

static int pinmux_set_lock(enum pmux_pingrp pin, enum pmux_pin_lock lock)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pin_lock = &pmt->pmt_ctl[pin];
	u32 reg;

	/* Error check on pin and lock */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_lock_isvalid(lock));

	if (lock == PMUX_PIN_LOCK_DEFAULT)
		return 0;

	reg = readl(pin_lock);
	reg &= ~(0x1 << PMUX_LOCK_SHIFT);
	if (lock == PMUX_PIN_LOCK_ENABLE)
		reg |= (0x1 << PMUX_LOCK_SHIFT);
	else {
		/* lock == DISABLE, which isn't possible */
		printf("%s: Warning: lock == %d, DISABLE is not allowed!\n",
			__func__, lock);
	}
	writel(reg, pin_lock);

	return 0;
}

static int pinmux_set_od(enum pmux_pingrp pin, enum pmux_pin_od od)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pin_od = &pmt->pmt_ctl[pin];
	u32 reg;

	/* Error check on pin and od */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_od_isvalid(od));

	if (od == PMUX_PIN_OD_DEFAULT)
		return 0;

	reg = readl(pin_od);
	reg &= ~(0x1 << PMUX_OD_SHIFT);
	if (od == PMUX_PIN_OD_ENABLE)
		reg |= (0x1 << PMUX_OD_SHIFT);
	writel(reg, pin_od);

	return 0;
}

static int pinmux_set_ioreset(enum pmux_pingrp pin,
				enum pmux_pin_ioreset ioreset)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pin_ioreset = &pmt->pmt_ctl[pin];
	u32 reg;

	/* Error check on pin and ioreset */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_ioreset_isvalid(ioreset));

	if (ioreset == PMUX_PIN_IO_RESET_DEFAULT)
		return 0;

	reg = readl(pin_ioreset);
	reg &= ~(0x1 << PMUX_IO_RESET_SHIFT);
	if (ioreset == PMUX_PIN_IO_RESET_ENABLE)
		reg |= (0x1 << PMUX_IO_RESET_SHIFT);
	writel(reg, pin_ioreset);

	return 0;
}

void pinmux_config_pingroup(struct pingroup_config *config)
{
	enum pmux_pingrp pin = config->pingroup;

	pinmux_set_func(pin, config->func);
	pinmux_set_pullupdown(pin, config->pull);
	pinmux_set_tristate(pin, config->tristate);
	pinmux_set_io(pin, config->io);
	pinmux_set_lock(pin, config->lock);
	pinmux_set_od(pin, config->od);
	pinmux_set_ioreset(pin, config->ioreset);
}

void pinmux_config_table(struct pingroup_config *config, int len)
{
	int i;

	for (i = 0; i < len; i++)
		pinmux_config_pingroup(&config[i]);
}

static int padgrp_set_drvup_slwf(enum pdrive_pingrp pad,
				int slwf)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pad_slwf = &pmt->pmt_drive[pad];
	u32 reg;

	/* Error check on pad and slwf */
	assert(pmux_padgrp_isvalid(pad));
	assert(pmux_pad_slw_isvalid(slwf));

	/* NONE means unspecified/do not change/use POR value */
	if (slwf == PGRP_SLWF_NONE)
		return 0;

	reg = readl(pad_slwf);
	reg &= ~PGRP_SLWF_MASK;
	reg |= (slwf << PGRP_SLWF_SHIFT);
	writel(reg, pad_slwf);

	return 0;
}

static int padgrp_set_drvdn_slwr(enum pdrive_pingrp pad, int slwr)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pad_slwr = &pmt->pmt_drive[pad];
	u32 reg;

	/* Error check on pad and slwr */
	assert(pmux_padgrp_isvalid(pad));
	assert(pmux_pad_slw_isvalid(slwr));

	/* NONE means unspecified/do not change/use POR value */
	if (slwr == PGRP_SLWR_NONE)
		return 0;

	reg = readl(pad_slwr);
	reg &= ~PGRP_SLWR_MASK;
	reg |= (slwr << PGRP_SLWR_SHIFT);
	writel(reg, pad_slwr);

	return 0;
}

static int padgrp_set_drvup(enum pdrive_pingrp pad, int drvup)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pad_drvup = &pmt->pmt_drive[pad];
	u32 reg;

	/* Error check on pad and drvup */
	assert(pmux_padgrp_isvalid(pad));
	assert(pmux_pad_drv_isvalid(drvup));

	/* NONE means unspecified/do not change/use POR value */
	if (drvup == PGRP_DRVUP_NONE)
		return 0;

	reg = readl(pad_drvup);
	reg &= ~PGRP_DRVUP_MASK;
	reg |= (drvup << PGRP_DRVUP_SHIFT);
	writel(reg, pad_drvup);

	return 0;
}

static int padgrp_set_drvdn(enum pdrive_pingrp pad, int drvdn)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pad_drvdn = &pmt->pmt_drive[pad];
	u32 reg;

	/* Error check on pad and drvdn */
	assert(pmux_padgrp_isvalid(pad));
	assert(pmux_pad_drv_isvalid(drvdn));

	/* NONE means unspecified/do not change/use POR value */
	if (drvdn == PGRP_DRVDN_NONE)
		return 0;

	reg = readl(pad_drvdn);
	reg &= ~PGRP_DRVDN_MASK;
	reg |= (drvdn << PGRP_DRVDN_SHIFT);
	writel(reg, pad_drvdn);

	return 0;
}

static int padgrp_set_lpmd(enum pdrive_pingrp pad, enum pgrp_lpmd lpmd)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pad_lpmd = &pmt->pmt_drive[pad];
	u32 reg;

	/* Error check pad and lpmd value */
	assert(pmux_padgrp_isvalid(pad));
	assert(pmux_pad_lpmd_isvalid(lpmd));

	/* NONE means unspecified/do not change/use POR value */
	if (lpmd == PGRP_LPMD_NONE)
		return 0;

	reg = readl(pad_lpmd);
	reg &= ~PGRP_LPMD_MASK;
	reg |= (lpmd << PGRP_LPMD_SHIFT);
	writel(reg, pad_lpmd);

	return 0;
}

static int padgrp_set_schmt(enum pdrive_pingrp pad, enum pgrp_schmt schmt)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pad_schmt = &pmt->pmt_drive[pad];
	u32 reg;

	/* Error check pad */
	assert(pmux_padgrp_isvalid(pad));

	reg = readl(pad_schmt);
	reg &= ~(1 << PGRP_SCHMT_SHIFT);
	if (schmt == PGRP_SCHMT_ENABLE)
		reg |= (0x1 << PGRP_SCHMT_SHIFT);
	writel(reg, pad_schmt);

	return 0;
}
static int padgrp_set_hsm(enum pdrive_pingrp pad,
			enum pgrp_hsm hsm)
{
	struct pmux_tri_ctlr *pmt =
			(struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *pad_hsm = &pmt->pmt_drive[pad];
	u32 reg;

	/* Error check pad */
	assert(pmux_padgrp_isvalid(pad));

	reg = readl(pad_hsm);
	reg &= ~(1 << PGRP_HSM_SHIFT);
	if (hsm == PGRP_HSM_ENABLE)
		reg |= (0x1 << PGRP_HSM_SHIFT);
	writel(reg, pad_hsm);

	return 0;
}

void padctrl_config_pingroup(struct padctrl_config *config)
{
	enum pdrive_pingrp pad = config->padgrp;

	padgrp_set_drvup_slwf(pad, config->slwf);
	padgrp_set_drvdn_slwr(pad, config->slwr);
	padgrp_set_drvup(pad, config->drvup);
	padgrp_set_drvdn(pad, config->drvdn);
	padgrp_set_lpmd(pad, config->lpmd);
	padgrp_set_schmt(pad, config->schmt);
	padgrp_set_hsm(pad, config->hsm);
}

void padgrp_config_table(struct padctrl_config *config, int len)
{
	int i;

	for (i = 0; i < len; i++)
		padctrl_config_pingroup(&config[i]);
}
