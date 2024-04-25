/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#ifndef MT47H128M16RT_H
#define MT47H128M16RT_H

/* Default DDR2 part: MT47H128M16RT-25E XIT:C, 2 Gb part */
/* For DCLK= 400 MHz */
#define DMC_DLLCALRDCNT                 72
#define DMC_DATACYC                     9
#define DMC_TRCD                        5
#define DMC_TWTR                        3
#define DMC_TRP                         5
#define DMC_TRAS                        16
#define DMC_TRC                         22
#define DMC_TMRD                        2
#define DMC_TREF                        3120
#define DMC_TRFC                        78
#define DMC_TRRD                        4
#define DMC_TFAW                        18
#define DMC_TRTP                        3
#define DMC_TWR                         6
#define DMC_TXP                         2
#define DMC_TCKE                        3
#define DMC_CL                          5
#define DMC_WRRECOV                     (DMC_TWR - 1)
#define DMC_MR1_DLLEN                   0
#define DMC_MR1_DIC0                    1
#define DMC_MR1_RTT0                    1
#define DMC_MR1_AL                      4
#define DMC_MR1_DIC1                    0
#define DMC_MR1_RTT1                    0
#define DMC_MR1_WL                      0
#define DMC_MR1_RTT2                    0
#define DMC_MR1_TDQS                    0
#define DMC_MR1_QOFF                    0
#define DMC_BL                          4
#define DMC_RDTOWR                      2
#define DMC_CTL_AL_EN                   0
#define SDR_CHIP_SIZE                   ENUM_DMC_CFG_SDRSIZE2G

#endif
