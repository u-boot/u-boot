// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2025 Linaro Ltd. */

#include <lwip/icmp.h>
#include <lwip/ip4_addr.h>
#include <lwip/pbuf.h>
#include <lwip/prot/ip4.h>

static const char *code_to_str(int code)
{
	switch (code) {
	case ICMP_DUR_NET:
		return "network unreachable";
	case ICMP_DUR_HOST:
		return "host unreachable";
	case ICMP_DUR_PROTO:
		return "protocol unreachable";
	case ICMP_DUR_PORT:
		return "port unreachable";
	case ICMP_DUR_FRAG:
		return "fragmentation needed and DF set";
	case ICMP_DUR_SR:
		return "source route failed";
	default:
		break;
	}
	return "unknown cause";
}

void net_lwip_icmp_dest_unreach(int code, struct pbuf *p)
{
	struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
	ip4_addr_t src;

	ip4_addr_copy(src, iphdr->src);
	printf("ICMP destination unreachable (%s) from %s\n",
	       code_to_str(code), ip4addr_ntoa(&src));
}
