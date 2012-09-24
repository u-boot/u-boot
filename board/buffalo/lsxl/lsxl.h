/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __LSXL_H
#define __LSXL_H

#define GPIO_HDD_POWER		10
#define GPIO_USB_VBUS		11
#define GPIO_FAN_HIGH		18
#define GPIO_FAN_LOW		19
#define GPIO_FUNC_LED		36
#define GPIO_ALARM_LED		37
#define GPIO_INFO_LED		38
#define GPIO_POWER_LED		39
#define GPIO_FAN_LOCK		40
#define GPIO_FUNC_BUTTON	41
#define GPIO_POWER_SWITCH	42
#define GPIO_POWER_AUTO_SWITCH	43
#define GPIO_FUNC_RED_LED	48

#define _BIT(x) (1<<(x))

#define LSXL_OE_LOW (~(_BIT(GPIO_HDD_POWER)		    \
			| _BIT(GPIO_USB_VBUS)		    \
			| _BIT(GPIO_FAN_HIGH)		    \
			| _BIT(GPIO_FAN_LOW)))

#define LSXL_OE_HIGH (~(_BIT(GPIO_FUNC_LED - 32)	    \
			| _BIT(GPIO_ALARM_LED - 32)	    \
			| _BIT(GPIO_INFO_LED - 32)	    \
			| _BIT(GPIO_POWER_LED - 32)	    \
			| _BIT(GPIO_FUNC_RED_LED - 32)))

#define LSXL_OE_VAL_LOW (_BIT(GPIO_HDD_POWER)		    \
			| _BIT(GPIO_USB_VBUS))

#define LSXL_OE_VAL_HIGH (_BIT(GPIO_FUNC_LED - 32)	    \
			| _BIT(GPIO_ALARM_LED - 32)	    \
			| _BIT(GPIO_INFO_LED - 32)	    \
			| _BIT(GPIO_POWER_LED - 32)	    \
			| _BIT(GPIO_FUNC_RED_LED - 32))

#define LSXL_POL_VAL_LOW (_BIT(GPIO_FAN_HIGH)		    \
			| _BIT(GPIO_FAN_LOW))

#define LSXL_POL_VAL_HIGH (_BIT(GPIO_FUNC_LED - 32)	    \
			| _BIT(GPIO_ALARM_LED - 32)	    \
			| _BIT(GPIO_INFO_LED - 32)	    \
			| _BIT(GPIO_POWER_LED - 32)	    \
			| _BIT(GPIO_FUNC_BUTTON - 32)	    \
			| _BIT(GPIO_POWER_SWITCH - 32)	    \
			| _BIT(GPIO_POWER_AUTO_SWITCH - 32) \
			| _BIT(GPIO_FUNC_RED_LED - 32))

#endif /* __LSXL_H */
