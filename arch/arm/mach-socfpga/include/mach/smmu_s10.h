/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

/* SMMU registers */
#define SMMU_SCR0			0
#define SMMU_SIDR0			0x20
#define SMMU_SIDR1			0x24

#define SMMU_SCR0_CLIENTPD		BIT(0)
#define SMMU_SIDR0_NUMSMRG_MASK		GENMASK(7, 0)
#define SMMU_SIDR1_NUMCB_MASK		GENMASK(7, 0)

/* Stream mapping registers */
#define SMMU_GR0_SMR(n)			(0x800 + ((n) << 2))
#define SMMU_SMR_VALID			BIT(31)
#define SMMU_SMR_MASK			GENMASK(30, 16)
#define SMMU_SMR_ID			GENMASK(14, 0)

#define SMMU_GR0_S2CR(n)		(0xc00 + ((n) << 2))
#define SMMU_S2CR_TYPE			GENMASK(17, 16)
#define SMMU_S2CR_CBNDX			GENMASK(7, 0)

/* Register groups for Context Bank */
#define SMMU_GR0_CB(n, r)		(0x20000 + ((n) << 12) + ((r) << 2))
#define SMMU_CB_SCTLR			0
#define SMMU_CB_SCTLR_M			BIT(0)

#define SMMU_SID_SDM2HPS_PSI_BE		0

#define SDM2HPS_PSI_BE_ADDR_BASE	0
/* PSI BE 512MB address window */
#define SDM2HPS_PSI_BE_WINDOW_SZ	0x20000000
#define SDM2HPS_PSI_BE_ADDR_END		\
	(SDM2HPS_PSI_BE_ADDR_BASE + SDM2HPS_PSI_BE_WINDOW_SZ - 1)

void socfpga_init_smmu(void);
int is_smmu_bypass(void);
int is_smmu_stream_id_enabled(u32 stream_id);

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
