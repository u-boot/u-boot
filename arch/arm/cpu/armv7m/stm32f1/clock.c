/*
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * Copyright 2015 ATS Advanced Telematics Systems GmbH
 * Copyright 2015 Konsulko Group, Matt Porter <mporter@konsulko.com>
 *
 * (C) Copyright 2014
 * STMicroelectronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>

#define RCC_CR_HSION		(1 << 0)
#define RCC_CR_HSEON		(1 << 16)
#define RCC_CR_HSERDY		(1 << 17)
#define RCC_CR_HSEBYP		(1 << 18)
#define RCC_CR_CSSON		(1 << 19)
#define RCC_CR_PLLON		(1 << 24)
#define RCC_CR_PLLRDY		(1 << 25)

#define RCC_CFGR_PLLMUL_MASK	0x3C0000
#define RCC_CFGR_PLLMUL_SHIFT	18
#define RCC_CFGR_PLLSRC_HSE	(1 << 16)

#define RCC_CFGR_AHB_PSC_MASK	0xF0
#define RCC_CFGR_APB1_PSC_MASK	0x700
#define RCC_CFGR_APB2_PSC_MASK	0x3800
#define RCC_CFGR_SW0		(1 << 0)
#define RCC_CFGR_SW1		(1 << 1)
#define RCC_CFGR_SW_MASK	0x3
#define RCC_CFGR_SW_HSI		0
#define RCC_CFGR_SW_HSE		RCC_CFGR_SW0
#define RCC_CFGR_SW_PLL		RCC_CFGR_SW1
#define RCC_CFGR_SWS0		(1 << 2)
#define RCC_CFGR_SWS1		(1 << 3)
#define RCC_CFGR_SWS_MASK	0xC
#define RCC_CFGR_SWS_HSI	0
#define RCC_CFGR_SWS_HSE	RCC_CFGR_SWS0
#define RCC_CFGR_SWS_PLL	RCC_CFGR_SWS1
#define RCC_CFGR_HPRE_SHIFT	4
#define RCC_CFGR_PPRE1_SHIFT	8
#define RCC_CFGR_PPRE2_SHIFT	11

#define RCC_APB1ENR_PWREN	(1 << 28)

#define PWR_CR_VOS0		(1 << 14)
#define PWR_CR_VOS1		(1 << 15)
#define PWR_CR_VOS_MASK		0xC000
#define PWR_CR_VOS_SCALE_MODE_1	(PWR_CR_VOS0 | PWR_CR_VOS1)
#define PWR_CR_VOS_SCALE_MODE_2	(PWR_CR_VOS1)
#define PWR_CR_VOS_SCALE_MODE_3	(PWR_CR_VOS0)

#define FLASH_ACR_WS(n)		n
#define FLASH_ACR_PRFTEN	(1 << 8)
#define FLASH_ACR_ICEN		(1 << 9)
#define FLASH_ACR_DCEN		(1 << 10)

struct psc {
	u8	ahb_psc;
	u8	apb1_psc;
	u8	apb2_psc;
};

#define AHB_PSC_1		0
#define AHB_PSC_2		0x8
#define AHB_PSC_4		0x9
#define AHB_PSC_8		0xA
#define AHB_PSC_16		0xB
#define AHB_PSC_64		0xC
#define AHB_PSC_128		0xD
#define AHB_PSC_256		0xE
#define AHB_PSC_512		0xF

#define APB_PSC_1		0
#define APB_PSC_2		0x4
#define APB_PSC_4		0x5
#define APB_PSC_8		0x6
#define APB_PSC_16		0x7

#if !defined(CONFIG_STM32_HSE_HZ)
#error "CONFIG_STM32_HSE_HZ not defined!"
#else
#if (CONFIG_STM32_HSE_HZ == 8000000)
#define RCC_CFGR_PLLMUL_CFG	0x7
struct psc psc_hse = {
	.ahb_psc = AHB_PSC_1,
	.apb1_psc = APB_PSC_2,
	.apb2_psc = APB_PSC_1
};
#else
#error "No PLL/Prescaler configuration for given CONFIG_STM32_HSE_HZ exists"
#endif
#endif

int configure_clocks(void)
{
	/* Reset RCC configuration */
	setbits_le32(&STM32_RCC->cr, RCC_CR_HSION);
	writel(0, &STM32_RCC->cfgr); /* Reset CFGR */
	clrbits_le32(&STM32_RCC->cr, (RCC_CR_HSEON | RCC_CR_CSSON
		| RCC_CR_PLLON));
	clrbits_le32(&STM32_RCC->cr, RCC_CR_HSEBYP);
	writel(0, &STM32_RCC->cir); /* Disable all interrupts */

	/* Configure for HSE+PLL operation */
	setbits_le32(&STM32_RCC->cr, RCC_CR_HSEON);
	while (!(readl(&STM32_RCC->cr) & RCC_CR_HSERDY))
		;

	/* Enable high performance mode, System frequency up to 168 MHz */
	setbits_le32(&STM32_RCC->apb1enr, RCC_APB1ENR_PWREN);
	writel(PWR_CR_VOS_SCALE_MODE_1, &STM32_PWR->cr);

	setbits_le32(&STM32_RCC->cfgr,
		     RCC_CFGR_PLLMUL_CFG << RCC_CFGR_PLLMUL_SHIFT);
	setbits_le32(&STM32_RCC->cfgr, RCC_CFGR_PLLSRC_HSE);
	setbits_le32(&STM32_RCC->cfgr, ((
		psc_hse.ahb_psc << RCC_CFGR_HPRE_SHIFT)
		| (psc_hse.apb1_psc << RCC_CFGR_PPRE1_SHIFT)
		| (psc_hse.apb2_psc << RCC_CFGR_PPRE2_SHIFT)));

	setbits_le32(&STM32_RCC->cr, RCC_CR_PLLON);

	while (!(readl(&STM32_RCC->cr) & RCC_CR_PLLRDY))
		;

	/* 5 wait states, Prefetch enabled, D-Cache enabled, I-Cache enabled */
	writel(FLASH_ACR_WS(5) | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN
		| FLASH_ACR_DCEN, &STM32_FLASH->acr);

	clrbits_le32(&STM32_RCC->cfgr, (RCC_CFGR_SW0 | RCC_CFGR_SW1));
	setbits_le32(&STM32_RCC->cfgr, RCC_CFGR_SW_PLL);

	while ((readl(&STM32_RCC->cfgr) & RCC_CFGR_SWS_MASK) !=
			RCC_CFGR_SWS_PLL)
		;

	return 0;
}

unsigned long clock_get(enum clock clck)
{
	u32 sysclk = 0;
	u32 shift = 0;
	/* PLL table lookups for clock computation */
	u8 pll_mul_table[16] = {
		2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16
	};
	/* Prescaler table lookups for clock computation */
	u8 ahb_psc_table[16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
	};
	u8 apb_psc_table[8] = {
		0, 0, 0, 0, 1, 2, 3, 4
	};

	if ((readl(&STM32_RCC->cfgr) & RCC_CFGR_SWS_MASK) ==
			RCC_CFGR_SWS_PLL) {
		u16 pll;
		pll = ((readl(&STM32_RCC->cfgr) & RCC_CFGR_PLLMUL_MASK)
			>> RCC_CFGR_PLLMUL_SHIFT);
		sysclk = CONFIG_STM32_HSE_HZ * pll_mul_table[pll];
	}

	switch (clck) {
	case CLOCK_CORE:
		return sysclk;
		break;
	case CLOCK_AHB:
		shift = ahb_psc_table[(
			(readl(&STM32_RCC->cfgr) & RCC_CFGR_AHB_PSC_MASK)
			>> RCC_CFGR_HPRE_SHIFT)];
		return sysclk >>= shift;
		break;
	case CLOCK_APB1:
		shift = apb_psc_table[(
			(readl(&STM32_RCC->cfgr) & RCC_CFGR_APB1_PSC_MASK)
			>> RCC_CFGR_PPRE1_SHIFT)];
		return sysclk >>= shift;
		break;
	case CLOCK_APB2:
		shift = apb_psc_table[(
			(readl(&STM32_RCC->cfgr) & RCC_CFGR_APB2_PSC_MASK)
			>> RCC_CFGR_PPRE2_SHIFT)];
		return sysclk >>= shift;
		break;
	default:
		return 0;
		break;
	}
}
