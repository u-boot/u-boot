/*
 * (C) Copyright 2016
 * Vikas Manocha, STMicroelectronics, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H

/* STM32F746 */
#define ITCM_FLASH_BASE		0x00200000UL
#define AXIM_FLASH_BASE		0x08000000UL

#define ITCM_SRAM_BASE		0x00000000UL
#define DTCM_SRAM_BASE		0x20000000UL
#define SRAM1_BASE		0x20010000UL
#define SRAM2_BASE		0x2004C000UL

#define PERIPH_BASE		0x40000000UL

#define APB1_PERIPH_BASE	(PERIPH_BASE + 0x00000000)
#define APB2_PERIPH_BASE	(PERIPH_BASE + 0x00010000)
#define AHB1_PERIPH_BASE	(PERIPH_BASE + 0x00020000)
#define AHB2_PERIPH_BASE	(PERIPH_BASE + 0x10000000)
#define AHB3_PERIPH_BASE	(PERIPH_BASE + 0x20000000)

#define TIM2_BASE		(APB1_PERIPH_BASE + 0x0000)
#define USART2_BASE		(APB1_PERIPH_BASE + 0x4400)
#define USART3_BASE		(APB1_PERIPH_BASE + 0x4800)
#define PWR_BASE		(APB1_PERIPH_BASE + 0x7000)

#define USART1_BASE		(APB2_PERIPH_BASE + 0x1000)
#define USART6_BASE		(APB2_PERIPH_BASE + 0x1400)
#define STM32_SYSCFG_BASE	(APB2_PERIPH_BASE + 0x3800)

#define STM32_GPIOA_BASE	(AHB1_PERIPH_BASE + 0x0000)
#define STM32_GPIOB_BASE	(AHB1_PERIPH_BASE + 0x0400)
#define STM32_GPIOC_BASE	(AHB1_PERIPH_BASE + 0x0800)
#define STM32_GPIOD_BASE	(AHB1_PERIPH_BASE + 0x0C00)
#define STM32_GPIOE_BASE	(AHB1_PERIPH_BASE + 0x1000)
#define STM32_GPIOF_BASE	(AHB1_PERIPH_BASE + 0x1400)
#define STM32_GPIOG_BASE	(AHB1_PERIPH_BASE + 0x1800)
#define STM32_GPIOH_BASE	(AHB1_PERIPH_BASE + 0x1C00)
#define STM32_GPIOI_BASE	(AHB1_PERIPH_BASE + 0x2000)
#define STM32_GPIOJ_BASE	(AHB1_PERIPH_BASE + 0x2400)
#define STM32_GPIOK_BASE	(AHB1_PERIPH_BASE + 0x2800)
#define RCC_BASE		(AHB1_PERIPH_BASE + 0x3800)
#define FLASH_CNTL_BASE		(AHB1_PERIPH_BASE + 0x3C00)


#define SDRAM_FMC_BASE		(AHB3_PERIPH_BASE + 0x40000140)

static const u32 sect_sz_kb[CONFIG_SYS_MAX_FLASH_SECT] = {
	[0 ... 3] =	32 * 1024,
	[4] =		128 * 1024,
	[5 ... 7] =	256 * 1024
};

#define STM32_BUS_MASK		GENMASK(31, 16)

struct stm32_rcc_regs {
	u32 cr;		/* RCC clock control */
	u32 pllcfgr;	/* RCC PLL configuration */
	u32 cfgr;	/* RCC clock configuration */
	u32 cir;	/* RCC clock interrupt */
	u32 ahb1rstr;	/* RCC AHB1 peripheral reset */
	u32 ahb2rstr;	/* RCC AHB2 peripheral reset */
	u32 ahb3rstr;	/* RCC AHB3 peripheral reset */
	u32 rsv0;
	u32 apb1rstr;	/* RCC APB1 peripheral reset */
	u32 apb2rstr;	/* RCC APB2 peripheral reset */
	u32 rsv1[2];
	u32 ahb1enr;	/* RCC AHB1 peripheral clock enable */
	u32 ahb2enr;	/* RCC AHB2 peripheral clock enable */
	u32 ahb3enr;	/* RCC AHB3 peripheral clock enable */
	u32 rsv2;
	u32 apb1enr;	/* RCC APB1 peripheral clock enable */
	u32 apb2enr;	/* RCC APB2 peripheral clock enable */
	u32 rsv3[2];
	u32 ahb1lpenr;	/* RCC AHB1 periph clk enable in low pwr mode */
	u32 ahb2lpenr;	/* RCC AHB2 periph clk enable in low pwr mode */
	u32 ahb3lpenr;	/* RCC AHB3 periph clk enable in low pwr mode */
	u32 rsv4;
	u32 apb1lpenr;	/* RCC APB1 periph clk enable in low pwr mode */
	u32 apb2lpenr;	/* RCC APB2 periph clk enable in low pwr mode */
	u32 rsv5[2];
	u32 bdcr;	/* RCC Backup domain control */
	u32 csr;	/* RCC clock control & status */
	u32 rsv6[2];
	u32 sscgr;	/* RCC spread spectrum clock generation */
	u32 plli2scfgr;	/* RCC PLLI2S configuration */
	u32 pllsaicfgr;	/* PLLSAI configuration */
	u32 dckcfgr;	/* dedicated clocks configuration register */
};
#define STM32_RCC		((struct stm32_rcc_regs *)RCC_BASE)

struct stm32_pwr_regs {
	u32 cr1;   /* power control register 1 */
	u32 csr1;  /* power control/status register 2 */
	u32 cr2;   /* power control register 2 */
	u32 csr2;  /* power control/status register 2 */
};
#define STM32_PWR		((struct stm32_pwr_regs *)PWR_BASE)

void stm32_flash_latency_cfg(int latency);

#endif /* _ASM_ARCH_HARDWARE_H */
