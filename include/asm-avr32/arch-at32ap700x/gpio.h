/*
 * Copyright (C) 2006 Atmel Corporation
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
#define GPIO_PIN_PA0	(GPIO_PIOA_BASE +  0)
#define GPIO_PIN_PA1	(GPIO_PIOA_BASE +  1)
#define GPIO_PIN_PA2	(GPIO_PIOA_BASE +  2)
#define GPIO_PIN_PA3	(GPIO_PIOA_BASE +  3)
#define GPIO_PIN_PA4	(GPIO_PIOA_BASE +  4)
#define GPIO_PIN_PA5	(GPIO_PIOA_BASE +  5)
#define GPIO_PIN_PA6	(GPIO_PIOA_BASE +  6)
#define GPIO_PIN_PA7	(GPIO_PIOA_BASE +  7)
#define GPIO_PIN_PA8	(GPIO_PIOA_BASE +  8)
#define GPIO_PIN_PA9	(GPIO_PIOA_BASE +  9)
#define GPIO_PIN_PA10	(GPIO_PIOA_BASE + 10)
#define GPIO_PIN_PA11	(GPIO_PIOA_BASE + 11)
#define GPIO_PIN_PA12	(GPIO_PIOA_BASE + 12)
#define GPIO_PIN_PA13	(GPIO_PIOA_BASE + 13)
#define GPIO_PIN_PA14	(GPIO_PIOA_BASE + 14)
#define GPIO_PIN_PA15	(GPIO_PIOA_BASE + 15)
#define GPIO_PIN_PA16	(GPIO_PIOA_BASE + 16)
#define GPIO_PIN_PA17	(GPIO_PIOA_BASE + 17)
#define GPIO_PIN_PA18	(GPIO_PIOA_BASE + 18)
#define GPIO_PIN_PA19	(GPIO_PIOA_BASE + 19)
#define GPIO_PIN_PA20	(GPIO_PIOA_BASE + 20)
#define GPIO_PIN_PA21	(GPIO_PIOA_BASE + 21)
#define GPIO_PIN_PA22	(GPIO_PIOA_BASE + 22)
#define GPIO_PIN_PA23	(GPIO_PIOA_BASE + 23)
#define GPIO_PIN_PA24	(GPIO_PIOA_BASE + 24)
#define GPIO_PIN_PA25	(GPIO_PIOA_BASE + 25)
#define GPIO_PIN_PA26	(GPIO_PIOA_BASE + 26)
#define GPIO_PIN_PA27	(GPIO_PIOA_BASE + 27)
#define GPIO_PIN_PA28	(GPIO_PIOA_BASE + 28)
#define GPIO_PIN_PA29	(GPIO_PIOA_BASE + 29)
#define GPIO_PIN_PA30	(GPIO_PIOA_BASE + 30)
#define GPIO_PIN_PA31	(GPIO_PIOA_BASE + 31)

#define GPIO_PIOB_BASE	(GPIO_PIOA_BASE + 32)
#define GPIO_PIN_PB0	(GPIO_PIOB_BASE +  0)
#define GPIO_PIN_PB1	(GPIO_PIOB_BASE +  1)
#define GPIO_PIN_PB2	(GPIO_PIOB_BASE +  2)
#define GPIO_PIN_PB3	(GPIO_PIOB_BASE +  3)
#define GPIO_PIN_PB4	(GPIO_PIOB_BASE +  4)
#define GPIO_PIN_PB5	(GPIO_PIOB_BASE +  5)
#define GPIO_PIN_PB6	(GPIO_PIOB_BASE +  6)
#define GPIO_PIN_PB7	(GPIO_PIOB_BASE +  7)
#define GPIO_PIN_PB8	(GPIO_PIOB_BASE +  8)
#define GPIO_PIN_PB9	(GPIO_PIOB_BASE +  9)
#define GPIO_PIN_PB10	(GPIO_PIOB_BASE + 10)
#define GPIO_PIN_PB11	(GPIO_PIOB_BASE + 11)
#define GPIO_PIN_PB12	(GPIO_PIOB_BASE + 12)
#define GPIO_PIN_PB13	(GPIO_PIOB_BASE + 13)
#define GPIO_PIN_PB14	(GPIO_PIOB_BASE + 14)
#define GPIO_PIN_PB15	(GPIO_PIOB_BASE + 15)
#define GPIO_PIN_PB16	(GPIO_PIOB_BASE + 16)
#define GPIO_PIN_PB17	(GPIO_PIOB_BASE + 17)
#define GPIO_PIN_PB18	(GPIO_PIOB_BASE + 18)
#define GPIO_PIN_PB19	(GPIO_PIOB_BASE + 19)
#define GPIO_PIN_PB20	(GPIO_PIOB_BASE + 20)
#define GPIO_PIN_PB21	(GPIO_PIOB_BASE + 21)
#define GPIO_PIN_PB22	(GPIO_PIOB_BASE + 22)
#define GPIO_PIN_PB23	(GPIO_PIOB_BASE + 23)
#define GPIO_PIN_PB24	(GPIO_PIOB_BASE + 24)
#define GPIO_PIN_PB25	(GPIO_PIOB_BASE + 25)
#define GPIO_PIN_PB26	(GPIO_PIOB_BASE + 26)
#define GPIO_PIN_PB27	(GPIO_PIOB_BASE + 27)
#define GPIO_PIN_PB28	(GPIO_PIOB_BASE + 28)
#define GPIO_PIN_PB29	(GPIO_PIOB_BASE + 29)
#define GPIO_PIN_PB30	(GPIO_PIOB_BASE + 30)

#define GPIO_PIOC_BASE	(GPIO_PIOB_BASE + 32)
#define GPIO_PIN_PC0	(GPIO_PIOC_BASE +  0)
#define GPIO_PIN_PC1	(GPIO_PIOC_BASE +  1)
#define GPIO_PIN_PC2	(GPIO_PIOC_BASE +  2)
#define GPIO_PIN_PC3	(GPIO_PIOC_BASE +  3)
#define GPIO_PIN_PC4	(GPIO_PIOC_BASE +  4)
#define GPIO_PIN_PC5	(GPIO_PIOC_BASE +  5)
#define GPIO_PIN_PC6	(GPIO_PIOC_BASE +  6)
#define GPIO_PIN_PC7	(GPIO_PIOC_BASE +  7)
#define GPIO_PIN_PC8	(GPIO_PIOC_BASE +  8)
#define GPIO_PIN_PC9	(GPIO_PIOC_BASE +  9)
#define GPIO_PIN_PC10	(GPIO_PIOC_BASE + 10)
#define GPIO_PIN_PC11	(GPIO_PIOC_BASE + 11)
#define GPIO_PIN_PC12	(GPIO_PIOC_BASE + 12)
#define GPIO_PIN_PC13	(GPIO_PIOC_BASE + 13)
#define GPIO_PIN_PC14	(GPIO_PIOC_BASE + 14)
#define GPIO_PIN_PC15	(GPIO_PIOC_BASE + 15)
#define GPIO_PIN_PC16	(GPIO_PIOC_BASE + 16)
#define GPIO_PIN_PC17	(GPIO_PIOC_BASE + 17)
#define GPIO_PIN_PC18	(GPIO_PIOC_BASE + 18)
#define GPIO_PIN_PC19	(GPIO_PIOC_BASE + 19)
#define GPIO_PIN_PC20	(GPIO_PIOC_BASE + 20)
#define GPIO_PIN_PC21	(GPIO_PIOC_BASE + 21)
#define GPIO_PIN_PC22	(GPIO_PIOC_BASE + 22)
#define GPIO_PIN_PC23	(GPIO_PIOC_BASE + 23)
#define GPIO_PIN_PC24	(GPIO_PIOC_BASE + 24)
#define GPIO_PIN_PC25	(GPIO_PIOC_BASE + 25)
#define GPIO_PIN_PC26	(GPIO_PIOC_BASE + 26)
#define GPIO_PIN_PC27	(GPIO_PIOC_BASE + 27)
#define GPIO_PIN_PC28	(GPIO_PIOC_BASE + 28)
#define GPIO_PIN_PC29	(GPIO_PIOC_BASE + 29)
#define GPIO_PIN_PC30	(GPIO_PIOC_BASE + 30)
#define GPIO_PIN_PC31	(GPIO_PIOC_BASE + 31)

#define GPIO_PIOD_BASE	(GPIO_PIOC_BASE + 32)
#define GPIO_PIN_PD0	(GPIO_PIOD_BASE +  0)
#define GPIO_PIN_PD1	(GPIO_PIOD_BASE +  1)
#define GPIO_PIN_PD2	(GPIO_PIOD_BASE +  2)
#define GPIO_PIN_PD3	(GPIO_PIOD_BASE +  3)
#define GPIO_PIN_PD4	(GPIO_PIOD_BASE +  4)
#define GPIO_PIN_PD5	(GPIO_PIOD_BASE +  5)
#define GPIO_PIN_PD6	(GPIO_PIOD_BASE +  6)
#define GPIO_PIN_PD7	(GPIO_PIOD_BASE +  7)
#define GPIO_PIN_PD8	(GPIO_PIOD_BASE +  8)
#define GPIO_PIN_PD9	(GPIO_PIOD_BASE +  9)
#define GPIO_PIN_PD10	(GPIO_PIOD_BASE + 10)
#define GPIO_PIN_PD11	(GPIO_PIOD_BASE + 11)
#define GPIO_PIN_PD12	(GPIO_PIOD_BASE + 12)
#define GPIO_PIN_PD13	(GPIO_PIOD_BASE + 13)
#define GPIO_PIN_PD14	(GPIO_PIOD_BASE + 14)
#define GPIO_PIN_PD15	(GPIO_PIOD_BASE + 15)
#define GPIO_PIN_PD16	(GPIO_PIOD_BASE + 16)
#define GPIO_PIN_PD17	(GPIO_PIOD_BASE + 17)

#define GPIO_PIOE_BASE	(GPIO_PIOD_BASE + 32)
#define GPIO_PIN_PE0	(GPIO_PIOE_BASE +  0)
#define GPIO_PIN_PE1	(GPIO_PIOE_BASE +  1)
#define GPIO_PIN_PE2	(GPIO_PIOE_BASE +  2)
#define GPIO_PIN_PE3	(GPIO_PIOE_BASE +  3)
#define GPIO_PIN_PE4	(GPIO_PIOE_BASE +  4)
#define GPIO_PIN_PE5	(GPIO_PIOE_BASE +  5)
#define GPIO_PIN_PE6	(GPIO_PIOE_BASE +  6)
#define GPIO_PIN_PE7	(GPIO_PIOE_BASE +  7)
#define GPIO_PIN_PE8	(GPIO_PIOE_BASE +  8)
#define GPIO_PIN_PE9	(GPIO_PIOE_BASE +  9)
#define GPIO_PIN_PE10	(GPIO_PIOE_BASE + 10)
#define GPIO_PIN_PE11	(GPIO_PIOE_BASE + 11)
#define GPIO_PIN_PE12	(GPIO_PIOE_BASE + 12)
#define GPIO_PIN_PE13	(GPIO_PIOE_BASE + 13)
#define GPIO_PIN_PE14	(GPIO_PIOE_BASE + 14)
#define GPIO_PIN_PE15	(GPIO_PIOE_BASE + 15)
#define GPIO_PIN_PE16	(GPIO_PIOE_BASE + 16)
#define GPIO_PIN_PE17	(GPIO_PIOE_BASE + 17)
#define GPIO_PIN_PE18	(GPIO_PIOE_BASE + 18)
#define GPIO_PIN_PE19	(GPIO_PIOE_BASE + 19)
#define GPIO_PIN_PE20	(GPIO_PIOE_BASE + 20)
#define GPIO_PIN_PE21	(GPIO_PIOE_BASE + 21)
#define GPIO_PIN_PE22	(GPIO_PIOE_BASE + 22)
#define GPIO_PIN_PE23	(GPIO_PIOE_BASE + 23)
#define GPIO_PIN_PE24	(GPIO_PIOE_BASE + 24)
#define GPIO_PIN_PE25	(GPIO_PIOE_BASE + 25)
#define GPIO_PIN_PE26	(GPIO_PIOE_BASE + 26)

#define GPIOF_PULLUP	0x00000001	/* (not-OUT) Enable pull-up */
#define GPIOF_OUTPUT	0x00000002	/* (OUT) Enable output driver */
#define GPIOF_DEGLITCH	0x00000004	/* (IN) Filter glitches */
#define GPIOF_MULTIDRV	0x00000008	/* Enable multidriver option */

static inline void *gpio_pin_to_addr(unsigned int pin)
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

void gpio_select_periph_A(unsigned int pin, int use_pullup);
void gpio_select_periph_B(unsigned int pin, int use_pullup);
void gpio_select_pio(unsigned int pin, unsigned long gpiof_flags);
void gpio_set_value(unsigned int pin, int value);
int gpio_get_value(unsigned int pin);

void gpio_enable_ebi(void);

#ifdef AT32AP700x_CHIP_HAS_USART
void gpio_enable_usart0(void);
void gpio_enable_usart1(void);
void gpio_enable_usart2(void);
void gpio_enable_usart3(void);
#endif
#ifdef AT32AP700x_CHIP_HAS_MACB
void gpio_enable_macb0(void);
void gpio_enable_macb1(void);
#endif
#ifdef AT32AP700x_CHIP_HAS_MMCI
void gpio_enable_mmci(void);
#endif
#ifdef AT32AP700x_CHIP_HAS_SPI
void gpio_enable_spi0(unsigned long cs_mask);
void gpio_enable_spi1(unsigned long cs_mask);
#endif

#endif /* __ASM_AVR32_ARCH_GPIO_H__ */
