/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated.
 * Lokesh Vutla	  <lokeshvutla@ti.com>
 *
 * Configuration settings for the TI DRA7XX board.
 * See omap5_common.h for omap5 common settings.
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

#ifndef __CONFIG_DRA7XX_EVM_H
#define __CONFIG_DRA7XX_EVM_H

/* High Level Configuration Options */
#define CONFIG_DRA7XX		/* in a TI DRA7XX core */
#define CONFIG_ENV_IS_NOWHERE		/* For now. */

#include <configs/omap5_common.h>

#define CONFIG_SYS_PROMPT		"DRA752 EVM # "

#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		UART1_BASE
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_OMAP_ABE_SYSCK

#define CONSOLEDEV		"ttyO0"

#endif /* __CONFIG_DRA7XX_EVM_H */
