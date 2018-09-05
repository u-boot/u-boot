// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <environment.h>
#include <asm/io.h>
#include <asm/arch/axg.h>
#include <asm/arch/sm.h>
#include <asm/arch/eth.h>
#include <asm/arch/mem.h>

int board_init(void)
{
	return 0;
}

int misc_init_r(void)
{
	meson_eth_init(PHY_INTERFACE_MODE_RGMII, 0);

	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	meson_init_reserved_memory(blob);

	return 0;
}
