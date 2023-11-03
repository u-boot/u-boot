// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <init.h>
#include <log.h>
#include <misc.h>
#include <spl.h>
#include <syscon.h>
#include <u-boot/crc.h>
#include <usb.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/misc.h>
#include <linux/delay.h>
#include <power/regulator.h>
#include <u-boot/sha256.h>

/*
 * Swap mmc0 and mmc1 in boot_targets if booted from SD-Card.
 *
 * If bootsource is uSD-card we can assume that we want to use the
 * SD-Card instead of the eMMC as first boot_target for distroboot.
 * We only want to swap the defaults and not any custom environment a
 * user has set. We exit early if a changed boot_targets environment
 * is detected.
 */
static int setup_boottargets(void)
{
	const char *boot_device =
		ofnode_read_chosen_string("u-boot,spl-boot-device");
	char *env_default, *env;

	if (!boot_device) {
		debug("%s: /chosen/u-boot,spl-boot-device not set\n",
		      __func__);
		return -1;
	}
	debug("%s: booted from %s\n", __func__, boot_device);

	env_default = env_get_default("boot_targets");
	env = env_get("boot_targets");
	if (!env) {
		debug("%s: boot_targets does not exist\n", __func__);
		return -1;
	}
	debug("%s: boot_targets current: %s - default: %s\n",
	      __func__, env, env_default);

	if (strcmp(env_default, env) != 0) {
		debug("%s: boot_targets not default, don't change it\n",
		      __func__);
		return 0;
	}

	/*
	 * Make the default boot medium between SD Card and eMMC, the one that
	 * was used to load U-Boot proper.
	 */
	bool sd_booted = !strcmp(boot_device, "/mmc@ff370000");
	char *mmc0, *mmc1;

	debug("%s: booted from %s\n", __func__,
	      sd_booted ? "SD-Card" : "eMMC");
	mmc0 = strstr(env, "mmc0");
	mmc1 = strstr(env, "mmc1");

	if (!mmc0 || !mmc1) {
		debug("%s: only one mmc boot_target found\n", __func__);
		return -1;
	}

	/*
	 * If mmc0 comes first in the boot order and U-Boot proper was
	 * loaded from mmc1, swap mmc0 and mmc1 in the list.
	 * If mmc1 comes first in the boot order and U-Boot proper was
	 * loaded from mmc0, swap mmc0 and mmc1 in the list.
	 */
	if ((mmc0 < mmc1 && sd_booted) ||
	    (mmc0 > mmc1 && !sd_booted)) {
		mmc0[3] = '1';
		mmc1[3] = '0';
		debug("%s: set boot_targets to: %s\n", __func__, env);
		env_set("boot_targets", env);
	}

	return 0;
}

int mmc_get_env_dev(void)
{
	const char *boot_device =
		ofnode_read_chosen_string("u-boot,spl-boot-device");

	if (!boot_device) {
		debug("%s: /chosen/u-boot,spl-boot-device not set\n",
		      __func__);
		return CONFIG_SYS_MMC_ENV_DEV;
	}

	debug("%s: booted from %s\n", __func__, boot_device);

	if (!strcmp(boot_device, "/mmc@ff370000"))
		return 1;

	if (!strcmp(boot_device, "/mmc@ff390000"))
		return 0;

	return CONFIG_SYS_MMC_ENV_DEV;
}

enum env_location arch_env_get_location(enum env_operation op, int prio)
{
	const char *boot_device =
		ofnode_read_chosen_string("u-boot,spl-boot-device");

	if (prio > 0)
		return ENVL_UNKNOWN;

	if (!boot_device) {
		debug("%s: /chosen/u-boot,spl-boot-device not set\n",
		      __func__);
		return ENVL_NOWHERE;
	}

	debug("%s: booted from %s\n", __func__, boot_device);

	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC) &&
	    (!strcmp(boot_device, "/mmc@ff370000") ||
	     !strcmp(boot_device, "/mmc@ff390000")))
		return ENVL_MMC;

	printf("%s: No environment available: booted from %s but U-Boot "
	       "config does not allow loading environment from it.",
	       __func__, boot_device);

	return ENVL_NOWHERE;
}

int misc_init_r(void)
{
	const u32 cpuid_offset = 0x7;
	const u32 cpuid_length = 0x10;
	u8 cpuid[cpuid_length];
	int ret;

	ret = rockchip_cpuid_from_efuse(cpuid_offset, cpuid_length, cpuid);
	if (ret)
		return ret;

	ret = rockchip_cpuid_set(cpuid, cpuid_length);
	if (ret)
		return ret;

	ret = rockchip_setup_macaddr();
	if (ret)
		return ret;

	setup_boottargets();

	return 0;
}

#define STM32_RST	100 /* GPIO3_A4 */
#define STM32_BOOT	101 /* GPIO3_A5 */

void spl_board_init(void)
{
	/*
	 * Glitches on STM32_BOOT and STM32_RST lines during poweroff or power
	 * on may put the STM32 companion microcontroller into DFU mode, let's
	 * always reset it into normal mode instead.
	 * Toggling the STM32_RST line is safe to do with the ATtiny companion
	 * microcontroller variant because it will not trigger an MCU reset
	 * since only a UPDI reset command will. Since a UPDI reset is difficult
	 * to mistakenly trigger, glitches to the lines are theoretically also
	 * incapable of triggering an actual ATtiny reset.
	 */
	int ret;

	ret = gpio_request(STM32_RST, "STM32_RST");
	if (ret) {
		debug("Failed to request STM32_RST\n");
		return;
	}

	ret = gpio_request(STM32_BOOT, "STM32_BOOT");
	if (ret) {
		debug("Failed to request STM32_BOOT\n");
		return;
	}

	/* Rely on HW pull-down for inactive level */
	ret = gpio_direction_input(STM32_BOOT);
	if (ret) {
		debug("Failed to configure STM32_BOOT as input\n");
		return;
	}

	ret = gpio_direction_output(STM32_RST, 0);
	if (ret) {
		debug("Failed to configure STM32_RST as output low\n");
		return;
	}

	mdelay(1);

	ret = gpio_direction_output(STM32_RST, 1);
	if (ret) {
		debug("Failed to configure STM32_RST as output high\n");
		return;
	}
}
