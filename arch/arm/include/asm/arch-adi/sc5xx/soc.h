/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#ifndef BOARD_ADI_COMMON_SOC_H
#define BOARD_ADI_COMMON_SOC_H

#include <phy.h>

void fixup_dp83867_phy(struct phy_device *phydev);

#endif
