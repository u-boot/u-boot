/*
 * (C) Copyright 2009
 * Vipin Kumar, STMicroelectronics, <vipin.kumar@st.com>
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SPEAR600

#if defined(CONFIG_usbtty)
#define CONFIG_SPEAR_USBTTY
#endif

#if defined(CONFIG_nand)
#define CONFIG_ENV_IS_IN_NAND
#else
#define CONFIG_ENV_IS_IN_FLASH
#endif

#include <configs/spear-common.h>

/* Serial Configuration (PL011) */
#define CONFIG_SYS_SERIAL0			0xD0000000
#define CONFIG_SYS_SERIAL1			0xD0080000
#define CONFIG_PL01x_PORTS			{ (void *)CONFIG_SYS_SERIAL0, \
						(void *)CONFIG_SYS_SERIAL1 }

/* NAND flash configuration */
#define CONFIG_SYS_FSMC_NAND_SP
#define CONFIG_SYS_FSMC_NAND_8BIT
#define CONFIG_SYS_NAND_BASE			0xD2000000

/* Environment Settings */
#define CONFIG_EXTRA_ENV_SETTINGS              CONFIG_EXTRA_ENV_USBTTY

#endif  /* __CONFIG_H */
