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

/* System Clock Frequency */
#define XILINX_CLOCK_FREQ	100000000

/* Microblaze is microblaze_0 */
#define XILINX_USE_MSR_INSTR	1
#define XILINX_FSL_NUMBER	3

/* Interrupt controller is opb_intc_0 */
#define XILINX_INTC_BASEADDR	0x41200000
#define XILINX_INTC_NUM_INTR_INPUTS	6

/* Timer pheriphery is opb_timer_1 */
#define XILINX_TIMER_BASEADDR	0x41c00000
#define XILINX_TIMER_IRQ	0

/* Uart pheriphery is RS232_Uart */
#define XILINX_UARTLITE_BASEADDR	0x40600000
#define XILINX_UARTLITE_BAUDRATE	115200

/* IIC pheriphery is IIC_EEPROM */
#define XILINX_IIC_0_BASEADDR	0x40800000
#define XILINX_IIC_0_FREQ	100000
#define XILINX_IIC_0_BIT	0

/* GPIO is LEDs_4Bit*/
#define XILINX_GPIO_BASEADDR	0x40000000

/* Flash Memory is FLASH_2Mx32 */
#define XILINX_FLASH_START	0x2c000000
#define XILINX_FLASH_SIZE	0x00800000

/* Main Memory is DDR_SDRAM_64Mx32 */
#define XILINX_RAM_START	0x28000000
#define XILINX_RAM_SIZE	0x04000000

/* Sysace Controller is SysACE_CompactFlash */
#define XILINX_SYSACE_BASEADDR	0x41800000
#define XILINX_SYSACE_HIGHADDR	0x4180ffff
#define XILINX_SYSACE_MEM_WIDTH	16

/* Ethernet controller is Ethernet_MAC */
#define XILINX_EMACLITE_BASEADDR       0x40C00000

/* Watchdog IP is wxi_timebase_wdt_0 */
#define XILINX_WATCHDOG_BASEADDR	0x50000000
#define XILINX_WATCHDOG_IRQ		1
