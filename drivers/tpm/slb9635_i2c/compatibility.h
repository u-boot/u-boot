/*
 * Copyright (C) 2011 Infineon Technologies
 *
 * Authors:
 * Peter Huewe <huewe.external@infineon.com>
 *
 * Version: 2.1.1
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
 */

#ifndef _COMPATIBILITY_H_
#define _COMPATIBILITY_H_

/* all includes from U-Boot */
#include <linux/types.h>
#include <linux/unaligned/be_byteshift.h>
#include <asm-generic/errno.h>
#include <compiler.h>
#include <common.h>

/* extended error numbers from linux (see errno.h) */
#define	ECANCELED	125	/* Operation Canceled */

#define msleep(t) udelay((t)*1000)

/* Timer frequency. Corresponds to msec timer resolution*/
#define HZ             1000

#define dev_dbg(dev, format, arg...) debug(format, ##arg)
#define dev_err(dev, format, arg...) printf(format, ##arg)
#define dev_info(dev, format, arg...) debug(format, ##arg)
#define dbg_printf debug

#endif
