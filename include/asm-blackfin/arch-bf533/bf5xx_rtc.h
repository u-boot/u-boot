/*
 * U-boot - bf533_rtc.h
 *
 * Copyright (c) 2005 blackfin.uclinux.org
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

#ifndef _BF533_RTC_H_
#define _BF533_RTC_H_

void rtc_init(void);
void wait_for_complete(void);
void rtc_reset(void);

#define MIN_TO_SECS(_x_)	(60 * _x_)
#define HRS_TO_SECS(_x_)	(60 * 60 * _x_)
#define DAYS_TO_SECS(_x_)	(24 * 60 * 60 * _x_)

#define NUM_SECS_IN_DAY		(24 * 3600)
#define NUM_SECS_IN_HOUR	(3600)
#define NUM_SECS_IN_MIN		(60)

/* Shift values for RTC_STAT register */
#define DAY_BITS_OFF		17
#define HOUR_BITS_OFF		12
#define MIN_BITS_OFF		6
#define SEC_BITS_OFF		0

#endif
