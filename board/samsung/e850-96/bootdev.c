// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Routines for checking current boot device.
 */

#include <linux/arm-smccc.h>
#include <vsprintf.h>
#include "bootdev.h"

/* Flag from BL2 bootloader in RAM */
#define BL2_TAG_ADDR			0x80000000	/* DRAM base */
#define BL2_TAG				0xabcdef

/* Boot device info location in iRAM (only accessible from EL3) */
#define IRAM_BASE			0x02020000
#define BOOTDEVICE_INFO_ADDR		(IRAM_BASE + 0x64)

/* SMC call for getting boot device information from EL3 monitor */
#define SMC_CMD_CHECK_SECOND_BOOT	-233

/* Boot device constants for the encoded boot device info value */
#define BD_NO_DEVICE			0x0
#define BD_UFS				0x1
#define BD_EMMC				0x2
#define BD_ERROR			0x3
#define BD_USB				0x4
#define BD_SDMMC			0x5
#define BD_UFS_CARD			0x6
#define BD_SPI				0x7

/* If BL2 bootloader wasn't executed, it means U-Boot is running via JTAG */
static bool bootdev_is_jtag_session(void)
{
	u32 bl2_tag_val = *(u32 *)BL2_TAG_ADDR;

	return bl2_tag_val != BL2_TAG;
}

/* Obtain boot device information encoded in 32-bit value */
static u32 bootdev_get_info(void)
{
	u32 info;

	/*
	 * On regular boot U-Boot is executed by BL2 bootloader, and is running
	 * in EL1 mode, so the boot device information has to be obtained via
	 * SMC call from EL3 software (EL3 monitor), which can read that info
	 * from the protected iRAM memory. If U-Boot is running via TRACE32 JTAG
	 * (in EL3 mode), read the boot device info directly from iRAM, as EL3
	 * software might not be available.
	 */
	if (bootdev_is_jtag_session()) {
		info = *(u32 *)BOOTDEVICE_INFO_ADDR;
	} else {
		struct arm_smccc_res res;

		arm_smccc_smc(SMC_CMD_CHECK_SECOND_BOOT, 0, 0, 0, 0, 0, 0, 0,
			      &res);
		info = (u32)res.a2;
	}

	return info;
}

enum bootdev bootdev_get_current(void)
{
	u32 info, magic, order, dev;

	info = bootdev_get_info();
	magic = info >> 24;
	order = info & 0xf;
	dev = (info >> (4 * order)) & 0xf;

	if (magic != 0xcb)
		panic("Abnormal boot");

	switch (dev) {
	case BD_UFS:
		return BOOTDEV_UFS;
	case BD_EMMC:
		return BOOTDEV_EMMC;
	case BD_USB:
		return BOOTDEV_USB;
	case BD_SDMMC:
		return BOOTDEV_SD;
	default:
		return BOOTDEV_ERROR;
	}

	return BOOTDEV_ERROR;
}

bool bootdev_is_usb(void)
{
	return bootdev_get_current() == BOOTDEV_USB;
}
