/*
 * Configuration for Xilinx ZynqMP zcu102
 *
 * (C) Copyright 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_ZCU102_H
#define __CONFIG_ZYNQMP_ZCU102_H

#define CONFIG_ZYNQ_GEM3
#define CONFIG_ZYNQ_GEM_PHY_ADDR3	-1
#define CONFIG_ZYNQ_GEM_INTERFACE	PHY_INTERFACE_MODE_RGMII_ID

#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_ZYNQ_SDHCI1
#define CONFIG_ZYNQ_I2C0
#define CONFIG_ZYNQ_I2C1
#define CONFIG_SYS_I2C_ZYNQ
#define CONFIG_AHCI

#define CONFIG_IDENT_STRING	" Xilinx ZynqMP ZCU102"

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		0x80000000

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZCU102_H */
