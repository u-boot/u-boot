/*
 * SGS M48-T59Y TOD/NVRAM Driver
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 1999, by Curt McDowell, 08-06-99, Broadcom Corp.
 *
 * (C) Copyright 2001, James Dougherty, 07/18/01, Broadcom Corp.
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

#ifndef __M48_T59_Y_H
#define __M48_T59_Y_H

/*
 * M48 T59Y -Timekeeping Battery backed SRAM.
 */

int m48_tod_init(void);

int m48_tod_set(int year,
		int month,
		int day,
		int hour,
		int minute,
		int second);

int m48_tod_get(int *year,
		int *month,
		int *day,
		int *hour,
		int *minute,
		int *second);

int m48_tod_get_second(void);

void m48_watchdog_arm(int usec);

#endif /*!__M48_T59_Y_H */
