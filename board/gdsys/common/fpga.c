/*
 * (C) Copyright 2013
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

#include <common.h>
#include <gdsys_fpga.h>

#include <asm/io.h>

int fpga_set_reg(u32 fpga, u16 *reg, off_t regoff, u16 data)
{
	out_le16(reg, data);

	return 0;
}

int fpga_get_reg(u32 fpga, u16 *reg, off_t regoff, u16 *data)
{
	*data = in_le16(reg);

	return 0;
}
