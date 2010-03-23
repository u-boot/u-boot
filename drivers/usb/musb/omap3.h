/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This file is based on the file drivers/usb/musb/davinci.h
 *
 * This is the unique part of its copyright:
 *
 * --------------------------------------------------------------------
 *
 * Copyright (c) 2008 Texas Instruments
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 *
 * --------------------------------------------------------------------
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
#ifndef _MUSB_OMAP3_H_
#define _MUSB_OMAP3_H_

#include "musb_core.h"

/* Base address of MUSB registers */
#define MENTOR_USB0_BASE (OMAP34XX_CORE_L4_IO_BASE + 0xAB000)

/* Base address of OTG registers */
#define OMAP3_OTG_BASE (MENTOR_USB0_BASE + 0x400)

/* Timeout for USB module */
#define OMAP3_USB_TIMEOUT 0x3FFFFFF

int musb_platform_init(void);

#endif /* _MUSB_OMAP3_H */
