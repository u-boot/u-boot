/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_LS102XA_STREAM_ID_H_
#define __FSL_LS102XA_STREAM_ID_H_

struct smmu_stream_id {
	uint16_t offset;
	uint16_t stream_id;
	char dev_name[32];
};

void ls102xa_config_smmu_stream_id(struct smmu_stream_id *id, uint32_t num);
#endif
