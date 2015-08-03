/*
 * (C) Copyright 2008
 *  Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 *  This work has been supported by: QTechnology  http://qtec.com/
 * SPDX-License-Identifier:	GPL-2.0+
*/

#ifndef __CONFIG_H
#define __CONFIG_H

/*CPU*/
#define CONFIG_440			1
#define CONFIG_XILINX_PPC440_GENERIC	1
#include "../board/xilinx/ppc440-generic/xparameters.h"

/*Mem Map*/
#define CONFIG_SYS_SDRAM_SIZE_MB	256

/*Env*/
#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_SIZE		0x20000
#define	CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OFFSET		0x340000
#define CONFIG_ENV_ADDR		(XPAR_FLASH_MEM0_BASEADDR+CONFIG_ENV_OFFSET)

/*Misc*/
#define CONFIG_PREBOOT		"echo U-Boot is up and runnining;"

/*Flash*/
#define	CONFIG_SYS_FLASH_SIZE		(32*1024*1024)
#define	CONFIG_SYS_MAX_FLASH_SECT	259
#define MTDIDS_DEFAULT		"nor0=ml507-flash"
#define MTDPARTS_DEFAULT	"mtdparts=ml507-flash:-(user)"

/*Generic Configs*/
#include <configs/xilinx-ppc440.h>

#endif						/* __CONFIG_H */
