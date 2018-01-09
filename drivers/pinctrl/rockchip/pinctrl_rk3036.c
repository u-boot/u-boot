/*
 * Pinctrl driver for Rockchip 3036 SoCs
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
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

DECLARE_GLOBAL_DATA_PTR;

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
