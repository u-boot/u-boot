// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <dm.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <efi_loader.h>
#include <asm/io.h>
#include <asm/arch/gx.h>
#include <asm/arch/sm.h>
#include <asm/arch/eth.h>
#include <asm/arch/mem.h>

#define EFUSE_SN_OFFSET		20
#define EFUSE_SN_SIZE		16
#define EFUSE_MAC_OFFSET	52
#define EFUSE_MAC_SIZE		6

struct efi_fw_image fw_images[] = {
	{
		.fw_name = u"AML_S805X_AC_BOOT",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "sf 0:0=u-boot-bin raw 0 0x10000",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#if IS_ENABLED(CONFIG_SET_DFU_ALT_INFO)
void set_dfu_alt_info(char *interface, char *devstr)
{
	if (interface && strcmp(interface, "ram") == 0)
		env_set("dfu_alt_info", "fitimage ram 0x08080000 0x4000000");
}
#endif

int misc_init_r(void)
{
	u8 mac_addr[EFUSE_MAC_SIZE + 1];
	char serial[EFUSE_SN_SIZE + 1];
	ssize_t len;

	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
		len = meson_sm_read_efuse(EFUSE_MAC_OFFSET,
					  mac_addr, EFUSE_MAC_SIZE);
		mac_addr[len] = '\0';
		if (len == EFUSE_MAC_SIZE && is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
		else
			meson_generate_serial_ethaddr();
	}

	if (!env_get("serial#")) {
		len = meson_sm_read_efuse(EFUSE_SN_OFFSET, serial,
			EFUSE_SN_SIZE);
		serial[len] = '\0';
		if (len == EFUSE_SN_SIZE)
			env_set("serial#", serial);
	}

	return 0;
}
