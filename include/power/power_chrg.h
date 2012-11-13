/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __POWER_CHARGER_H_
#define __POWER_CHARGER_H_

/* Type of available chargers */
enum {
	CHARGER_NO = 0,
	CHARGER_TA,
	CHARGER_USB,
	CHARGER_TA_500,
	CHARGER_UNKNOWN,
};

enum {
	UNKNOWN,
	EXT_SOURCE,
	CHARGE,
	NORMAL,
};

#endif /* __POWER_CHARGER_H_ */
