// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2018 Lothar Felten, lothar.felten@gmail.com */

#include <env.h>
#include <net.h>
#include <net/wol.h>
#include <vsprintf.h>

bool wol_check_magic(const void *packet, unsigned int len, const u8 *mac)
{
	const struct wol_hdr *wol = packet;
	int i;

	if (len < sizeof(*wol))
		return false;

	for (i = 0; i < WOL_SYNC_COUNT; i++)
		if (wol->wol_sync[i] != WOL_SYNC_BYTE)
			return false;

	for (i = 0; i < WOL_MAC_REPETITIONS; i++)
		if (memcmp(&wol->wol_dest[i * ARP_HLEN], mac, ARP_HLEN))
			return false;

	return true;
}

void wol_save_password(const void *packet, unsigned int len)
{
	const struct wol_hdr *wol = packet;

	/* Ethernet padding may follow the optional ether-wake password. */
	if (len >= sizeof(*wol) + WOL_PASSWORD_6B) {
		char buffer[ARP_HLEN_ASCII + 1];

		sprintf(buffer, "%pM", wol->wol_passwd);
		env_set("wolpassword", buffer);
	} else if (len >= sizeof(*wol) + WOL_PASSWORD_4B) {
		struct in_addr ip;
		char buffer[16];

		memcpy(&ip, wol->wol_passwd, sizeof(ip));
		ip_to_string(ip, buffer);
		env_set("wolpassword", buffer);
	}
}
