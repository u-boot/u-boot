/*
 * Pinctrl driver for Rockchip SoCs
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk3288.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <asm/arch/pmu_rk3288.h>
#include <dm/pinctrl.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

struct rk3288_pinctrl_priv {
	struct rk3288_grf *grf;
	struct rk3288_pmu *pmu;
};

static void pinctrl_rk3288_pwm_config(struct rk3288_grf *grf, int pwm_id)
{
	switch (pwm_id) {
	case PERIPH_ID_PWM0:
		rk_clrsetreg(&grf->gpio7a_iomux, GPIO7A0_MASK << GPIO7A0_SHIFT,
			     GPIO7A0_PWM_0 << GPIO7A0_SHIFT);
		break;
	case PERIPH_ID_PWM1:
		rk_clrsetreg(&grf->gpio7a_iomux, GPIO7A1_MASK << GPIO7A1_SHIFT,
			     GPIO7A1_PWM_1 << GPIO7A1_SHIFT);
		break;
	case PERIPH_ID_PWM2:
		rk_clrsetreg(&grf->gpio7a_iomux, GPIO7C6_MASK << GPIO7C6_SHIFT,
			     GPIO7C6_PWM_2 << GPIO7C6_SHIFT);
		break;
	case PERIPH_ID_PWM3:
		rk_clrsetreg(&grf->gpio7a_iomux, GPIO7C7_MASK << GPIO7C6_SHIFT,
			     GPIO7C7_PWM_3 << GPIO7C7_SHIFT);
		break;
	default:
		debug("pwm id = %d iomux error!\n", pwm_id);
		break;
	}
}

static void pinctrl_rk3288_i2c_config(struct rk3288_grf *grf,
				      struct rk3288_pmu *pmu, int i2c_id)
{
	switch (i2c_id) {
	case PERIPH_ID_I2C0:
		clrsetbits_le32(&pmu->gpio0b_iomux,
				GPIO0_B7_MASK << GPIO0_B7_SHIFT,
				GPIO0_B7_I2C0PMU_SDA << GPIO0_B7_SHIFT);
		clrsetbits_le32(&pmu->gpio0b_iomux,
				GPIO0_C0_MASK << GPIO0_C0_SHIFT,
				GPIO0_C0_I2C0PMU_SCL << GPIO0_C0_SHIFT);
		break;
	case PERIPH_ID_I2C1:
		rk_clrsetreg(&grf->gpio8a_iomux,
			     GPIO8A4_MASK << GPIO8A4_SHIFT |
			     GPIO8A5_MASK << GPIO8A5_SHIFT,
			     GPIO8A4_I2C2SENSOR_SDA << GPIO8A4_SHIFT |
			     GPIO8A5_I2C2SENSOR_SCL << GPIO8A5_SHIFT);
		break;
	case PERIPH_ID_I2C2:
		rk_clrsetreg(&grf->gpio6b_iomux,
			     GPIO6B1_MASK << GPIO6B1_SHIFT |
			     GPIO6B2_MASK << GPIO6B2_SHIFT,
			     GPIO6B1_I2C1AUDIO_SDA << GPIO6B1_SHIFT |
			     GPIO6B2_I2C1AUDIO_SCL << GPIO6B2_SHIFT);
		break;
	case PERIPH_ID_I2C3:
		rk_clrsetreg(&grf->gpio2c_iomux,
			     GPIO2C1_MASK << GPIO2C1_SHIFT |
			     GPIO2C0_MASK << GPIO2C0_SHIFT,
			     GPIO2C1_I2C3CAM_SDA << GPIO2C1_SHIFT |
			     GPIO2C0_I2C3CAM_SCL << GPIO2C0_SHIFT);
		break;
	case PERIPH_ID_I2C4:
		rk_clrsetreg(&grf->gpio7cl_iomux,
			     GPIO7C1_MASK << GPIO7C1_SHIFT |
			     GPIO7C2_MASK << GPIO7C2_SHIFT,
			     GPIO7C1_I2C4TP_SDA << GPIO7C1_SHIFT |
			     GPIO7C2_I2C4TP_SCL << GPIO7C2_SHIFT);
		break;
	case PERIPH_ID_I2C5:
		rk_clrsetreg(&grf->gpio7cl_iomux,
			     GPIO7C3_MASK << GPIO7C3_SHIFT,
			     GPIO7C3_I2C5HDMI_SDA << GPIO7C3_SHIFT);
		rk_clrsetreg(&grf->gpio7ch_iomux,
			     GPIO7C4_MASK << GPIO7C4_SHIFT,
			     GPIO7C4_I2C5HDMI_SCL << GPIO7C4_SHIFT);
		break;
	default:
		debug("i2c id = %d iomux error!\n", i2c_id);
		break;
	}
}

static void pinctrl_rk3288_lcdc_config(struct rk3288_grf *grf, int lcd_id)
{
	switch (lcd_id) {
	case PERIPH_ID_LCDC0:
		rk_clrsetreg(&grf->gpio1d_iomux,
			     GPIO1D3_MASK << GPIO1D0_SHIFT |
			     GPIO1D2_MASK << GPIO1D2_SHIFT |
			     GPIO1D1_MASK << GPIO1D1_SHIFT |
			     GPIO1D0_MASK << GPIO1D0_SHIFT,
			     GPIO1D3_LCDC0_DCLK << GPIO1D3_SHIFT |
			     GPIO1D2_LCDC0_DEN << GPIO1D2_SHIFT |
			     GPIO1D1_LCDC0_VSYNC << GPIO1D1_SHIFT |
			     GPIO1D0_LCDC0_HSYNC << GPIO1D0_SHIFT);
		break;
	default:
		debug("lcdc id = %d iomux error!\n", lcd_id);
		break;
	}
}

static int pinctrl_rk3288_spi_config(struct rk3288_grf *grf,
				     enum periph_id spi_id, int cs)
{
	switch (spi_id) {
	case PERIPH_ID_SPI0:
		switch (cs) {
		case 0:
			rk_clrsetreg(&grf->gpio5b_iomux,
				     GPIO5B5_MASK << GPIO5B5_SHIFT,
				     GPIO5B5_SPI0_CSN0 << GPIO5B5_SHIFT);
			break;
		case 1:
			rk_clrsetreg(&grf->gpio5c_iomux,
				     GPIO5C0_MASK << GPIO5C0_SHIFT,
				     GPIO5C0_SPI0_CSN1 << GPIO5C0_SHIFT);
			break;
		default:
			goto err;
		}
		rk_clrsetreg(&grf->gpio5b_iomux,
			     GPIO5B7_MASK << GPIO5B7_SHIFT |
			     GPIO5B6_MASK << GPIO5B6_SHIFT |
			     GPIO5B4_MASK << GPIO5B4_SHIFT,
			     GPIO5B7_SPI0_RXD << GPIO5B7_SHIFT |
			     GPIO5B6_SPI0_TXD << GPIO5B6_SHIFT |
			     GPIO5B4_SPI0_CLK << GPIO5B4_SHIFT);
		break;
	case PERIPH_ID_SPI1:
		if (cs != 0)
			goto err;
		rk_clrsetreg(&grf->gpio7b_iomux,
			     GPIO7B6_MASK << GPIO7B6_SHIFT |
			     GPIO7B7_MASK << GPIO7B7_SHIFT |
			     GPIO7B5_MASK << GPIO7B5_SHIFT |
			     GPIO7B4_MASK << GPIO7B4_SHIFT,
			     GPIO7B6_SPI1_RXD << GPIO7B6_SHIFT |
			     GPIO7B7_SPI1_TXD << GPIO7B7_SHIFT |
			     GPIO7B5_SPI1_CSN0 << GPIO7B5_SHIFT |
			     GPIO7B4_SPI1_CLK << GPIO7B4_SHIFT);
		break;
	case PERIPH_ID_SPI2:
		switch (cs) {
		case 0:
			rk_clrsetreg(&grf->gpio8a_iomux,
				     GPIO8A7_MASK << GPIO8A7_SHIFT,
				     GPIO8A7_SPI2_CSN0 << GPIO8A7_SHIFT);
			break;
		case 1:
			rk_clrsetreg(&grf->gpio8a_iomux,
				     GPIO8A3_MASK << GPIO8A3_SHIFT,
				     GPIO8A3_SPI2_CSN1 << GPIO8A3_SHIFT);
			break;
		default:
			goto err;
		}
		rk_clrsetreg(&grf->gpio8b_iomux,
			     GPIO8B1_MASK << GPIO8B1_SHIFT |
			     GPIO8B0_MASK << GPIO8B0_SHIFT,
			     GPIO8B1_SPI2_TXD << GPIO8B1_SHIFT |
			     GPIO8B0_SPI2_RXD << GPIO8B0_SHIFT);
		rk_clrsetreg(&grf->gpio8a_iomux,
			     GPIO8A6_MASK << GPIO8A6_SHIFT,
			     GPIO8A6_SPI2_CLK << GPIO8A6_SHIFT);
		break;
	default:
		goto err;
	}

	return 0;
err:
	debug("rkspi: periph%d cs=%d not supported", spi_id, cs);
	return -ENOENT;
}

static void pinctrl_rk3288_uart_config(struct rk3288_grf *grf, int uart_id)
{
	switch (uart_id) {
	case PERIPH_ID_UART_BT:
		rk_clrsetreg(&grf->gpio4c_iomux,
			     GPIO4C3_MASK << GPIO4C3_SHIFT |
			     GPIO4C2_MASK << GPIO4C2_SHIFT |
			     GPIO4C1_MASK << GPIO4C1_SHIFT |
			     GPIO4C0_MASK << GPIO4C0_SHIFT,
			     GPIO4C3_UART0BT_RTSN << GPIO4C3_SHIFT |
			     GPIO4C2_UART0BT_CTSN << GPIO4C2_SHIFT |
			     GPIO4C1_UART0BT_SOUT << GPIO4C1_SHIFT |
			     GPIO4C0_UART0BT_SIN << GPIO4C0_SHIFT);
		break;
	case PERIPH_ID_UART_BB:
		rk_clrsetreg(&grf->gpio5b_iomux,
			     GPIO5B3_MASK << GPIO5B3_SHIFT |
			     GPIO5B2_MASK << GPIO5B2_SHIFT |
			     GPIO5B1_MASK << GPIO5B1_SHIFT |
			     GPIO5B0_MASK << GPIO5B0_SHIFT,
			     GPIO5B3_UART1BB_RTSN << GPIO5B3_SHIFT |
			     GPIO5B2_UART1BB_CTSN << GPIO5B2_SHIFT |
			     GPIO5B1_UART1BB_SOUT << GPIO5B1_SHIFT |
			     GPIO5B0_UART1BB_SIN << GPIO5B0_SHIFT);
		break;
	case PERIPH_ID_UART_DBG:
		rk_clrsetreg(&grf->gpio7ch_iomux,
			     GPIO7C7_MASK << GPIO7C7_SHIFT |
			     GPIO7C6_MASK << GPIO7C6_SHIFT,
			     GPIO7C7_UART2DBG_SOUT << GPIO7C7_SHIFT |
			     GPIO7C6_UART2DBG_SIN << GPIO7C6_SHIFT);
		break;
	case PERIPH_ID_UART_GPS:
		rk_clrsetreg(&grf->gpio7b_iomux,
			     GPIO7B2_MASK << GPIO7B2_SHIFT |
			     GPIO7B1_MASK << GPIO7B1_SHIFT |
			     GPIO7B0_MASK << GPIO7B0_SHIFT,
			     GPIO7B2_UART3GPS_RTSN << GPIO7B2_SHIFT |
			     GPIO7B1_UART3GPS_CTSN << GPIO7B1_SHIFT |
			     GPIO7B0_UART3GPS_SOUT << GPIO7B0_SHIFT);
		rk_clrsetreg(&grf->gpio7a_iomux,
			     GPIO7A7_MASK << GPIO7A7_SHIFT,
			     GPIO7A7_UART3GPS_SIN << GPIO7A7_SHIFT);
		break;
	case PERIPH_ID_UART_EXP:
		rk_clrsetreg(&grf->gpio5b_iomux,
			     GPIO5B5_MASK << GPIO5B5_SHIFT |
			     GPIO5B4_MASK << GPIO5B4_SHIFT |
			     GPIO5B6_MASK << GPIO5B6_SHIFT |
			     GPIO5B7_MASK << GPIO5B7_SHIFT,
			     GPIO5B5_UART4EXP_RTSN << GPIO5B5_SHIFT |
			     GPIO5B4_UART4EXP_CTSN << GPIO5B4_SHIFT |
			     GPIO5B6_UART4EXP_SOUT << GPIO5B6_SHIFT |
			     GPIO5B7_UART4EXP_SIN << GPIO5B7_SHIFT);
		break;
	default:
		debug("uart id = %d iomux error!\n", uart_id);
		break;
	}
}

static void pinctrl_rk3288_sdmmc_config(struct rk3288_grf *grf, int mmc_id)
{
	switch (mmc_id) {
	case PERIPH_ID_EMMC:
		rk_clrsetreg(&grf->gpio3a_iomux, 0xffff,
			     GPIO3A7_EMMC_DATA7 << GPIO3A7_SHIFT |
			     GPIO3A6_EMMC_DATA6 << GPIO3A6_SHIFT |
			     GPIO3A5_EMMC_DATA5 << GPIO3A5_SHIFT |
			     GPIO3A4_EMMC_DATA4 << GPIO3A4_SHIFT |
			     GPIO3A3_EMMC_DATA3 << GPIO3A3_SHIFT |
			     GPIO3A2_EMMC_DATA2 << GPIO3A2_SHIFT |
			     GPIO3A1_EMMC_DATA1 << GPIO3A1_SHIFT |
			     GPIO3A0_EMMC_DATA0 << GPIO3A0_SHIFT);
		rk_clrsetreg(&grf->gpio3b_iomux, GPIO3B1_MASK << GPIO3B1_SHIFT,
			     GPIO3B1_EMMC_PWREN << GPIO3B1_SHIFT);
		rk_clrsetreg(&grf->gpio3c_iomux,
			     GPIO3C0_MASK << GPIO3C0_SHIFT,
			     GPIO3C0_EMMC_CMD << GPIO3C0_SHIFT);
		break;
	case PERIPH_ID_SDCARD:
		rk_clrsetreg(&grf->gpio6c_iomux, 0xffff,
			     GPIO6C6_SDMMC0_DECTN << GPIO6C6_SHIFT |
			     GPIO6C5_SDMMC0_CMD << GPIO6C5_SHIFT |
			     GPIO6C4_SDMMC0_CLKOUT << GPIO6C4_SHIFT |
			     GPIO6C3_SDMMC0_DATA3 << GPIO6C3_SHIFT |
			     GPIO6C2_SDMMC0_DATA2 << GPIO6C2_SHIFT |
			     GPIO6C1_SDMMC0_DATA1 << GPIO6C1_SHIFT |
			     GPIO6C0_SDMMC0_DATA0 << GPIO6C0_SHIFT);

		/* use sdmmc0 io, disable JTAG function */
		rk_clrsetreg(&grf->soc_con0, 1 << GRF_FORCE_JTAG_SHIFT, 0);
		break;
	default:
		debug("mmc id = %d iomux error!\n", mmc_id);
		break;
	}
}

static void pinctrl_rk3288_hdmi_config(struct rk3288_grf *grf, int hdmi_id)
{
	switch (hdmi_id) {
	case PERIPH_ID_HDMI:
		rk_clrsetreg(&grf->gpio7cl_iomux, GPIO7C3_MASK << GPIO7C3_SHIFT,
			     GPIO7C3_EDPHDMII2C_SDA << GPIO7C3_SHIFT);
		rk_clrsetreg(&grf->gpio7ch_iomux, GPIO7C4_MASK << GPIO7C4_SHIFT,
			     GPIO7C4_EDPHDMII2C_SCL << GPIO7C4_SHIFT);
		break;
	default:
		debug("hdmi id = %d iomux error!\n", hdmi_id);
		break;
	}
}

static int rk3288_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rk3288_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%x, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_PWM0:
	case PERIPH_ID_PWM1:
	case PERIPH_ID_PWM2:
	case PERIPH_ID_PWM3:
	case PERIPH_ID_PWM4:
		pinctrl_rk3288_pwm_config(priv->grf, func);
		break;
	case PERIPH_ID_I2C0:
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
	case PERIPH_ID_I2C3:
	case PERIPH_ID_I2C4:
	case PERIPH_ID_I2C5:
		pinctrl_rk3288_i2c_config(priv->grf, priv->pmu, func);
		break;
	case PERIPH_ID_SPI0:
	case PERIPH_ID_SPI1:
	case PERIPH_ID_SPI2:
		pinctrl_rk3288_spi_config(priv->grf, func, flags);
		break;
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
	case PERIPH_ID_UART4:
		pinctrl_rk3288_uart_config(priv->grf, func);
		break;
	case PERIPH_ID_LCDC0:
	case PERIPH_ID_LCDC1:
		pinctrl_rk3288_lcdc_config(priv->grf, func);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
		pinctrl_rk3288_sdmmc_config(priv->grf, func);
		break;
	case PERIPH_ID_HDMI:
		pinctrl_rk3288_hdmi_config(priv->grf, func);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk3288_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[3];
	int ret;

	ret = fdtdec_get_int_array(gd->fdt_blob, periph->of_offset,
				   "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 44:
		return PERIPH_ID_SPI0;
	case 45:
		return PERIPH_ID_SPI1;
	case 46:
		return PERIPH_ID_SPI2;
	case 60:
		return PERIPH_ID_I2C0;
	case 62: /* Note strange order */
		return PERIPH_ID_I2C1;
	case 61:
		return PERIPH_ID_I2C2;
	case 63:
		return PERIPH_ID_I2C3;
	case 64:
		return PERIPH_ID_I2C4;
	case 65:
		return PERIPH_ID_I2C5;
	}

	return -ENOENT;
}

static int rk3288_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rk3288_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;
	return rk3288_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops rk3288_pinctrl_ops = {
	.set_state_simple	= rk3288_pinctrl_set_state_simple,
	.request	= rk3288_pinctrl_request,
	.get_periph_id	= rk3288_pinctrl_get_periph_id,
};

static int rk3288_pinctrl_bind(struct udevice *dev)
{
	/* scan child GPIO banks */
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

static int rk3288_pinctrl_probe(struct udevice *dev)
{
	struct rk3288_pinctrl_priv *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	priv->pmu = syscon_get_first_range(ROCKCHIP_SYSCON_PMU);
	debug("%s: grf=%p, pmu=%p\n", __func__, priv->grf, priv->pmu);

	return 0;
}

static const struct udevice_id rk3288_pinctrl_ids[] = {
	{ .compatible = "rockchip,rk3288-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3288) = {
	.name		= "pinctrl_rk3288",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3288_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rk3288_pinctrl_priv),
	.ops		= &rk3288_pinctrl_ops,
	.bind		= rk3288_pinctrl_bind,
	.probe		= rk3288_pinctrl_probe,
};
