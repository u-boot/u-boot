/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk322x.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

/* GRF_GPIO0A_IOMUX */
enum {
	GPIO0A7_SHIFT		= 14,
	GPIO0A7_MASK		= 3 << GPIO0A7_SHIFT,
	GPIO0A7_GPIO		= 0,
	GPIO0A7_I2C3_SDA,
	GPIO0A7_HDMI_DDCSDA,

	GPIO0A6_SHIFT		= 12,
	GPIO0A6_MASK		= 3 << GPIO0A6_SHIFT,
	GPIO0A6_GPIO		= 0,
	GPIO0A6_I2C3_SCL,
	GPIO0A6_HDMI_DDCSCL,

	GPIO0A3_SHIFT		= 6,
	GPIO0A3_MASK		= 3 << GPIO0A3_SHIFT,
	GPIO0A3_GPIO		= 0,
	GPIO0A3_I2C1_SDA,
	GPIO0A3_SDIO_CMD,

	GPIO0A2_SHIFT		= 4,
	GPIO0A2_MASK		= 3 << GPIO0A2_SHIFT,
	GPIO0A2_GPIO		= 0,
	GPIO0A2_I2C1_SCL,

	GPIO0A1_SHIFT		= 2,
	GPIO0A1_MASK		= 3 << GPIO0A1_SHIFT,
	GPIO0A1_GPIO		= 0,
	GPIO0A1_I2C0_SDA,

	GPIO0A0_SHIFT		= 0,
	GPIO0A0_MASK		= 3 << GPIO0A0_SHIFT,
	GPIO0A0_GPIO		= 0,
	GPIO0A0_I2C0_SCL,
};

/* GRF_GPIO0B_IOMUX */
enum {
	GPIO0B7_SHIFT		= 14,
	GPIO0B7_MASK		= 3 << GPIO0B7_SHIFT,
	GPIO0B7_GPIO		= 0,
	GPIO0B7_HDMI_HDP,

	GPIO0B6_SHIFT		= 12,
	GPIO0B6_MASK		= 3 << GPIO0B6_SHIFT,
	GPIO0B6_GPIO		= 0,
	GPIO0B6_I2S_SDI,
	GPIO0B6_SPI_CSN0,

	GPIO0B5_SHIFT		= 10,
	GPIO0B5_MASK		= 3 << GPIO0B5_SHIFT,
	GPIO0B5_GPIO		= 0,
	GPIO0B5_I2S_SDO,
	GPIO0B5_SPI_RXD,

	GPIO0B3_SHIFT		= 6,
	GPIO0B3_MASK		= 3 << GPIO0B3_SHIFT,
	GPIO0B3_GPIO		= 0,
	GPIO0B3_I2S1_LRCKRX,
	GPIO0B3_SPI_TXD,

	GPIO0B1_SHIFT		= 2,
	GPIO0B1_MASK		= 3 << GPIO0B1_SHIFT,
	GPIO0B1_GPIO		= 0,
	GPIO0B1_I2S_SCLK,
	GPIO0B1_SPI_CLK,

	GPIO0B0_SHIFT		= 0,
	GPIO0B0_MASK		= 3,
	GPIO0B0_GPIO		= 0,
	GPIO0B0_I2S_MCLK,
};

/* GRF_GPIO0C_IOMUX */
enum {
	GPIO0C4_SHIFT		= 8,
	GPIO0C4_MASK		= 3 << GPIO0C4_SHIFT,
	GPIO0C4_GPIO		= 0,
	GPIO0C4_HDMI_CECSDA,

	GPIO0C1_SHIFT		= 2,
	GPIO0C1_MASK		= 3 << GPIO0C1_SHIFT,
	GPIO0C1_GPIO		= 0,
	GPIO0C1_UART0_RSTN,
	GPIO0C1_CLK_OUT1,
};

/* GRF_GPIO0D_IOMUX */
enum {
	GPIO0D6_SHIFT		= 12,
	GPIO0D6_MASK		= 3 << GPIO0D6_SHIFT,
	GPIO0D6_GPIO		= 0,
	GPIO0D6_SDIO_PWREN,
	GPIO0D6_PWM11,

	GPIO0D4_SHIFT		= 8,
	GPIO0D4_MASK		= 3 << GPIO0D4_SHIFT,
	GPIO0D4_GPIO		= 0,
	GPIO0D4_PWM2,

	GPIO0D3_SHIFT		= 6,
	GPIO0D3_MASK		= 3 << GPIO0D3_SHIFT,
	GPIO0D3_GPIO		= 0,
	GPIO0D3_PWM1,

	GPIO0D2_SHIFT		= 4,
	GPIO0D2_MASK		= 3 << GPIO0D2_SHIFT,
	GPIO0D2_GPIO		= 0,
	GPIO0D2_PWM0,
};

/* GRF_GPIO1A_IOMUX */
enum {
	GPIO1A7_SHIFT		= 14,
	GPIO1A7_MASK		= 1,
	GPIO1A7_GPIO		= 0,
	GPIO1A7_SDMMC_WRPRT,
};

/* GRF_GPIO1B_IOMUX */
enum {
	GPIO1B7_SHIFT		= 14,
	GPIO1B7_MASK		= 3 << GPIO1B7_SHIFT,
	GPIO1B7_GPIO		= 0,
	GPIO1B7_SDMMC_CMD,

	GPIO1B6_SHIFT		= 12,
	GPIO1B6_MASK		= 3 << GPIO1B6_SHIFT,
	GPIO1B6_GPIO		= 0,
	GPIO1B6_SDMMC_PWREN,

	GPIO1B4_SHIFT		= 8,
	GPIO1B4_MASK		= 3 << GPIO1B4_SHIFT,
	GPIO1B4_GPIO		= 0,
	GPIO1B4_SPI_CSN1,
	GPIO1B4_PWM12,

	GPIO1B3_SHIFT		= 6,
	GPIO1B3_MASK		= 3 << GPIO1B3_SHIFT,
	GPIO1B3_GPIO		= 0,
	GPIO1B3_UART1_RSTN,
	GPIO1B3_PWM13,

	GPIO1B2_SHIFT		= 4,
	GPIO1B2_MASK		= 3 << GPIO1B2_SHIFT,
	GPIO1B2_GPIO		= 0,
	GPIO1B2_UART1_SIN,
	GPIO1B2_UART21_SIN,

	GPIO1B1_SHIFT		= 2,
	GPIO1B1_MASK		= 3 << GPIO1B1_SHIFT,
	GPIO1B1_GPIO		= 0,
	GPIO1B1_UART1_SOUT,
	GPIO1B1_UART21_SOUT,
};

/* GRF_GPIO1C_IOMUX */
enum {
	GPIO1C7_SHIFT		= 14,
	GPIO1C7_MASK		= 3 << GPIO1C7_SHIFT,
	GPIO1C7_GPIO		= 0,
	GPIO1C7_NAND_CS3,
	GPIO1C7_EMMC_RSTNOUT,

	GPIO1C6_SHIFT		= 12,
	GPIO1C6_MASK		= 3 << GPIO1C6_SHIFT,
	GPIO1C6_GPIO		= 0,
	GPIO1C6_NAND_CS2,
	GPIO1C6_EMMC_CMD,

	GPIO1C5_SHIFT		= 10,
	GPIO1C5_MASK		= 3 << GPIO1C5_SHIFT,
	GPIO1C5_GPIO		= 0,
	GPIO1C5_SDMMC_D3,
	GPIO1C5_JTAG_TMS,

	GPIO1C4_SHIFT		= 8,
	GPIO1C4_MASK		= 3 << GPIO1C4_SHIFT,
	GPIO1C4_GPIO		= 0,
	GPIO1C4_SDMMC_D2,
	GPIO1C4_JTAG_TCK,

	GPIO1C3_SHIFT		= 6,
	GPIO1C3_MASK		= 3 << GPIO1C3_SHIFT,
	GPIO1C3_GPIO		= 0,
	GPIO1C3_SDMMC_D1,
	GPIO1C3_UART2_SIN,

	GPIO1C2_SHIFT		= 4,
	GPIO1C2_MASK		= 3 << GPIO1C2_SHIFT,
	GPIO1C2_GPIO		= 0,
	GPIO1C2_SDMMC_D0,
	GPIO1C2_UART2_SOUT,

	GPIO1C1_SHIFT		= 2,
	GPIO1C1_MASK		= 3 << GPIO1C1_SHIFT,
	GPIO1C1_GPIO		= 0,
	GPIO1C1_SDMMC_DETN,

	GPIO1C0_SHIFT		= 0,
	GPIO1C0_MASK		= 3 << GPIO1C0_SHIFT,
	GPIO1C0_GPIO		= 0,
	GPIO1C0_SDMMC_CLKOUT,
};

/* GRF_GPIO1D_IOMUX */
enum {
	GPIO1D7_SHIFT		= 14,
	GPIO1D7_MASK		= 3 << GPIO1D7_SHIFT,
	GPIO1D7_GPIO		= 0,
	GPIO1D7_NAND_D7,
	GPIO1D7_EMMC_D7,

	GPIO1D6_SHIFT		= 12,
	GPIO1D6_MASK		= 3 << GPIO1D6_SHIFT,
	GPIO1D6_GPIO		= 0,
	GPIO1D6_NAND_D6,
	GPIO1D6_EMMC_D6,

	GPIO1D5_SHIFT		= 10,
	GPIO1D5_MASK		= 3 << GPIO1D5_SHIFT,
	GPIO1D5_GPIO		= 0,
	GPIO1D5_NAND_D5,
	GPIO1D5_EMMC_D5,

	GPIO1D4_SHIFT		= 8,
	GPIO1D4_MASK		= 3 << GPIO1D4_SHIFT,
	GPIO1D4_GPIO		= 0,
	GPIO1D4_NAND_D4,
	GPIO1D4_EMMC_D4,

	GPIO1D3_SHIFT		= 6,
	GPIO1D3_MASK		= 3 << GPIO1D3_SHIFT,
	GPIO1D3_GPIO		= 0,
	GPIO1D3_NAND_D3,
	GPIO1D3_EMMC_D3,

	GPIO1D2_SHIFT		= 4,
	GPIO1D2_MASK		= 3 << GPIO1D2_SHIFT,
	GPIO1D2_GPIO		= 0,
	GPIO1D2_NAND_D2,
	GPIO1D2_EMMC_D2,

	GPIO1D1_SHIFT		= 2,
	GPIO1D1_MASK		= 3 << GPIO1D1_SHIFT,
	GPIO1D1_GPIO		= 0,
	GPIO1D1_NAND_D1,
	GPIO1D1_EMMC_D1,

	GPIO1D0_SHIFT		= 0,
	GPIO1D0_MASK		= 3 << GPIO1D0_SHIFT,
	GPIO1D0_GPIO		= 0,
	GPIO1D0_NAND_D0,
	GPIO1D0_EMMC_D0,
};

/* GRF_GPIO2A_IOMUX */
enum {
	GPIO2A7_SHIFT		= 14,
	GPIO2A7_MASK		= 3 << GPIO2A7_SHIFT,
	GPIO2A7_GPIO		= 0,
	GPIO2A7_NAND_DQS,
	GPIO2A7_EMMC_CLKOUT,

	GPIO2A5_SHIFT		= 10,
	GPIO2A5_MASK		= 3 << GPIO2A5_SHIFT,
	GPIO2A5_GPIO		= 0,
	GPIO2A5_NAND_WP,
	GPIO2A5_EMMC_PWREN,

	GPIO2A4_SHIFT		= 8,
	GPIO2A4_MASK		= 3 << GPIO2A4_SHIFT,
	GPIO2A4_GPIO		= 0,
	GPIO2A4_NAND_RDY,
	GPIO2A4_EMMC_CMD,

	GPIO2A3_SHIFT		= 6,
	GPIO2A3_MASK		= 3 << GPIO2A3_SHIFT,
	GPIO2A3_GPIO		= 0,
	GPIO2A3_NAND_RDN,
	GPIO2A4_SPI1_CSN1,

	GPIO2A2_SHIFT		= 4,
	GPIO2A2_MASK		= 3 << GPIO2A2_SHIFT,
	GPIO2A2_GPIO		= 0,
	GPIO2A2_NAND_WRN,
	GPIO2A4_SPI1_CSN0,

	GPIO2A1_SHIFT		= 2,
	GPIO2A1_MASK		= 3 << GPIO2A1_SHIFT,
	GPIO2A1_GPIO		= 0,
	GPIO2A1_NAND_CLE,
	GPIO2A1_SPI1_TXD,

	GPIO2A0_SHIFT		= 0,
	GPIO2A0_MASK		= 3 << GPIO2A0_SHIFT,
	GPIO2A0_GPIO		= 0,
	GPIO2A0_NAND_ALE,
	GPIO2A0_SPI1_RXD,
};

/* GRF_GPIO2B_IOMUX */
enum {
	GPIO2B7_SHIFT		= 14,
	GPIO2B7_MASK		= 3 << GPIO2B7_SHIFT,
	GPIO2B7_GPIO		= 0,
	GPIO2B7_GMAC_RXER,

	GPIO2B6_SHIFT		= 12,
	GPIO2B6_MASK		= 3 << GPIO2B6_SHIFT,
	GPIO2B6_GPIO		= 0,
	GPIO2B6_GMAC_CLK,
	GPIO2B6_MAC_LINK,

	GPIO2B5_SHIFT		= 10,
	GPIO2B5_MASK		= 3 << GPIO2B5_SHIFT,
	GPIO2B5_GPIO		= 0,
	GPIO2B5_GMAC_TXEN,

	GPIO2B4_SHIFT		= 8,
	GPIO2B4_MASK		= 3 << GPIO2B4_SHIFT,
	GPIO2B4_GPIO		= 0,
	GPIO2B4_GMAC_MDIO,

	GPIO2B3_SHIFT		= 6,
	GPIO2B3_MASK		= 3 << GPIO2B3_SHIFT,
	GPIO2B3_GPIO		= 0,
	GPIO2B3_GMAC_RXCLK,

	GPIO2B2_SHIFT		= 4,
	GPIO2B2_MASK		= 3 << GPIO2B2_SHIFT,
	GPIO2B2_GPIO		= 0,
	GPIO2B2_GMAC_CRS,

	GPIO2B1_SHIFT		= 2,
	GPIO2B1_MASK		= 3 << GPIO2B1_SHIFT,
	GPIO2B1_GPIO		= 0,
	GPIO2B1_GMAC_TXCLK,

	GPIO2B0_SHIFT		= 0,
	GPIO2B0_MASK		= 3 << GPIO2B0_SHIFT,
	GPIO2B0_GPIO		= 0,
	GPIO2B0_GMAC_RXDV,
	GPIO2B0_MAC_SPEED_IOUT,
};

/* GRF_GPIO2C_IOMUX */
enum {
	GPIO2C7_SHIFT		= 14,
	GPIO2C7_MASK		= 3 << GPIO2C7_SHIFT,
	GPIO2C7_GPIO		= 0,
	GPIO2C7_GMAC_TXD3,

	GPIO2C6_SHIFT		= 12,
	GPIO2C6_MASK		= 3 << GPIO2C6_SHIFT,
	GPIO2C6_GPIO		= 0,
	GPIO2C6_GMAC_TXD2,

	GPIO2C5_SHIFT		= 10,
	GPIO2C5_MASK		= 3 << GPIO2C5_SHIFT,
	GPIO2C5_GPIO		= 0,
	GPIO2C5_I2C2_SCL,
	GPIO2C5_GMAC_RXD2,

	GPIO2C4_SHIFT		= 8,
	GPIO2C4_MASK		= 3 << GPIO2C4_SHIFT,
	GPIO2C4_GPIO		= 0,
	GPIO2C4_I2C2_SDA,
	GPIO2C4_GMAC_RXD3,

	GPIO2C3_SHIFT		= 6,
	GPIO2C3_MASK		= 3 << GPIO2C3_SHIFT,
	GPIO2C3_GPIO		= 0,
	GPIO2C3_GMAC_TXD0,

	GPIO2C2_SHIFT		= 4,
	GPIO2C2_MASK		= 3 << GPIO2C2_SHIFT,
	GPIO2C2_GPIO		= 0,
	GPIO2C2_GMAC_TXD1,

	GPIO2C1_SHIFT		= 2,
	GPIO2C1_MASK		= 3 << GPIO2C1_SHIFT,
	GPIO2C1_GPIO		= 0,
	GPIO2C1_GMAC_RXD0,

	GPIO2C0_SHIFT		= 0,
	GPIO2C0_MASK		= 3 << GPIO2C0_SHIFT,
	GPIO2C0_GPIO		= 0,
	GPIO2C0_GMAC_RXD1,
};

/* GRF_GPIO2D_IOMUX */
enum {
	GPIO2D1_SHIFT		= 2,
	GPIO2D1_MASK		= 3 << GPIO2D1_SHIFT,
	GPIO2D1_GPIO		= 0,
	GPIO2D1_GMAC_MDC,

	GPIO2D0_SHIFT		= 0,
	GPIO2D0_MASK		= 3,
	GPIO2D0_GPIO		= 0,
	GPIO2D0_GMAC_COL,
};

/* GRF_GPIO3C_IOMUX */
enum {
	GPIO3C6_SHIFT		= 12,
	GPIO3C6_MASK		= 3 << GPIO3C6_SHIFT,
	GPIO3C6_GPIO		= 0,
	GPIO3C6_DRV_VBUS1,

	GPIO3C5_SHIFT		= 10,
	GPIO3C5_MASK		= 3 << GPIO3C5_SHIFT,
	GPIO3C5_GPIO		= 0,
	GPIO3C5_PWM10,

	GPIO3C1_SHIFT		= 2,
	GPIO3C1_MASK		= 3 << GPIO3C1_SHIFT,
	GPIO3C1_GPIO		= 0,
	GPIO3C1_DRV_VBUS,
};

/* GRF_GPIO3D_IOMUX */
enum {
	GPIO3D2_SHIFT	= 4,
	GPIO3D2_MASK	= 3 << GPIO3D2_SHIFT,
	GPIO3D2_GPIO	= 0,
	GPIO3D2_PWM3,
};

/* GRF_CON_IOMUX */
enum {
	CON_IOMUX_GMACSEL_SHIFT	= 15,
	CON_IOMUX_GMACSEL_MASK	= 1 << CON_IOMUX_GMACSEL_SHIFT,
	CON_IOMUX_GMACSEL_1	= 1,
	CON_IOMUX_UART1SEL_SHIFT	= 11,
	CON_IOMUX_UART1SEL_MASK	= 1 << CON_IOMUX_UART1SEL_SHIFT,
	CON_IOMUX_UART2SEL_SHIFT	= 8,
	CON_IOMUX_UART2SEL_MASK	= 1 << CON_IOMUX_UART2SEL_SHIFT,
	CON_IOMUX_UART2SEL_2	= 0,
	CON_IOMUX_UART2SEL_21,
	CON_IOMUX_EMMCSEL_SHIFT	= 7,
	CON_IOMUX_EMMCSEL_MASK	= 1 << CON_IOMUX_EMMCSEL_SHIFT,
	CON_IOMUX_PWM3SEL_SHIFT	= 3,
	CON_IOMUX_PWM3SEL_MASK	= 1 << CON_IOMUX_PWM3SEL_SHIFT,
	CON_IOMUX_PWM2SEL_SHIFT	= 2,
	CON_IOMUX_PWM2SEL_MASK	= 1 << CON_IOMUX_PWM2SEL_SHIFT,
	CON_IOMUX_PWM1SEL_SHIFT	= 1,
	CON_IOMUX_PWM1SEL_MASK	= 1 << CON_IOMUX_PWM1SEL_SHIFT,
	CON_IOMUX_PWM0SEL_SHIFT	= 0,
	CON_IOMUX_PWM0SEL_MASK	= 1 << CON_IOMUX_PWM0SEL_SHIFT,
};

/* GRF_GPIO2B_E */
enum {
	GRF_GPIO2B0_E_SHIFT = 0,
	GRF_GPIO2B0_E_MASK = 3 << GRF_GPIO2B0_E_SHIFT,
	GRF_GPIO2B1_E_SHIFT = 2,
	GRF_GPIO2B1_E_MASK = 3 << GRF_GPIO2B1_E_SHIFT,
	GRF_GPIO2B3_E_SHIFT = 6,
	GRF_GPIO2B3_E_MASK = 3 << GRF_GPIO2B3_E_SHIFT,
	GRF_GPIO2B4_E_SHIFT = 8,
	GRF_GPIO2B4_E_MASK = 3 << GRF_GPIO2B4_E_SHIFT,
	GRF_GPIO2B5_E_SHIFT = 10,
	GRF_GPIO2B5_E_MASK = 3 << GRF_GPIO2B5_E_SHIFT,
	GRF_GPIO2B6_E_SHIFT = 12,
	GRF_GPIO2B6_E_MASK = 3 << GRF_GPIO2B6_E_SHIFT,
};

/* GRF_GPIO2C_E */
enum {
	GRF_GPIO2C0_E_SHIFT = 0,
	GRF_GPIO2C0_E_MASK = 3 << GRF_GPIO2C0_E_SHIFT,
	GRF_GPIO2C1_E_SHIFT = 2,
	GRF_GPIO2C1_E_MASK = 3 << GRF_GPIO2C1_E_SHIFT,
	GRF_GPIO2C2_E_SHIFT = 4,
	GRF_GPIO2C2_E_MASK = 3 << GRF_GPIO2C2_E_SHIFT,
	GRF_GPIO2C3_E_SHIFT = 6,
	GRF_GPIO2C3_E_MASK = 3 << GRF_GPIO2C3_E_SHIFT,
	GRF_GPIO2C4_E_SHIFT = 8,
	GRF_GPIO2C4_E_MASK = 3 << GRF_GPIO2C4_E_SHIFT,
	GRF_GPIO2C5_E_SHIFT = 10,
	GRF_GPIO2C5_E_MASK = 3 << GRF_GPIO2C5_E_SHIFT,
	GRF_GPIO2C6_E_SHIFT = 12,
	GRF_GPIO2C6_E_MASK = 3 << GRF_GPIO2C6_E_SHIFT,
	GRF_GPIO2C7_E_SHIFT = 14,
	GRF_GPIO2C7_E_MASK = 3 << GRF_GPIO2C7_E_SHIFT,
};

/* GRF_GPIO2D_E */
enum {
	GRF_GPIO2D1_E_SHIFT = 2,
	GRF_GPIO2D1_E_MASK = 3 << GRF_GPIO2D1_E_SHIFT,
};

/* GPIO Bias drive strength settings */
enum GPIO_BIAS {
	GPIO_BIAS_2MA = 0,
	GPIO_BIAS_4MA,
	GPIO_BIAS_8MA,
	GPIO_BIAS_12MA,
};

struct rk322x_pinctrl_priv {
	struct rk322x_grf *grf;
};

static void pinctrl_rk322x_pwm_config(struct rk322x_grf *grf, int pwm_id)
{
	u32 mux_con = readl(&grf->con_iomux);

	switch (pwm_id) {
	case PERIPH_ID_PWM0:
		if (mux_con & CON_IOMUX_PWM0SEL_MASK)
			rk_clrsetreg(&grf->gpio3c_iomux, GPIO3C5_MASK,
				     GPIO3C5_PWM10 << GPIO3C5_SHIFT);
		else
			rk_clrsetreg(&grf->gpio0d_iomux, GPIO0D2_MASK,
				     GPIO0D2_PWM0 << GPIO0D2_SHIFT);
		break;
	case PERIPH_ID_PWM1:
		if (mux_con & CON_IOMUX_PWM1SEL_MASK)
			rk_clrsetreg(&grf->gpio0d_iomux, GPIO0D6_MASK,
				     GPIO0D6_PWM11 << GPIO0D6_SHIFT);
		else
			rk_clrsetreg(&grf->gpio0d_iomux, GPIO0D3_MASK,
				     GPIO0D3_PWM1 << GPIO0D3_SHIFT);
		break;
	case PERIPH_ID_PWM2:
		if (mux_con & CON_IOMUX_PWM2SEL_MASK)
			rk_clrsetreg(&grf->gpio1b_iomux, GPIO1B4_MASK,
				     GPIO1B4_PWM12 << GPIO1B4_SHIFT);
		else
			rk_clrsetreg(&grf->gpio0d_iomux, GPIO0D4_MASK,
				     GPIO0D4_PWM2 << GPIO0D4_SHIFT);
		break;
	case PERIPH_ID_PWM3:
		if (mux_con & CON_IOMUX_PWM3SEL_MASK)
			rk_clrsetreg(&grf->gpio1b_iomux, GPIO1B3_MASK,
				     GPIO1B3_PWM13 << GPIO1B3_SHIFT);
		else
			rk_clrsetreg(&grf->gpio3d_iomux, GPIO3D2_MASK,
				     GPIO3D2_PWM3 << GPIO3D2_SHIFT);
		break;
	default:
		debug("pwm id = %d iomux error!\n", pwm_id);
		break;
	}
}

static void pinctrl_rk322x_i2c_config(struct rk322x_grf *grf, int i2c_id)
{
	switch (i2c_id) {
	case PERIPH_ID_I2C0:
		rk_clrsetreg(&grf->gpio0a_iomux,
			     GPIO0A1_MASK | GPIO0A0_MASK,
			     GPIO0A1_I2C0_SDA << GPIO0A1_SHIFT |
			     GPIO0A0_I2C0_SCL << GPIO0A0_SHIFT);

		break;
	case PERIPH_ID_I2C1:
		rk_clrsetreg(&grf->gpio0a_iomux,
			     GPIO0A3_MASK | GPIO0A2_MASK,
			     GPIO0A3_I2C1_SDA << GPIO0A3_SHIFT |
			     GPIO0A2_I2C1_SCL << GPIO0A2_SHIFT);
		break;
	case PERIPH_ID_I2C2:
		rk_clrsetreg(&grf->gpio2c_iomux,
			     GPIO2C5_MASK | GPIO2C4_MASK,
			     GPIO2C5_I2C2_SCL << GPIO2C5_SHIFT |
			     GPIO2C4_I2C2_SDA << GPIO2C4_SHIFT);
		break;
	case PERIPH_ID_I2C3:
		rk_clrsetreg(&grf->gpio0a_iomux,
			     GPIO0A7_MASK | GPIO0A6_MASK,
			     GPIO0A7_I2C3_SDA << GPIO0A7_SHIFT |
			     GPIO0A6_I2C3_SCL << GPIO0A6_SHIFT);

		break;
	}
}

static void pinctrl_rk322x_spi_config(struct rk322x_grf *grf, int cs)
{
	switch (cs) {
	case 0:
		rk_clrsetreg(&grf->gpio0b_iomux, GPIO0B6_MASK,
			     GPIO0B6_SPI_CSN0 << GPIO0B6_SHIFT);
		break;
	case 1:
		rk_clrsetreg(&grf->gpio1b_iomux, GPIO1B4_MASK,
			     GPIO1B4_SPI_CSN1 << GPIO1B4_SHIFT);
		break;
	}
	rk_clrsetreg(&grf->gpio0b_iomux,
		     GPIO0B1_MASK | GPIO0B3_MASK | GPIO0B5_MASK,
		     GPIO0B5_SPI_RXD << GPIO0B5_SHIFT |
		     GPIO0B3_SPI_TXD << GPIO0B3_SHIFT |
		     GPIO0B1_SPI_CLK << GPIO0B1_SHIFT);
}

static void pinctrl_rk322x_uart_config(struct rk322x_grf *grf, int uart_id)
{
	u32 mux_con = readl(&grf->con_iomux);

	switch (uart_id) {
	case PERIPH_ID_UART1:
		if (!(mux_con & CON_IOMUX_UART1SEL_MASK))
			rk_clrsetreg(&grf->gpio1b_iomux,
				     GPIO1B1_MASK | GPIO1B2_MASK,
				     GPIO1B1_UART1_SOUT << GPIO1B1_SHIFT |
				     GPIO1B2_UART1_SIN << GPIO1B2_SHIFT);
		break;
	case PERIPH_ID_UART2:
		if (mux_con & CON_IOMUX_UART2SEL_MASK)
			rk_clrsetreg(&grf->gpio1b_iomux,
				     GPIO1B1_MASK | GPIO1B2_MASK,
				     GPIO1B1_UART21_SOUT << GPIO1B1_SHIFT |
				     GPIO1B2_UART21_SIN << GPIO1B2_SHIFT);
		else
			rk_clrsetreg(&grf->gpio1c_iomux,
				     GPIO1C3_MASK | GPIO1C2_MASK,
				     GPIO1C3_UART2_SIN << GPIO1C3_SHIFT |
				     GPIO1C2_UART2_SOUT << GPIO1C2_SHIFT);
		break;
	}
}

static void pinctrl_rk322x_sdmmc_config(struct rk322x_grf *grf, int mmc_id)
{
	switch (mmc_id) {
	case PERIPH_ID_EMMC:
		rk_clrsetreg(&grf->gpio1d_iomux, 0xffff,
			     GPIO1D7_EMMC_D7 << GPIO1D7_SHIFT |
			     GPIO1D6_EMMC_D6 << GPIO1D6_SHIFT |
			     GPIO1D5_EMMC_D5 << GPIO1D5_SHIFT |
			     GPIO1D4_EMMC_D4 << GPIO1D4_SHIFT |
			     GPIO1D3_EMMC_D3 << GPIO1D3_SHIFT |
			     GPIO1D2_EMMC_D2 << GPIO1D2_SHIFT |
			     GPIO1D1_EMMC_D1 << GPIO1D1_SHIFT |
			     GPIO1D0_EMMC_D0 << GPIO1D0_SHIFT);
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A5_MASK | GPIO2A7_MASK,
			     GPIO2A5_EMMC_PWREN << GPIO2A5_SHIFT |
			     GPIO2A7_EMMC_CLKOUT << GPIO2A7_SHIFT);
		rk_clrsetreg(&grf->gpio1c_iomux,
			     GPIO1C6_MASK | GPIO1C7_MASK,
			     GPIO1C6_EMMC_CMD << GPIO1C6_SHIFT |
			     GPIO1C7_EMMC_RSTNOUT << GPIO1C6_SHIFT);
		break;
	case PERIPH_ID_SDCARD:
		rk_clrsetreg(&grf->gpio1b_iomux,
			     GPIO1B6_MASK | GPIO1B7_MASK,
			     GPIO1B6_SDMMC_PWREN << GPIO1B6_SHIFT |
			     GPIO1B7_SDMMC_CMD << GPIO1B7_SHIFT);
		rk_clrsetreg(&grf->gpio1c_iomux, 0xfff,
			     GPIO1C5_SDMMC_D3 << GPIO1C5_SHIFT |
			     GPIO1C4_SDMMC_D2 << GPIO1C4_SHIFT |
			     GPIO1C3_SDMMC_D1 << GPIO1C3_SHIFT |
			     GPIO1C2_SDMMC_D0 << GPIO1C2_SHIFT |
			     GPIO1C1_SDMMC_DETN << GPIO1C1_SHIFT |
			     GPIO1C0_SDMMC_CLKOUT << GPIO1C0_SHIFT);
		break;
	}
}

#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
static void pinctrl_rk322x_gmac_config(struct rk322x_grf *grf, int gmac_id)
{
	switch (gmac_id) {
	case PERIPH_ID_GMAC:
		/* set rgmii pins mux */
		rk_clrsetreg(&grf->gpio2b_iomux,
			     GPIO2B0_MASK |
			     GPIO2B1_MASK |
			     GPIO2B3_MASK |
			     GPIO2B4_MASK |
			     GPIO2B5_MASK |
			     GPIO2B6_MASK,
			     GPIO2B0_GMAC_RXDV << GPIO2B0_SHIFT |
			     GPIO2B1_GMAC_TXCLK << GPIO2B1_SHIFT |
			     GPIO2B3_GMAC_RXCLK << GPIO2B3_SHIFT |
			     GPIO2B4_GMAC_MDIO << GPIO2B4_SHIFT |
			     GPIO2B5_GMAC_TXEN << GPIO2B5_SHIFT |
			     GPIO2B6_GMAC_CLK << GPIO2B6_SHIFT);

		rk_clrsetreg(&grf->gpio2c_iomux,
			     GPIO2C0_MASK |
			     GPIO2C1_MASK |
			     GPIO2C2_MASK |
			     GPIO2C3_MASK |
			     GPIO2C4_MASK |
			     GPIO2C5_MASK |
			     GPIO2C6_MASK |
			     GPIO2C7_MASK,
			     GPIO2C0_GMAC_RXD1 << GPIO2C0_SHIFT |
			     GPIO2C1_GMAC_RXD0 << GPIO2C1_SHIFT |
			     GPIO2C2_GMAC_TXD1 << GPIO2C2_SHIFT |
			     GPIO2C3_GMAC_TXD0 << GPIO2C3_SHIFT |
			     GPIO2C4_GMAC_RXD3 << GPIO2C4_SHIFT |
			     GPIO2C5_GMAC_RXD2 << GPIO2C5_SHIFT |
			     GPIO2C6_GMAC_TXD2 << GPIO2C6_SHIFT |
			     GPIO2C7_GMAC_TXD3 << GPIO2C7_SHIFT);

		rk_clrsetreg(&grf->gpio2d_iomux,
			     GPIO2D1_MASK,
			     GPIO2D1_GMAC_MDC << GPIO2D1_SHIFT);

		/*
		 * set rgmii tx pins to 12ma drive-strength,
		 * clean others with 2ma.
		 */
		rk_clrsetreg(&grf->gpio2_e[1],
			     GRF_GPIO2B0_E_MASK |
			     GRF_GPIO2B1_E_MASK |
			     GRF_GPIO2B3_E_MASK |
			     GRF_GPIO2B4_E_MASK |
			     GRF_GPIO2B5_E_MASK |
			     GRF_GPIO2B6_E_MASK,
			     GPIO_BIAS_2MA << GRF_GPIO2B0_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO2B1_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO2B3_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO2B4_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO2B5_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO2B6_E_SHIFT);

		rk_clrsetreg(&grf->gpio2_e[2],
			     GRF_GPIO2C0_E_MASK |
			     GRF_GPIO2C1_E_MASK |
			     GRF_GPIO2C2_E_MASK |
			     GRF_GPIO2C3_E_MASK |
			     GRF_GPIO2C4_E_MASK |
			     GRF_GPIO2C5_E_MASK |
			     GRF_GPIO2C6_E_MASK |
			     GRF_GPIO2C7_E_MASK,
			     GPIO_BIAS_2MA << GRF_GPIO2C0_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO2C1_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO2C2_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO2C3_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO2C4_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO2C5_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO2C6_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO2C7_E_SHIFT);

		rk_clrsetreg(&grf->gpio2_e[3],
			     GRF_GPIO2D1_E_MASK,
			     GPIO_BIAS_2MA << GRF_GPIO2D1_E_SHIFT);
		break;
	default:
		debug("gmac id = %d iomux error!\n", gmac_id);
		break;
	}
}
#endif

static int rk322x_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rk322x_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%x, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_PWM0:
	case PERIPH_ID_PWM1:
	case PERIPH_ID_PWM2:
	case PERIPH_ID_PWM3:
		pinctrl_rk322x_pwm_config(priv->grf, func);
		break;
	case PERIPH_ID_I2C0:
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
		pinctrl_rk322x_i2c_config(priv->grf, func);
		break;
	case PERIPH_ID_SPI0:
		pinctrl_rk322x_spi_config(priv->grf, flags);
		break;
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
		pinctrl_rk322x_uart_config(priv->grf, func);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
		pinctrl_rk322x_sdmmc_config(priv->grf, func);
		break;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case PERIPH_ID_GMAC:
		pinctrl_rk322x_gmac_config(priv->grf, func);
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk322x_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[3];
	int ret;

	ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(periph),
				   "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 12:
		return PERIPH_ID_SDCARD;
	case 14:
		return PERIPH_ID_EMMC;
	case 36:
		return PERIPH_ID_I2C0;
	case 37:
		return PERIPH_ID_I2C1;
	case 38:
		return PERIPH_ID_I2C2;
	case 49:
		return PERIPH_ID_SPI0;
	case 50:
		return PERIPH_ID_PWM0;
	case 55:
		return PERIPH_ID_UART0;
	case 56:
		return PERIPH_ID_UART1;
	case 57:
		return PERIPH_ID_UART2;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case 24:
		return PERIPH_ID_GMAC;
#endif
	}
	return -ENOENT;
}

static int rk322x_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rk322x_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;
	return rk322x_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops rk322x_pinctrl_ops = {
	.set_state_simple	= rk322x_pinctrl_set_state_simple,
	.request	= rk322x_pinctrl_request,
	.get_periph_id	= rk322x_pinctrl_get_periph_id,
};

static int rk322x_pinctrl_probe(struct udevice *dev)
{
	struct rk322x_pinctrl_priv *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	debug("%s: grf=%p\n", __func__, priv->grf);
	return 0;
}

static const struct udevice_id rk322x_pinctrl_ids[] = {
	{ .compatible = "rockchip,rk3228-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3228) = {
	.name		= "pinctrl_rk3228",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk322x_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rk322x_pinctrl_priv),
	.ops		= &rk322x_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= rk322x_pinctrl_probe,
};
