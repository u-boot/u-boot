/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Broadcom
 */

#ifndef DT_BINDINGS_BCM_NS3_MC_H
#define DT_BINDINGS_BCM_NS3_MC_H

/*
 * Reserved Memory Map : SHMEM & TZDRAM.
 * +--------+----------+ 0x8d000000
 * | SHMEM (NS)         | 16 MB
 * +-------------------+ 0x8e000000
 * |        | TEE_RAM(S)| 4MB
 * + TZDRAM +----------+ 0x8e400000
 * |        | TA_RAM(S) | 12MB
 * +--------+----------+ 0x8f000000
 * | BL31 + TMON + LPM  |
 * | memory             | 1MB
 * +-------------------+ 0x8f100000
 */

#define BCM_NS3_MEM_SHARE_START    0x8d000000
#define BCM_NS3_MEM_SHARE_LEN      0x020fffff

/* ATF/U-boot/Linux error logs */
#define BCM_NS3_MEM_ELOG_START     0x8f113000
#define BCM_NS3_MEM_ELOG_LEN       0x00100000

/* CRMU Page table memroy */
#define BCM_NS3_MEM_CRMU_PT_START  0x880000000
#define BCM_NS3_MEM_CRMU_PT_LEN    0x200000

#endif
