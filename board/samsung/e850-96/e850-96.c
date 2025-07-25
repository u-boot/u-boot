// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <efi_loader.h>
#include <env.h>
#include <init.h>
#include <mapmem.h>
#include <asm/io.h>
#include "fw.h"

/* OTP Controller base address and register offsets */
#define EXYNOS850_OTP_BASE	0x10000000
#define OTP_CHIPID0		0x4
#define OTP_CHIPID1		0x8

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = E850_96_FWBL1_IMAGE_GUID,
		.fw_name = u"E850-96-FWBL1",
		.image_index = 1,
	},
	{
		.image_type_id = E850_96_EPBL_IMAGE_GUID,
		.fw_name = u"E850-96-EPBL",
		.image_index = 2,
	},
	{
		.image_type_id = E850_96_BL2_IMAGE_GUID,
		.fw_name = u"E850-96-BL2",
		.image_index = 3,
	},
	{
		.image_type_id = E850_96_BOOTLOADER_IMAGE_GUID,
		.fw_name = u"E850-96-BOOTLOADER",
		.image_index = 4,
	},
	{
		.image_type_id = E850_96_EL3_MON_IMAGE_GUID,
		.fw_name = u"E850-96-EL3-MON",
		.image_index = 5,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 0="
			"fwbl1.img raw 0x0 0x18 mmcpart 1;"
			"epbl.img raw 0x18 0x98 mmcpart 1;"
			"bl2.img raw 0xb0 0x200 mmcpart 1;"
			"bootloader.img raw 0x438 0x1000 mmcpart 1;"
			"el3_mon.img raw 0x1438 0x200 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

/* Read the unique SoC ID from OTP registers */
static u64 get_chip_id(void)
{
	void __iomem *otp_base;
	u64 val;

	otp_base = map_sysmem(EXYNOS850_OTP_BASE, 12);
	val = readl(otp_base + OTP_CHIPID0);
	val |= (u64)readl(otp_base + OTP_CHIPID1) << 32UL;
	unmap_sysmem(otp_base);

	return val;
}

static void setup_serial(void)
{
	char serial_str[17] = { 0 };
	u64 serial_num;

	if (env_get("serial#"))
		return;

	serial_num = get_chip_id();
	snprintf(serial_str, sizeof(serial_str), "%016llx", serial_num);
	env_set("serial#", serial_str);
}

int board_late_init(void)
{
	int err;

	setup_serial();

	/*
	 * Do this in board_late_init() to make sure MMC is not probed before
	 * efi_init_early().
	 */
	err = load_ldfw();
	if (err)
		printf("ERROR: LDFW loading failed (%d)\n", err);

	return 0;
}
