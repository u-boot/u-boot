/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/arch/tegra.h>

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

__weak int tegra_soc_board_init_late(void)
{
	return 0;
}

int board_late_init(void)
{
	return tegra_soc_board_init_late();
}
