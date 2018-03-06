/*
 * (C) Copyright 2013
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
