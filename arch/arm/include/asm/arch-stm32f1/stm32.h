/*
 * (C) Copyright 2011
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * Copyright 2015 ATS Advanced Telematics Systems GmbH
 * Copyright 2015 Konsulko Group, Matt Porter <mporter@konsulko.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MACH_STM32_H_
#define _MACH_STM32_H_

/*
 * Peripheral memory map
 */
#define STM32_PERIPH_BASE	0x40000000
#define STM32_APB1PERIPH_BASE	(STM32_PERIPH_BASE + 0x00000000)
#define STM32_APB2PERIPH_BASE	(STM32_PERIPH_BASE + 0x00010000)
#define STM32_AHB1PERIPH_BASE	(STM32_PERIPH_BASE + 0x00018000)

#define STM32_BUS_MASK		0xFFFF0000

/*
 * Register maps
 */
struct stm32_des_regs {
	u16 flash_size;
	u16 pad1;
	u32 pad2;
	u32 uid0;
	u32 uid1;
	u32 uid2;
};

struct stm32_rcc_regs {
	u32 cr;		/* RCC clock control */
	u32 cfgr;	/* RCC clock configuration */
	u32 cir;	/* RCC clock interrupt */
	u32 apb2rstr;	/* RCC APB2 peripheral reset */
	u32 apb1rstr;	/* RCC APB1 peripheral reset */
	u32 ahbenr;	/* RCC AHB peripheral clock enable */
	u32 apb2enr;	/* RCC APB2 peripheral clock enable */
	u32 apb1enr;	/* RCC APB1 peripheral clock enable */
	u32 bdcr;	/* RCC Backup domain control */
	u32 csr;	/* RCC clock control & status */
};

struct stm32_pwr_regs {
	u32 cr;
	u32 csr;
};

struct stm32_flash_regs {
	u32 acr;
	u32 keyr;
	u32 optkeyr;
	u32 sr;
	u32 cr;
	u32 ar;
	u32 rsvd1;	/* Reserved */
	u32 obr;
	u32 wrpr;
	u32 rsvd2[8];	/* Reserved */
	u32 keyr2;
	u32 rsvd3;
	u32 sr2;
	u32 cr2;
	u32 ar2;
};

/* Per bank register set for XL devices */
struct stm32_flash_bank_regs {
	u32 keyr;
	u32 rsvd;	/* Reserved */
	u32 sr;
	u32 cr;
	u32 ar;
};

/*
 * Registers access macros
 */
#define STM32_DES_BASE		(0x1ffff7e0)
#define STM32_DES		((struct stm32_des_regs *)STM32_DES_BASE)

#define STM32_RCC_BASE		(STM32_AHB1PERIPH_BASE + 0x9000)
#define STM32_RCC		((struct stm32_rcc_regs *)STM32_RCC_BASE)

#define STM32_PWR_BASE		(STM32_APB1PERIPH_BASE + 0x7000)
#define STM32_PWR		((struct stm32_pwr_regs *)STM32_PWR_BASE)

#define STM32_FLASH_BASE	(STM32_AHB1PERIPH_BASE + 0xa000)
#define STM32_FLASH		((struct stm32_flash_regs *)STM32_FLASH_BASE)

#define STM32_FLASH_SR_BSY		(1 << 0)

#define STM32_FLASH_CR_PG		(1 << 0)
#define STM32_FLASH_CR_PER		(1 << 1)
#define STM32_FLASH_CR_STRT		(1 << 6)
#define STM32_FLASH_CR_LOCK		(1 << 7)

enum clock {
	CLOCK_CORE,
	CLOCK_AHB,
	CLOCK_APB1,
	CLOCK_APB2
};

int configure_clocks(void);
unsigned long clock_get(enum clock clck);

#endif /* _MACH_STM32_H_ */
