// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023-2024 Collabora Ltd.
 */

#include <fdtdec.h>
#include <fdt_support.h>

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	if (IS_ENABLED(CONFIG_TYPEC_FUSB302))
		fdt_status_okay_by_compatible(blob, "fcs,fusb302");
	return 0;
}
#endif
