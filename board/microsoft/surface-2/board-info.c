// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2025
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <stdio.h>
#include <env.h>
#include <spl_gpio.h>

#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <linux/string.h>

static int id_gpio_get_value(u32 pingrp, u32 pin)
{
	/* Configure pinmux */
	pinmux_set_func(pingrp, PMUX_FUNC_KBC);
	pinmux_set_pullupdown(pingrp, PMUX_PULL_DOWN);
	pinmux_tristate_enable(pingrp);
	pinmux_set_io(pingrp, PMUX_PIN_INPUT);

	/*
	 * Since this function may be called
	 * during DM reload we should use SPL
	 * GPIO functions which do not depend
	 * on DM.
	 */
	spl_gpio_input(NULL, pin);
	return spl_gpio_get_value(NULL, pin);
}

static int get_board_id(void)
{
	u32 pcb_id0, pcb_id1, pcb_id2, pcb_id3, pcb_id4, board_id;

	pcb_id0 = id_gpio_get_value(PMUX_PINGRP_KB_COL0_PQ0, TEGRA_GPIO(Q, 0));
	pcb_id1 = id_gpio_get_value(PMUX_PINGRP_KB_COL1_PQ1, TEGRA_GPIO(Q, 1));
	pcb_id2 = id_gpio_get_value(PMUX_PINGRP_KB_COL2_PQ2, TEGRA_GPIO(Q, 2));
	pcb_id3 = id_gpio_get_value(PMUX_PINGRP_KB_COL3_PQ3, TEGRA_GPIO(Q, 3));
	pcb_id4 = id_gpio_get_value(PMUX_PINGRP_KB_COL4_PQ4, TEGRA_GPIO(Q, 4));

	/* Construct board ID */
	board_id = pcb_id4 << 4 | pcb_id3 << 3 | pcb_id2 << 2 | pcb_id1 << 1 | pcb_id0;

	log_debug("[SURFACE-2]: Board ID %02x\n", board_id);

	return board_id & 0x1f;
}

int board_fit_config_name_match(const char *name)
{
	char dt_name[64] = { 0 };

	snprintf(dt_name, sizeof(dt_name), "tegra114-microsoft-surface-2-%02x.dtb",
		 get_board_id());

	if (!strcmp(name, dt_name))
		return 0;

	return -1;
}

void nvidia_board_late_init(void)
{
	char dt_path[64] = { 0 };

	snprintf(dt_path, sizeof(dt_path), "tegra114-microsoft-surface-2-%02x.dtb",
		 get_board_id());
	env_set("fdtfile", dt_path);
}
