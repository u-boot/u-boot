/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/gpio.h>

#ifdef CONFIG_TEGRA_MMC
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
void pin_mux_mmc(void)
{
	funcmux_select(PERIPH_ID_SDMMC4, FUNCMUX_SDMMC4_ATB_GMA_GME_8_BIT);
	funcmux_select(PERIPH_ID_SDMMC2, FUNCMUX_SDMMC2_DTA_DTD_8BIT);

	/* For power GPIO PI6 */
	pinmux_tristate_disable(PINGRP_ATA);
	/* For CD GPIO PH2 */
	pinmux_tristate_disable(PINGRP_ATD);

	/* For power GPIO PT3 */
	pinmux_tristate_disable(PINGRP_DTB);
	/* For CD GPIO PI5 */
	pinmux_tristate_disable(PINGRP_ATC);
}
#endif

void pin_mux_usb(void)
{
	funcmux_select(PERIPH_ID_USB2, FUNCMUX_USB2_ULPI);
	pinmux_set_func(PINGRP_CDEV2, PMUX_FUNC_PLLP_OUT4);
	pinmux_tristate_disable(PINGRP_CDEV2);
	/* USB2 PHY reset GPIO */
	pinmux_tristate_disable(PINGRP_UAC);
}

void pin_mux_display(void)
{
	pinmux_set_func(PINGRP_SDC, PMUX_FUNC_PWM);
	pinmux_tristate_disable(PINGRP_SDC);
}
