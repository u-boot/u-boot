// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Allied Telesis Labs NZ
 * Chris Packham, <judge.packham@gmail.com>
 *
 * Copyright (C) 2022 YADRO
 * Viacheslav Mitrofanov <v.v.mitrofanov@yadro.com>
 */

/* Simple IPv6 network layer implementation */

#include <common.h>
#include <env_internal.h>
#include <malloc.h>
#include <net.h>
#include <net6.h>

/* NULL IPv6 address */
struct in6_addr const net_null_addr_ip6 = ZERO_IPV6_ADDR;
/* Our gateway's IPv6 address */
struct in6_addr net_gateway6 = ZERO_IPV6_ADDR;
/* Our IPv6 addr (0 = unknown) */
struct in6_addr net_ip6 = ZERO_IPV6_ADDR;
/* Our link local IPv6 addr (0 = unknown) */
struct in6_addr net_link_local_ip6 = ZERO_IPV6_ADDR;
/* set server IPv6 addr (0 = unknown) */
struct in6_addr net_server_ip6 = ZERO_IPV6_ADDR;
/* The prefix length of our network */
u32 net_prefix_length;

bool use_ip6;

static int on_ip6addr(const char *name, const char *value, enum env_op op,
		      int flags)
{
	char *mask;
	size_t len;

	if (flags & H_PROGRAMMATIC)
		return 0;

	if (op == env_op_delete) {
		net_prefix_length = 0;
		net_copy_ip6(&net_ip6, &net_null_addr_ip6);
		return 0;
	}

	mask = strchr(value, '/');
	len = strlen(value);

	if (mask)
		net_prefix_length = simple_strtoul(value + len, NULL, 10);

	return string_to_ip6(value, len, &net_ip6);
}

U_BOOT_ENV_CALLBACK(ip6addr, on_ip6addr);

static int on_gatewayip6(const char *name, const char *value, enum env_op op,
			 int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	return string_to_ip6(value, strlen(value), &net_gateway6);
}

U_BOOT_ENV_CALLBACK(gatewayip6, on_gatewayip6);

static int on_serverip6(const char *name, const char *value, enum env_op op,
			int flags)
{
	if (flags & H_PROGRAMMATIC)
		return 0;

	return string_to_ip6(value, strlen(value), &net_server_ip6);
}

U_BOOT_ENV_CALLBACK(serverip6, on_serverip6);
