// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
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

struct rk3399_pinctrl_priv {
	struct rk3399_grf_regs *grf;
	struct rk3399_pmugrf_regs *pmugrf;
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
		rk_clrsetreg(&grf->gpio4a_iomux,
			     GRF_GPIO4A1_SEL_MASK,
			     GRF_I2C1_SDA << GRF_GPIO4A1_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio4a_iomux,
			     GRF_GPIO4A2_SEL_MASK,
			     GRF_I2C1_SCL << GRF_GPIO4A2_SEL_SHIFT);
		break;

	case PERIPH_ID_I2C2:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A0_SEL_MASK,
			     GRF_I2C2_SDA << GRF_GPIO2A0_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A1_SEL_MASK,
			     GRF_I2C2_SCL << GRF_GPIO2A1_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C3:
		rk_clrsetreg(&grf->gpio4c_iomux,
			     GRF_GPIO4C0_SEL_MASK,
			     GRF_HDMII2C_SCL << GRF_GPIO4C0_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio4c_iomux,
			     GRF_GPIO4C1_SEL_MASK,
			     GRF_HDMII2C_SDA << GRF_GPIO4C1_SEL_SHIFT);
		break;

	case PERIPH_ID_I2C4:
		rk_clrsetreg(&pmugrf->gpio1b_iomux,
			     PMUGRF_GPIO1B3_SEL_MASK,
			     PMUGRF_I2C4_SDA << PMUGRF_GPIO1B3_SEL_SHIFT);
		rk_clrsetreg(&pmugrf->gpio1b_iomux,
			     PMUGRF_GPIO1B4_SEL_MASK,
			     PMUGRF_I2C4_SCL << PMUGRF_GPIO1B4_SEL_SHIFT);
		break;

	case PERIPH_ID_I2C7:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A7_SEL_MASK,
			     GRF_I2C7_SDA << GRF_GPIO2A7_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio2b_iomux,
			     GRF_GPIO2B0_SEL_MASK,
			     GRF_I2C7_SCL << GRF_GPIO2B0_SEL_SHIFT);
		break;

	case PERIPH_ID_I2C6:
		rk_clrsetreg(&grf->gpio2b_iomux,
			     GRF_GPIO2B1_SEL_MASK,
			     GRF_I2C6_SDA << GRF_GPIO2B1_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio2b_iomux,
			     GRF_GPIO2B2_SEL_MASK,
			     GRF_I2C6_SDA << GRF_GPIO2B2_SEL_SHIFT);
		break;

	case PERIPH_ID_I2C8:
		rk_clrsetreg(&pmugrf->gpio1c_iomux,
			     PMUGRF_GPIO1C4_SEL_MASK,
			     PMUGRF_I2C8PMU_SDA << PMUGRF_GPIO1C4_SEL_SHIFT);
		rk_clrsetreg(&pmugrf->gpio1c_iomux,
			     PMUGRF_GPIO1C5_SEL_MASK,
			     PMUGRF_I2C8PMU_SCL << PMUGRF_GPIO1C5_SEL_SHIFT);
		break;

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
	case PERIPH_ID_SPI5:
		if (cs != 0)
			goto err;
		rk_clrsetreg(&grf->gpio2c_iomux,
			     GRF_GPIO2C4_SEL_MASK | GRF_GPIO2C5_SEL_MASK
			     | GRF_GPIO2C6_SEL_MASK | GRF_GPIO2C7_SEL_MASK,
			     GRF_SPI5EXPPLUS_RXD << GRF_GPIO2C4_SEL_SHIFT
			     | GRF_SPI5EXPPLUS_TXD << GRF_GPIO2C5_SEL_SHIFT
			     | GRF_SPI5EXPPLUS_CLK << GRF_GPIO2C6_SEL_SHIFT
			     | GRF_SPI5EXPPLUS_CSN0 << GRF_GPIO2C7_SEL_SHIFT);
		break;
	default:
		printf("%s: spi_id %d is not supported.\n", __func__, spi_id);
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

#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
static void pinctrl_rk3399_gmac_config(struct rk3399_grf_regs *grf, int mmc_id)
{
	rk_clrsetreg(&grf->gpio3a_iomux,
		     GRF_GPIO3A0_SEL_MASK | GRF_GPIO3A1_SEL_MASK |
		     GRF_GPIO3A2_SEL_MASK | GRF_GPIO3A3_SEL_MASK |
		     GRF_GPIO3A4_SEL_MASK | GRF_GPIO3A5_SEL_MASK |
		     GRF_GPIO3A6_SEL_MASK | GRF_GPIO3A7_SEL_MASK,
		     GRF_MAC_TXD2 << GRF_GPIO3A0_SEL_SHIFT |
		     GRF_MAC_TXD3 << GRF_GPIO3A1_SEL_SHIFT |
		     GRF_MAC_RXD2 << GRF_GPIO3A2_SEL_SHIFT |
		     GRF_MAC_RXD3 << GRF_GPIO3A3_SEL_SHIFT |
		     GRF_MAC_TXD0 << GRF_GPIO3A4_SEL_SHIFT |
		     GRF_MAC_TXD1 << GRF_GPIO3A5_SEL_SHIFT |
		     GRF_MAC_RXD0 << GRF_GPIO3A6_SEL_SHIFT |
		     GRF_MAC_RXD1 << GRF_GPIO3A7_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio3b_iomux,
		     GRF_GPIO3B0_SEL_MASK | GRF_GPIO3B1_SEL_MASK |
					    GRF_GPIO3B3_SEL_MASK |
		     GRF_GPIO3B4_SEL_MASK | GRF_GPIO3B5_SEL_MASK |
		     GRF_GPIO3B6_SEL_MASK,
		     GRF_MAC_MDC << GRF_GPIO3B0_SEL_SHIFT |
		     GRF_MAC_RXDV << GRF_GPIO3B1_SEL_SHIFT |
		     GRF_MAC_CLK << GRF_GPIO3B3_SEL_SHIFT |
		     GRF_MAC_TXEN << GRF_GPIO3B4_SEL_SHIFT |
		     GRF_MAC_MDIO << GRF_GPIO3B5_SEL_SHIFT |
		     GRF_MAC_RXCLK << GRF_GPIO3B6_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio3c_iomux,
		     GRF_GPIO3C1_SEL_MASK,
		     GRF_MAC_TXCLK << GRF_GPIO3C1_SEL_SHIFT);

	/* Set drive strength for GMAC tx io, value 3 means 13mA */
	rk_clrsetreg(&grf->gpio3_e[0],
		     GRF_GPIO3A0_E_MASK | GRF_GPIO3A1_E_MASK |
		     GRF_GPIO3A4_E_MASK | GRF_GPIO3A5_E0_MASK,
		     3 << GRF_GPIO3A0_E_SHIFT |
		     3 << GRF_GPIO3A1_E_SHIFT |
		     3 << GRF_GPIO3A4_E_SHIFT |
		     1 << GRF_GPIO3A5_E0_SHIFT);
	rk_clrsetreg(&grf->gpio3_e[1],
		     GRF_GPIO3A5_E12_MASK,
		     1 << GRF_GPIO3A5_E12_SHIFT);
	rk_clrsetreg(&grf->gpio3_e[2],
		     GRF_GPIO3B4_E_MASK,
		     3 << GRF_GPIO3B4_E_SHIFT);
	rk_clrsetreg(&grf->gpio3_e[4],
		     GRF_GPIO3C1_E_MASK,
		     3 << GRF_GPIO3C1_E_SHIFT);
}
#endif

#if !defined(CONFIG_SPL_BUILD)
static void pinctrl_rk3399_hdmi_config(struct rk3399_grf_regs *grf, int hdmi_id)
{
	switch (hdmi_id) {
	case PERIPH_ID_HDMI:
		rk_clrsetreg(&grf->gpio4c_iomux,
			     GRF_GPIO4C0_SEL_MASK | GRF_GPIO4C1_SEL_MASK,
			     (GRF_HDMII2C_SCL << GRF_GPIO4C0_SEL_SHIFT) |
			     (GRF_HDMII2C_SDA << GRF_GPIO4C1_SEL_SHIFT));
		break;
	default:
		debug("%s: hdmi_id = %d unsupported\n", __func__, hdmi_id);
		break;
	}
}
#endif

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
	case PERIPH_ID_I2C6:
	case PERIPH_ID_I2C7:
	case PERIPH_ID_I2C8:
		pinctrl_rk3399_i2c_config(priv->grf, priv->pmugrf, func);
		break;
	case PERIPH_ID_SPI0:
	case PERIPH_ID_SPI1:
	case PERIPH_ID_SPI2:
	case PERIPH_ID_SPI3:
	case PERIPH_ID_SPI4:
	case PERIPH_ID_SPI5:
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
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case PERIPH_ID_GMAC:
		pinctrl_rk3399_gmac_config(priv->grf, func);
		break;
#endif
#if !defined(CONFIG_SPL_BUILD)
	case PERIPH_ID_HDMI:
		pinctrl_rk3399_hdmi_config(priv->grf, func);
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk3399_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	u32 cell[3];
	int ret;

	ret = dev_read_u32_array(periph, "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 68:
		return PERIPH_ID_SPI0;
	case 53:
		return PERIPH_ID_SPI1;
	case 52:
		return PERIPH_ID_SPI2;
	case 132:
		return PERIPH_ID_SPI5;
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
	case 37:
		return PERIPH_ID_I2C6;
	case 36:
		return PERIPH_ID_I2C7;
	case 58:
		return PERIPH_ID_I2C8;
	case 65:
		return PERIPH_ID_SDMMC1;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case 12:
		return PERIPH_ID_GMAC;
#endif
#if !defined(CONFIG_SPL_BUILD)
	case 23:
		return PERIPH_ID_HDMI;
#endif
	}
#endif
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
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rk3399_pinctrl_probe,
};
