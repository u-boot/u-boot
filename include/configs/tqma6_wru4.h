/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA6_WRU4_H
#define __CONFIG_TQMA6_WRU4_H

/* DTT sensors */
#define CONFIG_DTT_SENSORS		{ 0, 1 }
#define CONFIG_SYS_DTT_BUS_NUM		2

/* Ethernet */
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0x01
#define CONFIG_PHY_SMSC

/* UART */
#define CONFIG_MXC_UART_BASE		UART4_BASE
#define CONSOLE_DEV		"ttymxc3"

#define CONFIG_MISC_INIT_R

/* Watchdog */
#define CONFIG_HW_WATCHDOG
#define CONFIG_IMX_WATCHDOG
#define CONFIG_WATCHDOG_TIMEOUT_MSECS	60000

/* Config on-board RTC */
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_RTC_BUS_NUM		2
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
/* Turn off RTC square-wave output to save battery */
#define CONFIG_SYS_RTC_DS1337_NOOSC
#define CONFIG_CMD_DATE

/* LED */
#define CONFIG_CMD_LED
#define CONFIG_STATUS_LED
#define CONFIG_BOARD_SPECIFIC_LED
#define STATUS_LED_BIT			0
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT1			1
#define STATUS_LED_STATE1		STATUS_LED_ON
#define STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT2			2
#define STATUS_LED_STATE2		STATUS_LED_ON
#define STATUS_LED_PERIOD2		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT3			3
#define STATUS_LED_STATE3		STATUS_LED_ON
#define STATUS_LED_PERIOD3		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT4			4
#define STATUS_LED_STATE4		STATUS_LED_ON
#define STATUS_LED_PERIOD4		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT5			5
#define STATUS_LED_STATE5		STATUS_LED_ON
#define STATUS_LED_PERIOD5		(CONFIG_SYS_HZ / 2)

/* Bootcounter */
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_BOOTCOUNT_BE

/*
 * Remove all unused interfaces / commands that are defined in
 * the common header tqms6.h
 */
#undef CONFIG_MXC_SPI

#endif /* __CONFIG_TQMA6_WRU4_H */
