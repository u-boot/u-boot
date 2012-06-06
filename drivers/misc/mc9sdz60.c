/*
 * (C) Copyright 2010 Stefano Babic <sbabic@denx.de>
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


#include <config.h>
#include <common.h>
#include <asm/errno.h>
#include <linux/types.h>
#include <i2c.h>
#include <mc9sdz60.h>

#ifndef CONFIG_SYS_FSL_MC9SDZ60_I2C_ADDR
#error "You have to configure I2C address for MC9SDZ60"
#endif


u8 mc9sdz60_reg_read(enum mc9sdz60_reg reg)
{
	u8 val;

	if (i2c_read(CONFIG_SYS_FSL_MC9SDZ60_I2C_ADDR, reg, 1, &val, 1)) {
		puts("Error reading MC9SDZ60 register\n");
		return -1;
	}

	return val;
}

void mc9sdz60_reg_write(enum mc9sdz60_reg reg, u8 val)
{
	i2c_write(CONFIG_SYS_FSL_MC9SDZ60_I2C_ADDR, reg, 1, &val, 1);
}
