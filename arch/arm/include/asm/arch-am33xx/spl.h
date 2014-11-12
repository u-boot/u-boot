/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_ASM_ARCH_SPL_H_
#define	_ASM_ARCH_SPL_H_

#if defined(CONFIG_TI816X)
#define BOOT_DEVICE_XIP		2
#define BOOT_DEVICE_NAND	3
#define BOOT_DEVICE_MMC1	6
#define BOOT_DEVICE_MMC2	5
#define BOOT_DEVICE_UART	0x43
#elif defined(CONFIG_AM43XX)
#define BOOT_DEVICE_NOR		1
#define BOOT_DEVICE_NAND	5
#define BOOT_DEVICE_MMC1	7
#define BOOT_DEVICE_MMC2	8
#define BOOT_DEVICE_SPI		10
#define BOOT_DEVICE_USB     13
#define BOOT_DEVICE_UART	65
#define BOOT_DEVICE_CPGMAC	71
#else
#define BOOT_DEVICE_XIP       	2
#define BOOT_DEVICE_NAND	5
#define BOOT_DEVICE_NAND_I2C	6
#if defined(CONFIG_AM33XX)
#define BOOT_DEVICE_MMC1	8
#define BOOT_DEVICE_MMC2	9	/* eMMC or daughter card */
#elif defined(CONFIG_TI814X)
#define BOOT_DEVICE_MMC1	9
#define BOOT_DEVICE_MMC2	8	/* ROM only supports 2nd instance */
#endif
#define BOOT_DEVICE_SPI		11
#define BOOT_DEVICE_UART	65
#define BOOT_DEVICE_USBETH	68
#define BOOT_DEVICE_CPGMAC	70
#endif
#define BOOT_DEVICE_MMC2_2      0xFF

#if defined(CONFIG_AM33XX)
#define MMC_BOOT_DEVICES_START BOOT_DEVICE_MMC1
#define MMC_BOOT_DEVICES_END   BOOT_DEVICE_MMC2
#elif defined(CONFIG_AM43XX)
#define MMC_BOOT_DEVICES_START BOOT_DEVICE_MMC1
#ifdef CONFIG_SPL_USB_SUPPORT
#define MMC_BOOT_DEVICES_END   BOOT_DEVICE_USB
#else
#define MMC_BOOT_DEVICES_END   BOOT_DEVICE_MMC2
#endif
#elif defined(CONFIG_TI81XX)
#define MMC_BOOT_DEVICES_START	BOOT_DEVICE_MMC2
#define MMC_BOOT_DEVICES_END	BOOT_DEVICE_MMC1
#endif
#endif
