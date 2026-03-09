// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2026, STMicroelectronics - All Rights Reserved
 * Author(s): Gabriel Fernandez, <gabriel.fernandez@foss.st.com> for STMicroelectronics.
 */

#include <dm.h>
#include <stm32-reset-core.h>
#include <stm32mp21_rcc.h>
#include <dt-bindings/reset/st,stm32mp21-rcc.h>

/* Reset clear offset for STM32MP RCC */
#define RCC_CLR_OFFSET			0x4

/* Timeout for deassert */
#define STM32_DEASSERT_TIMEOUT_US	10000

#define RESET(id, _offset, _bit_idx, _set_clr)		\
	[id] = &(struct stm32_reset_cfg){		\
		.offset		= (_offset),		\
		.bit_idx	= (_bit_idx),		\
		.set_clr	= (_set_clr),		\
	}

static const struct stm32_reset_cfg *stm32mp21_reset[] = {
	RESET(TIM1_R,		RCC_TIM1CFGR,		0,	0),
	RESET(TIM2_R,		RCC_TIM2CFGR,		0,	0),
	RESET(TIM3_R,		RCC_TIM3CFGR,		0,	0),
	RESET(TIM4_R,		RCC_TIM4CFGR,		0,	0),
	RESET(TIM5_R,		RCC_TIM5CFGR,		0,	0),
	RESET(TIM6_R,		RCC_TIM6CFGR,		0,	0),
	RESET(TIM7_R,		RCC_TIM7CFGR,		0,	0),
	RESET(TIM8_R,		RCC_TIM8CFGR,		0,	0),
	RESET(TIM10_R,		RCC_TIM10CFGR,		0,	0),
	RESET(TIM11_R,		RCC_TIM11CFGR,		0,	0),
	RESET(TIM12_R,		RCC_TIM12CFGR,		0,	0),
	RESET(TIM13_R,		RCC_TIM13CFGR,		0,	0),
	RESET(TIM14_R,		RCC_TIM14CFGR,		0,	0),
	RESET(TIM15_R,		RCC_TIM15CFGR,		0,	0),
	RESET(TIM16_R,		RCC_TIM16CFGR,		0,	0),
	RESET(TIM17_R,		RCC_TIM17CFGR,		0,	0),
	RESET(LPTIM1_R,		RCC_LPTIM1CFGR,		0,	0),
	RESET(LPTIM2_R,		RCC_LPTIM2CFGR,		0,	0),
	RESET(LPTIM3_R,		RCC_LPTIM3CFGR,		0,	0),
	RESET(LPTIM4_R,		RCC_LPTIM4CFGR,		0,	0),
	RESET(LPTIM5_R,		RCC_LPTIM5CFGR,		0,	0),
	RESET(SPI1_R,		RCC_SPI1CFGR,		0,	0),
	RESET(SPI2_R,		RCC_SPI2CFGR,		0,	0),
	RESET(SPI3_R,		RCC_SPI3CFGR,		0,	0),
	RESET(SPI4_R,		RCC_SPI4CFGR,		0,	0),
	RESET(SPI5_R,		RCC_SPI5CFGR,		0,	0),
	RESET(SPI6_R,		RCC_SPI6CFGR,		0,	0),
	RESET(SPDIFRX_R,	RCC_SPDIFRXCFGR,	0,	0),
	RESET(USART1_R,		RCC_USART1CFGR,		0,	0),
	RESET(USART2_R,		RCC_USART2CFGR,		0,	0),
	RESET(USART3_R,		RCC_USART3CFGR,		0,	0),
	RESET(UART4_R,		RCC_UART4CFGR,		0,	0),
	RESET(UART5_R,		RCC_UART5CFGR,		0,	0),
	RESET(USART6_R,		RCC_USART6CFGR,		0,	0),
	RESET(UART7_R,		RCC_UART7CFGR,		0,	0),
	RESET(LPUART1_R,	RCC_LPUART1CFGR,	0,	0),
	RESET(I2C1_R,		RCC_I2C1CFGR,		0,	0),
	RESET(I2C2_R,		RCC_I2C2CFGR,		0,	0),
	RESET(I2C3_R,		RCC_I2C3CFGR,		0,	0),
	RESET(SAI1_R,		RCC_SAI1CFGR,		0,	0),
	RESET(SAI2_R,		RCC_SAI2CFGR,		0,	0),
	RESET(SAI3_R,		RCC_SAI3CFGR,		0,	0),
	RESET(SAI4_R,		RCC_SAI4CFGR,		0,	0),
	RESET(MDF1_R,		RCC_MDF1CFGR,		0,	0),
	RESET(FDCAN_R,		RCC_FDCANCFGR,		0,	0),
	RESET(HDP_R,		RCC_HDPCFGR,		0,	0),
	RESET(ADC1_R,		RCC_ADC1CFGR,		0,	0),
	RESET(ADC2_R,		RCC_ADC2CFGR,		0,	0),
	RESET(ETH1_R,		RCC_ETH1CFGR,		0,	0),
	RESET(ETH2_R,		RCC_ETH2CFGR,		0,	0),
	RESET(USBH_R,		RCC_USBHCFGR,		0,	0),
	RESET(USB2PHY1_R,	RCC_USB2PHY1CFGR,	0,	0),
	RESET(USB2PHY2_R,	RCC_USB2PHY2CFGR,	0,	0),
	RESET(SDMMC1_R,		RCC_SDMMC1CFGR,		0,	0),
	RESET(SDMMC1DLL_R,	RCC_SDMMC1CFGR,		16,	0),
	RESET(SDMMC2_R,		RCC_SDMMC2CFGR,		0,	0),
	RESET(SDMMC2DLL_R,	RCC_SDMMC2CFGR,		16,	0),
	RESET(SDMMC3_R,		RCC_SDMMC3CFGR,		0,	0),
	RESET(SDMMC3DLL_R,	RCC_SDMMC3CFGR,		16,	0),
	RESET(LTDC_R,		RCC_LTDCCFGR,		0,	0),
	RESET(CSI_R,		RCC_CSICFGR,		0,	0),
	RESET(DCMIPP_R,		RCC_DCMIPPCFGR,		0,	0),
	RESET(DCMIPSSI_R,	RCC_DCMIPSSICFGR,	0,	0),
	RESET(WWDG1_R,		RCC_WWDG1CFGR,		0,	0),
	RESET(VREF_R,		RCC_VREFCFGR,		0,	0),
	RESET(DTS_R,		RCC_DTSCFGR,		0,	0),
	RESET(CRC_R,		RCC_CRCCFGR,		0,	0),
	RESET(SERC_R,		RCC_SERCCFGR,		0,	0),
	RESET(I3C1_R,		RCC_I3C1CFGR,		0,	0),
	RESET(I3C2_R,		RCC_I3C2CFGR,		0,	0),
	RESET(IWDG2_KER_R,	RCC_IWDGC1CFGSETR,	18,	1),
	RESET(IWDG4_KER_R,	RCC_IWDGC2CFGSETR,	18,	1),
	RESET(RNG1_R,		RCC_RNG1CFGR,		0,	0),
	RESET(RNG2_R,		RCC_RNG2CFGR,		0,	0),
	RESET(PKA_R,		RCC_PKACFGR,		0,	0),
	RESET(SAES_R,		RCC_SAESCFGR,		0,	0),
	RESET(HASH1_R,		RCC_HASH1CFGR,		0,	0),
	RESET(HASH2_R,		RCC_HASH2CFGR,		0,	0),
	RESET(CRYP1_R,		RCC_CRYP1CFGR,		0,	0),
	RESET(CRYP2_R,		RCC_CRYP2CFGR,		0,	0),
	RESET(OTG_R,		RCC_OTGCFGR,		0,	0),
};

static const struct stm32_reset_cfg *stm32_get_reset_line(struct reset_ctl *reset_ctl)
{
	unsigned long id = reset_ctl->id;

	if (id < ARRAY_SIZE(stm32mp21_reset))
		return stm32mp21_reset[id];

	return NULL;
}

static const struct stm32_reset_data stm32mp21_reset_data = {
	.get_reset_line	= stm32_get_reset_line,
	.clear_offset	= RCC_CLR_OFFSET,
	.reset_us	= STM32_DEASSERT_TIMEOUT_US,
};

static int stm32_reset_probe(struct udevice *dev)
{
	return stm32_reset_core_probe(dev, &stm32mp21_reset_data);
}

U_BOOT_DRIVER(stm32mp21_rcc_reset) = {
	.name		= "stm32mp21_reset",
	.id		= UCLASS_RESET,
	.probe		= stm32_reset_probe,
	.priv_auto	= sizeof(struct stm32_reset_priv),
	.ops		= &stm32_reset_ops,
};
