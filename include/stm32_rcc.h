/*
 * Copyright (C) STMicroelectronics SA 2017
 * Author(s): Patrice CHOTARD, <patrice.chotard@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __STM32_RCC_H_
#define __STM32_RCC_H_

#define AHB_PSC_1			0
#define AHB_PSC_2			0x8
#define AHB_PSC_4			0x9
#define AHB_PSC_8			0xA
#define AHB_PSC_16			0xB
#define AHB_PSC_64			0xC
#define AHB_PSC_128			0xD
#define AHB_PSC_256			0xE
#define AHB_PSC_512			0xF

#define APB_PSC_1			0
#define APB_PSC_2			0x4
#define APB_PSC_4			0x5
#define APB_PSC_8			0x6
#define APB_PSC_16			0x7

struct pll_psc {
	u8	pll_m;
	u16	pll_n;
	u8	pll_p;
	u8	pll_q;
	u8	ahb_psc;
	u8	apb1_psc;
	u8	apb2_psc;
};

struct stm32_clk_info {
	struct pll_psc sys_pll_psc;
	bool has_overdrive;
	bool v2;
};

enum soc_family {
	STM32F4,
	STM32F7,
};

struct stm32_rcc_clk {
	char *drv_name;
	enum soc_family soc;
};

#endif /* __STM32_RCC_H_ */
