// SPDX-License-Identifier: GPL-2.0+

#include <command.h>
#include <console.h>
#include <dm.h>
#include <env.h>
#include <net.h>
#include <net/wol.h>
#include <time.h>
#include <asm/eth.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <lwip/inet_chksum.h>
#include <lwip/timeouts.h>
#include <test/ut.h>

enum wol_test_kind {
	WOL_ETH,
	WOL_ETH_PASSWORD4,
	WOL_ETH_PASSWORD6,
	WOL_UDP0,
	WOL_UDP7,
	WOL_UDP9,
	WOL_UDP_CHECKSUM,
	WOL_UDP_PADDING,
	WOL_UDP_UNICAST,
	WOL_BAD_SYNC,
	WOL_BAD_MAC,
	WOL_SHORT_ETH,
	WOL_SHORT_MAGIC,
	WOL_BAD_PROTOCOL,
	WOL_BAD_VERSION,
	WOL_BAD_IHL,
	WOL_BAD_IPLEN,
	WOL_SHORT_IP,
	WOL_BAD_IP_CHECKSUM,
	WOL_BAD_UDP_CHECKSUM,
	WOL_BAD_PORT,
	WOL_SHORT_UDP,
	WOL_LONG_UDP,
	WOL_BAD_DEST,
	WOL_FRAGMENT,
	WOL_CANCEL,
	WOL_TIMEOUT,
};

struct wol_test {
	enum wol_test_kind kind;
	bool injected;
	int error;
};

static int wol_inject_packet(struct udevice *dev, enum wol_test_kind kind)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ethernet_hdr *eth;
	struct ip_udp_hdr *ip;
	struct wol_hdr *wol;
	unsigned int len = sizeof(*wol);
	unsigned int wirelen;
	ip4_addr_t src, dst;
	struct pbuf p = {};
	bool udp = kind >= WOL_UDP0;
	int i;

	if (priv->recv_packets >= PKTBUFSRX)
		return -ENOSPC;
	eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memset(eth, 0, PKTSIZE_ALIGN);
	memset(eth->et_dest, 0xff, ARP_HLEN);
	memcpy(eth->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth->et_protlen = htons(udp ? PROT_IP : PROT_WOL);
	ip = (void *)(eth + 1);
	wol = udp ? (void *)(ip + 1) : (void *)(eth + 1);
	memset(wol->wol_sync, WOL_SYNC_BYTE, WOL_SYNC_COUNT);
	for (i = 0; i < WOL_MAC_REPETITIONS; i++)
		memcpy(wol->wol_dest + i * ARP_HLEN, pdata->enetaddr, ARP_HLEN);
	for (i = 0; i < WOL_PASSWORD_6B; i++)
		wol->wol_passwd[i] = i + 1;
	if (kind == WOL_ETH_PASSWORD4)
		len += WOL_PASSWORD_4B;
	if (kind == WOL_ETH_PASSWORD6 || kind == WOL_UDP_CHECKSUM)
		len += WOL_PASSWORD_6B;
	if (kind == WOL_BAD_SYNC)
		wol->wol_sync[WOL_SYNC_COUNT - 1] ^= 1;
	if (kind == WOL_BAD_MAC)
		wol->wol_dest[sizeof(wol->wol_dest) - 1] ^= 1;
	if (kind == WOL_SHORT_MAGIC)
		len--;
	wirelen = ETHER_HDR_SIZE + len;
	if (udp) {
		ip->ip_hl_v = kind == WOL_BAD_VERSION ? 0x65 : 0x45;
		if (kind == WOL_BAD_IHL)
			ip->ip_hl_v = 0x4f;
		ip->ip_len = htons(IP_UDP_HDR_SIZE + len);
		if (kind == WOL_BAD_IPLEN)
			ip->ip_len = htons(IP_UDP_HDR_SIZE + len + 1);
		ip->ip_ttl = 64;
		ip->ip_p = kind == WOL_BAD_PROTOCOL ? IPPROTO_TCP : IPPROTO_UDP;
		ip->ip_src = string_to_ip("1.1.2.2");
		ip->ip_dst.s_addr = htonl(0xffffffff);
		if (kind == WOL_UDP_UNICAST)
			ip->ip_dst = string_to_ip("1.1.2.3");
		if (kind == WOL_BAD_DEST)
			ip->ip_dst = string_to_ip("1.1.2.4");
		if (kind == WOL_FRAGMENT)
			ip->ip_off = htons(0x2000);
		ip->udp_src = htons(12345);
		ip->udp_dst = htons(kind == WOL_UDP0 ? 0 : 9);
		if (kind == WOL_UDP7)
			ip->udp_dst = htons(7);
		if (kind == WOL_BAD_PORT)
			ip->udp_dst = htons(8);
		ip->udp_len = htons(UDP_HDR_SIZE + len);
		if (kind == WOL_SHORT_UDP)
			ip->udp_len = htons(UDP_HDR_SIZE - 1);
		if (kind == WOL_LONG_UDP)
			ip->udp_len = htons(UDP_HDR_SIZE + len + 1);
		if (kind == WOL_UDP_CHECKSUM || kind == WOL_UDP_PADDING ||
		    kind == WOL_BAD_UDP_CHECKSUM) {
			src.addr = ip->ip_src.s_addr;
			dst.addr = ip->ip_dst.s_addr;
			p.payload = &ip->udp_src;
			p.len = UDP_HDR_SIZE + len;
			p.tot_len = p.len;
			ip->udp_xsum = inet_chksum_pseudo(&p, IPPROTO_UDP, p.len, &src, &dst);
			if (!ip->udp_xsum)
				ip->udp_xsum = 0xffff;
			if (kind == WOL_BAD_UDP_CHECKSUM)
				ip->udp_xsum = ~ip->udp_xsum ?: 1;
		}
		ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);
		if (kind == WOL_BAD_IP_CHECKSUM)
			ip->ip_sum ^= 1;
		wirelen += IP_UDP_HDR_SIZE;
	}
	if (kind == WOL_SHORT_ETH)
		wirelen = ETHER_HDR_SIZE - 1;
	if (kind == WOL_SHORT_IP)
		wirelen = ETHER_HDR_SIZE + IP_HDR_SIZE - 1;
	if (kind == WOL_UDP_PADDING)
		wirelen += 21;
	priv->recv_packet_length[priv->recv_packets++] = wirelen;

	return 0;
}

static void wol_test_inject(void *arg)
{
	struct wol_test *test = arg;

	test->injected = true;
	if (test->kind == WOL_CANCEL)
		console_in_puts("\x03");
	else if (test->kind != WOL_TIMEOUT)
		test->error = wol_inject_packet(eth_get_dev(), test->kind);
	/* Invalid packets finish with a timeout without waiting a real second. */
	timer_test_add_offset(2000);
}

static int wol_test_cases(struct unit_test_state *uts, struct wol_test *test,
			  bool address)
{
	int last = address ? WOL_TIMEOUT : WOL_UDP_UNICAST;
	int kind;

	for (kind = WOL_ETH; kind <= last; kind++) {
		test->kind = kind;
		test->injected = false;
		test->error = 0;
		ut_assertok(env_set("wolpassword", "keep"));
		sys_timeout(1, wol_test_inject, test);
		ut_asserteq(kind > WOL_UDP_UNICAST, run_command("wol 1", 0));
		ut_assert(test->injected);
		ut_assertok(test->error);
		if (kind == WOL_CANCEL) {
			ut_assert(had_ctrlc());
			clear_ctrlc();
		}
		if (kind == WOL_ETH_PASSWORD4)
			ut_asserteq_str("1.2.3.4", env_get("wolpassword"));
		else if (kind == WOL_ETH_PASSWORD6)
			ut_asserteq_str("01:02:03:04:05:06", env_get("wolpassword"));
		else
			ut_asserteq_str("keep", env_get("wolpassword"));
	}

	/* Zero means an unlimited wait, still interruptible from the console. */
	test->kind = WOL_CANCEL;
	sys_timeout(1, wol_test_inject, test);
	ut_asserteq(1, run_command("wol 0", 0));
	ut_assert(had_ctrlc());

	return 0;
}

static int wol_test_run(struct unit_test_state *uts, bool shared, bool address,
			bool alternate)
{
	const char * const names[] = {
		"ethact", alternate ? "ipaddr6" : "ipaddr",
		alternate ? "netmask6" : "netmask",
		alternate ? "gatewayip6" : "gatewayip", "wolpassword",
	};
	const char * const values[] = {
		alternate ? "eth@10004000" : "eth@10002000",
		address ? "1.1.2.3" : NULL,
		address ? "255.255.255.0" : NULL, NULL, NULL,
	};
	struct net_lwip_ctx net = {};
	struct wol_test test = {};
	char *saved[ARRAY_SIZE(names)] = {};
	int old_ctrlc = disable_ctrlc(0);
	int ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(names); i++) {
		const char *value = env_get(names[i]);

		if (value) {
			saved[i] = strdup(value);
			if (!saved[i]) {
				ret = -ENOMEM;
				goto free_saved;
			}
		}
	}
	for (i = 0; i < ARRAY_SIZE(names); i++) {
		ret = env_set(names[i], values[i]);
		if (ret)
			goto restore;
	}
	ret = net_init();
	if (ret)
		goto restore;
	if (shared) {
		ret = net_lwip_start(&net, NET_LWIP_ADDR_ENV_STRICT);
		if (ret)
			goto restore;
	}
	ret = wol_test_cases(uts, &test, address);
	sys_untimeout(wol_test_inject, &test);
	if (shared) {
		if (!ret && !eth_is_active(net.dev))
			ret = -ENODEV;
		if (!ret) {
			/* A late packet must not be processed by a retired listener. */
			ret = env_set("wolpassword", "after");
			if (!ret)
				ret = wol_inject_packet(net.dev, WOL_ETH_PASSWORD6);
			if (!ret && net_lwip_poll() < 0)
				ret = -EIO;
			if (!ret && strcmp(env_get("wolpassword"), "after"))
				ret = -EINVAL;
		}
		net_lwip_stop(&net);
	}
restore:
	for (i = 0; i < ARRAY_SIZE(names); i++)
		env_set(names[i], saved[i]);
free_saved:
	for (i = 0; i < ARRAY_SIZE(names); i++)
		free(saved[i]);
	clear_ctrlc();
	disable_ctrlc(old_ctrlc);

	return ret;
}

static int dm_test_lwip_wol(struct unit_test_state *uts)
{
	ut_assertok(wol_test_run(uts, false, true, false));
	ut_asserteq(-ENODEV, net_lwip_poll());
	ut_assertok(wol_test_run(uts, true, true, false));
	ut_asserteq(-ENODEV, net_lwip_poll());

	return 0;
}

DM_TEST(dm_test_lwip_wol, UTF_SCAN_FDT | UTF_CONSOLE);

static int dm_test_lwip_wol_no_ip(struct unit_test_state *uts)
{
	ut_assertok(wol_test_run(uts, false, false, false));
	ut_asserteq(-ENODEV, net_lwip_poll());

	return 0;
}

DM_TEST(dm_test_lwip_wol_no_ip, UTF_SCAN_FDT | UTF_CONSOLE);

static int dm_test_lwip_wol_interface(struct unit_test_state *uts)
{
	ut_assertok(wol_test_run(uts, false, true, true));
	ut_asserteq(-ENODEV, net_lwip_poll());

	return 0;
}

DM_TEST(dm_test_lwip_wol_interface, UTF_SCAN_FDT | UTF_CONSOLE);
