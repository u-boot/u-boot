// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Kontron Electronics GmbH
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
#include <linux/errno.h>
#include <linux/kernel.h>
#include <mmc.h>
#include <net.h>

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
		.addr = (u32 *)(OCOTP_BASE_ADDR + 0x410),
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
		.image_type_id = KONTRON_SL_MX8MM_FIT_IMAGE_GUID,
		.fw_name = u"KONTROL-SL-MX8MM-UBOOT",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "sf 0:0=flash-bin raw 0x400 0x1f0000",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_phys_sdram_size(phys_size_t *size)
{
	u32 ddr_size = readl(MCU_BOOTROM_BASE_ADDR);

	if (ddr_size == 4) {
		*size = 0x100000000;
	} else if (ddr_size == 3) {
		*size = 0xc0000000;
	} else if (ddr_size == 2) {
		*size = 0x80000000;
	} else if (ddr_size == 1) {
		*size = 0x40000000;
	} else {
		printf("Unknown DDR type!!!\n");
		*size = 0x40000000;
	}

	return 0;
}

/*
 * If the SoM is mounted on a baseboard with a USB ethernet controller,
 * there might be an additional MAC address programmed to the MAC OTP fuses.
 * Although the i.MX8MM has only one MAC, the MAC0, MAC1 and MAC2 registers
 * in the OTP fuses can still be used to store two separate addresses.
 * Try to read the secondary address from MAC1 and MAC2 and adjust the
 * devicetree so Linux can pick up the MAC address.
 */
int fdt_set_usb_eth_addr(void *blob)
{
	u32 value = readl(OCOTP_BASE_ADDR + 0x660);
	unsigned char mac[6];
	int node, ret;

	mac[0] = value >> 24;
	mac[1] = value >> 16;
	mac[2] = value >> 8;
	mac[3] = value;

	value = readl(OCOTP_BASE_ADDR + 0x650);
	mac[4] = value >> 24;
	mac[5] = value >> 16;

	node = fdt_path_offset(blob, fdt_get_alias(blob, "ethernet1"));
	if (node < 0) {
		/*
		 * There is no node for the USB ethernet in the devicetree. Just skip.
		 */
		return 0;
	}

	if (is_zero_ethaddr(mac)) {
		printf("\nNo MAC address for USB ethernet set in OTP fuses!\n");
		return 0;
	}

	if (!is_valid_ethaddr(mac)) {
		printf("\nInvalid MAC address for USB ethernet set in OTP fuses!\n");
		return -EINVAL;
	}

	ret = fdt_setprop(blob, node, "local-mac-address", &mac, 6);
	if (ret)
		ret = fdt_setprop(blob, node, "mac-address", &mac, 6);

	if (ret)
		printf("\nMissing mac-address or local-mac-address property in dt, skip setting MAC address for USB ethernet\n");

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	enum env_location env_loc;
	enum boot_device boot_dev;
	char env_str_sd[] = "sd-card";
	char env_str_nor[] = "spi-nor";
	char env_str_emmc[] = "emmc";
	char *env_config_str;
	int ret;

	ret = fdt_set_usb_eth_addr(blob);
	if (ret)
		return ret;

	ret = fdt_fixup_memory(blob, PHYS_SDRAM, gd->ram_size);
	if (ret)
		return ret;

	env_loc = env_get_location(0, 0);
	if (env_loc == ENVL_MMC) {
		boot_dev = get_boot_device();
		if (boot_dev == SD2_BOOT)
			env_config_str = env_str_sd;
		else if (boot_dev == MMC1_BOOT)
			env_config_str = env_str_emmc;
		else
			return 0;
	} else if (env_loc == ENVL_SPI_FLASH) {
		env_config_str = env_str_nor;
	} else {
		return 0;
	}

	/*
	 * Export a string to the devicetree that tells userspace tools like
	 * libubootenv where the environment is currently coming from.
	 */
	return fdt_find_and_setprop(blob, "/chosen", "u-boot,env-config",
				    env_config_str, strlen(env_config_str) + 1, 1);
}

int board_late_init(void)
{
	struct udevice *dev;
	int ret;

	if (!fdt_node_check_compatible(gd->fdt_blob, 0, "kontron,imx8mm-n802x-som") ||
	    !fdt_node_check_compatible(gd->fdt_blob, 0, "kontron,imx8mm-osm-s")) {
		env_set("som_type", "osm-s");
		env_set("touch_rst_gpio", "111");

		ret = uclass_get_device_by_name(UCLASS_MISC, "usb-hub@2c", &dev);
		if (ret)
			printf("Error bringing up USB hub (%d)\n", ret);
	} else {
		env_set("som_type", "sl");
		env_set("touch_rst_gpio", "87");
	}

	if (IS_ENABLED(CONFIG_KONTRON_HW_UID))
		get_serial_number(uid_otp_locations, ARRAY_SIZE(uid_otp_locations));

	if (is_usb_boot()) {
		env_set("bootcmd", "fastboot 0");
		env_set("bootdelay", "0");
	}

	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	enum boot_device boot_dev = get_boot_device();

	if (prio)
		return ENVL_UNKNOWN;

	if (CONFIG_IS_ENABLED(ENV_IS_NOWHERE) && is_usb_boot())
		return ENVL_NOWHERE;

	/*
	 * Make sure that the environment is loaded from
	 * the MMC if we are running from SD card or eMMC.
	 */
	if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC) &&
	    (boot_dev == SD1_BOOT || boot_dev == SD2_BOOT ||
	     boot_dev == MMC1_BOOT || boot_dev == MMC2_BOOT))
		return ENVL_MMC;

	if (CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
		return ENVL_SPI_FLASH;

	if (CONFIG_IS_ENABLED(ENV_IS_NOWHERE))
		return ENVL_NOWHERE;

	return ENVL_UNKNOWN;
}

#if defined(CONFIG_ENV_IS_IN_MMC)
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
