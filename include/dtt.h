/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Digital Thermometers and Thermostats.
 */
#ifndef _DTT_H_
#define _DTT_H_

#if defined(CONFIG_DTT_LM75) || defined(CONFIG_DTT_DS1621)
#define CONFIG_DTT				/* We have a DTT */

#define DTT_COMMERCIAL_MAX_TEMP	70		/* 0 - +70 C */
#define DTT_INDUSTRIAL_MAX_TEMP	85		/* -40 - +85 C */
#define DTT_AUTOMOTIVE_MAX_TEMP	105		/* -40 - +105 C */
#ifndef CFG_DTT_MAX_TEMP
#define CFG_DTT_MAX_TEMP DTT_COMMERCIAL_MAX_TEMP
#endif
#ifndef CFG_DTT_HYSTERESIS
#define CFG_DTT_HYSTERESIS	5		/* 5 C */
#endif

extern int dtt_init (void);
extern int dtt_read(int sensor, int reg);
extern int dtt_write(int sensor, int reg, int val);
extern int dtt_get_temp(int sensor);
#endif

#if defined(CONFIG_DTT_LM75)
#define DTT_READ_TEMP		0x0
#define DTT_CONFIG		0x1
#define DTT_TEMP_HYST		0x2
#define DTT_TEMP_SET		0x3
#endif

#if defined(CONFIG_DTT_DS1621)
#define DTT_READ_TEMP		0xAA
#define DTT_READ_COUNTER	0xA8
#define DTT_READ_SLOPE		0xA9
#define DTT_WRITE_START_CONV	0xEE
#define DTT_WRITE_STOP_CONV	0x22
#define DTT_TEMP_HIGH		0xA1
#define DTT_TEMP_LOW		0xA2
#define DTT_CONFIG		0xAC
#endif

#endif /* _DTT_H_ */
