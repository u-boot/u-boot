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
#include <common.h>

#include <asm/arch/gpio.h>

/*
 * Lots of small functions here. We depend on --gc-sections getting
 * rid of the ones we don't need.
 */
void gpio_enable_ebi(void)
{
#ifdef CFG_HSDRAMC
#ifndef CFG_SDRAM_16BIT
	gpio_select_periph_A(GPIO_PIN_PE0, 0);
	gpio_select_periph_A(GPIO_PIN_PE1, 0);
	gpio_select_periph_A(GPIO_PIN_PE2, 0);
	gpio_select_periph_A(GPIO_PIN_PE3, 0);
	gpio_select_periph_A(GPIO_PIN_PE4, 0);
	gpio_select_periph_A(GPIO_PIN_PE5, 0);
	gpio_select_periph_A(GPIO_PIN_PE6, 0);
	gpio_select_periph_A(GPIO_PIN_PE7, 0);
	gpio_select_periph_A(GPIO_PIN_PE8, 0);
	gpio_select_periph_A(GPIO_PIN_PE9, 0);
	gpio_select_periph_A(GPIO_PIN_PE10, 0);
	gpio_select_periph_A(GPIO_PIN_PE11, 0);
	gpio_select_periph_A(GPIO_PIN_PE12, 0);
	gpio_select_periph_A(GPIO_PIN_PE13, 0);
	gpio_select_periph_A(GPIO_PIN_PE14, 0);
	gpio_select_periph_A(GPIO_PIN_PE15, 0);
#endif
	gpio_select_periph_A(GPIO_PIN_PE26, 0);
#endif
}

void gpio_enable_usart0(void)
{
	gpio_select_periph_B(GPIO_PIN_PA8, 0);
	gpio_select_periph_B(GPIO_PIN_PA9, 0);
}

void gpio_enable_usart1(void)
{
	gpio_select_periph_A(GPIO_PIN_PA17, 0);
	gpio_select_periph_A(GPIO_PIN_PA18, 0);
}

void gpio_enable_usart2(void)
{
	gpio_select_periph_B(GPIO_PIN_PB26, 0);
	gpio_select_periph_B(GPIO_PIN_PB27, 0);
}

void gpio_enable_usart3(void)
{
	gpio_select_periph_B(GPIO_PIN_PB18, 0);
	gpio_select_periph_B(GPIO_PIN_PB19, 0);
}
