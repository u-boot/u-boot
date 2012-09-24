/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
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

/* Handle multiple I2C buses instances */
int get_multi_scl_pin(void)
{
	unsigned int bus = I2C_GET_BUS();

	switch (bus) {
	case I2C_0: /* I2C_0 definition - compatibility layer */
	case I2C_5:
		return CONFIG_SOFT_I2C_I2C5_SCL;
	case I2C_9:
		return CONFIG_SOFT_I2C_I2C9_SCL;
	default:
		printf("I2C_%d not supported!\n", bus);
	};

	return 0;
}

int get_multi_sda_pin(void)
{
	unsigned int bus = I2C_GET_BUS();

	switch (bus) {
	case I2C_0: /* I2C_0 definition - compatibility layer */
	case I2C_5:
		return CONFIG_SOFT_I2C_I2C5_SDA;
	case I2C_9:
		return CONFIG_SOFT_I2C_I2C9_SDA;
	default:
		printf("I2C_%d not supported!\n", bus);
	};

	return 0;
}

int multi_i2c_init(void)
{
	return 0;
}
