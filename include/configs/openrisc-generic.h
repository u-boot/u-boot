/*
 * (C) Copyright 2011, Stefan Kristiansson, stefan.kristianssons@saunalahti.fi
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * BOARD/CPU
 */

#define CONFIG_SYS_CLK_FREQ		50000000
#define CONFIG_SYS_RESET_ADDR		0x00000100

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		0x02000000

#define CONFIG_SYS_CACHELINE_SIZE	16

#define CONFIG_SYS_UART_BASE		0x90000000
#define CONFIG_SYS_UART_FREQ		CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_UART_BAUD		115200

#define CONFIG_BOARD_NAME		"OpenRISC Generic"

#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_MAX_FLASH_SECT	0

/*
 * SERIAL
 */
# define CONFIG_SYS_NS16550
# define CONFIG_SYS_NS16550_SERIAL
# define CONFIG_SYS_NS16550_REG_SIZE	1
# define CONFIG_CONS_INDEX		1
# define CONFIG_SYS_NS16550_COM1	(0x90000000)
# define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_CLK_FREQ

#define CONFIG_BAUDRATE			CONFIG_SYS_UART_BAUD
#define CONFIG_SYS_BAUDRATE_TABLE	{CONFIG_BAUDRATE}
#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* Suppress console info */
#define CONSOLE_ARG			"console=console=ttyS0,115200\0"

/*
 * Ethernet
 */
#define CONFIG_ETHOC
#define CONFIG_SYS_ETHOC_BASE		0x92000000

#define CONFIG_BOOTFILE			"boot.img"
#define CONFIG_LOADADDR			0x100000 /* 1MB mark */

/*
 * TIMER
 */
#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_OPENRISC_TMR_HZ	100

/*
 * Memory organisation:
 *
 * RAM start ---------------------------
 *           | ...                     |
 *           ---------------------------
 *           | Stack                   |
 *           ---------------------------
 *           | Global data             |
 *           ---------------------------
 *           | Environment             |
 *           ---------------------------
 *           | Monitor                 |
 * RAM end   ---------------------------
 */
/* We're running in RAM */
#define CONFIG_MONITOR_IS_IN_RAM
#define CONFIG_SYS_MONITOR_LEN	0x40000	/* Reserve 256k */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_SDRAM_BASE + \
				CONFIG_SYS_SDRAM_SIZE - \
				CONFIG_SYS_MONITOR_LEN)

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE		0x20000 /* Total Size of Environment, 128KB */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)

/*
 * Global data object and stack pointer
 */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_ENV_ADDR \
					- GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_GBL_DATA_ADDR	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_STACK_LENGTH		0x10000 /* 64KB */
#define CONFIG_SYS_MALLOC_LEN		0x400000 /* 4MB */
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_INIT_SP_OFFSET \
					- CONFIG_SYS_STACK_LENGTH \
					- CONFIG_SYS_MALLOC_LEN)
/*
 * MISC
 */
#define CONFIG_SYS_LONGHELP		/* Provide extended help */
#define CONFIG_SYS_PROMPT		"==> "	/* Command prompt	*/
#define CONFIG_SYS_CBSIZE		256	/* Console I/O buf size */
#define CONFIG_SYS_MAXARGS		16	/* Max command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Bootarg buf size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + \
					16)	/* Print buf size */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x2000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP_ADDR - 0x20000)
#define CONFIG_CMDLINE_EDITING

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>


#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ELF
#define CONFIG_CMD_BSP

#define CONFIG_CMD_MII
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING

#define CONFIG_OF_LIBFDT
#define CONFIG_LMB

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


#endif /* __CONFIG_H */
