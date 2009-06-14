/*
 * (C) Copyright 2008
 *
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology  http://qtec.com
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Configuration file for the Virtex4FX12 Minimodul by Avnet/Memec,
 * see http://www.em.avnet.com
 */

#ifndef __CONFIG_FX12_H
#define __CONFIG_FX12_H

#include "../board/avnet/fx12mm/xparameters.h"

/* cmd config */
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#undef CONFIG_CMD_NET

/* sdram */
#define CONFIG_SYS_SDRAM_SIZE_MB       64

/* environment */
#define CONFIG_ENV_IS_IN_FLASH  1
#define CONFIG_ENV_SIZE         0x10000
#define CONFIG_ENV_SECT_SIZE    0x10000
#define CONFIG_SYS_ENV_OFFSET   0xA0000
#define CONFIG_ENV_ADDR         (CONFIG_SYS_FLASH_BASE+CONFIG_SYS_ENV_OFFSET)
#define CONFIG_ENV_OVERWRITE 	1

/*Misc*/
#define CONFIG_SYS_PROMPT	"FX12MM:/# " /* Monitor Command Prompt */
#define CONFIG_PREBOOT      	"echo U-Boot is up and running;"

/*Flash*/
#define CONFIG_SYS_FLASH_SIZE          (4*1024*1024)
#define CONFIG_SYS_MAX_FLASH_SECT      71
#define MTDIDS_DEFAULT		"nor0=fx12mm-flash"
#define MTDPARTS_DEFAULT	"mtdparts=fx12mm-flash:-(user)"

#include "configs/xilinx-ppc405.h"

#endif	/* __CONFIG_H */
