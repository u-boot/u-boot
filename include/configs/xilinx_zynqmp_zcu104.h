/*
 * Configuration for Xilinx ZynqMP zcu104
 *
 * (C) Copyright 2017 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_ZCU104_H
#define __CONFIG_ZYNQMP_ZCU104_H

#define CONFIG_ZYNQ_SDHCI1
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_NUM_I2C_BUSES	9
#define CONFIG_SYS_I2C_BUSES	{ \
				{0, {I2C_NULL_HOP} }, \
				{0, {{I2C_MUX_PCA9548, 0x74, 0} } }, \
				{0, {{I2C_MUX_PCA9548, 0x74, 1} } }, \
				{0, {{I2C_MUX_PCA9548, 0x74, 2} } }, \
				{0, {{I2C_MUX_PCA9548, 0x74, 3} } }, \
				{0, {{I2C_MUX_PCA9548, 0x74, 4} } }, \
				{0, {{I2C_MUX_PCA9548, 0x74, 5} } }, \
				{0, {{I2C_MUX_PCA9548, 0x74, 6} } }, \
				{0, {{I2C_MUX_PCA9548, 0x74, 7} } }, \
				}

#define CONFIG_SYS_I2C_ZYNQ
#define CONFIG_PCA953X

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZCU104_H */
