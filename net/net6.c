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
