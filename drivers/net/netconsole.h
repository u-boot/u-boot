/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __NETCONSOLE_INTERNAL_H
#define __NETCONSOLE_INTERNAL_H

#include <linux/types.h>
#include <net-common.h>

struct nc_settings {
	struct in_addr ip;
	short out_port;
	short in_port;
};

extern struct nc_settings nc_settings;

bool nc_has_input(void);
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
