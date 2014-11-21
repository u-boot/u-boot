/*
 * Copyright 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ls102xa_stream_id.h>

void ls102xa_config_smmu_stream_id(struct smmu_stream_id *id, uint32_t num)
{
	uint32_t *scfg = (uint32_t *)CONFIG_SYS_FSL_SCFG_ADDR;
	int i;

	for (i = 0; i < num; i++)
		out_be32(scfg + id[i].offset, id[i].stream_id);
}
