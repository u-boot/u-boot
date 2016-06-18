/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * CAUTION: This file is a faked configuration !!!
 *          There is no real target for the microblaze-generic
 *          configuration. You have to replace this file with
 *          the generated file from your Xilinx design flow.
 */

#define XILINX_BOARD_NAME	microblaze-generic

/* Microblaze is microblaze_0 */
#define XILINX_FSL_NUMBER	3

/* GPIO is LEDs_4Bit*/
#define XILINX_GPIO_BASEADDR	0x40000000

/* Flash Memory is FLASH_2Mx32 */
#define XILINX_FLASH_START	0x2c000000
#define XILINX_FLASH_SIZE	0x00800000

/* Watchdog IP is wxi_timebase_wdt_0 */
#define XILINX_WATCHDOG_BASEADDR	0x50000000
#define XILINX_WATCHDOG_IRQ		1
