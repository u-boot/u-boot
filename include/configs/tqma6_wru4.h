/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 */

#ifndef __CONFIG_TQMA6_WRU4_H
#define __CONFIG_TQMA6_WRU4_H

/* Ethernet */
#define CONFIG_FEC_MXC_PHYADDR		0x01

/* UART */
#define CONFIG_MXC_UART_BASE		UART4_BASE
#define CONSOLE_DEV		"ttymxc3"

/* Watchdog */

/* Config on-board RTC */
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_RTC_BUS_NUM		2
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
/* Turn off RTC square-wave output to save battery */
#define CONFIG_RTC_DS1337_NOOSC

/* LED */

/* Bootcounter */
#define CONFIG_SYS_BOOTCOUNT_BE

/* I2C */

#endif /* __CONFIG_TQMA6_WRU4_H */
