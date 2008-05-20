/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define GPIO_CPU_LED		GPIO_3


#define CPLD_BASE		0x10000000		/* t.b.m. */
#define DEBUG_LEDS_ADDR		CPLD_BASE + 0x01
#define HW_ID_ADDR		CPLD_BASE + 0x02
#define DIP_SWITCH_ADDR		CPLD_BASE + 0x04
#define PHY_CTRL_ADDR		CPLD_BASE + 0x05
#define SPI_OUT_ADDR		CPLD_BASE + 0x07
#define SPI_IN_ADDR		CPLD_BASE + 0x08
#define MDIO_OUT_ADDR		CPLD_BASE + 0x09
#define MDIO_IN_ADDR		CPLD_BASE + 0x0A
#define MISC_OUT_ADDR		CPLD_BASE + 0x0B

/* Addresses used on I2C bus */
#define LM75_CHIP_ADDR		0x9C
#define LM75_CPU_ADDR		0x9E
#define SDRAM_SPD_ADDR		0xA0

#define SDRAM_SPD_WRITE_ADDRESS	(SDRAM_SPD_ADDR)
#define SDRAM_SPD_READ_ADDRESS	(SDRAM_SPD_ADDR+1)

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
