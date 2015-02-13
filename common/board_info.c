/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <linux/compiler.h>

int __weak checkboard(void)
{
	printf("Board: Unknown\n");
	return 0;
}

/*
 * If the root node of the DTB has a "model" property, show it.
 * If CONFIG_OF_CONTROL is disabled or the "model" property is missing,
 * fall back to checkboard().
 */
int show_board_info(void)
{
#ifdef CONFIG_OF_CONTROL
	DECLARE_GLOBAL_DATA_PTR;
	const char *model;

	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);

	if (model) {
		printf("Model: %s\n", model);
		return 0;
	}
#endif

	return checkboard();
}
