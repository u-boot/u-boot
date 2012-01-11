/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* Tegra2 high-level function multiplexing */
#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/pinmux.h>

int funcmux_select(enum periph_id id, int config)
{
	int bad_config = config != 0;

	switch (id) {
	case PERIPH_ID_UART1:
		if (config == 0) {
			pinmux_set_func(PINGRP_IRRX, PMUX_FUNC_UARTA);
			pinmux_set_func(PINGRP_IRTX, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PINGRP_IRRX);
			pinmux_tristate_disable(PINGRP_IRTX);
			/*
			 * Tegra appears to boot with function UARTA pre-
			 * selected on mux group SDB. If two mux groups are
			 * both set to the same function, it's unclear which
			 * group's pins drive the RX signals into the HW.
			 * For UARTA, SDB certainly overrides group IRTX in
			 * practice. To solve this, configure some alternative
			 * function on SDB to avoid the conflict. Also, tri-
			 * state the group to avoid driving any signal onto it
			 * until we know what's connected.
			 */
			pinmux_tristate_enable(PINGRP_SDB);
			pinmux_set_func(PINGRP_SDB,  PMUX_FUNC_SDIO3);
		}
		break;

	case PERIPH_ID_UART2:
		if (config == 0) {
			pinmux_set_func(PINGRP_UAD, PMUX_FUNC_IRDA);
			pinmux_tristate_disable(PINGRP_UAD);
		}
		break;

	case PERIPH_ID_UART4:
		if (config == 0) {
			pinmux_set_func(PINGRP_GMC, PMUX_FUNC_UARTD);
			pinmux_tristate_disable(PINGRP_GMC);
		}
		break;

	default:
		debug("%s: invalid periph_id %d", __func__, id);
		return -1;
	}

	if (bad_config) {
		debug("%s: invalid config %d for periph_id %d", __func__,
		      config, id);
		return -1;
	}

	return 0;
}
