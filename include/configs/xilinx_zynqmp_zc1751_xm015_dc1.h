/*
 * Configuration for Xilinx ZynqMP zc1751 XM015 DC1
 *
 * (C) Copyright 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_ZC1751_XM015_DC1_H
#define __CONFIG_ZYNQMP_ZC1751_XM015_DC1_H

#define CONFIG_ZYNQ_GEM3
#define CONFIG_ZYNQ_GEM_PHY_ADDR3	-1

#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_ZYNQ_I2C1
#define CONFIG_SYS_I2C_ZYNQ
#define CONFIG_AHCI

#define CONFIG_IDENT_STRING	" Xilinx ZynqMP ZC1751 xm015 dc1"

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZC1751_XM015_DC1_H */
