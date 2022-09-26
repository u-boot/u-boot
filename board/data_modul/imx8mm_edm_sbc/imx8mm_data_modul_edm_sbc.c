// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <i2c_eeprom.h>
#include <malloc.h>
#include <net.h>
#include <spl.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

int mach_cpu_init(void)
{
	icache_enable();
	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	u8 memcfg = dmo_get_memcfg();

	*size = (4ULL >> ((memcfg >> 1) & 0x3)) * SZ_1G;

	return 0;
}

static void setup_mac_address(void)
{
	unsigned char enetaddr[6];
	struct udevice *dev;
	int off, ret;

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)	/* ethaddr is already set */
		return;

	off = fdt_path_offset(gd->fdt_blob, "eeprom0");
	if (off < 0) {
		printf("%s: No eeprom0 path offset\n", __func__);
		return;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		printf("Cannot find EEPROM!\n");
		return;
	}

	ret = i2c_eeprom_read(dev, 0xb0, enetaddr, 0x6);
	if (ret) {
		printf("Error reading configuration EEPROM!\n");
		return;
	}

	if (is_valid_ethaddr(enetaddr))
		eth_env_set_enetaddr("ethaddr", enetaddr);
}

static void setup_boot_device(void)
{
	int boot_device = get_boot_device();
	char *devnum;

	devnum = env_get("devnum");
	if (devnum)	/* devnum is already set */
		return;

	if (boot_device == MMC3_BOOT)	/* eMMC */
		env_set_ulong("devnum", 0);
	else
		env_set_ulong("devnum", 1);
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	struct udevice *dev;
	int ret;

	setup_boot_device();
	setup_mac_address();

	ret = uclass_get_device_by_name(UCLASS_MISC, "usb-hub@2c", &dev);
	if (ret)
		printf("Error bringing up USB hub (%d)\n", ret);

	return 0;
}
