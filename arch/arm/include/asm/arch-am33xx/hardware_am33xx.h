/*
 * hardware_am33xx.h
 *
 * AM33xx hardware specific header
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AM33XX_HARDWARE_AM33XX_H
#define __AM33XX_HARDWARE_AM33XX_H

/* Module base addresses */

/* UART Base Address */
#define UART0_BASE			0x44E09000

/* GPIO Base address */
#define GPIO2_BASE			0x481AC000

/* Watchdog Timer */
#define WDT_BASE			0x44E35000

/* Control Module Base Address */
#define CTRL_BASE			0x44E10000
#define CTRL_DEVICE_BASE		0x44E10600

/* PRCM Base Address */
#define PRCM_BASE			0x44E00000

/* VTP Base address */
#define VTP0_CTRL_ADDR			0x44E10E0C

/* DDR Base address */
#define DDR_PHY_CMD_ADDR		0x44E12000
#define DDR_PHY_DATA_ADDR		0x44E120C8
#define DDR_DATA_REGS_NR		2

/* CPSW Config space */
#define CPSW_MDIO_BASE			0x4A101000

/* RTC base address */
#define RTC_BASE			0x44E3E000

#endif /* __AM33XX_HARDWARE_AM33XX_H */
