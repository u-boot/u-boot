// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Marek Behún <kabel@kernel.org>
 */

#include <env.h>
#include <net.h>

#include "turris_common.h"

static void increment_mac(u8 *mac)
{
	int i;

	for (i = 5; i >= 3; i--) {
		mac[i] += 1;
		if (mac[i])
			break;
	}
}

static void set_mac_if_invalid(int i, u8 *mac)
{
	u8 oldmac[6];

	if (is_valid_ethaddr(mac) &&
	    !eth_env_get_enetaddr_by_index("eth", i, oldmac))
		eth_env_set_enetaddr_by_index("eth", i, mac);
}

void turris_init_mac_addresses(int first_idx, const u8 *first_mac)
{
	u8 mac[6];

	memcpy(mac, first_mac, sizeof(mac));

	set_mac_if_invalid((first_idx + 0) % 3, mac);
	increment_mac(mac);
	set_mac_if_invalid((first_idx + 1) % 3, mac);
	increment_mac(mac);
	set_mac_if_invalid((first_idx + 2) % 3, mac);
}
