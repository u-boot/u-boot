/*
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * BOARD/CPU
 */
#include "../board/altera/nios2-generic/custom_fpga.h" /* fpga parameters */
#define CONFIG_BOARD_NAME "nios2-generic" /* custom board name */
#define CONFIG_BOARD_EARLY_INIT_F	/* enable early board-spec. init */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_SYS_NIOS_SYSID_BASE	CONFIG_SYS_SYSID_BASE

/*
 * SERIAL
 */
#define CONFIG_ALTERA_JTAG_UART
#if defined(CONFIG_ALTERA_JTAG_UART)
# define CONFIG_SYS_NIOS_CONSOLE	CONFIG_SYS_JTAG_UART_BASE
#else
# define CONFIG_SYS_NIOS_CONSOLE	CONFIG_SYS_UART_BASE
#endif

#define CONFIG_ALTERA_JTAG_UART_BYPASS
#define CONFIG_SYS_NIOS_FIXEDBAUD
#define CONFIG_BAUDRATE		CONFIG_SYS_UART_BAUD
#define CONFIG_SYS_BAUDRATE_TABLE	{CONFIG_BAUDRATE}
#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* Suppress console info */

/*
 * TIMER
 */
#define CONFIG_SYS_LOW_RES_TIMER
#define CONFIG_SYS_NIOS_TMRBASE	CONFIG_SYS_TIMER_BASE
#define CONFIG_SYS_NIOS_TMRIRQ		CONFIG_SYS_TIMER_IRQ
#define CONFIG_SYS_NIOS_TMRMS		10	/* Desired period (msec)*/
#define CONFIG_SYS_NIOS_TMRCNT \
	(CONFIG_SYS_NIOS_TMRMS * (CONFIG_SYS_TIMER_FREQ / 1000) - 1)

/*
 * STATUS LED
 */
#define CONFIG_ALTERA_PIO
#define CONFIG_SYS_ALTERA_PIO_NUM	1
#define CONFIG_SYS_ALTERA_PIO_GPIO_NUM	LED_PIO_WIDTH

#define CONFIG_STATUS_LED		/* Enable status driver */
#define CONFIG_BOARD_SPECIFIC_LED
#define CONFIG_GPIO_LED		/* Enable GPIO LED driver */
#define CONFIG_GPIO			/* Enable GPIO driver */
#define LED_PIO_BASE			USER_LED_PIO_8OUT_BASE
#define LED_PIO_WIDTH			8
#define LED_PIO_RSTVAL			0xff

#define STATUS_LED_BIT			0	/* Bit-0 on GPIO */
#define STATUS_LED_STATE		1	/* Blinking */
#define STATUS_LED_PERIOD	(500 / CONFIG_SYS_NIOS_TMRMS) /* 500 msec */

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
#undef CONFIG_CMD_BOOTD
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_ITEST
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_XIMG

#ifdef CONFIG_CMD_NET
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_PING
#endif

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_LMB

/*
 * ENVIRONMENT -- Put environment in sector CONFIG_SYS_MONITOR_LEN above
 * CONFIG_SYS_RESET_ADDR, since we assume the monitor is stored at the
 * reset address, no? This will keep the environment in user region
 * of flash. NOTE: the monitor length must be multiple of sector size
 * (which is common practice).
 */
#define CONFIG_ENV_IS_IN_FLASH

#define CONFIG_ENV_SIZE		0x20000	/* 128k, 1 sector */
#define CONFIG_ENV_OVERWRITE		/* Serial change Ok	*/
#define CONFIG_ENV_ADDR		((CONFIG_SYS_RESET_ADDR + \
					  CONFIG_SYS_MONITOR_LEN) | \
					 CONFIG_SYS_FLASH_BASE)

/*
 * MEMORY ORGANIZATION
 * -Monitor at top of sdram.
 * -The heap is placed below the monitor
 * -The stack is placed below the heap (&grows down).
 */
#define CONFIG_MONITOR_IS_IN_RAM
#define CONFIG_SYS_MONITOR_LEN		0x40000	/* Reserve 256k */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_SDRAM_SIZE - \
					 CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 0x20000)
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_MONITOR_BASE - \
					 CONFIG_SYS_MALLOC_LEN)
#define CONFIG_SYS_INIT_SP		CONFIG_SYS_MALLOC_BASE

/*
 * MISC
 */
#define CONFIG_SYS_LONGHELP		/* Provide extended help */
#define CONFIG_SYS_CBSIZE		256	/* Console I/O buf size */
#define CONFIG_SYS_MAXARGS		16	/* Max command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Bootarg buf size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + \
					 16)	/* Print buf size */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP - 0x20000)
#define CONFIG_CMDLINE_EDITING

#define CONFIG_SYS_HUSH_PARSER

#endif /* __CONFIG_H */
