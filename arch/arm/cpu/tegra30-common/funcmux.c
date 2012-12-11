/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Tegra30 high-level function multiplexing */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>

int funcmux_select(enum periph_id id, int config)
{
	int bad_config = config != FUNCMUX_DEFAULT;

	switch (id) {
	case PERIPH_ID_UART1:
		switch (config) {
		case FUNCMUX_UART1_ULPI:
			pinmux_set_func(PINGRP_ULPI_DATA0, PMUX_FUNC_UARTA);
			pinmux_set_func(PINGRP_ULPI_DATA1, PMUX_FUNC_UARTA);
			pinmux_set_func(PINGRP_ULPI_DATA2, PMUX_FUNC_UARTA);
			pinmux_set_func(PINGRP_ULPI_DATA3, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PINGRP_ULPI_DATA0);
			pinmux_tristate_disable(PINGRP_ULPI_DATA1);
			pinmux_tristate_disable(PINGRP_ULPI_DATA2);
			pinmux_tristate_disable(PINGRP_ULPI_DATA3);
			break;
		}
		break;

	/* Add other periph IDs here as needed */

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
