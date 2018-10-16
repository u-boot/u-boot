/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 - 2018 Xilinx, Inc.
 */

#define VERSAL_CRL_APB_BASEADDR	0xFF5E0000

#define CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT	BIT(25)

#define IOU_SWITCH_CTRL_CLKACT_BIT	BIT(25)
#define IOU_SWITCH_CTRL_DIVISOR0_SHIFT	8

struct crlapb_regs {
	u32 reserved0[69];
	u32 iou_switch_ctrl; /* 0x114 */
	u32 reserved1[13];
	u32 timestamp_ref_ctrl; /* 0x14c */
	u32 reserved2[126];
	u32 rst_timestamp; /* 0x348 */
};

#define crlapb_base ((struct crlapb_regs *)VERSAL_CRL_APB_BASEADDR)

#define VERSAL_IOU_SCNTR_SECURE	0xFF140000

#define IOU_SCNTRS_CONTROL_EN	1

struct iou_scntrs_regs {
	u32 counter_control_register; /* 0x0 */
	u32 reserved0[7];
	u32 base_frequency_id_register; /* 0x20 */
};

#define iou_scntr_secure ((struct iou_scntrs_regs *)VERSAL_IOU_SCNTR_SECURE)
