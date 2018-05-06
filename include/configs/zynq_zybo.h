/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Xilinx
 * (C) Copyright 2014 Digilent Inc.
 *
 * Configuration for Zynq Development Board - ZYBO
 * See zynq-common.h for Zynq common configs
 */

#ifndef __CONFIG_ZYNQ_ZYBO_H
#define __CONFIG_ZYNQ_ZYBO_H

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_ZYNQ_GEM_EEPROM_ADDR	0x50
#define CONFIG_DISPLAY
#define CONFIG_I2C_EDID

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_ZYBO_H */
