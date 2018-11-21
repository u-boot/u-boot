// SPDX-License-Identifier: GPL-2.0+
/*
 * Pinctrl driver for Rockchip RK3188 SoCs
 * Copyright (c) 2016 Heiko Stuebner <heiko@sntech.de>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk3188.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <asm/arch/pmu_rk3188.h>
#include <dm/pinctrl.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

/* GRF_GPIO0D_IOMUX */
enum {
	GPIO0D7_SHIFT		= 14,
	GPIO0D7_MASK		= 1,
	GPIO0D7_GPIO		= 0,
	GPIO0D7_SPI1_CSN0,

	GPIO0D6_SHIFT		= 12,
	GPIO0D6_MASK		= 1,
	GPIO0D6_GPIO		= 0,
	GPIO0D6_SPI1_CLK,

	GPIO0D5_SHIFT		= 10,
	GPIO0D5_MASK		= 1,
	GPIO0D5_GPIO		= 0,
	GPIO0D5_SPI1_TXD,

	GPIO0D4_SHIFT		= 8,
	GPIO0D4_MASK		= 1,
	GPIO0D4_GPIO		= 0,
	GPIO0D4_SPI0_RXD,

	GPIO0D3_SHIFT		= 6,
	GPIO0D3_MASK		= 3,
	GPIO0D3_GPIO		= 0,
	GPIO0D3_FLASH_CSN3,
	GPIO0D3_EMMC_RSTN_OUT,

	GPIO0D2_SHIFT		= 4,
	GPIO0D2_MASK		= 3,
	GPIO0D2_GPIO		= 0,
	GPIO0D2_FLASH_CSN2,
	GPIO0D2_EMMC_CMD,

	GPIO0D1_SHIFT		= 2,
	GPIO0D1_MASK		= 1,
	GPIO0D1_GPIO		= 0,
	GPIO0D1_FLASH_CSN1,

	GPIO0D0_SHIFT		= 0,
	GPIO0D0_MASK		= 3,
	GPIO0D0_GPIO		= 0,
	GPIO0D0_FLASH_DQS,
	GPIO0D0_EMMC_CLKOUT
};

/* GRF_GPIO1A_IOMUX */
enum {
	GPIO1A7_SHIFT		= 14,
	GPIO1A7_MASK		= 3,
	GPIO1A7_GPIO		= 0,
	GPIO1A7_UART1_RTS_N,
	GPIO1A7_SPI0_CSN0,

	GPIO1A6_SHIFT		= 12,
	GPIO1A6_MASK		= 3,
	GPIO1A6_GPIO		= 0,
	GPIO1A6_UART1_CTS_N,
	GPIO1A6_SPI0_CLK,

	GPIO1A5_SHIFT		= 10,
	GPIO1A5_MASK		= 3,
	GPIO1A5_GPIO		= 0,
	GPIO1A5_UART1_SOUT,
	GPIO1A5_SPI0_TXD,

	GPIO1A4_SHIFT		= 8,
	GPIO1A4_MASK		= 3,
	GPIO1A4_GPIO		= 0,
	GPIO1A4_UART1_SIN,
	GPIO1A4_SPI0_RXD,

	GPIO1A3_SHIFT		= 6,
	GPIO1A3_MASK		= 1,
	GPIO1A3_GPIO		= 0,
	GPIO1A3_UART0_RTS_N,

	GPIO1A2_SHIFT		= 4,
	GPIO1A2_MASK		= 1,
	GPIO1A2_GPIO		= 0,
	GPIO1A2_UART0_CTS_N,

	GPIO1A1_SHIFT		= 2,
	GPIO1A1_MASK		= 1,
	GPIO1A1_GPIO		= 0,
	GPIO1A1_UART0_SOUT,

	GPIO1A0_SHIFT		= 0,
	GPIO1A0_MASK		= 1,
	GPIO1A0_GPIO		= 0,
	GPIO1A0_UART0_SIN,
};

/* GRF_GPIO1B_IOMUX */
enum {
	GPIO1B7_SHIFT		= 14,
	GPIO1B7_MASK		= 1,
	GPIO1B7_GPIO		= 0,
	GPIO1B7_SPI0_CSN1,

	GPIO1B6_SHIFT		= 12,
	GPIO1B6_MASK		= 3,
	GPIO1B6_GPIO		= 0,
	GPIO1B6_SPDIF_TX,
	GPIO1B6_SPI1_CSN1,

	GPIO1B5_SHIFT		= 10,
	GPIO1B5_MASK		= 3,
	GPIO1B5_GPIO		= 0,
	GPIO1B5_UART3_RTS_N,
	GPIO1B5_RESERVED,

	GPIO1B4_SHIFT		= 8,
	GPIO1B4_MASK		= 3,
	GPIO1B4_GPIO		= 0,
	GPIO1B4_UART3_CTS_N,
	GPIO1B4_GPS_RFCLK,

	GPIO1B3_SHIFT		= 6,
	GPIO1B3_MASK		= 3,
	GPIO1B3_GPIO		= 0,
	GPIO1B3_UART3_SOUT,
	GPIO1B3_GPS_SIG,

	GPIO1B2_SHIFT		= 4,
	GPIO1B2_MASK		= 3,
	GPIO1B2_GPIO		= 0,
	GPIO1B2_UART3_SIN,
	GPIO1B2_GPS_MAG,

	GPIO1B1_SHIFT		= 2,
	GPIO1B1_MASK		= 3,
	GPIO1B1_GPIO		= 0,
	GPIO1B1_UART2_SOUT,
	GPIO1B1_JTAG_TDO,

	GPIO1B0_SHIFT		= 0,
	GPIO1B0_MASK		= 3,
	GPIO1B0_GPIO		= 0,
	GPIO1B0_UART2_SIN,
	GPIO1B0_JTAG_TDI,
};

/* GRF_GPIO1D_IOMUX */
enum {
	GPIO1D7_SHIFT		= 14,
	GPIO1D7_MASK		= 1,
	GPIO1D7_GPIO		= 0,
	GPIO1D7_I2C4_SCL,

	GPIO1D6_SHIFT		= 12,
	GPIO1D6_MASK		= 1,
	GPIO1D6_GPIO		= 0,
	GPIO1D6_I2C4_SDA,

	GPIO1D5_SHIFT		= 10,
	GPIO1D5_MASK		= 1,
	GPIO1D5_GPIO		= 0,
	GPIO1D5_I2C2_SCL,

	GPIO1D4_SHIFT		= 8,
	GPIO1D4_MASK		= 1,
	GPIO1D4_GPIO		= 0,
	GPIO1D4_I2C2_SDA,

	GPIO1D3_SHIFT		= 6,
	GPIO1D3_MASK		= 1,
	GPIO1D3_GPIO		= 0,
	GPIO1D3_I2C1_SCL,

	GPIO1D2_SHIFT		= 4,
	GPIO1D2_MASK		= 1,
	GPIO1D2_GPIO		= 0,
	GPIO1D2_I2C1_SDA,

	GPIO1D1_SHIFT		= 2,
	GPIO1D1_MASK		= 1,
	GPIO1D1_GPIO		= 0,
	GPIO1D1_I2C0_SCL,

	GPIO1D0_SHIFT		= 0,
	GPIO1D0_MASK		= 1,
	GPIO1D0_GPIO		= 0,
	GPIO1D0_I2C0_SDA,
};

/* GRF_GPIO3A_IOMUX */
enum {
	GPIO3A7_SHIFT		= 14,
	GPIO3A7_MASK		= 1,
	GPIO3A7_GPIO		= 0,
	GPIO3A7_SDMMC0_DATA3,

	GPIO3A6_SHIFT		= 12,
	GPIO3A6_MASK		= 1,
	GPIO3A6_GPIO		= 0,
	GPIO3A6_SDMMC0_DATA2,

	GPIO3A5_SHIFT		= 10,
	GPIO3A5_MASK		= 1,
	GPIO3A5_GPIO		= 0,
	GPIO3A5_SDMMC0_DATA1,

	GPIO3A4_SHIFT		= 8,
	GPIO3A4_MASK		= 1,
	GPIO3A4_GPIO		= 0,
	GPIO3A4_SDMMC0_DATA0,

	GPIO3A3_SHIFT		= 6,
	GPIO3A3_MASK		= 1,
	GPIO3A3_GPIO		= 0,
	GPIO3A3_SDMMC0_CMD,

	GPIO3A2_SHIFT		= 4,
	GPIO3A2_MASK		= 1,
	GPIO3A2_GPIO		= 0,
	GPIO3A2_SDMMC0_CLKOUT,

	GPIO3A1_SHIFT		= 2,
	GPIO3A1_MASK		= 1,
	GPIO3A1_GPIO		= 0,
	GPIO3A1_SDMMC0_PWREN,

	GPIO3A0_SHIFT		= 0,
	GPIO3A0_MASK		= 1,
	GPIO3A0_GPIO		= 0,
	GPIO3A0_SDMMC0_RSTN,
};

/* GRF_GPIO3B_IOMUX */
enum {
	GPIO3B7_SHIFT		= 14,
	GPIO3B7_MASK		= 3,
	GPIO3B7_GPIO		= 0,
	GPIO3B7_CIF_DATA11,
	GPIO3B7_I2C3_SCL,

	GPIO3B6_SHIFT		= 12,
	GPIO3B6_MASK		= 3,
	GPIO3B6_GPIO		= 0,
	GPIO3B6_CIF_DATA10,
	GPIO3B6_I2C3_SDA,

	GPIO3B5_SHIFT		= 10,
	GPIO3B5_MASK		= 3,
	GPIO3B5_GPIO		= 0,
	GPIO3B5_CIF_DATA1,
	GPIO3B5_HSADC_DATA9,

	GPIO3B4_SHIFT		= 8,
	GPIO3B4_MASK		= 3,
	GPIO3B4_GPIO		= 0,
	GPIO3B4_CIF_DATA0,
	GPIO3B4_HSADC_DATA8,

	GPIO3B3_SHIFT		= 6,
	GPIO3B3_MASK		= 1,
	GPIO3B3_GPIO		= 0,
	GPIO3B3_CIF_CLKOUT,

	GPIO3B2_SHIFT		= 4,
	GPIO3B2_MASK		= 1,
	GPIO3B2_GPIO		= 0,
	/* no muxes */

	GPIO3B1_SHIFT		= 2,
	GPIO3B1_MASK		= 1,
	GPIO3B1_GPIO		= 0,
	GPIO3B1_SDMMC0_WRITE_PRT,

	GPIO3B0_SHIFT		= 0,
	GPIO3B0_MASK		= 1,
	GPIO3B0_GPIO		= 0,
	GPIO3B0_SDMMC_DETECT_N,
};

/* GRF_GPIO3C_IOMUX */
enum {
	GPIO3C7_SHIFT		= 14,
	GPIO3C7_MASK		= 3,
	GPIO3C7_GPIO		= 0,
	GPIO3C7_SDMMC1_WRITE_PRT,
	GPIO3C7_RMII_CRS_DVALID,
	GPIO3C7_RESERVED,

	GPIO3C6_SHIFT		= 12,
	GPIO3C6_MASK		= 3,
	GPIO3C6_GPIO		= 0,
	GPIO3C6_SDMMC1_DECTN,
	GPIO3C6_RMII_RX_ERR,
	GPIO3C6_RESERVED,

	GPIO3C5_SHIFT		= 10,
	GPIO3C5_MASK		= 3,
	GPIO3C5_GPIO		= 0,
	GPIO3C5_SDMMC1_CLKOUT,
	GPIO3C5_RMII_CLKOUT,
	GPIO3C5_RMII_CLKIN,

	GPIO3C4_SHIFT		= 8,
	GPIO3C4_MASK		= 3,
	GPIO3C4_GPIO		= 0,
	GPIO3C4_SDMMC1_DATA3,
	GPIO3C4_RMII_RXD1,
	GPIO3C4_RESERVED,

	GPIO3C3_SHIFT		= 6,
	GPIO3C3_MASK		= 3,
	GPIO3C3_GPIO		= 0,
	GPIO3C3_SDMMC1_DATA2,
	GPIO3C3_RMII_RXD0,
	GPIO3C3_RESERVED,

	GPIO3C2_SHIFT		= 4,
	GPIO3C2_MASK		= 3,
	GPIO3C2_GPIO		= 0,
	GPIO3C2_SDMMC1_DATA1,
	GPIO3C2_RMII_TXD0,
	GPIO3C2_RESERVED,

	GPIO3C1_SHIFT		= 2,
	GPIO3C1_MASK		= 3,
	GPIO3C1_GPIO		= 0,
	GPIO3C1_SDMMC1_DATA0,
	GPIO3C1_RMII_TXD1,
	GPIO3C1_RESERVED,

	GPIO3C0_SHIFT		= 0,
	GPIO3C0_MASK		= 3,
	GPIO3C0_GPIO		= 0,
	GPIO3C0_SDMMC1_CMD,
	GPIO3C0_RMII_TX_EN,
	GPIO3C0_RESERVED,
};

/* GRF_GPIO3D_IOMUX */
enum {
	GPIO3D6_SHIFT		= 12,
	GPIO3D6_MASK		= 3,
	GPIO3D6_GPIO		= 0,
	GPIO3D6_PWM_3,
	GPIO3D6_JTAG_TMS,
	GPIO3D6_HOST_DRV_VBUS,

	GPIO3D5_SHIFT		= 10,
	GPIO3D5_MASK		= 3,
	GPIO3D5_GPIO		= 0,
	GPIO3D5_PWM_2,
	GPIO3D5_JTAG_TCK,
	GPIO3D5_OTG_DRV_VBUS,

	GPIO3D4_SHIFT		= 8,
	GPIO3D4_MASK		= 3,
	GPIO3D4_GPIO		= 0,
	GPIO3D4_PWM_1,
	GPIO3D4_JTAG_TRSTN,

	GPIO3D3_SHIFT		= 6,
	GPIO3D3_MASK		= 3,
	GPIO3D3_GPIO		= 0,
	GPIO3D3_PWM_0,

	GPIO3D2_SHIFT		= 4,
	GPIO3D2_MASK		= 3,
	GPIO3D2_GPIO		= 0,
	GPIO3D2_SDMMC1_INT_N,

	GPIO3D1_SHIFT		= 2,
	GPIO3D1_MASK		= 3,
	GPIO3D1_GPIO		= 0,
	GPIO3D1_SDMMC1_BACKEND_PWR,
	GPIO3D1_MII_MDCLK,

	GPIO3D0_SHIFT		= 0,
	GPIO3D0_MASK		= 3,
	GPIO3D0_GPIO		= 0,
	GPIO3D0_SDMMC1_PWR_EN,
	GPIO3D0_MII_MD,
};

struct rk3188_pinctrl_priv {
	struct rk3188_grf *grf;
	struct rk3188_pmu *pmu;
	int num_banks;
};

/**
 * Encode variants of iomux registers into a type variable
 */
#define IOMUX_GPIO_ONLY		BIT(0)

/**
 * @type: iomux variant using IOMUX_* constants
 * @offset: if initialized to -1 it will be autocalculated, by specifying
 *	    an initial offset value the relevant source offset can be reset
 *	    to a new value for autocalculating the following iomux registers.
 */
struct rockchip_iomux {
	u8 type;
	s16 offset;
};

/**
 * @reg: register offset of the gpio bank
 * @nr_pins: number of pins in this bank
 * @bank_num: number of the bank, to account for holes
 * @name: name of the bank
 * @iomux: array describing the 4 iomux sources of the bank
 */
struct rockchip_pin_bank {
	u16 reg;
	u8 nr_pins;
	u8 bank_num;
	char *name;
	struct rockchip_iomux iomux[4];
};

#define PIN_BANK(id, pins, label)			\
	{						\
		.bank_num	= id,			\
		.nr_pins	= pins,			\
		.name		= label,		\
		.iomux		= {			\
			{ .offset = -1 },		\
			{ .offset = -1 },		\
			{ .offset = -1 },		\
			{ .offset = -1 },		\
		},					\
	}

#define PIN_BANK_IOMUX_FLAGS(id, pins, label, iom0, iom1, iom2, iom3)	\
	{								\
		.bank_num	= id,					\
		.nr_pins	= pins,					\
		.name		= label,				\
		.iomux		= {					\
			{ .type = iom0, .offset = -1 },			\
			{ .type = iom1, .offset = -1 },			\
			{ .type = iom2, .offset = -1 },			\
			{ .type = iom3, .offset = -1 },			\
		},							\
	}

#ifndef CONFIG_SPL_BUILD
static struct rockchip_pin_bank rk3188_pin_banks[] = {
	PIN_BANK_IOMUX_FLAGS(0, 32, "gpio0", IOMUX_GPIO_ONLY, 0, 0, 0),
	PIN_BANK(1, 32, "gpio1"),
	PIN_BANK(2, 32, "gpio2"),
	PIN_BANK(3, 32, "gpio3"),
};
#endif

static void pinctrl_rk3188_pwm_config(struct rk3188_grf *grf, int pwm_id)
{
	switch (pwm_id) {
	case PERIPH_ID_PWM0:
		rk_clrsetreg(&grf->gpio3d_iomux, GPIO3D3_MASK << GPIO3D3_SHIFT,
			     GPIO3D3_PWM_0 << GPIO3D3_SHIFT);
		break;
	case PERIPH_ID_PWM1:
		rk_clrsetreg(&grf->gpio3d_iomux, GPIO3D4_MASK << GPIO3D4_SHIFT,
			     GPIO3D4_PWM_1 << GPIO3D4_SHIFT);
		break;
	case PERIPH_ID_PWM2:
		rk_clrsetreg(&grf->gpio3d_iomux, GPIO3D5_MASK << GPIO3D5_SHIFT,
			     GPIO3D5_PWM_2 << GPIO3D5_SHIFT);
		break;
	case PERIPH_ID_PWM3:
		rk_clrsetreg(&grf->gpio3d_iomux, GPIO3D6_MASK << GPIO3D6_SHIFT,
			     GPIO3D6_PWM_3 << GPIO3D6_SHIFT);
		break;
	default:
		debug("pwm id = %d iomux error!\n", pwm_id);
		break;
	}
}

static void pinctrl_rk3188_i2c_config(struct rk3188_grf *grf,
				      struct rk3188_pmu *pmu, int i2c_id)
{
	switch (i2c_id) {
	case PERIPH_ID_I2C0:
		rk_clrsetreg(&grf->gpio1d_iomux,
				GPIO1D1_MASK << GPIO1D1_SHIFT |
				GPIO1D0_MASK << GPIO1D0_SHIFT,
				GPIO1D1_I2C0_SCL << GPIO1D1_SHIFT |
				GPIO1D0_I2C0_SDA << GPIO1D0_SHIFT);
		/* enable new i2c controller */
		rk_clrsetreg(&grf->soc_con1, 1 << RKI2C0_SEL_SHIFT,
					     1 << RKI2C0_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C1:
		rk_clrsetreg(&grf->gpio1d_iomux,
				GPIO1D3_MASK << GPIO1D3_SHIFT |
				GPIO1D2_MASK << GPIO1D2_SHIFT,
				GPIO1D3_I2C1_SCL << GPIO1D2_SHIFT |
				GPIO1D2_I2C1_SDA << GPIO1D2_SHIFT);
		rk_clrsetreg(&grf->soc_con1, 1 << RKI2C1_SEL_SHIFT,
					     1 << RKI2C1_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C2:
		rk_clrsetreg(&grf->gpio1d_iomux,
				GPIO1D5_MASK << GPIO1D5_SHIFT |
				GPIO1D4_MASK << GPIO1D4_SHIFT,
				GPIO1D5_I2C2_SCL << GPIO1D5_SHIFT |
				GPIO1D4_I2C2_SDA << GPIO1D4_SHIFT);
		rk_clrsetreg(&grf->soc_con1, 1 << RKI2C2_SEL_SHIFT,
					     1 << RKI2C2_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C3:
		rk_clrsetreg(&grf->gpio3b_iomux,
				GPIO3B7_MASK << GPIO3B7_SHIFT |
				GPIO3B6_MASK << GPIO3B6_SHIFT,
				GPIO3B7_I2C3_SCL << GPIO3B7_SHIFT |
				GPIO3B6_I2C3_SDA << GPIO3B6_SHIFT);
		rk_clrsetreg(&grf->soc_con1, 1 << RKI2C3_SEL_SHIFT,
					     1 << RKI2C3_SEL_SHIFT);
		break;
	case PERIPH_ID_I2C4:
		rk_clrsetreg(&grf->gpio1d_iomux,
				GPIO1D7_MASK << GPIO1D7_SHIFT |
				GPIO1D6_MASK << GPIO1D6_SHIFT,
				GPIO1D7_I2C4_SCL << GPIO1D7_SHIFT |
				GPIO1D6_I2C4_SDA << GPIO1D6_SHIFT);
		rk_clrsetreg(&grf->soc_con1, 1 << RKI2C4_SEL_SHIFT,
					     1 << RKI2C4_SEL_SHIFT);
		break;
	default:
		debug("i2c id = %d iomux error!\n", i2c_id);
		break;
	}
}

static int pinctrl_rk3188_spi_config(struct rk3188_grf *grf,
				     enum periph_id spi_id, int cs)
{
	switch (spi_id) {
	case PERIPH_ID_SPI0:
		switch (cs) {
		case 0:
			rk_clrsetreg(&grf->gpio1a_iomux,
				     GPIO1A7_MASK << GPIO1A7_SHIFT,
				     GPIO1A7_SPI0_CSN0 << GPIO1A7_SHIFT);
			break;
		case 1:
			rk_clrsetreg(&grf->gpio1b_iomux,
				     GPIO1B7_MASK << GPIO1B7_SHIFT,
				     GPIO1B7_SPI0_CSN1 << GPIO1B7_SHIFT);
			break;
		default:
			goto err;
		}
		rk_clrsetreg(&grf->gpio1a_iomux,
			     GPIO1A4_MASK << GPIO1A4_SHIFT |
			     GPIO1A5_MASK << GPIO1A5_SHIFT |
			     GPIO1A6_MASK << GPIO1A6_SHIFT,
			     GPIO1A4_SPI0_RXD << GPIO1A4_SHIFT |
			     GPIO1A5_SPI0_TXD << GPIO1A5_SHIFT |
			     GPIO1A6_SPI0_CLK << GPIO1A6_SHIFT);
		break;
	case PERIPH_ID_SPI1:
		switch (cs) {
		case 0:
			rk_clrsetreg(&grf->gpio0d_iomux,
				     GPIO0D7_MASK << GPIO0D7_SHIFT,
				     GPIO0D7_SPI1_CSN0 << GPIO0D7_SHIFT);
			break;
		case 1:
			rk_clrsetreg(&grf->gpio1b_iomux,
				     GPIO1B6_MASK << GPIO1B6_SHIFT,
				     GPIO1B6_SPI1_CSN1 << GPIO1B6_SHIFT);
			break;
		default:
			goto err;
		}
		rk_clrsetreg(&grf->gpio0d_iomux,
			     GPIO0D4_MASK << GPIO0D4_SHIFT |
			     GPIO0D5_MASK << GPIO0D5_SHIFT |
			     GPIO0D6_MASK << GPIO0D6_SHIFT,
			     GPIO0D4_SPI0_RXD << GPIO0D4_SHIFT |
			     GPIO0D5_SPI1_TXD << GPIO0D5_SHIFT |
			     GPIO0D6_SPI1_CLK << GPIO0D6_SHIFT);
		break;
	default:
		goto err;
	}

	return 0;
err:
	debug("rkspi: periph%d cs=%d not supported", spi_id, cs);
	return -ENOENT;
}

static void pinctrl_rk3188_uart_config(struct rk3188_grf *grf, int uart_id)
{
	switch (uart_id) {
	case PERIPH_ID_UART0:
		rk_clrsetreg(&grf->gpio1a_iomux,
			     GPIO1A3_MASK << GPIO1A3_SHIFT |
			     GPIO1A2_MASK << GPIO1A2_SHIFT |
			     GPIO1A1_MASK << GPIO1A1_SHIFT |
			     GPIO1A0_MASK << GPIO1A0_SHIFT,
			     GPIO1A3_UART0_RTS_N << GPIO1A3_SHIFT |
			     GPIO1A2_UART0_CTS_N << GPIO1A2_SHIFT |
			     GPIO1A1_UART0_SOUT << GPIO1A1_SHIFT |
			     GPIO1A0_UART0_SIN << GPIO1A0_SHIFT);
		break;
	case PERIPH_ID_UART1:
		rk_clrsetreg(&grf->gpio1a_iomux,
			     GPIO1A7_MASK << GPIO1A7_SHIFT |
			     GPIO1A6_MASK << GPIO1A6_SHIFT |
			     GPIO1A5_MASK << GPIO1A5_SHIFT |
			     GPIO1A4_MASK << GPIO1A4_SHIFT,
			     GPIO1A7_UART1_RTS_N << GPIO1A7_SHIFT |
			     GPIO1A6_UART1_CTS_N << GPIO1A6_SHIFT |
			     GPIO1A5_UART1_SOUT << GPIO1A5_SHIFT |
			     GPIO1A4_UART1_SIN << GPIO1A4_SHIFT);
		break;
	case PERIPH_ID_UART2:
		rk_clrsetreg(&grf->gpio1b_iomux,
			     GPIO1B1_MASK << GPIO1B1_SHIFT |
			     GPIO1B0_MASK << GPIO1B0_SHIFT,
			     GPIO1B1_UART2_SOUT << GPIO1B1_SHIFT |
			     GPIO1B0_UART2_SIN << GPIO1B0_SHIFT);
		break;
	case PERIPH_ID_UART3:
		rk_clrsetreg(&grf->gpio1b_iomux,
			     GPIO1B5_MASK << GPIO1B5_SHIFT |
			     GPIO1B4_MASK << GPIO1B4_SHIFT |
			     GPIO1B3_MASK << GPIO1B3_SHIFT |
			     GPIO1B2_MASK << GPIO1B2_SHIFT,
			     GPIO1B5_UART3_RTS_N << GPIO1B5_SHIFT |
			     GPIO1B4_UART3_CTS_N << GPIO1B4_SHIFT |
			     GPIO1B3_UART3_SOUT << GPIO1B3_SHIFT |
			     GPIO1B2_UART3_SIN << GPIO1B2_SHIFT);
		break;
	default:
		debug("uart id = %d iomux error!\n", uart_id);
		break;
	}
}

static void pinctrl_rk3188_sdmmc_config(struct rk3188_grf *grf, int mmc_id)
{
	switch (mmc_id) {
	case PERIPH_ID_EMMC:
		rk_clrsetreg(&grf->soc_con0, 1 << EMMC_FLASH_SEL_SHIFT,
					     1 << EMMC_FLASH_SEL_SHIFT);
		rk_clrsetreg(&grf->gpio0d_iomux,
			     GPIO0D2_MASK << GPIO0D2_SHIFT |
			     GPIO0D0_MASK << GPIO0D0_SHIFT,
			     GPIO0D2_EMMC_CMD << GPIO0D2_SHIFT |
			     GPIO0D0_EMMC_CLKOUT << GPIO0D0_SHIFT);
		break;
	case PERIPH_ID_SDCARD:
		rk_clrsetreg(&grf->gpio3b_iomux,
			     GPIO3B0_MASK << GPIO3B0_SHIFT,
			     GPIO3B0_SDMMC_DETECT_N << GPIO3B0_SHIFT);
		rk_clrsetreg(&grf->gpio3a_iomux,
			     GPIO3A7_MASK << GPIO3A7_SHIFT |
			     GPIO3A6_MASK << GPIO3A6_SHIFT |
			     GPIO3A5_MASK << GPIO3A5_SHIFT |
			     GPIO3A4_MASK << GPIO3A4_SHIFT |
			     GPIO3A3_MASK << GPIO3A3_SHIFT |
			     GPIO3A3_MASK << GPIO3A2_SHIFT,
			     GPIO3A7_SDMMC0_DATA3 << GPIO3A7_SHIFT |
			     GPIO3A6_SDMMC0_DATA2 << GPIO3A6_SHIFT |
			     GPIO3A5_SDMMC0_DATA1 << GPIO3A5_SHIFT |
			     GPIO3A4_SDMMC0_DATA0 << GPIO3A4_SHIFT |
			     GPIO3A3_SDMMC0_CMD << GPIO3A3_SHIFT |
			     GPIO3A2_SDMMC0_CLKOUT << GPIO3A2_SHIFT);
		break;
	default:
		debug("mmc id = %d iomux error!\n", mmc_id);
		break;
	}
}

static int rk3188_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rk3188_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%x, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_PWM0:
	case PERIPH_ID_PWM1:
	case PERIPH_ID_PWM2:
	case PERIPH_ID_PWM3:
	case PERIPH_ID_PWM4:
		pinctrl_rk3188_pwm_config(priv->grf, func);
		break;
	case PERIPH_ID_I2C0:
	case PERIPH_ID_I2C1:
	case PERIPH_ID_I2C2:
	case PERIPH_ID_I2C3:
	case PERIPH_ID_I2C4:
	case PERIPH_ID_I2C5:
		pinctrl_rk3188_i2c_config(priv->grf, priv->pmu, func);
		break;
	case PERIPH_ID_SPI0:
	case PERIPH_ID_SPI1:
	case PERIPH_ID_SPI2:
		pinctrl_rk3188_spi_config(priv->grf, func, flags);
		break;
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
	case PERIPH_ID_UART4:
		pinctrl_rk3188_uart_config(priv->grf, func);
		break;
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
		pinctrl_rk3188_sdmmc_config(priv->grf, func);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk3188_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	u32 cell[3];
	int ret;

	ret = dev_read_u32_array(periph, "interrupts", cell, ARRAY_SIZE(cell));
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
#endif

	return -ENOENT;
}

static int rk3188_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rk3188_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;
	return rk3188_pinctrl_request(dev, func, 0);
}

#ifndef CONFIG_SPL_BUILD
int rk3188_pinctrl_get_pin_info(struct rk3188_pinctrl_priv *priv,
				int banknum, int ind, u32 **addrp, uint *shiftp,
				uint *maskp)
{
	struct rockchip_pin_bank *bank = &rk3188_pin_banks[banknum];
	uint muxnum;
	u32 *addr;

	for (muxnum = 0; muxnum < 4; muxnum++) {
		struct rockchip_iomux *mux = &bank->iomux[muxnum];

		if (ind >= 8) {
			ind -= 8;
			continue;
		}

		addr = &priv->grf->gpio0c_iomux - 2;
		addr += mux->offset;
		*shiftp = ind & 7;
		*maskp = 3;
		*shiftp *= 2;

		debug("%s: addr=%p, mask=%x, shift=%x\n", __func__, addr,
		      *maskp, *shiftp);
		*addrp = addr;
		return 0;
	}

	return -EINVAL;
}

static int rk3188_pinctrl_get_gpio_mux(struct udevice *dev, int banknum,
				       int index)
{
	struct rk3188_pinctrl_priv *priv = dev_get_priv(dev);
	uint shift;
	uint mask;
	u32 *addr;
	int ret;

	ret = rk3188_pinctrl_get_pin_info(priv, banknum, index, &addr, &shift,
					  &mask);
	if (ret)
		return ret;
	return (readl(addr) & mask) >> shift;
}

static int rk3188_pinctrl_set_pins(struct udevice *dev, int banknum, int index,
				   int muxval, int flags)
{
	struct rk3188_pinctrl_priv *priv = dev_get_priv(dev);
	uint shift, ind = index;
	uint mask;
	u32 *addr;
	int ret;

	debug("%s: %x %x %x %x\n", __func__, banknum, index, muxval, flags);
	ret = rk3188_pinctrl_get_pin_info(priv, banknum, index, &addr, &shift,
					  &mask);
	if (ret)
		return ret;
	rk_clrsetreg(addr, mask << shift, muxval << shift);

	/* Handle pullup/pulldown */
	if (flags) {
		uint val = 0;

		if (flags & (1 << PIN_CONFIG_BIAS_PULL_UP))
			val = 1;
		else if (flags & (1 << PIN_CONFIG_BIAS_PULL_DOWN))
			val = 2;

		ind = index >> 3;

		if (banknum == 0 && index < 12) {
			addr = &priv->pmu->gpio0_p[ind];
			shift = (index & 7) * 2;
		} else if (banknum == 0 && index >= 12) {
			addr = &priv->grf->gpio0_p[ind - 1];
			/*
			 * The bits in the grf-registers have an inverse
			 * ordering with the lowest pin being in bits 15:14
			 * and the highest pin in bits 1:0 .
			 */
			shift = (7 - (index & 7)) * 2;
		} else {
			addr = &priv->grf->gpio1_p[banknum - 1][ind];
			shift = (7 - (index & 7)) * 2;
		}
		debug("%s: addr=%p, val=%x, shift=%x\n", __func__, addr, val,
		      shift);
		rk_clrsetreg(addr, 3 << shift, val << shift);
	}

	return 0;
}

static int rk3188_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	const void *blob = gd->fdt_blob;
	int pcfg_node, ret, flags, count, i;
	u32 cell[60], *ptr;

	debug("%s: %s %s\n", __func__, dev->name, config->name);
	ret = fdtdec_get_int_array_count(blob, dev_of_offset(config),
					 "rockchip,pins", cell,
					 ARRAY_SIZE(cell));
	if (ret < 0) {
		debug("%s: bad array %d\n", __func__, ret);
		return -EINVAL;
	}
	count = ret;
	for (i = 0, ptr = cell; i < count; i += 4, ptr += 4) {
		pcfg_node = fdt_node_offset_by_phandle(blob, ptr[3]);
		if (pcfg_node < 0)
			return -EINVAL;
		flags = pinctrl_decode_pin_config(blob, pcfg_node);
		if (flags < 0)
			return flags;

		ret = rk3188_pinctrl_set_pins(dev, ptr[0], ptr[1], ptr[2],
					      flags);
		if (ret)
			return ret;
	}

	return 0;
}
#endif

static struct pinctrl_ops rk3188_pinctrl_ops = {
#ifndef CONFIG_SPL_BUILD
	.set_state	= rk3188_pinctrl_set_state,
	.get_gpio_mux	= rk3188_pinctrl_get_gpio_mux,
#endif
	.set_state_simple	= rk3188_pinctrl_set_state_simple,
	.request	= rk3188_pinctrl_request,
	.get_periph_id	= rk3188_pinctrl_get_periph_id,
};

#ifndef CONFIG_SPL_BUILD
static int rk3188_pinctrl_parse_tables(struct rk3188_pinctrl_priv *priv,
				       struct rockchip_pin_bank *banks,
				       int count)
{
	struct rockchip_pin_bank *bank;
	uint reg, muxnum, banknum;

	reg = 0;
	for (banknum = 0; banknum < count; banknum++) {
		bank = &banks[banknum];
		bank->reg = reg;
		debug("%s: bank %d, reg %x\n", __func__, banknum, reg * 4);
		for (muxnum = 0; muxnum < 4; muxnum++) {
			struct rockchip_iomux *mux = &bank->iomux[muxnum];

			mux->offset = reg;
			reg += 1;
		}
	}

	return 0;
}
#endif

static int rk3188_pinctrl_probe(struct udevice *dev)
{
	struct rk3188_pinctrl_priv *priv = dev_get_priv(dev);
	int ret = 0;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	priv->pmu = syscon_get_first_range(ROCKCHIP_SYSCON_PMU);
	debug("%s: grf=%p, pmu=%p\n", __func__, priv->grf, priv->pmu);
#ifndef CONFIG_SPL_BUILD
	ret = rk3188_pinctrl_parse_tables(priv, rk3188_pin_banks,
					  ARRAY_SIZE(rk3188_pin_banks));
#endif

	return ret;
}

static const struct udevice_id rk3188_pinctrl_ids[] = {
	{ .compatible = "rockchip,rk3188-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3188) = {
	.name		= "rockchip_rk3188_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3188_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rk3188_pinctrl_priv),
	.ops		= &rk3188_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rk3188_pinctrl_probe,
};
