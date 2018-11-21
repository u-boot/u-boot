// SPDX-License-Identifier: GPL-2.0+
/*
 * Pinctrl driver for Rockchip 3036 SoCs
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk3036.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <dm/pinctrl.h>

/* GRF_GPIO0A_IOMUX */
enum {
	GPIO0A3_SHIFT		= 6,
	GPIO0A3_MASK		= 1 << GPIO0A3_SHIFT,
	GPIO0A3_GPIO		= 0,
	GPIO0A3_I2C1_SDA,

	GPIO0A2_SHIFT		= 4,
	GPIO0A2_MASK		= 1 << GPIO0A2_SHIFT,
	GPIO0A2_GPIO		= 0,
	GPIO0A2_I2C1_SCL,

	GPIO0A1_SHIFT		= 2,
	GPIO0A1_MASK		= 3 << GPIO0A1_SHIFT,
	GPIO0A1_GPIO		= 0,
	GPIO0A1_I2C0_SDA,
	GPIO0A1_PWM2,

	GPIO0A0_SHIFT		= 0,
	GPIO0A0_MASK		= 3 << GPIO0A0_SHIFT,
	GPIO0A0_GPIO		= 0,
	GPIO0A0_I2C0_SCL,
	GPIO0A0_PWM1,
};

/* GRF_GPIO0B_IOMUX */
enum {
	GPIO0B6_SHIFT		= 12,
	GPIO0B6_MASK		= 3 << GPIO0B6_SHIFT,
	GPIO0B6_GPIO		= 0,
	GPIO0B6_MMC1_D3,
	GPIO0B6_I2S1_SCLK,

	GPIO0B5_SHIFT		= 10,
	GPIO0B5_MASK		= 3 << GPIO0B5_SHIFT,
	GPIO0B5_GPIO		= 0,
	GPIO0B5_MMC1_D2,
	GPIO0B5_I2S1_SDI,

	GPIO0B4_SHIFT		= 8,
	GPIO0B4_MASK		= 3 << GPIO0B4_SHIFT,
	GPIO0B4_GPIO		= 0,
	GPIO0B4_MMC1_D1,
	GPIO0B4_I2S1_LRCKTX,

	GPIO0B3_SHIFT		= 6,
	GPIO0B3_MASK		= 3 << GPIO0B3_SHIFT,
	GPIO0B3_GPIO		= 0,
	GPIO0B3_MMC1_D0,
	GPIO0B3_I2S1_LRCKRX,

	GPIO0B1_SHIFT		= 2,
	GPIO0B1_MASK		= 3 << GPIO0B1_SHIFT,
	GPIO0B1_GPIO		= 0,
	GPIO0B1_MMC1_CLKOUT,
	GPIO0B1_I2S1_MCLK,

	GPIO0B0_SHIFT		= 0,
	GPIO0B0_MASK		= 3,
	GPIO0B0_GPIO		= 0,
	GPIO0B0_MMC1_CMD,
	GPIO0B0_I2S1_SDO,
};

/* GRF_GPIO0C_IOMUX */
enum {
	GPIO0C4_SHIFT		= 8,
	GPIO0C4_MASK		= 1 << GPIO0C4_SHIFT,
	GPIO0C4_GPIO		= 0,
	GPIO0C4_DRIVE_VBUS,

	GPIO0C3_SHIFT		= 6,
	GPIO0C3_MASK		= 1 << GPIO0C3_SHIFT,
	GPIO0C3_GPIO		= 0,
	GPIO0C3_UART0_CTSN,

	GPIO0C2_SHIFT		= 4,
	GPIO0C2_MASK		= 1 << GPIO0C2_SHIFT,
	GPIO0C2_GPIO		= 0,
	GPIO0C2_UART0_RTSN,

	GPIO0C1_SHIFT		= 2,
	GPIO0C1_MASK		= 1 << GPIO0C1_SHIFT,
	GPIO0C1_GPIO		= 0,
	GPIO0C1_UART0_SIN,


	GPIO0C0_SHIFT		= 0,
	GPIO0C0_MASK		= 1 << GPIO0C0_SHIFT,
	GPIO0C0_GPIO		= 0,
	GPIO0C0_UART0_SOUT,
};

/* GRF_GPIO0D_IOMUX */
enum {
	GPIO0D4_SHIFT		= 8,
	GPIO0D4_MASK		= 1 << GPIO0D4_SHIFT,
	GPIO0D4_GPIO		= 0,
	GPIO0D4_SPDIF,

	GPIO0D3_SHIFT		= 6,
	GPIO0D3_MASK		= 1 << GPIO0D3_SHIFT,
	GPIO0D3_GPIO		= 0,
	GPIO0D3_PWM3,

	GPIO0D2_SHIFT		= 4,
	GPIO0D2_MASK		= 1 << GPIO0D2_SHIFT,
	GPIO0D2_GPIO		= 0,
	GPIO0D2_PWM0,
};

/* GRF_GPIO1A_IOMUX */
enum {
	GPIO1A5_SHIFT		= 10,
	GPIO1A5_MASK		= 1 << GPIO1A5_SHIFT,
	GPIO1A5_GPIO		= 0,
	GPIO1A5_I2S_SDI,

	GPIO1A4_SHIFT		= 8,
	GPIO1A4_MASK		= 1 << GPIO1A4_SHIFT,
	GPIO1A4_GPIO		= 0,
	GPIO1A4_I2S_SD0,

	GPIO1A3_SHIFT		= 6,
	GPIO1A3_MASK		= 1 << GPIO1A3_SHIFT,
	GPIO1A3_GPIO		= 0,
	GPIO1A3_I2S_LRCKTX,

	GPIO1A2_SHIFT		= 4,
	GPIO1A2_MASK		= 3 << GPIO1A2_SHIFT,
	GPIO1A2_GPIO		= 0,
	GPIO1A2_I2S_LRCKRX,
	GPIO1A2_PWM1_0,

	GPIO1A1_SHIFT		= 2,
	GPIO1A1_MASK		= 1 << GPIO1A1_SHIFT,
	GPIO1A1_GPIO		= 0,
	GPIO1A1_I2S_SCLK,

	GPIO1A0_SHIFT		= 0,
	GPIO1A0_MASK		= 1 << GPIO1A0_SHIFT,
	GPIO1A0_GPIO		= 0,
	GPIO1A0_I2S_MCLK,

};

/* GRF_GPIO1B_IOMUX */
enum {
	GPIO1B7_SHIFT		= 14,
	GPIO1B7_MASK		= 1 << GPIO1B7_SHIFT,
	GPIO1B7_GPIO		= 0,
	GPIO1B7_MMC0_CMD,

	GPIO1B3_SHIFT		= 6,
	GPIO1B3_MASK		= 1 << GPIO1B3_SHIFT,
	GPIO1B3_GPIO		= 0,
	GPIO1B3_HDMI_HPD,

	GPIO1B2_SHIFT		= 4,
	GPIO1B2_MASK		= 1 << GPIO1B2_SHIFT,
	GPIO1B2_GPIO		= 0,
	GPIO1B2_HDMI_SCL,

	GPIO1B1_SHIFT		= 2,
	GPIO1B1_MASK		= 1 << GPIO1B1_SHIFT,
	GPIO1B1_GPIO		= 0,
	GPIO1B1_HDMI_SDA,

	GPIO1B0_SHIFT		= 0,
	GPIO1B0_MASK		= 1 << GPIO1B0_SHIFT,
	GPIO1B0_GPIO		= 0,
	GPIO1B0_HDMI_CEC,
};

/* GRF_GPIO1C_IOMUX */
enum {
	GPIO1C5_SHIFT		= 10,
	GPIO1C5_MASK		= 3 << GPIO1C5_SHIFT,
	GPIO1C5_GPIO		= 0,
	GPIO1C5_MMC0_D3,
	GPIO1C5_JTAG_TMS,

	GPIO1C4_SHIFT		= 8,
	GPIO1C4_MASK		= 3 << GPIO1C4_SHIFT,
	GPIO1C4_GPIO		= 0,
	GPIO1C4_MMC0_D2,
	GPIO1C4_JTAG_TCK,

	GPIO1C3_SHIFT		= 6,
	GPIO1C3_MASK		= 3 << GPIO1C3_SHIFT,
	GPIO1C3_GPIO		= 0,
	GPIO1C3_MMC0_D1,
	GPIO1C3_UART2_SOUT,

	GPIO1C2_SHIFT		= 4,
	GPIO1C2_MASK		= 3 << GPIO1C2_SHIFT ,
	GPIO1C2_GPIO		= 0,
	GPIO1C2_MMC0_D0,
	GPIO1C2_UART2_SIN,

	GPIO1C1_SHIFT		= 2,
	GPIO1C1_MASK		= 1 << GPIO1C1_SHIFT,
	GPIO1C1_GPIO		= 0,
	GPIO1C1_MMC0_DETN,

	GPIO1C0_SHIFT		= 0,
	GPIO1C0_MASK		= 1 << GPIO1C0_SHIFT,
	GPIO1C0_GPIO		= 0,
	GPIO1C0_MMC0_CLKOUT,
};

/* GRF_GPIO1D_IOMUX */
enum {
	GPIO1D7_SHIFT		= 14,
	GPIO1D7_MASK		= 3 << GPIO1D7_SHIFT,
	GPIO1D7_GPIO		= 0,
	GPIO1D7_NAND_D7,
	GPIO1D7_EMMC_D7,
	GPIO1D7_SPI_CSN1,

	GPIO1D6_SHIFT		= 12,
	GPIO1D6_MASK		= 3 << GPIO1D6_SHIFT,
	GPIO1D6_GPIO		= 0,
	GPIO1D6_NAND_D6,
	GPIO1D6_EMMC_D6,
	GPIO1D6_SPI_CSN0,

	GPIO1D5_SHIFT		= 10,
	GPIO1D5_MASK		= 3 << GPIO1D5_SHIFT,
	GPIO1D5_GPIO		= 0,
	GPIO1D5_NAND_D5,
	GPIO1D5_EMMC_D5,
	GPIO1D5_SPI_TXD,

	GPIO1D4_SHIFT		= 8,
	GPIO1D4_MASK		= 3 << GPIO1D4_SHIFT,
	GPIO1D4_GPIO		= 0,
	GPIO1D4_NAND_D4,
	GPIO1D4_EMMC_D4,
	GPIO1D4_SPI_RXD,

	GPIO1D3_SHIFT		= 6,
	GPIO1D3_MASK		= 3 << GPIO1D3_SHIFT,
	GPIO1D3_GPIO		= 0,
	GPIO1D3_NAND_D3,
	GPIO1D3_EMMC_D3,
	GPIO1D3_SFC_SIO3,

	GPIO1D2_SHIFT		= 4,
	GPIO1D2_MASK		= 3 << GPIO1D2_SHIFT,
	GPIO1D2_GPIO		= 0,
	GPIO1D2_NAND_D2,
	GPIO1D2_EMMC_D2,
	GPIO1D2_SFC_SIO2,

	GPIO1D1_SHIFT		= 2,
	GPIO1D1_MASK		= 3 << GPIO1D1_SHIFT,
	GPIO1D1_GPIO		= 0,
	GPIO1D1_NAND_D1,
	GPIO1D1_EMMC_D1,
	GPIO1D1_SFC_SIO1,

	GPIO1D0_SHIFT		= 0,
	GPIO1D0_MASK		= 3 << GPIO1D0_SHIFT,
	GPIO1D0_GPIO		= 0,
	GPIO1D0_NAND_D0,
	GPIO1D0_EMMC_D0,
	GPIO1D0_SFC_SIO0,
};

/* GRF_GPIO2A_IOMUX */
enum {
	GPIO2A7_SHIFT		= 14,
	GPIO2A7_MASK		= 1 << GPIO2A7_SHIFT,
	GPIO2A7_GPIO		= 0,
	GPIO2A7_TESTCLK_OUT,

	GPIO2A6_SHIFT		= 12,
	GPIO2A6_MASK		= 1 << GPIO2A6_SHIFT,
	GPIO2A6_GPIO		= 0,
	GPIO2A6_NAND_CS0,

	GPIO2A4_SHIFT		= 8,
	GPIO2A4_MASK		= 3 << GPIO2A4_SHIFT,
	GPIO2A4_GPIO		= 0,
	GPIO2A4_NAND_RDY,
	GPIO2A4_EMMC_CMD,
	GPIO2A3_SFC_CLK,

	GPIO2A3_SHIFT		= 6,
	GPIO2A3_MASK		= 3 << GPIO2A3_SHIFT,
	GPIO2A3_GPIO		= 0,
	GPIO2A3_NAND_RDN,
	GPIO2A4_SFC_CSN1,

	GPIO2A2_SHIFT		= 4,
	GPIO2A2_MASK		= 3 << GPIO2A2_SHIFT,
	GPIO2A2_GPIO		= 0,
	GPIO2A2_NAND_WRN,
	GPIO2A4_SFC_CSN0,

	GPIO2A1_SHIFT		= 2,
	GPIO2A1_MASK		= 3 << GPIO2A1_SHIFT,
	GPIO2A1_GPIO		= 0,
	GPIO2A1_NAND_CLE,
	GPIO2A1_EMMC_CLKOUT,

	GPIO2A0_SHIFT		= 0,
	GPIO2A0_MASK		= 3 << GPIO2A0_SHIFT,
	GPIO2A0_GPIO		= 0,
	GPIO2A0_NAND_ALE,
	GPIO2A0_SPI_CLK,
};

/* GRF_GPIO2B_IOMUX */
enum {
	GPIO2B7_SHIFT		= 14,
	GPIO2B7_MASK		= 1 << GPIO2B7_SHIFT,
	GPIO2B7_GPIO		= 0,
	GPIO2B7_MAC_RXER,

	GPIO2B6_SHIFT		= 12,
	GPIO2B6_MASK		= 3 << GPIO2B6_SHIFT,
	GPIO2B6_GPIO		= 0,
	GPIO2B6_MAC_CLKOUT,
	GPIO2B6_MAC_CLKIN,

	GPIO2B5_SHIFT		= 10,
	GPIO2B5_MASK		= 1 << GPIO2B5_SHIFT,
	GPIO2B5_GPIO		= 0,
	GPIO2B5_MAC_TXEN,

	GPIO2B4_SHIFT		= 8,
	GPIO2B4_MASK		= 1 << GPIO2B4_SHIFT,
	GPIO2B4_GPIO		= 0,
	GPIO2B4_MAC_MDIO,

	GPIO2B2_SHIFT		= 4,
	GPIO2B2_MASK		= 1 << GPIO2B2_SHIFT,
	GPIO2B2_GPIO		= 0,
	GPIO2B2_MAC_CRS,
};

/* GRF_GPIO2C_IOMUX */
enum {
	GPIO2C7_SHIFT		= 14,
	GPIO2C7_MASK		= 3 << GPIO2C7_SHIFT,
	GPIO2C7_GPIO		= 0,
	GPIO2C7_UART1_SOUT,
	GPIO2C7_TESTCLK_OUT1,

	GPIO2C6_SHIFT		= 12,
	GPIO2C6_MASK		= 1 << GPIO2C6_SHIFT,
	GPIO2C6_GPIO		= 0,
	GPIO2C6_UART1_SIN,

	GPIO2C5_SHIFT		= 10,
	GPIO2C5_MASK		= 1 << GPIO2C5_SHIFT,
	GPIO2C5_GPIO		= 0,
	GPIO2C5_I2C2_SCL,

	GPIO2C4_SHIFT		= 8,
	GPIO2C4_MASK		= 1 << GPIO2C4_SHIFT,
	GPIO2C4_GPIO		= 0,
	GPIO2C4_I2C2_SDA,

	GPIO2C3_SHIFT		= 6,
	GPIO2C3_MASK		= 1 << GPIO2C3_SHIFT,
	GPIO2C3_GPIO		= 0,
	GPIO2C3_MAC_TXD0,

	GPIO2C2_SHIFT		= 4,
	GPIO2C2_MASK		= 1 << GPIO2C2_SHIFT,
	GPIO2C2_GPIO		= 0,
	GPIO2C2_MAC_TXD1,

	GPIO2C1_SHIFT		= 2,
	GPIO2C1_MASK		= 1 << GPIO2C1_SHIFT,
	GPIO2C1_GPIO		= 0,
	GPIO2C1_MAC_RXD0,

	GPIO2C0_SHIFT		= 0,
	GPIO2C0_MASK		= 1 << GPIO2C0_SHIFT,
	GPIO2C0_GPIO		= 0,
	GPIO2C0_MAC_RXD1,
};

/* GRF_GPIO2D_IOMUX */
enum {
	GPIO2D6_SHIFT		= 12,
	GPIO2D6_MASK		= 1 << GPIO2D6_SHIFT,
	GPIO2D6_GPIO		= 0,
	GPIO2D6_I2S_SDO1,

	GPIO2D5_SHIFT		= 10,
	GPIO2D5_MASK		= 1 << GPIO2D5_SHIFT,
	GPIO2D5_GPIO		= 0,
	GPIO2D5_I2S_SDO2,

	GPIO2D4_SHIFT		= 8,
	GPIO2D4_MASK		= 1 << GPIO2D4_SHIFT,
	GPIO2D4_GPIO		= 0,
	GPIO2D4_I2S_SDO3,

	GPIO2D1_SHIFT		= 2,
	GPIO2D1_MASK		= 1 << GPIO2D1_SHIFT,
	GPIO2D1_GPIO		= 0,
	GPIO2D1_MAC_MDC,
};

struct rk3036_pinctrl_priv {
	struct rk3036_grf *grf;
};

static void pinctrl_rk3036_pwm_config(struct rk3036_grf *grf, int pwm_id)
{
	switch (pwm_id) {
	case PERIPH_ID_PWM0:
		rk_clrsetreg(&grf->gpio0d_iomux, GPIO0D2_MASK,
			     GPIO0D2_PWM0 << GPIO0D2_SHIFT);
		break;
	case PERIPH_ID_PWM1:
		rk_clrsetreg(&grf->gpio0a_iomux, GPIO0A0_MASK,
			     GPIO0A0_PWM1 << GPIO0A0_SHIFT);
		break;
	case PERIPH_ID_PWM2:
		rk_clrsetreg(&grf->gpio0a_iomux, GPIO0A1_MASK,
			     GPIO0A1_PWM2 << GPIO0A1_SHIFT);
		break;
	case PERIPH_ID_PWM3:
		rk_clrsetreg(&grf->gpio0a_iomux, GPIO0D3_MASK,
			     GPIO0D3_PWM3 << GPIO0D3_SHIFT);
		break;
	default:
		debug("pwm id = %d iomux error!\n", pwm_id);
		break;
	}
}

static void pinctrl_rk3036_i2c_config(struct rk3036_grf *grf, int i2c_id)
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
	}
}

static void pinctrl_rk3036_spi_config(struct rk3036_grf *grf, int cs)
{
	switch (cs) {
	case 0:
		rk_clrsetreg(&grf->gpio1d_iomux, GPIO1D6_MASK,
			     GPIO1D6_SPI_CSN0 << GPIO1D6_SHIFT);
		break;
	case 1:
		rk_clrsetreg(&grf->gpio1d_iomux, GPIO1D7_MASK,
			     GPIO1D7_SPI_CSN1 << GPIO1D7_SHIFT);
		break;
	}
	rk_clrsetreg(&grf->gpio1d_iomux,
		     GPIO1D5_MASK | GPIO1D4_MASK,
		     GPIO1D5_SPI_TXD << GPIO1D5_SHIFT |
		     GPIO1D4_SPI_RXD << GPIO1D4_SHIFT);

	rk_clrsetreg(&grf->gpio2a_iomux, GPIO2A0_MASK,
		     GPIO2A0_SPI_CLK << GPIO2A0_SHIFT);
}

static void pinctrl_rk3036_uart_config(struct rk3036_grf *grf, int uart_id)
{
	switch (uart_id) {
	case PERIPH_ID_UART0:
		rk_clrsetreg(&grf->gpio0c_iomux,
			     GPIO0C3_MASK | GPIO0C2_MASK |
			     GPIO0C1_MASK |  GPIO0C0_MASK,
			     GPIO0C3_UART0_CTSN << GPIO0C3_SHIFT |
			     GPIO0C2_UART0_RTSN << GPIO0C2_SHIFT |
			     GPIO0C1_UART0_SIN << GPIO0C1_SHIFT |
			     GPIO0C0_UART0_SOUT << GPIO0C0_SHIFT);
		break;
	case PERIPH_ID_UART1:
		rk_clrsetreg(&grf->gpio2c_iomux,
			     GPIO2C7_MASK | GPIO2C6_MASK,
			     GPIO2C7_UART1_SOUT << GPIO2C7_SHIFT |
			     GPIO2C6_UART1_SIN << GPIO2C6_SHIFT);
		break;
	case PERIPH_ID_UART2:
		rk_clrsetreg(&grf->gpio1c_iomux,
			     GPIO1C3_MASK | GPIO1C2_MASK,
			     GPIO1C3_UART2_SOUT << GPIO1C3_SHIFT |
			     GPIO1C2_UART2_SIN << GPIO1C2_SHIFT);
		break;
	}
}

static void pinctrl_rk3036_sdmmc_config(struct rk3036_grf *grf, int mmc_id)
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
			     GPIO2A4_MASK | GPIO2A1_MASK,
			     GPIO2A4_EMMC_CMD << GPIO2A4_SHIFT |
			     GPIO2A1_EMMC_CLKOUT << GPIO2A1_SHIFT);
		break;
	case PERIPH_ID_SDCARD:
		rk_clrsetreg(&grf->gpio1c_iomux, 0xffff,
			     GPIO1C5_MMC0_D3 << GPIO1C5_SHIFT |
			     GPIO1C4_MMC0_D2 << GPIO1C4_SHIFT |
			     GPIO1C3_MMC0_D1 << GPIO1C3_SHIFT |
			     GPIO1C2_MMC0_D0 << GPIO1C2_SHIFT |
			     GPIO1C1_MMC0_DETN << GPIO1C1_SHIFT |
			     GPIO1C0_MMC0_CLKOUT << GPIO1C0_SHIFT);
		break;
	}
}

static int rk3036_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rk3036_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%x, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_PWM0:
	case PERIPH_ID_PWM1:
	case PERIPH_ID_PWM2:
	case PERIPH_ID_PWM3:
		pinctrl_rk3036_pwm_config(priv->grf, func);
		break;
	case PERIPH_ID_I2C0:
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
		pinctrl_rk3036_i2c_config(priv->grf, func);
		break;
	case PERIPH_ID_SPI0:
		pinctrl_rk3036_spi_config(priv->grf, flags);
		break;
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
		pinctrl_rk3036_uart_config(priv->grf, func);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
		pinctrl_rk3036_sdmmc_config(priv->grf, func);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk3036_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[3];
	int ret;

	ret = dev_read_u32_array(periph, "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 14:
		return PERIPH_ID_SDCARD;
	case 16:
		return PERIPH_ID_EMMC;
	case 20:
		return PERIPH_ID_UART0;
	case 21:
		return PERIPH_ID_UART1;
	case 22:
		return PERIPH_ID_UART2;
	case 23:
		return PERIPH_ID_SPI0;
	case 24:
		return PERIPH_ID_I2C0;
	case 25:
		return PERIPH_ID_I2C1;
	case 26:
		return PERIPH_ID_I2C2;
	case 30:
		return PERIPH_ID_PWM0;
	}
	return -ENOENT;
}

static int rk3036_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rk3036_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;
	return rk3036_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops rk3036_pinctrl_ops = {
	.set_state_simple	= rk3036_pinctrl_set_state_simple,
	.request	= rk3036_pinctrl_request,
	.get_periph_id	= rk3036_pinctrl_get_periph_id,
};

static int rk3036_pinctrl_probe(struct udevice *dev)
{
	struct rk3036_pinctrl_priv *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	debug("%s: grf=%p\n", __func__, priv->grf);
	return 0;
}

static const struct udevice_id rk3036_pinctrl_ids[] = {
	{ .compatible = "rockchip,rk3036-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3036) = {
	.name		= "pinctrl_rk3036",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3036_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rk3036_pinctrl_priv),
	.ops		= &rk3036_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= rk3036_pinctrl_probe,
};
