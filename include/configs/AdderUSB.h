/*
 * Copyright (C) 2006 CodeHermit.
 * Bryan O'Donoghue <bodonoghue@codehermit.ie>
 *
 * Provides support for USB console on the Analogue & Micro Adder87x
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ADDERUSB__
#define __ADDERUSB__

/* Include the board port */
#include "Adder.h"

#define CONFIG_USB_DEVICE		/* Include UDC driver */
#define CONFIG_USB_TTY			/* Bind the TTY driver to UDC */
#define CFG_USB_EXTC_CLK 0x02		/* Oscillator on EXTC_CLK 2 */
#define CFG_USB_BRG_CLK	0x04		/* or use Baud rate generator 0x04 */
#define CFG_CONSOLE_IS_IN_ENV		/* Console is in env */

/* If you have a USB-IF assigned VendorID then you may wish to define
 * your own vendor specific values either in BoardName.h or directly in
 * usbd_vendor_info.h
 */

/*
#define CONFIG_USBD_MANUFACTURER	"CodeHermit.ie"
#define CONFIG_USBD_PRODUCT_NAME	"Das U-Boot"
#define CONFIG_USBD_VENDORID		0xFFFF
#define CONFIG_USBD_PRODUCTID_GSERIAL	0xFFFF
#define CONFIG_USBD_PRODUCTID_CDCACM	0xFFFE
*/

#endif /* __ADDERUSB_H__ */
