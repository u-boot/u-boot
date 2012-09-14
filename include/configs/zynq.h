/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
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

#ifndef __CONFIG_ZYNQ_H
#define __CONFIG_ZYNQ_H

#define CONFIG_ARMV7 /* This is an ARM V7 CPU core */
#define CONFIG_ZYNQ

/* CPU clock */
#define CONFIG_CPU_FREQ_HZ	800000000
#define CONFIG_SYS_HZ		1000

/* Ram */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_TEXT_BASE		0
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		0x40000000
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x1000)

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CONFIG_BAUDRATE		115200

/* XPSS Serial driver */
#define CONFIG_ZYNQ_SERIAL
#define CONFIG_ZYNQ_SERIAL_BASEADDR0	0xE0001000
#define CONFIG_ZYNQ_SERIAL_BAUDRATE0	CONFIG_BAUDRATE
#define CONFIG_ZYNQ_SERIAL_CLOCK0	50000000

/* SCU timer address is hardcoded */
#define CONFIG_SCUTIMER_BASEADDR	0xF8F00600

/* Ethernet driver */
#define CONFIG_NET_MULTI
#define CONFIG_ZYNQ_GEM
#define CONFIG_ZYNQ_GEM_BASEADDR0	0xE000B000

#define CONFIG_BOOTP_SERVERIP
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MAY_FAIL

/* MII and Phylib */
#define CONFIG_MII
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#define CONFIG_PHYLIB
#define CONFIG_PHY_MARVELL

/* Environment */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE 0x10000

#define CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_MALLOC_LEN		0x400000
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_PROMPT	"U-Boot> "
#define CONFIG_SYS_CBSIZE	256 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_LOAD_ADDR	0
#define CONFIG_SYS_MAXARGS	15 /* max number of command args */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

/* OF */
#define CONFIG_FIT
#define CONFIG_OF_LIBFDT

/* Commands */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII

#endif /* __CONFIG_ZYNQ_H */
