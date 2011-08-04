/*
 * Copyright (C) 2011
 * Jason Cooper <u-boot@lakedaemon.net>
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

/*
 * Date & Time support for Marvell Integrated RTC
 */

#ifndef _MVRTC_H_
#define _MVRTC_H_

#include <asm/arch/kirkwood.h>
#include <compiler.h>

/* RTC registers */
struct mvrtc_registers {
	u32 time;
	u32 date;
};

/* time register */
#define MVRTC_SEC_SFT		0
#define MVRTC_SEC_MSK		0x7f
#define MVRTC_MIN_SFT		8
#define MVRTC_MIN_MSK		0x7f
#define MVRTC_HOUR_SFT		16
#define MVRTC_HOUR_MSK		0x3f
#define MVRTC_DAY_SFT		24
#define MVRTC_DAY_MSK		0x7

/*
 * Hour format bit
 *   1 = 12 hour clock
 *   0 = 24 hour clock
 */
#define MVRTC_HRFMT_MSK		0x00400000

/* date register */
#define MVRTC_DATE_SFT		0
#define MVRTC_DATE_MSK		0x3f
#define MVRTC_MON_SFT		8
#define MVRTC_MON_MSK		0x1f
#define MVRTC_YEAR_SFT		16
#define MVRTC_YEAR_MSK		0xff

#endif
