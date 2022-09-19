/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 - 2022, Xilinx, Inc.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.
 */

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define PMC_TAP	0xF11A0000

#define PMC_TAP_IDCODE		(PMC_TAP + 0)
#define PMC_TAP_VERSION		(PMC_TAP + 0x4)
# define PMC_VERSION_MASK	GENMASK(7, 0)
# define PS_VERSION_MASK	GENMASK(15, 8)
# define RTL_VERSION_MASK	GENMASK(23, 16)
# define PLATFORM_MASK		GENMASK(27, 24)
# define PLATFORM_VERSION_MASK	GENMASK(31, 28)
#define PMC_TAP_USERCODE	(PMC_TAP + 0x8)

enum versal_net_platform {
	VERSAL_NET_SILICON = 0,
	VERSAL_NET_SPP = 1,
	VERSAL_NET_EMU = 2,
	VERSAL_NET_QEMU = 3,
};

#define VERSAL_SLCR_BASEADDR	0xF1060000
#define VERSAL_AXI_MUX_SEL	(VERSAL_SLCR_BASEADDR + 0x504)
#define VERSAL_OSPI_LINEAR_MODE	BIT(1)
