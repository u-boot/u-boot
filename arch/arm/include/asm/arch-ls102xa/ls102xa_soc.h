/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_LS102XA_SOC_H
#define __FSL_LS102XA_SOC_H

unsigned int get_soc_major_rev(void);
int arch_soc_init(void);
int ls102xa_smmu_stream_id_init(void);

#endif /* __FSL_LS102XA_SOC_H */
