/*
 *  Copyright (C) 2012 Lucas Stach
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch-tegra/mmc.h>

#include "../colibri_t20-common/colibri_t20-common.h"

#ifdef CONFIG_USB_EHCI_TEGRA
void pin_mux_usb(void)
{
	colibri_t20_common_pin_mux_usb();

	/* USB 1 aka Tegra USB port 3 VBus*/
	pinmux_tristate_disable(PINGRP_SPIG);
}
#endif

#ifdef CONFIG_TEGRA_MMC
int board_mmc_init(bd_t *bd)
{
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_4_BIT);
	pinmux_tristate_disable(PINGRP_GMB);

	tegra_mmc_init(0, 4, -1, GPIO_PC7);

	return 0;
}
#endif
