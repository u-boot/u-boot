/*
 * (C) Copyright 2014 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

#define ZYNQ_SILICON_VER_MASK	0xF000
#define ZYNQ_SILICON_VER_SHIFT	12

unsigned int zynqmp_get_silicon_version(void)
{
	unsigned int ver;

	ver = (readl(&csu_base->version) & ZYNQ_SILICON_VER_MASK) >>
	       ZYNQ_SILICON_VER_SHIFT;

	return ver;
}
