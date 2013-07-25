/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define DWMCI_CLKSEL		0x09C
#define DWMCI_SHIFT_0		0x0
#define DWMCI_SHIFT_1		0x1
#define DWMCI_SHIFT_2		0x2
#define DWMCI_SHIFT_3		0x3
#define DWMCI_SET_SAMPLE_CLK(x)	(x)
#define DWMCI_SET_DRV_CLK(x)	((x) << 16)
#define DWMCI_SET_DIV_RATIO(x)	((x) << 24)

#ifdef CONFIG_OF_CONTROL
int exynos_dwmmc_init(const void *blob);
#endif
int exynos_dwmci_add_port(int index, u32 regbase, int bus_width, u32 clksel);
