// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * (C) 2018 Theobroma Systems Design und Consulting GmbH
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

#if CONFIG_IS_ENABLED(PINCTRL_ROCKCHIP_RK3399_FULL)
static const u32 RK_GRF_P_PULLUP = 1;
static const u32 RK_GRF_P_PULLDOWN = 2;
#endif /* PINCTRL_ROCKCHIP_RK3399_FULL */

struct rk3399_pinctrl_priv {
	struct rk3399_grf_regs *grf;
	struct rk3399_pmugrf_regs *pmugrf;
	struct rockchip_pin_bank *banks;
};

#if CONFIG_IS_ENABLED(PINCTRL_ROCKCHIP_RK3399_FULL)
/* Location of pinctrl/pinconf registers. */
enum rk_grf_location {
	RK_GRF,
	RK_PMUGRF,
};

/**
 * @nr_pins: number of pins in this bank
 * @grf_location: location of pinctrl/pinconf registers
 * @bank_num: number of the bank, to account for holes
 * @iomux: array describing the 4 iomux sources of the bank
 */
struct rockchip_pin_bank {
	u8 nr_pins;
	enum rk_grf_location grf_location;
	size_t iomux_offset;
	size_t pupd_offset;
};

#define PIN_BANK(pins, grf, iomux, pupd)		\
	{						\
		.nr_pins = pins,			\
		.grf_location = grf,			\
		.iomux_offset = iomux,			\
		.pupd_offset = pupd,			\
	}

static struct rockchip_pin_bank rk3399_pin_banks[] = {
	PIN_BANK(16, RK_PMUGRF,
		 offsetof(struct rk3399_pmugrf_regs, gpio0a_iomux),
		 offsetof(struct rk3399_pmugrf_regs, gpio0_p)),
	PIN_BANK(32, RK_PMUGRF,
		 offsetof(struct rk3399_pmugrf_regs, gpio1a_iomux),
		 offsetof(struct rk3399_pmugrf_regs, gpio1_p)),
	PIN_BANK(32, RK_GRF,
		 offsetof(struct rk3399_grf_regs, gpio2a_iomux),
		 offsetof(struct rk3399_grf_regs, gpio2_p)),
	PIN_BANK(32, RK_GRF,
		 offsetof(struct rk3399_grf_regs, gpio3a_iomux),
		 offsetof(struct rk3399_grf_regs, gpio3_p)),
	PIN_BANK(32, RK_GRF,
		 offsetof(struct rk3399_grf_regs, gpio4a_iomux),
		 offsetof(struct rk3399_grf_regs, gpio4_p)),
};

static void rk_pinctrl_get_info(uintptr_t base, u32 index, uintptr_t *addr,
				u32 *shift, u32 *mask)
{
	/*
	 * In general we four subsequent 32-bit configuration registers
	 * per bank (e.g. GPIO2A_P, GPIO2B_P, GPIO2C_P, GPIO2D_P).
	 * The configuration for each pin has two bits.
	 *
	 * @base...contains the address to the first register.
	 * @index...defines the pin within the bank (0..31).
	 * @addr...will be the address of the actual register to use
	 * @shift...will be the bit position in the configuration register
	 * @mask...will be the (unshifted) mask
	 */

	const u32 pins_per_register = 8;
	const u32 config_bits_per_pin = 2;

	/* Get the address of the configuration register. */
	*addr = base + (index / pins_per_register) * sizeof(u32);

	/* Get the bit offset within the configuration register. */
	*shift = (index & (pins_per_register - 1)) * config_bits_per_pin;

	/* Get the (unshifted) mask for the configuration pins. */
	*mask = ((1 << config_bits_per_pin) - 1);

	pr_debug("%s: addr=0x%lx, mask=0x%x, shift=0x%x\n",
		 __func__, *addr, *mask, *shift);
}

static void rk3399_pinctrl_set_pin_iomux(uintptr_t grf_addr,
					 struct rockchip_pin_bank *bank,
					 u32 index, u32 muxval)
{
	uintptr_t iomux_base, addr;
	u32 shift, mask;

	iomux_base = grf_addr + bank->iomux_offset;
	rk_pinctrl_get_info(iomux_base, index, &addr, &shift, &mask);

	/* Set pinmux register */
	rk_clrsetreg(addr, mask << shift, muxval << shift);
}

static void rk3399_pinctrl_set_pin_pupd(uintptr_t grf_addr,
					struct rockchip_pin_bank *bank,
					u32 index, int pinconfig)
{
	uintptr_t pupd_base, addr;
	u32 shift, mask, pupdval;

	/* Fast path in case there's nothing to do. */
	if (!pinconfig)
		return;

	if (pinconfig & (1 << PIN_CONFIG_BIAS_PULL_UP))
		pupdval = RK_GRF_P_PULLUP;
	else if (pinconfig & (1 << PIN_CONFIG_BIAS_PULL_DOWN)) {
		pupdval = RK_GRF_P_PULLDOWN;
	} else {
		/* Flag not supported. */
		pr_warn("%s: Unsupported pinconfig flag: 0x%x\n", __func__,
			pinconfig);
		return;
	}

	pupd_base = grf_addr + (uintptr_t)bank->pupd_offset;
	rk_pinctrl_get_info(pupd_base, index, &addr, &shift, &mask);

	/* Set pull-up/pull-down regisrer */
	rk_clrsetreg(addr, mask << shift, pupdval << shift);
}

static int rk3399_pinctrl_set_pin(struct udevice *dev, u32 banknum, u32 index,
				  u32 muxval, int pinconfig)
{
	struct rk3399_pinctrl_priv *priv = dev_get_priv(dev);
	struct rockchip_pin_bank *bank = &priv->banks[banknum];
	uintptr_t grf_addr;

	pr_debug("%s: 0x%x 0x%x 0x%x 0x%x\n", __func__, banknum, index, muxval,
		 pinconfig);

	if (bank->grf_location == RK_GRF)
		grf_addr = (uintptr_t)priv->grf;
	else if (bank->grf_location == RK_PMUGRF)
		grf_addr = (uintptr_t)priv->pmugrf;
	else
		return -EINVAL;

	rk3399_pinctrl_set_pin_iomux(grf_addr, bank, index, muxval);

	rk3399_pinctrl_set_pin_pupd(grf_addr, bank, index, pinconfig);
	return 0;
}

static int rk3399_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	/*
	 * The order of the fields in this struct must match the order of
	 * the fields in the "rockchip,pins" property.
	 */
	struct rk_pin {
		u32 banknum;
		u32 index;
		u32 muxval;
		u32 phandle;
	} __packed;

	u32 *fields = NULL;
	const int fields_per_pin = 4;
	int num_fields, num_pins;
	int ret;
	int size;
	int i;
	struct rk_pin *pin;

	pr_debug("%s: %s\n", __func__, config->name);

	size = dev_read_size(config, "rockchip,pins");
	if (size < 0)
		return -EINVAL;

	num_fields = size / sizeof(u32);
	num_pins = num_fields / fields_per_pin;

	if (num_fields * sizeof(u32) != size ||
	    num_pins * fields_per_pin != num_fields) {
		pr_warn("Invalid number of rockchip,pins fields.\n");
		return -EINVAL;
	}

	fields = calloc(num_fields, sizeof(u32));
	if (!fields)
		return -ENOMEM;

	ret = dev_read_u32_array(config, "rockchip,pins", fields, num_fields);
	if (ret) {
		pr_warn("%s: Failed to read rockchip,pins fields.\n",
			config->name);
		goto end;
	}

	pin = (struct rk_pin *)fields;
	for (i = 0; i < num_pins; i++, pin++) {
		struct udevice *dev_pinconfig;
		int pinconfig;

		ret = uclass_get_device_by_phandle_id(UCLASS_PINCONFIG,
						      pin->phandle,
						      &dev_pinconfig);
		if (ret) {
			pr_debug("Could not get pinconfig device\n");
			goto end;
		}

		pinconfig = pinctrl_decode_pin_config_dm(dev_pinconfig);
		if (pinconfig < 0) {
			pr_warn("Could not parse pinconfig\n");
			goto end;
		}

		ret = rk3399_pinctrl_set_pin(dev, pin->banknum, pin->index,
					     pin->muxval, pinconfig);
		if (ret) {
			pr_warn("Could not set pinctrl settings\n");
			goto end;
		}
	}

end:
	free(fields);
	return ret;
}

#endif /* PINCTRL_ROCKCHIP_RK3399_FULL */

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
#if CONFIG_IS_ENABLED(PINCTRL_ROCKCHIP_RK3399_FULL)
	.set_state	= rk3399_pinctrl_set_state,
#endif
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
#if CONFIG_IS_ENABLED(PINCTRL_ROCKCHIP_RK3399_FULL)
	priv->banks = rk3399_pin_banks;
#endif /* PINCTRL_ROCKCHIP_RK3399_FULL */

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
