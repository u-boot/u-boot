/*
 * Copyright (C) 2006, 2008 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __ASM_AVR32_ARCH_GPIO_H__
#define __ASM_AVR32_ARCH_GPIO_H__

#include <asm/arch/chip-features.h>
#include <asm/arch/memory-map.h>

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
		return (void *)PIOA_BASE;
	case 1:
		return (void *)PIOB_BASE;
	case 2:
		return (void *)PIOC_BASE;
	case 3:
		return (void *)PIOD_BASE;
	case 4:
		return (void *)PIOE_BASE;
	default:
		return NULL;
	}
}

#include <asm/arch-common/portmux-pio.h>

#endif /* __ASM_AVR32_ARCH_GPIO_H__ */
