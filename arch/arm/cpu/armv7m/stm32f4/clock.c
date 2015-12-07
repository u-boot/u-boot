/*
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
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

#define RCC_PLLCFGR_PLLM_MASK	0x3F
#define RCC_PLLCFGR_PLLN_MASK	0x7FC0
#define RCC_PLLCFGR_PLLP_MASK	0x30000
#define RCC_PLLCFGR_PLLQ_MASK	0xF000000
#define RCC_PLLCFGR_PLLSRC	(1 << 22)
#define RCC_PLLCFGR_PLLN_SHIFT	6
#define RCC_PLLCFGR_PLLP_SHIFT	16
#define RCC_PLLCFGR_PLLQ_SHIFT	24

#define RCC_CFGR_AHB_PSC_MASK	0xF0
#define RCC_CFGR_APB1_PSC_MASK	0x1C00
#define RCC_CFGR_APB2_PSC_MASK	0xE000
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
#define RCC_CFGR_PPRE1_SHIFT	10
#define RCC_CFGR_PPRE2_SHIFT	13

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

struct pll_psc {
	u8	pll_m;
	u16	pll_n;
	u8	pll_p;
	u8	pll_q;
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
#if (CONFIG_SYS_CLK_FREQ == 180000000)
/* 180 MHz */
struct pll_psc sys_pll_psc = {
	.pll_m = 8,
	.pll_n = 360,
	.pll_p = 2,
	.pll_q = 8,
	.ahb_psc = AHB_PSC_1,
	.apb1_psc = APB_PSC_4,
	.apb2_psc = APB_PSC_2
};
#else
/* default 168 MHz */
struct pll_psc sys_pll_psc = {
	.pll_m = 8,
	.pll_n = 336,
	.pll_p = 2,
	.pll_q = 7,
	.ahb_psc = AHB_PSC_1,
	.apb1_psc = APB_PSC_4,
	.apb2_psc = APB_PSC_2
};
#endif
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
	writel(0x24003010, &STM32_RCC->pllcfgr); /* Reset value from RM */
	clrbits_le32(&STM32_RCC->cr, RCC_CR_HSEBYP);
	writel(0, &STM32_RCC->cir); /* Disable all interrupts */

	/* Configure for HSE+PLL operation */
	setbits_le32(&STM32_RCC->cr, RCC_CR_HSEON);
	while (!(readl(&STM32_RCC->cr) & RCC_CR_HSERDY))
		;

	/* Enable high performance mode, System frequency up to 180 MHz */
	setbits_le32(&STM32_RCC->apb1enr, RCC_APB1ENR_PWREN);
	writel(PWR_CR_VOS_SCALE_MODE_1, &STM32_PWR->cr);

	setbits_le32(&STM32_RCC->cfgr, ((
		sys_pll_psc.ahb_psc << RCC_CFGR_HPRE_SHIFT)
		| (sys_pll_psc.apb1_psc << RCC_CFGR_PPRE1_SHIFT)
		| (sys_pll_psc.apb2_psc << RCC_CFGR_PPRE2_SHIFT)));

	writel(sys_pll_psc.pll_m
		| (sys_pll_psc.pll_n << RCC_PLLCFGR_PLLN_SHIFT)
		| (((sys_pll_psc.pll_p >> 1) - 1) << RCC_PLLCFGR_PLLP_SHIFT)
		| (sys_pll_psc.pll_q << RCC_PLLCFGR_PLLQ_SHIFT),
		&STM32_RCC->pllcfgr);
	setbits_le32(&STM32_RCC->pllcfgr, RCC_PLLCFGR_PLLSRC);

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
	/* Prescaler table lookups for clock computation */
	u8 ahb_psc_table[16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
	};
	u8 apb_psc_table[8] = {
		0, 0, 0, 0, 1, 2, 3, 4
	};

	if ((readl(&STM32_RCC->cfgr) & RCC_CFGR_SWS_MASK) ==
			RCC_CFGR_SWS_PLL) {
		u16 pllm, plln, pllp;
		pllm = (readl(&STM32_RCC->pllcfgr) & RCC_PLLCFGR_PLLM_MASK);
		plln = ((readl(&STM32_RCC->pllcfgr) & RCC_PLLCFGR_PLLN_MASK)
			>> RCC_PLLCFGR_PLLN_SHIFT);
		pllp = ((((readl(&STM32_RCC->pllcfgr) & RCC_PLLCFGR_PLLP_MASK)
			>> RCC_PLLCFGR_PLLP_SHIFT) + 1) << 1);
		sysclk = ((CONFIG_STM32_HSE_HZ / pllm) * plln) / pllp;
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
