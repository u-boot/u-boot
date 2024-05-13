// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <dm/lists.h>

#include "ethernet.h"

int board_init(void)
{
	if (IS_ENABLED(CONFIG_SYSRESET_CV1800B))
		device_bind_driver(gd->dm_root, "cv1800b_sysreset", "sysreset", NULL);

	if (IS_ENABLED(CONFIG_NET))
		cv1800b_ephy_init();

	return 0;
}
