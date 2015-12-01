/*
 * (C) Copyright 2011
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MACH_STM32_H_
#define _MACH_STM32_H_

/*
 * Peripheral memory map
 */
#define STM32_SYSMEM_BASE	0x1FFF0000
#define STM32_PERIPH_BASE	0x40000000
#define STM32_APB1PERIPH_BASE	(STM32_PERIPH_BASE + 0x00000000)
#define STM32_APB2PERIPH_BASE	(STM32_PERIPH_BASE + 0x00010000)
#define STM32_AHB1PERIPH_BASE	(STM32_PERIPH_BASE + 0x00020000)
#define STM32_AHB2PERIPH_BASE	(STM32_PERIPH_BASE + 0x10000000)

#define STM32_BUS_MASK		0xFFFF0000

/*
 * Register maps
 */
struct stm32_u_id_regs {
	u32 u_id_low;
	u32 u_id_mid;
	u32 u_id_high;
};

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
	u32 pllsaicfgr;
	u32 dckcfgr;
};

struct stm32_pwr_regs {
	u32 cr;
	u32 csr;
};

struct stm32_flash_regs {
	u32 acr;
	u32 key;
	u32 optkeyr;
	u32 sr;
	u32 cr;
	u32 optcr;
	u32 optcr1;
};

/*
 * Registers access macros
 */
#define STM32_U_ID_BASE		(STM32_SYSMEM_BASE + 0x7A10)
#define STM32_U_ID		((struct stm32_u_id_regs *)STM32_U_ID_BASE)

#define STM32_RCC_BASE		(STM32_AHB1PERIPH_BASE + 0x3800)
#define STM32_RCC		((struct stm32_rcc_regs *)STM32_RCC_BASE)

#define STM32_PWR_BASE		(STM32_APB1PERIPH_BASE + 0x7000)
#define STM32_PWR		((struct stm32_pwr_regs *)STM32_PWR_BASE)

#define STM32_FLASH_BASE	(STM32_AHB1PERIPH_BASE + 0x3C00)
#define STM32_FLASH		((struct stm32_flash_regs *)STM32_FLASH_BASE)

#define STM32_FLASH_SR_BSY		(1 << 16)

#define STM32_FLASH_CR_PG		(1 << 0)
#define STM32_FLASH_CR_SER		(1 << 1)
#define STM32_FLASH_CR_STRT		(1 << 16)
#define STM32_FLASH_CR_LOCK		(1 << 31)
#define STM32_FLASH_CR_SNB_OFFSET	3
#define STM32_FLASH_CR_SNB_MASK		(15 << STM32_FLASH_CR_SNB_OFFSET)

/*
 * Peripheral base addresses
 */
#define STM32_USART1_BASE	(STM32_APB2PERIPH_BASE + 0x1000)
#define STM32_USART2_BASE	(STM32_APB1PERIPH_BASE + 0x4400)
#define STM32_USART3_BASE	(STM32_APB1PERIPH_BASE + 0x4800)
#define STM32_USART6_BASE	(STM32_APB2PERIPH_BASE + 0x1400)

enum clock {
	CLOCK_CORE,
	CLOCK_AHB,
	CLOCK_APB1,
	CLOCK_APB2
};

int configure_clocks(void);
unsigned long clock_get(enum clock clck);

#endif /* _MACH_STM32_H_ */
