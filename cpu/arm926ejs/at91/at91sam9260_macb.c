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

void at91_macb_hw_init(void)
{
	at91_set_A_periph(AT91_PIN_PA19, 0);	/* ETXCK_EREFCK */
	at91_set_A_periph(AT91_PIN_PA17, 0);	/* ERXDV */
	at91_set_A_periph(AT91_PIN_PA14, 0);	/* ERX0 */
	at91_set_A_periph(AT91_PIN_PA15, 0);	/* ERX1 */
	at91_set_A_periph(AT91_PIN_PA18, 0);	/* ERXER */
	at91_set_A_periph(AT91_PIN_PA16, 0);	/* ETXEN */
	at91_set_A_periph(AT91_PIN_PA12, 0);	/* ETX0 */
	at91_set_A_periph(AT91_PIN_PA13, 0);	/* ETX1 */
	at91_set_A_periph(AT91_PIN_PA21, 0);	/* EMDIO */
	at91_set_A_periph(AT91_PIN_PA20, 0);	/* EMDC */

#ifndef CONFIG_RMII
	at91_set_B_periph(AT91_PIN_PA28, 0);	/* ECRS */
	at91_set_B_periph(AT91_PIN_PA29, 0);	/* ECOL */
	at91_set_B_periph(AT91_PIN_PA25, 0);	/* ERX2 */
	at91_set_B_periph(AT91_PIN_PA26, 0);	/* ERX3 */
	at91_set_B_periph(AT91_PIN_PA27, 0);	/* ERXCK */
#if defined(CONFIG_AT91SAM9260EK) || defined(CONFIG_AFEB9260)
	/*
	 * use PA10, PA11 for ETX2, ETX3.
	 * PA23 and PA24 are for TWI EEPROM
	 */
	at91_set_B_periph(AT91_PIN_PA10, 0);	/* ETX2 */
	at91_set_B_periph(AT91_PIN_PA11, 0);	/* ETX3 */
#else
	at91_set_B_periph(AT91_PIN_PA23, 0);	/* ETX2 */
	at91_set_B_periph(AT91_PIN_PA24, 0);	/* ETX3 */
#endif
	at91_set_B_periph(AT91_PIN_PA22, 0);	/* ETXER */
#endif
}
