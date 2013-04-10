/*
 * Copyright (C) 2013 Boundary Devices Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef __ASM_ARCH_MX6_PINS_H__
#define __ASM_ARCH_MX6_PINS_H__

#ifdef CONFIG_MX6Q
#include "mx6q_pins.h"
#else
#if defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
#include "mx6dl_pins.h"
#else
#if defined(CONFIG_MX6SL)
#include "mx6sl_pins.h"
#else
#error "Please select cpu"
#endif	/* CONFIG_MX6SL */
#endif	/* CONFIG_MX6DL or CONFIG_MX6S */
#endif	/* CONFIG_MX6Q */

#endif	/*__ASM_ARCH_MX6_PINS_H__ */
