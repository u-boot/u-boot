/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
 *
 * (C) Copyright 2010
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
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
 *
 */

#include <common.h>
#include <spartan3.h>
#include <command.h>
#include <asm/io.h>
#include "fpga.h"
#include "mvsmr.h"

Xilinx_Spartan3_Slave_Serial_fns fpga_fns = {
	fpga_pre_config_fn,
	fpga_pgm_fn,
	fpga_clk_fn,
	fpga_init_fn,
	fpga_done_fn,
	fpga_wr_fn,
	0
};

Xilinx_desc spartan3 = {
	Xilinx_Spartan2,
	slave_serial,
	XILINX_XC3S200_SIZE,
	(void *) &fpga_fns,
	0,
};

DECLARE_GLOBAL_DATA_PTR;

int mvsmr_init_fpga(void)
{
	fpga_init();
	fpga_add(fpga_xilinx, &spartan3);

	return 1;
}

int fpga_init_fn(int cookie)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	if (in_be32(&gpio->simple_ival) & FPGA_CONFIG)
		return 0;

	return 1;
}

int fpga_done_fn(int cookie)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	int result = 0;

	udelay(10);
	if (in_be32(&gpio->simple_ival) & FPGA_DONE)
		result = 1;

	return result;
}

int fpga_pgm_fn(int assert, int flush, int cookie)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	if (!assert)
		setbits_8(&gpio->sint_dvo, FPGA_STATUS);
	else
		clrbits_8(&gpio->sint_dvo, FPGA_STATUS);

	return assert;
}

int fpga_clk_fn(int assert_clk, int flush, int cookie)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	if (assert_clk)
		setbits_be32(&gpio->simple_dvo, FPGA_CCLK);
	else
		clrbits_be32(&gpio->simple_dvo, FPGA_CCLK);

	return assert_clk;
}

int fpga_wr_fn(int assert_write, int flush, int cookie)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	if (assert_write)
		setbits_be32(&gpio->simple_dvo, FPGA_DIN);
	else
		clrbits_be32(&gpio->simple_dvo, FPGA_DIN);

	return assert_write;
}

int fpga_pre_config_fn(int cookie)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	setbits_8(&gpio->sint_dvo, FPGA_STATUS);

	return 0;
}
