/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_DWMMC_H
#define __ASM_ARM_ARCH_DWMMC_H

#include <linux/bitops.h>

#define DWMCI_CLKSEL			0x09c
#define DWMCI_CLKSEL64			0x0a8
#define DWMCI_SET_SAMPLE_CLK(x)		(x)
#define DWMCI_SET_DRV_CLK(x)		((x) << 16)
#define DWMCI_SET_DIV_RATIO(x)		((x) << 24)

/* Protector Register */
#define DWMCI_EMMCP_BASE		0x1000
#define EMMCP_MPSBEGIN0			(DWMCI_EMMCP_BASE + 0x0200)
#define EMMCP_SEND0			(DWMCI_EMMCP_BASE + 0x0204)
#define EMMCP_CTRL0			(DWMCI_EMMCP_BASE + 0x020c)

#define MPSCTRL_SECURE_READ_BIT		BIT(7)
#define MPSCTRL_SECURE_WRITE_BIT	BIT(6)
#define MPSCTRL_NON_SECURE_READ_BIT	BIT(5)
#define MPSCTRL_NON_SECURE_WRITE_BIT	BIT(4)
#define MPSCTRL_USE_FUSE_KEY		BIT(3)
#define MPSCTRL_ECB_MODE		BIT(2)
#define MPSCTRL_ENCRYPTION		BIT(1)
#define MPSCTRL_VALID			BIT(0)

/* CLKSEL Register */
#define DWMCI_DIVRATIO_BIT		24
#define DWMCI_DIVRATIO_MASK		0x7

#endif /* __ASM_ARM_ARCH_DWMMC_H */
