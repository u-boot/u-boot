// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include <env.h>
#include <efi_loader.h>
#include <i2c.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/boot_mode.h>
#include <dm/device.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
#define IMX_BOOT_IMAGE_GUID \
	EFI_GUID(0xbc550d86, 0xda26, 0x4b70, 0xac, 0x05, \
		 0x2a, 0x44, 0x8e, 0xda, 0x6f, 0x21)

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = IMX_BOOT_IMAGE_GUID,
		.fw_name = u"IMX93-11X11-FRDM-RAW",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 0=flash-bin raw 0 0x2000 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

#define TCPC_ALERT 0x10
#define TCPC_ALERT_MASK 0x12
#define TCPC_FAULT_STATUS_MASK 0x15

#define TCPC1_I2C_BUS 2
#define TCPC1_I2C_ADDR 0x50

/*
 * Mask all interrupts and clear alert status for the PTN5110 TCPC USB Power
 * Delivery controller. This is required to avoid an interrupt storm on OS
 * startup, since the interrupt line for the PTN5110 is shared also by the
 * PCAL6524 I/O expander.
 */
static int clear_pd_alert(void)
{
	struct udevice *bus;
	struct udevice *i2c_dev = NULL;
	int ret;
	u8 buffer_0[2] = {0, 0};
	u8 buffer_1[2] = {0xff, 0xff};

	ret = uclass_get_device_by_seq(UCLASS_I2C, TCPC1_I2C_BUS, &bus);
	if (ret) {
		printf("Failed to get I2C bus %d\n", TCPC1_I2C_BUS);
		return ret;
	}

	ret = dm_i2c_probe(bus, TCPC1_I2C_ADDR, 0, &i2c_dev);
	if (ret) {
		printf("Can't find USB PD device at 0x%02x\n", TCPC1_I2C_ADDR);
		return ret;
	}

	/* Mask all alert status*/
	ret = dm_i2c_write(i2c_dev, TCPC_ALERT_MASK, buffer_0, 2);
	if (ret) {
		printf("%s dm_i2c_write failed: %d\n", __func__, ret);
		return ret;
	}

	ret = dm_i2c_write(i2c_dev, TCPC_FAULT_STATUS_MASK, buffer_0, 2);
	if (ret) {
		printf("%s dm_i2c_write failed: %d\n", __func__, ret);
		return ret;
	}

	/* Clear active alerts */
	ret = dm_i2c_write(i2c_dev, TCPC_ALERT, buffer_1, 2);
	if (ret) {
		printf("%s dm_i2c_write failed: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC) || IS_ENABLED(CONFIG_ENV_IS_IN_NOWHERE))
		board_late_mmc_env_init();

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", "11X11_FRDM");
		env_set("board_rev", "iMX93");
	}

	if (get_boot_device() == USB_BOOT) {
		printf("USB boot detected. Will enter fasboot mode\n");
		env_set_ulong("dofastboot", 1);
	}

	clear_pd_alert();

	return 0;
}
