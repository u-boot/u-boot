// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Lothar Felten, lothar.felten@gmail.com
 */

#include <command.h>
#include <net.h>
#include <net/wol.h>

static ulong wol_timeout;

void wol_receive(struct ip_udp_hdr *ip, unsigned int len)
{
	struct wol_hdr *wol;

	wol = (struct wol_hdr *)ip;

	if (!wol_check_magic(wol, len, net_ethaddr))
		return;

	wol_save_password(wol, len);
	net_set_state(NETLOOP_SUCCESS);
}

static void wol_udp_handler(uchar *pkt, unsigned int dest, struct in_addr sip,
			    unsigned int src, unsigned int len)
{
	struct wol_hdr *wol;

	wol = (struct wol_hdr *)pkt;

	/* UDP destination port must be 0, 7 or 9 */
	if (dest != 0 && dest != 7 && dest != 9)
		return;

	if (!wol_check_magic(wol, len, net_ethaddr))
		return;

	net_set_state(NETLOOP_SUCCESS);
}

int wol_wait(ulong timeout)
{
	wol_timeout = timeout;
	return net_loop(WOL);
}

static void wol_timeout_handler(void)
{
	eth_halt();
	net_set_state(NETLOOP_FAIL);
}

void wol_start(void)
{
	net_set_timeout_handler(wol_timeout, wol_timeout_handler);
	net_set_udp_handler(wol_udp_handler);
}
