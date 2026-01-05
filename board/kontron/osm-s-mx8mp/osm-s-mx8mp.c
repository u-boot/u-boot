// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Kontron Electronics GmbH
 */

#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <dm/uclass.h>
#include <efi.h>
#include <efi_loader.h>
#include <env_internal.h>
#include <fdt_support.h>
#include <init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <mmc.h>
#include <net.h>
#include <power/regulator.h>

#include "../common/hw-uid.h"

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_KONTRON_HW_UID)
struct uid_otp_loc uid_otp_locations[] = {
	{
		.addr = (u32 *)(OCOTP_BASE_ADDR + 0x7A0),
		.len = 2,
		.format = UID_OTP_FORMAT_DEC,
		.desc = "BOARD"
	},
	{
		.addr = (u32 *)(OCOTP_BASE_ADDR + 0x780),
		.len = 2,
		.format = UID_OTP_FORMAT_DEC,
		.desc = "SOM"
	},
#if IS_ENABLED(CONFIG_KONTRON_HW_UID_USE_SOC_FALLBACK)
	{
		.addr = (u32 *)(OCOTP_BASE_ADDR + 0xE00),
		.len = 2,
		.format = UID_OTP_FORMAT_HEX,
		.desc = "SOC"
	}
#endif
};
#endif /* CONFIG_KONTRON_HW_UID */

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.fw_name = u"KONTRON-OSM-S-MX8MP-UBOOT",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 0=flash-bin raw 0 0x2000 mmcpart 1",
	.images = fw_images,
};

u8 num_image_type_guids = ARRAY_SIZE(fw_images);
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_phys_sdram_size(phys_size_t *size)
{
	if (!size)
		return -EINVAL;

	*size = get_ram_size((void *)PHYS_SDRAM, 0x200000000);

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	enum boot_device boot_dev;
	char env_str_sd[] = "sd-card";
	char env_str_emmc[] = "emmc";
	char *env_config_str;

	if (env_get_location(0, 0) != ENVL_MMC)
		return 0;

	boot_dev = get_boot_device();
	if (boot_dev == SD2_BOOT)
		env_config_str = env_str_sd;
	else if (boot_dev == MMC1_BOOT)
		env_config_str = env_str_emmc;
	else
		return 0;

	/*
	 * Export a string to the devicetree that tells userspace tools like
	 * libubootenv where the environment is currently coming from.
	 */
	return fdt_find_and_setprop(blob, "/chosen", "u-boot,env-config",
				    env_config_str, strlen(env_config_str) + 1, 1);
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_KONTRON_HW_UID))
		get_serial_number(uid_otp_locations, ARRAY_SIZE(uid_otp_locations));

	if (is_usb_boot()) {
		env_set("bootcmd", "fastboot 0");
		env_set("bootdelay", "0");
	}

	return 0;
}

#if IS_ENABLED(CONFIG_ENV_IS_IN_MMC)
int board_mmc_get_env_dev(int devno)
{
	return devno;
}

uint mmc_get_env_part(struct mmc *mmc)
{
	if (IS_SD(mmc))
		return EMMC_HWPART_DEFAULT;

	switch (EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)) {
	case EMMC_BOOT_PART_BOOT1:
		return EMMC_HWPART_BOOT1;
	case EMMC_BOOT_PART_BOOT2:
		return EMMC_HWPART_BOOT2;
	default:
		return EMMC_HWPART_DEFAULT;
	}
}

int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	/* use normal offset for SD card */
	if (IS_SD(mmc)) {
		*env_addr = CONFIG_ENV_OFFSET;
		if (copy)
			*env_addr = CONFIG_ENV_OFFSET_REDUND;

		return 0;
	}

	switch (EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)) {
	case EMMC_BOOT_PART_BOOT1:
	case EMMC_BOOT_PART_BOOT2:
		*env_addr = mmc->capacity - CONFIG_ENV_SIZE - CONFIG_ENV_SIZE;
		if (copy)
			*env_addr = mmc->capacity - CONFIG_ENV_SIZE;
	break;
	default:
		*env_addr = CONFIG_ENV_OFFSET;
		if (copy)
			*env_addr = CONFIG_ENV_OFFSET_REDUND;
	}

	return 0;
}
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio)
		return ENVL_UNKNOWN;

	if (CONFIG_IS_ENABLED(ENV_IS_NOWHERE) && is_usb_boot())
		return ENVL_NOWHERE;

	return arch_env_get_location(op, prio);
}
