// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include "../common/ethernet.h"

int board_init(void)
{
	if (IS_ENABLED(CONFIG_NET_LEGACY))
		cv1800b_ephy_init();

	return 0;
}
