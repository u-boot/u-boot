/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This file contains Marvell Board Specific common defincations.
 * This file should be included in board config header file.
 *
 * It supports common definations for Kirkwood platform
 * TBD: support for Orion5X platforms
 */

#ifndef _MV_COMMON_H
#define _MV_COMMON_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_MARVELL		1

/*
 * Custom CONFIG_SYS_TEXT_BASE can be done in <board>.h
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_TEXT_BASE	0x00600000
#endif /* CONFIG_SYS_TEXT_BASE */

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE	0x00000000

/*
 * CLKs configurations
 */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#if !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_COM1		MV_UART_CONSOLE_BASE
#endif

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define CONFIG_CONS_INDEX	1	/*Console on UART0 */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, \
					  115200,230400, 460800, 921600 }
/* auto boot */
#define CONFIG_PREBOOT

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG	1	/* enable INITRD tag */
#define CONFIG_SETUP_MEMORY_TAGS 1	/* enable memory tag */

#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buff Size */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(1024 * 1024 * 4) /* 4MiB for malloc() */

/*
 * Other required minimal configurations
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_ARCH_CPU_INIT	/* call arch_cpu_init() */
#define CONFIG_SYS_LOAD_ADDR	0x00800000	/* default load adr- 8M */
#define CONFIG_SYS_MEMTEST_START 0x00800000	/* 8M */
#define CONFIG_SYS_MEMTEST_END	0x00ffffff	/*(_16M -1) */
#define CONFIG_SYS_RESET_ADDRESS 0xffff0000	/* Rst Vector Adr */
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */

/* ====> Include platform Common Definitions */
#include <asm/arch/config.h>

/*
 * DRAM Banks configuration, Custom config can be done in <board>.h
 */
#ifndef CONFIG_NR_DRAM_BANKS
#define CONFIG_NR_DRAM_BANKS	CONFIG_NR_DRAM_BANKS_MAX
#else
#if (CONFIG_NR_DRAM_BANKS > CONFIG_NR_DRAM_BANKS_MAX)
#error CONFIG_NR_DRAM_BANKS Configurated more than available
#endif
#endif /* CONFIG_NR_DRAM_BANKS */

/* ====> Include driver Common Definitions */
/*
 * Common NAND configuration
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#endif

/*
 * Common SPI Flash configuration
 */
#ifdef CONFIG_CMD_SF
#endif

/*
 * Common USB/EHCI configuration
 */
#if defined(CONFIG_CMD_USB) && !defined(CONFIG_DM)
#define CONFIG_SUPPORT_VFAT
#endif /* CONFIG_CMD_USB */

/*
 * File system
 */
#ifdef CONFIG_SYS_MVFS
#define CONFIG_MTD_DEVICE               /* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS
#endif

#endif /* _MV_COMMON_H */
