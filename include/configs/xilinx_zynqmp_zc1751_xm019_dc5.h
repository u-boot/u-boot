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

#define CONFIG_ZYNQ_GEM1
#define CONFIG_ZYNQ_GEM_PHY_ADDR1	-1

#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_SERIAL_UART1
#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_ZYNQ_I2C0
#define CONFIG_ZYNQ_I2C1
#define CONFIG_SYS_I2C_ZYNQ

#define CONFIG_IDENT_STRING	" Xilinx ZynqMP ZC1751 xm019 dc5"

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		0x80000000

#define CONFIG_KERNEL_FDT_OFST_SIZE \
	"kernel_offset=0x400000\0" \
	"fdt_offset=0x2400000\0" \
	"kernel_size=0x2000000\0" \
	"fdt_size=0x80000\0"

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZC1751_XM019_DC5_H */
