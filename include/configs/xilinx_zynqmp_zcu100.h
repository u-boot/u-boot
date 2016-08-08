/*
 * Configuration for Xilinx ZynqMP zcu100
 *
 * (C) Copyright 2015 - 2016 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_ZCU100_H
#define __CONFIG_ZYNQMP_ZCU100_H

/* FIXME Will go away soon */
#define CONFIG_ZYNQ_I2C0
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_NUM_I2C_BUSES	9
#define CONFIG_SYS_I2C_BUSES	{ \
				{0, {I2C_NULL_HOP} }, \
				{0, {{I2C_MUX_PCA9548, 0x75, 0} } }, \
				{0, {{I2C_MUX_PCA9548, 0x75, 1} } }, \
				{0, {{I2C_MUX_PCA9548, 0x75, 2} } }, \
				{0, {{I2C_MUX_PCA9548, 0x75, 3} } }, \
				{0, {{I2C_MUX_PCA9548, 0x75, 4} } }, \
				{0, {{I2C_MUX_PCA9548, 0x75, 5} } }, \
				{0, {{I2C_MUX_PCA9548, 0x75, 6} } }, \
				{0, {{I2C_MUX_PCA9548, 0x75, 7} } }, \
				}


/* #define CONFIG_ZYNQ_I2C1 */ /* FIXME for 96 compatible bitstream */
#define CONFIG_SYS_I2C_ZYNQ

#define CONFIG_ZYNQMP_XHCI_LIST {ZYNQMP_USB0_XHCI_BASEADDR, \
				 ZYNQMP_USB1_XHCI_BASEADDR}

#define CONFIG_IDENT_STRING	" Xilinx ZynqMP ZCU100"

#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX

/* #define CONFIG_MMC_TRACE */
/* #define CONFIG_ZYNQ_SDHCI_MAX_FREQ 15000000 */

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZCU100_H */
