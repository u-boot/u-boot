/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
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

#include <common.h>
#include <i2c.h>

/* I2C chip addresses */
#define TWL6035_CHIP_ADDR	0x48

/* 0x1XY translates to page 1, register address 0xXY */
#define LDO9_CTRL		0x60
#define LDO9_VOLTAGE		0x61

/* Bit field definitions for LDOx_CTRL */
#define LDO_ON			(1 << 4)
#define LDO_MODE_SLEEP		(1 << 2)
#define LDO_MODE_ACTIVE		(1 << 0)

int twl6035_i2c_write_u8(u8 chip_no, u8 val, u8 reg);
int twl6035_i2c_read_u8(u8 chip_no, u8 *val, u8 reg);
void twl6035_init_settings(void);
void twl6035_mmc1_poweron_ldo(void);
