/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/grf_rk3399.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <asm/arch/clock.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

struct rk3399_pinctrl_priv {
	struct rk3399_grf_regs *grf;
	struct rk3399_pmugrf_regs *pmugrf;
};

enum {
	/* GRF_GPIO2B_IOMUX */
	GRF_GPIO2B1_SEL_SHIFT	= 0,
	GRF_GPIO2B1_SEL_MASK	= 3 << GRF_GPIO2B1_SEL_SHIFT,
	GRF_SPI2TPM_RXD		= 1,
	GRF_GPIO2B2_SEL_SHIFT	= 2,
	GRF_GPIO2B2_SEL_MASK	= 3 << GRF_GPIO2B2_SEL_SHIFT,
	GRF_SPI2TPM_TXD		= 1,
	GRF_GPIO2B3_SEL_SHIFT	= 6,
	GRF_GPIO2B3_SEL_MASK	= 3 << GRF_GPIO2B3_SEL_SHIFT,
	GRF_SPI2TPM_CLK		= 1,
	GRF_GPIO2B4_SEL_SHIFT	= 8,
	GRF_GPIO2B4_SEL_MASK	= 3 << GRF_GPIO2B4_SEL_SHIFT,
	GRF_SPI2TPM_CSN0	= 1,

	/* GRF_GPIO3A_IOMUX */
	GRF_GPIO3A4_SEL_SHIFT	= 8,
	GRF_GPIO3A4_SEL_MASK	= 3 << GRF_GPIO3A4_SEL_SHIFT,
	GRF_SPI0NORCODEC_RXD	= 2,
	GRF_GPIO3A5_SEL_SHIFT	= 10,
	GRF_GPIO3A5_SEL_MASK	= 3 << GRF_GPIO3A5_SEL_SHIFT,
	GRF_SPI0NORCODEC_TXD	= 2,
	GRF_GPIO3A6_SEL_SHIFT	= 12,
	GRF_GPIO3A6_SEL_MASK	= 3 << GRF_GPIO3A6_SEL_SHIFT,
	GRF_SPI0NORCODEC_CLK	= 2,
	GRF_GPIO3A7_SEL_SHIFT	= 14,
	GRF_GPIO3A7_SEL_MASK	= 3 << GRF_GPIO3A7_SEL_SHIFT,
	GRF_SPI0NORCODEC_CSN0	= 2,

	/* GRF_GPIO3B_IOMUX */
	GRF_GPIO3B0_SEL_SHIFT	= 0,
	GRF_GPIO3B0_SEL_MASK	= 3 << GRF_GPIO3B0_SEL_SHIFT,
	GRF_SPI0NORCODEC_CSN1	= 2,

	/* GRF_GPIO4B_IOMUX */
	GRF_GPIO4B0_SEL_SHIFT	= 0,
	GRF_GPIO4B0_SEL_MASK	= 3 << GRF_GPIO4B0_SEL_SHIFT,
	GRF_SDMMC_DATA0		= 1,
	GRF_UART2DBGA_SIN	= 2,
	GRF_GPIO4B1_SEL_SHIFT	= 2,
	GRF_GPIO4B1_SEL_MASK	= 3 << GRF_GPIO4B1_SEL_SHIFT,
	GRF_SDMMC_DATA1		= 1,
	GRF_UART2DBGA_SOUT	= 2,
	GRF_GPIO4B2_SEL_SHIFT	= 4,
	GRF_GPIO4B2_SEL_MASK	= 3 << GRF_GPIO4B2_SEL_SHIFT,
	GRF_SDMMC_DATA2		= 1,
	GRF_GPIO4B3_SEL_SHIFT	= 6,
	GRF_GPIO4B3_SEL_MASK	= 3 << GRF_GPIO4B3_SEL_SHIFT,
	GRF_SDMMC_DATA3		= 1,
	GRF_GPIO4B4_SEL_SHIFT	= 8,
	GRF_GPIO4B4_SEL_MASK	= 3 << GRF_GPIO4B4_SEL_SHIFT,
	GRF_SDMMC_CLKOUT	= 1,
	GRF_GPIO4B5_SEL_SHIFT	= 10,
	GRF_GPIO4B5_SEL_MASK	= 3 << GRF_GPIO4B5_SEL_SHIFT,
	GRF_SDMMC_CMD		= 1,

	/* GRF_GPIO4C_IOMUX */
	GRF_GPIO4C2_SEL_SHIFT	= 4,
	GRF_GPIO4C2_SEL_MASK	= 3 << GRF_GPIO4C2_SEL_SHIFT,
	GRF_PWM_0		= 1,
	GRF_GPIO4C3_SEL_SHIFT	= 6,
	GRF_GPIO4C3_SEL_MASK	= 3 << GRF_GPIO4C3_SEL_SHIFT,
	GRF_UART2DGBC_SIN	= 1,
	GRF_GPIO4C4_SEL_SHIFT	= 8,
	GRF_GPIO4C4_SEL_MASK	= 3 << GRF_GPIO4C4_SEL_SHIFT,
	GRF_UART2DBGC_SOUT	= 1,
	GRF_GPIO4C6_SEL_SHIFT	= 12,
	GRF_GPIO4C6_SEL_MASK	= 3 << GRF_GPIO4C6_SEL_SHIFT,
	GRF_PWM_1		= 1,

	/* PMUGRF_GPIO0A_IOMUX */
	PMUGRF_GPIO0A6_SEL_SHIFT	= 12,
	PMUGRF_GPIO0A6_SEL_MASK	= 3 << PMUGRF_GPIO0A6_SEL_SHIFT,
	PMUGRF_PWM_3A		= 1,

	/* PMUGRF_GPIO1A_IOMUX */
	PMUGRF_GPIO1A7_SEL_SHIFT	= 14,
	PMUGRF_GPIO1A7_SEL_MASK	= 3 << PMUGRF_GPIO1A7_SEL_SHIFT,
	PMUGRF_SPI1EC_RXD	= 2,

	/* PMUGRF_GPIO1B_IOMUX */
	PMUGRF_GPIO1B0_SEL_SHIFT	= 0,
	PMUGRF_GPIO1B0_SEL_MASK = 3 << PMUGRF_GPIO1B0_SEL_SHIFT,
	PMUGRF_SPI1EC_TXD	= 2,
	PMUGRF_GPIO1B1_SEL_SHIFT	= 2,
	PMUGRF_GPIO1B1_SEL_MASK = 3 << PMUGRF_GPIO1B1_SEL_SHIFT,
	PMUGRF_SPI1EC_CLK	= 2,
	PMUGRF_GPIO1B2_SEL_SHIFT	= 4,
	PMUGRF_GPIO1B2_SEL_MASK = 3 << PMUGRF_GPIO1B2_SEL_SHIFT,
	PMUGRF_SPI1EC_CSN0	= 2,
	PMUGRF_GPIO1B6_SEL_SHIFT	= 12,
	PMUGRF_GPIO1B6_SEL_MASK	= 3 << PMUGRF_GPIO1B6_SEL_SHIFT,
	PMUGRF_PWM_3B		= 1,
	PMUGRF_GPIO1B7_SEL_SHIFT	= 14,
	PMUGRF_GPIO1B7_SEL_MASK	= 3 << PMUGRF_GPIO1B7_SEL_SHIFT,
	PMUGRF_I2C0PMU_SDA	= 2,

	/* PMUGRF_GPIO1C_IOMUX */
	PMUGRF_GPIO1C0_SEL_SHIFT	= 0,
	PMUGRF_GPIO1C0_SEL_MASK	= 3 << PMUGRF_GPIO1C0_SEL_SHIFT,
	PMUGRF_I2C0PMU_SCL	= 2,
	PMUGRF_GPIO1C3_SEL_SHIFT	= 6,
	PMUGRF_GPIO1C3_SEL_MASK	= 3 << PMUGRF_GPIO1C3_SEL_SHIFT,
	PMUGRF_PWM_2		= 1,

};
static void pinctrl_rk3399_pwm_config(struct rk3399_grf_regs *grf,
		struct rk3399_pmugrf_regs *pmugrf, int pwm_id)
{
	switch (pwm_id) {
	case PERIPH_ID_PWM0:
		rk_clrsetreg(&grf->gpio4c_iomux,
			     GRF_GPIO4C2_SEL_MASK,
			     GRF_PWM_0 << GRF_GPIO4C2_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM1:
		rk_clrsetreg(&grf->gpio4c_iomux,
			     GRF_GPIO4C6_SEL_MASK,
			     GRF_PWM_1 << GRF_GPIO4C6_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM2:
		rk_clrsetreg(&pmugrf->gpio1c_iomux,
			     PMUGRF_GPIO1C3_SEL_MASK,
			     PMUGRF_PWM_2 << PMUGRF_GPIO1C3_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM3:
		if (readl(&pmugrf->soc_con0) & (1 << 5))
			rk_clrsetreg(&pmugrf->gpio1b_iomux,
				     PMUGRF_GPIO1B6_SEL_MASK,
				     PMUGRF_PWM_3B << PMUGRF_GPIO1B6_SEL_SHIFT);
		else
			rk_clrsetreg(&pmugrf->gpio0a_iomux,
				     PMUGRF_GPIO0A6_SEL_MASK,
				     PMUGRF_PWM_3A << PMUGRF_GPIO0A6_SEL_SHIFT);
		break;
	default:
		debug("pwm id = %d iomux error!\n", pwm_id);
		break;
	}
}

static void pinctrl_rk3399_i2c_config(struct rk3399_grf_regs *grf,
				      struct rk3399_pmugrf_regs *pmugrf,
				      int i2c_id)
{
	switch (i2c_id) {
	case PERIPH_ID_I2C0:
		rk_clrsetreg(&pmugrf->gpio1b_iomux,
			     PMUGRF_GPIO1B7_SEL_MASK,
			     PMUGRF_I2C0PMU_SDA << PMUGRF_GPIO1B7_SEL_SHIFT);
		rk_clrsetreg(&pmugrf->gpio1c_iomux,
			     PMUGRF_GPIO1C0_SEL_MASK,
			     PMUGRF_I2C0PMU_SCL << PMUGRF_GPIO1C0_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
	case PERIPH_ID_I2C3:
	case PERIPH_ID_I2C4:
	case PERIPH_ID_I2C5:
	default:
		debug("i2c id = %d iomux error!\n", i2c_id);
		break;
	}
}

static void pinctrl_rk3399_lcdc_config(struct rk3399_grf_regs *grf, int lcd_id)
{
	switch (lcd_id) {
	case PERIPH_ID_LCDC0:
		break;
	default:
		debug("lcdc id = %d iomux error!\n", lcd_id);
		break;
	}
}

static int pinctrl_rk3399_spi_config(struct rk3399_grf_regs *grf,
				     struct rk3399_pmugrf_regs *pmugrf,
				     enum periph_id spi_id, int cs)
{
	switch (spi_id) {
	case PERIPH_ID_SPI0:
		switch (cs) {
		case 0:
			rk_clrsetreg(&grf->gpio3a_iomux,
				     GRF_GPIO3A7_SEL_MASK,
				     GRF_SPI0NORCODEC_CSN0
				     << GRF_GPIO3A7_SEL_SHIFT);
			break;
		case 1:
			rk_clrsetreg(&grf->gpio3b_iomux,
				     GRF_GPIO3B0_SEL_MASK,
				     GRF_SPI0NORCODEC_CSN1
				     << GRF_GPIO3B0_SEL_SHIFT);
			break;
		default:
			goto err;
		}
		rk_clrsetreg(&grf->gpio3a_iomux,
			     GRF_GPIO3A4_SEL_MASK | GRF_GPIO3A5_SEL_SHIFT
			     | GRF_GPIO3A6_SEL_SHIFT,
			     GRF_SPI0NORCODEC_RXD << GRF_GPIO3A4_SEL_SHIFT
			     | GRF_SPI0NORCODEC_RXD << GRF_GPIO3A5_SEL_SHIFT
			     | GRF_SPI0NORCODEC_RXD << GRF_GPIO3A6_SEL_SHIFT);
		break;
	case PERIPH_ID_SPI1:
		if (cs != 0)
			goto err;
		rk_clrsetreg(&pmugrf->gpio1a_iomux,
			     PMUGRF_GPIO1A7_SEL_MASK,
			     PMUGRF_SPI1EC_RXD << PMUGRF_GPIO1A7_SEL_SHIFT);
		rk_clrsetreg(&pmugrf->gpio1b_iomux,
			     PMUGRF_GPIO1B0_SEL_MASK | PMUGRF_GPIO1B1_SEL_MASK
			     | PMUGRF_GPIO1B2_SEL_MASK,
			     PMUGRF_SPI1EC_TXD << PMUGRF_GPIO1B0_SEL_SHIFT
			     | PMUGRF_SPI1EC_CLK << PMUGRF_GPIO1B1_SEL_SHIFT
			     | PMUGRF_SPI1EC_CSN0 << PMUGRF_GPIO1B2_SEL_SHIFT);
		break;
	case PERIPH_ID_SPI2:
		if (cs != 0)
			goto err;
		rk_clrsetreg(&grf->gpio2b_iomux,
			     GRF_GPIO2B1_SEL_MASK | GRF_GPIO2B2_SEL_MASK
			     | GRF_GPIO2B3_SEL_MASK | GRF_GPIO2B4_SEL_MASK,
			     GRF_SPI2TPM_RXD << GRF_GPIO2B1_SEL_SHIFT
			     | GRF_SPI2TPM_TXD << GRF_GPIO2B2_SEL_SHIFT
			     | GRF_SPI2TPM_CLK << GRF_GPIO2B3_SEL_SHIFT
			     | GRF_SPI2TPM_CSN0 << GRF_GPIO2B4_SEL_SHIFT);
		break;
	default:
		goto err;
	}

	return 0;
err:
	debug("rkspi: periph%d cs=%d not supported", spi_id, cs);
	return -ENOENT;
}

static void pinctrl_rk3399_uart_config(struct rk3399_grf_regs *grf,
				       struct rk3399_pmugrf_regs *pmugrf,
				       int uart_id)
{
	switch (uart_id) {
	case PERIPH_ID_UART2:
		/* Using channel-C by default */
		rk_clrsetreg(&grf->gpio4c_iomux,
			     GRF_GPIO4C3_SEL_MASK,
			     GRF_UART2DGBC_SIN << GRF_GPIO4C3_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio4c_iomux,
			     GRF_GPIO4C4_SEL_MASK,
			     GRF_UART2DBGC_SOUT << GRF_GPIO4C4_SEL_SHIFT);
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

static void pinctrl_rk3399_sdmmc_config(struct rk3399_grf_regs *grf, int mmc_id)
{
	switch (mmc_id) {
	case PERIPH_ID_EMMC:
		break;
	case PERIPH_ID_SDCARD:
		rk_clrsetreg(&grf->gpio4b_iomux,
			     GRF_GPIO4B0_SEL_MASK | GRF_GPIO4B1_SEL_MASK
			     | GRF_GPIO4B2_SEL_MASK | GRF_GPIO4B3_SEL_MASK
			     | GRF_GPIO4B4_SEL_MASK | GRF_GPIO4B5_SEL_MASK,
			     GRF_SDMMC_DATA0 << GRF_GPIO4B0_SEL_SHIFT
			     | GRF_SDMMC_DATA1 << GRF_GPIO4B1_SEL_SHIFT
			     | GRF_SDMMC_DATA2 << GRF_GPIO4B2_SEL_SHIFT
			     | GRF_SDMMC_DATA3 << GRF_GPIO4B3_SEL_SHIFT
			     | GRF_SDMMC_CLKOUT << GRF_GPIO4B4_SEL_SHIFT
			     | GRF_SDMMC_CMD << GRF_GPIO4B5_SEL_SHIFT);
		break;
	default:
		debug("mmc id = %d iomux error!\n", mmc_id);
		break;
	}
}

static int rk3399_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rk3399_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%x, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_PWM0:
	case PERIPH_ID_PWM1:
	case PERIPH_ID_PWM2:
	case PERIPH_ID_PWM3:
	case PERIPH_ID_PWM4:
		pinctrl_rk3399_pwm_config(priv->grf, priv->pmugrf, func);
		break;
	case PERIPH_ID_I2C0:
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
	case PERIPH_ID_I2C3:
	case PERIPH_ID_I2C4:
	case PERIPH_ID_I2C5:
		pinctrl_rk3399_i2c_config(priv->grf, priv->pmugrf, func);
		break;
	case PERIPH_ID_SPI0:
	case PERIPH_ID_SPI1:
	case PERIPH_ID_SPI2:
		pinctrl_rk3399_spi_config(priv->grf, priv->pmugrf, func, flags);
		break;
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
	case PERIPH_ID_UART4:
		pinctrl_rk3399_uart_config(priv->grf, priv->pmugrf, func);
		break;
	case PERIPH_ID_LCDC0:
	case PERIPH_ID_LCDC1:
		pinctrl_rk3399_lcdc_config(priv->grf, func);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
		pinctrl_rk3399_sdmmc_config(priv->grf, func);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk3399_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[3];
	int ret;

	ret = fdtdec_get_int_array(gd->fdt_blob, periph->of_offset,
				   "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 68:
		return PERIPH_ID_SPI0;
	case 53:
		return PERIPH_ID_SPI1;
	case 52:
		return PERIPH_ID_SPI2;
	case 57:
		return PERIPH_ID_I2C0;
	case 59: /* Note strange order */
		return PERIPH_ID_I2C1;
	case 35:
		return PERIPH_ID_I2C2;
	case 34:
		return PERIPH_ID_I2C3;
	case 56:
		return PERIPH_ID_I2C4;
	case 38:
		return PERIPH_ID_I2C5;
	case 65:
		return PERIPH_ID_SDMMC1;
	}

	return -ENOENT;
}

static int rk3399_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rk3399_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;

	return rk3399_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops rk3399_pinctrl_ops = {
	.set_state_simple	= rk3399_pinctrl_set_state_simple,
	.request	= rk3399_pinctrl_request,
	.get_periph_id	= rk3399_pinctrl_get_periph_id,
};

static int rk3399_pinctrl_probe(struct udevice *dev)
{
	struct rk3399_pinctrl_priv *priv = dev_get_priv(dev);
	int ret = 0;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	priv->pmugrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);
	debug("%s: grf=%p, pmugrf=%p\n", __func__, priv->grf, priv->pmugrf);

	return ret;
}

static const struct udevice_id rk3399_pinctrl_ids[] = {
	{ .compatible = "rockchip,rk3399-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3399) = {
	.name		= "rockchip_rk3399_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3399_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rk3399_pinctrl_priv),
	.ops		= &rk3399_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= rk3399_pinctrl_probe,
};
