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

#if defined(CONFIG_DTT_LM75) || \
    defined(CONFIG_DTT_DS1621) || \
    defined(CONFIG_DTT_DS1775) || \
    defined(CONFIG_DTT_LM81) || \
    defined(CONFIG_DTT_ADM1021) || \
    defined(CONFIG_DTT_LM73)

#define CONFIG_DTT				/* We have a DTT */

#ifndef CONFIG_DTT_ADM1021
#define DTT_COMMERCIAL_MAX_TEMP	70		/* 0 - +70 C */
#define DTT_INDUSTRIAL_MAX_TEMP	85		/* -40 - +85 C */
#define DTT_AUTOMOTIVE_MAX_TEMP	105		/* -40 - +105 C */
#ifndef CFG_DTT_MAX_TEMP
#define CFG_DTT_MAX_TEMP DTT_COMMERCIAL_MAX_TEMP
#endif
#ifndef CFG_DTT_HYSTERESIS
#define CFG_DTT_HYSTERESIS	5		/* 5 C */
#endif
#endif /* CONFIG_DTT_ADM1021 */

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

#if defined(CONFIG_DTT_LM81)
#define DTT_READ_TEMP		0x27
#define DTT_CONFIG_TEMP		0x4b
#define DTT_TEMP_MAX		0x39
#define DTT_TEMP_HYST		0x3a
#define DTT_CONFIG		0x40
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

#if defined(CONFIG_DTT_DS1775)
#define DTT_READ_TEMP		0x0
#define DTT_CONFIG		0x1
#define DTT_TEMP_HYST		0x2
#define DTT_TEMP_OS		0x3
#endif

#if defined(CONFIG_DTT_ADM1021)
#define DTT_READ_LOC_VALUE	0x00
#define DTT_READ_REM_VALUE	0x01
#define DTT_READ_STATUS		0x02
#define DTT_READ_CONFIG		0x03
#define DTT_READ_CONVRATE	0x04
#define DTT_READ_LOC_HIGHLIM	0x05
#define DTT_READ_LOC_LOWLIM	0x06
#define DTT_READ_REM_HIGHLIM	0x07
#define DTT_READ_REM_LOWLIM	0x08
#define DTT_READ_DEVID		0xfe

#define DTT_WRITE_CONFIG	0x09
#define DTT_WRITE_CONVRATE	0x0a
#define DTT_WRITE_LOC_HIGHLIM	0x0b
#define DTT_WRITE_LOC_LOWLIM	0x0c
#define DTT_WRITE_REM_HIGHLIM	0x0d
#define DTT_WRITE_REM_LOWLIM	0x0e
#define DTT_WRITE_ONESHOT	0x0f

#define DTT_STATUS_BUSY		0x80	/* 1=ADC Converting */
#define DTT_STATUS_LHIGH	0x40	/* 1=Local High Temp Limit Tripped */
#define DTT_STATUS_LLOW		0x20	/* 1=Local Low Temp Limit Tripped */
#define DTT_STATUS_RHIGH	0x10	/* 1=Remote High Temp Limit Tripped */
#define DTT_STATUS_RLOW		0x08	/* 1=Remote Low Temp Limit Tripped */
#define DTT_STATUS_OPEN		0x04	/* 1=Remote Sensor Open-Circuit */

#define DTT_CONFIG_ALERT_MASKED	0x80	/* 0=ALERT Enabled, 1=ALERT Masked */
#define DTT_CONFIG_STANDBY	0x40	/* 0=Run, 1=Standby */

#define DTT_ADM1021_DEVID	0x41
#endif

#if defined(CONFIG_DTT_LM73)
#define DTT_READ_TEMP		0x0
#define DTT_CONFIG		0x1
#define DTT_TEMP_HIGH		0x2
#define DTT_TEMP_LOW		0x3
#define DTT_CONTROL		0x4
#define DTT_ID			0x7
#endif

#endif /* _DTT_H_ */
