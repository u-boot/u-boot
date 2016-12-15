/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/mmc.h>
#include <asm/arch-tegra/tegra_mmc.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	return 0;
}

__weak int tegra_board_init(void)
{
	return 0;
}

int board_init(void)
{
	return tegra_board_init();
}

int board_late_init(void)
{
	return 0;
}

void pad_init_mmc(struct mmc_host *host)
{
}

int board_mmc_init(bd_t *bd)
{
	tegra_mmc_init();

	return 0;
}
