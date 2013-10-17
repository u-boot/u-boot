/*
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/gpio.h>
#include <fpga.h>
#include <lattice.h>
#include "qong_fpga.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA)

static void qong_jtag_init(void)
{
	return;
}

static void qong_fpga_jtag_set_tdi(int value)
{
	gpio_set_value(QONG_FPGA_TDI_PIN, value);
}

static void qong_fpga_jtag_set_tms(int value)
{
	gpio_set_value(QONG_FPGA_TMS_PIN, value);
}

static void qong_fpga_jtag_set_tck(int value)
{
	gpio_set_value(QONG_FPGA_TCK_PIN, value);
}

static int qong_fpga_jtag_get_tdo(void)
{
	return gpio_get_value(QONG_FPGA_TDO_PIN);
}

lattice_board_specific_func qong_fpga_fns = {
	qong_jtag_init,
	qong_fpga_jtag_set_tdi,
	qong_fpga_jtag_set_tms,
	qong_fpga_jtag_set_tck,
	qong_fpga_jtag_get_tdo
};

Lattice_desc qong_fpga[CONFIG_FPGA_COUNT] = {
	{
		Lattice_XP2,
		lattice_jtag_mode,
		356519,
		(void *) &qong_fpga_fns,
		NULL,
		0,
		"lfxp2_5e_ftbga256"
	},
};

int qong_fpga_init(void)
{
	int i;

	fpga_init();

	for (i = 0; i < CONFIG_FPGA_COUNT; i++) {
		fpga_add(fpga_lattice, &qong_fpga[i]);
	}
	return 0;
}

#endif
