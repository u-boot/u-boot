// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_BOARD

#include <common.h>
#include <config.h>
#include <env.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <asm/arch/sys_proto.h>

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

/* board dependent setup after realloc */
int board_init(void)
{
	return 0;
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
