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
#include <asm/arch/eth.h>

struct efi_fw_image fw_images[] = {
	{
		.fw_name = u"AML_S905D3_CC_BOOT",
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
	meson_generate_serial_ethaddr();

	return 0;
}
