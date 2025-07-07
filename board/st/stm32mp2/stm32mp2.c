// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_BOARD

#include <button.h>
#include <config.h>
#include <env_internal.h>
#include <fdt_support.h>
#include <led.h>
#include <log.h>
#include <misc.h>
#include <mmc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>
#include <linux/delay.h>

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	int ret;
	u32 otp;
	struct udevice *dev;
	const char *fdt_compat;
	int fdt_compat_len;

	fdt_compat = ofnode_get_property(ofnode_root(), "compatible", &fdt_compat_len);

	log_info("Board: stm32mp2 (%s)\n", fdt_compat && fdt_compat_len ? fdt_compat : "");

	/* display the STMicroelectronics board identification */
	if (CONFIG_IS_ENABLED(CMD_STBOARD)) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stm32mp_bsec),
						  &dev);
		if (!ret)
			ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_BOARD),
					&otp, sizeof(otp));
		if (ret > 0 && otp)
			log_info("Board: MB%04x Var%d.%d Rev.%c-%02d\n",
				 otp >> 16,
				 (otp >> 12) & 0xF,
				 (otp >> 4) & 0xF,
				 ((otp >> 8) & 0xF) - 1 + 'A',
				 otp & 0xF);
	}

	return 0;
}

static int get_led(struct udevice **dev, char *led_string)
{
	const char *led_name;
	int ret;

	led_name = ofnode_conf_read_str(led_string);
	if (!led_name) {
		log_debug("could not find %s config string\n", led_string);
		return -ENOENT;
	}
	ret = led_get_by_label(led_name, dev);
	if (ret) {
		log_debug("get=%d\n", ret);
		return ret;
	}

	return 0;
}

static int setup_led(enum led_state_t cmd)
{
	struct udevice *dev;
	int ret;

	if (!CONFIG_IS_ENABLED(LED))
		return 0;

	ret = get_led(&dev, "u-boot,boot-led");
	if (ret)
		return ret;

	ret = led_set_state(dev, cmd);
	return ret;
}

static void check_user_button(void)
{
	struct udevice *button;
	int i;

	if (!IS_ENABLED(CONFIG_CMD_STM32PROG) || !IS_ENABLED(CONFIG_BUTTON))
		return;

	if (button_get_by_label("User-2", &button))
		return;

	for (i = 0; i < 21; ++i) {
		if (button_get_state(button) != BUTTON_ON)
			return;
		if (i < 20)
			mdelay(50);
	}

	log_notice("entering download mode...\n");
	clrsetbits_le32(TAMP_BOOT_CONTEXT, TAMP_BOOT_FORCED_MASK, BOOT_STM32PROG);
}

/* board dependent setup after realloc */
int board_init(void)
{
	setup_led(LEDST_ON);
	check_user_button();

	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = get_bootmode();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
			return ENVL_MMC;
		else
			return ENVL_NOWHERE;

	case BOOT_FLASH_NAND:
	case BOOT_FLASH_SPINAND:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_UBI))
			return ENVL_UBI;
		else
			return ENVL_NOWHERE;

	case BOOT_FLASH_NOR:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		else
			return ENVL_NOWHERE;
	default:
		return ENVL_NOWHERE;
	}
}

int mmc_get_boot(void)
{
	struct udevice *dev;
	u32 boot_mode = get_bootmode();
	unsigned int instance = (boot_mode & TAMP_BOOT_INSTANCE_MASK) - 1;
	char cmd[20];
	const u32 sdmmc_addr[] = {
		STM32_SDMMC1_BASE,
		STM32_SDMMC2_BASE,
		STM32_SDMMC3_BASE
	};

	if (instance > ARRAY_SIZE(sdmmc_addr))
		return 0;

	/* search associated sdmmc node in devicetree */
	snprintf(cmd, sizeof(cmd), "mmc@%x", sdmmc_addr[instance]);
	if (uclass_get_device_by_name(UCLASS_MMC, cmd, &dev)) {
		log_err("mmc%d = %s not found in device tree!\n", instance, cmd);
		return 0;
	}

	return dev_seq(dev);
};

int mmc_get_env_dev(void)
{
	const int mmc_env_dev = CONFIG_IS_ENABLED(ENV_IS_IN_MMC, (CONFIG_ENV_MMC_DEVICE_INDEX), (-1));

	if (mmc_env_dev >= 0)
		return mmc_env_dev;

	/* use boot instance to select the correct mmc device identifier */
	return mmc_get_boot();
}

int board_late_init(void)
{
	const void *fdt_compat;
	int fdt_compat_len;
	char dtb_name[256];
	int buf_len;

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
					 &fdt_compat_len);
		if (fdt_compat && fdt_compat_len) {
			if (strncmp(fdt_compat, "st,", 3) != 0) {
				env_set("board_name", fdt_compat);
			} else {
				env_set("board_name", fdt_compat + 3);

				buf_len = sizeof(dtb_name);
				strlcpy(dtb_name, fdt_compat + 3, buf_len);
				buf_len -= strlen(fdt_compat + 3);
				strlcat(dtb_name, ".dtb", buf_len);
				env_set("fdtfile", dtb_name);
			}
		}
	}

	return 0;
}

void board_quiesce_devices(void)
{
	setup_led(LEDST_OFF);
}
