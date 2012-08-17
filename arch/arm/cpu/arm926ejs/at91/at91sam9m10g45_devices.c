/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

/*
 * if CONFIG_AT91_GPIO_PULLUP ist set, keep pullups on on all
 * peripheral pins. Good to have if hardware is soldered optionally
 * or in case of SPI no slave is selected. Avoid lines to float
 * needlessly. Use a short local PUP define.
 *
 * Due to errata "TXD floats when CTS is inactive" pullups are always
 * on for TXD pins.
 */
#ifdef CONFIG_AT91_GPIO_PULLUP
# define PUP CONFIG_AT91_GPIO_PULLUP
#else
# define PUP 0
#endif

void at91_serial0_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTB, 19, 1);	/* TXD0 */
	at91_set_a_periph(AT91_PIO_PORTB, 18, PUP);	/* RXD0 */
	writel(1 << ATMEL_ID_USART0, &pmc->pcer);
}

void at91_serial1_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTB, 4, 1);		/* TXD1 */
	at91_set_a_periph(AT91_PIO_PORTB, 5, PUP);		/* RXD1 */
	writel(1 << ATMEL_ID_USART1, &pmc->pcer);
}

void at91_serial2_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTD, 6, 1);		/* TXD2 */
	at91_set_a_periph(AT91_PIO_PORTD, 7, PUP);		/* RXD2 */
	writel(1 << ATMEL_ID_USART2, &pmc->pcer);
}

void at91_seriald_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTB, 12, 0);	/* DRXD */
	at91_set_a_periph(AT91_PIO_PORTB, 13, 1);	/* DTXD */
	writel(1 << ATMEL_ID_SYS, &pmc->pcer);
}

#if defined(CONFIG_HAS_DATAFLASH) || defined(CONFIG_ATMEL_SPI)
void at91_spi0_hw_init(unsigned long cs_mask)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTB, 0, PUP);	/* SPI0_MISO */
	at91_set_a_periph(AT91_PIO_PORTB, 1, PUP);	/* SPI0_MOSI */
	at91_set_a_periph(AT91_PIO_PORTB, 2, PUP);	/* SPI0_SPCK */

	/* Enable clock */
	writel(1 << ATMEL_ID_SPI0, &pmc->pcer);

	if (cs_mask & (1 << 0)) {
		at91_set_a_periph(AT91_PIO_PORTB, 3, 1);
	}
	if (cs_mask & (1 << 1)) {
		at91_set_b_periph(AT91_PIO_PORTB, 18, 1);
	}
	if (cs_mask & (1 << 2)) {
		at91_set_b_periph(AT91_PIO_PORTB, 19, 1);
	}
	if (cs_mask & (1 << 3)) {
		at91_set_b_periph(AT91_PIO_PORTD, 27, 1);
	}
	if (cs_mask & (1 << 4)) {
		at91_set_pio_output(AT91_PIO_PORTB, 3, 1);
	}
	if (cs_mask & (1 << 5)) {
		at91_set_pio_output(AT91_PIO_PORTB, 18, 1);
	}
	if (cs_mask & (1 << 6)) {
		at91_set_pio_output(AT91_PIO_PORTB, 19, 1);
	}
	if (cs_mask & (1 << 7)) {
		at91_set_pio_output(AT91_PIO_PORTD, 27, 1);
	}
}

void at91_spi1_hw_init(unsigned long cs_mask)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTB, 14, PUP);	/* SPI1_MISO */
	at91_set_a_periph(AT91_PIO_PORTB, 15, PUP);	/* SPI1_MOSI */
	at91_set_a_periph(AT91_PIO_PORTB, 16, PUP);	/* SPI1_SPCK */

	/* Enable clock */
	writel(1 << ATMEL_ID_SPI1, &pmc->pcer);

	if (cs_mask & (1 << 0)) {
		at91_set_a_periph(AT91_PIO_PORTB, 17, 1);
	}
	if (cs_mask & (1 << 1)) {
		at91_set_b_periph(AT91_PIO_PORTD, 28, 1);
	}
	if (cs_mask & (1 << 2)) {
		at91_set_a_periph(AT91_PIO_PORTD, 18, 1);
	}
	if (cs_mask & (1 << 3)) {
		at91_set_a_periph(AT91_PIO_PORTD, 19, 1);
	}
	if (cs_mask & (1 << 4)) {
		at91_set_pio_output(AT91_PIO_PORTB, 17, 1);
	}
	if (cs_mask & (1 << 5)) {
		at91_set_pio_output(AT91_PIO_PORTD, 28, 1);
	}
	if (cs_mask & (1 << 6)) {
		at91_set_pio_output(AT91_PIO_PORTD, 18, 1);
	}
	if (cs_mask & (1 << 7)) {
		at91_set_pio_output(AT91_PIO_PORTD, 19, 1);
	}

}
#endif

#ifdef CONFIG_MACB
void at91_macb_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTA, 17, 0);	/* ETXCK_EREFCK */
	at91_set_a_periph(AT91_PIO_PORTA, 15, 0);	/* ERXDV */
	at91_set_a_periph(AT91_PIO_PORTA, 12, 0);	/* ERX0 */
	at91_set_a_periph(AT91_PIO_PORTA, 13, 0);	/* ERX1 */
	at91_set_a_periph(AT91_PIO_PORTA, 16, 0);	/* ERXER */
	at91_set_a_periph(AT91_PIO_PORTA, 14, 0);	/* ETXEN */
	at91_set_a_periph(AT91_PIO_PORTA, 10, 0);	/* ETX0 */
	at91_set_a_periph(AT91_PIO_PORTA, 11, 0);	/* ETX1 */
	at91_set_a_periph(AT91_PIO_PORTA, 19, 0);	/* EMDIO */
	at91_set_a_periph(AT91_PIO_PORTA, 18, 0);	/* EMDC */
#ifndef CONFIG_RMII
	at91_set_b_periph(AT91_PIO_PORTA, 29, 0);	/* ECRS */
	at91_set_b_periph(AT91_PIO_PORTA, 30, 0);	/* ECOL */
	at91_set_b_periph(AT91_PIO_PORTA, 8,  0);	/* ERX2 */
	at91_set_b_periph(AT91_PIO_PORTA, 9,  0);	/* ERX3 */
	at91_set_b_periph(AT91_PIO_PORTA, 28, 0);	/* ERXCK */
	at91_set_b_periph(AT91_PIO_PORTA, 6,  0);	/* ETX2 */
	at91_set_b_periph(AT91_PIO_PORTA, 7,  0);	/* ETX3 */
	at91_set_b_periph(AT91_PIO_PORTA, 27, 0);	/* ETXER */
#endif
}
#endif
