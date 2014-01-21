/*
 * (C) Copyright 2013 Xilinx, Inc.
 *
 * Configuration for Micro Zynq Evaluation and Development Board - MicroZedBoard
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_MICROZED_H
#define __CONFIG_ZYNQ_MICROZED_H

#define CONFIG_SYS_SDRAM_SIZE		(1024 * 1024 * 1024)

#define CONFIG_ZYNQ_SERIAL_UART1
#define CONFIG_ZYNQ_GEM0
#define CONFIG_ZYNQ_GEM_PHY_ADDR0	0

#define CONFIG_SYS_NO_FLASH

#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_DEFAULT_DEVICE_TREE	zynq-microzed

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_MICROZED_H */
