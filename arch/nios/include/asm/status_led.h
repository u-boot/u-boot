/*
 * (C) Copyright 2003, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
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
 *
 * asm-nios/status_led.h
 *
 * NIOS PIO based status led support functions
 */

#ifndef __ASM_STATUS_LED_H__
#define __ASM_STATUS_LED_H__

#include <nios-io.h>

/* led_id_t is unsigned int mask */
typedef unsigned int led_id_t;

#ifdef	STATUS_LED_WRONLY	/* emulate read access */
static led_id_t __led_portval = 0;
#endif

static inline void __led_init (led_id_t mask, int state)
{
	nios_pio_t *piop = (nios_pio_t*)STATUS_LED_BASE;

#ifdef	STATUS_LED_WRONLY	/* emulate read access */

#if (STATUS_LED_ACTIVE == 0)
	if (state == STATUS_LED_ON)
		__led_portval &= ~mask;
	else
		__led_portval |= mask;
#else
	if (state == STATUS_LED_ON)
		__led_portval |= mask;
	else
		__led_portval &= ~mask;
#endif

	piop->data = __led_portval;

#else	/* !STATUS_LED_WRONLY */

#if (STATUS_LED_ACTIVE == 0)
	if (state == STATUS_LED_ON)
		piop->data &= ~mask;
	else
		piop->data |= mask;
#else
	if (state == STATUS_LED_ON)
		piop->data |= mask;
	else
		piop->data &= ~mask;
#endif

	piop->direction |= mask;

#endif	/* STATUS_LED_WRONLY */
}

static inline void __led_toggle (led_id_t mask)
{
	nios_pio_t *piop = (nios_pio_t*)STATUS_LED_BASE;

#ifdef	STATUS_LED_WRONLY	/* emulate read access */

	__led_portval ^= mask;
	piop->data = __led_portval;

#else	/* !STATUS_LED_WRONLY */

	piop->data ^= mask;

#endif	/* STATUS_LED_WRONLY */
}

static inline void __led_set (led_id_t mask, int state)
{
	nios_pio_t *piop = (nios_pio_t*)STATUS_LED_BASE;

#ifdef	STATUS_LED_WRONLY	/* emulate read access */

#if (STATUS_LED_ACTIVE == 0)
	if (state == STATUS_LED_ON)
		__led_portval &= ~mask;
	else
		__led_portval |= mask;
#else
	if (state == STATUS_LED_ON)
		__led_portval |= mask;
	else
		__led_portval &= ~mask;
#endif

	piop->data = __led_portval;

#else	/* !STATUS_LED_WRONLY */

#if (STATUS_LED_ACTIVE == 0)
	if (state == STATUS_LED_ON)
		piop->data &= ~mask;
	else
		piop->data |= mask;
#else
	if (state == STATUS_LED_ON)
		piop->data |= mask;
	else
		piop->data &= ~mask;
#endif

#endif	/* STATUS_LED_WRONLY */
}

#endif	/* __ASM_STATUS_LED_H__ */
