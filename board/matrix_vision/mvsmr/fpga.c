/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
 *
 * (C) Copyright 2010
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
