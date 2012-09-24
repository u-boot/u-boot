/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef __ASM_ARCH_MXC_MXC_I2C_H__
#define __ASM_ARCH_MXC_MXC_I2C_H__
#include <asm/imx-common/iomux-v3.h>

struct i2c_pin_ctrl {
	iomux_v3_cfg_t i2c_mode;
	iomux_v3_cfg_t gpio_mode;
	unsigned char gp;
	unsigned char spare;
};

struct i2c_pads_info {
	struct i2c_pin_ctrl scl;
	struct i2c_pin_ctrl sda;
};

void setup_i2c(unsigned i2c_index, int speed, int slave_addr,
		struct i2c_pads_info *p);
void bus_i2c_init(void *base, int speed, int slave_addr,
		int (*idle_bus_fn)(void *p), void *p);
int bus_i2c_read(void *base, uchar chip, uint addr, int alen, uchar *buf,
		int len);
int bus_i2c_write(void *base, uchar chip, uint addr, int alen,
		const uchar *buf, int len);
#endif
