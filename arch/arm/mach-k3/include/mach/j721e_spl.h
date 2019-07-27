/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018-2019 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */
#ifndef _ASM_ARCH_J721E_SPL_H_
#define _ASM_ARCH_J721E_SPL_H_

/* With BootMode B = 0 */
#define BOOT_DEVICE_HYPERFLASH		0x00
#define BOOT_DEVICE_OSPI		0x01
#define BOOT_DEVICE_QSPI		0x02
#define BOOT_DEVICE_SPI			0x03
#define BOOT_DEVICE_ETHERNET		0x04
#define BOOT_DEVICE_I2C			0x06
#define BOOT_DEVICE_UART		0x07

/* With BootMode B = 1 */
#define BOOT_DEVICE_MMC2		0x10
#define BOOT_DEVICE_MMC1		0x11
#define BOOT_DEVICE_USB			0x12
#define BOOT_DEVICE_UFS			0x13
#define BOOT_DEVIE_GPMC			0x14
#define BOOT_DEVICE_PCIE		0x15
#define BOOT_DEVICE_MMC2_2		0x16
#define BOOT_DEVICE_RAM			0x17

#define BOOT_MODE_B_SHIFT		4
#define BOOT_MODE_B_MASK		BIT(4)

#endif
