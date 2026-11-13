/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 * DDR ECC operations for the ddr_ecc command.
 */

#ifndef __SOCFPGA_DDR_ECC_H__
#define __SOCFPGA_DDR_ECC_H__

#include <linux/types.h>

/**
 * struct ddr_ecc_ops - DDR ECC operations for the ddr_ecc command
 * @name: backend name (e.g. "io96b")
 * @status_io: drain the IO ECC queue; set *@dbe if a DBE was observed
 * @status_sdm: drain the SDM ECC queue; set *@dbe if a DBE was observed
 * @inject: inject SBE/DBE (backend uses instance 0 / interface 0)
 *
 * status_* return 0 / negative errno only; DBE is reported via @dbe.
 * Any unused hook may be NULL; the CLI checks before calling.
 */
struct ddr_ecc_ops {
	const char *name;
	int (*status_io)(bool *dbe);
	int (*status_sdm)(bool *dbe);
	int (*inject)(bool dbe);
};

/**
 * ddr_ecc_get_ops() - return the DDR ECC ops for the ddr_ecc command
 */
const struct ddr_ecc_ops *ddr_ecc_get_ops(void);

#endif /* __SOCFPGA_DDR_ECC_H__ */
