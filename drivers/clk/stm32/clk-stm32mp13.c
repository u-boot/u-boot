// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 * Author: Gabriel Fernandez <gabriel.fernandez@foss.st.com> for STMicroelectronics.
 */
#define LOG_CATEGORY UCLASS_CLK

#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/io.h>
#include <dt-bindings/clock/stm32mp13-clks.h>
#include <linux/clk-provider.h>
#include <dt-bindings/clock/stm32mp13-clksrc.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm/device_compat.h>
#include <init.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <regmap.h>
#include <spl.h>
#include <syscon.h>
#include <time.h>
#include <vsprintf.h>
#include <asm/arch/sys_proto.h>

#include "clk-stm32-core.h"
#include "stm32mp13_rcc.h"

DECLARE_GLOBAL_DATA_PTR;

static const char * const adc12_src[] = {
	"pll4_r", "ck_per", "pll3_q"
};

static const char * const dcmipp_src[] = {
	"ck_axi", "pll2_q", "pll4_p", "ck_per",
};

static const char * const eth12_src[] = {
	"pll4_p", "pll3_q"
};

static const char * const fdcan_src[] = {
	"ck_hse", "pll3_q", "pll4_q", "pll4_r"
};

static const char * const fmc_src[] = {
	"ck_axi", "pll3_r", "pll4_p", "ck_per"
};

static const char * const i2c12_src[] = {
	"pclk1", "pll4_r", "ck_hsi", "ck_csi"
};

static const char * const i2c345_src[] = {
	"pclk6", "pll4_r", "ck_hsi", "ck_csi"
};

static const char * const lptim1_src[] = {
	"pclk1", "pll4_p", "pll3_q", "ck_lse", "ck_lsi", "ck_per"
};

static const char * const lptim23_src[] = {
	"pclk3", "pll4_q", "ck_per", "ck_lse", "ck_lsi"
};

static const char * const lptim45_src[] = {
	"pclk3", "pll4_p", "pll3_q", "ck_lse", "ck_lsi", "ck_per"
};

static const char * const mco1_src[] = {
	"ck_hsi", "ck_hse", "ck_csi", "ck_lsi", "ck_lse"
};

static const char * const mco2_src[] = {
	"ck_mpu", "ck_axi", "ck_mlahb", "pll4_p", "ck_hse", "ck_hsi"
};

static const char * const qspi_src[] = {
	"ck_axi", "pll3_r", "pll4_p", "ck_per"
};

static const char * const rng1_src[] = {
	"ck_csi", "pll4_r", "reserved", "ck_lsi"
};

static const char * const saes_src[] = {
	"ck_axi", "ck_per", "pll4_r", "ck_lsi"
};

static const char * const sai1_src[] = {
	"pll4_q", "pll3_q", "i2s_ckin", "ck_per", "pll3_r"
};

static const char * const sai2_src[] = {
	"pll4_q", "pll3_q", "i2s_ckin", "ck_per", "spdif_ck_symb", "pll3_r"
};

static const char * const sdmmc12_src[] = {
	"ck_axi", "pll3_r", "pll4_p", "ck_hsi"
};

static const char * const spdif_src[] = {
	"pll4_p", "pll3_q", "ck_hsi"
};

static const char * const spi123_src[] = {
	"pll4_p", "pll3_q", "i2s_ckin", "ck_per", "pll3_r"
};

static const char * const spi4_src[] = {
	"pclk6", "pll4_q", "ck_hsi", "ck_csi", "ck_hse", "i2s_ckin"
};

static const char * const spi5_src[] = {
	"pclk6", "pll4_q", "ck_hsi", "ck_csi", "ck_hse"
};

static const char * const stgen_src[] = {
	"ck_hsi", "ck_hse"
};

static const char * const usart12_src[] = {
	"pclk6", "pll3_q", "ck_hsi", "ck_csi", "pll4_q", "ck_hse"
};

static const char * const usart34578_src[] = {
	"pclk1", "pll4_q", "ck_hsi", "ck_csi", "ck_hse"
};

static const char * const usart6_src[] = {
	"pclk2", "pll4_q", "ck_hsi", "ck_csi", "ck_hse"
};

static const char * const usbo_src[] = {
	"pll4_r", "ck_usbo_48m"
};

static const char * const usbphy_src[] = {
	"ck_hse", "pll4_r", "clk-hse-div2"
};


#define MUX_CFG(id, src, _offset, _shift, _witdh) \
	[id] = { \
		.num_parents	= ARRAY_SIZE(src), \
		.parent_names	= (src), \
		.reg_off	= (_offset), \
		.shift		= (_shift), \
		.width		= (_witdh), \
	}

static const struct stm32_mux_cfg stm32mp13_muxes[] = {
	MUX_CFG(MUX_I2C12,	i2c12_src,	RCC_I2C12CKSELR, 0, 3),
	MUX_CFG(MUX_LPTIM45,	lptim45_src,	RCC_LPTIM45CKSELR, 0, 3),
	MUX_CFG(MUX_SPI23,	spi123_src,	RCC_SPI2S23CKSELR, 0, 3),
	MUX_CFG(MUX_UART35,	usart34578_src,	RCC_UART35CKSELR, 0, 3),
	MUX_CFG(MUX_UART78,	usart34578_src,	RCC_UART78CKSELR, 0, 3),
	MUX_CFG(MUX_ADC1,	adc12_src,	RCC_ADC12CKSELR, 0, 2),
	MUX_CFG(MUX_ADC2,	adc12_src,	RCC_ADC12CKSELR, 2, 2),
	MUX_CFG(MUX_DCMIPP,	dcmipp_src,	RCC_DCMIPPCKSELR, 0, 2),
	MUX_CFG(MUX_ETH1,	eth12_src,	RCC_ETH12CKSELR, 0, 2),
	MUX_CFG(MUX_ETH2,	eth12_src,	RCC_ETH12CKSELR, 8, 2),
	MUX_CFG(MUX_FDCAN,	fdcan_src,	RCC_FDCANCKSELR, 0, 2),
	MUX_CFG(MUX_FMC,	fmc_src,	RCC_FMCCKSELR, 0, 2),
	MUX_CFG(MUX_I2C3,	i2c345_src,	RCC_I2C345CKSELR, 0, 3),
	MUX_CFG(MUX_I2C4,	i2c345_src,	RCC_I2C345CKSELR, 3, 3),
	MUX_CFG(MUX_I2C5,	i2c345_src,	RCC_I2C345CKSELR, 6, 3),
	MUX_CFG(MUX_LPTIM1,	lptim1_src,	RCC_LPTIM1CKSELR, 0, 3),
	MUX_CFG(MUX_LPTIM2,	lptim23_src,	RCC_LPTIM23CKSELR, 0, 3),
	MUX_CFG(MUX_LPTIM3,	lptim23_src,	RCC_LPTIM23CKSELR, 3, 3),
	MUX_CFG(MUX_MCO1,	mco1_src,	RCC_MCO1CFGR, 0, 3),
	MUX_CFG(MUX_MCO2,	mco2_src,	RCC_MCO2CFGR, 0, 3),
	MUX_CFG(MUX_QSPI,	qspi_src,	RCC_QSPICKSELR, 0, 2),
	MUX_CFG(MUX_RNG1,	rng1_src,	RCC_RNG1CKSELR, 0, 2),
	MUX_CFG(MUX_SAES,	saes_src,	RCC_SAESCKSELR, 0, 2),
	MUX_CFG(MUX_SAI1,	sai1_src,	RCC_SAI1CKSELR, 0, 3),
	MUX_CFG(MUX_SAI2,	sai2_src,	RCC_SAI2CKSELR, 0, 3),
	MUX_CFG(MUX_SDMMC1,	sdmmc12_src,	RCC_SDMMC12CKSELR, 0, 3),
	MUX_CFG(MUX_SDMMC2,	sdmmc12_src,	RCC_SDMMC12CKSELR, 3, 3),
	MUX_CFG(MUX_SPDIF,	spdif_src,	RCC_SPDIFCKSELR, 0, 2),
	MUX_CFG(MUX_SPI1,	spi123_src,	RCC_SPI2S1CKSELR, 0, 3),
	MUX_CFG(MUX_SPI4,	spi4_src,	RCC_SPI45CKSELR, 0, 3),
	MUX_CFG(MUX_SPI5,	spi5_src,	RCC_SPI45CKSELR, 3, 3),
	MUX_CFG(MUX_STGEN,	stgen_src,	RCC_STGENCKSELR, 0, 2),
	MUX_CFG(MUX_UART1,	usart12_src,	RCC_UART12CKSELR, 0, 3),
	MUX_CFG(MUX_UART2,	usart12_src,	RCC_UART12CKSELR, 3, 3),
	MUX_CFG(MUX_UART4,	usart34578_src,	RCC_UART4CKSELR, 0, 3),
	MUX_CFG(MUX_UART6,	usart6_src,	RCC_UART6CKSELR, 0, 3),
	MUX_CFG(MUX_USBO,	usbo_src,	RCC_USBCKSELR, 4, 1),
	MUX_CFG(MUX_USBPHY,	usbphy_src,	RCC_USBCKSELR, 0, 2),
};

enum enum_gate_cfg {
	GATE_ZERO, /* reserved for no gate */
	GATE_MCO1,
	GATE_MCO2,
	GATE_DBGCK,
	GATE_TRACECK,
	GATE_DDRC1,
	GATE_DDRC1LP,
	GATE_DDRPHYC,
	GATE_DDRPHYCLP,
	GATE_DDRCAPB,
	GATE_DDRCAPBLP,
	GATE_AXIDCG,
	GATE_DDRPHYCAPB,
	GATE_DDRPHYCAPBLP,
	GATE_TIM2,
	GATE_TIM3,
	GATE_TIM4,
	GATE_TIM5,
	GATE_TIM6,
	GATE_TIM7,
	GATE_LPTIM1,
	GATE_SPI2,
	GATE_SPI3,
	GATE_USART3,
	GATE_UART4,
	GATE_UART5,
	GATE_UART7,
	GATE_UART8,
	GATE_I2C1,
	GATE_I2C2,
	GATE_SPDIF,
	GATE_TIM1,
	GATE_TIM8,
	GATE_SPI1,
	GATE_USART6,
	GATE_SAI1,
	GATE_SAI2,
	GATE_DFSDM,
	GATE_ADFSDM,
	GATE_FDCAN,
	GATE_LPTIM2,
	GATE_LPTIM3,
	GATE_LPTIM4,
	GATE_LPTIM5,
	GATE_VREF,
	GATE_DTS,
	GATE_PMBCTRL,
	GATE_HDP,
	GATE_SYSCFG,
	GATE_DCMIPP,
	GATE_DDRPERFM,
	GATE_IWDG2APB,
	GATE_USBPHY,
	GATE_STGENRO,
	GATE_LTDC,
	GATE_TZC,
	GATE_ETZPC,
	GATE_IWDG1APB,
	GATE_BSEC,
	GATE_STGENC,
	GATE_USART1,
	GATE_USART2,
	GATE_SPI4,
	GATE_SPI5,
	GATE_I2C3,
	GATE_I2C4,
	GATE_I2C5,
	GATE_TIM12,
	GATE_TIM13,
	GATE_TIM14,
	GATE_TIM15,
	GATE_TIM16,
	GATE_TIM17,
	GATE_DMA1,
	GATE_DMA2,
	GATE_DMAMUX1,
	GATE_DMA3,
	GATE_DMAMUX2,
	GATE_ADC1,
	GATE_ADC2,
	GATE_USBO,
	GATE_TSC,
	GATE_GPIOA,
	GATE_GPIOB,
	GATE_GPIOC,
	GATE_GPIOD,
	GATE_GPIOE,
	GATE_GPIOF,
	GATE_GPIOG,
	GATE_GPIOH,
	GATE_GPIOI,
	GATE_PKA,
	GATE_SAES,
	GATE_CRYP1,
	GATE_HASH1,
	GATE_RNG1,
	GATE_BKPSRAM,
	GATE_AXIMC,
	GATE_MCE,
	GATE_ETH1CK,
	GATE_ETH1TX,
	GATE_ETH1RX,
	GATE_ETH1MAC,
	GATE_FMC,
	GATE_QSPI,
	GATE_SDMMC1,
	GATE_SDMMC2,
	GATE_CRC1,
	GATE_USBH,
	GATE_ETH2CK,
	GATE_ETH2TX,
	GATE_ETH2RX,
	GATE_ETH2MAC,
	GATE_ETH1STP,
	GATE_ETH2STP,
	GATE_MDMA
};

#define GATE_CFG(id, _offset, _bit_idx, _offset_clr) \
	[id] = { \
		.reg_off	= (_offset), \
		.bit_idx	= (_bit_idx), \
		.set_clr	= (_offset_clr), \
	}

static const struct stm32_gate_cfg stm32mp13_gates[] = {
	GATE_CFG(GATE_MCO1,		RCC_MCO1CFGR,	12,	0),
	GATE_CFG(GATE_MCO2,		RCC_MCO2CFGR,	12,	0),
	GATE_CFG(GATE_DBGCK,		RCC_DBGCFGR,	8,	0),
	GATE_CFG(GATE_TRACECK,		RCC_DBGCFGR,	9,	0),
	GATE_CFG(GATE_DDRC1,		RCC_DDRITFCR,	0,	0),
	GATE_CFG(GATE_DDRC1LP,		RCC_DDRITFCR,	1,	0),
	GATE_CFG(GATE_DDRPHYC,		RCC_DDRITFCR,	4,	0),
	GATE_CFG(GATE_DDRPHYCLP,	RCC_DDRITFCR,	5,	0),
	GATE_CFG(GATE_DDRCAPB,		RCC_DDRITFCR,	6,	0),
	GATE_CFG(GATE_DDRCAPBLP,	RCC_DDRITFCR,	7,	0),
	GATE_CFG(GATE_AXIDCG,		RCC_DDRITFCR,	8,	0),
	GATE_CFG(GATE_DDRPHYCAPB,	RCC_DDRITFCR,	9,	0),
	GATE_CFG(GATE_DDRPHYCAPBLP,	RCC_DDRITFCR,	10,	0),
	GATE_CFG(GATE_TIM2,		RCC_MP_APB1ENSETR,	0,	1),
	GATE_CFG(GATE_TIM3,		RCC_MP_APB1ENSETR,	1,	1),
	GATE_CFG(GATE_TIM4,		RCC_MP_APB1ENSETR,	2,	1),
	GATE_CFG(GATE_TIM5,		RCC_MP_APB1ENSETR,	3,	1),
	GATE_CFG(GATE_TIM6,		RCC_MP_APB1ENSETR,	4,	1),
	GATE_CFG(GATE_TIM7,		RCC_MP_APB1ENSETR,	5,	1),
	GATE_CFG(GATE_LPTIM1,		RCC_MP_APB1ENSETR,	9,	1),
	GATE_CFG(GATE_SPI2,		RCC_MP_APB1ENSETR,	11,	1),
	GATE_CFG(GATE_SPI3,		RCC_MP_APB1ENSETR,	12,	1),
	GATE_CFG(GATE_USART3,		RCC_MP_APB1ENSETR,	15,	1),
	GATE_CFG(GATE_UART4,		RCC_MP_APB1ENSETR,	16,	1),
	GATE_CFG(GATE_UART5,		RCC_MP_APB1ENSETR,	17,	1),
	GATE_CFG(GATE_UART7,		RCC_MP_APB1ENSETR,	18,	1),
	GATE_CFG(GATE_UART8,		RCC_MP_APB1ENSETR,	19,	1),
	GATE_CFG(GATE_I2C1,		RCC_MP_APB1ENSETR,	21,	1),
	GATE_CFG(GATE_I2C2,		RCC_MP_APB1ENSETR,	22,	1),
	GATE_CFG(GATE_SPDIF,		RCC_MP_APB1ENSETR,	26,	1),
	GATE_CFG(GATE_TIM1,		RCC_MP_APB2ENSETR,	0,	1),
	GATE_CFG(GATE_TIM8,		RCC_MP_APB2ENSETR,	1,	1),
	GATE_CFG(GATE_SPI1,		RCC_MP_APB2ENSETR,	8,	1),
	GATE_CFG(GATE_USART6,		RCC_MP_APB2ENSETR,	13,	1),
	GATE_CFG(GATE_SAI1,		RCC_MP_APB2ENSETR,	16,	1),
	GATE_CFG(GATE_SAI2,		RCC_MP_APB2ENSETR,	17,	1),
	GATE_CFG(GATE_DFSDM,		RCC_MP_APB2ENSETR,	20,	1),
	GATE_CFG(GATE_ADFSDM,		RCC_MP_APB2ENSETR,	21,	1),
	GATE_CFG(GATE_FDCAN,		RCC_MP_APB2ENSETR,	24,	1),
	GATE_CFG(GATE_LPTIM2,		RCC_MP_APB3ENSETR,	0,	1),
	GATE_CFG(GATE_LPTIM3,		RCC_MP_APB3ENSETR,	1,	1),
	GATE_CFG(GATE_LPTIM4,		RCC_MP_APB3ENSETR,	2,	1),
	GATE_CFG(GATE_LPTIM5,		RCC_MP_APB3ENSETR,	3,	1),
	GATE_CFG(GATE_VREF,		RCC_MP_APB3ENSETR,	13,	1),
	GATE_CFG(GATE_DTS,		RCC_MP_APB3ENSETR,	16,	1),
	GATE_CFG(GATE_PMBCTRL,		RCC_MP_APB3ENSETR,	17,	1),
	GATE_CFG(GATE_HDP,		RCC_MP_APB3ENSETR,	20,	1),
	GATE_CFG(GATE_SYSCFG,		RCC_MP_NS_APB3ENSETR,	0,	1),
	GATE_CFG(GATE_DCMIPP,		RCC_MP_APB4ENSETR,	1,	1),
	GATE_CFG(GATE_DDRPERFM,		RCC_MP_APB4ENSETR,	8,	1),
	GATE_CFG(GATE_IWDG2APB,		RCC_MP_APB4ENSETR,	15,	1),
	GATE_CFG(GATE_USBPHY,		RCC_MP_APB4ENSETR,	16,	1),
	GATE_CFG(GATE_STGENRO,		RCC_MP_APB4ENSETR,	20,	1),
	GATE_CFG(GATE_LTDC,		RCC_MP_NS_APB4ENSETR,	0,	1),
	GATE_CFG(GATE_TZC,		RCC_MP_APB5ENSETR,	11,	1),
	GATE_CFG(GATE_ETZPC,		RCC_MP_APB5ENSETR,	13,	1),
	GATE_CFG(GATE_IWDG1APB,		RCC_MP_APB5ENSETR,	15,	1),
	GATE_CFG(GATE_BSEC,		RCC_MP_APB5ENSETR,	16,	1),
	GATE_CFG(GATE_STGENC,		RCC_MP_APB5ENSETR,	20,	1),
	GATE_CFG(GATE_USART1,		RCC_MP_APB6ENSETR,	0,	1),
	GATE_CFG(GATE_USART2,		RCC_MP_APB6ENSETR,	1,	1),
	GATE_CFG(GATE_SPI4,		RCC_MP_APB6ENSETR,	2,	1),
	GATE_CFG(GATE_SPI5,		RCC_MP_APB6ENSETR,	3,	1),
	GATE_CFG(GATE_I2C3,		RCC_MP_APB6ENSETR,	4,	1),
	GATE_CFG(GATE_I2C4,		RCC_MP_APB6ENSETR,	5,	1),
	GATE_CFG(GATE_I2C5,		RCC_MP_APB6ENSETR,	6,	1),
	GATE_CFG(GATE_TIM12,		RCC_MP_APB6ENSETR,	7,	1),
	GATE_CFG(GATE_TIM13,		RCC_MP_APB6ENSETR,	8,	1),
	GATE_CFG(GATE_TIM14,		RCC_MP_APB6ENSETR,	9,	1),
	GATE_CFG(GATE_TIM15,		RCC_MP_APB6ENSETR,	10,	1),
	GATE_CFG(GATE_TIM16,		RCC_MP_APB6ENSETR,	11,	1),
	GATE_CFG(GATE_TIM17,		RCC_MP_APB6ENSETR,	12,	1),
	GATE_CFG(GATE_DMA1,		RCC_MP_AHB2ENSETR,	0,	1),
	GATE_CFG(GATE_DMA2,		RCC_MP_AHB2ENSETR,	1,	1),
	GATE_CFG(GATE_DMAMUX1,		RCC_MP_AHB2ENSETR,	2,	1),
	GATE_CFG(GATE_DMA3,		RCC_MP_AHB2ENSETR,	3,	1),
	GATE_CFG(GATE_DMAMUX2,		RCC_MP_AHB2ENSETR,	4,	1),
	GATE_CFG(GATE_ADC1,		RCC_MP_AHB2ENSETR,	5,	1),
	GATE_CFG(GATE_ADC2,		RCC_MP_AHB2ENSETR,	6,	1),
	GATE_CFG(GATE_USBO,		RCC_MP_AHB2ENSETR,	8,	1),
	GATE_CFG(GATE_TSC,		RCC_MP_AHB4ENSETR,	15,	1),
	GATE_CFG(GATE_GPIOA,		RCC_MP_NS_AHB4ENSETR,	0,	1),
	GATE_CFG(GATE_GPIOB,		RCC_MP_NS_AHB4ENSETR,	1,	1),
	GATE_CFG(GATE_GPIOC,		RCC_MP_NS_AHB4ENSETR,	2,	1),
	GATE_CFG(GATE_GPIOD,		RCC_MP_NS_AHB4ENSETR,	3,	1),
	GATE_CFG(GATE_GPIOE,		RCC_MP_NS_AHB4ENSETR,	4,	1),
	GATE_CFG(GATE_GPIOF,		RCC_MP_NS_AHB4ENSETR,	5,	1),
	GATE_CFG(GATE_GPIOG,		RCC_MP_NS_AHB4ENSETR,	6,	1),
	GATE_CFG(GATE_GPIOH,		RCC_MP_NS_AHB4ENSETR,	7,	1),
	GATE_CFG(GATE_GPIOI,		RCC_MP_NS_AHB4ENSETR,	8,	1),
	GATE_CFG(GATE_PKA,		RCC_MP_AHB5ENSETR,	2,	1),
	GATE_CFG(GATE_SAES,		RCC_MP_AHB5ENSETR,	3,	1),
	GATE_CFG(GATE_CRYP1,		RCC_MP_AHB5ENSETR,	4,	1),
	GATE_CFG(GATE_HASH1,		RCC_MP_AHB5ENSETR,	5,	1),
	GATE_CFG(GATE_RNG1,		RCC_MP_AHB5ENSETR,	6,	1),
	GATE_CFG(GATE_BKPSRAM,		RCC_MP_AHB5ENSETR,	8,	1),
	GATE_CFG(GATE_AXIMC,		RCC_MP_AHB5ENSETR,	16,	1),
	GATE_CFG(GATE_MCE,		RCC_MP_AHB6ENSETR,	1,	1),
	GATE_CFG(GATE_ETH1CK,		RCC_MP_AHB6ENSETR,	7,	1),
	GATE_CFG(GATE_ETH1TX,		RCC_MP_AHB6ENSETR,	8,	1),
	GATE_CFG(GATE_ETH1RX,		RCC_MP_AHB6ENSETR,	9,	1),
	GATE_CFG(GATE_ETH1MAC,		RCC_MP_AHB6ENSETR,	10,	1),
	GATE_CFG(GATE_FMC,		RCC_MP_AHB6ENSETR,	12,	1),
	GATE_CFG(GATE_QSPI,		RCC_MP_AHB6ENSETR,	14,	1),
	GATE_CFG(GATE_SDMMC1,		RCC_MP_AHB6ENSETR,	16,	1),
	GATE_CFG(GATE_SDMMC2,		RCC_MP_AHB6ENSETR,	17,	1),
	GATE_CFG(GATE_CRC1,		RCC_MP_AHB6ENSETR,	20,	1),
	GATE_CFG(GATE_USBH,		RCC_MP_AHB6ENSETR,	24,	1),
	GATE_CFG(GATE_ETH2CK,		RCC_MP_AHB6ENSETR,	27,	1),
	GATE_CFG(GATE_ETH2TX,		RCC_MP_AHB6ENSETR,	28,	1),
	GATE_CFG(GATE_ETH2RX,		RCC_MP_AHB6ENSETR,	29,	1),
	GATE_CFG(GATE_ETH2MAC,		RCC_MP_AHB6ENSETR,	30,	1),
	GATE_CFG(GATE_ETH1STP,		RCC_MP_AHB6LPENSETR,	11,	1),
	GATE_CFG(GATE_ETH2STP,		RCC_MP_AHB6LPENSETR,	31,	1),
	GATE_CFG(GATE_MDMA,		RCC_MP_NS_AHB6ENSETR,	0,	1),
};

static const struct clk_div_table ck_trace_div_table[] = {
	{ 0, 1 }, { 1, 2 }, { 2, 4 }, { 3, 8 },
	{ 4, 16 }, { 5, 16 }, { 6, 16 }, { 7, 16 },
	{ 0 },
};

#define DIV_CFG(id, _offset, _shift, _width, _flags, _table) \
	[id] = { \
		.reg_off	= _offset, \
		.shift	= _shift, \
		.width	= _width, \
		.div_flags	= _flags, \
		.table	= _table, \
	}

static const struct stm32_div_cfg stm32mp13_dividers[] = {
	DIV_CFG(DIV_MCO1, RCC_MCO1CFGR, 4, 4, 0, NULL),
	DIV_CFG(DIV_MCO2, RCC_MCO2CFGR, 4, 4, 0, NULL),
	DIV_CFG(DIV_TRACE, RCC_DBGCFGR, 0, 3, 0, ck_trace_div_table),
	DIV_CFG(DIV_ETH1PTP, RCC_ETH12CKSELR, 4, 4, 0, NULL),
	DIV_CFG(DIV_ETH2PTP, RCC_ETH12CKSELR, 12, 4, 0, NULL),
};

struct clk_stm32_security {
	u16	offset;
	u8	bit_idx;
};

enum securit_clk {
	SECF_NONE,
	SECF_LPTIM2,
	SECF_LPTIM3,
	SECF_VREF,
	SECF_DCMIPP,
	SECF_USBPHY,
	SECF_RTC,
	SECF_TZC,
	SECF_ETZPC,
	SECF_IWDG1,
	SECF_BSEC,
	SECF_STGENC,
	SECF_STGENRO,
	SECF_USART1,
	SECF_USART2,
	SECF_SPI4,
	SECF_SPI5,
	SECF_I2C3,
	SECF_I2C4,
	SECF_I2C5,
	SECF_TIM12,
	SECF_TIM13,
	SECF_TIM14,
	SECF_TIM15,
	SECF_TIM16,
	SECF_TIM17,
	SECF_DMA3,
	SECF_DMAMUX2,
	SECF_ADC1,
	SECF_ADC2,
	SECF_USBO,
	SECF_TSC,
	SECF_PKA,
	SECF_SAES,
	SECF_CRYP1,
	SECF_HASH1,
	SECF_RNG1,
	SECF_BKPSRAM,
	SECF_MCE,
	SECF_FMC,
	SECF_QSPI,
	SECF_SDMMC1,
	SECF_SDMMC2,
	SECF_ETH1CK,
	SECF_ETH1TX,
	SECF_ETH1RX,
	SECF_ETH1MAC,
	SECF_ETH1STP,
	SECF_ETH2CK,
	SECF_ETH2TX,
	SECF_ETH2RX,
	SECF_ETH2MAC,
	SECF_ETH2STP,
	SECF_MCO1,
	SECF_MCO2
};

#define SECF(_sec_id, _offset, _bit_idx) \
	[_sec_id] = { \
		.offset	= _offset, \
		.bit_idx	= _bit_idx, \
	}

#ifdef CONFIG_TFABOOT
static const struct clk_stm32_security stm32mp13_security[] = {
	SECF(SECF_LPTIM2, RCC_APB3SECSR, RCC_APB3SECSR_LPTIM2SECF),
	SECF(SECF_LPTIM3, RCC_APB3SECSR, RCC_APB3SECSR_LPTIM3SECF),
	SECF(SECF_VREF, RCC_APB3SECSR, RCC_APB3SECSR_VREFSECF),
	SECF(SECF_DCMIPP, RCC_APB4SECSR, RCC_APB4SECSR_DCMIPPSECF),
	SECF(SECF_USBPHY, RCC_APB4SECSR, RCC_APB4SECSR_USBPHYSECF),
	SECF(SECF_RTC, RCC_APB5SECSR, RCC_APB5SECSR_RTCSECF),
	SECF(SECF_TZC, RCC_APB5SECSR, RCC_APB5SECSR_TZCSECF),
	SECF(SECF_ETZPC, RCC_APB5SECSR, RCC_APB5SECSR_ETZPCSECF),
	SECF(SECF_IWDG1, RCC_APB5SECSR, RCC_APB5SECSR_IWDG1SECF),
	SECF(SECF_BSEC, RCC_APB5SECSR, RCC_APB5SECSR_BSECSECF),
	SECF(SECF_STGENC, RCC_APB5SECSR, RCC_APB5SECSR_STGENCSECF),
	SECF(SECF_STGENRO, RCC_APB5SECSR, RCC_APB5SECSR_STGENROSECF),
	SECF(SECF_USART1, RCC_APB6SECSR, RCC_APB6SECSR_USART1SECF),
	SECF(SECF_USART2, RCC_APB6SECSR, RCC_APB6SECSR_USART2SECF),
	SECF(SECF_SPI4, RCC_APB6SECSR, RCC_APB6SECSR_SPI4SECF),
	SECF(SECF_SPI5, RCC_APB6SECSR, RCC_APB6SECSR_SPI5SECF),
	SECF(SECF_I2C3, RCC_APB6SECSR, RCC_APB6SECSR_I2C3SECF),
	SECF(SECF_I2C4, RCC_APB6SECSR, RCC_APB6SECSR_I2C4SECF),
	SECF(SECF_I2C5, RCC_APB6SECSR, RCC_APB6SECSR_I2C5SECF),
	SECF(SECF_TIM12, RCC_APB6SECSR, RCC_APB6SECSR_TIM12SECF),
	SECF(SECF_TIM13, RCC_APB6SECSR, RCC_APB6SECSR_TIM13SECF),
	SECF(SECF_TIM14, RCC_APB6SECSR, RCC_APB6SECSR_TIM14SECF),
	SECF(SECF_TIM15, RCC_APB6SECSR, RCC_APB6SECSR_TIM15SECF),
	SECF(SECF_TIM16, RCC_APB6SECSR, RCC_APB6SECSR_TIM16SECF),
	SECF(SECF_TIM17, RCC_APB6SECSR, RCC_APB6SECSR_TIM17SECF),
	SECF(SECF_DMA3, RCC_AHB2SECSR, RCC_AHB2SECSR_DMA3SECF),
	SECF(SECF_DMAMUX2, RCC_AHB2SECSR, RCC_AHB2SECSR_DMAMUX2SECF),
	SECF(SECF_ADC1, RCC_AHB2SECSR, RCC_AHB2SECSR_ADC1SECF),
	SECF(SECF_ADC2, RCC_AHB2SECSR, RCC_AHB2SECSR_ADC2SECF),
	SECF(SECF_USBO, RCC_AHB2SECSR, RCC_AHB2SECSR_USBOSECF),
	SECF(SECF_TSC, RCC_AHB4SECSR, RCC_AHB4SECSR_TSCSECF),
	SECF(SECF_PKA, RCC_AHB5SECSR, RCC_AHB5SECSR_PKASECF),
	SECF(SECF_SAES, RCC_AHB5SECSR, RCC_AHB5SECSR_SAESSECF),
	SECF(SECF_CRYP1, RCC_AHB5SECSR, RCC_AHB5SECSR_CRYP1SECF),
	SECF(SECF_HASH1, RCC_AHB5SECSR, RCC_AHB5SECSR_HASH1SECF),
	SECF(SECF_RNG1, RCC_AHB5SECSR, RCC_AHB5SECSR_RNG1SECF),
	SECF(SECF_BKPSRAM, RCC_AHB5SECSR, RCC_AHB5SECSR_BKPSRAMSECF),
	SECF(SECF_MCE, RCC_AHB6SECSR, RCC_AHB6SECSR_MCESECF),
	SECF(SECF_FMC, RCC_AHB6SECSR, RCC_AHB6SECSR_FMCSECF),
	SECF(SECF_QSPI, RCC_AHB6SECSR, RCC_AHB6SECSR_QSPISECF),
	SECF(SECF_SDMMC1, RCC_AHB6SECSR, RCC_AHB6SECSR_SDMMC1SECF),
	SECF(SECF_SDMMC2, RCC_AHB6SECSR, RCC_AHB6SECSR_SDMMC2SECF),
	SECF(SECF_ETH1CK, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH1CKSECF),
	SECF(SECF_ETH1TX, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH1TXSECF),
	SECF(SECF_ETH1RX, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH1RXSECF),
	SECF(SECF_ETH1MAC, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH1MACSECF),
	SECF(SECF_ETH1STP, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH1STPSECF),
	SECF(SECF_ETH2CK, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH2CKSECF),
	SECF(SECF_ETH2TX, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH2TXSECF),
	SECF(SECF_ETH2RX, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH2RXSECF),
	SECF(SECF_ETH2MAC, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH2MACSECF),
	SECF(SECF_ETH2STP, RCC_AHB6SECSR, RCC_AHB6SECSR_ETH2STPSECF),
	SECF(SECF_MCO1, RCC_SECCFGR, RCC_SECCFGR_MCO1SECF),
	SECF(SECF_MCO2, RCC_SECCFGR, RCC_SECCFGR_MCO2SECF),
};
#endif

#define PCLK(_id, _name, _parent, _flags, _gate_id, _sec_id) \
	STM32_GATE(_id, _name, _parent, _flags, _gate_id, _sec_id)

#define TIMER(_id, _name, _parent, _flags, _gate_id, _sec_id) \
	STM32_GATE(_id, _name, _parent, ((_flags) | CLK_SET_RATE_PARENT), \
		   _gate_id, _sec_id)

#define KCLK(_id, _name, _flags, _gate_id, _mux_id, _sec_id) \
	STM32_COMPOSITE(_id, _name, _flags, _sec_id, \
			_gate_id, _mux_id, NO_STM32_DIV)

static const struct clock_config stm32mp13_clock_cfg[] = {
#ifndef CONFIG_XPL_BUILD
	TIMER(TIM2_K, "tim2_k", "timg1_ck", 0, GATE_TIM2, SECF_NONE),
	TIMER(TIM3_K, "tim3_k", "timg1_ck", 0, GATE_TIM3, SECF_NONE),
	TIMER(TIM4_K, "tim4_k", "timg1_ck", 0, GATE_TIM4, SECF_NONE),
	TIMER(TIM5_K, "tim5_k", "timg1_ck", 0, GATE_TIM5, SECF_NONE),
	TIMER(TIM6_K, "tim6_k", "timg1_ck", 0, GATE_TIM6, SECF_NONE),
	TIMER(TIM7_K, "tim7_k", "timg1_ck", 0, GATE_TIM7, SECF_NONE),
	TIMER(TIM1_K, "tim1_k", "timg2_ck", 0, GATE_TIM1, SECF_NONE),
	TIMER(TIM8_K, "tim8_k", "timg2_ck", 0, GATE_TIM8, SECF_NONE),
	TIMER(TIM12_K, "tim12_k", "timg3_ck", 0, GATE_TIM12, SECF_TIM12),
	TIMER(TIM13_K, "tim13_k", "timg3_ck", 0, GATE_TIM13, SECF_TIM13),
	TIMER(TIM14_K, "tim14_k", "timg3_ck", 0, GATE_TIM14, SECF_TIM14),
	TIMER(TIM15_K, "tim15_k", "timg3_ck", 0, GATE_TIM15, SECF_TIM15),
	TIMER(TIM16_K, "tim16_k", "timg3_ck", 0, GATE_TIM16, SECF_TIM16),
	TIMER(TIM17_K, "tim17_k", "timg3_ck", 0, GATE_TIM17, SECF_TIM17),
#endif

	/* Peripheral clocks */
	PCLK(SYSCFG, "syscfg", "pclk3", 0, GATE_SYSCFG, SECF_NONE),
	PCLK(VREF, "vref", "pclk3", 0, GATE_VREF, SECF_VREF),
#ifndef CONFIG_XPL_BUILD
	PCLK(PMBCTRL, "pmbctrl", "pclk3", 0, GATE_PMBCTRL, SECF_NONE),
	PCLK(HDP, "hdp", "pclk3", 0, GATE_HDP, SECF_NONE),
#endif
	PCLK(IWDG2, "iwdg2", "pclk4", 0, GATE_IWDG2APB, SECF_NONE),
	PCLK(STGENRO, "stgenro", "pclk4", 0, GATE_STGENRO, SECF_STGENRO),
	PCLK(TZPC, "tzpc", "pclk5", 0, GATE_TZC, SECF_TZC),
	PCLK(IWDG1, "iwdg1", "pclk5", 0, GATE_IWDG1APB, SECF_IWDG1),
	PCLK(BSEC, "bsec", "pclk5", 0, GATE_BSEC, SECF_BSEC),
#ifndef CONFIG_XPL_BUILD
	PCLK(DMA1, "dma1", "ck_mlahb", 0, GATE_DMA1, SECF_NONE),
	PCLK(DMA2, "dma2", "ck_mlahb",  0, GATE_DMA2, SECF_NONE),
	PCLK(DMAMUX1, "dmamux1", "ck_mlahb", 0, GATE_DMAMUX1, SECF_NONE),
	PCLK(DMAMUX2, "dmamux2", "ck_mlahb", 0, GATE_DMAMUX2, SECF_DMAMUX2),
	PCLK(ADC1, "adc1", "ck_mlahb", 0, GATE_ADC1, SECF_ADC1),
	PCLK(ADC2, "adc2", "ck_mlahb", 0, GATE_ADC2, SECF_ADC2),
#endif
	PCLK(GPIOA, "gpioa", "pclk4", 0, GATE_GPIOA, SECF_NONE),
	PCLK(GPIOB, "gpiob", "pclk4", 0, GATE_GPIOB, SECF_NONE),
	PCLK(GPIOC, "gpioc", "pclk4", 0, GATE_GPIOC, SECF_NONE),
	PCLK(GPIOD, "gpiod", "pclk4", 0, GATE_GPIOD, SECF_NONE),
	PCLK(GPIOE, "gpioe", "pclk4", 0, GATE_GPIOE, SECF_NONE),
	PCLK(GPIOF, "gpiof", "pclk4", 0, GATE_GPIOF, SECF_NONE),
	PCLK(GPIOG, "gpiog", "pclk4", 0, GATE_GPIOG, SECF_NONE),
	PCLK(GPIOH, "gpioh", "pclk4", 0, GATE_GPIOH, SECF_NONE),
	PCLK(GPIOI, "gpioi", "pclk4", 0, GATE_GPIOI, SECF_NONE),
	PCLK(TSC, "tsc", "pclk4", 0, GATE_TSC, SECF_TZC),
	PCLK(PKA, "pka", "ck_axi", 0, GATE_PKA, SECF_PKA),
	PCLK(CRYP1, "cryp1", "ck_axi", 0, GATE_CRYP1, SECF_CRYP1),
	PCLK(HASH1, "hash1", "ck_axi", 0, GATE_HASH1, SECF_HASH1),
	PCLK(BKPSRAM, "bkpsram", "ck_axi", 0, GATE_BKPSRAM, SECF_BKPSRAM),
	PCLK(MDMA, "mdma", "ck_axi", 0, GATE_MDMA, SECF_NONE),
#ifndef CONFIG_XPL_BUILD
	PCLK(ETH1TX, "eth1tx", "ck_axi", 0, GATE_ETH1TX, SECF_ETH1TX),
	PCLK(ETH1RX, "eth1rx", "ck_axi", 0, GATE_ETH1RX, SECF_ETH1RX),
	PCLK(ETH1MAC, "eth1mac", "ck_axi", 0, GATE_ETH1MAC, SECF_ETH1MAC),
	PCLK(ETH2TX, "eth2tx", "ck_axi", 0, GATE_ETH2TX, SECF_ETH2TX),
	PCLK(ETH2RX, "eth2rx", "ck_axi", 0, GATE_ETH2RX, SECF_ETH2RX),
	PCLK(ETH2MAC, "eth2mac", "ck_axi", 0, GATE_ETH2MAC, SECF_ETH2MAC),
#endif
	PCLK(CRC1, "crc1", "ck_axi", 0, GATE_CRC1, SECF_NONE),
#ifndef CONFIG_XPL_BUILD
	PCLK(USBH, "usbh", "ck_axi", 0, GATE_USBH, SECF_NONE),
#endif
	PCLK(DDRPERFM, "ddrperfm", "pclk4", 0, GATE_DDRPERFM, SECF_NONE),
#ifndef CONFIG_XPL_BUILD
	PCLK(ETH1STP, "eth1stp", "ck_axi", 0, GATE_ETH1STP, SECF_ETH1STP),
	PCLK(ETH2STP, "eth2stp", "ck_axi", 0, GATE_ETH2STP, SECF_ETH2STP),
#endif

	/* Kernel clocks */
	KCLK(SDMMC1_K, "sdmmc1_k", 0, GATE_SDMMC1, MUX_SDMMC1, SECF_SDMMC1),
	KCLK(SDMMC2_K, "sdmmc2_k", 0, GATE_SDMMC2, MUX_SDMMC2, SECF_SDMMC2),
	KCLK(FMC_K, "fmc_k", 0, GATE_FMC, MUX_FMC, SECF_FMC),
	KCLK(QSPI_K, "qspi_k", 0, GATE_QSPI, MUX_QSPI, SECF_QSPI),
	KCLK(SPI2_K, "spi2_k", 0, GATE_SPI2, MUX_SPI23, SECF_NONE),
	KCLK(SPI3_K, "spi3_k", 0, GATE_SPI3, MUX_SPI23, SECF_NONE),
	KCLK(I2C1_K, "i2c1_k", 0, GATE_I2C1, MUX_I2C12, SECF_NONE),
	KCLK(I2C2_K, "i2c2_k", 0, GATE_I2C2, MUX_I2C12, SECF_NONE),
#ifndef CONFIG_XPL_BUILD
	KCLK(LPTIM4_K, "lptim4_k", 0, GATE_LPTIM4, MUX_LPTIM45, SECF_NONE),
	KCLK(LPTIM5_K, "lptim5_k", 0, GATE_LPTIM5, MUX_LPTIM45, SECF_NONE),
#endif
	KCLK(USART3_K, "usart3_k", 0, GATE_USART3, MUX_UART35, SECF_NONE),
	KCLK(UART5_K, "uart5_k", 0, GATE_UART5, MUX_UART35, SECF_NONE),
	KCLK(UART7_K, "uart7_k", 0, GATE_UART7, MUX_UART78, SECF_NONE),
	KCLK(UART8_K, "uart8_k", 0, GATE_UART8, MUX_UART78, SECF_NONE),
	KCLK(RNG1_K, "rng1_k", 0, GATE_RNG1, MUX_RNG1, SECF_RNG1),
	KCLK(USBPHY_K, "usbphy_k", 0, GATE_USBPHY, MUX_USBPHY, SECF_USBPHY),
	KCLK(STGEN_K, "stgen_k", 0, GATE_STGENC, MUX_STGEN, SECF_STGENC),
#ifndef CONFIG_XPL_BUILD
	KCLK(SPDIF_K, "spdif_k", 0, GATE_SPDIF, MUX_SPDIF, SECF_NONE),
#endif
	KCLK(SPI1_K, "spi1_k", 0, GATE_SPI1, MUX_SPI1, SECF_NONE),
	KCLK(SPI4_K, "spi4_k", 0, GATE_SPI4, MUX_SPI4, SECF_SPI4),
	KCLK(SPI5_K, "spi5_k", 0, GATE_SPI5, MUX_SPI5, SECF_SPI5),
#ifdef CONFIG_TFABOOT
	KCLK(I2C3_K, "i2c3_k", 0, GATE_I2C3, MUX_I2C3, SECF_I2C3),
#else
	KCLK(I2C3_K, "i2c3_k", 0, GATE_I2C3, MUX_I2C3, SECF_NONE),
#endif
	KCLK(I2C4_K, "i2c4_k", 0, GATE_I2C4, MUX_I2C4, SECF_I2C4),
	KCLK(I2C5_K, "i2c5_k", 0, GATE_I2C5, MUX_I2C5, SECF_I2C5),
#ifndef CONFIG_XPL_BUILD
	KCLK(LPTIM1_K, "lptim1_k", 0, GATE_LPTIM1, MUX_LPTIM1, SECF_NONE),
	KCLK(LPTIM2_K, "lptim2_k", 0, GATE_LPTIM2, MUX_LPTIM2, SECF_LPTIM2),
	KCLK(LPTIM3_K, "lptim3_k", 0, GATE_LPTIM3, MUX_LPTIM3, SECF_LPTIM3),
#endif
	KCLK(USART1_K, "usart1_k", 0, GATE_USART1, MUX_UART1, SECF_USART1),
	KCLK(USART2_K, "usart2_k", 0, GATE_USART2, MUX_UART2, SECF_USART2),
	KCLK(UART4_K, "uart4_k", 0, GATE_UART4, MUX_UART4, SECF_NONE),
	KCLK(USART6_K, "uart6_k", 0, GATE_USART6, MUX_UART6, SECF_NONE),
#ifndef CONFIG_XPL_BUILD
	KCLK(FDCAN_K, "fdcan_k", 0, GATE_FDCAN, MUX_FDCAN, SECF_NONE),
	KCLK(SAI1_K, "sai1_k", 0, GATE_SAI1, MUX_SAI1, SECF_NONE),
	KCLK(SAI2_K, "sai2_k", 0, GATE_SAI2, MUX_SAI2, SECF_NONE),
	KCLK(ADC1_K, "adc1_k", 0, GATE_ADC1, MUX_ADC1, SECF_ADC1),
	KCLK(ADC2_K, "adc2_k", 0, GATE_ADC2, MUX_ADC2, SECF_ADC2),
	KCLK(DCMIPP_K, "dcmipp_k", 0, GATE_DCMIPP, MUX_DCMIPP, SECF_DCMIPP),
	KCLK(ADFSDM_K, "adfsdm_k", 0, GATE_ADFSDM, MUX_SAI1, SECF_NONE),
#endif
	KCLK(USBO_K, "usbo_k", 0, GATE_USBO, MUX_USBO, SECF_USBO),
#ifndef CONFIG_XPL_BUILD
	KCLK(ETH1CK_K, "eth1ck_k", 0, GATE_ETH1CK, MUX_ETH1, SECF_ETH1CK),
	KCLK(ETH2CK_K, "eth2ck_k", 0, GATE_ETH2CK, MUX_ETH2, SECF_ETH2CK),
	KCLK(SAES_K, "saes_k", 0, GATE_SAES, MUX_SAES, SECF_SAES),

	STM32_GATE(DFSDM_K, "dfsdm_k", "ck_mlahb", 0, GATE_DFSDM, SECF_NONE),
	STM32_GATE(LTDC_PX, "ltdc_px", "pll4_q", CLK_SET_RATE_PARENT,
		   GATE_LTDC, SECF_NONE),

	STM32_GATE(DTS_K, "dts_k", "ck_lse", 0, GATE_DTS, SECF_NONE),
#endif

	STM32_COMPOSITE(ETH1PTP_K, "eth1ptp_k", CLK_OPS_PARENT_ENABLE |
		  CLK_SET_RATE_NO_REPARENT, SECF_ETH1CK,
		  NO_STM32_GATE, MUX_ETH1, DIV_ETH1PTP),

	STM32_COMPOSITE(ETH2PTP_K, "eth2ptp_k", CLK_OPS_PARENT_ENABLE |
		  CLK_SET_RATE_NO_REPARENT, SECF_ETH2CK,
		  NO_STM32_GATE, MUX_ETH2, DIV_ETH2PTP),

	/* MCO clocks */
	STM32_COMPOSITE(CK_MCO1, "ck_mco1", CLK_OPS_PARENT_ENABLE |
			CLK_SET_RATE_NO_REPARENT, SECF_MCO1,
			GATE_MCO1, MUX_MCO1, DIV_MCO1),

	STM32_COMPOSITE(CK_MCO2, "ck_mco2", CLK_OPS_PARENT_ENABLE |
			CLK_SET_RATE_NO_REPARENT, SECF_MCO2,
			GATE_MCO2, MUX_MCO2, DIV_MCO2),

	/* Debug clocks */
	STM32_GATE(CK_DBG, "ck_sys_dbg", "ck_axi", CLK_IGNORE_UNUSED,
		   GATE_DBGCK, SECF_NONE),

	STM32_COMPOSITE_NOMUX(CK_TRACE, "ck_trace", "ck_axi",
			      CLK_OPS_PARENT_ENABLE, SECF_NONE,
			      GATE_TRACECK, DIV_TRACE),

#ifdef CONFIG_XPL_BUILD
	STM32_GATE(AXIDCG, "axidcg", "ck_axi", CLK_IGNORE_UNUSED,
		   GATE_AXIDCG, SECF_NONE),
	STM32_GATE(DDRC1, "ddrc1", "ck_axi", CLK_IGNORE_UNUSED,
		   GATE_DDRC1, SECF_NONE),
	STM32_GATE(DDRPHYC, "ddrphyc", "pll2_r", CLK_IGNORE_UNUSED,
		   GATE_DDRPHYC, SECF_NONE),
	STM32_GATE(DDRCAPB, "ddrcapb", "pclk4", CLK_IGNORE_UNUSED,
		   GATE_DDRCAPB, SECF_NONE),
	STM32_GATE(DDRPHYCAPB, "ddrphycapb", "pclk4", CLK_IGNORE_UNUSED,
		   GATE_DDRPHYCAPB, SECF_NONE),
#endif
};

#ifdef CONFIG_TFABOOT
static int stm32mp13_check_security(struct udevice *dev, void __iomem *base,
				    const struct clock_config *cfg)
{
	int sec_id = cfg->sec_id;
	int secured = 0;

	if (sec_id != SECF_NONE) {
		const struct clk_stm32_security *secf;

		secf = &stm32mp13_security[sec_id];
		secured = !!(readl(base + secf->offset) & BIT(secf->bit_idx));
	}

	return secured;
}
#endif

static const struct stm32_clock_match_data stm32mp13_data = {
	.tab_clocks	= stm32mp13_clock_cfg,
	.num_clocks	= ARRAY_SIZE(stm32mp13_clock_cfg),
	.clock_data = &(const struct clk_stm32_clock_data) {
		.num_gates	= ARRAY_SIZE(stm32mp13_gates),
		.gates		= stm32mp13_gates,
		.muxes		= stm32mp13_muxes,
		.dividers	= stm32mp13_dividers,
	},
#ifdef CONFIG_TFABOOT
	.check_security = stm32mp13_check_security,
#endif
};

#ifndef CONFIG_TFABOOT

enum stm32mp1_parent_id {
/*
 * _HSI, _HSE, _CSI, _LSI, _LSE should not be moved
 * they are used as index in osc_clk[] as clock reference
 */
	_HSI,
	_HSE,
	_CSI,
	_LSI,
	_LSE,
	_I2S_CKIN,
	NB_OSC,

/* other parent source */
	_HSI_KER = NB_OSC,
	_HSE_KER,
	_HSE_KER_DIV2,
	_CSI_KER,
	_PLL1_P,
	_PLL1_Q,
	_PLL1_R,
	_PLL2_P,
	_PLL2_Q,
	_PLL2_R,
	_PLL3_P,
	_PLL3_Q,
	_PLL3_R,
	_PLL4_P,
	_PLL4_Q,
	_PLL4_R,
	_ACLK,
	_PCLK1,
	_PCLK2,
	_PCLK3,
	_PCLK4,
	_PCLK5,
	_HCLK6,
	_HCLK2,
	_CK_PER,
	_CK_MPU,
	_CK_MCU,
	_DSI_PHY,
	_USB_PHY_48,
	_PARENT_NB,
	_UNKNOWN_ID = 0xff,
};

#if defined(CONFIG_XPL_BUILD)

#define MAX_HSI_HZ		64000000

/* TIMEOUT */
#define TIMEOUT_200MS		200000
#define TIMEOUT_1S		1000000

/* STGEN registers */
#define STGENC_CNTCR		0x00
#define STGENC_CNTSR		0x04
#define STGENC_CNTCVL		0x08
#define STGENC_CNTCVU		0x0C
#define STGENC_CNTFID0		0x20

#define STGENC_CNTCR_EN		BIT(0)

enum stm32mp1_clksrc_id {
	CLKSRC_MPU,
	CLKSRC_AXI,
	CLKSRC_MLAHB,
	CLKSRC_PLL12,
	CLKSRC_PLL3,
	CLKSRC_PLL4,
	CLKSRC_RTC,
	CLKSRC_MCO1,
	CLKSRC_MCO2,
	CLKSRC_NB
};

enum stm32mp1_clkdiv_id {
	CLKDIV_AXI,
	CLKDIV_MLAHB,
	CLKDIV_APB1,
	CLKDIV_APB2,
	CLKDIV_APB3,
	CLKDIV_APB4,
	CLKDIV_APB5,
	CLKDIV_APB6,
	CLKDIV_RTC,
	CLKDIV_NB
};

enum stm32mp1_pll_id {
	_PLL1,
	_PLL2,
	_PLL3,
	_PLL4,
	_PLL_NB
};

enum stm32mp1_div_id {
	_DIV_P,
	_DIV_Q,
	_DIV_R,
	_DIV_NB,
};

/* define characteristic of PLL according type */
#define DIVM_MIN	1
#define DIVM_MAX	63
#define DIVN_MIN	24
#define DIVP_MIN	0
#define DIVP_MAX	127
#define FRAC_MAX	8192

#define PLL2000_VCO_MIN	992000000
#define PLL2000_VCO_MAX	2000000000

enum stm32mp1_pllcfg {
	PLLCFG_M,
	PLLCFG_N,
	PLLCFG_P,
	PLLCFG_Q,
	PLLCFG_R,
	PLLCFG_O,
	PLLCFG_NB
};

enum stm32mp1_pllcsg {
	PLLCSG_MOD_PER,
	PLLCSG_INC_STEP,
	PLLCSG_SSCG_MODE,
	PLLCSG_NB
};

enum stm32mp1_plltype {
	PLL_800,
	PLL_1600,
	PLL_2000,
	PLL_TYPE_NB
};

struct stm32mp1_pll {
	u8 refclk_min;
	u8 refclk_max;
	u8 divn_max;
};

#define REFCLK_SIZE 4
struct stm32mp1_clk_pll {
	enum stm32mp1_plltype plltype;
	u16 rckxselr;
	u16 pllxcfgr1;
	u16 pllxcfgr2;
	u16 pllxfracr;
	u16 pllxcr;
	u16 pllxcsgr;
	u8 refclk[REFCLK_SIZE];
};

static const struct stm32mp1_pll stm32mp1_pll[PLL_TYPE_NB] = {
	[PLL_800] = {
		.refclk_min = 4,
		.refclk_max = 16,
		.divn_max = 99,
		},
	[PLL_1600] = {
		.refclk_min = 8,
		.refclk_max = 16,
		.divn_max = 199,
		},
	[PLL_2000] = {
		.refclk_min = 8,
		.refclk_max = 16,
		.divn_max = 99,
		},
};

#define STM32MP1_CLK_PLL(idx, type, off1, off2, off3, off4, off5, off6,\
			p1, p2, p3, p4) \
	[(idx)] = {				\
		.plltype = (type),			\
		.rckxselr = (off1),		\
		.pllxcfgr1 = (off2),		\
		.pllxcfgr2 = (off3),		\
		.pllxfracr = (off4),		\
		.pllxcr = (off5),		\
		.pllxcsgr = (off6),		\
		.refclk[0] = (p1),		\
		.refclk[1] = (p2),		\
		.refclk[2] = (p3),		\
		.refclk[3] = (p4),		\
	}

static const struct stm32mp1_clk_pll stm32mp1_clk_pll[_PLL_NB] = {
	STM32MP1_CLK_PLL(_PLL1, PLL_2000,
			 RCC_RCK12SELR, RCC_PLL1CFGR1, RCC_PLL1CFGR2,
			 RCC_PLL1FRACR, RCC_PLL1CR, RCC_PLL1CSGR,
			 _HSI, _HSE, _UNKNOWN_ID, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL2, PLL_1600,
			 RCC_RCK12SELR, RCC_PLL2CFGR1, RCC_PLL2CFGR2,
			 RCC_PLL2FRACR, RCC_PLL2CR, RCC_PLL2CSGR,
			 _HSI, _HSE, _UNKNOWN_ID, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL3, PLL_800,
			 RCC_RCK3SELR, RCC_PLL3CFGR1, RCC_PLL3CFGR2,
			 RCC_PLL3FRACR, RCC_PLL3CR, RCC_PLL3CSGR,
			 _HSI, _HSE, _CSI, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL4, PLL_800,
			 RCC_RCK4SELR, RCC_PLL4CFGR1, RCC_PLL4CFGR2,
			 RCC_PLL4FRACR, RCC_PLL4CR, RCC_PLL4CSGR,
			 _HSI, _HSE, _CSI, _I2S_CKIN),
};

static ulong stm32mp1_clk_get_fixed(struct stm32mp_rcc_priv *priv, int idx)
{
	if (idx >= NB_OSC) {
		log_debug("clk id %d not found\n", idx);
		return 0;
	}

	return clk_get_rate(&priv->osc_clk[idx]);
}

bool stm32mp1_supports_opp(u32 opp_id, u32 cpu_type)
{
	/* 650 MHz is always supported */
	if (opp_id == 1)
		return true;

	/*
	 * 1000 MHz is supported on STM32MP13xDxx and STM32MP13xFxx,
	 * which all have bit 11 i.e. 0x800 set in CPU ID.
	 */
	if (opp_id == 2)
		return !!(cpu_type & BIT(11));

	/* Any other OPP is invalid. */
	return false;
}

__weak void board_vddcore_init(u32 voltage_mv)
{
}

/*
 * gets OPP parameters (frequency in KHz and voltage in mV) from
 * an OPP table subnode. Platform HW support capabilities are also checked.
 * Returns 0 on success and a negative FDT error code on failure.
 */
static int stm32mp1_get_opp(u32 cpu_type, ofnode subnode,
			    u32 *freq_khz, u32 *voltage_mv)
{
	u32 opp_hw;
	u64 read_freq_64;
	u32 read_voltage_32;

	*freq_khz = 0;
	*voltage_mv = 0;

	opp_hw = ofnode_read_u32_default(subnode, "opp-supported-hw", 0);
	if (opp_hw)
		if (!stm32mp1_supports_opp(opp_hw, cpu_type))
			return -FDT_ERR_BADVALUE;

	read_freq_64 = ofnode_read_u64_default(subnode, "opp-hz", 0) /
		       1000ULL;
	read_voltage_32 = ofnode_read_u32_default(subnode, "opp-microvolt", 0) /
			  1000U;

	if (!read_voltage_32 || !read_freq_64)
		return -FDT_ERR_NOTFOUND;

	/* Frequency value expressed in KHz must fit on 32 bits */
	if (read_freq_64 > U32_MAX)
		return -FDT_ERR_BADVALUE;

	/* Millivolt value must fit on 16 bits */
	if (read_voltage_32 > U16_MAX)
		return -FDT_ERR_BADVALUE;

	*freq_khz = (u32)read_freq_64;
	*voltage_mv = read_voltage_32;

	return 0;
}

/*
 * parses OPP table in DT and finds the parameters for the
 * highest frequency supported by the HW platform.
 * Returns 0 on success and a negative FDT error code on failure.
 */
int stm32mp1_get_max_opp_freq(struct stm32mp_rcc_priv *priv, u64 *freq_hz)
{
	ofnode node, subnode;
	int ret;
	u32 freq = 0U, voltage = 0U;
	u32 cpu_type = get_cpu_type();

	node = ofnode_by_compatible(ofnode_null(), "operating-points-v2");
	if (!ofnode_valid(node))
		return -FDT_ERR_NOTFOUND;

	ofnode_for_each_subnode(subnode, node) {
		unsigned int read_freq;
		unsigned int read_voltage;

		ret = stm32mp1_get_opp(cpu_type, subnode,
				       &read_freq, &read_voltage);
		if (ret)
			continue;

		if (read_freq > freq) {
			freq = read_freq;
			voltage = read_voltage;
		}
	}

	if (!freq || !voltage)
		return -FDT_ERR_NOTFOUND;

	*freq_hz = (u64)1000U * freq;
	board_vddcore_init(voltage);

	return 0;
}

static int stm32mp1_pll1_opp(struct stm32mp_rcc_priv *priv, int clksrc,
			     u32 *pllcfg, u32 *fracv)
{
	u32 post_divm;
	u32 input_freq;
	u64 output_freq;
	u64 freq;
	u64 vco;
	u32 divm, divn, divp, frac;
	int i, ret;
	u32 diff;
	u32 best_diff = U32_MAX;

	/* PLL1 is 2000 */
	const u32 DIVN_MAX = stm32mp1_pll[PLL_2000].divn_max;
	const u32 POST_DIVM_MIN = stm32mp1_pll[PLL_2000].refclk_min * 1000000U;
	const u32 POST_DIVM_MAX = stm32mp1_pll[PLL_2000].refclk_max * 1000000U;

	ret = stm32mp1_get_max_opp_freq(priv, &output_freq);
	if (ret) {
		log_debug("PLL1 OPP configuration not found (%d).\n", ret);
		return ret;
	}

	switch (clksrc) {
	case CLK_PLL12_HSI:
		input_freq = stm32mp1_clk_get_fixed(priv, _HSI);
		break;
	case CLK_PLL12_HSE:
		input_freq = stm32mp1_clk_get_fixed(priv, _HSE);
		break;
	default:
		return -EINTR;
	}

	/* Following parameters have always the same value */
	pllcfg[PLLCFG_Q] = 0;
	pllcfg[PLLCFG_R] = 0;
	pllcfg[PLLCFG_O] = PQR(1, 1, 1);

	for (divm = DIVM_MAX; divm >= DIVM_MIN; divm--)	{
		post_divm = (u32)(input_freq / (divm + 1));
		if (post_divm < POST_DIVM_MIN || post_divm > POST_DIVM_MAX)
			continue;

		for (divp = DIVP_MIN; divp <= DIVP_MAX; divp++) {
			freq = output_freq * (divm + 1) * (divp + 1);
			divn = (u32)((freq / input_freq) - 1);
			if (divn < DIVN_MIN || divn > DIVN_MAX)
				continue;

			frac = (u32)(((freq * FRAC_MAX) / input_freq) -
				     ((divn + 1) * FRAC_MAX));
			/* 2 loops to refine the fractional part */
			for (i = 2; i != 0; i--) {
				if (frac > FRAC_MAX)
					break;

				vco = (post_divm * (divn + 1)) +
				      ((post_divm * (u64)frac) /
				       FRAC_MAX);
				if (vco < (PLL2000_VCO_MIN / 2) ||
				    vco > (PLL2000_VCO_MAX / 2)) {
					frac++;
					continue;
				}
				freq = vco / (divp + 1);
				if (output_freq < freq)
					diff = (u32)(freq - output_freq);
				else
					diff = (u32)(output_freq - freq);
				if (diff < best_diff)  {
					pllcfg[PLLCFG_M] = divm;
					pllcfg[PLLCFG_N] = divn;
					pllcfg[PLLCFG_P] = divp;
					*fracv = frac;

					if (diff == 0) {
						return 0;
					}

					best_diff = diff;
				}
				frac++;
			}
		}
	}

	if (best_diff == U32_MAX)
		return -1;

	return 0;
}

static void stm32mp1_ls_osc_set(int enable, fdt_addr_t rcc, u32 offset,
				u32 mask_on)
{
	u32 address = rcc + offset;

	if (enable)
		setbits_le32(address, mask_on);
	else
		clrbits_le32(address, mask_on);
}

static void stm32mp1_hs_ocs_set(int enable, fdt_addr_t rcc, u32 mask_on)
{
	writel(mask_on, rcc + (enable ? RCC_OCENSETR : RCC_OCENCLRR));
}

static int stm32mp1_osc_wait(int enable, fdt_addr_t rcc, u32 offset,
			     u32 mask_rdy)
{
	u32 mask_test = 0;
	u32 address = rcc + offset;
	u32 val;
	int ret;

	if (enable)
		mask_test = mask_rdy;

	ret = readl_poll_timeout(address, val,
				 (val & mask_rdy) == mask_test,
				 TIMEOUT_1S);

	if (ret)
		log_err("OSC %x @ %x timeout for enable=%d : 0x%x\n",
			mask_rdy, address, enable, readl(address));

	return ret;
}

static void stm32mp1_lse_enable(fdt_addr_t rcc, int bypass, int digbyp,
				u32 lsedrv)
{
	u32 value;

	if (digbyp)
		setbits_le32(rcc + RCC_BDCR, RCC_BDCR_DIGBYP);

	if (bypass || digbyp)
		setbits_le32(rcc + RCC_BDCR, RCC_BDCR_LSEBYP);

	/*
	 * warning: not recommended to switch directly from "high drive"
	 * to "medium low drive", and vice-versa.
	 */
	value = (readl(rcc + RCC_BDCR) & RCC_BDCR_LSEDRV_MASK)
		>> RCC_BDCR_LSEDRV_SHIFT;

	while (value != lsedrv) {
		if (value > lsedrv)
			value--;
		else
			value++;

		clrsetbits_le32(rcc + RCC_BDCR,
				RCC_BDCR_LSEDRV_MASK,
				value << RCC_BDCR_LSEDRV_SHIFT);
	}

	stm32mp1_ls_osc_set(1, rcc, RCC_BDCR, RCC_BDCR_LSEON);
}

static void stm32mp1_lse_wait(fdt_addr_t rcc)
{
	stm32mp1_osc_wait(1, rcc, RCC_BDCR, RCC_BDCR_LSERDY);
}

static void stm32mp1_lsi_set(fdt_addr_t rcc, int enable)
{
	stm32mp1_ls_osc_set(enable, rcc, RCC_RDLSICR, RCC_RDLSICR_LSION);
	stm32mp1_osc_wait(enable, rcc, RCC_RDLSICR, RCC_RDLSICR_LSIRDY);
}

static void stm32mp1_hse_enable(fdt_addr_t rcc, int bypass, int digbyp, int css)
{
	if (digbyp)
		writel(RCC_OCENR_DIGBYP, rcc + RCC_OCENSETR);
	if (bypass || digbyp)
		writel(RCC_OCENR_HSEBYP, rcc + RCC_OCENSETR);

	stm32mp1_hs_ocs_set(1, rcc, RCC_OCENR_HSEON);
	stm32mp1_osc_wait(1, rcc, RCC_OCRDYR, RCC_OCRDYR_HSERDY);

	if (css)
		writel(RCC_OCENR_HSECSSON, rcc + RCC_OCENSETR);
}

static void stm32mp1_csi_set(fdt_addr_t rcc, int enable)
{
	stm32mp1_hs_ocs_set(enable, rcc, RCC_OCENR_CSION);
	stm32mp1_osc_wait(enable, rcc, RCC_OCRDYR, RCC_OCRDYR_CSIRDY);
}

static void stm32mp1_hsi_set(fdt_addr_t rcc, int enable)
{
	stm32mp1_hs_ocs_set(enable, rcc, RCC_OCENR_HSION);
	stm32mp1_osc_wait(enable, rcc, RCC_OCRDYR, RCC_OCRDYR_HSIRDY);
}

static int stm32mp1_set_hsidiv(fdt_addr_t rcc, u8 hsidiv)
{
	u32 address = rcc + RCC_OCRDYR;
	u32 val;
	int ret;

	clrsetbits_le32(rcc + RCC_HSICFGR,
			RCC_HSICFGR_HSIDIV_MASK,
			RCC_HSICFGR_HSIDIV_MASK & hsidiv);

	ret = readl_poll_timeout(address, val,
				 val & RCC_OCRDYR_HSIDIVRDY,
				 TIMEOUT_200MS);
	if (ret)
		log_err("HSIDIV failed @ 0x%x: 0x%x\n",
			address, readl(address));

	return ret;
}

static int stm32mp1_hsidiv(fdt_addr_t rcc, ulong hsifreq)
{
	u8 hsidiv;
	u32 hsidivfreq = MAX_HSI_HZ;

	for (hsidiv = 0; hsidiv < 4; hsidiv++,
	     hsidivfreq = hsidivfreq / 2)
		if (hsidivfreq == hsifreq)
			break;

	if (hsidiv == 4) {
		log_err("hsi frequency invalid");
		return -1;
	}

	if (hsidiv > 0)
		return stm32mp1_set_hsidiv(rcc, hsidiv);

	return 0;
}

static void pll_start(struct stm32mp_rcc_priv *priv, int pll_id)
{
	clrsetbits_le32((u32)(priv->base) + stm32mp1_clk_pll[pll_id].pllxcr,
			RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN |
			RCC_PLLNCR_DIVREN,
			RCC_PLLNCR_PLLON);
}

static int pll_output(struct stm32mp_rcc_priv *priv, int pll_id, int output)
{
	u32 pllxcr = (u32)(priv->base) + stm32mp1_clk_pll[pll_id].pllxcr;
	u32 val;
	int ret;

	ret = readl_poll_timeout(pllxcr, val, val & RCC_PLLNCR_PLLRDY,
				 TIMEOUT_200MS);

	if (ret) {
		log_err("PLL%d start failed @ 0x%x: 0x%x\n",
			pll_id, pllxcr, readl(pllxcr));
		return ret;
	}

	/* start the requested output */
	setbits_le32(pllxcr, output << RCC_PLLNCR_DIVEN_SHIFT);

	return 0;
}

static int pll_stop(struct stm32mp_rcc_priv *priv, int pll_id)
{
	u32 pllxcr = (u32)(priv->base) + stm32mp1_clk_pll[pll_id].pllxcr;
	u32 val;

	/* stop all output */
	clrbits_le32(pllxcr,
		     RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN | RCC_PLLNCR_DIVREN);

	/* stop PLL */
	clrbits_le32(pllxcr, RCC_PLLNCR_PLLON);

	/* wait PLL stopped */
	return readl_poll_timeout(pllxcr, val, (val & RCC_PLLNCR_PLLRDY) == 0,
				  TIMEOUT_200MS);
}

static void pll_config_output(struct stm32mp_rcc_priv *priv,
			      int pll_id, u32 *pllcfg)
{
	fdt_addr_t rcc = (u32)(priv->base);
	u32 value;

	value = (pllcfg[PLLCFG_P] << RCC_PLLNCFGR2_DIVP_SHIFT)
		& RCC_PLLNCFGR2_DIVP_MASK;
	value |= (pllcfg[PLLCFG_Q] << RCC_PLLNCFGR2_DIVQ_SHIFT)
		 & RCC_PLLNCFGR2_DIVQ_MASK;
	value |= (pllcfg[PLLCFG_R] << RCC_PLLNCFGR2_DIVR_SHIFT)
		 & RCC_PLLNCFGR2_DIVR_MASK;
	writel(value, rcc + stm32mp1_clk_pll[pll_id].pllxcfgr2);
}

static int pll_config(struct stm32mp_rcc_priv *priv, int pll_id,
		      u32 *pllcfg, u32 fracv)
{
	fdt_addr_t rcc = (u32)(priv->base);
	enum stm32mp1_plltype type = stm32mp1_clk_pll[pll_id].plltype;
	int src;
	ulong refclk;
	u8 ifrge = 0;
	u32 value;

	src = readl((u32)(priv->base) + stm32mp1_clk_pll[pll_id].rckxselr) & RCC_SELR_SRC_MASK;
	refclk = stm32mp1_clk_get_fixed(priv, stm32mp1_clk_pll[pll_id].refclk[src]) /
		 (pllcfg[PLLCFG_M] + 1);

	if (refclk < (stm32mp1_pll[type].refclk_min * 1000000) ||
	    refclk > (stm32mp1_pll[type].refclk_max * 1000000)) {
		log_err("invalid refclk = %x\n", (u32)refclk);
		return -EINVAL;
	}


	if (type == PLL_800 && refclk >= 8000000)
		ifrge = 1;

	value = (pllcfg[PLLCFG_N] << RCC_PLLNCFGR1_DIVN_SHIFT)
		 & RCC_PLLNCFGR1_DIVN_MASK;
	value |= (pllcfg[PLLCFG_M] << RCC_PLLNCFGR1_DIVM_SHIFT)
		 & RCC_PLLNCFGR1_DIVM_MASK;
	value |= (ifrge << RCC_PLLNCFGR1_IFRGE_SHIFT)
		 & RCC_PLLNCFGR1_IFRGE_MASK;
	writel(value, rcc + stm32mp1_clk_pll[pll_id].pllxcfgr1);

	/* fractional configuration: load sigma-delta modulator (SDM) */

	/* Write into FRACV the new fractional value , and FRACLE to 0 */
	writel(fracv << RCC_PLLNFRACR_FRACV_SHIFT,
	       rcc + stm32mp1_clk_pll[pll_id].pllxfracr);

	/* Write FRACLE to 1 : FRACV value is loaded into the SDM */
	setbits_le32(rcc + stm32mp1_clk_pll[pll_id].pllxfracr,
		     RCC_PLLNFRACR_FRACLE);

	pll_config_output(priv, pll_id, pllcfg);

	return 0;
}

static void pll_csg(struct stm32mp_rcc_priv *priv, int pll_id, u32 *csg)
{
	u32 pllxcsg;

	pllxcsg = ((csg[PLLCSG_MOD_PER] << RCC_PLLNCSGR_MOD_PER_SHIFT) &
		    RCC_PLLNCSGR_MOD_PER_MASK) |
		  ((csg[PLLCSG_INC_STEP] << RCC_PLLNCSGR_INC_STEP_SHIFT) &
		    RCC_PLLNCSGR_INC_STEP_MASK) |
		  ((csg[PLLCSG_SSCG_MODE] << RCC_PLLNCSGR_SSCG_MODE_SHIFT) &
		    RCC_PLLNCSGR_SSCG_MODE_MASK);

	writel(pllxcsg, (u32)(priv->base) + stm32mp1_clk_pll[pll_id].pllxcsgr);

	setbits_le32((u32)(priv->base) + stm32mp1_clk_pll[pll_id].pllxcr, RCC_PLLNCR_SSCG_CTRL);
}

static ulong pll_get_fref_ck(struct stm32mp_rcc_priv *priv,
			      int pll_id)
{
	u32 selr;
	int src;

	/* Get current refclk */
	selr = readl(priv->base + stm32mp1_clk_pll[pll_id].rckxselr);
	src = selr & RCC_SELR_SRC_MASK;

	return stm32mp1_clk_get_fixed(priv, stm32mp1_clk_pll[pll_id].refclk[src]);
}

static  __maybe_unused int pll_set_rate(struct udevice *dev,
					int pll_id,
					int div_id,
					unsigned long clk_rate)
{
	struct stm32mp_rcc_priv *priv = dev_get_priv(dev);
	unsigned int pllcfg[PLLCFG_NB];
	ofnode plloff;
	char name[12];
	enum stm32mp1_plltype type = stm32mp1_clk_pll[pll_id].plltype;
	int divm, divn, divy;
	int ret;
	ulong fck_ref;
	u32 fracv;
	u64 value;

	if (div_id > _DIV_NB)
		return -EINVAL;

	sprintf(name, "st,pll@%d", pll_id);
	plloff = dev_read_subnode(dev, name);
	if (!ofnode_valid(plloff))
		return -FDT_ERR_NOTFOUND;

	ret = ofnode_read_u32_array(plloff, "cfg",
				    pllcfg, PLLCFG_NB);
	if (ret < 0)
		return -FDT_ERR_NOTFOUND;

	fck_ref = pll_get_fref_ck(priv, pll_id);

	divm = pllcfg[PLLCFG_M];
	/* select output divider = 0: for _DIV_P, 1:_DIV_Q 2:_DIV_R */
	divy = pllcfg[PLLCFG_P + div_id];

	/* For: PLL1 & PLL2 => VCO is * 2 but ck_pll_y is also / 2
	 * So same final result than PLL2 et 4
	 * with FRACV
	 * Fck_pll_y = Fck_ref * ((DIVN + 1) + FRACV / 2^13)
	 *             / (DIVy + 1) * (DIVM + 1)
	 * value = (DIVN + 1) * 2^13 + FRACV / 2^13
	 *       = Fck_pll_y (DIVy + 1) * (DIVM + 1) * 2^13 / Fck_ref
	 */
	value = ((u64)clk_rate * (divy + 1) * (divm + 1)) << 13;
	value = lldiv(value, fck_ref);

	divn = (value >> 13) - 1;
	if (divn < DIVN_MIN ||
	    divn > stm32mp1_pll[type].divn_max) {
		dev_err(dev, "divn invalid = %d", divn);
		return -EINVAL;
	}
	fracv = value - ((divn + 1) << 13);
	pllcfg[PLLCFG_N] = divn;

	/* reconfigure PLL */
	pll_stop(priv, pll_id);
	pll_config(priv, pll_id, pllcfg, fracv);
	pll_start(priv, pll_id);
	pll_output(priv, pll_id, pllcfg[PLLCFG_O]);

	return 0;
}

static int set_clksrc(struct stm32mp_rcc_priv *priv, unsigned int clksrc)
{
	u32 address = (u32)(priv->base);
	u32 mux = (clksrc & MUX_ID_MASK) >> MUX_ID_SHIFT;
	u32 val;
	int ret;

	/* List of relevant muxes to keep the size down */
	if (mux == MUX_PLL12)
		address += RCC_RCK12SELR;
	else if (mux == MUX_PLL3)
		address += RCC_RCK3SELR;
	else if (mux == MUX_PLL4)
		address += RCC_RCK4SELR;
	else if (mux == MUX_MPU)
		address += RCC_MPCKSELR;
	else if (mux == MUX_AXI)
		address += RCC_ASSCKSELR;
	else if (mux == MUX_MLAHB)
		address += RCC_MSSCKSELR;
	else if (mux == MUX_CKPER)
		address += RCC_CPERCKSELR;
	else
		return -EINVAL;

	clrsetbits_le32(address, RCC_SELR_SRC_MASK, clksrc & RCC_SELR_SRC_MASK);
	ret = readl_poll_timeout(address, val, val & RCC_SELR_SRCRDY,
				 TIMEOUT_200MS);
	if (ret)
		log_err("CLKSRC %x start failed @ 0x%x: 0x%x\n",
			clksrc, address, readl(address));

	return ret;
}

static void stgen_config(struct stm32mp_rcc_priv *priv)
{
	u32 stgenc, cntfid0;
	ulong rate = clk_get_rate(&priv->osc_clk[_HSI]);
	stgenc = STM32_STGEN_BASE;
	cntfid0 = readl(stgenc + STGENC_CNTFID0);

	if (cntfid0 != rate) {
		u64 counter;

		log_debug("System Generic Counter (STGEN) update\n");
		clrbits_le32(stgenc + STGENC_CNTCR, STGENC_CNTCR_EN);
		counter = (u64)readl(stgenc + STGENC_CNTCVL);
		counter |= ((u64)(readl(stgenc + STGENC_CNTCVU))) << 32;
		counter = lldiv(counter * (u64)rate, cntfid0);
		writel((u32)counter, stgenc + STGENC_CNTCVL);
		writel((u32)(counter >> 32), stgenc + STGENC_CNTCVU);
		writel(rate, stgenc + STGENC_CNTFID0);
		setbits_le32(stgenc + STGENC_CNTCR, STGENC_CNTCR_EN);

		__asm__ volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (rate));

		/* need to update gd->arch.timer_rate_hz with new frequency */
		timer_init();
	}
}

static int set_clkdiv(unsigned int clkdiv, u32 address)
{
	u32 val;
	int ret;


	clrsetbits_le32(address, RCC_DIVR_DIV_MASK, clkdiv & RCC_DIVR_DIV_MASK);
	ret = readl_poll_timeout(address, val, val & RCC_DIVR_DIVRDY,
				 TIMEOUT_200MS);
	if (ret)
		log_err("CLKDIV %x start failed @ 0x%x: 0x%x\n",
			clkdiv, address, readl(address));

	return ret;
}

static void set_rtcsrc(struct stm32mp_rcc_priv *priv,
		       unsigned int clksrc,
		       int lse_css)
{
	u32 address = (u32)(priv->base) + RCC_BDCR;

	if (readl(address) & RCC_BDCR_RTCCKEN)
		goto skip_rtc;

	if (clksrc == CLK_RTC_DISABLED)
		goto skip_rtc;

	clrsetbits_le32(address,
			RCC_BDCR_RTCSRC_MASK,
			clksrc << RCC_BDCR_RTCSRC_SHIFT);

	setbits_le32(address, RCC_BDCR_RTCCKEN);

skip_rtc:
	if (lse_css)
		setbits_le32(address, RCC_BDCR_LSECSSON);
}

static void pkcs_config(struct stm32mp_rcc_priv *priv, u32 pkcs)
{
	u32 mux = (pkcs & MUX_ID_MASK) >> MUX_ID_SHIFT;
	u32 address = (u32)(priv->base) + stm32mp13_muxes[mux].reg_off;
	u32 mask = (BIT(stm32mp13_muxes[mux].width) - 1) << stm32mp13_muxes[mux].shift;
	u32 value = (pkcs << stm32mp13_muxes[mux].shift) & mask;

	clrsetbits_le32(address, mask, value);
}

static int stm32mp1_clktree(struct udevice *dev)
{
	struct stm32mp_rcc_priv *priv = dev_get_priv(dev);
	fdt_addr_t rcc = (u32)(priv->base);
	unsigned int clksrc[CLKSRC_NB];
	unsigned int clkdiv[CLKDIV_NB];
	unsigned int pllcfg[_PLL_NB][PLLCFG_NB];
	unsigned int pllfracv[_PLL_NB];
	unsigned int pllcsg[_PLL_NB][PLLCSG_NB];
	bool pllcfg_valid[_PLL_NB];
	bool pllcsg_set[_PLL_NB];
	int ret;
	int i, len;
	int lse_css = 0;
	const u32 *pkcs_cell;

	/* check mandatory field */
	ret = dev_read_u32_array(dev, "st,clksrc", clksrc, CLKSRC_NB);
	if (ret < 0) {
		dev_dbg(dev, "field st,clksrc invalid: error %d\n", ret);
		return -FDT_ERR_NOTFOUND;
	}

	ret = dev_read_u32_array(dev, "st,clkdiv", clkdiv, CLKDIV_NB);
	if (ret < 0) {
		dev_dbg(dev, "field st,clkdiv invalid: error %d\n", ret);
		return -FDT_ERR_NOTFOUND;
	}

	/* check mandatory field in each pll */
	for (i = 0; i < _PLL_NB; i++) {
		char name[12];
		ofnode node;

		sprintf(name, "st,pll@%d", i);
		node = dev_read_subnode(dev, name);
		pllcfg_valid[i] = ofnode_valid(node);
		pllcsg_set[i] = false;
		if (pllcfg_valid[i]) {
			dev_dbg(dev, "DT for PLL %d @ %s\n", i, name);
			ret = ofnode_read_u32_array(node, "cfg",
						    pllcfg[i], PLLCFG_NB);
			if (ret < 0) {
				dev_dbg(dev, "field cfg invalid: error %d\n", ret);
				return -FDT_ERR_NOTFOUND;
			}
			pllfracv[i] = ofnode_read_u32_default(node, "frac", 0);

			ret = ofnode_read_u32_array(node, "csg", pllcsg[i],
						    PLLCSG_NB);
			if (!ret) {
				pllcsg_set[i] = true;
			} else if (ret != -FDT_ERR_NOTFOUND) {
				dev_dbg(dev, "invalid csg node for pll@%d res=%d\n",
					i, ret);
				return ret;
			}
		} else if (i == _PLL1)	{
			/* use OPP for PLL1 for A7 CPU */
			dev_dbg(dev, "DT for PLL %d with OPP\n", i);
			ret = stm32mp1_pll1_opp(priv,
						clksrc[CLKSRC_PLL12],
						pllcfg[i],
						&pllfracv[i]);
			if (ret) {
				dev_dbg(dev, "PLL %d with OPP error = %d\n", i, ret);
				return ret;
			}
			pllcfg_valid[i] = true;
		}
	}

	dev_dbg(dev, "switch ON osillator\n");
	/*
	 * switch ON oscillator found in device-tree,
	 * HSI already ON after bootrom
	 */
	if (clk_valid(&priv->osc_clk[_LSI]))
		stm32mp1_lsi_set(rcc, 1);

	if (clk_valid(&priv->osc_clk[_LSE])) {
		int bypass, digbyp;
		u32 lsedrv;
		struct udevice *dev = priv->osc_clk[_LSE].dev;

		bypass = dev_read_bool(dev, "st,bypass");
		digbyp = dev_read_bool(dev, "st,digbypass");
		lse_css = dev_read_bool(dev, "st,css");
		lsedrv = dev_read_u32_default(dev, "st,drive",
					      LSEDRV_MEDIUM_HIGH);

		stm32mp1_lse_enable(rcc, bypass, digbyp, lsedrv);
	}


	if (clk_valid(&priv->osc_clk[_HSE])) {
		int bypass, digbyp, css;
		struct udevice *dev = priv->osc_clk[_HSE].dev;

		bypass = dev_read_bool(dev, "st,bypass");
		digbyp = dev_read_bool(dev, "st,digbypass");
		css = dev_read_bool(dev, "st,css");

		stm32mp1_hse_enable(rcc, bypass, digbyp, css);
	}

	/* CSI is mandatory for automatic I/O compensation (SYSCFG_CMPCR)
	 * => switch on CSI even if node is not present in device tree
	 */
	stm32mp1_csi_set(rcc, 1);

	/* come back to HSI */
	dev_dbg(dev, "come back to HSI\n");
	set_clksrc(priv, CLK_MPU_HSI);
	set_clksrc(priv, CLK_AXI_HSI);
	set_clksrc(priv, CLK_MLAHBS_HSI);

	dev_dbg(dev, "pll stop\n");
	for (i = 0; i < _PLL_NB; i++)
		pll_stop(priv, i);

	/* configure HSIDIV */
	dev_dbg(dev, "configure HSIDIV\n");
	if (clk_valid(&priv->osc_clk[_HSI])) {
		stm32mp1_hsidiv(rcc, clk_get_rate(&priv->osc_clk[_HSI]));
		stgen_config(priv);
	}

	/* select DIV */
	dev_dbg(dev, "select DIV\n");
	/* no ready bit when MPUSRC != CLK_MPU_PLL1P_DIV, MPUDIV is disabled */
	set_clkdiv(clkdiv[CLKDIV_AXI], rcc + RCC_AXIDIVR);
	set_clkdiv(clkdiv[CLKDIV_APB4], rcc + RCC_APB4DIVR);
	set_clkdiv(clkdiv[CLKDIV_APB5], rcc + RCC_APB5DIVR);
	set_clkdiv(clkdiv[CLKDIV_APB6], rcc + RCC_APB6DIVR);
	set_clkdiv(clkdiv[CLKDIV_APB1], rcc + RCC_APB1DIVR);
	set_clkdiv(clkdiv[CLKDIV_APB2], rcc + RCC_APB2DIVR);
	set_clkdiv(clkdiv[CLKDIV_APB3], rcc + RCC_APB3DIVR);

	/* no ready bit for RTC */
	writel(clkdiv[CLKDIV_RTC] & RCC_DIVR_DIV_MASK, rcc + RCC_RTCDIVR);

	/* configure PLLs source */
	dev_dbg(dev, "configure PLLs source\n");
	set_clksrc(priv, clksrc[CLKSRC_PLL12]);
	set_clksrc(priv, clksrc[CLKSRC_PLL3]);
	set_clksrc(priv, clksrc[CLKSRC_PLL4]);

	/* configure and start PLLs */
	dev_dbg(dev, "configure PLLs\n");
	for (i = 0; i < _PLL_NB; i++) {
		if (!pllcfg_valid[i])
			continue;
		dev_dbg(dev, "configure PLL %d\n", i);
		pll_config(priv, i, pllcfg[i], pllfracv[i]);
		if (pllcsg_set[i])
			pll_csg(priv, i, pllcsg[i]);
		pll_start(priv, i);
	}

	/* wait and start PLLs ouptut when ready */
	for (i = 0; i < _PLL_NB; i++) {
		if (!pllcfg_valid[i])
			continue;
		dev_dbg(dev, "output PLL %d\n", i);
		pll_output(priv, i, pllcfg[i][PLLCFG_O]);
	}

	/* wait LSE ready before to use it */
	if (clk_valid(&priv->osc_clk[_LSE]))
		stm32mp1_lse_wait(rcc);

	/* configure with expected clock source */
	dev_dbg(dev, "CLKSRC\n");
	set_clksrc(priv, clksrc[CLKSRC_MPU]);
	set_clksrc(priv, clksrc[CLKSRC_AXI]);
	set_clksrc(priv, clksrc[CLKSRC_MLAHB]);
	set_rtcsrc(priv, clksrc[CLKSRC_RTC], lse_css);

	/* configure PKCK */
	dev_dbg(dev, "PKCK\n");
	pkcs_cell = dev_read_prop(dev, "st,pkcs", &len);
	if (pkcs_cell) {
		bool ckper_disabled = false;

		for (i = 0; i < len / sizeof(u32); i++) {
			u32 pkcs = (u32)fdt32_to_cpu(pkcs_cell[i]);

			if (pkcs == CLK_CKPER_DISABLED) {
				ckper_disabled = true;
				continue;
			}
			pkcs_config(priv, pkcs);
		}
		/* CKPER is source for some peripheral clock
		 * (FMC-NAND / QPSI-NOR) and switching source is allowed
		 * only if previous clock is still ON
		 * => deactivated CKPER only after switching clock
		 */
		if (ckper_disabled)
			pkcs_config(priv, CLK_CKPER_DISABLED);
	}

	/* STGEN clock source can change with CLK_STGEN_XXX */
	stgen_config(priv);

	dev_dbg(dev, "oscillator off\n");
	/* switch OFF HSI if not found in device-tree */
	if (!clk_valid(&priv->osc_clk[_HSI]))
		stm32mp1_hsi_set(rcc, 0);

	/* Software Self-Refresh mode (SSR) during DDR initilialization */
	clrsetbits_le32((u32)(priv->base) + RCC_DDRITFCR,
			RCC_DDRITFCR_DDRCKMOD_MASK,
			RCC_DDRITFCR_DDRCKMOD_SSR <<
			RCC_DDRITFCR_DDRCKMOD_SHIFT);

	return 0;
}
#endif

static int stm32mp1_osc_init(struct udevice *dev)
{
	struct stm32mp_rcc_priv *priv = dev_get_priv(dev);
	fdt_addr_t base = dev_read_addr(dev->parent);
	struct clk *ck;
	int i;

	const char *name[NB_OSC] = {
		[_LSI] = "lsi",
		[_LSE] = "lse",
		[_HSI] = "hsi",
		[_HSE] = "hse",
		[_CSI] = "csi",
		[_I2S_CKIN] = "i2s_ckin",
	};

	const struct {
		const char *name;
		const int rate;
	} fixed_clk[] = {
		{ "bsec", 66625000 },
		{ "ck_axi", 266500000 },
		{ "ck_mlahb", 200000000 },
		{ "ck_mpu", 1000000000 },
		{ "ck_per", 24000000 },
		{ "ck_rtc", 32768 },
		{ "clk-hse-div2", 12000000 },
		{ "pclk1", 100000000 },
		{ "pclk2", 100000000 },
		{ "pclk3", 100000000 },
		{ "pclk4", 133250000 },
		{ "pclk5", 66625000 },
		{ "pclk6", 100000000 },
		{ "pll2_q", 266500000 },
		{ "pll2_r", 533000000 },
		{ "pll3_p", 200000000 },
		{ "pll3_q", 150000000 },
		{ "pll3_r", 200000000 },
		{ "pll4_p", 125000000 },
		{ "pll4_q", 83333333 },
		{ "pll4_r", 75000000 },
		{ "rtcapb", 66625000 },
		{ "timg1_ck", 200000000 },
		{ "timg2_ck", 200000000 },
		{ "timg3_ck", 200000000 },
	};

	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (void __iomem *)base;

	for (i = 0; i < NB_OSC; i++) {
		if (clk_get_by_name(dev, name[i], &priv->osc_clk[i]))
			dev_dbg(dev, "No source clock \"%s\"\n", name[i]);
		else
			dev_dbg(dev, "%s clock rate: %luHz\n",
				name[i], clk_get_rate(&priv->osc_clk[i]));
	}

	for (i = 0; i < ARRAY_SIZE(fixed_clk); i++) {
		ck = clk_register_fixed_rate(NULL, fixed_clk[i].name, fixed_clk[i].rate);
		if (!ck)
			dev_dbg(dev, "Cannot register fixed clock \"%s\"\n", fixed_clk[i].name);
	}

	return 0;
}
#endif

static int stm32mp1_clk_probe(struct udevice *dev)
{
	int err;

#ifdef CONFIG_TFABOOT
	struct udevice *scmi;

	/* force SCMI probe to register all SCMI clocks */
	uclass_get_device_by_driver(UCLASS_CLK, DM_DRIVER_GET(scmi_clock), &scmi);
#else
	err = stm32mp1_osc_init(dev);
	if (err)
		return err;

#if defined(CONFIG_XPL_BUILD)
	/* clock tree init is done only one time, before relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		err = stm32mp1_clktree(dev);
	if (err)
		dev_err(dev, "clock tree initialization failed (%d)\n", err);
#endif
#endif

	err = stm32_rcc_init(dev, &stm32mp13_data);
	if (err)
		return err;

	gd->cpu_clk = clk_stm32_get_rate_by_name("ck_mpu");
	gd->bus_clk = clk_stm32_get_rate_by_name("ck_axi");

	/* DDRPHYC father */
	gd->mem_clk = clk_stm32_get_rate_by_name("pll2_r");

#ifndef CONFIG_XPL_BUILD
	if (IS_ENABLED(CONFIG_DISPLAY_CPUINFO)) {
		if (gd->flags & GD_FLG_RELOC) {
			char buf[32];

			log_info("Clocks:\n");
			log_info("- MPU : %s MHz\n", strmhz(buf, gd->cpu_clk));
			log_info("- AXI : %s MHz\n", strmhz(buf, gd->bus_clk));
			log_info("- PER : %s MHz\n",
				 strmhz(buf, clk_stm32_get_rate_by_name("ck_per")));
			log_info("- DDR : %s MHz\n", strmhz(buf, gd->mem_clk));
		}
	}
#endif

	return 0;
}

U_BOOT_DRIVER(stm32mp1_clock) = {
	.name = "stm32mp13_clk",
	.id = UCLASS_CLK,
	.ops = &stm32_clk_ops,
	.priv_auto = sizeof(struct stm32mp_rcc_priv),
	.probe = stm32mp1_clk_probe,
};
