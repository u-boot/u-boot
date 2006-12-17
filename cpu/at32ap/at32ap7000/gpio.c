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

void gpio_enable_macb0(void)
{
	gpio_select_periph_A(GPIO_PIN_PC3,  0);	/* TXD0	*/
	gpio_select_periph_A(GPIO_PIN_PC4,  0);	/* TXD1	*/
	gpio_select_periph_A(GPIO_PIN_PC7,  0);	/* TXEN	*/
	gpio_select_periph_A(GPIO_PIN_PC8,  0);	/* TXCK */
	gpio_select_periph_A(GPIO_PIN_PC9,  0);	/* RXD0	*/
	gpio_select_periph_A(GPIO_PIN_PC10, 0);	/* RXD1	*/
	gpio_select_periph_A(GPIO_PIN_PC13, 0);	/* RXER	*/
	gpio_select_periph_A(GPIO_PIN_PC15, 0);	/* RXDV	*/
	gpio_select_periph_A(GPIO_PIN_PC16, 0);	/* MDC	*/
	gpio_select_periph_A(GPIO_PIN_PC17, 0);	/* MDIO	*/
#if !defined(CONFIG_RMII)
	gpio_select_periph_A(GPIO_PIN_PC0,  0);	/* COL	*/
	gpio_select_periph_A(GPIO_PIN_PC1,  0);	/* CRS	*/
	gpio_select_periph_A(GPIO_PIN_PC2,  0);	/* TXER	*/
	gpio_select_periph_A(GPIO_PIN_PC5,  0);	/* TXD2	*/
	gpio_select_periph_A(GPIO_PIN_PC6,  0);	/* TXD3 */
	gpio_select_periph_A(GPIO_PIN_PC11, 0);	/* RXD2	*/
	gpio_select_periph_A(GPIO_PIN_PC12, 0);	/* RXD3	*/
	gpio_select_periph_A(GPIO_PIN_PC14, 0);	/* RXCK	*/
	gpio_select_periph_A(GPIO_PIN_PC18, 0);	/* SPD	*/
#endif
}

void gpio_enable_macb1(void)
{
	gpio_select_periph_B(GPIO_PIN_PD13, 0);	/* TXD0	*/
	gpio_select_periph_B(GPIO_PIN_PD14, 0);	/* TXD1	*/
	gpio_select_periph_B(GPIO_PIN_PD11, 0);	/* TXEN	*/
	gpio_select_periph_B(GPIO_PIN_PD12, 0);	/* TXCK */
	gpio_select_periph_B(GPIO_PIN_PD10, 0);	/* RXD0	*/
	gpio_select_periph_B(GPIO_PIN_PD6,  0);	/* RXD1	*/
	gpio_select_periph_B(GPIO_PIN_PD5,  0);	/* RXER	*/
	gpio_select_periph_B(GPIO_PIN_PD4,  0);	/* RXDV	*/
	gpio_select_periph_B(GPIO_PIN_PD3,  0);	/* MDC	*/
	gpio_select_periph_B(GPIO_PIN_PD2,  0);	/* MDIO	*/
#if !defined(CONFIG_RMII)
	gpio_select_periph_B(GPIO_PIN_PC19, 0);	/* COL	*/
	gpio_select_periph_B(GPIO_PIN_PC23, 0);	/* CRS	*/
	gpio_select_periph_B(GPIO_PIN_PC26, 0);	/* TXER	*/
	gpio_select_periph_B(GPIO_PIN_PC27, 0);	/* TXD2	*/
	gpio_select_periph_B(GPIO_PIN_PC28, 0);	/* TXD3 */
	gpio_select_periph_B(GPIO_PIN_PC29, 0);	/* RXD2	*/
	gpio_select_periph_B(GPIO_PIN_PC30, 0);	/* RXD3	*/
	gpio_select_periph_B(GPIO_PIN_PC24, 0);	/* RXCK	*/
	gpio_select_periph_B(GPIO_PIN_PD15, 0);	/* SPD	*/
#endif
}

void gpio_enable_mmci(void)
{
	gpio_select_periph_A(GPIO_PIN_PA10, 0);	/* CLK	 */
	gpio_select_periph_A(GPIO_PIN_PA11, 0);	/* CMD	 */
	gpio_select_periph_A(GPIO_PIN_PA12, 0);	/* DATA0 */
	gpio_select_periph_A(GPIO_PIN_PA13, 0);	/* DATA1 */
	gpio_select_periph_A(GPIO_PIN_PA14, 0);	/* DATA2 */
	gpio_select_periph_A(GPIO_PIN_PA15, 0);	/* DATA3 */
}
