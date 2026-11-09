// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2025 Linaro Ltd. */

#include <linux/kconfig.h>
#include <lwip/apps/tftp_common.h>
#include <lwip/icmp.h>
#include <lwip/ip.h>
#include <lwip/ip4_addr.h>
#include <lwip/pbuf.h>
#include <lwip/prot/ip4.h>
#include <lwip/prot/udp.h>

static void tftp_port_unreachable(struct pbuf *p, u16 offset)
{
	struct ip_hdr quoted;
	struct udp_hdr udp;
	ip4_addr_t local;
	ip_addr_t peer;
	u16 hlen;

	/* RFC 792 quotes the original IP header and at least eight data bytes.
	 * They may cross pbuf boundaries, and IP options change the UDP offset.
	 */
	if (pbuf_copy_partial(p, &quoted, sizeof(quoted), offset) != sizeof(quoted))
		return;
	hlen = IPH_HL_BYTES(&quoted);
	if (IPH_V(&quoted) != 4 || hlen < IP_HLEN ||
	    IPH_PROTO(&quoted) != IP_PROTO_UDP ||
	    (lwip_ntohs(IPH_OFFSET(&quoted)) & IP_OFFMASK) ||
	    lwip_ntohs(IPH_LEN(&quoted)) < hlen + UDP_HLEN)
		return;
	if (pbuf_copy_partial(p, &udp, sizeof(udp), offset + hlen) != sizeof(udp) ||
	    lwip_ntohs(udp.len) < UDP_HLEN)
		return;
	ip4_addr_copy(local, quoted.src);
	if (!ip4_addr_cmp(&local, netif_ip4_addr(ip_current_netif())) ||
	    !ip4_addr_cmp(&local, ip4_current_dest_addr()))
		return;
	ip_addr_copy_from_ip4(peer, quoted.dest);
	tftp_handle_port_unreachable(&peer, lwip_ntohs(udp.src), lwip_ntohs(udp.dest));
}

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

	if (IS_ENABLED(CONFIG_LWIP_ICMP_SHOW_UNREACH)) {
		ip4_addr_copy(src, iphdr->src);
		printf("ICMP destination unreachable (%s) from %s\n",
		       code_to_str(code), ip4addr_ntoa(&src));
	}
	if (IS_ENABLED(CONFIG_CMD_TFTPBOOT) && code == ICMP_DUR_PORT)
		tftp_port_unreachable(p, IPH_HL_BYTES(iphdr) + sizeof(struct icmp_hdr));
}
