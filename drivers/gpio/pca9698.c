/*
 * (C) Copyright 2011
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
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

/*
 * Driver for NXP's pca9698 40 bit I2C gpio expander
 */

#include <common.h>
#include <i2c.h>
#include <pca9698.h>

/*
 * The pca9698 registers
 */

#define PCA9698_REG_INPUT		0x00
#define PCA9698_REG_OUTPUT		0x08
#define PCA9698_REG_POLARITY		0x10
#define PCA9698_REG_CONFIG		0x18

#define PCA9698_BUFFER_SIZE		5

static int pca9698_read40(u8 chip, u8 offset, u8 *buffer)
{
	u8 command = offset | 0x80;  /* autoincrement */

	return i2c_read(chip, command, 1, buffer, PCA9698_BUFFER_SIZE);
}

static int pca9698_write40(u8 chip, u8 offset, u8 *buffer)
{
	u8 command = offset | 0x80;  /* autoincrement */

	return i2c_write(chip, command, 1, buffer, PCA9698_BUFFER_SIZE);
}

static void pca9698_set_bit(unsigned gpio, u8 *buffer, unsigned value)
{
	unsigned byte = gpio / 8;
	unsigned bit = gpio % 8;

	if (value)
		buffer[byte] |= (1 << bit);
	else
		buffer[byte] &= ~(1 << bit);
}

int pca9698_direction_input(u8 chip, unsigned offset)
{
	u8 data[PCA9698_BUFFER_SIZE];
	int res;

	res = pca9698_read40(chip, PCA9698_REG_CONFIG, data);
	if (res)
		return res;

	pca9698_set_bit(offset, data, 1);
	return pca9698_write40(chip, PCA9698_REG_CONFIG, data);
}

int pca9698_direction_output(u8 chip, unsigned offset)
{
	u8 data[PCA9698_BUFFER_SIZE];
	int res;

	res = pca9698_read40(chip, PCA9698_REG_CONFIG, data);
	if (res)
		return res;

	pca9698_set_bit(offset, data, 0);
	return pca9698_write40(chip, PCA9698_REG_CONFIG, data);
}

int pca9698_get_input(u8 chip, unsigned offset)
{
	unsigned config_byte = offset / 8;
	unsigned config_bit = offset % 8;
	unsigned value;
	u8 data[PCA9698_BUFFER_SIZE];
	int res;

	res = pca9698_read40(chip, PCA9698_REG_INPUT, data);
	if (res)
		return -1;

	value = data[config_byte] & (1 << config_bit);

	return !!value;
}

int pca9698_set_output(u8 chip, unsigned offset, int value)
{
	u8 data[PCA9698_BUFFER_SIZE];
	int res;

	res = pca9698_read40(chip, PCA9698_REG_OUTPUT, data);
	if (res)
		return res;

	memset(data, sizeof(data), 0);
	pca9698_set_bit(offset, data, value);
	return pca9698_write40(chip, PCA9698_REG_OUTPUT, data);
}
