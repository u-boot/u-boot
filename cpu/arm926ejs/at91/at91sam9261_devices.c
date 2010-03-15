/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
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
#include <asm/arch/io.h>

void at91_serial0_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) AT91_PMC_BASE;

	at91_set_a_periph(AT91_PIO_PORTC, 8, 1);		/* TXD0 */
	at91_set_a_periph(AT91_PIO_PORTC, 9, 0);		/* RXD0 */
	writel(1 << AT91SAM9261_ID_US0, &pmc->pcer);
}

void at91_serial1_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) AT91_PMC_BASE;

	at91_set_a_periph(AT91_PIO_PORTC, 12, 1);		/* TXD1 */
	at91_set_a_periph(AT91_PIO_PORTC, 13, 0);		/* RXD1 */
	writel(1 << AT91SAM9261_ID_US1, &pmc->pcer);
}

void at91_serial2_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) AT91_PMC_BASE;

	at91_set_a_periph(AT91_PIO_PORTC, 14, 1);		/* TXD2 */
	at91_set_a_periph(AT91_PIO_PORTC, 15, 0);		/* RXD2 */
	writel(1 << AT91SAM9261_ID_US2, &pmc->pcer);
}

void at91_serial3_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) AT91_PMC_BASE;

	at91_set_a_periph(AT91_PIO_PORTA, 9, 0);		/* DRXD */
	at91_set_a_periph(AT91_PIO_PORTA, 10, 1);		/* DTXD */
	writel(1 << AT91_ID_SYS, &pmc->pcer);
}

void at91_serial_hw_init(void)
{
#ifdef CONFIG_USART0
	at91_serial0_hw_init();
#endif

#ifdef CONFIG_USART1
	at91_serial1_hw_init();
#endif

#ifdef CONFIG_USART2
	at91_serial2_hw_init();
#endif

#ifdef CONFIG_USART3	/* DBGU */
	at91_serial3_hw_init();
#endif
}

#ifdef CONFIG_HAS_DATAFLASH
void at91_spi0_hw_init(unsigned long cs_mask)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) AT91_PMC_BASE;

	at91_set_a_periph(AT91_PIO_PORTA, 0, 0);	/* SPI0_MISO */
	at91_set_a_periph(AT91_PIO_PORTA, 1, 0);	/* SPI0_MOSI */
	at91_set_a_periph(AT91_PIO_PORTA, 2, 0);	/* SPI0_SPCK */

	/* Enable clock */
	writel(1 << AT91SAM9261_ID_SPI0, &pmc->pcer);

	if (cs_mask & (1 << 0)) {
		at91_set_a_periph(AT91_PIO_PORTA, 3, 1);
	}
	if (cs_mask & (1 << 1)) {
		at91_set_a_periph(AT91_PIO_PORTA, 4, 1);
	}
	if (cs_mask & (1 << 2)) {
		at91_set_a_periph(AT91_PIO_PORTA, 5, 1);
	}
	if (cs_mask & (1 << 3)) {
		at91_set_a_periph(AT91_PIO_PORTA, 6, 1);
	}
	if (cs_mask & (1 << 4)) {
		at91_set_pio_output(AT91_PIO_PORTA, 3, 1);
	}
	if (cs_mask & (1 << 5)) {
		at91_set_pio_output(AT91_PIO_PORTA, 4, 1);
	}
	if (cs_mask & (1 << 6)) {
		at91_set_pio_output(AT91_PIO_PORTA, 5, 1);
	}
	if (cs_mask & (1 << 7)) {
		at91_set_pio_output(AT91_PIO_PORTA, 6, 1);
	}
}

void at91_spi1_hw_init(unsigned long cs_mask)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) AT91_PMC_BASE;

	at91_set_a_periph(AT91_PIO_PORTB, 30, 0);	/* SPI1_MISO */
	at91_set_a_periph(AT91_PIO_PORTB, 31, 0);	/* SPI1_MOSI */
	at91_set_a_periph(AT91_PIO_PORTB, 29, 0);	/* SPI1_SPCK */

	/* Enable clock */
	writel(1 << AT91SAM9261_ID_SPI1, &pmc->pcer);

	if (cs_mask & (1 << 0)) {
		at91_set_a_periph(AT91_PIO_PORTB, 28, 1);
	}
	if (cs_mask & (1 << 1)) {
		at91_set_b_periph(AT91_PIO_PORTA, 24, 1);
	}
	if (cs_mask & (1 << 2)) {
		at91_set_b_periph(AT91_PIO_PORTA, 25, 1);
	}
	if (cs_mask & (1 << 3)) {
		at91_set_a_periph(AT91_PIO_PORTA, 26, 1);
	}
	if (cs_mask & (1 << 4)) {
		at91_set_pio_output(AT91_PIO_PORTB, 28, 1);
	}
	if (cs_mask & (1 << 5)) {
		at91_set_pio_output(AT91_PIO_PORTA, 24, 1);
	}
	if (cs_mask & (1 << 6)) {
		at91_set_pio_output(AT91_PIO_PORTA, 25, 1);
	}
	if (cs_mask & (1 << 7)) {
		at91_set_pio_output(AT91_PIO_PORTA, 26, 1);
	}
}
#endif
