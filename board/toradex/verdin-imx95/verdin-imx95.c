// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Toradex */

#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <env.h>
#include <errno.h>
#include <fdt_support.h>
#include <init.h>
#include <stdio.h>
#include <string.h>

#include "../common/tdx-cfg-block.h"
#include "../common/tdx-common.h"

DECLARE_GLOBAL_DATA_PTR;

static void select_dt_from_module_version(void)
{
	char variant[32];
	char *env_variant = env_get("variant");
	bool is_wifi = false;

	if (IS_ENABLED(CONFIG_TDX_CFG_BLOCK)) {
		/*
		 * If we have a valid config block and it says we are a
		 * module with Wi-Fi/Bluetooth make sure we use the -wifi
		 * device tree.
		 */
		is_wifi = (tdx_hw_tag.prodid == VERDIN_IMX95H_8G_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_IMX95H_4G_WB_IT);
	}

	if (is_wifi)
		strlcpy(&variant[0], "wifi", sizeof(variant));
	else
		strlcpy(&variant[0], "nonwifi", sizeof(variant));

	if (!env_variant || strcmp(variant, env_variant)) {
		printf("Setting variant to %s\n", variant);
		env_set("variant", variant);
	}
}

int board_late_init(void)
{
	select_dt_from_module_version();

	return 0;
}

static const struct ram_alias_check ram_alias_checks[] = {
	{ (void *)(PHYS_SDRAM + SZ_8G), (void *)(PHYS_SDRAM), SZ_16G },
	{ (void *)(PHYS_SDRAM + SZ_4G), (void *)(PHYS_SDRAM), SZ_8G },
	{ (void *)(PHYS_SDRAM + SZ_2G), (void *)(PHYS_SDRAM), SZ_4G },
	{ (void *)(PHYS_SDRAM + SZ_1G), (void *)(PHYS_SDRAM), SZ_2G },
	{ NULL }
};

int board_phys_sdram_size(phys_size_t *size)
{
	phys_size_t sz;

	sz = probe_ram_size_by_alias(ram_alias_checks);
	if (!sz) {
		puts("## WARNING: Less than 2GB RAM detected\n");
		return -EINVAL;
	}

	*size = sz - PHYS_SDRAM_FW_RSVD;

	return 0;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif
