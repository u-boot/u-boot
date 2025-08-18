/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

void socfpga_init_smmu(void);

#define SMMU_SET_STREAMID(x, r, w)	(((x) << (r)) | ((x) << (w)))

#define SYSMGR_EMAC0_SID_ADDR	0xffd12050	/* EMAC0 (emac0_ace) */
#define SYSMGR_EMAC1_SID_ADDR	0xffd12054	/* EMAC0 (emac1_ace) */
#define SYSMGR_EMAC2_SID_ADDR	0xffd12058	/* EMAC0 (emac2_ace) */
#define SYSMGR_NAND_SID_ADDR	0xffd1205c	/* NAND (nand_axuser) */
#define SYSMGR_SDMMC_SID_ADDR	0xffd1202c	/* SDMMC (sdmmcgrp_l3master) */
#define SYSMGR_USB0_SID_ADDR	0xffd12038	/* USB0 (usb0_l3master) */
#define SYSMGR_USB1_SID_ADDR	0xffd1203c	/* USB0 (usb1_l3master) */
#define SYSMGR_DMA_SID_ADDR	0xffd12074	/* DMA (dma_l3master) */
#define SYSMGR_ETR_SID_ADDR	0xffd12078	/* ETR (etr_l3master) */

/* Stream ID field offsets */
#define EMAC_W_OFST	20
#define EMAC_R_OFST	8
#define NAND_W_OFST	0
#define NAND_R_OFST	16
#define SDMMC_OFST	16
#define USB_OFST	16
#define DMA_W_OFST	0
#define DMA_R_OFST	16
#define ETR_W_OFST	0
#define ETR_R_OFST	16

struct smmu_stream_id {
	unsigned long addr;
	u32 sid;
	u32 r_bit_ofst;
	u32 w_bit_ofst;
	u32 secure_bit_offset;
};
