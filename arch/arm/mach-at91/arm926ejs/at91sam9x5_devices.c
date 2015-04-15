/*
 * Copyright (C) 2012 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

unsigned int get_chip_id(void)
{
	/* The 0x40 is the offset of cidr in DBGU */
	return readl(ATMEL_BASE_DBGU + 0x40) & ~ARCH_ID_VERSION_MASK;
}

unsigned int get_extension_chip_id(void)
{
	/* The 0x44 is the offset of exid in DBGU */
	return readl(ATMEL_BASE_DBGU + 0x44);
}

unsigned int has_emac1()
{
	return cpu_is_at91sam9x25();
}

unsigned int has_emac0()
{
	return !(cpu_is_at91sam9g15());
}

unsigned int has_lcdc()
{
	return cpu_is_at91sam9g15() || cpu_is_at91sam9g35()
		|| cpu_is_at91sam9x35();
}

char *get_cpu_name()
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_at91sam9x5()) {
		switch (extension_id) {
		case ARCH_EXID_AT91SAM9G15:
			return "AT91SAM9G15";
		case ARCH_EXID_AT91SAM9G25:
			return "AT91SAM9G25";
		case ARCH_EXID_AT91SAM9G35:
			return "AT91SAM9G35";
		case ARCH_EXID_AT91SAM9X25:
			return "AT91SAM9X25";
		case ARCH_EXID_AT91SAM9X35:
			return "AT91SAM9X35";
		default:
			return "Unknown CPU type";
		}
	} else {
		return "Unknown CPU type";
	}
}

void at91_seriald_hw_init(void)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTA, 9, 0);	/* DRXD */
	at91_set_a_periph(AT91_PIO_PORTA, 10, 1);	/* DTXD */

	writel(1 << ATMEL_ID_SYS, &pmc->pcer);
}

void at91_serial0_hw_init(void)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTA, 0, 1);	/* TXD */
	at91_set_a_periph(AT91_PIO_PORTA, 1, 0);	/* RXD */

	writel(1 << ATMEL_ID_USART0, &pmc->pcer);
}

void at91_serial1_hw_init(void)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTA, 5, 1);	/* TXD */
	at91_set_a_periph(AT91_PIO_PORTA, 6, 0);	/* RXD */

	writel(1 << ATMEL_ID_USART1, &pmc->pcer);
}

void at91_serial2_hw_init(void)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTA, 7, 1);	/* TXD */
	at91_set_a_periph(AT91_PIO_PORTA, 8, 0);	/* RXD */

	writel(1 << ATMEL_ID_USART2, &pmc->pcer);
}

void at91_mci_hw_init(void)
{
	/* Initialize the MCI0 */
	at91_set_a_periph(AT91_PIO_PORTA, 17, 1);	/* MCCK */
	at91_set_a_periph(AT91_PIO_PORTA, 16, 1);	/* MCCDA */
	at91_set_a_periph(AT91_PIO_PORTA, 15, 1);	/* MCDA0 */
	at91_set_a_periph(AT91_PIO_PORTA, 18, 1);	/* MCDA1 */
	at91_set_a_periph(AT91_PIO_PORTA, 19, 1);	/* MCDA2 */
	at91_set_a_periph(AT91_PIO_PORTA, 20, 1);	/* MCDA3 */

	/* Enable clock for MCI0 */
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	writel(1 << ATMEL_ID_HSMCI0, &pmc->pcer);
}

#ifdef CONFIG_ATMEL_SPI
void at91_spi0_hw_init(unsigned long cs_mask)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTA, 11, 0);	/* SPI0_MISO */
	at91_set_a_periph(AT91_PIO_PORTA, 12, 0);	/* SPI0_MOSI */
	at91_set_a_periph(AT91_PIO_PORTA, 13, 0);	/* SPI0_SPCK */

	/* Enable clock */
	writel(1 << ATMEL_ID_SPI0, &pmc->pcer);

	if (cs_mask & (1 << 0))
		at91_set_a_periph(AT91_PIO_PORTA, 14, 0);
	if (cs_mask & (1 << 1))
		at91_set_b_periph(AT91_PIO_PORTA, 7, 0);
	if (cs_mask & (1 << 2))
		at91_set_b_periph(AT91_PIO_PORTA, 1, 0);
	if (cs_mask & (1 << 3))
		at91_set_b_periph(AT91_PIO_PORTB, 3, 0);
	if (cs_mask & (1 << 4))
		at91_set_pio_output(AT91_PIO_PORTA, 14, 0);
	if (cs_mask & (1 << 5))
		at91_set_pio_output(AT91_PIO_PORTA, 7, 0);
	if (cs_mask & (1 << 6))
		at91_set_pio_output(AT91_PIO_PORTA, 1, 0);
	if (cs_mask & (1 << 7))
		at91_set_pio_output(AT91_PIO_PORTB, 3, 0);
}

void at91_spi1_hw_init(unsigned long cs_mask)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_b_periph(AT91_PIO_PORTA, 21, 0);	/* SPI1_MISO */
	at91_set_b_periph(AT91_PIO_PORTA, 22, 0);	/* SPI1_MOSI */
	at91_set_b_periph(AT91_PIO_PORTA, 23, 0);	/* SPI1_SPCK */

	/* Enable clock */
	writel(1 << ATMEL_ID_SPI1, &pmc->pcer);

	if (cs_mask & (1 << 0))
		at91_set_b_periph(AT91_PIO_PORTA, 8, 0);
	if (cs_mask & (1 << 1))
		at91_set_b_periph(AT91_PIO_PORTA, 0, 0);
	if (cs_mask & (1 << 2))
		at91_set_b_periph(AT91_PIO_PORTA, 31, 0);
	if (cs_mask & (1 << 3))
		at91_set_b_periph(AT91_PIO_PORTA, 30, 0);
	if (cs_mask & (1 << 4))
		at91_set_pio_output(AT91_PIO_PORTA, 8, 0);
	if (cs_mask & (1 << 5))
		at91_set_pio_output(AT91_PIO_PORTA, 0, 0);
	if (cs_mask & (1 << 6))
		at91_set_pio_output(AT91_PIO_PORTA, 31, 0);
	if (cs_mask & (1 << 7))
		at91_set_pio_output(AT91_PIO_PORTA, 30, 0);
}
#endif

#if defined(CONFIG_USB_OHCI_NEW) || defined(CONFIG_USB_EHCI)
void at91_uhp_hw_init(void)
{
	/* Enable VBus on UHP ports */
	at91_set_pio_output(AT91_PIO_PORTD, 18, 0); /* port A */
	at91_set_pio_output(AT91_PIO_PORTD, 19, 0); /* port B */
#if defined(CONFIG_USB_OHCI_NEW)
	/* port C is OHCI only */
	at91_set_pio_output(AT91_PIO_PORTD, 20, 0); /* port C */
#endif
}
#endif

#ifdef CONFIG_MACB
void at91_macb_hw_init(void)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	if (has_emac0()) {
		/* Enable EMAC0 clock */
		writel(1 << ATMEL_ID_EMAC0, &pmc->pcer);
		/* EMAC0 pins setup */
		at91_set_a_periph(AT91_PIO_PORTB, 4, 0);	/* ETXCK */
		at91_set_a_periph(AT91_PIO_PORTB, 3, 0);	/* ERXDV */
		at91_set_a_periph(AT91_PIO_PORTB, 0, 0);	/* ERX0 */
		at91_set_a_periph(AT91_PIO_PORTB, 1, 0);	/* ERX1 */
		at91_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* ERXER */
		at91_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* ETXEN */
		at91_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* ETX0 */
		at91_set_a_periph(AT91_PIO_PORTB, 10, 0);	/* ETX1 */
		at91_set_a_periph(AT91_PIO_PORTB, 5, 0);	/* EMDIO */
		at91_set_a_periph(AT91_PIO_PORTB, 6, 0);	/* EMDC */
	}

	if (has_emac1()) {
		/* Enable EMAC1 clock */
		writel(1 << ATMEL_ID_EMAC1, &pmc->pcer);
		/* EMAC1 pins setup */
		at91_set_b_periph(AT91_PIO_PORTC, 29, 0);	/* ETXCK */
		at91_set_b_periph(AT91_PIO_PORTC, 28, 0);	/* ECRSDV */
		at91_set_b_periph(AT91_PIO_PORTC, 20, 0);	/* ERXO */
		at91_set_b_periph(AT91_PIO_PORTC, 21, 0);	/* ERX1 */
		at91_set_b_periph(AT91_PIO_PORTC, 16, 0);	/* ERXER */
		at91_set_b_periph(AT91_PIO_PORTC, 27, 0);	/* ETXEN */
		at91_set_b_periph(AT91_PIO_PORTC, 18, 0);	/* ETX0 */
		at91_set_b_periph(AT91_PIO_PORTC, 19, 0);	/* ETX1 */
		at91_set_b_periph(AT91_PIO_PORTC, 31, 0);	/* EMDIO */
		at91_set_b_periph(AT91_PIO_PORTC, 30, 0);	/* EMDC */
	}

#ifndef CONFIG_RMII
	/* Only emac0 support MII */
	if (has_emac0()) {
		at91_set_a_periph(AT91_PIO_PORTB, 16, 0);	/* ECRS */
		at91_set_a_periph(AT91_PIO_PORTB, 17, 0);	/* ECOL */
		at91_set_a_periph(AT91_PIO_PORTB, 13, 0);	/* ERX2 */
		at91_set_a_periph(AT91_PIO_PORTB, 14, 0);	/* ERX3 */
		at91_set_a_periph(AT91_PIO_PORTB, 15, 0);	/* ERXCK */
		at91_set_a_periph(AT91_PIO_PORTB, 11, 0);	/* ETX2 */
		at91_set_a_periph(AT91_PIO_PORTB, 12, 0);	/* ETX3 */
		at91_set_a_periph(AT91_PIO_PORTB, 8, 0);	/* ETXER */
	}
#endif
}
#endif
