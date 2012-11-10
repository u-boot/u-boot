/*
 * Copyright 2012 Freescale Semiconductor, Inc.
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

#ifndef __VSC_CROSSBAR_H_
#define __VSC_CROSSBAR_H	1_

#include <common.h>
#include <i2c.h>
#include <errno.h>

int vsc_if_enable(unsigned int vsc_addr);
int vsc3316_config(unsigned int vsc_addr, const int8_t con_arr[][2],
		unsigned int num_con);
int vsc3308_config(unsigned int vsc_addr, const int8_t con_arr[][2],
		unsigned int num_con);
void vsc_wp_config(unsigned int vsc_addr);

#endif	/* __VSC_CROSSBAR_H_ */
