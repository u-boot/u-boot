/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
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

#define ZYNQ_SPI_BASEADDR0	0xFF040000
#define ZYNQ_SPI_BASEADDR1	0xFF050000

#define ZYNQMP_QSPI_BASEADDR	0xFF0F0000

#define ZYNQ_I2C_BASEADDR0	0xFF020000
#define ZYNQ_I2C_BASEADDR1	0xFF030000

#define ZYNQ_SDHCI_BASEADDR0	0xFF160000
#define ZYNQ_SDHCI_BASEADDR1	0xFF170000

#define ARASAN_NAND_BASEADDR	0xFF100000

#define ZYNQMP_SATA_BASEADDR	0xFD0C0000

#define ZYNQMP_USB0_XHCI_BASEADDR	0xFE200000
#define ZYNQMP_USB1_XHCI_BASEADDR	0xFE300000

#define ZYNQMP_CRL_APB_BASEADDR	0xFF5E0000
#define ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT	0x1000000

struct crlapb_regs {
	u32 reserved0[36];
	u32 cpu_r5_ctrl; /* 0x90 */
	u32 reserved1[37];
	u32 timestamp_ref_ctrl; /* 0x128 */
	u32 reserved2[53];
	u32 boot_mode; /* 0x200 */
	u32 reserved3[14];
	u32 rst_lpd_top; /* 0x23C */
	u32 reserved4[26];
};

#define crlapb_base ((struct crlapb_regs *)ZYNQMP_CRL_APB_BASEADDR)

#if defined(CONFIG_SECURE_IOU)
#define ZYNQMP_IOU_SCNTR	0xFF260000
#else
#define ZYNQMP_IOU_SCNTR	0xFF250000
#endif
#define ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN	0x1
#define ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_HDBG	0x2

struct iou_scntr {
	u32 counter_control_register;
	u32 reserved0[7];
	u32 base_frequency_id_register;
};

#define iou_scntr ((struct iou_scntr *)ZYNQMP_IOU_SCNTR)

/* Bootmode setting values */
#define BOOT_MODES_MASK	0x0000000F
#define QSPI_MODE_24BIT	0x00000001
#define QSPI_MODE_32BIT	0x00000002
#define SD_MODE		0x00000003 /* sd 0 */
#define SD_MODE1	0x00000005 /* sd 1 */
#define NAND_MODE	0x00000004
#define EMMC_MODE	0x00000006
#define JTAG_MODE	0x00000000

#define ZYNQMP_IOU_SLCR_BASEADDR	0xFF180000

struct iou_slcr_regs {
	u32 mio_pin[78];
	u32 reserved[442];
};

#define slcr_base ((struct iou_slcr_regs *)ZYNQMP_IOU_SLCR_BASEADDR)

#define ZYNQMP_RPU_BASEADDR	0xFF9A0000

struct rpu_regs {
	u32 rpu_glbl_ctrl;
	u32 reserved0[63];
	u32 rpu0_cfg; /* 0x100 */
	u32 reserved1[63];
	u32 rpu1_cfg; /* 0x200 */
};

#define rpu_base ((struct rpu_regs *)ZYNQMP_RPU_BASEADDR)

#define ZYNQMP_CRF_APB_BASEADDR	0xFD1A0000

struct crfapb_regs {
	u32 reserved0[65];
	u32 rst_fpd_apu; /* 0x104 */
	u32 reserved1;
};

#define crfapb_base ((struct crfapb_regs *)ZYNQMP_CRF_APB_BASEADDR)

#define ZYNQMP_APU_BASEADDR	0xFD5C0000

struct apu_regs {
	u32 reserved0[16];
	u32 rvbar_addr0_l; /* 0x40 */
	u32 rvbar_addr0_h; /* 0x44 */
	u32 reserved1[20];
};

#define apu_base ((struct apu_regs *)ZYNQMP_APU_BASEADDR)

/* Board version value */
#define ZYNQMP_CSU_VERSION_SILICON	0x0
#define ZYNQMP_CSU_VERSION_EP108	0x1
#define ZYNQMP_CSU_VERSION_VELOCE	0x2
#define ZYNQMP_CSU_VERSION_QEMU		0x3

#endif /* _ASM_ARCH_HARDWARE_H */
