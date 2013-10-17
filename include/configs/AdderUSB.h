/*
 * Copyright (C) 2006 CodeHermit.
 * Bryan O'Donoghue <bodonoghue@codehermit.ie>
 *
 * Provides support for USB console on the Analogue & Micro Adder87x
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ADDERUSB__
#define __ADDERUSB__

/* Include the board port */
#include "Adder.h"

#define CONFIG_USB_DEVICE		/* Include UDC driver */
#define CONFIG_USB_TTY			/* Bind the TTY driver to UDC */
#define CONFIG_SYS_USB_EXTC_CLK 0x02		/* Oscillator on EXTC_CLK 2 */
#define CONFIG_SYS_USB_BRG_CLK	0x04		/* or use Baud rate generator 0x04 */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV		/* Console is in env */

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
