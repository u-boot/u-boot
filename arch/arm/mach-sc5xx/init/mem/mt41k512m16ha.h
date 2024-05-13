/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#ifndef MT41K512M16HA_H
#define MT41K512M16HA_H

/* Default DDR3 part assumed: MT41K512M16HA-107, 8Gb part */
/* For DCLK= 450 MHz */
#define DMC_DLLCALRDCNT                 72
#define DMC_DATACYC                     9
#define DMC_TRCD                        7
#define DMC_TWTR                        4
#define DMC_TRP                         7
#define DMC_TRAS                        10
#define DMC_TRC                         16
#define DMC_TMRD                        4
#define DMC_TREF                        3510
#define DMC_TRFC                        158
#define DMC_TRRD                        6
#define DMC_TFAW                        16
#define DMC_TRTP                        4
#define DMC_TWR                         7
#define DMC_TXP                         3
#define DMC_TCKE                        3
#define DMC_CL0                         0
#define DMC_CL123                       3
#define DMC_WRRECOV                     (DMC_TWR - 1)
#define DMC_MR1_DLLEN                   0
#define DMC_MR1_DIC0                    1
#define DMC_MR1_RTT0                    1
#define DMC_MR1_AL                      0
#define DMC_MR1_DIC1                    0
#define DMC_MR1_RTT1                    0
#define DMC_MR1_WL                      0
#define DMC_MR1_RTT2                    0
#define DMC_MR1_TDQS                    0
#define DMC_MR1_QOFF                    0
#define DMC_WL                          1
#define DMC_RDTOWR                      2
#define DMC_CTL_AL_EN                   0
#define SDR_CHIP_SIZE                   ENUM_DMC_CFG_SDRSIZE8G

#endif
