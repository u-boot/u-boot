/*
 * Copyright (c) 2011 The Chromium OS Authors.
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

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_NR_DRAM_BANKS	1
#define CONFIG_DRAM_SIZE	(128 << 20)

/* Number of bits in a C 'long' on this architecture */
#define CONFIG_SANDBOX_BITS_PER_LONG	64

#define CONFIG_OF_CONTROL
#define CONFIG_OF_LIBFDT
#define CONFIG_LMB

#define CONFIG_SYS_VSNPRINTF

#define CONFIG_CMD_GPIO
#define CONFIG_SANDBOX_GPIO
#define CONFIG_SANDBOX_GPIO_COUNT	20

/*
 * Size of malloc() pool, although we don't actually use this yet.
 */
#define CONFIG_SYS_MALLOC_LEN		(4 << 20)	/* 4MB  */

#define CONFIG_SYS_PROMPT		"=>"	/* Command Prompt */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_LONGHELP			/* #undef to save memory */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16

/* turn on command-line edit/c/auto */
#define CONFIG_CMDLINE_EDITING
#define CONFIG_COMMAND_HISTORY
#define CONFIG_AUTO_COMPLETE

#define CONFIG_ENV_SIZE		8192
#define CONFIG_ENV_IS_NOWHERE

#define CONFIG_SYS_HZ			1000

/* Memory things - we don't really want a memory test */
#define CONFIG_SYS_LOAD_ADDR		0x10000000
#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x1000)
#define CONFIG_PHYS_64BIT

/* Size of our emulated memory */
#define CONFIG_SYS_SDRAM_SIZE		(128 << 20)

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
#define CONFIG_SANDBOX_SERIAL

#define CONFIG_SYS_NO_FLASH

/* include default commands */
#include <config_cmd_default.h>

/* We don't have networking support yet */
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

#define CONFIG_BOOTARGS ""

#define CONFIG_EXTRA_ENV_SETTINGS	"stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#endif
