/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define ZYNQMP_TCM_BASE_ADDR	0xFFE00000
#define ZYNQMP_TCM_SIZE		0x40000

#define ZYNQMP_CRL_APB_BASEADDR	0xFF5E0000
#define ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT	0x1000000
#define ZYNQMP_CRL_APB_BOOT_PIN_CTRL_OUT_EN_SHIFT	0
#define ZYNQMP_CRL_APB_BOOT_PIN_CTRL_OUT_VAL_SHIFT	8

#define ZYNQMP_AMS_PS_SYSMON_BASEADDR      0XFFA50800
#define ZYNQMP_AMS_PS_SYSMON_ANALOG_BUS ((ZYNQMP_AMS_PS_SYSMON_BASEADDR) \
							    + 0x00000114)
#define ZYNQMP_PS_SYSMON_ANALOG_BUS_VAL 0x00003210

#define ADMA_CH0_BASEADDR	0xFFA80000

#define PS_MODE0	BIT(0)
#define PS_MODE1	BIT(1)
#define PS_MODE2	BIT(2)
#define PS_MODE3	BIT(3)

#define RESET_REASON_DEBUG_SYS	BIT(6)
#define RESET_REASON_SOFT	BIT(5)
#define RESET_REASON_SRST	BIT(4)
#define RESET_REASON_PSONLY	BIT(3)
#define RESET_REASON_PMU	BIT(2)
#define RESET_REASON_INTERNAL	BIT(1)
#define RESET_REASON_EXTERNAL	BIT(0)

#define CRLAPB_DBG_LPD_CTRL_SETUP_CLK	0x01002002
#define CRLAPB_RST_LPD_DBG_RESET	0

#define CRL_APB_SOFT_RESET_CTRL_MASK	0x10

struct crlapb_regs {
	u32 reserved0[36];
	u32 cpu_r5_ctrl; /* 0x90 */
	u32 reserved1[7];
	u32 dbg_lpd_ctrl; /* 0xB0 */
	u32 reserved2[29];
	u32 timestamp_ref_ctrl; /* 0x128 */
	u32 reserved3[53];
	u32 boot_mode; /* 0x200 */
	u32 reserved4_0[5];
	u32 soft_reset; /* 0x218 */
	u32 reserved4_10;
	u32 reset_reason; /* 0x220 */
	u32 reserved4_1[6];
	u32 rst_lpd_top; /* 0x23C */
	u32 rst_lpd_dbg; /* 0x240 */
	u32 reserved5[3];
	u32 boot_pin_ctrl; /* 0x250 */
	u32 reserved6[21];
};

#define crlapb_base ((struct crlapb_regs *)ZYNQMP_CRL_APB_BASEADDR)

#define ZYNQMP_IOU_SECURE_SLCR 0xFF240000

#define ZYNQMP_IOU_SCNTR_SECURE	0xFF260000
#define ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN	0x1
#define ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_HDBG	0x2

struct iou_scntr_secure {
	u32 counter_control_register;
	u32 reserved0[7];
	u32 base_frequency_id_register;
};

#define iou_scntr_secure ((struct iou_scntr_secure *)ZYNQMP_IOU_SCNTR_SECURE)

#define ZYNQMP_PS_VERSION	0xFFCA0044
#define ZYNQMP_PS_VER_MASK	GENMASK(1, 0)

/* Bootmode setting values */
#define BOOT_MODES_MASK	0x0000000F
#define QSPI_MODE_24BIT	0x00000001
#define QSPI_MODE_32BIT	0x00000002
#define SD_MODE		0x00000003 /* sd 0 */
#define SD_MODE1	0x00000005 /* sd 1 */
#define NAND_MODE	0x00000004
#define EMMC_MODE	0x00000006
#define USB_MODE	0x00000007
#define SD1_LSHFT_MODE	0x0000000E /* SD1 Level shifter */
#define JTAG_MODE	0x00000000
#define BOOT_MODE_USE_ALT	0x100
#define BOOT_MODE_ALT_SHIFT	12
/* SW secondary boot modes 0xa - 0xd */
#define SW_USBHOST_MODE	0x0000000A
#define SW_SATA_MODE	0x0000000B

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

#define ZYNQMP_CCI_REG_CCI_MISC_CTRL	0xFD5E0040
#define ZYNQMP_CCI_REG_CCI_MISC_CTRL_NIDEN	BIT(1)

#define ZYNQMP_APU_BASEADDR	0xFD5C0000

struct apu_regs {
	u32 reserved0[16];
	u32 rvbar_addr0_l; /* 0x40 */
	u32 rvbar_addr0_h; /* 0x44 */
	u32 reserved1[20];
};

#define apu_base ((struct apu_regs *)ZYNQMP_APU_BASEADDR)

/* Board version value */
#define ZYNQMP_CSU_BASEADDR		0xFFCA0000
#define ZYNQMP_CSU_VERSION_SILICON	0x0
#define ZYNQMP_CSU_VERSION_QEMU		0x3

#define ZYNQMP_CSU_VERSION_EMPTY_SHIFT		20

#define ZYNQMP_SILICON_VER_MASK		0xF
#define ZYNQMP_SILICON_VER_SHIFT	0

#define CSU_JTAG_SEC_GATE_DISABLE	GENMASK(7, 0)
#define CSU_JTAG_DAP_ENABLE_DEBUG	GENMASK(7, 0)
#define CSU_JTAG_CHAIN_WR_SETUP		GENMASK(1, 0)
#define CSU_PCAP_PROG_RELEASE_PL	BIT(0)

#define ZYNQMP_CSU_STATUS_AUTHENTICATED	BIT(0)
#define ZYNQMP_CSU_STATUS_ENCRYPTED	BIT(1)

struct csu_regs {
	u32 status;
	u32 reserved0[3];
	u32 multi_boot;
	u32 reserved1[7];
	u32 jtag_chain_status_wr;
	u32 jtag_chain_status;
	u32 jtag_sec;
	u32 jtag_dap_cfg;
	u32 idcode;
	u32 version;
	u32 reserved2[3054];
	u32 pcap_prog;
};

#define csu_base ((struct csu_regs *)ZYNQMP_CSU_BASEADDR)

#define ZYNQMP_PMU_BASEADDR	0xFFD80000

struct pmu_regs {
	u32 reserved0[16];
	u32 gen_storage4; /* 0x40 */
	u32 reserved1[1];
	u32 gen_storage6; /* 0x48 */
	u32 reserved2[3];
	u32 pers_gen_storage2; /* 0x58 */
};

#define pmu_base ((struct pmu_regs *)ZYNQMP_PMU_BASEADDR)

#endif /* _ASM_ARCH_HARDWARE_H */
