// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/grf_rk3368.h>
#include <asm/arch/periph.h>
#include <dm/pinctrl.h>

/* PMUGRF_GPIO0B_IOMUX */
enum {
	GPIO0B5_SHIFT           = 10,
	GPIO0B5_MASK            = GENMASK(GPIO0B5_SHIFT + 1, GPIO0B5_SHIFT),
	GPIO0B5_GPIO            = 0,
	GPIO0B5_SPI2_CSN0       = (2 << GPIO0B5_SHIFT),

	GPIO0B4_SHIFT           = 8,
	GPIO0B4_MASK            = GENMASK(GPIO0B4_SHIFT + 1, GPIO0B4_SHIFT),
	GPIO0B4_GPIO            = 0,
	GPIO0B4_SPI2_CLK        = (2 << GPIO0B4_SHIFT),

	GPIO0B3_SHIFT           = 6,
	GPIO0B3_MASK            = GENMASK(GPIO0B3_SHIFT + 1, GPIO0B3_SHIFT),
	GPIO0B3_GPIO            = 0,
	GPIO0B3_SPI2_TXD        = (2 << GPIO0B3_SHIFT),

	GPIO0B2_SHIFT           = 4,
	GPIO0B2_MASK            = GENMASK(GPIO0B2_SHIFT + 1, GPIO0B2_SHIFT),
	GPIO0B2_GPIO            = 0,
	GPIO0B2_SPI2_RXD        = (2 << GPIO0B2_SHIFT),
};

/*GRF_GPIO0C_IOMUX*/
enum {
	GPIO0C7_SHIFT           = 14,
	GPIO0C7_MASK	        = GENMASK(GPIO0C7_SHIFT + 1, GPIO0C7_SHIFT),
	GPIO0C7_GPIO	        = 0,
	GPIO0C7_LCDC_D19        = (1 << GPIO0C7_SHIFT),
	GPIO0C7_TRACE_D9        = (2 << GPIO0C7_SHIFT),
	GPIO0C7_UART1_RTSN      = (3 << GPIO0C7_SHIFT),

	GPIO0C6_SHIFT           = 12,
	GPIO0C6_MASK            = GENMASK(GPIO0C6_SHIFT + 1, GPIO0C6_SHIFT),
	GPIO0C6_GPIO            = 0,
	GPIO0C6_LCDC_D18        = (1 << GPIO0C6_SHIFT),
	GPIO0C6_TRACE_D8        = (2 << GPIO0C6_SHIFT),
	GPIO0C6_UART1_CTSN      = (3 << GPIO0C6_SHIFT),

	GPIO0C5_SHIFT           = 10,
	GPIO0C5_MASK            = GENMASK(GPIO0C5_SHIFT + 1, GPIO0C5_SHIFT),
	GPIO0C5_GPIO            = 0,
	GPIO0C5_LCDC_D17        = (1 << GPIO0C5_SHIFT),
	GPIO0C5_TRACE_D7        = (2 << GPIO0C5_SHIFT),
	GPIO0C5_UART1_SOUT      = (3 << GPIO0C5_SHIFT),

	GPIO0C4_SHIFT           = 8,
	GPIO0C4_MASK            = GENMASK(GPIO0C4_SHIFT + 1, GPIO0C4_SHIFT),
	GPIO0C4_GPIO            = 0,
	GPIO0C4_LCDC_D16        = (1 << GPIO0C4_SHIFT),
	GPIO0C4_TRACE_D6        = (2 << GPIO0C4_SHIFT),
	GPIO0C4_UART1_SIN       = (3 << GPIO0C4_SHIFT),

	GPIO0C3_SHIFT           = 6,
	GPIO0C3_MASK            = GENMASK(GPIO0C3_SHIFT + 1, GPIO0C3_SHIFT),
	GPIO0C3_GPIO            = 0,
	GPIO0C3_LCDC_D15        = (1 << GPIO0C3_SHIFT),
	GPIO0C3_TRACE_D5        = (2 << GPIO0C3_SHIFT),
	GPIO0C3_MCU_JTAG_TDO    = (3 << GPIO0C3_SHIFT),

	GPIO0C2_SHIFT           = 4,
	GPIO0C2_MASK            = GENMASK(GPIO0C2_SHIFT + 1, GPIO0C2_SHIFT),
	GPIO0C2_GPIO            = 0,
	GPIO0C2_LCDC_D14        = (1 << GPIO0C2_SHIFT),
	GPIO0C2_TRACE_D4        = (2 << GPIO0C2_SHIFT),
	GPIO0C2_MCU_JTAG_TDI    = (3 << GPIO0C2_SHIFT),

	GPIO0C1_SHIFT           = 2,
	GPIO0C1_MASK            = GENMASK(GPIO0C1_SHIFT + 1, GPIO0C1_SHIFT),
	GPIO0C1_GPIO            = 0,
	GPIO0C1_LCDC_D13        = (1 << GPIO0C1_SHIFT),
	GPIO0C1_TRACE_D3        = (2 << GPIO0C1_SHIFT),
	GPIO0C1_MCU_JTAG_TRTSN  = (3 << GPIO0C1_SHIFT),

	GPIO0C0_SHIFT           = 0,
	GPIO0C0_MASK            = GENMASK(GPIO0C0_SHIFT + 1, GPIO0C0_SHIFT),
	GPIO0C0_GPIO            = 0,
	GPIO0C0_LCDC_D12        = (1 << GPIO0C0_SHIFT),
	GPIO0C0_TRACE_D2        = (2 << GPIO0C0_SHIFT),
	GPIO0C0_MCU_JTAG_TDO    = (3 << GPIO0C0_SHIFT),
};

/*GRF_GPIO0D_IOMUX*/
enum {
	GPIO0D7_SHIFT           = 14,
	GPIO0D7_MASK            = GENMASK(GPIO0D7_SHIFT + 1, GPIO0D7_SHIFT),
	GPIO0D7_GPIO            = 0,
	GPIO0D7_LCDC_DCLK       = (1 << GPIO0D7_SHIFT),
	GPIO0D7_TRACE_CTL       = (2 << GPIO0D7_SHIFT),
	GPIO0D7_PMU_DEBUG5      = (3 << GPIO0D7_SHIFT),

	GPIO0D6_SHIFT           = 12,
	GPIO0D6_MASK            = GENMASK(GPIO0D6_SHIFT + 1, GPIO0D6_SHIFT),
	GPIO0D6_GPIO            = 0,
	GPIO0D6_LCDC_DEN        = (1 << GPIO0D6_SHIFT),
	GPIO0D6_TRACE_CLK       = (2 << GPIO0D6_SHIFT),
	GPIO0D6_PMU_DEBUG4      = (3 << GPIO0D6_SHIFT),

	GPIO0D5_SHIFT           = 10,
	GPIO0D5_MASK            = GENMASK(GPIO0D5_SHIFT + 1, GPIO0D5_SHIFT),
	GPIO0D5_GPIO            = 0,
	GPIO0D5_LCDC_VSYNC      = (1 << GPIO0D5_SHIFT),
	GPIO0D5_TRACE_D15       = (2 << GPIO0D5_SHIFT),
	GPIO0D5_PMU_DEBUG3      = (3 << GPIO0D5_SHIFT),

	GPIO0D4_SHIFT           = 8,
	GPIO0D4_MASK            = GENMASK(GPIO0D4_SHIFT + 1, GPIO0D4_SHIFT),
	GPIO0D4_GPIO            = 0,
	GPIO0D4_LCDC_HSYNC      = (1 << GPIO0D4_SHIFT),
	GPIO0D4_TRACE_D14       = (2 << GPIO0D4_SHIFT),
	GPIO0D4_PMU_DEBUG2      = (3 << GPIO0D4_SHIFT),

	GPIO0D3_SHIFT           = 6,
	GPIO0D3_MASK            = GENMASK(GPIO0D3_SHIFT + 1, GPIO0D3_SHIFT),
	GPIO0D3_GPIO            = 0,
	GPIO0D3_LCDC_D23        = (1 << GPIO0D3_SHIFT),
	GPIO0D3_TRACE_D13       = (2 << GPIO0D3_SHIFT),
	GPIO0D3_UART4_SIN       = (3 << GPIO0D3_SHIFT),

	GPIO0D2_SHIFT           = 4,
	GPIO0D2_MASK            = GENMASK(GPIO0D2_SHIFT + 1, GPIO0D2_SHIFT),
	GPIO0D2_GPIO            = 0,
	GPIO0D2_LCDC_D22        = (1 << GPIO0D2_SHIFT),
	GPIO0D2_TRACE_D12       = (2 << GPIO0D2_SHIFT),
	GPIO0D2_UART4_SOUT      = (3 << GPIO0D2_SHIFT),

	GPIO0D1_SHIFT           = 2,
	GPIO0D1_MASK            = GENMASK(GPIO0D1_SHIFT + 1, GPIO0D1_SHIFT),
	GPIO0D1_GPIO            = 0,
	GPIO0D1_LCDC_D21        = (1 << GPIO0D1_SHIFT),
	GPIO0D1_TRACE_D11       = (2 << GPIO0D1_SHIFT),
	GPIO0D1_UART4_RTSN      = (3 << GPIO0D1_SHIFT),

	GPIO0D0_SHIFT           = 0,
	GPIO0D0_MASK            = GENMASK(GPIO0D0_SHIFT + 1, GPIO0D0_SHIFT),
	GPIO0D0_GPIO            = 0,
	GPIO0D0_LCDC_D20        = (1 << GPIO0D0_SHIFT),
	GPIO0D0_TRACE_D10       = (2 << GPIO0D0_SHIFT),
	GPIO0D0_UART4_CTSN      = (3 << GPIO0D0_SHIFT),
};

/*GRF_GPIO2A_IOMUX*/
enum {
	GPIO2A7_SHIFT           = 14,
	GPIO2A7_MASK            = GENMASK(GPIO2A7_SHIFT + 1, GPIO2A7_SHIFT),
	GPIO2A7_GPIO            = 0,
	GPIO2A7_SDMMC0_D2       = (1 << GPIO2A7_SHIFT),
	GPIO2A7_JTAG_TCK        = (2 << GPIO2A7_SHIFT),

	GPIO2A6_SHIFT           = 12,
	GPIO2A6_MASK            = GENMASK(GPIO2A6_SHIFT + 1, GPIO2A6_SHIFT),
	GPIO2A6_GPIO            = 0,
	GPIO2A6_SDMMC0_D1       = (1 << GPIO2A6_SHIFT),
	GPIO2A6_UART2_SIN       = (2 << GPIO2A6_SHIFT),

	GPIO2A5_SHIFT           = 10,
	GPIO2A5_MASK            = GENMASK(GPIO2A5_SHIFT + 1, GPIO2A5_SHIFT),
	GPIO2A5_GPIO            = 0,
	GPIO2A5_SDMMC0_D0       = (1 << GPIO2A5_SHIFT),
	GPIO2A5_UART2_SOUT      = (2 << GPIO2A5_SHIFT),

	GPIO2A4_SHIFT           = 8,
	GPIO2A4_MASK            = GENMASK(GPIO2A4_SHIFT + 1, GPIO2A4_SHIFT),
	GPIO2A4_GPIO            = 0,
	GPIO2A4_FLASH_DQS       = (1 << GPIO2A4_SHIFT),
	GPIO2A4_EMMC_CLKOUT     = (2 << GPIO2A4_SHIFT),

	GPIO2A3_SHIFT           = 6,
	GPIO2A3_MASK            = GENMASK(GPIO2A3_SHIFT + 1, GPIO2A3_SHIFT),
	GPIO2A3_GPIO            = 0,
	GPIO2A3_FLASH_CSN3      = (1 << GPIO2A3_SHIFT),
	GPIO2A3_EMMC_RSTNOUT    = (2 << GPIO2A3_SHIFT),

	GPIO2A2_SHIFT           = 4,
	GPIO2A2_MASK            = GENMASK(GPIO2A2_SHIFT + 1, GPIO2A2_SHIFT),
	GPIO2A2_GPIO            = 0,
	GPIO2A2_FLASH_CSN2      = (1 << GPIO2A2_SHIFT),

	GPIO2A1_SHIFT           = 2,
	GPIO2A1_MASK            = GENMASK(GPIO2A1_SHIFT + 1, GPIO2A1_SHIFT),
	GPIO2A1_GPIO            = 0,
	GPIO2A1_FLASH_CSN1      = (1 << GPIO2A1_SHIFT),

	GPIO2A0_SHIFT           = 0,
	GPIO2A0_MASK            = GENMASK(GPIO2A0_SHIFT + 1, GPIO2A0_SHIFT),
	GPIO2A0_GPIO            = 0,
	GPIO2A0_FLASH_CSN0      = (1 << GPIO2A0_SHIFT),
};

/*GRF_GPIO2B_IOMUX*/
enum {
	GPIO2B3_SHIFT           = 6,
	GPIO2B3_MASK            = GENMASK(GPIO2B3_SHIFT + 1, GPIO2B3_SHIFT),
	GPIO2B3_GPIO            = 0,
	GPIO2B3_SDMMC0_DTECTN   = (1 << GPIO2B3_SHIFT),

	GPIO2B2_SHIFT           = 4,
	GPIO2B2_MASK            = GENMASK(GPIO2B2_SHIFT + 1, GPIO2B2_SHIFT),
	GPIO2B2_GPIO            = 0,
	GPIO2B2_SDMMC0_CMD      = (1 << GPIO2B2_SHIFT),

	GPIO2B1_SHIFT           = 2,
	GPIO2B1_MASK            = GENMASK(GPIO2B1_SHIFT + 1, GPIO2B1_SHIFT),
	GPIO2B1_GPIO            = 0,
	GPIO2B1_SDMMC0_CLKOUT   = (1 << GPIO2B1_SHIFT),

	GPIO2B0_SHIFT           = 0,
	GPIO2B0_MASK            = GENMASK(GPIO2B0_SHIFT + 1, GPIO2B0_SHIFT),
	GPIO2B0_GPIO            = 0,
	GPIO2B0_SDMMC0_D3       = (1 << GPIO2B0_SHIFT),
};

/*GRF_GPIO2D_IOMUX*/
enum {
	GPIO2D7_SHIFT           = 14,
	GPIO2D7_MASK            = GENMASK(GPIO2D7_SHIFT + 1, GPIO2D7_SHIFT),
	GPIO2D7_GPIO            = 0,
	GPIO2D7_SDIO0_D3        = (1 << GPIO2D7_SHIFT),

	GPIO2D6_SHIFT           = 12,
	GPIO2D6_MASK            = GENMASK(GPIO2D6_SHIFT + 1, GPIO2D6_SHIFT),
	GPIO2D6_GPIO            = 0,
	GPIO2D6_SDIO0_D2        = (1 << GPIO2D6_SHIFT),

	GPIO2D5_SHIFT           = 10,
	GPIO2D5_MASK            = GENMASK(GPIO2D5_SHIFT + 1, GPIO2D5_SHIFT),
	GPIO2D5_GPIO            = 0,
	GPIO2D5_SDIO0_D1        = (1 << GPIO2D5_SHIFT),

	GPIO2D4_SHIFT           = 8,
	GPIO2D4_MASK            = GENMASK(GPIO2D4_SHIFT + 1, GPIO2D4_SHIFT),
	GPIO2D4_GPIO            = 0,
	GPIO2D4_SDIO0_D0        = (1 << GPIO2D4_SHIFT),

	GPIO2D3_SHIFT           = 6,
	GPIO2D3_MASK            = GENMASK(GPIO2D3_SHIFT + 1, GPIO2D3_SHIFT),
	GPIO2D3_GPIO            = 0,
	GPIO2D3_UART0_RTS0      = (1 << GPIO2D3_SHIFT),

	GPIO2D2_SHIFT           = 4,
	GPIO2D2_MASK            = GENMASK(GPIO2D2_SHIFT + 1, GPIO2D2_SHIFT),
	GPIO2D2_GPIO            = 0,
	GPIO2D2_UART0_CTS0      = (1 << GPIO2D2_SHIFT),

	GPIO2D1_SHIFT           = 2,
	GPIO2D1_MASK            = GENMASK(GPIO2D1_SHIFT + 1, GPIO2D1_SHIFT),
	GPIO2D1_GPIO            = 0,
	GPIO2D1_UART0_SOUT      = (1 << GPIO2D1_SHIFT),

	GPIO2D0_SHIFT           = 0,
	GPIO2D0_MASK            = GENMASK(GPIO2D0_SHIFT + 1, GPIO2D0_SHIFT),
	GPIO2D0_GPIO            = 0,
	GPIO2D0_UART0_SIN       = (1 << GPIO2D0_SHIFT),
};

/* GRF_GPIO1B_IOMUX */
enum {
	GPIO1B7_SHIFT           = 14,
	GPIO1B7_MASK            = GENMASK(GPIO1B7_SHIFT + 1, GPIO1B7_SHIFT),
	GPIO1B7_GPIO            = 0,
	GPIO1B7_SPI1_CSN0       = (2 << GPIO1B7_SHIFT),

	GPIO1B6_SHIFT           = 12,
	GPIO1B6_MASK            = GENMASK(GPIO1B6_SHIFT + 1, GPIO1B6_SHIFT),
	GPIO1B6_GPIO            = 0,
	GPIO1B6_SPI1_CLK        = (2 << GPIO1B6_SHIFT),
};

/* GRF_GPIO1C_IOMUX */
enum {
	GPIO1C7_SHIFT           = 14,
	GPIO1C7_MASK            = GENMASK(GPIO1C7_SHIFT + 1, GPIO1C7_SHIFT),
	GPIO1C7_GPIO            = 0,
	GPIO1C7_EMMC_DATA5      = (2 << GPIO1C7_SHIFT),
	GPIO1C7_SPI0_TXD        = (3 << GPIO1C7_SHIFT),

	GPIO1C6_SHIFT           = 12,
	GPIO1C6_MASK            = GENMASK(GPIO1C6_SHIFT + 1, GPIO1C6_SHIFT),
	GPIO1C6_GPIO            = 0,
	GPIO1C6_EMMC_DATA4      = (2 << GPIO1C6_SHIFT),
	GPIO1C6_SPI0_RXD        = (3 << GPIO1C6_SHIFT),

	GPIO1C5_SHIFT           = 10,
	GPIO1C5_MASK            = GENMASK(GPIO1C5_SHIFT + 1, GPIO1C5_SHIFT),
	GPIO1C5_GPIO            = 0,
	GPIO1C5_EMMC_DATA3      = (2 << GPIO1C5_SHIFT),

	GPIO1C4_SHIFT           = 8,
	GPIO1C4_MASK            = GENMASK(GPIO1C4_SHIFT + 1, GPIO1C4_SHIFT),
	GPIO1C4_GPIO            = 0,
	GPIO1C4_EMMC_DATA2      = (2 << GPIO1C4_SHIFT),

	GPIO1C3_SHIFT           = 6,
	GPIO1C3_MASK            = GENMASK(GPIO1C3_SHIFT + 1, GPIO1C3_SHIFT),
	GPIO1C3_GPIO            = 0,
	GPIO1C3_EMMC_DATA1      = (2 << GPIO1C3_SHIFT),

	GPIO1C2_SHIFT           = 4,
	GPIO1C2_MASK            = GENMASK(GPIO1C2_SHIFT + 1, GPIO1C2_SHIFT),
	GPIO1C2_GPIO            = 0,
	GPIO1C2_EMMC_DATA0      = (2 << GPIO1C2_SHIFT),

	GPIO1C1_SHIFT           = 2,
	GPIO1C1_MASK            = GENMASK(GPIO1C1_SHIFT + 1, GPIO1C1_SHIFT),
	GPIO1C1_GPIO            = 0,
	GPIO1C1_SPI1_RXD        = (2 << GPIO1C1_SHIFT),

	GPIO1C0_SHIFT           = 0,
	GPIO1C0_MASK            = GENMASK(GPIO1C0_SHIFT + 1, GPIO1C0_SHIFT),
	GPIO1C0_GPIO            = 0,
	GPIO1C0_SPI1_TXD        = (2 << GPIO1C0_SHIFT),
};

/* GRF_GPIO1D_IOMUX*/
enum {
	GPIO1D5_SHIFT           = 10,
	GPIO1D5_MASK            = GENMASK(GPIO1D5_SHIFT + 1, GPIO1D5_SHIFT),
	GPIO1D5_GPIO            = 0,
	GPIO1D5_SPI0_CLK        = (2 << GPIO1D5_SHIFT),

	GPIO1D3_SHIFT           = 6,
	GPIO1D3_MASK            = GENMASK(GPIO1D3_SHIFT + 1, GPIO1D3_SHIFT),
	GPIO1D3_GPIO            = 0,
	GPIO1D3_EMMC_PWREN      = (2 << GPIO1D3_SHIFT),

	GPIO1D2_SHIFT           = 4,
	GPIO1D2_MASK            = GENMASK(GPIO1D2_SHIFT + 1, GPIO1D2_SHIFT),
	GPIO1D2_GPIO            = 0,
	GPIO1D2_EMMC_CMD        = (2 << GPIO1D2_SHIFT),

	GPIO1D1_SHIFT           = 2,
	GPIO1D1_MASK            = GENMASK(GPIO1D1_SHIFT + 1, GPIO1D1_SHIFT),
	GPIO1D1_GPIO            = 0,
	GPIO1D1_EMMC_DATA7      = (2 << GPIO1D1_SHIFT),
	GPIO1D1_SPI0_CSN1       = (3 << GPIO1D1_SHIFT),

	GPIO1D0_SHIFT           = 0,
	GPIO1D0_MASK            = GENMASK(GPIO1D0_SHIFT + 1, GPIO1D0_SHIFT),
	GPIO1D0_GPIO            = 0,
	GPIO1D0_EMMC_DATA6      = (2 << GPIO1D0_SHIFT),
	GPIO1D0_SPI0_CSN0       = (3 << GPIO1D0_SHIFT),
};


/*GRF_GPIO3B_IOMUX*/
enum {
	GPIO3B7_SHIFT           = 14,
	GPIO3B7_MASK            = GENMASK(GPIO3B7_SHIFT + 1, GPIO3B7_SHIFT),
	GPIO3B7_GPIO            = 0,
	GPIO3B7_MAC_RXD0        = (1 << GPIO3B7_SHIFT),

	GPIO3B6_SHIFT           = 12,
	GPIO3B6_MASK            = GENMASK(GPIO3B6_SHIFT + 1, GPIO3B6_SHIFT),
	GPIO3B6_GPIO            = 0,
	GPIO3B6_MAC_TXD3        = (1 << GPIO3B6_SHIFT),

	GPIO3B5_SHIFT           = 10,
	GPIO3B5_MASK            = GENMASK(GPIO3B5_SHIFT + 1, GPIO3B5_SHIFT),
	GPIO3B5_GPIO            = 0,
	GPIO3B5_MAC_TXEN        = (1 << GPIO3B5_SHIFT),

	GPIO3B4_SHIFT           = 8,
	GPIO3B4_MASK            = GENMASK(GPIO3B4_SHIFT + 1, GPIO3B4_SHIFT),
	GPIO3B4_GPIO            = 0,
	GPIO3B4_MAC_COL         = (1 << GPIO3B4_SHIFT),

	GPIO3B3_SHIFT           = 6,
	GPIO3B3_MASK            = GENMASK(GPIO3B3_SHIFT + 1, GPIO3B3_SHIFT),
	GPIO3B3_GPIO            = 0,
	GPIO3B3_MAC_CRS         = (1 << GPIO3B3_SHIFT),

	GPIO3B2_SHIFT           = 4,
	GPIO3B2_MASK            = GENMASK(GPIO3B2_SHIFT + 1, GPIO3B2_SHIFT),
	GPIO3B2_GPIO            = 0,
	GPIO3B2_MAC_TXD2        = (1 << GPIO3B2_SHIFT),

	GPIO3B1_SHIFT           = 2,
	GPIO3B1_MASK            = GENMASK(GPIO3B1_SHIFT + 1, GPIO3B1_SHIFT),
	GPIO3B1_GPIO            = 0,
	GPIO3B1_MAC_TXD1        = (1 << GPIO3B1_SHIFT),

	GPIO3B0_SHIFT           = 0,
	GPIO3B0_MASK            = GENMASK(GPIO3B0_SHIFT + 1, GPIO3B0_SHIFT),
	GPIO3B0_GPIO            = 0,
	GPIO3B0_MAC_TXD0        = (1 << GPIO3B0_SHIFT),
	GPIO3B0_PWM0            = (2 << GPIO3B0_SHIFT),
};

/*GRF_GPIO3C_IOMUX*/
enum {
	GPIO3C6_SHIFT           = 12,
	GPIO3C6_MASK            = GENMASK(GPIO3C6_SHIFT + 1, GPIO3C6_SHIFT),
	GPIO3C6_GPIO            = 0,
	GPIO3C6_MAC_CLK         = (1 << GPIO3C6_SHIFT),

	GPIO3C5_SHIFT           = 10,
	GPIO3C5_MASK            = GENMASK(GPIO3C5_SHIFT + 1, GPIO3C5_SHIFT),
	GPIO3C5_GPIO            = 0,
	GPIO3C5_MAC_RXEN        = (1 << GPIO3C5_SHIFT),

	GPIO3C4_SHIFT           = 8,
	GPIO3C4_MASK            = GENMASK(GPIO3C4_SHIFT + 1, GPIO3C4_SHIFT),
	GPIO3C4_GPIO            = 0,
	GPIO3C4_MAC_RXDV        = (1 << GPIO3C4_SHIFT),

	GPIO3C3_SHIFT           = 6,
	GPIO3C3_MASK            = GENMASK(GPIO3C3_SHIFT + 1, GPIO3C3_SHIFT),
	GPIO3C3_GPIO            = 0,
	GPIO3C3_MAC_MDC         = (1 << GPIO3C3_SHIFT),

	GPIO3C2_SHIFT           = 4,
	GPIO3C2_MASK            = GENMASK(GPIO3C2_SHIFT + 1, GPIO3C2_SHIFT),
	GPIO3C2_GPIO            = 0,
	GPIO3C2_MAC_RXD3        = (1 << GPIO3C2_SHIFT),

	GPIO3C1_SHIFT           = 2,
	GPIO3C1_MASK            = GENMASK(GPIO3C1_SHIFT + 1, GPIO3C1_SHIFT),
	GPIO3C1_GPIO            = 0,
	GPIO3C1_MAC_RXD2        = (1 << GPIO3C1_SHIFT),

	GPIO3C0_SHIFT           = 0,
	GPIO3C0_MASK            = GENMASK(GPIO3C0_SHIFT + 1, GPIO3C0_SHIFT),
	GPIO3C0_GPIO            = 0,
	GPIO3C0_MAC_RXD1        = (1 << GPIO3C0_SHIFT),
};

/*GRF_GPIO3D_IOMUX*/
enum {
	GPIO3D4_SHIFT           = 8,
	GPIO3D4_MASK            = GENMASK(GPIO3D4_SHIFT + 1, GPIO3D4_SHIFT),
	GPIO3D4_GPIO            = 0,
	GPIO3D4_MAC_TXCLK       = (1 << GPIO3D4_SHIFT),
	GPIO3D4_SPI1_CNS1       = (2 << GPIO3D4_SHIFT),

	GPIO3D1_SHIFT           = 2,
	GPIO3D1_MASK            = GENMASK(GPIO3D1_SHIFT + 1, GPIO3D1_SHIFT),
	GPIO3D1_GPIO            = 0,
	GPIO3D1_MAC_RXCLK       = (1 << GPIO3D1_SHIFT),

	GPIO3D0_SHIFT           = 0,
	GPIO3D0_MASK            = GENMASK(GPIO3D0_SHIFT + 1, GPIO3D0_SHIFT),
	GPIO3D0_GPIO            = 0,
	GPIO3D0_MAC_MDIO        = (1 << GPIO3D0_SHIFT),
};

struct rk3368_pinctrl_priv {
	struct rk3368_grf *grf;
	struct rk3368_pmu_grf *pmugrf;
};

static void pinctrl_rk3368_uart_config(struct rk3368_pinctrl_priv *priv,
				       int uart_id)
{
	struct rk3368_grf *grf = priv->grf;
	struct rk3368_pmu_grf *pmugrf = priv->pmugrf;

	switch (uart_id) {
	case PERIPH_ID_UART2:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A6_MASK | GPIO2A5_MASK,
			     GPIO2A6_UART2_SIN | GPIO2A5_UART2_SOUT);
		break;
	case PERIPH_ID_UART0:
		break;
	case PERIPH_ID_UART1:
		break;
	case PERIPH_ID_UART3:
		break;
	case PERIPH_ID_UART4:
		rk_clrsetreg(&pmugrf->gpio0d_iomux,
			     GPIO0D0_MASK | GPIO0D1_MASK |
			     GPIO0D2_MASK | GPIO0D3_MASK,
			     GPIO0D0_GPIO | GPIO0D1_GPIO |
			     GPIO0D2_UART4_SOUT | GPIO0D3_UART4_SIN);
		break;
	default:
		debug("uart id = %d iomux error!\n", uart_id);
		break;
	}
}

static void pinctrl_rk3368_spi_config(struct rk3368_pinctrl_priv *priv,
				      int spi_id)
{
	struct rk3368_grf *grf = priv->grf;
	struct rk3368_pmu_grf *pmugrf = priv->pmugrf;

	switch (spi_id) {
	case PERIPH_ID_SPI0:
		/*
		 * eMMC can only be connected with 4 bits, when SPI0 is used.
		 * This is all-or-nothing, so we assume that if someone asks us
		 * to configure SPI0, that their eMMC interface is unused or
		 * configured appropriately.
		 */
		rk_clrsetreg(&grf->gpio1d_iomux,
			     GPIO1D0_MASK | GPIO1D1_MASK |
			     GPIO1D5_MASK,
			     GPIO1D0_SPI0_CSN0 | GPIO1D1_SPI0_CSN1 |
			     GPIO1D5_SPI0_CLK);
		rk_clrsetreg(&grf->gpio1c_iomux,
			     GPIO1C6_MASK | GPIO1C7_MASK,
			     GPIO1C6_SPI0_RXD | GPIO1C7_SPI0_TXD);
		break;
	case PERIPH_ID_SPI1:
		/*
		 * We don't implement support for configuring SPI1_CSN#1, as it
		 * conflicts with the GMAC (MAC TX clk-out).
		 */
		rk_clrsetreg(&grf->gpio1b_iomux,
			     GPIO1B6_MASK | GPIO1B7_MASK,
			     GPIO1B6_SPI1_CLK | GPIO1B7_SPI1_CSN0);
		rk_clrsetreg(&grf->gpio1c_iomux,
			     GPIO1C0_MASK | GPIO1C1_MASK,
			     GPIO1C0_SPI1_TXD | GPIO1C1_SPI1_RXD);
		break;
	case PERIPH_ID_SPI2:
		rk_clrsetreg(&pmugrf->gpio0b_iomux,
			     GPIO0B2_MASK | GPIO0B3_MASK |
			     GPIO0B4_MASK | GPIO0B5_MASK,
			     GPIO0B2_SPI2_RXD | GPIO0B3_SPI2_TXD |
			     GPIO0B4_SPI2_CLK | GPIO0B5_SPI2_CSN0);
		break;
	default:
		debug("%s: spi id = %d iomux error!\n", __func__, spi_id);
		break;
	}
}

#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
static void pinctrl_rk3368_gmac_config(struct rk3368_grf *grf, int gmac_id)
{
	rk_clrsetreg(&grf->gpio3b_iomux,
		     GPIO3B0_MASK | GPIO3B1_MASK |
		     GPIO3B2_MASK | GPIO3B5_MASK |
		     GPIO3B6_MASK | GPIO3B7_MASK,
		     GPIO3B0_MAC_TXD0 | GPIO3B1_MAC_TXD1 |
		     GPIO3B2_MAC_TXD2 | GPIO3B5_MAC_TXEN |
		     GPIO3B6_MAC_TXD3 | GPIO3B7_MAC_RXD0);
	rk_clrsetreg(&grf->gpio3c_iomux,
		     GPIO3C0_MASK | GPIO3C1_MASK |
		     GPIO3C2_MASK | GPIO3C3_MASK |
		     GPIO3C4_MASK | GPIO3C5_MASK |
		     GPIO3C6_MASK,
		     GPIO3C0_MAC_RXD1 | GPIO3C1_MAC_RXD2 |
		     GPIO3C2_MAC_RXD3 | GPIO3C3_MAC_MDC |
		     GPIO3C4_MAC_RXDV | GPIO3C5_MAC_RXEN |
		     GPIO3C6_MAC_CLK);
	rk_clrsetreg(&grf->gpio3d_iomux,
		     GPIO3D0_MASK | GPIO3D1_MASK |
		     GPIO3D4_MASK,
		     GPIO3D0_MAC_MDIO | GPIO3D1_MAC_RXCLK |
		     GPIO3D4_MAC_TXCLK);
}
#endif

static void pinctrl_rk3368_sdmmc_config(struct rk3368_grf *grf, int mmc_id)
{
	switch (mmc_id) {
	case PERIPH_ID_EMMC:
		debug("mmc id = %d setting registers!\n", mmc_id);
		rk_clrsetreg(&grf->gpio1c_iomux,
			     GPIO1C2_MASK | GPIO1C3_MASK |
			     GPIO1C4_MASK | GPIO1C5_MASK |
			     GPIO1C6_MASK | GPIO1C7_MASK,
			     GPIO1C2_EMMC_DATA0 |
			     GPIO1C3_EMMC_DATA1 |
			     GPIO1C4_EMMC_DATA2 |
			     GPIO1C5_EMMC_DATA3 |
			     GPIO1C6_EMMC_DATA4 |
			     GPIO1C7_EMMC_DATA5);
		rk_clrsetreg(&grf->gpio1d_iomux,
			     GPIO1D0_MASK | GPIO1D1_MASK |
			     GPIO1D2_MASK | GPIO1D3_MASK,
			     GPIO1D0_EMMC_DATA6 |
			     GPIO1D1_EMMC_DATA7 |
			     GPIO1D2_EMMC_CMD |
			     GPIO1D3_EMMC_PWREN);
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A3_MASK | GPIO2A4_MASK,
			     GPIO2A3_EMMC_RSTNOUT |
			     GPIO2A4_EMMC_CLKOUT);
		break;
	case PERIPH_ID_SDCARD:
		debug("mmc id = %d setting registers!\n", mmc_id);
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A5_MASK | GPIO2A7_MASK |
			     GPIO2A7_MASK,
			     GPIO2A5_SDMMC0_D0 | GPIO2A6_SDMMC0_D1 |
			     GPIO2A7_SDMMC0_D2);
		rk_clrsetreg(&grf->gpio2b_iomux,
			     GPIO2B0_MASK | GPIO2B1_MASK |
			     GPIO2B2_MASK | GPIO2B3_MASK,
			     GPIO2B0_SDMMC0_D3 | GPIO2B1_SDMMC0_CLKOUT |
			     GPIO2B2_SDMMC0_CMD | GPIO2B3_SDMMC0_DTECTN);
		break;
	default:
		debug("mmc id = %d iomux error!\n", mmc_id);
		break;
	}
}

static int rk3368_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rk3368_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%d, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
	case PERIPH_ID_UART4:
		pinctrl_rk3368_uart_config(priv, func);
		break;
	case PERIPH_ID_SPI0:
	case PERIPH_ID_SPI1:
	case PERIPH_ID_SPI2:
		pinctrl_rk3368_spi_config(priv, func);
		break;
	case PERIPH_ID_EMMC:
	case PERIPH_ID_SDCARD:
		pinctrl_rk3368_sdmmc_config(priv->grf, func);
		break;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case PERIPH_ID_GMAC:
		pinctrl_rk3368_gmac_config(priv->grf, func);
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk3368_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[3];
	int ret;

	ret = dev_read_u32_array(periph, "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 59:
		return PERIPH_ID_UART4;
	case 58:
		return PERIPH_ID_UART3;
	case 57:
		return PERIPH_ID_UART2;
	case 56:
		return PERIPH_ID_UART1;
	case 55:
		return PERIPH_ID_UART0;
	case 44:
		return PERIPH_ID_SPI0;
	case 45:
		return PERIPH_ID_SPI1;
	case 41:
		return PERIPH_ID_SPI2;
	case 35:
		return PERIPH_ID_EMMC;
	case 32:
		return PERIPH_ID_SDCARD;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case 27:
		return PERIPH_ID_GMAC;
#endif
	}

	return -ENOENT;
}

static int rk3368_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rk3368_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;

	return rk3368_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops rk3368_pinctrl_ops = {
	.set_state_simple	= rk3368_pinctrl_set_state_simple,
	.request	= rk3368_pinctrl_request,
	.get_periph_id	= rk3368_pinctrl_get_periph_id,
};

static int rk3368_pinctrl_probe(struct udevice *dev)
{
	struct rk3368_pinctrl_priv *priv = dev_get_priv(dev);
	int ret = 0;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	priv->pmugrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);

	debug("%s: grf=%p pmugrf:%p\n", __func__, priv->grf, priv->pmugrf);

	return ret;
}

static const struct udevice_id rk3368_pinctrl_ids[] = {
	{ .compatible = "rockchip,rk3368-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3368) = {
	.name		= "rockchip_rk3368_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3368_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rk3368_pinctrl_priv),
	.ops		= &rk3368_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= rk3368_pinctrl_probe,
};
