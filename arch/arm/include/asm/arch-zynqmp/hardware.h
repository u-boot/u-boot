/*
 * (C) Copyright 2014 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H

#define ZYNQ_SERIAL_BASEADDR0	0xFF000000
#define ZYNQ_SERIAL_BASEADDR1	0xFF001000

#define ZYNQ_GEM_BASEADDR0	0xFF009000
#define ZYNQ_GEM_BASEADDR1	0xFF00A000
#define ZYNQ_GEM_BASEADDR2	0xFF00B000
#define ZYNQ_GEM_BASEADDR3	0xFF00C000

#define ZYNQ_TTC_BASEADDR0	0xFF00F000

#define ZYNQ_QSPI_BASEADDR	0xFF00D000

#define ZYNQ_SDHCI_BASEADDR0	0xFF014000
#define ZYNQ_SDHCI_BASEADDR1	0xFF015000

#define ZYNQMP_CRL_APB_BASEADDR	0xFF400000

struct crlapb_regs {
	u32 reserved0[128];
	u32 boot_mode;
	u32 reserved1[26];
};

#define crlapb_base ((struct crlapb_regs *)ZYNQMP_CRL_APB_BASEADDR)

/* Bootmode setting values */
#define BOOT_MODES_MASK	0x0000000F
#define QSPI_MODE	0x00000001
#define SD_MODE		0x00000005
#define JTAG_MODE	0x00000000

#endif /* _ASM_ARCH_HARDWARE_H */
