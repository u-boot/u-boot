/***********************************************************************
 *
 * Copyright (C) 2004 by FS Forth-Systeme GmbH.
 * All rights reserved.
 *
 * $Id: led.c,v 1.1 2004/02/16 10:37:20 mpietrek Exp $
 * @Author: Markus Pietrek
 * @Descr: Defines helper functions for toggeling LEDs
 * @Usage:
 * @References: [1]
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
 ***********************************************************************/

#ifdef CONFIG_STATUS_LED

#include <ns9750_bbus.h>

static inline void __led_init( led_id_t mask, int state )
{
	XXXX;
}

static inline void __led_toggle( led_id_t mask )
{
}

static inline void __led_set( led_id_t mask, int state )
{
}

#endif /* CONFIG_STATUS_LED */
