/* SPDX-License-Identifier: GPL-2.0 */
/*
 * RZ/G2L USB PHY common definitions
 *
 * Copyright (C) 2021-2023 Renesas Electronics Corp.
 */

#ifndef RENESAS_RZG2L_USBPHY_H
#define RENESAS_RZG2L_USBPHY_H

#include <fdtdec.h>

struct rzg2l_usbphy_ctrl_priv {
	fdt_addr_t regs;
};

#endif /* RENESAS_RZG2L_USBPHY_H */
