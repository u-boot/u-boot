/*
 *  Copyright (C) 2012 Lucas Stach
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-tegra/board.h>

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
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_4_BIT);
	pinmux_tristate_disable(PINGRP_GMB);
}
#endif
