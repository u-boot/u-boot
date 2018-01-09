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
