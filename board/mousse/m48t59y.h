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
 * SPDX-License-Identifier:	GPL-2.0+
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
