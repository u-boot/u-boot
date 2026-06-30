/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SMC / PSCI-visible RSU state (secure RAM). Keep separate from rsu_s10.h so
 * normal-world RSU code does not declare secure-only symbols.
 */
#ifndef __RSU_SMC_H__
#define __RSU_SMC_H__

#include <asm/types.h>
#include <asm/secure.h>

extern u32 smc_rsu_update_address __secure_data;

#endif
