/*
 * (C) Copyright 2010 DENX Software Engineering
 * Anatolij Gustschin <agust@denx.de>
 *
 * Common configuration options for MPC5121 based boards
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
#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

#endif /* __MPC5121_COMMON_H */
