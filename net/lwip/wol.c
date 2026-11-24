// SPDX-License-Identifier: GPL-2.0+

#include <console.h>
#include <net.h>
#include <net/wol.h>
#include <time.h>
#include <asm/unaligned.h>
#include <lwip/inet_chksum.h>
#include <lwip/prot/ip4.h>
#include <lwip/prot/udp.h>

static struct netif *wol_netif;
static bool wol_received;

static void wol_receive_udp(struct netif *netif, const u8 *packet, unsigned int len)
{
	struct ip_hdr ip;
	struct udp_hdr udp;
	ip4_addr_t src, dst;
	unsigned int hlen, iplen, udplen;
	struct pbuf p = {};
	u16 port;

	if (len < sizeof(ip))
		return;
	memcpy(&ip, packet, sizeof(ip));
	hlen = IPH_HL_BYTES(&ip);
	iplen = ntohs(IPH_LEN(&ip));
	if (IPH_V(&ip) != 4 || hlen < sizeof(ip) || iplen > len ||
	    iplen < hlen + sizeof(udp) || IPH_PROTO(&ip) != IPPROTO_UDP)
		return;
	/* Do not mistake a fragment's payload for a UDP header or magic packet. */
	if (IPH_OFFSET(&ip) & htons(IP_OFFMASK | IP_MF))
		return;
	if (inet_chksum(packet, hlen))
		return;

	ip4_addr_copy(src, ip.src);
	ip4_addr_copy(dst, ip.dest);
	if (!ip4_addr_isany_val(*netif_ip4_addr(netif)) &&
	    !ip4_addr_eq(&dst, netif_ip4_addr(netif)) &&
	    !ip4_addr_isbroadcast(&dst, netif))
		return;

	packet += hlen;
	memcpy(&udp, packet, sizeof(udp));
	port = ntohs(udp.dest);
	udplen = ntohs(udp.len);
	if ((port != 0 && port != 7 && port != 9) ||
	    udplen < sizeof(udp) || udplen > iplen - hlen)
		return;

	/* A read-only pbuf view lets lwIP verify the complete UDP checksum. */
	p.payload = (void *)packet;
	p.len = udplen;
	p.tot_len = udplen;
	if (udp.chksum && inet_chksum_pseudo(&p, IPPROTO_UDP, udplen, &src, &dst))
		return;

	/* As in the legacy command, only Ethernet packets export a password. */
	wol_received = wol_check_magic(packet + sizeof(udp), udplen - sizeof(udp),
				       netif->hwaddr);
}

void net_lwip_wol_receive(struct netif *netif, const u8 *packet, int len)
{
	u16 protocol;

	if (netif != wol_netif || wol_received || len < ETHER_HDR_SIZE)
		return;
	protocol = get_unaligned_be16(packet + 2 * ARP_HLEN);
	packet += ETHER_HDR_SIZE;
	len -= ETHER_HDR_SIZE;

	switch (protocol) {
	case PROT_WOL:
		if (wol_check_magic(packet, len, netif->hwaddr)) {
			wol_save_password(packet, len);
			wol_received = true;
		}
		break;
	case PROT_IP:
		wol_receive_udp(netif, packet, len);
		break;
	}
}

int wol_wait(ulong timeout)
{
	struct net_lwip_ctx net = {};
	ulong start;
	int ret;

	if (wol_netif)
		return -EBUSY;

	ret = net_lwip_start(&net, NET_LWIP_ADDR_ENV_FLEXIBLE);
	if (ret)
		return ret;

	/*
	 * Observe Ethernet input: WoL also works without an IP address and on
	 * UDP port zero, which udp_bind() reserves for ephemeral port selection.
	 * Other runtime clients continue to receive their packets normally.
	 */
	wol_netif = net.netif;
	wol_received = false;
	start = get_timer(0);
	while (!wol_received) {
		if (ctrlc()) {
			puts("\nAbort\n");
			ret = -EINTR;
			break;
		}
		if (timeout && get_timer(start) >= timeout) {
			ret = -ETIMEDOUT;
			break;
		}
		ret = net_lwip_poll();
		if (ret < 0)
			break;
	}
	if (wol_received)
		ret = 0;
	wol_netif = NULL;
	net_lwip_stop(&net);

	return ret;
}
