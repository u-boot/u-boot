/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#ifndef __ASM_AST2700_FMC_HDR_H__
#define __ASM_AST2700_FMC_HDR_H__

#include <linux/types.h>

#define HDR_MAGIC		0x48545341	/* ASTH */
#define HDR_PB_MAX		30

enum prebuilt_type {
	PBT_END_MARK = 0x0,

	PBT_DDR4_PMU_TRAIN_IMEM,
	PBT_DDR4_PMU_TRAIN_DMEM,
	PBT_DDR4_2D_PMU_TRAIN_IMEM,
	PBT_DDR4_2D_PMU_TRAIN_DMEM,
	PBT_DDR5_PMU_TRAIN_IMEM,
	PBT_DDR5_PMU_TRAIN_DMEM,
	PBT_DP_FW,
	PBT_UEFI_X64_AST2700,

	PBT_NUM
};

struct fmc_hdr_preamble {
	uint32_t magic;
	uint32_t version;
};

struct fmc_hdr_body {
	uint32_t fmc_size;
	union {
		struct {
			uint32_t type;
			uint32_t size;
		} pbs[0];
		uint32_t raz[29];
	};
};

struct fmc_hdr {
	struct fmc_hdr_preamble preamble;
	struct fmc_hdr_body body;
} __packed;

int fmc_hdr_get_prebuilt(uint32_t type, uint32_t *ofst, uint32_t *size);

#endif
