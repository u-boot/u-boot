/*
 *  Copyright (C) 2012 Lucas Stach
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-tegra/board.h>

#include "colibri_t20-common.h"

#ifdef CONFIG_USB_EHCI_TEGRA
void colibri_t20_common_pin_mux_usb(void)
{
	/* module internal USB bus to connect ethernet chipset */
	funcmux_select(PERIPH_ID_USB2, FUNCMUX_USB2_ULPI);
	/* ULPI reference clock output */
	pinmux_set_func(PINGRP_CDEV2, PMUX_FUNC_PLLP_OUT4);
	pinmux_tristate_disable(PINGRP_CDEV2);
	/* PHY reset GPIO */
	pinmux_tristate_disable(PINGRP_UAC);
	/* VBus GPIO */
	pinmux_tristate_disable(PINGRP_DTE);
}
#endif

#ifdef CONFIG_TEGRA_NAND
void pin_mux_nand(void)
{
	funcmux_select(PERIPH_ID_NDFLASH, FUNCMUX_NDFLASH_KBC_8_BIT);
}
#endif
