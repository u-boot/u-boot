/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/grf_rk3328.h>
#include <asm/arch/periph.h>
#include <asm/io.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* GPIO0A_IOMUX */
	GPIO0A5_SEL_SHIFT	= 10,
	GPIO0A5_SEL_MASK	= 3 << GPIO0A5_SEL_SHIFT,
	GPIO0A5_I2C3_SCL	= 2,

	GPIO0A6_SEL_SHIFT	= 12,
	GPIO0A6_SEL_MASK	= 3 << GPIO0A6_SEL_SHIFT,
	GPIO0A6_I2C3_SDA	= 2,

	GPIO0A7_SEL_SHIFT	= 14,
	GPIO0A7_SEL_MASK	= 3 << GPIO0A7_SEL_SHIFT,
	GPIO0A7_EMMC_DATA0	= 2,

	/* GPIO0B_IOMUX*/
	GPIO0B0_SEL_SHIFT	= 0,
	GPIO0B0_SEL_MASK	= 3 << GPIO0B0_SEL_SHIFT,
	GPIO0B0_GAMC_CLKTXM0	= 1,

	GPIO0B4_SEL_SHIFT	= 8,
	GPIO0B4_SEL_MASK	= 3 << GPIO0B4_SEL_SHIFT,
	GPIO0B4_GAMC_TXENM0	= 1,

	/* GPIO0C_IOMUX*/
	GPIO0C0_SEL_SHIFT	= 0,
	GPIO0C0_SEL_MASK	= 3 << GPIO0C0_SEL_SHIFT,
	GPIO0C0_GAMC_TXD1M0	= 1,

	GPIO0C1_SEL_SHIFT	= 2,
	GPIO0C1_SEL_MASK	= 3 << GPIO0C1_SEL_SHIFT,
	GPIO0C1_GAMC_TXD0M0	= 1,

	GPIO0C6_SEL_SHIFT	= 12,
	GPIO0C6_SEL_MASK	= 3 << GPIO0C6_SEL_SHIFT,
	GPIO0C6_GAMC_TXD2M0	= 1,

	GPIO0C7_SEL_SHIFT	= 14,
	GPIO0C7_SEL_MASK	= 3 << GPIO0C7_SEL_SHIFT,
	GPIO0C7_GAMC_TXD3M0	= 1,

	/* GPIO0D_IOMUX*/
	GPIO0D0_SEL_SHIFT	= 0,
	GPIO0D0_SEL_MASK	= 3 << GPIO0D0_SEL_SHIFT,
	GPIO0D0_GMAC_CLKM0	= 1,

	GPIO0D6_SEL_SHIFT	= 12,
	GPIO0D6_SEL_MASK	= 3 << GPIO0D6_SEL_SHIFT,
	GPIO0D6_GPIO		= 0,
	GPIO0D6_SDMMC0_PWRENM1	= 3,

	/* GPIO1A_IOMUX */
	GPIO1A0_SEL_SHIFT	= 0,
	GPIO1A0_SEL_MASK	= 0x3fff << GPIO1A0_SEL_SHIFT,
	GPIO1A0_CARD_DATA_CLK_CMD_DETN	= 0x1555,

	/* GPIO1B_IOMUX */
	GPIO1B0_SEL_SHIFT	= 0,
	GPIO1B0_SEL_MASK	= 3 << GPIO1B0_SEL_SHIFT,
	GPIO1B0_GMAC_TXD1M1	= 2,

	GPIO1B1_SEL_SHIFT	= 2,
	GPIO1B1_SEL_MASK	= 3 << GPIO1B1_SEL_SHIFT,
	GPIO1B1_GMAC_TXD0M1	= 2,

	GPIO1B2_SEL_SHIFT	= 4,
	GPIO1B2_SEL_MASK	= 3 << GPIO1B2_SEL_SHIFT,
	GPIO1B2_GMAC_RXD1M1	= 2,

	GPIO1B3_SEL_SHIFT	= 6,
	GPIO1B3_SEL_MASK	= 3 << GPIO1B3_SEL_SHIFT,
	GPIO1B3_GMAC_RXD0M1	= 2,

	GPIO1B4_SEL_SHIFT	= 8,
	GPIO1B4_SEL_MASK	= 3 << GPIO1B4_SEL_SHIFT,
	GPIO1B4_GMAC_TXCLKM1	= 2,

	GPIO1B5_SEL_SHIFT	= 10,
	GPIO1B5_SEL_MASK	= 3 << GPIO1B5_SEL_SHIFT,
	GPIO1B5_GMAC_RXCLKM1	= 2,

	GPIO1B6_SEL_SHIFT	= 12,
	GPIO1B6_SEL_MASK	= 3 << GPIO1B6_SEL_SHIFT,
	GPIO1B6_GMAC_RXD3M1	= 2,

	GPIO1B7_SEL_SHIFT	= 14,
	GPIO1B7_SEL_MASK	= 3 << GPIO1B7_SEL_SHIFT,
	GPIO1B7_GMAC_RXD2M1	= 2,

	/* GPIO1C_IOMUX */
	GPIO1C0_SEL_SHIFT	= 0,
	GPIO1C0_SEL_MASK	= 3 << GPIO1C0_SEL_SHIFT,
	GPIO1C0_GMAC_TXD3M1	= 2,

	GPIO1C1_SEL_SHIFT	= 2,
	GPIO1C1_SEL_MASK	= 3 << GPIO1C1_SEL_SHIFT,
	GPIO1C1_GMAC_TXD2M1	= 2,

	GPIO1C3_SEL_SHIFT	= 6,
	GPIO1C3_SEL_MASK	= 3 << GPIO1C3_SEL_SHIFT,
	GPIO1C3_GMAC_MDIOM1	= 2,

	GPIO1C5_SEL_SHIFT	= 10,
	GPIO1C5_SEL_MASK	= 3 << GPIO1C5_SEL_SHIFT,
	GPIO1C5_GMAC_CLKM1	= 2,

	GPIO1C6_SEL_SHIFT	= 12,
	GPIO1C6_SEL_MASK	= 3 << GPIO1C6_SEL_SHIFT,
	GPIO1C6_GMAC_RXDVM1	= 2,

	GPIO1C7_SEL_SHIFT	= 14,
	GPIO1C7_SEL_MASK	= 3 << GPIO1C7_SEL_SHIFT,
	GPIO1C7_GMAC_MDCM1	= 2,

	/* GPIO1D_IOMUX */
	GPIO1D1_SEL_SHIFT	= 2,
	GPIO1D1_SEL_MASK	= 3 << GPIO1D1_SEL_SHIFT,
	GPIO1D1_GMAC_TXENM1	= 2,

	/* GPIO2A_IOMUX */
	GPIO2A0_SEL_SHIFT	= 0,
	GPIO2A0_SEL_MASK	= 3 << GPIO2A0_SEL_SHIFT,
	GPIO2A0_UART2_TX_M1	= 1,

	GPIO2A1_SEL_SHIFT	= 2,
	GPIO2A1_SEL_MASK	= 3 << GPIO2A1_SEL_SHIFT,
	GPIO2A1_UART2_RX_M1	= 1,

	GPIO2A2_SEL_SHIFT	= 4,
	GPIO2A2_SEL_MASK	= 3 << GPIO2A2_SEL_SHIFT,
	GPIO2A2_PWM_IR		= 1,

	GPIO2A4_SEL_SHIFT	= 8,
	GPIO2A4_SEL_MASK	= 3 << GPIO2A4_SEL_SHIFT,
	GPIO2A4_PWM_0		= 1,
	GPIO2A4_I2C1_SDA,

	GPIO2A5_SEL_SHIFT	= 10,
	GPIO2A5_SEL_MASK	= 3 << GPIO2A5_SEL_SHIFT,
	GPIO2A5_PWM_1		= 1,
	GPIO2A5_I2C1_SCL,

	GPIO2A6_SEL_SHIFT	= 12,
	GPIO2A6_SEL_MASK	= 3 << GPIO2A6_SEL_SHIFT,
	GPIO2A6_PWM_2		= 1,

	GPIO2A7_SEL_SHIFT	= 14,
	GPIO2A7_SEL_MASK	= 3 << GPIO2A7_SEL_SHIFT,
	GPIO2A7_GPIO		= 0,
	GPIO2A7_SDMMC0_PWRENM0,

	/* GPIO2BL_IOMUX */
	GPIO2BL0_SEL_SHIFT	= 0,
	GPIO2BL0_SEL_MASK	= 0x3f << GPIO2BL0_SEL_SHIFT,
	GPIO2BL0_SPI_CLK_TX_RX_M0	= 0x15,

	GPIO2BL3_SEL_SHIFT	= 6,
	GPIO2BL3_SEL_MASK	= 3 << GPIO2BL3_SEL_SHIFT,
	GPIO2BL3_SPI_CSN0_M0	= 1,

	GPIO2BL4_SEL_SHIFT	= 8,
	GPIO2BL4_SEL_MASK	= 3 << GPIO2BL4_SEL_SHIFT,
	GPIO2BL4_SPI_CSN1_M0	= 1,

	GPIO2BL5_SEL_SHIFT	= 10,
	GPIO2BL5_SEL_MASK	= 3 << GPIO2BL5_SEL_SHIFT,
	GPIO2BL5_I2C2_SDA	= 1,

	GPIO2BL6_SEL_SHIFT	= 12,
	GPIO2BL6_SEL_MASK	= 3 << GPIO2BL6_SEL_SHIFT,
	GPIO2BL6_I2C2_SCL	= 1,

	/* GPIO2D_IOMUX */
	GPIO2D0_SEL_SHIFT	= 0,
	GPIO2D0_SEL_MASK	= 3 << GPIO2D0_SEL_SHIFT,
	GPIO2D0_I2C0_SCL	= 1,

	GPIO2D1_SEL_SHIFT	= 2,
	GPIO2D1_SEL_MASK	= 3 << GPIO2D1_SEL_SHIFT,
	GPIO2D1_I2C0_SDA	= 1,

	GPIO2D4_SEL_SHIFT	= 8,
	GPIO2D4_SEL_MASK	= 0xff << GPIO2D4_SEL_SHIFT,
	GPIO2D4_EMMC_DATA1234	= 0xaa,

	/* GPIO3C_IOMUX */
	GPIO3C0_SEL_SHIFT	= 0,
	GPIO3C0_SEL_MASK	= 0x3fff << GPIO3C0_SEL_SHIFT,
	GPIO3C0_EMMC_DATA567_PWR_CLK_RSTN_CMD	= 0x2aaa,

	/* COM_IOMUX */
	IOMUX_SEL_UART2_SHIFT	= 0,
	IOMUX_SEL_UART2_MASK	= 3 << IOMUX_SEL_UART2_SHIFT,
	IOMUX_SEL_UART2_M0	= 0,
	IOMUX_SEL_UART2_M1,

	IOMUX_SEL_GMAC_SHIFT	= 2,
	IOMUX_SEL_GMAC_MASK	= 1 << IOMUX_SEL_GMAC_SHIFT,
	IOMUX_SEL_GMAC_M0	= 0,
	IOMUX_SEL_GMAC_M1,

	IOMUX_SEL_SPI_SHIFT	= 4,
	IOMUX_SEL_SPI_MASK	= 3 << IOMUX_SEL_SPI_SHIFT,
	IOMUX_SEL_SPI_M0	= 0,
	IOMUX_SEL_SPI_M1,
	IOMUX_SEL_SPI_M2,

	IOMUX_SEL_SDMMC_SHIFT	= 7,
	IOMUX_SEL_SDMMC_MASK	= 1 << IOMUX_SEL_SDMMC_SHIFT,
	IOMUX_SEL_SDMMC_M0	= 0,
	IOMUX_SEL_SDMMC_M1,

	IOMUX_SEL_GMACM1_OPTIMIZATION_SHIFT	= 10,
	IOMUX_SEL_GMACM1_OPTIMIZATION_MASK	= 1 << IOMUX_SEL_GMACM1_OPTIMIZATION_SHIFT,
	IOMUX_SEL_GMACM1_OPTIMIZATION_BEFORE	= 0,
	IOMUX_SEL_GMACM1_OPTIMIZATION_AFTER,

	/* GRF_GPIO1B_E */
	GRF_GPIO1B0_E_SHIFT = 0,
	GRF_GPIO1B0_E_MASK = 3 << GRF_GPIO1B0_E_SHIFT,
	GRF_GPIO1B1_E_SHIFT = 2,
	GRF_GPIO1B1_E_MASK = 3 << GRF_GPIO1B1_E_SHIFT,
	GRF_GPIO1B2_E_SHIFT = 4,
	GRF_GPIO1B2_E_MASK = 3 << GRF_GPIO1B2_E_SHIFT,
	GRF_GPIO1B3_E_SHIFT = 6,
	GRF_GPIO1B3_E_MASK = 3 << GRF_GPIO1B3_E_SHIFT,
	GRF_GPIO1B4_E_SHIFT = 8,
	GRF_GPIO1B4_E_MASK = 3 << GRF_GPIO1B4_E_SHIFT,
	GRF_GPIO1B5_E_SHIFT = 10,
	GRF_GPIO1B5_E_MASK = 3 << GRF_GPIO1B5_E_SHIFT,
	GRF_GPIO1B6_E_SHIFT = 12,
	GRF_GPIO1B6_E_MASK = 3 << GRF_GPIO1B6_E_SHIFT,
	GRF_GPIO1B7_E_SHIFT = 14,
	GRF_GPIO1B7_E_MASK = 3 << GRF_GPIO1B7_E_SHIFT,

	/*  GRF_GPIO1C_E */
	GRF_GPIO1C0_E_SHIFT = 0,
	GRF_GPIO1C0_E_MASK = 3 << GRF_GPIO1C0_E_SHIFT,
	GRF_GPIO1C1_E_SHIFT = 2,
	GRF_GPIO1C1_E_MASK = 3 << GRF_GPIO1C1_E_SHIFT,
	GRF_GPIO1C3_E_SHIFT = 6,
	GRF_GPIO1C3_E_MASK = 3 << GRF_GPIO1C3_E_SHIFT,
	GRF_GPIO1C5_E_SHIFT = 10,
	GRF_GPIO1C5_E_MASK = 3 << GRF_GPIO1C5_E_SHIFT,
	GRF_GPIO1C6_E_SHIFT = 12,
	GRF_GPIO1C6_E_MASK = 3 << GRF_GPIO1C6_E_SHIFT,
	GRF_GPIO1C7_E_SHIFT = 14,
	GRF_GPIO1C7_E_MASK = 3 << GRF_GPIO1C7_E_SHIFT,

	/*  GRF_GPIO1D_E */
	GRF_GPIO1D1_E_SHIFT = 2,
	GRF_GPIO1D1_E_MASK = 3 << GRF_GPIO1D1_E_SHIFT,
};

/* GPIO Bias drive strength settings */
enum GPIO_BIAS {
	GPIO_BIAS_2MA = 0,
	GPIO_BIAS_4MA,
	GPIO_BIAS_8MA,
	GPIO_BIAS_12MA,
};

struct rk3328_pinctrl_priv {
	struct rk3328_grf_regs *grf;
};

static void pinctrl_rk3328_pwm_config(struct rk3328_grf_regs *grf, int pwm_id)
{
	switch (pwm_id) {
	case PERIPH_ID_PWM0:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A4_SEL_MASK,
			     GPIO2A4_PWM_0 << GPIO2A4_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM1:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A5_SEL_MASK,
			     GPIO2A5_PWM_1 << GPIO2A5_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM2:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A6_SEL_MASK,
			     GPIO2A6_PWM_2 << GPIO2A6_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM3:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A2_SEL_MASK,
			     GPIO2A2_PWM_IR << GPIO2A2_SEL_SHIFT);
		break;
	default:
		debug("pwm id = %d iomux error!\n", pwm_id);
		break;
	}
}

static void pinctrl_rk3328_i2c_config(struct rk3328_grf_regs *grf, int i2c_id)
{
	switch (i2c_id) {
	case PERIPH_ID_I2C0:
		rk_clrsetreg(&grf->gpio2d_iomux,
			     GPIO2D0_SEL_MASK | GPIO2D1_SEL_MASK,
			     GPIO2D0_I2C0_SCL << GPIO2D0_SEL_SHIFT |
			     GPIO2D1_I2C0_SDA << GPIO2D1_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C1:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A4_SEL_MASK | GPIO2A5_SEL_MASK,
			     GPIO2A5_I2C1_SCL << GPIO2A5_SEL_SHIFT |
			     GPIO2A4_I2C1_SDA << GPIO2A4_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C2:
		rk_clrsetreg(&grf->gpio2bl_iomux,
			     GPIO2BL5_SEL_MASK | GPIO2BL6_SEL_MASK,
			     GPIO2BL6_I2C2_SCL << GPIO2BL6_SEL_SHIFT |
			     GPIO2BL5_I2C2_SDA << GPIO2BL5_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C3:
		rk_clrsetreg(&grf->gpio0a_iomux,
			     GPIO0A5_SEL_MASK | GPIO0A6_SEL_MASK,
			     GPIO0A5_I2C3_SCL << GPIO0A5_SEL_SHIFT |
			     GPIO0A6_I2C3_SDA << GPIO0A6_SEL_SHIFT);
		break;
	default:
		debug("i2c id = %d iomux error!\n", i2c_id);
		break;
	}
}

static void pinctrl_rk3328_lcdc_config(struct rk3328_grf_regs *grf, int lcd_id)
{
	switch (lcd_id) {
	case PERIPH_ID_LCDC0:
		break;
	default:
		debug("lcdc id = %d iomux error!\n", lcd_id);
		break;
	}
}

static int pinctrl_rk3328_spi_config(struct rk3328_grf_regs *grf,
				     enum periph_id spi_id, int cs)
{
	u32 com_iomux = readl(&grf->com_iomux);

	if ((com_iomux & IOMUX_SEL_SPI_MASK) !=
		IOMUX_SEL_SPI_M0 << IOMUX_SEL_SPI_SHIFT) {
		debug("driver do not support iomux other than m0\n");
		goto err;
	}

	switch (spi_id) {
	case PERIPH_ID_SPI0:
		switch (cs) {
		case 0:
			rk_clrsetreg(&grf->gpio2bl_iomux,
				     GPIO2BL3_SEL_MASK,
				     GPIO2BL3_SPI_CSN0_M0
				     << GPIO2BL3_SEL_SHIFT);
			break;
		case 1:
			rk_clrsetreg(&grf->gpio2bl_iomux,
				     GPIO2BL4_SEL_MASK,
				     GPIO2BL4_SPI_CSN1_M0
				     << GPIO2BL4_SEL_SHIFT);
			break;
		default:
			goto err;
		}
		rk_clrsetreg(&grf->gpio2bl_iomux,
			     GPIO2BL0_SEL_MASK,
			     GPIO2BL0_SPI_CLK_TX_RX_M0 << GPIO2BL0_SEL_SHIFT);
		break;
	default:
		goto err;
	}

	return 0;
err:
	debug("rkspi: periph%d cs=%d not supported", spi_id, cs);
	return -ENOENT;
}

static void pinctrl_rk3328_uart_config(struct rk3328_grf_regs *grf, int uart_id)
{
	u32 com_iomux = readl(&grf->com_iomux);

	switch (uart_id) {
	case PERIPH_ID_UART2:
		break;
		if (com_iomux & IOMUX_SEL_UART2_MASK)
			rk_clrsetreg(&grf->gpio2a_iomux,
				     GPIO2A0_SEL_MASK | GPIO2A1_SEL_MASK,
				     GPIO2A0_UART2_TX_M1 << GPIO2A0_SEL_SHIFT |
				     GPIO2A1_UART2_RX_M1 << GPIO2A1_SEL_SHIFT);

		break;
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART3:
	case PERIPH_ID_UART4:
	default:
		debug("uart id = %d iomux error!\n", uart_id);
		break;
	}
}

static void pinctrl_rk3328_sdmmc_config(struct rk3328_grf_regs *grf,
					int mmc_id)
{
	u32 com_iomux = readl(&grf->com_iomux);

	switch (mmc_id) {
	case PERIPH_ID_EMMC:
		rk_clrsetreg(&grf->gpio0a_iomux,
			     GPIO0A7_SEL_MASK,
			     GPIO0A7_EMMC_DATA0 << GPIO0A7_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio2d_iomux,
			     GPIO2D4_SEL_MASK,
			     GPIO2D4_EMMC_DATA1234 << GPIO2D4_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio3c_iomux,
			     GPIO3C0_SEL_MASK,
			     GPIO3C0_EMMC_DATA567_PWR_CLK_RSTN_CMD
			     << GPIO3C0_SEL_SHIFT);
		break;
	case PERIPH_ID_SDCARD:
		/* SDMMC_PWREN use GPIO and init as regulator-fiexed  */
		if (com_iomux & IOMUX_SEL_SDMMC_MASK)
			rk_clrsetreg(&grf->gpio0d_iomux,
				     GPIO0D6_SEL_MASK,
				     GPIO0D6_GPIO << GPIO0D6_SEL_SHIFT);
		else
			rk_clrsetreg(&grf->gpio2a_iomux,
				     GPIO2A7_SEL_MASK,
				     GPIO2A7_GPIO << GPIO2A7_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio1a_iomux,
			     GPIO1A0_SEL_MASK,
			     GPIO1A0_CARD_DATA_CLK_CMD_DETN
			     << GPIO1A0_SEL_SHIFT);
		break;
	default:
		debug("mmc id = %d iomux error!\n", mmc_id);
		break;
	}
}

#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
static void pinctrl_rk3328_gmac_config(struct rk3328_grf_regs *grf, int gmac_id)
{
	switch (gmac_id) {
	case PERIPH_ID_GMAC:
		/* set rgmii m1 pins mux */
		rk_clrsetreg(&grf->gpio1b_iomux,
			     GPIO1B0_SEL_MASK |
			     GPIO1B1_SEL_MASK |
			     GPIO1B2_SEL_MASK |
			     GPIO1B3_SEL_MASK |
			     GPIO1B4_SEL_MASK |
			     GPIO1B5_SEL_MASK |
			     GPIO1B6_SEL_MASK |
			     GPIO1B7_SEL_MASK,
			     GPIO1B0_GMAC_TXD1M1 << GPIO1B0_SEL_SHIFT |
			     GPIO1B1_GMAC_TXD0M1 << GPIO1B1_SEL_SHIFT |
			     GPIO1B2_GMAC_RXD1M1 << GPIO1B2_SEL_SHIFT |
			     GPIO1B3_GMAC_RXD0M1 << GPIO1B3_SEL_SHIFT |
			     GPIO1B4_GMAC_TXCLKM1 << GPIO1B4_SEL_SHIFT |
			     GPIO1B5_GMAC_RXCLKM1 << GPIO1B5_SEL_SHIFT |
			     GPIO1B6_GMAC_RXD3M1 << GPIO1B6_SEL_SHIFT |
			     GPIO1B7_GMAC_RXD2M1 << GPIO1B7_SEL_SHIFT);

		rk_clrsetreg(&grf->gpio1c_iomux,
			     GPIO1C0_SEL_MASK |
			     GPIO1C1_SEL_MASK |
			     GPIO1C3_SEL_MASK |
			     GPIO1C5_SEL_MASK |
			     GPIO1C6_SEL_MASK |
			     GPIO1C7_SEL_MASK,
			     GPIO1C0_GMAC_TXD3M1 << GPIO1C0_SEL_SHIFT |
			     GPIO1C1_GMAC_TXD2M1 << GPIO1C1_SEL_SHIFT |
			     GPIO1C3_GMAC_MDIOM1 << GPIO1C3_SEL_SHIFT |
			     GPIO1C5_GMAC_CLKM1 << GPIO1C5_SEL_SHIFT |
			     GPIO1C6_GMAC_RXDVM1 << GPIO1C6_SEL_SHIFT |
			     GPIO1C7_GMAC_MDCM1 << GPIO1C7_SEL_SHIFT);

		rk_clrsetreg(&grf->gpio1d_iomux,
			     GPIO1D1_SEL_MASK,
			     GPIO1D1_GMAC_TXENM1 << GPIO1D1_SEL_SHIFT);

		/* set rgmii m0 tx pins mux */
		rk_clrsetreg(&grf->gpio0b_iomux,
			     GPIO0B0_SEL_MASK |
			     GPIO0B4_SEL_MASK,
			     GPIO0B0_GAMC_CLKTXM0 << GPIO0B0_SEL_SHIFT |
			     GPIO0B4_GAMC_TXENM0 << GPIO0B4_SEL_SHIFT);

		rk_clrsetreg(&grf->gpio0c_iomux,
			     GPIO0C0_SEL_MASK |
			     GPIO0C1_SEL_MASK |
			     GPIO0C6_SEL_MASK |
			     GPIO0C7_SEL_MASK,
			     GPIO0C0_GAMC_TXD1M0 << GPIO0C0_SEL_SHIFT |
			     GPIO0C1_GAMC_TXD0M0 << GPIO0C1_SEL_SHIFT |
			     GPIO0C6_GAMC_TXD2M0 << GPIO0C6_SEL_SHIFT |
			     GPIO0C7_GAMC_TXD3M0 << GPIO0C7_SEL_SHIFT);

		rk_clrsetreg(&grf->gpio0d_iomux,
			     GPIO0D0_SEL_MASK,
			     GPIO0D0_GMAC_CLKM0 << GPIO0D0_SEL_SHIFT);

		/* set com mux */
		rk_clrsetreg(&grf->com_iomux,
			     IOMUX_SEL_GMAC_MASK |
			     IOMUX_SEL_GMACM1_OPTIMIZATION_MASK,
			     IOMUX_SEL_GMAC_M1 << IOMUX_SEL_GMAC_SHIFT |
			     IOMUX_SEL_GMACM1_OPTIMIZATION_AFTER <<
			     IOMUX_SEL_GMACM1_OPTIMIZATION_SHIFT);

		/*
		 * set rgmii m1 tx pins to 12ma drive-strength,
		 * and clean others to 2ma.
		 */
		rk_clrsetreg(&grf->gpio1b_e,
			     GRF_GPIO1B0_E_MASK |
			     GRF_GPIO1B1_E_MASK |
			     GRF_GPIO1B2_E_MASK |
			     GRF_GPIO1B3_E_MASK |
			     GRF_GPIO1B4_E_MASK |
			     GRF_GPIO1B5_E_MASK |
			     GRF_GPIO1B6_E_MASK |
			     GRF_GPIO1B7_E_MASK,
			     GPIO_BIAS_12MA << GRF_GPIO1B0_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO1B1_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1B2_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1B3_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO1B4_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1B5_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1B6_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1B7_E_SHIFT);

		rk_clrsetreg(&grf->gpio1c_e,
			     GRF_GPIO1C0_E_MASK |
			     GRF_GPIO1C1_E_MASK |
			     GRF_GPIO1C3_E_MASK |
			     GRF_GPIO1C5_E_MASK |
			     GRF_GPIO1C6_E_MASK |
			     GRF_GPIO1C7_E_MASK,
			     GPIO_BIAS_12MA << GRF_GPIO1C0_E_SHIFT |
			     GPIO_BIAS_12MA << GRF_GPIO1C1_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1C3_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1C5_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1C6_E_SHIFT |
			     GPIO_BIAS_2MA << GRF_GPIO1C7_E_SHIFT);

		rk_clrsetreg(&grf->gpio1d_e,
			     GRF_GPIO1D1_E_MASK,
			     GPIO_BIAS_12MA << GRF_GPIO1D1_E_SHIFT);
		break;
	default:
		debug("gmac id = %d iomux error!\n", gmac_id);
		break;
	}
}
#endif

static int rk3328_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rk3328_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%x, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_PWM0:
	case PERIPH_ID_PWM1:
	case PERIPH_ID_PWM2:
	case PERIPH_ID_PWM3:
		pinctrl_rk3328_pwm_config(priv->grf, func);
		break;
	case PERIPH_ID_I2C0:
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
	case PERIPH_ID_I2C3:
		pinctrl_rk3328_i2c_config(priv->grf, func);
		break;
	case PERIPH_ID_SPI0:
		pinctrl_rk3328_spi_config(priv->grf, func, flags);
		break;
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
	case PERIPH_ID_UART4:
		pinctrl_rk3328_uart_config(priv->grf, func);
		break;
	case PERIPH_ID_LCDC0:
	case PERIPH_ID_LCDC1:
		pinctrl_rk3328_lcdc_config(priv->grf, func);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
		pinctrl_rk3328_sdmmc_config(priv->grf, func);
		break;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case PERIPH_ID_GMAC:
		pinctrl_rk3328_gmac_config(priv->grf, func);
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk3328_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[3];
	int ret;

	ret = dev_read_u32_array(periph, "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 49:
		return PERIPH_ID_SPI0;
	case 50:
		return PERIPH_ID_PWM0;
	case 36:
		return PERIPH_ID_I2C0;
	case 37: /* Note strange order */
		return PERIPH_ID_I2C1;
	case 38:
		return PERIPH_ID_I2C2;
	case 39:
		return PERIPH_ID_I2C3;
	case 12:
		return PERIPH_ID_SDCARD;
	case 14:
		return PERIPH_ID_EMMC;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case 24:
		return PERIPH_ID_GMAC;
#endif
	}

	return -ENOENT;
}

static int rk3328_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rk3328_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;

	return rk3328_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops rk3328_pinctrl_ops = {
	.set_state_simple	= rk3328_pinctrl_set_state_simple,
	.request	= rk3328_pinctrl_request,
	.get_periph_id	= rk3328_pinctrl_get_periph_id,
};

static int rk3328_pinctrl_probe(struct udevice *dev)
{
	struct rk3328_pinctrl_priv *priv = dev_get_priv(dev);
	int ret = 0;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	debug("%s: grf=%p\n", __func__, priv->grf);

	return ret;
}

static const struct udevice_id rk3328_pinctrl_ids[] = {
	{ .compatible = "rockchip,rk3328-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3328) = {
	.name		= "rockchip_rk3328_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3328_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rk3328_pinctrl_priv),
	.ops		= &rk3328_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= rk3328_pinctrl_probe,
};
