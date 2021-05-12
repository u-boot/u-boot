/* SPDX-License-Identifier: BSD-3-Clause */
/**********************************************************************
 * Copyright (C) 2012-2018 Cadence Design Systems, Inc.
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 **********************************************************************
 * Cadence Core Driver for LPDDR4.
 **********************************************************************
 */

#ifndef LPDDR4_PRIV_H
#define LPDDR4_PRIV_H

#define PRODUCT_ID (0x1046U)
#define VERSION_0  (0x54d5da40U)
#define VERSION_1  (0xc1865a1U)

#define LPDDR4_BIT_MASK	(0x1U)
#define BYTE_MASK	(0xffU)
#define NIBBLE_MASK	(0xfU)

#define WORD_SHIFT (32U)
#define WORD_MASK (0xffffffffU)
#define SLICE_WIDTH (0x100)
/* Number of Data slices */
#define DSLICE_NUM (4U)
/*Number of Address Slices */
#define ASLICE_NUM (1U)

/* Number of accessible registers in each slice */
#define DSLICE0_REG_COUNT  (140U)
#define DSLICE1_REG_COUNT  (140U)
#define DSLICE2_REG_COUNT  (140U)
#define DSLICE3_REG_COUNT  (140U)
#define ASLICE0_REG_COUNT  (52U)
#define PHY_CORE_REG_COUNT (140U)

#define CTL_OFFSET 0
#define PI_OFFSET (((uint32_t)1) <<  11)
#define PHY_OFFSET (((uint32_t)1) << 12)

/* BIT[17] on INT_MASK_1 register. */
#define CTL_INT_MASK_ALL ((uint32_t)LPDDR4_LOR_BITS - WORD_SHIFT)

/* Init Error information bits */
#define PLL_READY (0x3U)
#define IO_CALIB_DONE ((uint32_t)0x1U << 23U)
#define IO_CALIB_FIELD ((uint32_t)NIBBLE_MASK << 28U)
#define IO_CALIB_STATE ((uint32_t)0xBU << 28U)
#define RX_CAL_DONE ((uint32_t)LPDDR4_BIT_MASK << 4U)
#define CA_TRAIN_RL (((uint32_t)LPDDR4_BIT_MASK << 5U) | \
		     ((uint32_t)LPDDR4_BIT_MASK << 4U))
#define WR_LVL_STATE (((uint32_t)NIBBLE_MASK) << 13U)
#define GATE_LVL_ERROR_FIELDS (((uint32_t)LPDDR4_BIT_MASK << 7U) | \
			       ((uint32_t)LPDDR4_BIT_MASK << 6U))
#define READ_LVL_ERROR_FIELDS ((((uint32_t)NIBBLE_MASK) << 28U) | \
			       (((uint32_t)BYTE_MASK) << 16U))
#define DQ_LVL_STATUS (((uint32_t)LPDDR4_BIT_MASK << 26U) | \
		       (((uint32_t)BYTE_MASK) << 18U))

#endif  /* LPDDR4_PRIV_H */
