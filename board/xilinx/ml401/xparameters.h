/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* DDR SDRAM */
#define CONFIG_XILINX_ERAM_START 0x10000000
#define CONFIG_XILINX_ERAM_SIZE 0x04000000

/* FLASH_MEMORY Settings */
#define CONFIG_XILINX_FLASH_START 0x28000000
#define CONFIG_XILINX_FLASH_SIZE 0x00800000

/* serial line */
#define CONFIG_XILINX_UARTLITE_0_BASEADDR 0xA0000000
#define CONFIG_XILINX_UARTLITE_0_BAUDRATE 115200

/* GPIO */
#define CONFIG_XILINX_GPIO_0_BASEADDR 0x90000000

/* INTC */
#define CONFIG_XILINX_INTC_0_BASEADDR 0xD1000FC0
#define CONFIG_XILINX_INTC_0_NUM_INTR_INPUTS 12

/* TIMER */
#define CONFIG_XILINX_TIMER_0_BASEADDR 0xA2000000
#define CONFIG_XILINX_TIMER_0_IRQ 0

/* ethernet */
#define XPAR_XEMAC_NUM_INSTANCES 1
#define XPAR_OPB_ETHERNET_0_BASEADDR 0x60000000
#define XPAR_OPB_ETHERNET_0_HIGHADDR 0x60003FFF
#define XPAR_OPB_ETHERNET_0_DEVICE_ID 0
#define XPAR_EMAC_0_DEVICE_ID 0
#define XPAR_OPB_ETHERNET_0_ERR_COUNT_EXIST 1
#define XPAR_OPB_ETHERNET_0_DMA_PRESENT 1
#define XPAR_OPB_ETHERNET_0_MII_EXIST 1
