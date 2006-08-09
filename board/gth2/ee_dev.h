/* By Thomas.Lange@Corelatus.com 001025 */
/* Definitions for EEPROM/VOLT METER  DS2438 */
/* Copyright (C) 2000-2005 Corelatus AB */

/* This program is free software; you can redistribute it and/or
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

#ifndef INCeedevh
#define INCeedevh

#define E_DEBUG(fmt,args...) if( Debug ) printk(KERN_DEBUG"EE: " fmt, ##args)

/* MIPS */
#define WRITE_PORT(Value) write_gpio_data(Value)

#define READ_PORT (gpio_read()&GPIO_EEDQ)

/* 64 bytes chip */
#define EE_CHIP_SIZE 64

/* Board with new current resistor */
#define EE_GTH_0304 1

/* new dsp and 64 MB SDRAM */
#define EE_DSP_64 0x10

/* microsecs */
/* Pull line down at least this long for reset pulse */
#define RESET_LOW_TIME    490

/* Read presence pulse after we release reset pulse */
#define PRESENCE_TIMEOUT  100
#define PRESENCE_LOW_TIME 200

#define WRITE_0_LOW 60
#define WRITE_1_LOW 1
#define TOTAL_WRITE_LOW 60

#define READ_LOW        1
#define READ_TIMEOUT   10
#define TOTAL_READ_LOW 70

/* Rom function commands */
#define READ_ROM   0x33
#define MATCH_ROM  0x55
#define SKIP_ROM   0xCC
#define SEARCH_ROM 0xF0


/* Memory_command_function */
#define WRITE_SCRATCHPAD 0x4E
#define READ_SCRATCHPAD  0xBE
#define COPY_SCRATCHPAD  0x48
#define RECALL_MEMORY    0xB8
#define CONVERT_TEMP     0x44
#define CONVERT_VOLTAGE  0xB4

/* Chip is divided in 8 pages, 8 bytes each */

#define EE_PAGE_SIZE 8

/* All chip data we want are in page 0 */

/* Bytes in page 0 */
#define EE_P0_STATUS   0
#define EE_P0_TEMP_LSB 1
#define EE_P0_TEMP_MSB 2
#define EE_P0_VOLT_LSB 3
#define EE_P0_VOLT_MSB 4
#define EE_P0_CURRENT_LSB 5
#define EE_P0_CURRENT_MSB 6


/* 40 byte user data is located at page 3-7 */
#define EE_USER_PAGE_0 3
#define USER_PAGES 5

/* Layout of gth user pages usage */
/* Bytes 0-16   ethernet addr in ascii ( len 17 ) */

#define EE_ETHERNET_OFFSET       0

#endif /* INCeedevh */
