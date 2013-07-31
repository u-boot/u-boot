/*
 * Common definitions for LPC32XX board configurations
 *
 * Copyright (C) 2011 Vladimir Zapolskiy <vz@mleia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _LPC32XX_CONFIG_H
#define _LPC32XX_CONFIG_H

/* Basic CPU architecture */
#define CONFIG_ARM926EJS
#define CONFIG_ARCH_CPU_INIT

#define CONFIG_NR_DRAM_BANKS_MAX	2

/* 1KHz clock tick */
#define CONFIG_SYS_HZ			1000

/* UART configuration */
#if (CONFIG_SYS_LPC32XX_UART >= 3) && (CONFIG_SYS_LPC32XX_UART <= 6)
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_CONS_INDEX		(CONFIG_SYS_LPC32XX_UART - 2)
#elif	(CONFIG_SYS_LPC32XX_UART == 1) || (CONFIG_SYS_LPC32XX_UART == 2) || \
	(CONFIG_SYS_LPC32XX_UART == 7)
#define CONFIG_LPC32XX_HSUART
#else
#error "define CONFIG_SYS_LPC32XX_UART in the range from 1 to 7"
#endif

#if defined(CONFIG_SYS_NS16550_SERIAL)
#define CONFIG_SYS_NS16550

#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#define CONFIG_SYS_NS16550_COM1		UART3_BASE
#define CONFIG_SYS_NS16550_COM2		UART4_BASE
#define CONFIG_SYS_NS16550_COM3		UART5_BASE
#define CONFIG_SYS_NS16550_COM4		UART6_BASE
#endif

#if defined(CONFIG_LPC32XX_HSUART)
#if	CONFIG_SYS_LPC32XX_UART == 1
#define HS_UART_BASE			HS_UART1_BASE
#elif	CONFIG_SYS_LPC32XX_UART == 2
#define HS_UART_BASE			HS_UART2_BASE
#else	/* CONFIG_SYS_LPC32XX_UART == 7 */
#define HS_UART_BASE			HS_UART7_BASE
#endif
#endif

#define CONFIG_SYS_BAUDRATE_TABLE	\
		{ 9600, 19200, 38400, 57600, 115200, 230400, 460800 }

/* NOR Flash */
#if defined(CONFIG_SYS_FLASH_CFI)
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_PROTECTION
#endif

#endif /* _LPC32XX_CONFIG_H */
