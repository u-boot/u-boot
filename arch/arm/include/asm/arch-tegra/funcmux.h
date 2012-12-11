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

/* Tegra high-level function multiplexing */

#ifndef _TEGRA_FUNCMUX_H_
#define _TEGRA_FUNCMUX_H_

/**
 * Select a config for a particular peripheral.
 *
 * Each peripheral can operate through a number of configurations,
 * which are sets of pins that it uses to bring out its signals.
 * The basic config is 0, and higher numbers indicate different
 * pinmux settings to bring the peripheral out on other pins,
 *
 * This function also disables tristate for the function's pins,
 * so that they operate in normal mode.
 *
 * @param id		Peripheral id
 * @param config	Configuration to use (FUNCMUX_...), 0 for default
 * @return 0 if ok, -1 on error (e.g. incorrect id or config)
 */
int funcmux_select(enum periph_id id, int config);

#endif	/* _TEGRA_FUNCMUX_H_ */
