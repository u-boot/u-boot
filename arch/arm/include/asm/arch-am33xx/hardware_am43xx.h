/*
 * hardware_am43xx.h
 *
 * AM43xx hardware specific header
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AM43XX_HARDWARE_AM43XX_H
#define __AM43XX_HARDWARE_AM43XX_H

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
#define PRCM_BASE			0x44DF0000
#define	CM_WKUP				0x44DF2800
#define	CM_PER				0x44DF8800

#define PRM_RSTCTRL			(PRCM_BASE + 0x4000)
#define PRM_RSTST			(PRM_RSTCTRL + 4)

/* VTP Base address */
#define VTP0_CTRL_ADDR			0x44E10E0C
#define VTP1_CTRL_ADDR			0x48140E10

/* DDR Base address */
#define DDR_PHY_CMD_ADDR		0x44E12000
#define DDR_PHY_DATA_ADDR		0x44E120C8
#define DDR_PHY_CMD_ADDR2		0x47C0C800
#define DDR_PHY_DATA_ADDR2		0x47C0C8C8
#define DDR_DATA_REGS_NR		2

/* CPSW Config space */
#define CPSW_MDIO_BASE			0x4A101000

/* RTC base address */
#define RTC_BASE			0x44E3E000

#endif /* __AM43XX_HARDWARE_AM43XX_H */
