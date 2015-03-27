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
#include <asm/gpio.h>

#ifdef CONFIG_TEGRA_MMC
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_4_BIT);
	pinmux_tristate_disable(PMUX_PINGRP_GMB);
}
#endif

#ifdef CONFIG_TEGRA_NAND
void pin_mux_nand(void)
{
	funcmux_select(PERIPH_ID_NDFLASH, FUNCMUX_NDFLASH_KBC_8_BIT);

	/*
	 * configure pingroup ATC to something unrelated to
	 * avoid ATC overriding KBC
	 */
	pinmux_set_func(PMUX_PINGRP_ATC, PMUX_FUNC_GMI);
}
#endif

#ifdef CONFIG_USB_EHCI_TEGRA
void pin_mux_usb(void)
{
	/* module internal USB bus to connect ethernet chipset */
	funcmux_select(PERIPH_ID_USB2, FUNCMUX_USB2_ULPI);

	/* ULPI reference clock output */
	pinmux_set_func(PMUX_PINGRP_CDEV2, PMUX_FUNC_PLLP_OUT4);
	pinmux_tristate_disable(PMUX_PINGRP_CDEV2);

	/* PHY reset GPIO */
	pinmux_tristate_disable(PMUX_PINGRP_UAC);

	/* VBus GPIO */
	pinmux_tristate_disable(PMUX_PINGRP_DTE);

	/* Reset ASIX using LAN_RESET */
	gpio_request(GPIO_PV4, "LAN_RESET");
	gpio_direction_output(GPIO_PV4, 0);
	pinmux_tristate_disable(PMUX_PINGRP_GPV);
	udelay(5);
	gpio_set_value(GPIO_PV4, 1);

	/* USBH_PEN: USB 1 aka Tegra USB port 3 VBus */
	pinmux_tristate_disable(PMUX_PINGRP_SPIG);
}
#endif
