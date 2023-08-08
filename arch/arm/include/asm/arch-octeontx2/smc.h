/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __SMC_H__
#define __SMC_H__

#include <asm/arch/smc-id.h>

ssize_t smc_configure_ooo(unsigned int val);
ssize_t smc_dram_size(unsigned int node);
ssize_t smc_disable_rvu_lfs(unsigned int node);
ssize_t smc_flsf_fw_booted(void);
ssize_t smc_flsf_clr_force_2ndry(void);
ssize_t smc_mdio_dbg_read(int cgx_lmac, int mode, int phyaddr, int devad,
			  int reg);
ssize_t smc_mdio_dbg_write(int cgx_lmac, int mode, int phyaddr, int devad,
			   int reg, int val);
#ifdef CONFIG_CMD_ATTEST
ssize_t smc_attest(long subcmd, long ctx_arg);
#endif

#endif
