/*
 * Copyright (C) 2006, 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_ARCH_PORTMUX_H__
#define __ASM_AVR32_ARCH_PORTMUX_H__

#include <asm/arch/gpio.h>

#define PORTMUX_PORT_A		((void *)ATMEL_BASE_PIOA)
#define PORTMUX_PORT_B		((void *)ATMEL_BASE_PIOB)
#define PORTMUX_PORT_C		((void *)ATMEL_BASE_PIOC)
#define PORTMUX_PORT_D		((void *)ATMEL_BASE_PIOD)
#define PORTMUX_PORT_E		((void *)ATMEL_BASE_PIOE)

void portmux_enable_ebi(unsigned int bus_width, unsigned int addr_width,
		unsigned long flags, unsigned long drive_strength);

#define PORTMUX_EBI_CS(x)	(1 << (x))
#define PORTMUX_EBI_NAND	(1 << 6)
#define PORTMUX_EBI_CF(x)	(1 << ((x) + 7))
#define PORTMUX_EBI_NWAIT	(1 << 9)

#ifdef AT32AP700x_CHIP_HAS_USART
static inline void portmux_enable_usart0(unsigned long drive_strength)
{
	portmux_select_peripheral(PORTMUX_PORT_A, (1 << 8) | (1 << 9),
			PORTMUX_FUNC_B, 0);
}

static inline void portmux_enable_usart1(unsigned long drive_strength)
{
	portmux_select_peripheral(PORTMUX_PORT_A, (1 << 17) | (1 << 18),
			PORTMUX_FUNC_A, 0);
}

static inline void portmux_enable_usart2(unsigned long drive_strength)
{
	portmux_select_peripheral(PORTMUX_PORT_B, (1 << 26) | (1 << 27),
			PORTMUX_FUNC_B, 0);
}

static inline void portmux_enable_usart3(unsigned long drive_strength)
{
	portmux_select_peripheral(PORTMUX_PORT_B, (1 << 17) | (1 << 18),
			PORTMUX_FUNC_B, 0);
}
#endif
#ifdef AT32AP700x_CHIP_HAS_MACB
void portmux_enable_macb0(unsigned long flags, unsigned long drive_strength);
void portmux_enable_macb1(unsigned long flags, unsigned long drive_strength);

#define PORTMUX_MACB_RMII	(0)
#define PORTMUX_MACB_MII	(1 << 0)
#define PORTMUX_MACB_SPEED	(1 << 1)

#endif
#ifdef AT32AP700x_CHIP_HAS_MMCI
void portmux_enable_mmci(unsigned int slot, unsigned long flags,
		unsigned long drive_strength);

#define PORTMUX_MMCI_4BIT	(1 << 0)
#define PORTMUX_MMCI_8BIT	(PORTMUX_MMCI_4BIT | (1 << 1))
#define PORTMUX_MMCI_EXT_PULLUP	(1 << 2)

#endif
#ifdef AT32AP700x_CHIP_HAS_SPI
void portmux_enable_spi0(unsigned long cs_mask, unsigned long drive_strength);
void portmux_enable_spi1(unsigned long cs_mask, unsigned long drive_strength);
#endif
#ifdef AT32AP700x_CHIP_HAS_LCDC
void portmux_enable_lcdc(int pin_config);
#endif

#endif /* __ASM_AVR32_ARCH_PORTMUX_H__ */
