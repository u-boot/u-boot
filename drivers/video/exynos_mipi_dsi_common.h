/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/fb.h>

#ifndef _EXYNOS_MIPI_DSI_COMMON_H
#define _EXYNOS_MIPI_DSI_COMMON_H

int exynos_mipi_dsi_wr_data(struct mipi_dsim_device *dsim, unsigned int data_id,
	unsigned int data0, unsigned int data1);
int exynos_mipi_dsi_pll_on(struct mipi_dsim_device *dsim, unsigned int enable);
unsigned long exynos_mipi_dsi_change_pll(struct mipi_dsim_device *dsim,
	unsigned int pre_divider, unsigned int main_divider,
	unsigned int scaler);
int exynos_mipi_dsi_set_clock(struct mipi_dsim_device *dsim,
	unsigned int byte_clk_sel, unsigned int enable);
int exynos_mipi_dsi_init_dsim(struct mipi_dsim_device *dsim);
int exynos_mipi_dsi_set_display_mode(struct mipi_dsim_device *dsim,
			struct mipi_dsim_config *dsim_info);
int exynos_mipi_dsi_init_link(struct mipi_dsim_device *dsim);
int exynos_mipi_dsi_set_hs_enable(struct mipi_dsim_device *dsim);
int exynos_mipi_dsi_set_data_transfer_mode(struct mipi_dsim_device *dsim,
		unsigned int mode);
int exynos_mipi_dsi_enable_frame_done_int(struct mipi_dsim_device *dsim,
	unsigned int enable);
int exynos_mipi_dsi_get_frame_done_status(struct mipi_dsim_device *dsim);
int exynos_mipi_dsi_clear_frame_done(struct mipi_dsim_device *dsim);

#endif /* _EXYNOS_MIPI_DSI_COMMON_H */
