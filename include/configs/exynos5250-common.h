/* SPDX-License-Identifier: GPL-2.0+ */

/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5250 board.
 */

#ifndef __CONFIG_5250_H
#define __CONFIG_5250_H

#define CONFIG_SYS_SDRAM_BASE		0x40000000

/* USB */
#define CONFIG_USB_EHCI_EXYNOS

#define CONFIG_USB_XHCI_EXYNOS

/* DRAM Memory Banks */
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */

#endif  /* __CONFIG_5250_H */
