// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

int __weak checkboard(void)
{
	return 0;
}

/*
 * Check sysinfo for board information. Failing that if the root node of the DTB
 * has a "model" property, show it.
 *
 * Then call checkboard().
 */
int __weak show_board_info(void)
{
	if (IS_ENABLED(CONFIG_OF_CONTROL)) {
		struct udevice *dev;
		const char *model;
		char str[80];
		int ret = -ENOSYS;

		if (IS_ENABLED(CONFIG_SYSINFO)) {
			/* This might provide more detail */
			ret = uclass_first_device_err(UCLASS_SYSINFO, &dev);
			if (!ret)
				ret = sysinfo_get_str(dev,
						      SYSINFO_ID_BOARD_MODEL,
						      sizeof(str), str);
		}

		/* Fail back to the main 'model' if available */
		if (ret)
			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
		else
			model = str;

		if (model)
			printf("Model: %s\n", model);
	}

	return checkboard();
}
