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
 *	PCB_ID[3] is kb_col7_pq7
 *	PCB_ID[4] is kb_row2_pr2
 *	PCB_ID[5] is kb_col5_pq5
 *	PCB_ID[7] is gmi_cs1_n_pj2
 *
 *			  Project ID
 *	=====================================================
 *	PCB_ID[7] PCB_ID[5] PCB_ID[4] PCB_ID[3]	Project
 *	0	  0	    0	      0		TF201
 *	0	  0	    0	      1		P1801
 *	0	  0	    1	      0		TF300T
 *	0	  0	    1	      1		TF300TG
 *	0	  1	    0	      0		TF700T
 *	0	  1	    0	      1		TF300TL
 *	0	  1	    1	      0		Extension
 *	0	  1	    1	      1		TF500T
 *	1	  0	    0	      0		TF502T/TF600T
 *	=====================================================
 */
enum project_rev {
	TF201, P1801, TF300T, TF300TG, TF700T,
	TF300TL, EXT, TF500T, TF600T
};

static const char * const project_id_to_fdt[] = {
	[TF201] = "tegra30-asus-tf201",
	[P1801] = "tegra30-asus-p1801-t",
	[TF300T] = "tegra30-asus-tf300t",
	[TF300TG] = "tegra30-asus-tf300tg",
	[TF700T] = "tegra30-asus-tf700t",
	[TF300TL] = "tegra30-asus-tf300tl",
	[TF600T] = "tegra30-asus-tf600t",
};

static int id_gpio_get_value(u32 pingrp, u32 func, u32 pin)
{
	/* Configure pinmux */
	pinmux_set_func(pingrp, func);
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
	u32 pcb_id3, pcb_id4, pcb_id5, pcb_id7;

	pcb_id3 = id_gpio_get_value(PMUX_PINGRP_KB_COL7_PQ7,
				    PMUX_FUNC_KBC, TEGRA_GPIO(Q, 7));
	pcb_id4 = id_gpio_get_value(PMUX_PINGRP_KB_ROW2_PR2,
				    PMUX_FUNC_KBC, TEGRA_GPIO(R, 2));
	pcb_id5 = id_gpio_get_value(PMUX_PINGRP_KB_COL5_PQ5,
				    PMUX_FUNC_KBC, TEGRA_GPIO(Q, 5));
	pcb_id7 = id_gpio_get_value(PMUX_PINGRP_GMI_CS1_N_PJ2,
				    PMUX_FUNC_RSVD1, TEGRA_GPIO(J, 2));

	/* Construct board ID */
	int proj_id = pcb_id7 << 3 | pcb_id5 << 2 |
		      pcb_id4 << 1 | pcb_id3;

	log_debug("[TRANSFORMER]: project id %d (%s)\n", proj_id,
		  project_id_to_fdt[proj_id]);

	/* Mark tablet with SPI flash */
	if (proj_id == TF600T)
		env_set_hex("spiflash", true);
	else
		env_set_hex("spiflash", false);

	return proj_id & 0xf;
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
