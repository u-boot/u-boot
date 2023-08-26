// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2021
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

/* T30 Transformers derive from Cardhu board */

#include <dm.h>
#include <fdt_support.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gp_padctrl.h>

#include "pinmux-config-transformer.h"

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_config_pingrp_table(transformer_pinmux_common,
		ARRAY_SIZE(transformer_pinmux_common));

	pinmux_config_drvgrp_table(transformer_padctrl,
		ARRAY_SIZE(transformer_padctrl));

	if (of_machine_is_compatible("asus,tf700t")) {
		pinmux_config_pingrp_table(tf700t_mipi_pinmux,
			ARRAY_SIZE(tf700t_mipi_pinmux));
	}
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	/* Remove TrustZone nodes */
	fdt_del_node_and_alias(blob, "/firmware");
	fdt_del_node_and_alias(blob, "/reserved-memory/trustzone@bfe00000");

	return 0;
}
#endif
