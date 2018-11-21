// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <asm/arch/tegra.h>

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
