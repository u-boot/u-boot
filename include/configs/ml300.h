/*
 * ML300.h: ML300 specific config options
 *
 * http://www.xilinx.com/ml300
 *
 * Derived from : ML2.h
 *
 *     Author: Xilinx, Inc.
 *
 *
 *     This program is free software; you can redistribute it and/or modify it
 *     under the terms of the GNU General Public License as published by the
 *     Free Software Foundation; either version 2 of the License, or (at your
 *     option) any later version.
 *
 *
 *     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 *     COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 *     ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
 *     XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
 *     FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR
 *     OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 *     XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 *     THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
 *     WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
 *     CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *     FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 *     Xilinx products are not intended for use in life support appliances,
 *     devices, or systems. Use in such applications is expressly prohibited.
 *
 *
 *     (c) Copyright 2002 Xilinx Inc.
 *     All rights reserved.
 *
 *
 *     You should have received a copy of the GNU General Public License along
 *     with this program; if not, write to the Free Software Foundation, Inc.,
 *     675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* #define DEBUG */
/* #define ET_DEBUG 1 */

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_405		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_XILINX_ML300	1	/* ...on a Xilinx ML300 board	*/

#define CONFIG_SYSTEMACE	1
#define CONFIG_DOS_PARTITION	1
#define CFG_SYSTEMACE_BASE	XPAR_OPB_SYSACE_0_BASEADDR
#define CFG_SYSTEMACE_WIDTH	XPAR_XSYSACE_MEM_WIDTH

#define CFG_ENV_IS_IN_EEPROM	1	/* environment is in EEPROM */

/* following are used only if env is in EEPROM */
#ifdef	CFG_ENV_IS_IN_EEPROM
#define CFG_I2C_EEPROM_ADDR	XPAR_PERSISTENT_0_IIC_0_EEPROMADDR
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_ENV_OFFSET		XPAR_PERSISTENT_0_IIC_0_BASEADDR
#define CONFIG_MISC_INIT_R	1	/* used to call out convert_env() */
#define CONFIG_ENV_OVERWRITE	1	/* allow users to update ethaddr and serial# */
#endif

#include "../board/xilinx/ml300/xparameters.h"

#define CFG_NO_FLASH		1	/* no flash */
#define CFG_ENV_SIZE		XPAR_PERSISTENT_0_IIC_0_HIGHADDR - XPAR_PERSISTENT_0_IIC_0_BASEADDR + 1
#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#define CONFIG_BOOTCOMMAND	"bootp" /* autoboot command	*/

#define CONFIG_BOOTARGS		"console=ttyS0,9600 ip=off " \
				"root=/dev/xsysace/disc0/part3 rw"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_NET

#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_FAT
#undef CONFIG_CMD_IMLS


/* #define CONFIG_SYS_CLK_FREQ XPAR_CORE_CLOCK_FREQ_HZ */
/* 300000000 */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		/* undef to save memory		*/
#define CFG_PROMPT	"=> "	/* Monitor Command Prompt	*/

#define CFG_CBSIZE	256	/* Console I/O Buffer Size	*/

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS	16	/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CFG_DUART_CHAN		0
#define CFG_NS16550_REG_SIZE -4
#define CFG_NS16550 1
#define CFG_INIT_CHAN1	 1

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CFG_LOAD_ADDR		0x400000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ		1000	/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MONITOR_BASE	0x04000000
#define CFG_MONITOR_LEN		(192 * 1024)	/* Reserve 196 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */

#define CFG_INIT_RAM_ADDR	0x800000  /* inside of SDRAM */
#define CFG_INIT_RAM_END	0x2000	  /* End of used area in RAM */
#define CFG_GBL_DATA_SIZE	128	  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02	/* Software reboot			*/

#endif				/* __CONFIG_H */
