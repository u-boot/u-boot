/*
 * Mentor USB OTG Core host controller driver.
 *
 * Copyright (c) 2008 Texas Instruments
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
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 */

#ifndef __MUSB_HCD_H__
#define __MUSB_HCD_H__

#include "musb_core.h"
#ifdef CONFIG_USB_KEYBOARD
#include <stdio_dev.h>
extern unsigned char new[];
#endif

#ifndef CONFIG_MUSB_TIMEOUT
# define CONFIG_MUSB_TIMEOUT 100000
#endif

/* This defines the endpoint number used for control transfers */
#define MUSB_CONTROL_EP 0

/* This defines the endpoint number used for bulk transfer */
#define MUSB_BULK_EP 1

/* This defines the endpoint number used for interrupt transfer */
#define MUSB_INTR_EP 2

/* Determine the operating speed of MUSB core */
#define musb_ishighspeed() \
	((readb(&musbr->power) & MUSB_POWER_HSMODE) \
		>> MUSB_POWER_HSMODE_SHIFT)

/* extern functions */
extern int musb_platform_init(void);
extern void musb_platform_deinit(void);

#endif	/* __MUSB_HCD_H__ */
