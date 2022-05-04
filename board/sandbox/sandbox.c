// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <cpu_func.h>
#include <cros_ec.h>
#include <dfu.h>
#include <dm.h>
#include <efi.h>
#include <efi_loader.h>
#include <env_internal.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <init.h>
#include <led.h>
#include <mtd.h>
#include <os.h>
#include <uuid.h>
#include <asm/global_data.h>
#include <asm/test.h>
#include <asm/u-boot-sandbox.h>
#include <linux/kernel.h>
#include <malloc.h>

#include <extension_board.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it.
 */
gd_t *gd;

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
#if defined(CONFIG_EFI_CAPSULE_FIRMWARE_RAW)
	{
		.image_type_id = SANDBOX_UBOOT_IMAGE_GUID,
		.fw_name = u"SANDBOX-UBOOT",
		.image_index = 1,
	},
	{
		.image_type_id = SANDBOX_UBOOT_ENV_IMAGE_GUID,
		.fw_name = u"SANDBOX-UBOOT-ENV",
		.image_index = 2,
	},
#elif defined(CONFIG_EFI_CAPSULE_FIRMWARE_FIT)
	{
		.image_type_id = SANDBOX_FIT_IMAGE_GUID,
		.fw_name = u"SANDBOX-FIT",
		.image_index = 1,
	},
#endif
};

struct efi_capsule_update_info update_info = {
#if defined(CONFIG_FWU_MULTI_BANK_UPDATE)
	.dfu_string = "mtd nor0=u-boot-bin-a raw 0x100000 0x10000;"
		"u-boot-env-a raw 0x120000 0x10000;"
		"u-boot-bin-b raw 0x140000 0x10000;"
		"u-boot-env-b raw 0x160000 0x10000",
#else
	.dfu_string = "sf 0:0=u-boot-bin raw 0x100000 0x50000;"
		"u-boot-env raw 0x150000 0x200000",
#endif
	.images = fw_images,
};

u8 num_image_type_guids = ARRAY_SIZE(fw_images);
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

#if !CONFIG_IS_ENABLED(OF_PLATDATA)
/*
 * Add a simple GPIO device (don't use with of-platdata as it interferes with
 * the auto-generated devices)
 */
U_BOOT_DRVINFO(gpio_sandbox) = {
	.name = "sandbox_gpio",
};
#endif

#ifndef CONFIG_TIMER
/* system timer offset in ms */
static unsigned long sandbox_timer_offset;

void timer_test_add_offset(unsigned long offset)
{
	sandbox_timer_offset += offset;
}

unsigned long timer_read_counter(void)
{
	return os_get_nsec() / 1000 + sandbox_timer_offset * 1000;
}
#endif

/* specific order for sandbox: nowhere is the first value, used by default */
static enum env_location env_locations[] = {
	ENVL_NOWHERE,
	ENVL_EXT4,
	ENVL_FAT,
};

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio >= ARRAY_SIZE(env_locations))
		return ENVL_UNKNOWN;

	return env_locations[prio];
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

int board_init(void)
{
	return 0;
}

int ft_board_setup(void *fdt, struct bd_info *bd)
{
	/* Create an arbitrary reservation to allow testing OF_BOARD_SETUP.*/
	return fdt_add_mem_rsv(fdt, 0x00d02000, 0x4000);
}

#ifdef CONFIG_CMD_EXTENSION
int extension_board_scan(struct list_head *extension_list)
{
	struct extension *extension;
	int i;

	for (i = 0; i < 2; i++) {
		extension = calloc(1, sizeof(struct extension));
		snprintf(extension->overlay, sizeof(extension->overlay), "overlay%d.dtbo", i);
		snprintf(extension->name, sizeof(extension->name), "extension board %d", i);
		snprintf(extension->owner, sizeof(extension->owner), "sandbox");
		snprintf(extension->version, sizeof(extension->version), "1.1");
		snprintf(extension->other, sizeof(extension->other), "Fictionnal extension board");
		list_add_tail(&extension->list, extension_list);
	}

	return i;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_CROS_EC, &dev);
	if (ret && ret != -ENODEV) {
		/* Force console on */
		gd->flags &= ~GD_FLG_SILENT;

		printf("cros-ec communications failure %d\n", ret);
		puts("\nPlease reset with Power+Refresh\n\n");
		panic("Cannot init cros-ec device");
		return -1;
	}
	return 0;
}
#endif

#if defined(CONFIG_FWU_MULTI_BANK_UPDATE)
int fwu_plat_get_alt_num(struct udevice __always_unused *dev,
			 efi_guid_t *image_id, int *alt_num)
{
	return fwu_get_mtd_alt_num(image_id, alt_num, "nor0", 1);
}

int fwu_plat_get_update_index(u32 *update_idx)
{
	int ret;
	u32 active_idx;

	ret = fwu_get_active_index(&active_idx);

	if (ret < 0)
		return -1;

	*update_idx = active_idx ^= 0x1;

	return ret;
}

void fwu_plat_get_bootidx(void *boot_idx)
{
	int ret;
	u32 active_idx;
	u32 *bootidx = boot_idx;

	ret = fwu_get_active_index(&active_idx);

	if (ret < 0)
		*bootidx = -1;

	*bootidx = active_idx;
}
#endif /* CONFIG_FWU_MULTI_BANK_UPDATE */
