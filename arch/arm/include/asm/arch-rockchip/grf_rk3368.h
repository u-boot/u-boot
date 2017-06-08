/* (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef _ASM_ARCH_GRF_RK3368_H
#define _ASM_ARCH_GRF_RK3368_H

#include <common.h>

struct rk3368_grf {
	u32 gpio1a_iomux;
	u32 gpio1b_iomux;
	u32 gpio1c_iomux;
	u32 gpio1d_iomux;
	u32 gpio2a_iomux;
	u32 gpio2b_iomux;
	u32 gpio2c_iomux;
	u32 gpio2d_iomux;
	u32 gpio3a_iomux;
	u32 gpio3b_iomux;
	u32 gpio3c_iomux;
	u32 gpio3d_iomux;
	u32 reserved[0x34];
	u32 gpio1a_pull;
	u32 gpio1b_pull;
	u32 gpio1c_pull;
	u32 gpio1d_pull;
	u32 gpio2a_pull;
	u32 gpio2b_pull;
	u32 gpio2c_pull;
	u32 gpio2d_pull;
	u32 gpio3a_pull;
	u32 gpio3b_pull;
	u32 gpio3c_pull;
	u32 gpio3d_pull;
	u32 reserved1[0x34];
	u32 gpio1a_drv;
	u32 gpio1b_drv;
	u32 gpio1c_drv;
	u32 gpio1d_drv;
	u32 gpio2a_drv;
	u32 gpio2b_drv;
	u32 gpio2c_drv;
	u32 gpio2d_drv;
	u32 gpio3a_drv;
	u32 gpio3b_drv;
	u32 gpio3c_drv;
	u32 gpio3d_drv;
	u32 reserved2[0x34];
	u32 gpio1l_sr;
	u32 gpio1h_sr;
	u32 gpio2l_sr;
	u32 gpio2h_sr;
	u32 gpio3l_sr;
	u32 gpio3h_sr;
	u32 reserved3[0x1a];
	u32 gpio_smt;
	u32 reserved4[0x1f];
	u32 soc_con0;
	u32 soc_con1;
	u32 soc_con2;
	u32 soc_con3;
	u32 soc_con4;
	u32 soc_con5;
	u32 soc_con6;
	u32 soc_con7;
	u32 soc_con8;
	u32 soc_con9;
	u32 soc_con10;
	u32 soc_con11;
	u32 soc_con12;
	u32 soc_con13;
	u32 soc_con14;
	u32 soc_con15;
	u32 soc_con16;
	u32 soc_con17;
};
check_member(rk3368_grf, soc_con17, 0x444);

struct rk3368_pmu_grf {
	u32 gpio0a_iomux;
	u32 gpio0b_iomux;
	u32 gpio0c_iomux;
	u32 gpio0d_iomux;
	u32 gpio0a_pull;
	u32 gpio0b_pull;
	u32 gpio0c_pull;
	u32 gpio0d_pull;
	u32 gpio0a_drv;
	u32 gpio0b_drv;
	u32 gpio0c_drv;
	u32 gpio0d_drv;
	u32 gpio0l_sr;
	u32 gpio0h_sr;
};
check_member(rk3368_pmu_grf, gpio0h_sr, 0x34);

/*GRF_GPIO0C_IOMUX*/
enum {
	GPIO0C7_SHIFT		= 14,
	GPIO0C7_MASK		= 3 << GPIO0C7_SHIFT,
	GPIO0C7_GPIO		= 0,
	GPIO0C7_LCDC_D19,
	GPIO0C7_TRACE_D9,
	GPIO0C7_UART1_RTSN,

	GPIO0C6_SHIFT           = 12,
	GPIO0C6_MASK            = 3 << GPIO0C6_SHIFT,
	GPIO0C6_GPIO            = 0,
	GPIO0C6_LCDC_D18,
	GPIO0C6_TRACE_D8,
	GPIO0C6_UART1_CTSN,

	GPIO0C5_SHIFT           = 10,
	GPIO0C5_MASK            = 3 << GPIO0C5_SHIFT,
	GPIO0C5_GPIO            = 0,
	GPIO0C5_LCDC_D17,
	GPIO0C5_TRACE_D7,
	GPIO0C5_UART1_SOUT,

	GPIO0C4_SHIFT           = 8,
	GPIO0C4_MASK            = 3 << GPIO0C4_SHIFT,
	GPIO0C4_GPIO            = 0,
	GPIO0C4_LCDC_D16,
	GPIO0C4_TRACE_D6,
	GPIO0C4_UART1_SIN,

	GPIO0C3_SHIFT           = 6,
	GPIO0C3_MASK            = 3 << GPIO0C3_SHIFT,
	GPIO0C3_GPIO            = 0,
	GPIO0C3_LCDC_D15,
	GPIO0C3_TRACE_D5,
	GPIO0C3_MCU_JTAG_TDO,

	GPIO0C2_SHIFT           = 4,
	GPIO0C2_MASK            = 3 << GPIO0C2_SHIFT,
	GPIO0C2_GPIO            = 0,
	GPIO0C2_LCDC_D14,
	GPIO0C2_TRACE_D4,
	GPIO0C2_MCU_JTAG_TDI,

	GPIO0C1_SHIFT           = 2,
	GPIO0C1_MASK            = 3 << GPIO0C1_SHIFT,
	GPIO0C1_GPIO            = 0,
	GPIO0C1_LCDC_D13,
	GPIO0C1_TRACE_D3,
	GPIO0C1_MCU_JTAG_TRTSN,

	GPIO0C0_SHIFT           = 0,
	GPIO0C0_MASK            = 3 << GPIO0C0_SHIFT,
	GPIO0C0_GPIO            = 0,
	GPIO0C0_LCDC_D12,
	GPIO0C0_TRACE_D2,
	GPIO0C0_MCU_JTAG_TDO,
};

/*GRF_GPIO0D_IOMUX*/
enum {
	GPIO0D7_SHIFT           = 14,
	GPIO0D7_MASK            = 3 << GPIO0D7_SHIFT,
	GPIO0D7_GPIO            = 0,
	GPIO0D7_LCDC_DCLK,
	GPIO0D7_TRACE_CTL,
	GPIO0D7_PMU_DEBUG5,

	GPIO0D6_SHIFT           = 12,
	GPIO0D6_MASK            = 3 << GPIO0D6_SHIFT,
	GPIO0D6_GPIO            = 0,
	GPIO0D6_LCDC_DEN,
	GPIO0D6_TRACE_CLK,
	GPIO0D6_PMU_DEBUG4,

	GPIO0D5_SHIFT           = 10,
	GPIO0D5_MASK            = 3 << GPIO0D5_SHIFT,
	GPIO0D5_GPIO            = 0,
	GPIO0D5_LCDC_VSYNC,
	GPIO0D5_TRACE_D15,
	GPIO0D5_PMU_DEBUG3,

	GPIO0D4_SHIFT           = 8,
	GPIO0D4_MASK            = 3 << GPIO0D4_SHIFT,
	GPIO0D4_GPIO            = 0,
	GPIO0D4_LCDC_HSYNC,
	GPIO0D4_TRACE_D14,
	GPIO0D4_PMU_DEBUG2,

	GPIO0D3_SHIFT           = 6,
	GPIO0D3_MASK            = 3 << GPIO0D3_SHIFT,
	GPIO0D3_GPIO            = 0,
	GPIO0D3_LCDC_D23,
	GPIO0D3_TRACE_D13,
	GPIO0D3_UART4_SIN,

	GPIO0D2_SHIFT           = 4,
	GPIO0D2_MASK            = 3 << GPIO0D2_SHIFT,
	GPIO0D2_GPIO            = 0,
	GPIO0D2_LCDC_D22,
	GPIO0D2_TRACE_D12,
	GPIO0D2_UART4_SOUT,

	GPIO0D1_SHIFT           = 2,
	GPIO0D1_MASK            = 3 << GPIO0D1_SHIFT,
	GPIO0D1_GPIO            = 0,
	GPIO0D1_LCDC_D21,
	GPIO0D1_TRACE_D11,
	GPIO0D1_UART4_RTSN,

	GPIO0D0_SHIFT           = 0,
	GPIO0D0_MASK            = 3 << GPIO0D0_SHIFT,
	GPIO0D0_GPIO            = 0,
	GPIO0D0_LCDC_D20,
	GPIO0D0_TRACE_D10,
	GPIO0D0_UART4_CTSN,
};

/*GRF_GPIO2A_IOMUX*/
enum {
	GPIO2A7_SHIFT           = 14,
	GPIO2A7_MASK            = 3 << GPIO2A7_SHIFT,
	GPIO2A7_GPIO            = 0,
	GPIO2A7_SDMMC0_D2,
	GPIO2A7_JTAG_TCK,

	GPIO2A6_SHIFT           = 12,
	GPIO2A6_MASK            = 3 << GPIO2A6_SHIFT,
	GPIO2A6_GPIO            = 0,
	GPIO2A6_SDMMC0_D1,
	GPIO2A6_UART2_SIN,

	GPIO2A5_SHIFT           = 10,
	GPIO2A5_MASK            = 3 << GPIO2A5_SHIFT,
	GPIO2A5_GPIO            = 0,
	GPIO2A5_SDMMC0_D0,
	GPIO2A5_UART2_SOUT,

	GPIO2A4_SHIFT           = 8,
	GPIO2A4_MASK            = 3 << GPIO2A4_SHIFT,
	GPIO2A4_GPIO            = 0,
	GPIO2A4_FLASH_DQS,
	GPIO2A4_EMMC_CLKO,

	GPIO2A3_SHIFT           = 6,
	GPIO2A3_MASK            = 3 << GPIO2A3_SHIFT,
	GPIO2A3_GPIO            = 0,
	GPIO2A3_FLASH_CSN3,
	GPIO2A3_EMMC_RSTNO,

	GPIO2A2_SHIFT           = 4,
	GPIO2A2_MASK            = 3 << GPIO2A2_SHIFT,
	GPIO2A2_GPIO           = 0,
	GPIO2A2_FLASH_CSN2,

	GPIO2A1_SHIFT           = 2,
	GPIO2A1_MASK            = 3 << GPIO2A1_SHIFT,
	GPIO2A1_GPIO            = 0,
	GPIO2A1_FLASH_CSN1,

	GPIO2A0_SHIFT           = 0,
	GPIO2A0_MASK            = 3 << GPIO2A0_SHIFT,
	GPIO2A0_GPIO            = 0,
	GPIO2A0_FLASH_CSN0,
};

/*GRF_GPIO2D_IOMUX*/
enum {
	GPIO2D7_SHIFT           = 14,
	GPIO2D7_MASK            = 3 << GPIO2D7_SHIFT,
	GPIO2D7_GPIO            = 0,
	GPIO2D7_SDIO0_D3,

	GPIO2D6_SHIFT           = 12,
	GPIO2D6_MASK            = 3 << GPIO2D6_SHIFT,
	GPIO2D6_GPIO            = 0,
	GPIO2D6_SDIO0_D2,

	GPIO2D5_SHIFT           = 10,
	GPIO2D5_MASK            = 3 << GPIO2D5_SHIFT,
	GPIO2D5_GPIO            = 0,
	GPIO2D5_SDIO0_D1,

	GPIO2D4_SHIFT           = 8,
	GPIO2D4_MASK            = 3 << GPIO2D4_SHIFT,
	GPIO2D4_GPIO            = 0,
	GPIO2D4_SDIO0_D0,

	GPIO2D3_SHIFT           = 6,
	GPIO2D3_MASK            = 3 << GPIO2D3_SHIFT,
	GPIO2D3_GPIO            = 0,
	GPIO2D3_UART0_RTS0,

	GPIO2D2_SHIFT           = 4,
	GPIO2D2_MASK            = 3 << GPIO2D2_SHIFT,
	GPIO2D2_GPIO            = 0,
	GPIO2D2_UART0_CTS0,

	GPIO2D1_SHIFT           = 2,
	GPIO2D1_MASK            = 3 << GPIO2D1_SHIFT,
	GPIO2D1_GPIO            = 0,
	GPIO2D1_UART0_SOUT,

	GPIO2D0_SHIFT           = 0,
	GPIO2D0_MASK            = 3 << GPIO2D0_SHIFT,
	GPIO2D0_GPIO            = 0,
	GPIO2D0_UART0_SIN,
};

/*GRF_GPIO3C_IOMUX*/
enum {
	GPIO3C7_SHIFT           = 14,
	GPIO3C7_MASK            = 3 << GPIO3C7_SHIFT,
	GPIO3C7_GPIO            = 0,
	GPIO3C7_EDPHDMI_CECINOUT,
	GPIO3C7_ISP_FLASHTRIGIN,

	GPIO3C6_SHIFT           = 12,
	GPIO3C6_MASK            = 3 << GPIO3C6_SHIFT,
	GPIO3C6_GPIO            = 0,
	GPIO3C6_MAC_CLK,
	GPIO3C6_ISP_SHUTTERTRIG,

	GPIO3C5_SHIFT           = 10,
	GPIO3C5_MASK            = 3 << GPIO3C5_SHIFT,
	GPIO3C5_GPIO            = 0,
	GPIO3C5_MAC_RXER,
	GPIO3C5_ISP_PRELIGHTTRIG,

	GPIO3C4_SHIFT           = 8,
	GPIO3C4_MASK            = 3 << GPIO3C4_SHIFT,
	GPIO3C4_GPIO            = 0,
	GPIO3C4_MAC_RXDV,
	GPIO3C4_ISP_FLASHTRIGOUT,

	GPIO3C3_SHIFT           = 6,
	GPIO3C3_MASK            = 3 << GPIO3C3_SHIFT,
	GPIO3C3_GPIO            = 0,
	GPIO3C3_MAC_RXDV,
	GPIO3C3_EMMC_RSTNO,

	GPIO3C2_SHIFT           = 4,
	GPIO3C2_MASK            = 3 << GPIO3C2_SHIFT,
	GPIO3C2_MAC_MDC            = 0,
	GPIO3C2_ISP_SHUTTEREN,

	GPIO3C1_SHIFT           = 2,
	GPIO3C1_MASK            = 3 << GPIO3C1_SHIFT,
	GPIO3C1_GPIO            = 0,
	GPIO3C1_MAC_RXD2,
	GPIO3C1_UART3_RTSN,

	GPIO3C0_SHIFT           = 0,
	GPIO3C0_MASK            = 3 << GPIO3C0_SHIFT,
	GPIO3C0_GPIO            = 0,
	GPIO3C0_MAC_RXD1,
	GPIO3C0_UART3_CTSN,
	GPIO3C0_GPS_RFCLK,
};

/*GRF_GPIO3D_IOMUX*/
enum {
	GPIO3D7_SHIFT           = 14,
	GPIO3D7_MASK            = 3 << GPIO3D7_SHIFT,
	GPIO3D7_GPIO            = 0,
	GPIO3D7_SC_VCC18V,
	GPIO3D7_I2C2_SDA,
	GPIO3D7_GPUJTAG_TCK,

	GPIO3D6_SHIFT           = 12,
	GPIO3D6_MASK            = 3 << GPIO3D6_SHIFT,
	GPIO3D6_GPIO            = 0,
	GPIO3D6_IR_TX,
	GPIO3D6_UART3_SOUT,
	GPIO3D6_PWM3,

	GPIO3D5_SHIFT           = 10,
	GPIO3D5_MASK            = 3 << GPIO3D5_SHIFT,
	GPIO3D5_GPIO            = 0,
	GPIO3D5_IR_RX,
	GPIO3D5_UART3_SIN,

	GPIO3D4_SHIFT           = 8,
	GPIO3D4_MASK            = 3 << GPIO3D4_SHIFT,
	GPIO3D4_GPIO            = 0,
	GPIO3D4_MAC_TXCLKOUT,
	GPIO3D4_SPI1_CSN1,

	GPIO3D3_SHIFT           = 6,
	GPIO3D3_MASK            = 3 << GPIO3D3_SHIFT,
	GPIO3D3_GPIO            = 0,
	GPIO3D3_HDMII2C_SCL,
	GPIO3D3_I2C5_SCL,

	GPIO3D2_SHIFT           = 4,
	GPIO3D2_MASK            = 3 << GPIO3D2_SHIFT,
	GPIO3D2_GPIO            = 0,
	GPIO3D2_HDMII2C_SDA,
	GPIO3D2_I2C5_SDA,

	GPIO3D1_SHIFT           = 2,
	GPIO3D1_MASK            = 3 << GPIO3D1_SHIFT,
	GPIO3D1_GPIO            = 0,
	GPIO3D1_MAC_RXCLKIN,
	GPIO3D1_I2C4_SCL,

	GPIO3D0_SHIFT           = 0,
	GPIO3D0_MASK            = 3 << GPIO3D0_SHIFT,
	GPIO3D0_GPIO            = 0,
	GPIO3D0_MAC_MDIO,
	GPIO3D0_I2C4_SDA,
};

/*GRF_SOC_CON11/12/13*/
enum {
	MCU_SRAM_BASE_BIT27_BIT12_SHIFT	= 0,
	MCU_SRAM_BASE_BIT27_BIT12_MASK	= GENMASK(15, 0),
};

/*GRF_SOC_CON12*/
enum {
	MCU_EXSRAM_BASE_BIT27_BIT12_SHIFT  = 0,
	MCU_EXSRAM_BASE_BIT27_BIT12_MASK   = GENMASK(15, 0),
};

/*GRF_SOC_CON13*/
enum {
	MCU_EXPERI_BASE_BIT27_BIT12_SHIFT  = 0,
	MCU_EXPERI_BASE_BIT27_BIT12_MASK   = GENMASK(15, 0),
};

/*GRF_SOC_CON14*/
enum {
	MCU_EXPERI_BASE_BIT31_BIT28_SHIFT	= 12,
	MCU_EXPERI_BASE_BIT31_BIT28_MASK	= GENMASK(15, 12),
	MCU_EXSRAM_BASE_BIT31_BIT28_SHIFT	= 8,
	MCU_EXSRAM_BASE_BIT31_BIT28_MASK	= GENMASK(11, 8),
	MCU_SRAM_BASE_BIT31_BIT28_SHIFT		= 4,
	MCU_SRAM_BASE_BIT31_BIT28_MASK		= GENMASK(7, 4),
	MCU_CODE_BASE_BIT31_BIT28_SHIFT		= 0,
	MCU_CODE_BASE_BIT31_BIT28_MASK		= GENMASK(3, 0),
};
#endif
