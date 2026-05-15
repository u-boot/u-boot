// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 */

#include <exports.h>
#include <fsl-mc/fsl_mc.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
	mc_env_boot();
}
#endif /* CONFIG_RESET_PHY_R */
