// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <dm.h>
#include <env.h>
#include <image.h>
#include <malloc.h>
#include <net.h>
#include <asm/eth.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <test/ut.h>

static int net_loop_reply(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	int mode = *(int *)priv->priv;
	struct ethernet_hdr *eth = packet, *reply;
	struct ip_udp_hdr *ip, *rip;
	u8 *data;
	unsigned int size = mode ? 5 : 4;

	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + 2 ||
	    ntohs(eth->et_protlen) != PROT_IP)
		return 0;
	ip = (void *)(eth + 1);
	if (ip->ip_p != IPPROTO_UDP || ntohs(ip->udp_dst) != 69 ||
	    get_unaligned_be16(ip + 1) != 1)
		return 0;
	if (strcmp((char *)(ip + 1) + 2, "test.bin"))
		mode = -1;
	if (priv->recv_packets >= PKTBUFSRX)
		return -ENOSPC;
	reply = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(reply->et_dest, eth->et_src, ARP_HLEN);
	memcpy(reply->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	reply->et_protlen = htons(PROT_IP);
	rip = (void *)(reply + 1);
	memset(rip, 0, IP_UDP_HDR_SIZE);
	rip->ip_hl_v = 0x45;
	rip->ip_len = htons(IP_UDP_HDR_SIZE + size);
	rip->ip_ttl = 64;
	rip->ip_p = IPPROTO_UDP;
	rip->ip_src = ip->ip_dst;
	rip->ip_dst = ip->ip_src;
	rip->udp_src = htons(1234);
	rip->udp_dst = ip->udp_src;
	rip->udp_len = htons(UDP_HDR_SIZE + size);
	rip->ip_sum = compute_ip_checksum(rip, IP_HDR_SIZE);
	data = (void *)(rip + 1);
	put_unaligned_be16(mode < 0 ? 5 : 3, data);
	put_unaligned_be16(1, data + 2);
	data[4] = 0;
	priv->recv_packet_length[priv->recv_packets++] =
		ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + size;
	return 0;
}

static int net_loop_check(struct unit_test_state *uts, int *mode)
{
	static const char * const flags[] = { "filesize:xr", "fileaddr:xr" };
	int i;

	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("ipaddr", "1.1.2.3"));
	ut_assertok(env_set("netmask", "255.255.255.0"));
	ut_assertok(env_set("serverip", "1.1.2.2"));
	ut_assertok(env_set("tftpserverip", NULL));
	ut_assertok(env_set("tftpdstp", NULL));
	ut_assertok(env_set("loadaddr", "2000000"));
	ut_assertok(env_set("bootfile", "other.bin"));
	ut_assertok(env_set("autostart", "yes"));
	strcpy(net_boot_file_name, "test.bin");
	image_load_addr = 0x2100000;
	for (*mode = -1; *mode <= 1; (*mode)++) {
		net_boot_file_size = 1234;
		ut_asserteq(*mode < 0 ? -EINVAL : *mode, net_loop(TFTPGET));
		ut_asserteq(*mode > 0 ? 1 : 0, net_boot_file_size);
		ut_asserteq_str("yes", env_get("autostart"));
		if (*mode >= 0)
			ut_asserteq_str("2100000", env_get("fileaddr"));
	}
	for (i = 0; i < ARRAY_SIZE(flags); i++) {
		/* A complete transfer with unpublishable metadata is still a failure. */
		*mode = 1;
		ut_assertok(env_set(".flags", flags[i]));
		net_boot_file_size = 1234;
		ut_asserteq(-EINVAL, net_loop(TFTPGET));
		ut_asserteq(0, net_boot_file_size);
		ut_assertok(env_set(".flags", NULL));
		ut_asserteq(1, net_loop(TFTPGET));
		ut_asserteq(1, net_boot_file_size);
	}
	/* Failure before the transport starts must not expose the last size. */
	ut_assertok(env_set("serverip", NULL));
	net_boot_file_size = 1234;
	ut_asserteq(-EINVAL, net_loop(TFTPGET));
	ut_asserteq(0, net_boot_file_size);
	return 0;
}

static int dm_test_lwip_net_loop(struct unit_test_state *uts)
{
	static const char * const vars[] = {
		"ethact", "ipaddr", "netmask", "serverip", "tftpserverip",
		"loadaddr", "bootfile", "filesize", "fileaddr", "autostart", "tftpdstp",
		".flags",
	};
	char bootfile[sizeof(net_boot_file_name)];
	ulong load_addr = image_load_addr;
	u32 size = net_boot_file_size;
	char *saved[ARRAY_SIZE(vars)] = {};
	int i, mode, ret = -ENOMEM;

	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		const char *value = env_get(vars[i]);

		if (value) {
			saved[i] = strdup(value);
			if (!saved[i])
				goto out;
		}
	}
	memcpy(bootfile, net_boot_file_name, sizeof(bootfile));
	sandbox_eth_set_tx_handler(0, net_loop_reply);
	sandbox_eth_set_priv(0, &mode);
	ret = net_loop_check(uts, &mode);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_priv(0, NULL);
	env_set(".flags", NULL);
	memcpy(net_boot_file_name, bootfile, sizeof(bootfile));
	image_load_addr = load_addr;
	net_boot_file_size = size;
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		if (env_set(vars[i], saved[i]))
			ret = -EINVAL;
out:
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		free(saved[i]);
	return ret;
}
DM_TEST(dm_test_lwip_net_loop, UTF_SCAN_FDT);
