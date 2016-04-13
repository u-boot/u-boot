/*
 * Configuration for Xilinx ZynqMP zc1751 XM019 DC5
 *
 * (C) Copyright 2015 Xilinx, Inc.
 * Siva Durga Prasad <siva.durga.paladugu@xilinx.com>
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_ZC1751_XM019_DC5_H
#define __CONFIG_ZYNQMP_ZC1751_XM019_DC5_H

#define CONFIG_ZYNQ_SDHCI0

#define CONFIG_IDENT_STRING	" Xilinx ZynqMP ZC1751 xm019 dc5"

#define CONFIG_KERNEL_FDT_OFST_SIZE \
	"kernel_offset=0x400000\0" \
	"fdt_offset=0x2400000\0" \
	"kernel_size=0x2000000\0" \
	"fdt_size=0x80000\0" \
	"board=zc1751-dc5\0"

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZC1751_XM019_DC5_H */
