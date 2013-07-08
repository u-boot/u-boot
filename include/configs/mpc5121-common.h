/*
 * (C) Copyright 2010 DENX Software Engineering
 * Anatolij Gustschin <agust@denx.de>
 *
 * Common configuration options for MPC5121 based boards
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MPC5121_COMMON_H
#define __MPC5121_COMMON_H

/* Use SRAM for initial stack */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_SRAM_BASE /* Init RAM base */
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_SRAM_SIZE /* Size of area */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * Serial console
 */
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_CMDLINE_EDITING		1	/* command line history */
/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

#endif /* __MPC5121_COMMON_H */
