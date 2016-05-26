/*
 * Configuration for Xilinx ZynqMP zc1751 XM018 DC4
 *
 * (C) Copyright 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_ZC1751_XM018_DC4_H
#define __CONFIG_ZYNQMP_ZC1751_XM018_DC4_H

#define CONFIG_IDENT_STRING	" Xilinx ZynqMP ZC1751 xm018 dc4"

#define CONFIG_KERNEL_FDT_OFST_SIZE \
	"kernel_offset=0x400000\0" \
	"fdt_offset=0x2400000\0" \
	"kernel_size=0x2000000\0" \
	"fdt_size=0x80000\0" \
	"board=zc1751-dc4\0"

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZC1751_XM018_DC4_H */
