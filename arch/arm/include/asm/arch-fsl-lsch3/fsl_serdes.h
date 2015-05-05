/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_SERDES_H
#define __FSL_SERDES_H

#include <config.h>

#define	SRDS_MAX_LANES	8

enum srds_prtcl {
	NONE = 0,
	PCIE1,
	PCIE2,
	PCIE3,
	PCIE4,
	SATA1,
	SATA2,
	XAUI1,
	XAUI2,
	XFI1,
	XFI2,
	XFI3,
	XFI4,
	XFI5,
	XFI6,
	XFI7,
	XFI8,
	SGMII1,
	SGMII2,
	SGMII3,
	SGMII4,
	SGMII5,
	SGMII6,
	SGMII7,
	SGMII8,
	SGMII9,
	SGMII10,
	SGMII11,
	SGMII12,
	SGMII13,
	SGMII14,
	SGMII15,
	SGMII16,
	QSGMII_A, /* A indicates MACs 1-4 */
	QSGMII_B, /* B indicates MACs 5-8 */
	QSGMII_C, /* C indicates MACs 9-12 */
	QSGMII_D, /* D indicates MACs 12-16 */
	SERDES_PRCTL_COUNT
};

enum srds {
	FSL_SRDS_1  = 0,
	FSL_SRDS_2  = 1,
};

int is_serdes_configured(enum srds_prtcl device);
void fsl_serdes_init(void);

int serdes_get_first_lane(u32 sd, enum srds_prtcl device);
enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane);
int is_serdes_prtcl_valid(int serdes, u32 prtcl);

#endif /* __FSL_SERDES_H */
