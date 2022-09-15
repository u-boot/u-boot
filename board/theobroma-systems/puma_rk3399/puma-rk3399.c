// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
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
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/misc.h>
#include <power/regulator.h>
#include <u-boot/sha256.h>

static void setup_iodomain(void)
{
	const u32 GRF_IO_VSEL_GPIO4CD_SHIFT = 3;
	struct rk3399_grf_regs *grf =
	    syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	/*
	 * Set bit 3 in GRF_IO_VSEL so PCIE_RST# works (pin GPIO4_C6).
	 * Linux assumes that PCIE_RST# works out of the box as it probes
	 * PCIe before loading the iodomain driver.
	 */
	rk_setreg(&grf->io_vsel, 1 << GRF_IO_VSEL_GPIO4CD_SHIFT);
}

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
	 * was used to load U-Boot proper. If SPI-NOR flash was used, keep
	 * original default order.
	 */
	if (strcmp(boot_device, "/spi@ff1d0000/flash@0")) {
		bool sd_booted = !strcmp(boot_device, "/mmc@fe320000");
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

	if (!strcmp(boot_device, "/mmc@fe320000"))
		return 1;

	if (!strcmp(boot_device, "/mmc@fe330000"))
		return 0;

	return CONFIG_SYS_MMC_ENV_DEV;
}

#if !IS_ENABLED(CONFIG_ENV_IS_NOWHERE)
#error Please enable CONFIG_ENV_IS_NOWHERE
#endif

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

	if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH) &&
	    !strcmp(boot_device, "/spi@ff1d0000/flash@0"))
		return ENVL_SPI_FLASH;

	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC) &&
	    (!strcmp(boot_device, "/mmc@fe320000") ||
	     !strcmp(boot_device, "/mmc@fe330000")))
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

	setup_iodomain();
	setup_boottargets();

	return 0;
}
