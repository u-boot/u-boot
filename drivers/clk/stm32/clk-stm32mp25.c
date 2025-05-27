// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 */

#include <clk-uclass.h>
#include <dm.h>
#include <dt-bindings/clock/st,stm32mp25-rcc.h>
#include <linux/clk-provider.h>
#include <linux/io.h>

#include "clk-stm32-core.h"
#include "stm32mp25_rcc.h"

/* Clock security definition */
#define SECF_NONE	-1

static const char * const adc12_src[] = {
	"ck_flexgen_46", "ck_icn_ls_mcu"
};

static const char * const adc3_src[] = {
	"ck_flexgen_47", "ck_icn_ls_mcu", "ck_flexgen_46"
};

static const char * const usb2phy1_src[] = {
	"ck_flexgen_57", "hse_div2_ck"
};

static const char * const usb2phy2_src[] = {
	"ck_flexgen_58", "hse_div2_ck"
};

static const char * const usb3pciphy_src[] = {
	"ck_flexgen_34", "hse_div2_ck"
};

static const char * const dsiblane_src[] = {
	"txbyteclk", "ck_ker_ltdc"
};

static const char * const dsiphy_src[] = {
	"ck_flexgen_28", "hse_ck"
};

static const char * const lvdsphy_src[] = {
	"ck_flexgen_32", "hse_ck"
};

static const char * const dts_src[] = {
	"hsi_ck", "hse_ck", "msi_ck"
};

static const char * const mco1_src[] = {
	"ck_flexgen_61", "ck_obs0"
};

static const char * const mco2_src[] = {
	"ck_flexgen_62", "ck_obs1"
};

enum enum_mux_cfg {
	MUX_MCO1,
	MUX_MCO2,
	MUX_ADC12,
	MUX_ADC3,
	MUX_USB2PHY1,
	MUX_USB2PHY2,
	MUX_USB3PCIEPHY,
	MUX_DSIBLANE,
	MUX_DSIPHY,
	MUX_LVDSPHY,
	MUX_DTS,
	MUX_NB
};

#define MUX_CFG(id, src, _offset, _shift, _witdh)[id] = {\
		.num_parents	= ARRAY_SIZE(src),\
		.parent_names	= src,\
		.reg_off	= (_offset),\
		.shift		= (_shift),\
		.width		= (_witdh),\
}

static const struct stm32_mux_cfg stm32mp25_muxes[MUX_NB] = {
	MUX_CFG(MUX_ADC12,		adc12_src,	RCC_ADC12CFGR,		12,	1),
	MUX_CFG(MUX_ADC3,		adc3_src,	RCC_ADC3CFGR,		12,	2),
	MUX_CFG(MUX_DSIBLANE,		dsiblane_src,	RCC_DSICFGR,		12,	1),
	MUX_CFG(MUX_DSIPHY,		dsiphy_src,	RCC_DSICFGR,		15,	1),
	MUX_CFG(MUX_DTS,		dts_src,	RCC_DTSCFGR,		12,	2),
	MUX_CFG(MUX_MCO1,		mco1_src,	RCC_MCO1CFGR,		0,	1),
	MUX_CFG(MUX_MCO2,		mco2_src,	RCC_MCO2CFGR,		0,	1),
	MUX_CFG(MUX_LVDSPHY,		lvdsphy_src,	RCC_LVDSCFGR,		15,	1),
	MUX_CFG(MUX_USB2PHY1,		usb2phy1_src,	RCC_USB2PHY1CFGR,	15,	1),
	MUX_CFG(MUX_USB2PHY2,		usb2phy2_src,	RCC_USB2PHY2CFGR,	15,	1),
	MUX_CFG(MUX_USB3PCIEPHY,	usb3pciphy_src,	RCC_USB3PCIEPHYCFGR,	15,	1),
};

enum enum_gate_cfg {
	GATE_ADC12,
	GATE_ADC3,
	GATE_ADF1,
	GATE_CCI,
	GATE_CRC,
	GATE_CRYP1,
	GATE_CRYP2,
	GATE_CSI,
	GATE_DBG,
	GATE_DCMIPP,
	GATE_DSI,
	GATE_DTS,
	GATE_ETH1,
	GATE_ETH1MAC,
	GATE_ETH1RX,
	GATE_ETH1STP,
	GATE_ETH1TX,
	GATE_ETH2,
	GATE_ETH2MAC,
	GATE_ETH2RX,
	GATE_ETH2STP,
	GATE_ETH2TX,
	GATE_ETHSW,
	GATE_ETHSWMAC,
	GATE_ETHSWREF,
	GATE_ETR,
	GATE_FDCAN,
	GATE_GPU,
	GATE_HASH,
	GATE_HDP,
	GATE_I2C1,
	GATE_I2C2,
	GATE_I2C3,
	GATE_I2C4,
	GATE_I2C5,
	GATE_I2C6,
	GATE_I2C7,
	GATE_I2C8,
	GATE_I3C1,
	GATE_I3C2,
	GATE_I3C3,
	GATE_I3C4,
	GATE_IS2M,
	GATE_IWDG1,
	GATE_IWDG2,
	GATE_IWDG3,
	GATE_IWDG4,
	GATE_IWDG5,
	GATE_LPTIM1,
	GATE_LPTIM2,
	GATE_LPTIM3,
	GATE_LPTIM4,
	GATE_LPTIM5,
	GATE_LPUART1,
	GATE_LTDC,
	GATE_LVDS,
	GATE_MCO1,
	GATE_MCO2,
	GATE_MDF1,
	GATE_OSPI1,
	GATE_OSPI2,
	GATE_OSPIIOM,
	GATE_PCIE,
	GATE_PKA,
	GATE_RNG,
	GATE_SAES,
	GATE_SAI1,
	GATE_SAI2,
	GATE_SAI3,
	GATE_SAI4,
	GATE_SDMMC1,
	GATE_SDMMC2,
	GATE_SDMMC3,
	GATE_SERC,
	GATE_SPDIFRX,
	GATE_SPI1,
	GATE_SPI2,
	GATE_SPI3,
	GATE_SPI4,
	GATE_SPI5,
	GATE_SPI6,
	GATE_SPI7,
	GATE_SPI8,
	GATE_STGEN,
	GATE_STM500,
	GATE_TIM1,
	GATE_TIM2,
	GATE_TIM3,
	GATE_TIM4,
	GATE_TIM5,
	GATE_TIM6,
	GATE_TIM7,
	GATE_TIM8,
	GATE_TIM10,
	GATE_TIM11,
	GATE_TIM12,
	GATE_TIM13,
	GATE_TIM14,
	GATE_TIM15,
	GATE_TIM16,
	GATE_TIM17,
	GATE_TIM20,
	GATE_TRACE,
	GATE_UART4,
	GATE_UART5,
	GATE_UART7,
	GATE_UART8,
	GATE_UART9,
	GATE_USART1,
	GATE_USART2,
	GATE_USART3,
	GATE_USART6,
	GATE_USBH,
	GATE_USB2PHY1,
	GATE_USB2PHY2,
	GATE_USB3DR,
	GATE_USB3PCIEPHY,
	GATE_USBTC,
	GATE_VDEC,
	GATE_VENC,
	GATE_VREF,
	GATE_WWDG1,
	GATE_WWDG2,
	GATE_NB
};

#define GATE_CFG(id, _offset, _bit_idx, _offset_clr)[id] = {\
	.reg_off	= (_offset),\
	.bit_idx	= (_bit_idx),\
	.set_clr	= (_offset_clr),\
}

static const struct stm32_gate_cfg stm32mp25_gates[GATE_NB] = {
	GATE_CFG(GATE_MCO1,		RCC_MCO1CFGR,		8,	0),
	GATE_CFG(GATE_MCO2,		RCC_MCO2CFGR,		8,	0),
	GATE_CFG(GATE_OSPI1,		RCC_OSPI1CFGR,		1,	0),
	GATE_CFG(GATE_OSPI2,		RCC_OSPI2CFGR,		1,	0),
	GATE_CFG(GATE_DBG,		RCC_DBGCFGR,		8,	0),
	GATE_CFG(GATE_TRACE,		RCC_DBGCFGR,		9,	0),
	GATE_CFG(GATE_STM500,		RCC_STM500CFGR,		1,	0),
	GATE_CFG(GATE_ETR,		RCC_ETRCFGR,		1,	0),
	GATE_CFG(GATE_IS2M,		RCC_IS2MCFGR,		1,	0),
	GATE_CFG(GATE_TIM1,		RCC_TIM1CFGR,		1,	0),
	GATE_CFG(GATE_TIM2,		RCC_TIM2CFGR,		1,	0),
	GATE_CFG(GATE_TIM3,		RCC_TIM3CFGR,		1,	0),
	GATE_CFG(GATE_TIM4,		RCC_TIM4CFGR,		1,	0),
	GATE_CFG(GATE_TIM5,		RCC_TIM5CFGR,		1,	0),
	GATE_CFG(GATE_TIM6,		RCC_TIM6CFGR,		1,	0),
	GATE_CFG(GATE_TIM7,		RCC_TIM7CFGR,		1,	0),
	GATE_CFG(GATE_TIM8,		RCC_TIM8CFGR,		1,	0),
	GATE_CFG(GATE_TIM10,		RCC_TIM10CFGR,		1,	0),
	GATE_CFG(GATE_TIM11,		RCC_TIM11CFGR,		1,	0),
	GATE_CFG(GATE_TIM12,		RCC_TIM12CFGR,		1,	0),
	GATE_CFG(GATE_TIM13,		RCC_TIM13CFGR,		1,	0),
	GATE_CFG(GATE_TIM14,		RCC_TIM14CFGR,		1,	0),
	GATE_CFG(GATE_TIM15,		RCC_TIM15CFGR,		1,	0),
	GATE_CFG(GATE_TIM16,		RCC_TIM16CFGR,		1,	0),
	GATE_CFG(GATE_TIM17,		RCC_TIM17CFGR,		1,	0),
	GATE_CFG(GATE_TIM20,		RCC_TIM20CFGR,		1,	0),
	GATE_CFG(GATE_LPTIM1,		RCC_LPTIM1CFGR,		1,	0),
	GATE_CFG(GATE_LPTIM2,		RCC_LPTIM2CFGR,		1,	0),
	GATE_CFG(GATE_LPTIM3,		RCC_LPTIM3CFGR,		1,	0),
	GATE_CFG(GATE_LPTIM4,		RCC_LPTIM4CFGR,		1,	0),
	GATE_CFG(GATE_LPTIM5,		RCC_LPTIM5CFGR,		1,	0),
	GATE_CFG(GATE_SPI1,		RCC_SPI1CFGR,		1,	0),
	GATE_CFG(GATE_SPI2,		RCC_SPI2CFGR,		1,	0),
	GATE_CFG(GATE_SPI3,		RCC_SPI3CFGR,		1,	0),
	GATE_CFG(GATE_SPI4,		RCC_SPI4CFGR,		1,	0),
	GATE_CFG(GATE_SPI5,		RCC_SPI5CFGR,		1,	0),
	GATE_CFG(GATE_SPI6,		RCC_SPI6CFGR,		1,	0),
	GATE_CFG(GATE_SPI7,		RCC_SPI7CFGR,		1,	0),
	GATE_CFG(GATE_SPI8,		RCC_SPI8CFGR,		1,	0),
	GATE_CFG(GATE_SPDIFRX,		RCC_SPDIFRXCFGR,	1,	0),
	GATE_CFG(GATE_USART1,		RCC_USART1CFGR,		1,	0),
	GATE_CFG(GATE_USART2,		RCC_USART2CFGR,		1,	0),
	GATE_CFG(GATE_USART3,		RCC_USART3CFGR,		1,	0),
	GATE_CFG(GATE_UART4,		RCC_UART4CFGR,		1,	0),
	GATE_CFG(GATE_UART5,		RCC_UART5CFGR,		1,	0),
	GATE_CFG(GATE_USART6,		RCC_USART6CFGR,		1,	0),
	GATE_CFG(GATE_UART7,		RCC_UART7CFGR,		1,	0),
	GATE_CFG(GATE_UART8,		RCC_UART8CFGR,		1,	0),
	GATE_CFG(GATE_UART9,		RCC_UART9CFGR,		1,	0),
	GATE_CFG(GATE_LPUART1,		RCC_LPUART1CFGR,	1,	0),
	GATE_CFG(GATE_I2C1,		RCC_I2C1CFGR,		1,	0),
	GATE_CFG(GATE_I2C2,		RCC_I2C2CFGR,		1,	0),
	GATE_CFG(GATE_I2C3,		RCC_I2C3CFGR,		1,	0),
	GATE_CFG(GATE_I2C4,		RCC_I2C4CFGR,		1,	0),
	GATE_CFG(GATE_I2C5,		RCC_I2C5CFGR,		1,	0),
	GATE_CFG(GATE_I2C6,		RCC_I2C6CFGR,		1,	0),
	GATE_CFG(GATE_I2C7,		RCC_I2C7CFGR,		1,	0),
	GATE_CFG(GATE_I2C8,		RCC_I2C8CFGR,		1,	0),
	GATE_CFG(GATE_SAI1,		RCC_SAI1CFGR,		1,	0),
	GATE_CFG(GATE_SAI2,		RCC_SAI2CFGR,		1,	0),
	GATE_CFG(GATE_SAI3,		RCC_SAI3CFGR,		1,	0),
	GATE_CFG(GATE_SAI4,		RCC_SAI4CFGR,		1,	0),
	GATE_CFG(GATE_MDF1,		RCC_MDF1CFGR,		1,	0),
	GATE_CFG(GATE_ADF1,		RCC_ADF1CFGR,		1,	0),
	GATE_CFG(GATE_FDCAN,		RCC_FDCANCFGR,		1,	0),
	GATE_CFG(GATE_HDP,		RCC_HDPCFGR,		1,	0),
	GATE_CFG(GATE_ADC12,		RCC_ADC12CFGR,		1,	0),
	GATE_CFG(GATE_ADC3,		RCC_ADC3CFGR,		1,	0),
	GATE_CFG(GATE_ETH1MAC,		RCC_ETH1CFGR,		1,	0),
	GATE_CFG(GATE_ETH1STP,		RCC_ETH1CFGR,		4,	0),
	GATE_CFG(GATE_ETH1,		RCC_ETH1CFGR,		5,	0),
	GATE_CFG(GATE_ETH1TX,		RCC_ETH1CFGR,		8,	0),
	GATE_CFG(GATE_ETH1RX,		RCC_ETH1CFGR,		10,	0),
	GATE_CFG(GATE_ETH2MAC,		RCC_ETH2CFGR,		1,	0),
	GATE_CFG(GATE_ETH2STP,		RCC_ETH2CFGR,		4,	0),
	GATE_CFG(GATE_ETH2,		RCC_ETH2CFGR,		5,	0),
	GATE_CFG(GATE_ETH2TX,		RCC_ETH2CFGR,		8,	0),
	GATE_CFG(GATE_ETH2RX,		RCC_ETH2CFGR,		10,	0),
	GATE_CFG(GATE_USBH,		RCC_USBHCFGR,		1,	0),
	GATE_CFG(GATE_USB2PHY1,		RCC_USB2PHY1CFGR,	1,	0),
	GATE_CFG(GATE_USB2PHY2,		RCC_USB2PHY2CFGR,	1,	0),
	GATE_CFG(GATE_USB3DR,		RCC_USB3DRCFGR,		1,	0),
	GATE_CFG(GATE_USB3PCIEPHY,	RCC_USB3PCIEPHYCFGR,	1,	0),
	GATE_CFG(GATE_PCIE,		RCC_PCIECFGR,		1,	0),
	GATE_CFG(GATE_USBTC,		RCC_UCPDCFGR,		1,	0),
	GATE_CFG(GATE_ETHSWMAC,		RCC_ETHSWCFGR,		1,	0),
	GATE_CFG(GATE_ETHSW,		RCC_ETHSWCFGR,		5,	0),
	GATE_CFG(GATE_ETHSWREF,		RCC_ETHSWCFGR,		21,	0),
	GATE_CFG(GATE_STGEN,		RCC_STGENCFGR,		1,	0),
	GATE_CFG(GATE_SDMMC1,		RCC_SDMMC1CFGR,		1,	0),
	GATE_CFG(GATE_SDMMC2,		RCC_SDMMC2CFGR,		1,	0),
	GATE_CFG(GATE_SDMMC3,		RCC_SDMMC3CFGR,		1,	0),
	GATE_CFG(GATE_GPU,		RCC_GPUCFGR,		1,	0),
	GATE_CFG(GATE_LTDC,		RCC_LTDCCFGR,		1,	0),
	GATE_CFG(GATE_DSI,		RCC_DSICFGR,		1,	0),
	GATE_CFG(GATE_LVDS,		RCC_LVDSCFGR,		1,	0),
	GATE_CFG(GATE_CSI,		RCC_CSICFGR,		1,	0),
	GATE_CFG(GATE_DCMIPP,		RCC_DCMIPPCFGR,		1,	0),
	GATE_CFG(GATE_CCI,		RCC_CCICFGR,		1,	0),
	GATE_CFG(GATE_VDEC,		RCC_VDECCFGR,		1,	0),
	GATE_CFG(GATE_VENC,		RCC_VENCCFGR,		1,	0),
	GATE_CFG(GATE_RNG,		RCC_RNGCFGR,		1,	0),
	GATE_CFG(GATE_PKA,		RCC_PKACFGR,		1,	0),
	GATE_CFG(GATE_SAES,		RCC_SAESCFGR,		1,	0),
	GATE_CFG(GATE_HASH,		RCC_HASHCFGR,		1,	0),
	GATE_CFG(GATE_CRYP1,		RCC_CRYP1CFGR,		1,	0),
	GATE_CFG(GATE_CRYP2,		RCC_CRYP2CFGR,		1,	0),
	GATE_CFG(GATE_IWDG1,		RCC_IWDG1CFGR,		1,	0),
	GATE_CFG(GATE_IWDG2,		RCC_IWDG2CFGR,		1,	0),
	GATE_CFG(GATE_IWDG3,		RCC_IWDG3CFGR,		1,	0),
	GATE_CFG(GATE_IWDG4,		RCC_IWDG4CFGR,		1,	0),
	GATE_CFG(GATE_IWDG5,		RCC_IWDG5CFGR,		1,	0),
	GATE_CFG(GATE_WWDG1,		RCC_WWDG1CFGR,		1,	0),
	GATE_CFG(GATE_WWDG2,		RCC_WWDG2CFGR,		1,	0),
	GATE_CFG(GATE_VREF,		RCC_VREFCFGR,		1,	0),
	GATE_CFG(GATE_DTS,		RCC_DTSCFGR,		1,	0),
	GATE_CFG(GATE_CRC,		RCC_CRCCFGR,		1,	0),
	GATE_CFG(GATE_SERC,		RCC_SERCCFGR,		1,	0),
	GATE_CFG(GATE_OSPIIOM,		RCC_OSPIIOMCFGR,	1,	0),
	GATE_CFG(GATE_I3C1,		RCC_I3C1CFGR,		1,	0),
	GATE_CFG(GATE_I3C2,		RCC_I3C2CFGR,		1,	0),
	GATE_CFG(GATE_I3C3,		RCC_I3C3CFGR,		1,	0),
	GATE_CFG(GATE_I3C4,		RCC_I3C4CFGR,		1,	0),
};

#define STM32_COMPOSITE_NODIV(_id, _name, _flags, _sec_id, _gate_id, _mux_id)\
	STM32_COMPOSITE(_id, _name, _flags, _sec_id, _gate_id, _mux_id, NO_STM32_DIV)

static const struct clock_config stm32mp25_clock_cfg[] = {
	/* ADC */
	STM32_GATE(CK_BUS_ADC12, "ck_icn_p_adc12", "ck_icn_ls_mcu", 0, GATE_ADC12, SECF_NONE),
	STM32_COMPOSITE_NODIV(CK_KER_ADC12, "ck_ker_adc12", 0, SECF_NONE, GATE_ADC12, MUX_ADC12),
	STM32_GATE(CK_BUS_ADC3, "ck_icn_p_adc3", "ck_icn_ls_mcu", 0, GATE_ADC3, SECF_NONE),
	STM32_COMPOSITE_NODIV(CK_KER_ADC3, "ck_ker_adc3", 0, SECF_NONE, GATE_ADC3, MUX_ADC3),

	/* ADF */
	STM32_GATE(CK_BUS_ADF1, "ck_icn_p_adf1", "ck_icn_ls_mcu", 0, GATE_ADF1, SECF_NONE),
	STM32_GATE(CK_KER_ADF1, "ck_ker_adf1", "ck_flexgen_42", 0, GATE_ADF1, SECF_NONE),

	/* Camera */
	/* DCMI */
	STM32_GATE(CK_BUS_CCI, "ck_icn_p_cci", "ck_icn_ls_mcu", 0, GATE_CCI, SECF_NONE),

	/* CSI-HOST */
	STM32_GATE(CK_BUS_CSI, "ck_icn_p_csi", "ck_icn_apb4", 0, GATE_CSI, SECF_NONE),
	STM32_GATE(CK_KER_CSI, "ck_ker_csi", "ck_flexgen_29", 0, GATE_CSI, SECF_NONE),
	STM32_GATE(CK_KER_CSITXESC, "ck_ker_csitxesc", "ck_flexgen_30", 0, GATE_CSI, SECF_NONE),

	/* CSI-PHY */
	STM32_GATE(CK_KER_CSIPHY, "ck_ker_csiphy", "ck_flexgen_31", 0, GATE_CSI, SECF_NONE),

	/* DCMIPP */
	STM32_GATE(CK_BUS_DCMIPP, "ck_icn_p_dcmipp", "ck_icn_apb4", 0, GATE_DCMIPP, SECF_NONE),

	/* CRC */
	STM32_GATE(CK_BUS_CRC, "ck_icn_p_crc", "ck_icn_ls_mcu", 0, GATE_CRC, SECF_NONE),

	/* CRYP */
	STM32_GATE(CK_BUS_CRYP1, "ck_icn_p_cryp1", "ck_icn_ls_mcu", 0, GATE_CRYP1, SECF_NONE),
	STM32_GATE(CK_BUS_CRYP2, "ck_icn_p_cryp2", "ck_icn_ls_mcu", 0, GATE_CRYP2, SECF_NONE),

	/* DBG & TRACE*/
	STM32_GATE(CK_KER_TSDBG, "ck_ker_tsdbg", "ck_flexgen_43", 0, GATE_DBG, SECF_NONE),
	STM32_GATE(CK_KER_TPIU, "ck_ker_tpiu", "ck_flexgen_44", 0, GATE_TRACE, SECF_NONE),
	STM32_GATE(CK_BUS_SYSATB, "ck_sys_atb", "ck_flexgen_45", 0, GATE_DBG, SECF_NONE),
	STM32_GATE(CK_BUS_ETR, "ck_icn_m_etr", "ck_flexgen_45", 0, GATE_ETR, SECF_NONE),

	/* Display subsystem */
	/* LTDC */
	STM32_GATE(CK_BUS_LTDC, "ck_icn_p_ltdc", "ck_icn_apb4", 0, GATE_LTDC, SECF_NONE),
	STM32_GATE(CK_KER_LTDC, "ck_ker_ltdc", "ck_flexgen_27", CLK_SET_RATE_PARENT,
		   GATE_LTDC, SECF_NONE),

	/* DSI */
	STM32_GATE(CK_BUS_DSI, "ck_icn_p_dsi", "ck_icn_apb4", 0, GATE_DSI, SECF_NONE),
	STM32_COMPOSITE_NODIV(CK_KER_DSIBLANE, "clk_lanebyte", 0, SECF_NONE,
			      GATE_DSI, MUX_DSIBLANE),

	/* LVDS */
	STM32_GATE(CK_BUS_LVDS, "ck_icn_p_lvds", "ck_icn_apb4", 0, GATE_LVDS, SECF_NONE),

	/* DSI PHY */
	STM32_COMPOSITE_NODIV(CK_KER_DSIPHY, "ck_ker_dsiphy", 0, SECF_NONE, GATE_DSI, MUX_DSIPHY),

	/* LVDS PHY */
	STM32_COMPOSITE_NODIV(CK_KER_LVDSPHY, "ck_ker_lvdsphy", 0,
			      SECF_NONE, GATE_LVDS, MUX_LVDSPHY),

	/* DTS */
	STM32_COMPOSITE_NODIV(CK_KER_DTS, "ck_ker_dts", 0, SECF_NONE, GATE_DTS, MUX_DTS),

	/* ETHERNET */
	STM32_GATE(CK_BUS_ETH1, "ck_icn_p_eth1", "ck_icn_ls_mcu", 0, GATE_ETH1, SECF_NONE),
	STM32_GATE(CK_ETH1_STP, "ck_ker_eth1stp", "ck_icn_ls_mcu", 0, GATE_ETH1STP, SECF_NONE),
	STM32_GATE(CK_KER_ETH1, "ck_ker_eth1", "ck_flexgen_54", 0, GATE_ETH1, SECF_NONE),
	STM32_GATE(CK_KER_ETH1, "ck_ker_eth1ptp", "ck_flexgen_56", 0, GATE_ETH1, SECF_NONE),
	STM32_GATE(CK_ETH1_MAC, "ck_ker_eth1mac", "ck_icn_ls_mcu", 0, GATE_ETH1MAC, SECF_NONE),
	STM32_GATE(CK_ETH1_TX, "ck_ker_eth1tx", "ck_icn_ls_mcu", 0, GATE_ETH1TX, SECF_NONE),
	STM32_GATE(CK_ETH1_RX, "ck_ker_eth1rx", "ck_icn_ls_mcu", 0, GATE_ETH1RX, SECF_NONE),

	STM32_GATE(CK_BUS_ETH2, "ck_icn_p_eth2", "ck_icn_ls_mcu", 0, GATE_ETH2, SECF_NONE),
	STM32_GATE(CK_ETH2_STP, "ck_ker_eth2stp", "ck_icn_ls_mcu", 0, GATE_ETH2STP, SECF_NONE),
	STM32_GATE(CK_KER_ETH2, "ck_ker_eth2", "ck_flexgen_54", 0, GATE_ETH2, SECF_NONE),
	STM32_GATE(CK_KER_ETH2, "ck_ker_eth2ptp", "ck_flexgen_56", 0, GATE_ETH2, SECF_NONE),
	STM32_GATE(CK_ETH2_MAC, "ck_ker_eth2mac", "ck_icn_ls_mcu", 0, GATE_ETH2MAC, SECF_NONE),
	STM32_GATE(CK_ETH2_TX, "ck_ker_eth2tx", "ck_icn_ls_mcu", 0, GATE_ETH2TX, SECF_NONE),
	STM32_GATE(CK_ETH2_RX, "ck_ker_eth2rx", "ck_icn_ls_mcu", 0, GATE_ETH2RX, SECF_NONE),

	STM32_GATE(CK_BUS_ETHSW, "ck_icn_p_ethsw", "ck_icn_ls_mcu", 0, GATE_ETHSWMAC, SECF_NONE),
	STM32_GATE(CK_KER_ETHSW, "ck_ker_ethsw", "ck_flexgen_54", 0, GATE_ETHSW, SECF_NONE),
	STM32_GATE(CK_KER_ETHSWREF, "ck_ker_ethswref", "ck_flexgen_60", 0,
		   GATE_ETHSWREF, SECF_NONE),

	/* FDCAN */
	STM32_GATE(CK_BUS_FDCAN, "ck_icn_p_fdcan", "ck_icn_apb2", 0, GATE_FDCAN, SECF_NONE),
	STM32_GATE(CK_KER_FDCAN, "ck_ker_fdcan", "ck_flexgen_26", 0, GATE_FDCAN, SECF_NONE),

	/* GPU */
	STM32_GATE(CK_BUS_GPU, "ck_icn_m_gpu", "ck_flexgen_59", 0, GATE_GPU, SECF_NONE),

	/* HASH */
	STM32_GATE(CK_BUS_HASH, "ck_icn_p_hash", "ck_icn_ls_mcu", 0, GATE_HASH, SECF_NONE),

	/* HDP */
	STM32_GATE(CK_BUS_HDP, "ck_icn_p_hdp", "ck_icn_apb3", 0, GATE_HDP, SECF_NONE),

	/* I2C */
	STM32_GATE(CK_KER_I2C1, "ck_ker_i2c1", "ck_flexgen_12", 0, GATE_I2C1, SECF_NONE),
	STM32_GATE(CK_KER_I2C2, "ck_ker_i2c2", "ck_flexgen_12", 0, GATE_I2C2, SECF_NONE),
	STM32_GATE(CK_KER_I2C3, "ck_ker_i2c3", "ck_flexgen_13", 0, GATE_I2C3, SECF_NONE),
	STM32_GATE(CK_KER_I2C5, "ck_ker_i2c5", "ck_flexgen_13", 0, GATE_I2C5, SECF_NONE),
	STM32_GATE(CK_KER_I2C4, "ck_ker_i2c4", "ck_flexgen_14", 0, GATE_I2C4, SECF_NONE),
	STM32_GATE(CK_KER_I2C6, "ck_ker_i2c6", "ck_flexgen_14", 0, GATE_I2C6, SECF_NONE),
	STM32_GATE(CK_KER_I2C7, "ck_ker_i2c7", "ck_flexgen_15", 0, GATE_I2C7, SECF_NONE),
	STM32_GATE(CK_KER_I2C8, "ck_ker_i2c8", "ck_flexgen_38", 0, GATE_I2C8, SECF_NONE),

	/* I3C */
	STM32_GATE(CK_KER_I3C1, "ck_ker_i3c1", "ck_flexgen_12", 0, GATE_I3C1, SECF_NONE),
	STM32_GATE(CK_KER_I3C2, "ck_ker_i3c2", "ck_flexgen_12", 0, GATE_I3C2, SECF_NONE),
	STM32_GATE(CK_KER_I3C3, "ck_ker_i3c3", "ck_flexgen_13", 0, GATE_I3C3, SECF_NONE),
	STM32_GATE(CK_KER_I3C4, "ck_ker_i3c4", "ck_flexgen_36", 0, GATE_I3C4, SECF_NONE),

	/* I2S */
	STM32_GATE(CK_BUS_IS2M, "ck_icn_p_is2m", "ck_icn_apb3", 0, GATE_IS2M, SECF_NONE),

	/* IWDG */
	STM32_GATE(CK_BUS_IWDG1, "ck_icn_p_iwdg1", "ck_icn_apb3", 0, GATE_IWDG1, SECF_NONE),
	STM32_GATE(CK_BUS_IWDG2, "ck_icn_p_iwdg2", "ck_icn_apb3", 0, GATE_IWDG2, SECF_NONE),
	STM32_GATE(CK_BUS_IWDG3, "ck_icn_p_iwdg3", "ck_icn_apb3", 0, GATE_IWDG3, SECF_NONE),
	STM32_GATE(CK_BUS_IWDG4, "ck_icn_p_iwdg4", "ck_icn_apb3", 0, GATE_IWDG4, SECF_NONE),
	STM32_GATE(CK_BUS_IWDG5, "ck_icn_p_iwdg5", "ck_icn_ls_mcu", 0, GATE_IWDG5, SECF_NONE),

	/* LPTIM */
	STM32_GATE(CK_KER_LPTIM1, "ck_ker_lptim1", "ck_flexgen_07", 0, GATE_LPTIM1, SECF_NONE),
	STM32_GATE(CK_KER_LPTIM2, "ck_ker_lptim2", "ck_flexgen_07", 0, GATE_LPTIM2, SECF_NONE),
	STM32_GATE(CK_KER_LPTIM3, "ck_ker_lptim3", "ck_flexgen_40", 0, GATE_LPTIM3, SECF_NONE),
	STM32_GATE(CK_KER_LPTIM4, "ck_ker_lptim4", "ck_flexgen_41", 0, GATE_LPTIM4, SECF_NONE),
	STM32_GATE(CK_KER_LPTIM5, "ck_ker_lptim5", "ck_flexgen_41", 0, GATE_LPTIM5, SECF_NONE),

	/* LPUART */
	STM32_GATE(CK_KER_LPUART1, "ck_ker_lpuart1", "ck_flexgen_39", 0, GATE_LPUART1, SECF_NONE),

	/* MCO1 & MCO2 */
	STM32_COMPOSITE_NODIV(CK_MCO1, "ck_mco1", 0, SECF_NONE, GATE_MCO1, MUX_MCO1),
	STM32_COMPOSITE_NODIV(CK_MCO2, "ck_mco2", 0, SECF_NONE, GATE_MCO2, MUX_MCO2),

	/* MDF */
	STM32_GATE(CK_KER_MDF1, "ck_ker_mdf1", "ck_flexgen_23", 0, GATE_MDF1, SECF_NONE),

	/* OCTOSPI */
	STM32_GATE(CK_BUS_OSPI1, "ck_icn_s_ospi1,", "ck_icn_hs_mcu", 0, GATE_OSPI1, SECF_NONE),
	STM32_GATE(CK_BUS_OTFD1, "ck_icn_p_otfd1,", "ck_icn_hs_mcu", 0, GATE_OSPI1, SECF_NONE),
	STM32_GATE(CK_KER_OSPI1, "ck_ker_ospi1", "ck_flexgen_48", 0, GATE_OSPI1, SECF_NONE),
	STM32_GATE(CK_BUS_OSPI2, "ck_icn_s_ospi2,", "ck_icn_hs_mcu", 0, GATE_OSPI2, SECF_NONE),
	STM32_GATE(CK_BUS_OTFD2, "ck_icn_p_otfd2,", "ck_icn_hs_mcu", 0, GATE_OSPI2, SECF_NONE),
	STM32_GATE(CK_KER_OSPI2, "ck_ker_ospi2", "ck_flexgen_49", 0, GATE_OSPI2, SECF_NONE),
	STM32_GATE(CK_BUS_OSPIIOM, "ck_icn_p_ospiiom", "ck_icn_ls_mcu", 0, GATE_OSPIIOM, SECF_NONE),

	/* PCIE */
	STM32_GATE(CK_BUS_PCIE, "ck_icn_p_pcie", "ck_icn_ls_mcu", 0, GATE_PCIE, SECF_NONE),

	/* PKA */
	STM32_GATE(CK_BUS_PKA, "ck_icn_p_pka", "ck_icn_ls_mcu", 0, GATE_PKA, SECF_NONE),

	/* RNG */
	STM32_GATE(CK_BUS_RNG, "ck_icn_p_rng", "ck_icn_ls_mcu", CLK_IGNORE_UNUSED,
		   GATE_RNG, SECF_NONE),

	/* SAES */
	STM32_GATE(CK_BUS_SAES, "ck_icn_p_saes", "ck_icn_ls_mcu", 0, GATE_SAES, SECF_NONE),

	/* SAI [1..4] */
	STM32_GATE(CK_BUS_SAI1, "ck_icn_p_sai1", "ck_icn_apb2", 0, GATE_SAI1, SECF_NONE),
	STM32_GATE(CK_BUS_SAI2, "ck_icn_p_sai2", "ck_icn_apb2", 0, GATE_SAI2, SECF_NONE),
	STM32_GATE(CK_BUS_SAI3, "ck_icn_p_sai3", "ck_icn_apb2", 0, GATE_SAI3, SECF_NONE),
	STM32_GATE(CK_BUS_SAI4, "ck_icn_p_sai4", "ck_icn_apb2", 0, GATE_SAI4, SECF_NONE),
	STM32_GATE(CK_KER_SAI1, "ck_ker_sai1", "ck_flexgen_23", 0, GATE_SAI1, SECF_NONE),
	STM32_GATE(CK_KER_SAI2, "ck_ker_sai2", "ck_flexgen_24", 0, GATE_SAI2, SECF_NONE),
	STM32_GATE(CK_KER_SAI3, "ck_ker_sai3", "ck_flexgen_25", 0, GATE_SAI3, SECF_NONE),
	STM32_GATE(CK_KER_SAI4, "ck_ker_sai4", "ck_flexgen_25", 0, GATE_SAI4, SECF_NONE),

	/* SDMMC */
	STM32_GATE(CK_KER_SDMMC1, "ck_ker_sdmmc1", "ck_flexgen_51", 0, GATE_SDMMC1, SECF_NONE),
	STM32_GATE(CK_KER_SDMMC2, "ck_ker_sdmmc2", "ck_flexgen_52", 0, GATE_SDMMC2, SECF_NONE),
	STM32_GATE(CK_KER_SDMMC3, "ck_ker_sdmmc3", "ck_flexgen_53", 0, GATE_SDMMC3, SECF_NONE),

	/* SERC */
	STM32_GATE(CK_BUS_SERC, "ck_icn_p_serc", "ck_icn_apb3", 0, GATE_SERC, SECF_NONE),

	/* SPDIF */
	STM32_GATE(CK_KER_SPDIFRX, "ck_ker_spdifrx", "ck_flexgen_11", 0, GATE_SPDIFRX, SECF_NONE),

	/* SPI */
	STM32_GATE(CK_KER_SPI1, "ck_ker_spi1", "ck_flexgen_16", 0, GATE_SPI1, SECF_NONE),
	STM32_GATE(CK_KER_SPI2, "ck_ker_spi2", "ck_flexgen_10", 0, GATE_SPI2, SECF_NONE),
	STM32_GATE(CK_KER_SPI3, "ck_ker_spi3", "ck_flexgen_10", 0, GATE_SPI3, SECF_NONE),
	STM32_GATE(CK_KER_SPI4, "ck_ker_spi4", "ck_flexgen_17", 0, GATE_SPI4, SECF_NONE),
	STM32_GATE(CK_KER_SPI5, "ck_ker_spi5", "ck_flexgen_17", 0, GATE_SPI5, SECF_NONE),
	STM32_GATE(CK_KER_SPI6, "ck_ker_spi6", "ck_flexgen_18", 0, GATE_SPI6, SECF_NONE),
	STM32_GATE(CK_KER_SPI7, "ck_ker_spi7", "ck_flexgen_18", 0, GATE_SPI7, SECF_NONE),
	STM32_GATE(CK_KER_SPI8, "ck_ker_spi8", "ck_flexgen_37", 0, GATE_SPI8, SECF_NONE),

	/* STGEN */
	STM32_GATE(CK_KER_STGEN, "ck_ker_stgen", "ck_flexgen_33", CLK_IGNORE_UNUSED,
		   GATE_STGEN, SECF_NONE),

	/* STM500 */
	STM32_GATE(CK_BUS_STM500, "ck_icn_s_stm500", "ck_icn_ls_mcu", 0, GATE_STM500, SECF_NONE),

	/* Timers */
	STM32_GATE(CK_KER_TIM2, "ck_ker_tim2", "timg1_ck", 0, GATE_TIM2, SECF_NONE),
	STM32_GATE(CK_KER_TIM3, "ck_ker_tim3", "timg1_ck", 0, GATE_TIM3, SECF_NONE),
	STM32_GATE(CK_KER_TIM4, "ck_ker_tim4", "timg1_ck", 0, GATE_TIM4, SECF_NONE),
	STM32_GATE(CK_KER_TIM5, "ck_ker_tim5", "timg1_ck", 0, GATE_TIM5, SECF_NONE),
	STM32_GATE(CK_KER_TIM6, "ck_ker_tim6", "timg1_ck", 0, GATE_TIM6, SECF_NONE),
	STM32_GATE(CK_KER_TIM7, "ck_ker_tim7", "timg1_ck", 0, GATE_TIM7, SECF_NONE),
	STM32_GATE(CK_KER_TIM10, "ck_ker_tim10", "timg1_ck", 0, GATE_TIM10, SECF_NONE),
	STM32_GATE(CK_KER_TIM11, "ck_ker_tim11", "timg1_ck", 0, GATE_TIM11, SECF_NONE),
	STM32_GATE(CK_KER_TIM12, "ck_ker_tim12", "timg1_ck", 0, GATE_TIM12, SECF_NONE),
	STM32_GATE(CK_KER_TIM13, "ck_ker_tim13", "timg1_ck", 0, GATE_TIM13, SECF_NONE),
	STM32_GATE(CK_KER_TIM14, "ck_ker_tim14", "timg1_ck", 0, GATE_TIM14, SECF_NONE),

	STM32_GATE(CK_KER_TIM1, "ck_ker_tim1", "timg2_ck", 0, GATE_TIM1, SECF_NONE),
	STM32_GATE(CK_KER_TIM8, "ck_ker_tim8", "timg2_ck", 0, GATE_TIM8, SECF_NONE),
	STM32_GATE(CK_KER_TIM15, "ck_ker_tim15", "timg2_ck", 0, GATE_TIM15, SECF_NONE),
	STM32_GATE(CK_KER_TIM16, "ck_ker_tim16", "timg2_ck", 0, GATE_TIM16, SECF_NONE),
	STM32_GATE(CK_KER_TIM17, "ck_ker_tim17", "timg2_ck", 0, GATE_TIM17, SECF_NONE),
	STM32_GATE(CK_KER_TIM20, "ck_ker_tim20", "timg2_ck", 0, GATE_TIM20, SECF_NONE),

	/* UART/USART */
	STM32_GATE(CK_KER_USART2, "ck_ker_usart2", "ck_flexgen_08", 0, GATE_USART2, SECF_NONE),
	STM32_GATE(CK_KER_UART4, "ck_ker_uart4", "ck_flexgen_08", 0, GATE_UART4, SECF_NONE),
	STM32_GATE(CK_KER_USART3, "ck_ker_usart3", "ck_flexgen_09", 0, GATE_USART3, SECF_NONE),
	STM32_GATE(CK_KER_UART5, "ck_ker_uart5", "ck_flexgen_09", 0, GATE_UART5, SECF_NONE),
	STM32_GATE(CK_KER_USART1, "ck_ker_usart1", "ck_flexgen_19", 0, GATE_USART1, SECF_NONE),
	STM32_GATE(CK_KER_USART6, "ck_ker_usart6", "ck_flexgen_20", 0, GATE_USART6, SECF_NONE),
	STM32_GATE(CK_KER_UART7, "ck_ker_uart7", "ck_flexgen_21", 0, GATE_UART7, SECF_NONE),
	STM32_GATE(CK_KER_UART8, "ck_ker_uart8", "ck_flexgen_21", 0, GATE_UART8, SECF_NONE),
	STM32_GATE(CK_KER_UART9, "ck_ker_uart9", "ck_flexgen_22", 0, GATE_UART9, SECF_NONE),

	/* USB2PHY1 */
	STM32_COMPOSITE_NODIV(CK_KER_USB2PHY1, "ck_ker_usb2phy1", 0,
			      SECF_NONE, GATE_USB2PHY1, MUX_USB2PHY1),

	/* USBH */
	STM32_GATE(CK_BUS_USB2OHCI, "ck_icn_m_usb2ohci", "ck_icn_hsl", 0, GATE_USBH, SECF_NONE),
	STM32_GATE(CK_BUS_USB2EHCI, "ck_icn_m_usb2ehci", "ck_icn_hsl", 0, GATE_USBH, SECF_NONE),

	/* USB2PHY2 */
	STM32_COMPOSITE_NODIV(CK_KER_USB2PHY2EN, "ck_ker_usb2phy2_en", 0,
			      SECF_NONE, GATE_USB2PHY2, MUX_USB2PHY2),

	/* USB3 PCIe COMBOPHY */
	STM32_GATE(CK_BUS_USB3PCIEPHY, "ck_icn_p_usb3pciephy", "ck_icn_apb4", 0,
		   GATE_USB3PCIEPHY, SECF_NONE),

	STM32_COMPOSITE_NODIV(CK_KER_USB3PCIEPHY, "ck_ker_usb3pciephy", 0,
			      SECF_NONE, GATE_USB3PCIEPHY, MUX_USB3PCIEPHY),

	/* USB3 DRD */
	STM32_GATE(CK_BUS_USB3DR, "ck_icn_m_usb3dr", "ck_icn_hsl", 0, GATE_USB3DR, SECF_NONE),
	STM32_GATE(CK_KER_USB2PHY2, "ck_ker_usb2phy2", "ck_flexgen_58", 0,
		   GATE_USB3DR, SECF_NONE),

	/* UCPD */
	STM32_GATE(CK_BUS_USBTC, "ck_icn_p_usbtc", "ck_flexgen_35", 0, GATE_USBTC, SECF_NONE),
	STM32_GATE(CK_KER_USBTC, "ck_ker_usbtc", "ck_flexgen_35", 0, GATE_USBTC, SECF_NONE),

	/* VDEC / VENC */
	STM32_GATE(CK_BUS_VDEC, "ck_icn_p_vdec", "ck_icn_apb4", 0, GATE_VDEC, SECF_NONE),
	STM32_GATE(CK_BUS_VENC, "ck_icn_p_venc", "ck_icn_apb4", 0, GATE_VENC, SECF_NONE),

	/* VREF */
	STM32_GATE(CK_BUS_VREF, "ck_icn_p_vref", "ck_icn_apb3", 0, RCC_VREFCFGR, SECF_NONE),

	/* WWDG */
	STM32_GATE(CK_BUS_WWDG1, "ck_icn_p_wwdg1", "ck_icn_apb3", 0, GATE_WWDG1, SECF_NONE),
	STM32_GATE(CK_BUS_WWDG2, "ck_icn_p_wwdg2", "ck_icn_ls_mcu", 0, GATE_WWDG2, SECF_NONE),
};

static const struct stm32_clock_match_data stm32mp25_data = {
	.tab_clocks	= stm32mp25_clock_cfg,
	.num_clocks	= ARRAY_SIZE(stm32mp25_clock_cfg),
	.clock_data = &(const struct clk_stm32_clock_data) {
		.num_gates	= ARRAY_SIZE(stm32mp25_gates),
		.gates		= stm32mp25_gates,
		.muxes		= stm32mp25_muxes,
	},
};

static int stm32mp25_clk_probe(struct udevice *dev)
{
	fdt_addr_t base = dev_read_addr(dev->parent);
	struct udevice *scmi;

	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* force SCMI probe to register all SCMI clocks */
	uclass_get_device_by_driver(UCLASS_CLK, DM_DRIVER_GET(scmi_clock), &scmi);

	stm32_rcc_init(dev, &stm32mp25_data);

	return 0;
}

U_BOOT_DRIVER(stm32mp25_clock) = {
	.name = "stm32mp25_clk",
	.id = UCLASS_CLK,
	.ops = &stm32_clk_ops,
	.priv_auto = sizeof(struct stm32mp_rcc_priv),
	.probe = stm32mp25_clk_probe,
};
