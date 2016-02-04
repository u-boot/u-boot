/*
 * (C) Copyright 2012 Xilinx
 * (C) Copyright 2014 Digilent Inc.
 *
 * Configuration for Zynq Development Board - ZYBO
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_ZYBO_H
#define __CONFIG_ZYNQ_ZYBO_H

#define CONFIG_SYS_NO_FLASH

#define CONFIG_ZYNQ_USB

/* Define ZYBO PS Clock Frequency to 50MHz */
#define CONFIG_ZYNQ_PS_CLK_FREQ	50000000UL

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_ZYBO_H */
