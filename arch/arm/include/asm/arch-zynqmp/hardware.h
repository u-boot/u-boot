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

#define ZYNQ_GEM_BASEADDR0	0xFF0B0000
#define ZYNQ_GEM_BASEADDR1	0xFF0C0000
#define ZYNQ_GEM_BASEADDR2	0xFF0D0000
#define ZYNQ_GEM_BASEADDR3	0xFF0E0000

#define ZYNQ_TTC_BASEADDR0	0xFF110000

#define ZYNQ_QSPI_BASEADDR	0xFF0F0000

#define ZYNQ_SDHCI_BASEADDR0	0xFF160000
#define ZYNQ_SDHCI_BASEADDR1	0xFF170000

#define ZYNQMP_CRL_APB_BASEADDR	0xFF5E0000

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

#define ZYNQMP_CSU_BASEADDR	0xFFCA0000

struct csu_regs {
	u32 reserved0[16];
	u32 version; /* 0x44 */
};

#define csu_base ((struct csu_regs *)ZYNQMP_CSU_BASEADDR)

/* Board version value */
#define ZYNQMP_CSU_VERSION_SILICON	0x0
#define ZYNQMP_CSU_VERSION_EP108	0x1
#define ZYNQMP_CSU_VERSION_VELOCE	0x2
#define ZYNQMP_CSU_VERSION_QEMU		0x3

#endif /* _ASM_ARCH_HARDWARE_H */
