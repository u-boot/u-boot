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

#ifndef _TEGRA30_PINMUX_H_
#define _TEGRA30_PINMUX_H_

/*
 * Pin groups which we adjust. There are three basic attributes of each pin
 * group which use this enum:
 *
 *	- function
 *	- pullup / pulldown
 *	- tristate or normal
 */
enum pmux_pingrp {
	PINGRP_ULPI_DATA0 = 0,  /* offset 0x3000 */
	PINGRP_ULPI_DATA1,
	PINGRP_ULPI_DATA2,
	PINGRP_ULPI_DATA3,
	PINGRP_ULPI_DATA4,
	PINGRP_ULPI_DATA5,
	PINGRP_ULPI_DATA6,
	PINGRP_ULPI_DATA7,
	PINGRP_ULPI_CLK,
	PINGRP_ULPI_DIR,
	PINGRP_ULPI_NXT,
	PINGRP_ULPI_STP,
	PINGRP_DAP3_FS,
	PINGRP_DAP3_DIN,
	PINGRP_DAP3_DOUT,
	PINGRP_DAP3_SCLK,
	PINGRP_GPIO_PV0,
	PINGRP_GPIO_PV1,
	PINGRP_SDMMC1_CLK,
	PINGRP_SDMMC1_CMD,
	PINGRP_SDMMC1_DAT3,
	PINGRP_SDMMC1_DAT2,
	PINGRP_SDMMC1_DAT1,
	PINGRP_SDMMC1_DAT0,
	PINGRP_GPIO_PV2,
	PINGRP_GPIO_PV3,
	PINGRP_CLK2_OUT,
	PINGRP_CLK2_REQ,
	PINGRP_LCD_PWR1,
	PINGRP_LCD_PWR2,
	PINGRP_LCD_SDIN,
	PINGRP_LCD_SDOUT,
	PINGRP_LCD_WR_N,
	PINGRP_LCD_CS0_N,
	PINGRP_LCD_DC0,
	PINGRP_LCD_SCK,
	PINGRP_LCD_PWR0,
	PINGRP_LCD_PCLK,
	PINGRP_LCD_DE,
	PINGRP_LCD_HSYNC,
	PINGRP_LCD_VSYNC,
	PINGRP_LCD_D0,
	PINGRP_LCD_D1,
	PINGRP_LCD_D2,
	PINGRP_LCD_D3,
	PINGRP_LCD_D4,
	PINGRP_LCD_D5,
	PINGRP_LCD_D6,
	PINGRP_LCD_D7,
	PINGRP_LCD_D8,
	PINGRP_LCD_D9,
	PINGRP_LCD_D10,
	PINGRP_LCD_D11,
	PINGRP_LCD_D12,
	PINGRP_LCD_D13,
	PINGRP_LCD_D14,
	PINGRP_LCD_D15,
	PINGRP_LCD_D16,
	PINGRP_LCD_D17,
	PINGRP_LCD_D18,
	PINGRP_LCD_D19,
	PINGRP_LCD_D20,
	PINGRP_LCD_D21,
	PINGRP_LCD_D22,
	PINGRP_LCD_D23,
	PINGRP_LCD_CS1_N,
	PINGRP_LCD_M1,
	PINGRP_LCD_DC1,
	PINGRP_HDMI_INT,
	PINGRP_DDC_SCL,
	PINGRP_DDC_SDA,
	PINGRP_CRT_HSYNC,
	PINGRP_CRT_VSYNC,
	PINGRP_VI_D0,
	PINGRP_VI_D1,
	PINGRP_VI_D2,
	PINGRP_VI_D3,
	PINGRP_VI_D4,
	PINGRP_VI_D5,
	PINGRP_VI_D6,
	PINGRP_VI_D7,
	PINGRP_VI_D8,
	PINGRP_VI_D9,
	PINGRP_VI_D10,
	PINGRP_VI_D11,
	PINGRP_VI_PCLK,
	PINGRP_VI_MCLK,
	PINGRP_VI_VSYNC,
	PINGRP_VI_HSYNC,
	PINGRP_UART2_RXD,
	PINGRP_UART2_TXD,
	PINGRP_UART2_RTS_N,
	PINGRP_UART2_CTS_N,
	PINGRP_UART3_TXD,
	PINGRP_UART3_RXD,
	PINGRP_UART3_CTS_N,
	PINGRP_UART3_RTS_N,
	PINGRP_GPIO_PU0,
	PINGRP_GPIO_PU1,
	PINGRP_GPIO_PU2,
	PINGRP_GPIO_PU3,
	PINGRP_GPIO_PU4,
	PINGRP_GPIO_PU5,
	PINGRP_GPIO_PU6,
	PINGRP_GEN1_I2C_SDA,
	PINGRP_GEN1_I2C_SCL,
	PINGRP_DAP4_FS,
	PINGRP_DAP4_DIN,
	PINGRP_DAP4_DOUT,
	PINGRP_DAP4_SCLK,
	PINGRP_CLK3_OUT,
	PINGRP_CLK3_REQ,
	PINGRP_GMI_WP_N,
	PINGRP_GMI_IORDY,
	PINGRP_GMI_WAIT,
	PINGRP_GMI_ADV_N,
	PINGRP_GMI_CLK,
	PINGRP_GMI_CS0_N,
	PINGRP_GMI_CS1_N,
	PINGRP_GMI_CS2_N,
	PINGRP_GMI_CS3_N,
	PINGRP_GMI_CS4_N,
	PINGRP_GMI_CS6_N,
	PINGRP_GMI_CS7_N,
	PINGRP_GMI_AD0,
	PINGRP_GMI_AD1,
	PINGRP_GMI_AD2,
	PINGRP_GMI_AD3,
	PINGRP_GMI_AD4,
	PINGRP_GMI_AD5,
	PINGRP_GMI_AD6,
	PINGRP_GMI_AD7,
	PINGRP_GMI_AD8,
	PINGRP_GMI_AD9,
	PINGRP_GMI_AD10,
	PINGRP_GMI_AD11,
	PINGRP_GMI_AD12,
	PINGRP_GMI_AD13,
	PINGRP_GMI_AD14,
	PINGRP_GMI_AD15,
	PINGRP_GMI_A16,
	PINGRP_GMI_A17,
	PINGRP_GMI_A18,
	PINGRP_GMI_A19,
	PINGRP_GMI_WR_N,
	PINGRP_GMI_OE_N,
	PINGRP_GMI_DQS,
	PINGRP_GMI_RST_N,
	PINGRP_GEN2_I2C_SCL,
	PINGRP_GEN2_I2C_SDA,
	PINGRP_SDMMC4_CLK,
	PINGRP_SDMMC4_CMD,
	PINGRP_SDMMC4_DAT0,
	PINGRP_SDMMC4_DAT1,
	PINGRP_SDMMC4_DAT2,
	PINGRP_SDMMC4_DAT3,
	PINGRP_SDMMC4_DAT4,
	PINGRP_SDMMC4_DAT5,
	PINGRP_SDMMC4_DAT6,
	PINGRP_SDMMC4_DAT7,
	PINGRP_SDMMC4_RST_N,
	PINGRP_CAM_MCLK,
	PINGRP_GPIO_PCC1,
	PINGRP_GPIO_PBB0,
	PINGRP_CAM_I2C_SCL,
	PINGRP_CAM_I2C_SDA,
	PINGRP_GPIO_PBB3,
	PINGRP_GPIO_PBB4,
	PINGRP_GPIO_PBB5,
	PINGRP_GPIO_PBB6,
	PINGRP_GPIO_PBB7,
	PINGRP_GPIO_PCC2,
	PINGRP_JTAG_RTCK,
	PINGRP_PWR_I2C_SCL,
	PINGRP_PWR_I2C_SDA,
	PINGRP_KB_ROW0,
	PINGRP_KB_ROW1,
	PINGRP_KB_ROW2,
	PINGRP_KB_ROW3,
	PINGRP_KB_ROW4,
	PINGRP_KB_ROW5,
	PINGRP_KB_ROW6,
	PINGRP_KB_ROW7,
	PINGRP_KB_ROW8,
	PINGRP_KB_ROW9,
	PINGRP_KB_ROW10,
	PINGRP_KB_ROW11,
	PINGRP_KB_ROW12,
	PINGRP_KB_ROW13,
	PINGRP_KB_ROW14,
	PINGRP_KB_ROW15,
	PINGRP_KB_COL0,
	PINGRP_KB_COL1,
	PINGRP_KB_COL2,
	PINGRP_KB_COL3,
	PINGRP_KB_COL4,
	PINGRP_KB_COL5,
	PINGRP_KB_COL6,
	PINGRP_KB_COL7,
	PINGRP_CLK_32K_OUT,
	PINGRP_SYS_CLK_REQ,
	PINGRP_CORE_PWR_REQ,
	PINGRP_CPU_PWR_REQ,
	PINGRP_PWR_INT_N,
	PINGRP_CLK_32K_IN,
	PINGRP_OWR,
	PINGRP_DAP1_FS,
	PINGRP_DAP1_DIN,
	PINGRP_DAP1_DOUT,
	PINGRP_DAP1_SCLK,
	PINGRP_CLK1_REQ,
	PINGRP_CLK1_OUT,
	PINGRP_SPDIF_IN,
	PINGRP_SPDIF_OUT,
	PINGRP_DAP2_FS,
	PINGRP_DAP2_DIN,
	PINGRP_DAP2_DOUT,
	PINGRP_DAP2_SCLK,
	PINGRP_SPI2_MOSI,
	PINGRP_SPI2_MISO,
	PINGRP_SPI2_CS0_N,
	PINGRP_SPI2_SCK,
	PINGRP_SPI1_MOSI,
	PINGRP_SPI1_SCK,
	PINGRP_SPI1_CS0_N,
	PINGRP_SPI1_MISO,
	PINGRP_SPI2_CS1_N,
	PINGRP_SPI2_CS2_N,
	PINGRP_SDMMC3_CLK,
	PINGRP_SDMMC3_CMD,
	PINGRP_SDMMC3_DAT0,
	PINGRP_SDMMC3_DAT1,
	PINGRP_SDMMC3_DAT2,
	PINGRP_SDMMC3_DAT3,
	PINGRP_SDMMC3_DAT4,
	PINGRP_SDMMC3_DAT5,
	PINGRP_SDMMC3_DAT6,
	PINGRP_SDMMC3_DAT7,
	PINGRP_PEX_L0_PRSNT_N,
	PINGRP_PEX_L0_RST_N,
	PINGRP_PEX_L0_CLKREQ_N,
	PINGRP_PEX_WAKE_N,
	PINGRP_PEX_L1_PRSNT_N,
	PINGRP_PEX_L1_RST_N,
	PINGRP_PEX_L1_CLKREQ_N,
	PINGRP_PEX_L2_PRSNT_N,
	PINGRP_PEX_L2_RST_N,
	PINGRP_PEX_L2_CLKREQ_N,
	PINGRP_HDMI_CEC,	/* offset 0x33e0 */
	PINGRP_COUNT,
};

enum pdrive_pingrp {
	PDRIVE_PINGROUP_AO1 = 0, /* offset 0x868 */
	PDRIVE_PINGROUP_AO2,
	PDRIVE_PINGROUP_AT1,
	PDRIVE_PINGROUP_AT2,
	PDRIVE_PINGROUP_AT3,
	PDRIVE_PINGROUP_AT4,
	PDRIVE_PINGROUP_AT5,
	PDRIVE_PINGROUP_CDEV1,
	PDRIVE_PINGROUP_CDEV2,
	PDRIVE_PINGROUP_CSUS,
	PDRIVE_PINGROUP_DAP1,
	PDRIVE_PINGROUP_DAP2,
	PDRIVE_PINGROUP_DAP3,
	PDRIVE_PINGROUP_DAP4,
	PDRIVE_PINGROUP_DBG,
	PDRIVE_PINGROUP_LCD1,
	PDRIVE_PINGROUP_LCD2,
	PDRIVE_PINGROUP_SDIO2,
	PDRIVE_PINGROUP_SDIO3,
	PDRIVE_PINGROUP_SPI,
	PDRIVE_PINGROUP_UAA,
	PDRIVE_PINGROUP_UAB,
	PDRIVE_PINGROUP_UART2,
	PDRIVE_PINGROUP_UART3,
	PDRIVE_PINGROUP_VI1 = 24,	/* offset 0x8c8 */
	PDRIVE_PINGROUP_SDIO1 = 33,	/* offset 0x8ec */
	PDRIVE_PINGROUP_CRT = 36,	/* offset 0x8f8 */
	PDRIVE_PINGROUP_DDC,
	PDRIVE_PINGROUP_GMA,
	PDRIVE_PINGROUP_GMB,
	PDRIVE_PINGROUP_GMC,
	PDRIVE_PINGROUP_GMD,
	PDRIVE_PINGROUP_GME,
	PDRIVE_PINGROUP_GMF,
	PDRIVE_PINGROUP_GMG,
	PDRIVE_PINGROUP_GMH,
	PDRIVE_PINGROUP_OWR,
	PDRIVE_PINGROUP_UAD,
	PDRIVE_PINGROUP_GPV,
	PDRIVE_PINGROUP_DEV3 = 49,	/* offset 0x92c */
	PDRIVE_PINGROUP_CEC = 52,	/* offset 0x938 */
	PDRIVE_PINGROUP_COUNT,
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
	PMUX_FUNC_I2C1,
	PMUX_FUNC_I2C2,
	PMUX_FUNC_I2C3,
	PMUX_FUNC_IDE,
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
	PMUX_FUNC_SDMMC1,
	PMUX_FUNC_SDMMC2,
	PMUX_FUNC_SDMMC3,
	PMUX_FUNC_SDMMC4,
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
	PMUX_FUNC_BLINK,
	PMUX_FUNC_CEC,
	PMUX_FUNC_CLK12,
	PMUX_FUNC_DAP,
	PMUX_FUNC_DAPSDMMC2,
	PMUX_FUNC_DDR,
	PMUX_FUNC_DEV3,
	PMUX_FUNC_DTV,
	PMUX_FUNC_VI_ALT1,
	PMUX_FUNC_VI_ALT2,
	PMUX_FUNC_VI_ALT3,
	PMUX_FUNC_EMC_DLL,
	PMUX_FUNC_EXTPERIPH1,
	PMUX_FUNC_EXTPERIPH2,
	PMUX_FUNC_EXTPERIPH3,
	PMUX_FUNC_GMI_ALT,
	PMUX_FUNC_HDA,
	PMUX_FUNC_HSI,
	PMUX_FUNC_I2C4,
	PMUX_FUNC_I2C5,
	PMUX_FUNC_I2CPWR,
	PMUX_FUNC_I2S0,
	PMUX_FUNC_I2S1,
	PMUX_FUNC_I2S2,
	PMUX_FUNC_I2S3,
	PMUX_FUNC_I2S4,
	PMUX_FUNC_NAND_ALT,
	PMUX_FUNC_POPSDIO4,
	PMUX_FUNC_POPSDMMC4,
	PMUX_FUNC_PWM0,
	PMUX_FUNC_PWM1,
	PMUX_FUNC_PWM2,
	PMUX_FUNC_PWM3,
	PMUX_FUNC_SATA,
	PMUX_FUNC_SPI5,
	PMUX_FUNC_SPI6,
	PMUX_FUNC_SYSCLK,
	PMUX_FUNC_VGP1,
	PMUX_FUNC_VGP2,
	PMUX_FUNC_VGP3,
	PMUX_FUNC_VGP4,
	PMUX_FUNC_VGP5,
	PMUX_FUNC_VGP6,
	PMUX_FUNC_CLK_12M_OUT,
	PMUX_FUNC_HDCP,
	PMUX_FUNC_TEST,
	PMUX_FUNC_CORE_PWR_REQ,
	PMUX_FUNC_CPU_PWR_REQ,
	PMUX_FUNC_PWR_INT_N,
	PMUX_FUNC_CLK_32K_IN,
	PMUX_FUNC_SAFE,

	PMUX_FUNC_MAX,

	PMUX_FUNC_RSVD1 = 0x8000,
	PMUX_FUNC_RSVD2 = 0x8001,
	PMUX_FUNC_RSVD3 = 0x8002,
	PMUX_FUNC_RSVD4 = 0x8003,
};

/* return 1 if a pmux_func is in range */
#define pmux_func_isvalid(func) ((((func) >= 0) && ((func) < PMUX_FUNC_MAX)) \
	|| (((func) >= PMUX_FUNC_RSVD1) && ((func) <= PMUX_FUNC_RSVD4)))

/* return 1 if a pingrp is in range */
#define pmux_pingrp_isvalid(pin) (((pin) >= 0) && ((pin) < PINGRP_COUNT))

/* The pullup/pulldown state of a pin group */
enum pmux_pull {
	PMUX_PULL_NORMAL = 0,
	PMUX_PULL_DOWN,
	PMUX_PULL_UP,
};
/* return 1 if a pin_pupd_is in range */
#define pmux_pin_pupd_isvalid(pupd) (((pupd) >= PMUX_PULL_NORMAL) && \
				((pupd) <= PMUX_PULL_UP))

/* Defines whether a pin group is tristated or in normal operation */
enum pmux_tristate {
	PMUX_TRI_NORMAL = 0,
	PMUX_TRI_TRISTATE = 1,
};
/* return 1 if a pin_tristate_is in range */
#define pmux_pin_tristate_isvalid(tristate) (((tristate) >= PMUX_TRI_NORMAL) \
				&& ((tristate) <= PMUX_TRI_TRISTATE))

enum pmux_pin_io {
	PMUX_PIN_OUTPUT = 0,
	PMUX_PIN_INPUT = 1,
};
/* return 1 if a pin_io_is in range */
#define pmux_pin_io_isvalid(io) (((io) >= PMUX_PIN_OUTPUT) && \
				((io) <= PMUX_PIN_INPUT))

enum pmux_pin_lock {
	PMUX_PIN_LOCK_DEFAULT = 0,
	PMUX_PIN_LOCK_DISABLE,
	PMUX_PIN_LOCK_ENABLE,
};
/* return 1 if a pin_lock is in range */
#define pmux_pin_lock_isvalid(lock) (((lock) >= PMUX_PIN_LOCK_DEFAULT) && \
				((lock) <= PMUX_PIN_LOCK_ENABLE))

enum pmux_pin_od {
	PMUX_PIN_OD_DEFAULT = 0,
	PMUX_PIN_OD_DISABLE,
	PMUX_PIN_OD_ENABLE,
};
/* return 1 if a pin_od is in range */
#define pmux_pin_od_isvalid(od) (((od) >= PMUX_PIN_OD_DEFAULT) && \
				((od) <= PMUX_PIN_OD_ENABLE))

enum pmux_pin_ioreset {
	PMUX_PIN_IO_RESET_DEFAULT = 0,
	PMUX_PIN_IO_RESET_DISABLE,
	PMUX_PIN_IO_RESET_ENABLE,
};
/* return 1 if a pin_ioreset_is in range */
#define pmux_pin_ioreset_isvalid(ioreset) \
				(((ioreset) >= PMUX_PIN_IO_RESET_DEFAULT) && \
				((ioreset) <= PMUX_PIN_IO_RESET_ENABLE))

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
	PMUX_VDDIO_CAM,
	PMUX_VDDIO_GMI,
	PMUX_VDDIO_PEXCTL,
	PMUX_VDDIO_SDMMC1,
	PMUX_VDDIO_SDMMC3,
	PMUX_VDDIO_SDMMC4,

	PMUX_VDDIO_NONE
};

#define PGRP_SLWF_NONE	-1
#define PGRP_SLWF_MAX	3
#define	PGRP_SLWR_NONE	PGRP_SLWF_NONE
#define PGRP_SLWR_MAX	PGRP_SLWF_MAX

#define PGRP_DRVUP_NONE	-1
#define PGRP_DRVUP_MAX	127
#define	PGRP_DRVDN_NONE	PGRP_DRVUP_NONE
#define PGRP_DRVDN_MAX	PGRP_DRVUP_MAX

/* return 1 if a padgrp is in range */
#define pmux_padgrp_isvalid(pd) (((pd) >= 0) && ((pd) < PDRIVE_PINGROUP_COUNT))

/* return 1 if a slew-rate rising/falling edge value is in range */
#define pmux_pad_slw_isvalid(slw) (((slw) >= 0) && ((slw) <= PGRP_SLWF_MAX))

/* return 1 if a driver output pull-up/down strength code value is in range */
#define pmux_pad_drv_isvalid(drv) (((drv) >= 0) && ((drv) <= PGRP_DRVUP_MAX))

/* return 1 if a low-power mode value is in range */
#define pmux_pad_lpmd_isvalid(lpm) (((lpm) >= 0) && ((lpm) <= PGRP_LPMD_X))

/* Defines a pin group cfg's low-power mode select */
enum pgrp_lpmd {
	PGRP_LPMD_X8 = 0,
	PGRP_LPMD_X4,
	PGRP_LPMD_X2,
	PGRP_LPMD_X,
	PGRP_LPMD_NONE = -1,
};

/* Defines whether a pin group cfg's schmidt is enabled or not */
enum pgrp_schmt {
	PGRP_SCHMT_DISABLE = 0,
	PGRP_SCHMT_ENABLE = 1,
};

/* Defines whether a pin group cfg's high-speed mode is enabled or not */
enum pgrp_hsm {
	PGRP_HSM_DISABLE = 0,
	PGRP_HSM_ENABLE = 1,
};

/*
 * This defines the configuration for a pin group's pad control config
 */
struct padctrl_config {
	enum pdrive_pingrp padgrp;	/* pin group PDRIVE_PINGRP_x */
	int slwf;			/* falling edge slew         */
	int slwr;			/* rising edge slew          */
	int drvup;			/* pull-up drive strength    */
	int drvdn;			/* pull-down drive strength  */
	enum pgrp_lpmd lpmd;		/* low-power mode selection  */
	enum pgrp_schmt schmt;		/* schmidt enable            */
	enum pgrp_hsm hsm;		/* high-speed mode enable    */
};

/* t30 pin drive group and pin mux registers */
#define PDRIVE_PINGROUP_OFFSET	(0x868 >> 2)
#define PMUX_OFFSET	((0x3000 >> 2) - PDRIVE_PINGROUP_OFFSET - \
				PDRIVE_PINGROUP_COUNT)
struct pmux_tri_ctlr {
	uint pmt_reserved0;		/* ABP_MISC_PP_ reserved offset 00 */
	uint pmt_reserved1;		/* ABP_MISC_PP_ reserved offset 04 */
	uint pmt_strap_opt_a;		/* _STRAPPING_OPT_A_0, offset 08   */
	uint pmt_reserved2;		/* ABP_MISC_PP_ reserved offset 0C */
	uint pmt_reserved3;		/* ABP_MISC_PP_ reserved offset 10 */
	uint pmt_reserved4[4];		/* _TRI_STATE_REG_A/B/C/D in t20 */
	uint pmt_cfg_ctl;		/* _CONFIG_CTL_0, offset 24        */

	uint pmt_reserved[528];		/* ABP_MISC_PP_ reserved offs 28-864 */

	uint pmt_drive[PDRIVE_PINGROUP_COUNT];	/* pin drive grps offs 868 */
	uint pmt_reserved5[PMUX_OFFSET];
	uint pmt_ctl[PINGRP_COUNT];	/* mux/pupd/tri regs, offset 0x3000 */
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
	enum pmux_pin_io io;		/* input or output PMUX_PIN_...  */
	enum pmux_pin_lock lock;	/* lock enable/disable PMUX_PIN...  */
	enum pmux_pin_od od;		/* open-drain or push-pull driver  */
	enum pmux_pin_ioreset ioreset;	/* input/output reset PMUX_PIN...  */
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
void pinmux_config_pingroup(struct pingroup_config *config);

/* Set a pin group to tristate or normal */
void pinmux_set_tristate(enum pmux_pingrp pin, int enable);

/* Set a pin group as input or output */
void pinmux_set_io(enum pmux_pingrp pin, enum pmux_pin_io io);

/**
 * Configure a list of pin groups
 *
 * @param config	List of config items
 * @param len		Number of config items in list
 */
void pinmux_config_table(struct pingroup_config *config, int len);

/* Set a group of pins from a table */
void pinmux_init(void);

/**
 * Set the GP pad configs
 *
 * @param config	List of config items
 * @param len		Number of config items in list
 */
void padgrp_config_table(struct padctrl_config *config, int len);

#endif	/* _TEGRA30_PINMUX_H_ */
