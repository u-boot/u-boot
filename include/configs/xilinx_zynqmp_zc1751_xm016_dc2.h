/*
 * Configuration for Xilinx ZynqMP zc1751 XM016 DC2
 *
 * (C) Copyright 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_ZC1751_XM016_DC2_H
#define __CONFIG_ZYNQMP_ZC1751_XM016_DC2_H

#define CONFIG_ZYNQMP_XHCI_LIST {ZYNQMP_USB1_XHCI_BASEADDR}

#define CONFIG_IDENT_STRING	" Xilinx ZynqMP ZC1751 xm016 dc2"

#define CONFIG_KERNEL_FDT_OFST_SIZE \
	"kernel_offset=0x400000\0" \
	"fdt_offset=0x2400000\0" \
	"kernel_size=0x2000000\0" \
	"fdt_size=0x80000\0" \
	"board=zc1751-dc2\0"

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZC1751_XM016_DC2_H */
