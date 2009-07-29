/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix at windriver.com>
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
 *
 * twl4030_led_init is from cpu/omap3/common.c, power_init_r
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05 at gmail.com>
 *	Shashi Ranjan <shashiranjanmca05 at gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2 at ti.com>
 *	Syed Mohammed Khasim <khasim at ti.com>
 *
 */

#include <twl4030.h>

#define LEDAON			(0x1 << 0)
#define LEDBON			(0x1 << 1)
#define LEDAPWM			(0x1 << 4)
#define LEDBPWM			(0x1 << 5)

void twl4030_led_init(void)
{
	unsigned char byte;

	/* enable LED */
	byte = LEDBPWM | LEDAPWM | LEDBON | LEDAON;

	twl4030_i2c_write_u8(TWL4030_CHIP_LED, byte,
			     TWL4030_LED_LEDEN);

}
