/*
 * Copyright 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARMV8_FSL_LAYERSCAPE_FDT_H_
#define _ASM_ARMV8_FSL_LAYERSCAPE_FDT_H_

void alloc_stream_ids(int start_id, int count, u32 *stream_ids, int max_cnt);
void append_mmu_masters(void *blob, const char *smmu_path,
			const char *master_name, u32 *stream_ids, int count);
void fdt_fixup_smmu_pcie(void *blob);
void fdt_fixup_board_enet(void *fdt);
#endif	/* _ASM_ARMV8_FSL_LAYERSCAPE_FDT_H_ */
