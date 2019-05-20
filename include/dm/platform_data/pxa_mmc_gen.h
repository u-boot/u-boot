/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 Marcel Ziswiler <marcel.ziswiler@toradex.com>
 */

#ifndef __PXA_MMC_GEN_H
#define __PXA_MMC_GEN_H

#include <mmc.h>

/*
 * struct pxa_mmc_platdata - information about a PXA MMC controller
 *
 * @base:	MMC controller base register address
 */
struct pxa_mmc_plat {
	struct mmc_config	cfg;
	struct mmc		mmc;
	struct pxa_mmc_regs	*base;
};

#endif /* __PXA_MMC_GEN_H */
