// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <asm/arch-fsl-layerscape/immap_lsch2.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>

struct icid_id_table icid_tbl[] = {
#ifdef CONFIG_SYS_DPAA_QBMAN
	SET_QMAN_ICID(FSL_DPAA1_STREAM_ID_START),
	SET_BMAN_ICID(FSL_DPAA1_STREAM_ID_START + 1),
#endif

	SET_SDHC_ICID(FSL_SDHC_STREAM_ID),

	SET_USB_ICID(1, "snps,dwc3", FSL_USB1_STREAM_ID),
	SET_USB_ICID(2, "snps,dwc3", FSL_USB2_STREAM_ID),
	SET_USB_ICID(3, "snps,dwc3", FSL_USB3_STREAM_ID),

	SET_SATA_ICID("fsl,ls1046a-ahci", FSL_SATA_STREAM_ID),
	SET_QDMA_ICID("fsl,ls1046a-qdma", FSL_QDMA_STREAM_ID),
	SET_EDMA_ICID(FSL_EDMA_STREAM_ID),
	SET_ETR_ICID(FSL_ETR_STREAM_ID),
	SET_DEBUG_ICID(FSL_DEBUG_STREAM_ID),
};

int icid_tbl_sz = ARRAY_SIZE(icid_tbl);
