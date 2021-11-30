/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2021 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2021 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_16BIT_H
#define LPDDR4_16BIT_H

#define DSLICE_NUM (2U)
#define ASLICE_NUM (3U)

#ifdef __cplusplus
extern "C" {
#endif

#define DSLICE0_REG_COUNT  (126U)
#define DSLICE1_REG_COUNT  (126U)
#define ASLICE0_REG_COUNT  (42U)
#define ASLICE1_REG_COUNT  (42U)
#define ASLICE2_REG_COUNT  (42U)
#define PHY_CORE_REG_COUNT (126U)

#define GRP_SHIFT 1
#define INT_SHIFT 2

#ifdef __cplusplus
}
#endif

#endif /* LPDDR4_16BIT_H */
