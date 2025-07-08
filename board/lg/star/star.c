// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2023
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/root.h>
#include <fdt_support.h>
#include <log.h>
#include <spl_gpio.h>
#include <asm/gpio.h>

static int star_fix_panel(void *fdt)
{
	int panel_offset, ret;

	/* Patch panel compatible */
	spl_gpio_input(NULL, TEGRA_GPIO(J, 5));
	if (spl_gpio_get_value(NULL, TEGRA_GPIO(J, 5))) {
		panel_offset = fdt_node_offset_by_compatible(fdt, -1,
							     "hit,tx10d07vm0baa");
		if (panel_offset < 0) {
			log_debug("%s: panel node not found\n", __func__);
			return panel_offset;
		}

		ret = fdt_setprop_string(fdt, panel_offset, "compatible",
					 "lg,lh400wv3-sd04");
		if (ret) {
			log_debug("%s: panel comapible patch failed\n", __func__);
			return ret;
		}
	}

	return 0;
}

void pinmux_init(void)
{
	void *fdt = (void *)gd->fdt_blob;

	star_fix_panel(fdt);
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *fdt, struct bd_info *bd)
{
	return star_fix_panel(fdt);
}
#endif
