/*
 * (C) Copyright 2007 Czech Technical University.
 *
 * Michal SIMEK <monstr@monstr.eu>
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

#include "../board/xilinx/xupv2p/xparameters.h"

#define	CONFIG_MICROBLAZE	1	/* MicroBlaze CPU */
#define	CONFIG_XUPV2P		1

/* uart */
#define	CONFIG_SERIAL_BASE	XILINX_UART_BASEADDR
#define	CONFIG_BAUDRATE		XILINX_UART_BAUDRATE
#define	CFG_BAUDRATE_TABLE	{ CONFIG_BAUDRATE }

/* ethernet */
#define CONFIG_EMAC	1
#define XPAR_EMAC_0_DEVICE_ID	XPAR_XEMAC_NUM_INSTANCES

/*
 * setting reset address
 *
 * TEXT_BASE is set to place, where the U-BOOT run in RAM, but
 * if you want to store U-BOOT in flash, set CFG_RESET_ADDRESS
 * to FLASH memory and after loading bitstream jump to FLASH.
 * U-BOOT auto-relocate to TEXT_BASE. After RESET command Microblaze
 * jump to CFG_RESET_ADDRESS where is the original U-BOOT code.
 */
#define	CFG_RESET_ADDRESS	0x36000000

/* gpio */
#define	CFG_GPIO_0		1
#define	CFG_GPIO_0_ADDR		XILINX_GPIO_BASEADDR

/* interrupt controller */
#define	CFG_INTC_0		1
#define	CFG_INTC_0_ADDR		XILINX_INTC_BASEADDR
#define	CFG_INTC_0_NUM		XILINX_INTC_NUM_INTR_INPUTS

/* timer */
#define	CFG_TIMER_0		1
#define	CFG_TIMER_0_ADDR	XILINX_TIMER_BASEADDR
#define	CFG_TIMER_0_IRQ		XILINX_TIMER_IRQ
#define	FREQUENCE		XILINX_CLOCK_FREQ
#define	CFG_TIMER_0_PRELOAD	( FREQUENCE/1000 )

/*
 * memory layout - Example
 * TEXT_BASE = 0x3600_0000;
 * CFG_SRAM_BASE = 0x3000_0000;
 * CFG_SRAM_SIZE = 0x1000_0000;
 *
 * CFG_GBL_DATA_OFFSET = 0x3000_0000 + 0x1000_0000 - 0x1000 = 0x3FFF_F000
 * CFG_MONITOR_BASE = 0x3FFF_F000 - 0x40000 = 0x3FFB_F000
 * CFG_MALLOC_BASE = 0x3FFB_F000 - 0x40000 = 0x3FF7_F000
 *
 * 0x3000_0000	CFG_SDRAM_BASE
 *					FREE
 * 0x3600_0000	TEXT_BASE
 *		U-BOOT code
 * 0x3602_0000
 *					FREE
 *
 *					STACK
 * 0x3FF7_F000	CFG_MALLOC_BASE
 *					MALLOC_AREA	256kB	Alloc
 * 0x3FFB_F000	CFG_MONITOR_BASE
 *					MONITOR_CODE	256kB	Env
 * 0x3FFF_F000	CFG_GBL_DATA_OFFSET
 * 					GLOBAL_DATA	4kB	bd, gd
 * 0x4000_0000	CFG_SDRAM_BASE + CFG_SDRAM_SIZE
 */

/* ddr sdram - main memory */
#define	CFG_SDRAM_BASE		XILINX_RAM_START
#define	CFG_SDRAM_SIZE		XILINX_RAM_SIZE
#define	CFG_MEMTEST_START	CFG_SDRAM_BASE
#define	CFG_MEMTEST_END		(CFG_SDRAM_BASE + 0x1000)

/* global pointer */
#define	CFG_GBL_DATA_SIZE	0x1000	/* size of global data */
#define	CFG_GBL_DATA_OFFSET     (CFG_SDRAM_BASE + CFG_SDRAM_SIZE - CFG_GBL_DATA_SIZE) /* start of global data */

/* monitor code */
#define	SIZE			0x40000
#define	CFG_MONITOR_LEN		SIZE
#define	CFG_MONITOR_BASE	(CFG_GBL_DATA_OFFSET - CFG_MONITOR_LEN)
#define	CFG_MONITOR_END		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define	CFG_MALLOC_LEN		SIZE
#define	CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)

/* stack */
#define	CFG_INIT_SP_OFFSET	CFG_MALLOC_BASE

#define	CFG_NO_FLASH		1
#define	CFG_ENV_IS_NOWHERE	1
#define	CFG_ENV_SIZE		0x1000
#define	CFG_ENV_ADDR		(CFG_MONITOR_BASE - CFG_ENV_SIZE)


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_BDI
#define CONFIG_CMD_NET
#define CONFIG_CMD_IMI
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_RUN
#define CONFIG_CMD_AUTOSCRIPT
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_MISC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_PING


/* Miscellaneous configurable options */
#define	CFG_PROMPT	"U-Boot-mONStR> "
#define	CFG_CBSIZE	512	/* size of console buffer */
#define	CFG_PBSIZE	(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16) /* print buffer size */
#define	CFG_MAXARGS	15	/* max number of command args */
#define	CFG_LONGHELP
#define	CFG_LOAD_ADDR	0x12000000 /* default load address */

#define	CONFIG_BOOTDELAY 	30
#define	CONFIG_BOOTARGS		"root=romfs"
#define	CONFIG_HOSTNAME		"ml401"
#define	CONFIG_BOOTCOMMAND 	"base 0;tftp 11000000 image.img;bootm"
#define	CONFIG_IPADDR		192.168.0.3
#define	CONFIG_SERVERIP 	192.168.0.5
#define	CONFIG_GATEWAYIP 	192.168.0.1
#define	CONFIG_ETHADDR		00:E0:0C:00:00:FD

/* architecture dependent code */
#define	CFG_USR_EXCEP	/* user exception */
#define CFG_HZ	1000

#define CONFIG_PREBOOT	"echo U-BOOT by mONStR;"	\
	"base 0;" \
	"echo"

/* system ace */
#define	CONFIG_SYSTEMACE
/* #define DEBUG_SYSTEMACE */
#define	SYSTEMACE_CONFIG_FPGA
#define	CFG_SYSTEMACE_BASE	XILINX_SYSACE_BASEADDR
#define	CFG_SYSTEMACE_WIDTH	XILINX_SYSACE_MEM_WIDTH
#define	CONFIG_DOS_PARTITION

#endif	/* __CONFIG_H */
