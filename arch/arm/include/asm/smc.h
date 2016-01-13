/*
 * Copyright (C) 2015 - Xilinx, Inc., Michal Simek
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARM_SMC_H__
#define __ARM_SMC_H__

extern int invoke_smc(u64 function_id, u64 arg0, u64 arg1, u64 arg2);

#endif /* __ARM_SMC_H__ */
