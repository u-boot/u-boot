// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2024
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <env.h>
#include <spl_gpio.h>

#include <asm/gpio.h>
#include <asm/arch/pinmux.h>

/*
 *	PMIC_ID is GMI_CS2_N_PK3
 *	MODEM_ID is GMI_CS4_N_PK2
 *
 *		Extended Project ID
 *	====================================
 *	MODEM_ID  PMIC_ID	project name
 *	0	  0		grouper-E1565
 *	0	  1		grouper-PM269
 *	1	  0		tilapia
 */
enum project_rev {
	E1565, PM269, TILAPIA, COUNT,
};

static const char * const project_id_to_fdt[] = {
	[E1565] = "tegra30-asus-nexus7-grouper-E1565",
	[PM269] = "tegra30-asus-nexus7-grouper-PM269",
	[TILAPIA] = "tegra30-asus-nexus7-tilapia-E1565",
};

static int id_gpio_get_value(u32 pingrp, u32 pin)
{
	/* Configure pinmux */
	pinmux_set_func(pingrp, PMUX_FUNC_GMI);
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

static int get_project_id(void)
{
	u32 pmic_id, modem_id, proj_id;

	modem_id = id_gpio_get_value(PMUX_PINGRP_GMI_CS4_N_PK2,
				     TEGRA_GPIO(K, 2));
	pmic_id = id_gpio_get_value(PMUX_PINGRP_GMI_CS2_N_PK3,
				    TEGRA_GPIO(K, 3));

	proj_id = (modem_id << 1 | pmic_id) & COUNT;

	log_debug("[GROUPER]: project id %d (%s)\n", proj_id,
		  project_id_to_fdt[proj_id]);

	return proj_id;
}

int board_fit_config_name_match(const char *name)
{
	if (!strcmp(name, project_id_to_fdt[get_project_id()]))
		return 0;

	return -1;
}

void nvidia_board_late_init(void)
{
	char dt_path[64] = { 0 };

	snprintf(dt_path, sizeof(dt_path), "%s.dtb",
		 project_id_to_fdt[get_project_id()]);
	env_set("fdtfile", dt_path);
}
