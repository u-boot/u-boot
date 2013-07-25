/*
 * Copyright (C) 2006, 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_ARCH_GPIO_H__
#define __ASM_AVR32_ARCH_GPIO_H__

#include <asm/arch/chip-features.h>
#include <asm/arch/hardware.h>

#define NR_GPIO_CONTROLLERS	5

/*
 * Pin numbers identifying specific GPIO pins on the chip.
 */
#define GPIO_PIOA_BASE	(0)
#define GPIO_PIOB_BASE	(GPIO_PIOA_BASE + 32)
#define GPIO_PIOC_BASE	(GPIO_PIOB_BASE + 32)
#define GPIO_PIOD_BASE	(GPIO_PIOC_BASE + 32)
#define GPIO_PIOE_BASE	(GPIO_PIOD_BASE + 32)
#define GPIO_PIN_PA(x)	(GPIO_PIOA_BASE + (x))
#define GPIO_PIN_PB(x)	(GPIO_PIOB_BASE + (x))
#define GPIO_PIN_PC(x)	(GPIO_PIOC_BASE + (x))
#define GPIO_PIN_PD(x)	(GPIO_PIOD_BASE + (x))
#define GPIO_PIN_PE(x)	(GPIO_PIOE_BASE + (x))

static inline void *pio_pin_to_port(unsigned int pin)
{
	switch (pin >> 5) {
	case 0:
		return (void *)ATMEL_BASE_PIOA;
	case 1:
		return (void *)ATMEL_BASE_PIOB;
	case 2:
		return (void *)ATMEL_BASE_PIOC;
	case 3:
		return (void *)ATMEL_BASE_PIOD;
	case 4:
		return (void *)ATMEL_BASE_PIOE;
	default:
		return NULL;
	}
}

#include <asm/arch-common/portmux-pio.h>

#endif /* __ASM_AVR32_ARCH_GPIO_H__ */
