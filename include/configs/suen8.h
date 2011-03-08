/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2010-2011
 * Holger Brunck, Keymile GmbH Hannover, holger.brunck@keymile.com.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

/* for linking errors see
 * http://lists.denx.de/pipermail/u-boot/2009-July/057350.html */

#ifndef _CONFIG_SUEN8_H
#define _CONFIG_SUEN8_H

/* include common defines/options for all arm based Keymile boards */
#include "km_arm.h"

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nKeymile SUEN8"

#define CONFIG_HOSTNAME			suen8

#define KM_IVM_BUS	"pca9544a:70:9" /* I2C2 (Mux-Port 1)*/
#define KM_ENV_BUS	"pca9544a:70:d" /* I2C2 (Mux-Port 5)*/

/*
 * Default environment variables
 */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_KM_DEF_ENV						\
	"newenv=setenv addr 0x100000 && "				\
		"i2c dev 1; mw.b ${addr} 0 4 && "			\
		"eeprom write " xstr(CONFIG_SYS_DEF_EEPROM_ADDR)	\
		" ${addr} " xstr(CONFIG_ENV_OFFSET) " 4 && "		\
		"eeprom write " xstr(CONFIG_SYS_DEF_EEPROM_ADDR)	\
		" ${addr} " xstr(CONFIG_ENV_OFFSET_REDUND) " 4\0"	\
	"rootpath=/opt/eldk/arm\0"					\
	"EEprom_ivm=" KM_IVM_BUS "\0"					\
	""

#endif /* _CONFIG_SUEN8_H */
