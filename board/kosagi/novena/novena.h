/*
 * Novena board support
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __BOARD_KOSAGI_NOVENA_NOVENA_H__
#define __BOARD_KOSAGI_NOVENA_NOVENA_H__

#define NOVENA_AUDIO_PWRON		IMX_GPIO_NR(5, 17)
#define NOVENA_BUTTON_GPIO		IMX_GPIO_NR(4, 14)
#define NOVENA_FPGA_RESET_N_GPIO	IMX_GPIO_NR(5, 7)
#define NOVENA_HDMI_GHOST_HPD		IMX_GPIO_NR(5, 4)
#define NOVENA_PCIE_DISABLE_GPIO	IMX_GPIO_NR(2, 16)
#define NOVENA_PCIE_POWER_ON_GPIO	IMX_GPIO_NR(7, 12)
#define NOVENA_PCIE_RESET_GPIO		IMX_GPIO_NR(3, 29)
#define NOVENA_PCIE_WAKE_UP_GPIO	IMX_GPIO_NR(3, 22)
#define NOVENA_SD_CD			IMX_GPIO_NR(1, 4)
#define NOVENA_SD_WP			IMX_GPIO_NR(1, 2)

void setup_display_clock(void);

#endif	/* __BOARD_KOSAGI_NOVENA_NOVENA_H__ */
