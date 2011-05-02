/*
 * (C) Copyright 2007-2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8247
#define CONFIG_MGCOGE
#define CONFIG_HOSTNAME		mgcoge

#define	CONFIG_SYS_TEXT_BASE	0xFE000000

/* include common defines/options for all Keymile boards */
#include "keymile-common.h"
#include "km-powerpc.h"

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFE000000
#define CONFIG_SYS_FLASH_SIZE		32
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_MAX_FLASH_BANKS	3
/* max num of sects on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	512

#define CONFIG_SYS_FLASH_BASE_1	0x50000000
#define CONFIG_SYS_FLASH_SIZE_1	32
#define CONFIG_SYS_FLASH_BASE_2	0x52000000
#define CONFIG_SYS_FLASH_SIZE_2	32

#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE, \
					CONFIG_SYS_FLASH_BASE_1, \
					CONFIG_SYS_FLASH_BASE_2 }
#define MTDIDS_DEFAULT		"nor3=app"

/* include further common stuff for all keymile 82xx boards */
#include "km82xx-common.h"

#endif /* __CONFIG_H */
