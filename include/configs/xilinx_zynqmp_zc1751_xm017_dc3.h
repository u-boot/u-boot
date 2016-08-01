/*
 * Configuration for Xilinx ZynqMP zc1751 XM017 DC3
 *
 * (C) Copyright 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_ZC1751_XM017_DC3_H
#define __CONFIG_ZYNQMP_ZC1751_XM017_DC3_H

#define CONFIG_ZYNQ_SDHCI1
#define CONFIG_ZYNQ_I2C0
#define CONFIG_ZYNQ_I2C1

#define CONFIG_ZYNQMP_XHCI_LIST {ZYNQMP_USB0_XHCI_BASEADDR, ZYNQMP_USB1_XHCI_BASEADDR}

#define CONFIG_AHCI
#define CONFIG_SATA_CEVA

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_ZC1751_XM017_DC3_H */
