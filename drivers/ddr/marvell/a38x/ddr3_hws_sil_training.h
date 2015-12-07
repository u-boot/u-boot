/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_HWS_SIL_TRAINING_H
#define _DDR3_HWS_SIL_TRAINING_H

#include "ddr3_training_ip.h"
#include "ddr3_training_ip_prv_if.h"

int ddr3_silicon_pre_config(void);
int ddr3_silicon_init(void);
int ddr3_silicon_get_ddr_target_freq(u32 *ddr_freq);

#endif /* _DDR3_HWS_SIL_TRAINING_H */
