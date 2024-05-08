/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#ifndef IS43TR16512BL_H
#define IS43TR16512BL_H

/* DMC0 setup for the EV-21593-SOM and EV-SC594-SOM :
 * - uses a single 8GB IS43TR16512BL-125KBL DDR3 chip configured for
 *   800 MHz DCLK.
 * DMC0 setup for the EV-SC594-SOMS :
 * - uses a single 4GB IS43TR16256BL-093NBL DDR3 chip configured for
 *   800 MHz DCLK.
 */
#define DMC_DLLCALRDCNT                 240
#define DMC_DATACYC                     12
#define DMC_TRCD                        11
#define DMC_TWTR                        6
#define DMC_TRP                         11
#define DMC_TRAS                        28
#define DMC_TRC                         39
#define DMC_TMRD                        4
#define DMC_TREF                        6240
#define DMC_TRRD                        6
#define DMC_TFAW                        32
#define DMC_TRTP                        6
#define DMC_TWR                         12
#define DMC_TXP                         5
#define DMC_TCKE                        4
#define DMC_CL0                         0
#define DMC_CL123                       7
#define DMC_WRRECOV                     6
#define DMC_MR1_DLLEN                   0
#define DMC_MR1_DIC0                    0
#define DMC_MR1_RTT0                    0
#define DMC_MR1_AL                      0
#define DMC_MR1_DIC1                    0
#define DMC_MR1_RTT1                    1
#define DMC_MR1_WL                      0
#define DMC_MR1_RTT2                    0
#define DMC_MR1_TDQS                    0
#define DMC_MR1_QOFF                    0
#define DMC_WL                          3
#define DMC_RDTOWR                      5
#define DMC_CTL_AL_EN                   1
#if defined(MEM_ISSI_4Gb_DDR3_800MHZ)
    #define SDR_CHIP_SIZE                    (ENUM_DMC_CFG_SDRSIZE4G)
    #define DMC_TRFC                        208ul
#elif defined(MEM_ISSI_8Gb_DDR3_800MHZ)
    #define SDR_CHIP_SIZE                    (ENUM_DMC_CFG_SDRSIZE8G)
    #define DMC_TRFC                        280ul
#else
    #error "Need to select MEM_ISSI_4Gb_DDR3_800MHZ or MEM_ISSI_8Gb_DDR3_800MHZ"
#endif

#endif
