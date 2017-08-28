
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5250 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_5250_H
#define __CONFIG_5250_H

#define CONFIG_EXYNOS5250

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_TEXT_BASE		0x43E00000

#define CONFIG_MACH_TYPE		MACH_TYPE_SMDK5250

#define CONFIG_SPL_MAX_FOOTPRINT	(14 * 1024)

#define CONFIG_SPL_TEXT_BASE	0x02023400

#define CONFIG_IRAM_STACK	0x02050000

#define CONFIG_SYS_INIT_SP_ADDR	CONFIG_IRAM_STACK

/* USB */
#define CONFIG_USB_EHCI_EXYNOS

#define CONFIG_USB_XHCI_EXYNOS

/* DRAM Memory Banks */
#define CONFIG_NR_DRAM_BANKS	8
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */

#endif  /* __CONFIG_5250_H */
