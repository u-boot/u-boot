/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __NETCONSOLE_INTERNAL_H
#define __NETCONSOLE_INTERNAL_H

#include <linux/types.h>
#include <net-common.h>
#if CONFIG_IS_ENABLED(NET_LWIP) && CONFIG_IS_ENABLED(IPV6)
#include <lwip/ip6_addr.h>
#endif

struct nc_settings {
	struct in_addr ip;
#if CONFIG_IS_ENABLED(NET_LWIP) && CONFIG_IS_ENABLED(IPV6)
	ip6_addr_t ip6;
	bool ipv6;
#endif
	u16 out_port;
	u16 in_port;
};

extern struct nc_settings nc_settings;

bool nc_has_input(void);
int nc_input_data(const uchar *pkt, unsigned int len);
bool nc_is_broadcast(struct in_addr ip);
int nc_refresh_settings_from_env(void);
int nc_input_packet(uchar *pkt, struct in_addr src_ip, unsigned int dest_port,
		    unsigned int src_port, unsigned int len);
void nc_set_input_recursion(bool enable);

int nc_transport_start(void);
int nc_transport_stop(void);
void nc_transport_send(const char *buf, int len);
void nc_transport_poll(bool block);

#endif /* __NETCONSOLE_INTERNAL_H */
