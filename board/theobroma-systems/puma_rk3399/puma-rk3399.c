// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <misc.h>
#include <spl.h>
#include <syscon.h>
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
		ofnode_get_chosen_prop("u-boot,spl-boot-device");
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
	 * Only run, if booting from mmc1 (i.e. /dwmmc@fe320000) and
	 * only consider cases where the default boot-order first
	 * tries to boot from mmc0 (eMMC) and then from mmc1
	 * (i.e. external SD).
	 *
	 * In other words: the SD card will be moved to earlier in the
	 * order, if U-Boot was also loaded from the SD-card.
	 */
	if (!strcmp(boot_device, "/dwmmc@fe320000")) {
		char *mmc0, *mmc1;

		debug("%s: booted from SD-Card\n", __func__);
		mmc0 = strstr(env, "mmc0");
		mmc1 = strstr(env, "mmc1");

		if (!mmc0 || !mmc1) {
			debug("%s: only one mmc boot_target found\n", __func__);
			return -1;
		}

		/*
		 * If mmc0 comes first in the boot order, we need to change
		 * the strings to make mmc1 first.
		 */
		if (mmc0 < mmc1) {
			mmc0[3] = '1';
			mmc1[3] = '0';
			debug("%s: set boot_targets to: %s\n", __func__, env);
			env_set("boot_targets", env);
		}
	}

	return 0;
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

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial_string;
	u64 serial = 0;

	serial_string = env_get("serial#");

	if (serial_string)
		serial = simple_strtoull(serial_string, NULL, 16);

	serialnr->high = (u32)(serial >> 32);
	serialnr->low = (u32)(serial & 0xffffffff);
}
#endif

/**
 * Switch power at an external regulator (for our root hub).
 *
 * @param ctrl pointer to the xHCI controller
 * @param port port number as in the control message (one-based)
 * @param enable boolean indicating whether to enable or disable power
 * @return returns 0 on success, an error-code on failure
 */
static int board_usb_port_power_set(struct udevice *dev, int port,
				    bool enable)
{
#if CONFIG_IS_ENABLED(OF_CONTROL) && CONFIG_IS_ENABLED(DM_REGULATOR)
	/* We start counting ports at 0, while USB counts from 1. */
	int index = port - 1;
	const char *regname = NULL;
	struct udevice *regulator;
	const char *prop = "tsd,usb-port-power";
	int ret;

	debug("%s: ctrl '%s' port %d enable %s\n", __func__,
	      dev_read_name(dev), port, enable ? "true" : "false");

	ret = dev_read_string_index(dev, prop, index, &regname);
	if (ret < 0) {
		debug("%s: ctrl '%s' port %d: no entry in '%s'\n",
		      __func__, dev_read_name(dev), port, prop);
		return ret;
	}

	ret = regulator_get_by_platname(regname, &regulator);
	if (ret) {
		debug("%s: ctrl '%s' port %d: could not get regulator '%s'\n",
		      __func__, dev_read_name(dev), port, regname);
		return ret;
	}

	regulator_set_enable(regulator, enable);
	return 0;
#else
	return -ENOTSUPP;
#endif
}

void usb_hub_reset_devices(struct usb_hub_device *hub, int port)
{
	struct udevice *dev = hub->pusb_dev->dev;
	struct udevice *ctrl;

	/* We are only interested in our root-hubs */
	if (usb_hub_is_root_hub(dev) == false)
		return;

	ctrl = usb_get_bus(dev);
	if (!ctrl) {
		debug("%s: could not retrieve ctrl for hub\n", __func__);
		return;
	}

	/*
	 * To work around an incompatibility between the single-threaded
	 * USB stack in U-Boot and (a strange low-power mode of) the USB
	 * hub we have on-module, we need to delay powering on the hub
	 * until the first time the port is probed.
	 */
	board_usb_port_power_set(ctrl, port, true);
}
