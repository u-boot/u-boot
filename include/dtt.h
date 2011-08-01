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

#if defined(CONFIG_DTT_ADM1021)	|| \
    defined(CONFIG_DTT_ADT7460)	|| \
    defined(CONFIG_DTT_DS1621)	|| \
    defined(CONFIG_DTT_DS1775)	|| \
    defined(CONFIG_DTT_LM63)	|| \
    defined(CONFIG_DTT_LM73)	|| \
    defined(CONFIG_DTT_LM75)	|| \
    defined(CONFIG_DTT_LM81)

#define CONFIG_DTT				/* We have a DTT */

#ifndef CONFIG_DTT_ADM1021
#define DTT_COMMERCIAL_MAX_TEMP	70		/* 0 - +70 C */
#define DTT_INDUSTRIAL_MAX_TEMP	85		/* -40 - +85 C */
#define DTT_AUTOMOTIVE_MAX_TEMP	105		/* -40 - +105 C */

#ifndef CONFIG_SYS_DTT_MAX_TEMP
#define CONFIG_SYS_DTT_MAX_TEMP DTT_COMMERCIAL_MAX_TEMP
#endif

#ifndef CONFIG_SYS_DTT_HYSTERESIS
#define CONFIG_SYS_DTT_HYSTERESIS	5		/* 5 C */
#endif
#endif /* CONFIG_DTT_ADM1021 */

extern int dtt_init_one(int);
extern int dtt_read(int sensor, int reg);
extern int dtt_write(int sensor, int reg, int val);
extern int dtt_get_temp(int sensor);
#endif

#endif /* _DTT_H_ */
