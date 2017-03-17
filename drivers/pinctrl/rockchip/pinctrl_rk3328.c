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

struct rk3328_pinctrl_priv {
	struct rk3328_grf_regs *grf;
};

enum {
	/* GRF_GPIO0A_IOMUX */
	GRF_GPIO0A5_SEL_SHIFT	= 10,
	GRF_GPIO0A5_SEL_MASK	= 3 << GRF_GPIO0A5_SEL_SHIFT,
	GRF_I2C3_SCL		= 2,

	GRF_GPIO0A6_SEL_SHIFT	= 12,
	GRF_GPIO0A6_SEL_MASK	= 3 << GRF_GPIO0A6_SEL_SHIFT,
	GRF_I2C3_SDA		= 2,

	GRF_GPIO0A7_SEL_SHIFT	= 14,
	GRF_GPIO0A7_SEL_MASK	= 3 << GRF_GPIO0A7_SEL_SHIFT,
	GRF_EMMC_DATA0		= 2,

	/* GRF_GPIO1A_IOMUX */
	GRF_GPIO1A0_SEL_SHIFT	= 0,
	GRF_GPIO1A0_SEL_MASK	= 0x3fff << GRF_GPIO1A0_SEL_SHIFT,
	GRF_CARD_DATA_CLK_CMD_DETN	= 0x1555,

	/* GRF_GPIO2A_IOMUX */
	GRF_GPIO2A0_SEL_SHIFT	= 0,
	GRF_GPIO2A0_SEL_MASK	= 3 << GRF_GPIO2A0_SEL_SHIFT,
	GRF_UART2_TX_M1		= 1,

	GRF_GPIO2A1_SEL_SHIFT	= 2,
	GRF_GPIO2A1_SEL_MASK	= 3 << GRF_GPIO2A1_SEL_SHIFT,
	GRF_UART2_RX_M1		= 1,

	GRF_GPIO2A2_SEL_SHIFT	= 4,
	GRF_GPIO2A2_SEL_MASK	= 3 << GRF_GPIO2A2_SEL_SHIFT,
	GRF_PWM_IR		= 1,

	GRF_GPIO2A4_SEL_SHIFT	= 8,
	GRF_GPIO2A4_SEL_MASK	= 3 << GRF_GPIO2A4_SEL_SHIFT,
	GRF_PWM_0		= 1,
	GRF_I2C1_SDA,

	GRF_GPIO2A5_SEL_SHIFT	= 10,
	GRF_GPIO2A5_SEL_MASK	= 3 << GRF_GPIO2A5_SEL_SHIFT,
	GRF_PWM_1		= 1,
	GRF_I2C1_SCL,

	GRF_GPIO2A6_SEL_SHIFT	= 12,
	GRF_GPIO2A6_SEL_MASK	= 3 << GRF_GPIO2A6_SEL_SHIFT,
	GRF_PWM_2		= 1,

	GRF_GPIO2A7_SEL_SHIFT	= 14,
	GRF_GPIO2A7_SEL_MASK	= 3 << GRF_GPIO2A7_SEL_SHIFT,
	GRF_CARD_PWR_EN_M0	= 1,

	/* GRF_GPIO2BL_IOMUX */
	GRF_GPIO2BL0_SEL_SHIFT	= 0,
	GRF_GPIO2BL0_SEL_MASK	= 0x3f << GRF_GPIO2BL0_SEL_SHIFT,
	GRF_SPI_CLK_TX_RX_M0	= 0x15,

	GRF_GPIO2BL3_SEL_SHIFT	= 6,
	GRF_GPIO2BL3_SEL_MASK	= 3 << GRF_GPIO2BL3_SEL_SHIFT,
	GRF_SPI_CSN0_M0		= 1,

	GRF_GPIO2BL4_SEL_SHIFT	= 8,
	GRF_GPIO2BL4_SEL_MASK	= 3 << GRF_GPIO2BL4_SEL_SHIFT,
	GRF_SPI_CSN1_M0		= 1,

	GRF_GPIO2BL5_SEL_SHIFT	= 10,
	GRF_GPIO2BL5_SEL_MASK	= 3 << GRF_GPIO2BL5_SEL_SHIFT,
	GRF_I2C2_SDA		= 1,

	GRF_GPIO2BL6_SEL_SHIFT	= 12,
	GRF_GPIO2BL6_SEL_MASK	= 3 << GRF_GPIO2BL6_SEL_SHIFT,
	GRF_I2C2_SCL		= 1,

	/* GRF_GPIO2D_IOMUX */
	GRF_GPIO2D0_SEL_SHIFT	= 0,
	GRF_GPIO2D0_SEL_MASK	= 3 << GRF_GPIO2D0_SEL_SHIFT,
	GRF_I2C0_SCL		= 1,

	GRF_GPIO2D1_SEL_SHIFT	= 2,
	GRF_GPIO2D1_SEL_MASK	= 3 << GRF_GPIO2D1_SEL_SHIFT,
	GRF_I2C0_SDA		= 1,

	GRF_GPIO2D4_SEL_SHIFT	= 8,
	GRF_GPIO2D4_SEL_MASK	= 0xff << GRF_GPIO2D4_SEL_SHIFT,
	GRF_EMMC_DATA123	= 0xaa,

	/* GRF_GPIO3C_IOMUX */
	GRF_GPIO3C0_SEL_SHIFT	= 0,
	GRF_GPIO3C0_SEL_MASK	= 0x3fff << GRF_GPIO3C0_SEL_SHIFT,
	GRF_EMMC_DATA567_PWR_CLK_RSTN_CMD	= 0x2aaa,

	/* GRF_COM_IOMUX */
	GRF_UART2_IOMUX_SEL_SHIFT	= 0,
	GRF_UART2_IOMUX_SEL_MASK	= 3 << GRF_UART2_IOMUX_SEL_SHIFT,
	GRF_UART2_IOMUX_SEL_M0		= 0,
	GRF_UART2_IOMUX_SEL_M1,

	GRF_SPI_IOMUX_SEL_SHIFT = 4,
	GRF_SPI_IOMUX_SEL_MASK	= 3 << GRF_SPI_IOMUX_SEL_SHIFT,
	GRF_SPI_IOMUX_SEL_M0	= 0,
	GRF_SPI_IOMUX_SEL_M1,
	GRF_SPI_IOMUX_SEL_M2,

	GRF_CARD_IOMUX_SEL_SHIFT	= 7,
	GRF_CARD_IOMUX_SEL_MASK		= 1 << GRF_CARD_IOMUX_SEL_SHIFT,
	GRF_CARD_IOMUX_SEL_M0		= 0,
	GRF_CARD_IOMUX_SEL_M1,
};

static void pinctrl_rk3328_pwm_config(struct rk3328_grf_regs *grf, int pwm_id)
{
	switch (pwm_id) {
	case PERIPH_ID_PWM0:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A4_SEL_MASK,
			     GRF_PWM_0 << GRF_GPIO2A4_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM1:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A5_SEL_MASK,
			     GRF_PWM_1 << GRF_GPIO2A5_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM2:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A6_SEL_MASK,
			     GRF_PWM_2 << GRF_GPIO2A6_SEL_SHIFT);
		break;
	case PERIPH_ID_PWM3:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A2_SEL_MASK,
			     GRF_PWM_IR << GRF_GPIO2A2_SEL_SHIFT);
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
			     GRF_GPIO2D0_SEL_MASK | GRF_GPIO2D1_SEL_MASK,
			     GRF_I2C0_SCL << GRF_GPIO2D0_SEL_SHIFT
			     | GRF_I2C0_SDA << GRF_GPIO2D1_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C1:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A4_SEL_MASK | GRF_GPIO2A5_SEL_MASK,
			     GRF_I2C1_SCL << GRF_GPIO2A5_SEL_SHIFT
			     | GRF_I2C1_SDA << GRF_GPIO2A4_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C2:
		rk_clrsetreg(&grf->gpio2bl_iomux,
			     GRF_GPIO2BL5_SEL_MASK | GRF_GPIO2BL6_SEL_MASK,
			     GRF_I2C2_SCL << GRF_GPIO2BL6_SEL_SHIFT
			     | GRF_I2C2_SDA << GRF_GPIO2BL6_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C3:
		rk_clrsetreg(&grf->gpio0a_iomux,
			     GRF_GPIO0A5_SEL_MASK | GRF_GPIO0A6_SEL_MASK,
			     GRF_I2C3_SCL << GRF_GPIO0A5_SEL_SHIFT
			     | GRF_I2C3_SDA << GRF_GPIO0A6_SEL_SHIFT);
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
	rk_clrsetreg(&grf->com_iomux,
		     GRF_SPI_IOMUX_SEL_MASK,
		     GRF_SPI_IOMUX_SEL_M0 << GRF_SPI_IOMUX_SEL_SHIFT);

	switch (spi_id) {
	case PERIPH_ID_SPI0:
		switch (cs) {
		case 0:
			rk_clrsetreg(&grf->gpio2bl_iomux,
				     GRF_GPIO2BL3_SEL_MASK,
				     GRF_SPI_CSN0_M0 << GRF_GPIO2BL3_SEL_SHIFT);
			break;
		case 1:
			rk_clrsetreg(&grf->gpio2bl_iomux,
				     GRF_GPIO2BL4_SEL_MASK,
				     GRF_SPI_CSN1_M0 << GRF_GPIO2BL4_SEL_SHIFT);
			break;
		default:
			goto err;
		}
		rk_clrsetreg(&grf->gpio2bl_iomux,
			     GRF_GPIO2BL0_SEL_MASK,
			     GRF_SPI_CLK_TX_RX_M0 << GRF_GPIO2BL0_SEL_SHIFT);
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
	switch (uart_id) {
	case PERIPH_ID_UART2:
		break;
		/* uart2 iomux select m1 */
		rk_clrsetreg(&grf->com_iomux,
			     GRF_UART2_IOMUX_SEL_MASK,
			     GRF_UART2_IOMUX_SEL_M1
			     << GRF_UART2_IOMUX_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A0_SEL_MASK | GRF_GPIO2A1_SEL_MASK,
			     GRF_UART2_TX_M1 << GRF_GPIO2A0_SEL_SHIFT |
			     GRF_UART2_RX_M1 << GRF_GPIO2A1_SEL_SHIFT);
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
	switch (mmc_id) {
	case PERIPH_ID_EMMC:
		rk_clrsetreg(&grf->gpio0a_iomux,
			     GRF_GPIO0A7_SEL_MASK,
			     GRF_EMMC_DATA0 << GRF_GPIO0A7_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio2d_iomux,
			     GRF_GPIO2D4_SEL_MASK,
			     GRF_EMMC_DATA123 << GRF_GPIO2D4_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio3c_iomux,
			     GRF_GPIO3C0_SEL_MASK,
			     GRF_EMMC_DATA567_PWR_CLK_RSTN_CMD
			     << GRF_GPIO3C0_SEL_SHIFT);
		break;
	case PERIPH_ID_SDCARD:
		/* sdcard iomux select m0 */
		rk_clrsetreg(&grf->com_iomux,
			     GRF_CARD_IOMUX_SEL_MASK,
			     GRF_CARD_IOMUX_SEL_M0 << GRF_CARD_IOMUX_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GRF_GPIO2A7_SEL_MASK,
			     GRF_CARD_PWR_EN_M0 << GRF_GPIO2A7_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio1a_iomux,
			     GRF_GPIO1A0_SEL_MASK,
			     GRF_CARD_DATA_CLK_CMD_DETN
			     << GRF_GPIO1A0_SEL_SHIFT);
		break;
	default:
		debug("mmc id = %d iomux error!\n", mmc_id);
		break;
	}
}

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

	ret = fdtdec_get_int_array(gd->fdt_blob, periph->of_offset,
				   "interrupts", cell, ARRAY_SIZE(cell));
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
