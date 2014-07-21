/*
 * (C) Copyright 2014 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

unsigned long get_uart_clk(int dev_id)
{
	u32 ver = zynqmp_get_silicon_version();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		return 400000;
	case ZYNQMP_CSU_VERSION_EP108:
		return 25000000;
	}

	return 133000000;
}

unsigned long get_ttc_clk(int dev_id)
{
	return get_uart_clk(dev_id);
}
