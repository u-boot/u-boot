/*
 * hardware_ti814x.h
 *
 * TI814x hardware specific header
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __AM33XX_HARDWARE_TI814X_H
#define __AM33XX_HARDWARE_TI814X_H

/* Module base addresses */

/* UART Base Address */
#define UART0_BASE			0x48020000

/* Watchdog Timer */
#define WDT_BASE			0x481C7000

/* Control Module Base Address */
#define CTRL_BASE			0x48140000
#define CTRL_DEVICE_BASE		0x48140600

/* PRCM Base Address */
#define PRCM_BASE			0x48180000

/* PLL Subsystem Base Address */
#define PLL_SUBSYS_BASE			0x481C5000

/* VTP Base address */
#define VTP0_CTRL_ADDR			0x48140E0C

/* DDR Base address */
#define DDR_PHY_CMD_ADDR		0x47C0C400
#define DDR_PHY_DATA_ADDR		0x47C0C4C8
#define DDR_DATA_REGS_NR		4

/* CPSW Config space */
#define CPSW_MDIO_BASE			0x4A100800

/* RTC base address */
#define RTC_BASE			0x480C0000

#endif /* __AM33XX_HARDWARE_TI814X_H */
