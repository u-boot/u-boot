/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/*
 * for linking errors see
 * http://lists.denx.de/pipermail/u-boot/2009-July/057350.html
 */

#ifndef _CONFIG_SUEN3_H
#define _CONFIG_SUEN3_H

/* include common defines/options for all arm based Keymile boards */
#include "km_arm.h"

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nKeymile SUEN3"

#define CONFIG_HOSTNAME			suen3

/*
 *  Environment variables configurations
 */
#define CONFIG_ENV_IS_IN_EEPROM		/* use EEPROM for environment vars */
#define CONFIG_SYS_DEF_EEPROM_ADDR	0x50
#define CONFIG_ENV_EEPROM_IS_ON_I2C	1
#define CONFIG_SYS_EEPROM_WREN		1
#define CONFIG_ENV_OFFSET		0x0 /* no bracets! */
#undef	CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE			(0x2000 - CONFIG_ENV_OFFSET)
#define CONFIG_I2C_ENV_EEPROM_BUS	"pca9547:70:d\0"

/* offset redund: (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE) */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND	0x2000 /* no bracets! */
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)

#define CONFIG_CMD_SF

#define CONFIG_SPI_FLASH
#define CONFIG_HARD_SPI
#define CONFIG_KIRKWOOD_SPI
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_SPI_MAX_HZ		50000000	/* 50Mhz */

#define FLASH_GPIO_PIN			0x00010000

#define MTDIDS_DEFAULT		"nand0=orion_nand"
/* test-only: partitioning needs some tuning, this is just for tests */
#define MTDPARTS_DEFAULT	"mtdparts="				\
	"orion_nand:"							\
		"-(" CONFIG_KM_UBI_PARTITION_NAME ")"

#define	CONFIG_KM_DEF_ENV_UPDATE					\
	"update="							\
		"spi on;sf probe 0;sf erase 0 50000;"			\
		"sf write ${u-boot_addr_r} 0 ${filesize};"		\
		"spi off\0"

/*
 * Default environment variables
 */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_KM_DEF_ENV						\
	"memsize=0x8000000\0"						\
	"newenv=setenv addr 0x100000 && "				\
		"i2c dev 1; mw.b ${addr} 0 4 && "			\
		"eeprom write " xstr(CONFIG_SYS_DEF_EEPROM_ADDR)	\
		" ${addr} " xstr(CONFIG_ENV_OFFSET) " 4 && "		\
		"eeprom write " xstr(CONFIG_SYS_DEF_EEPROM_ADDR)	\
		" ${addr} " xstr(CONFIG_ENV_OFFSET_REDUND) " 4\0"	\
	"rootpath=/opt/eldk/arm\0"					\
	"EEprom_ivm=pca9544a:70:9\0"					\
	""

#endif /* _CONFIG_SUEN3_H */
