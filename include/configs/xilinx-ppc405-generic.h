/*
 *
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology http://qtec.com/
 *
 * (C) Copyright 2008
 * Georg Schardt <schardt@team-ctech.de>
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
#ifndef __CONFIG_GEN_H
#define __CONFIG_GEN_H

#include "../board/xilinx/ppc405-generic/xparameters.h"

/* sdram */
#define CONFIG_SYS_SDRAM_SIZE_MB	256

/* environment */
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_ENV_SECT_SIZE		0x10000
#define CONFIG_SYS_ENV_OFFSET		0x3F0000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE+CONFIG_SYS_ENV_OFFSET)
#define CONFIG_ENV_OVERWRITE		1

/*Misc*/
#define CONFIG_SYS_PROMPT	"xlx-ppc405:/# " /* Monitor Command Prompt */
#define CONFIG_PREBOOT		"echo U-Boot is up and runnining;"

/*Flash*/
#define CONFIG_SYS_FLASH_BASE			XPAR_FLASH_MEM0_BASEADDR
#define CONFIG_SYS_FLASH_SIZE		(32*1024*1024)
#define CONFIG_SYS_MAX_FLASH_SECT	71
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
#define MTDIDS_DEFAULT			"nor0=ppc405-flash"
#define MTDPARTS_DEFAULT		"mtdpartsa=ppc405-flash:-(user)"

#include <configs/xilinx-ppc405.h>
#endif			/* __CONFIG_H */
