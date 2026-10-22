// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <command.h>
#include <console.h>
#include <dm.h>
#include <env.h>
#include <lwip-dhcp.h>
#include <malloc.h>
#include <net.h>
#include <time.h>
#include <asm/eth.h>
#include <dm/test.h>
#include <lwip/dhcp.h>
#include <lwip/prot/dhcp.h>
#include <lwip/timeouts.h>
#include <test/ut.h>

struct dhcp_policy_test {
	unsigned int packets;
	unsigned int first_packets;
	unsigned int bind_after;
	bool fail_first;
	bool cancel;
	bool ticking;
	u16 secs;
	u32 initial;
	u32 second;
	u32 maximum;
	char file[DHCP_FILE_LEN];
};

static void dhcp_policy_tick(void *arg)
{
	struct dhcp_policy_test *test = arg;

	if (!test->ticking)
		return;
	timer_test_add_offset(500);
	sys_timeout(1, dhcp_policy_tick, test);
}

static int dhcp_policy_tx(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct dhcp_policy_test *test = priv->priv;
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip;
	struct dhcp_msg *msg;
	struct dhcp *dhcp;

	if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + DHCP_OPTIONS_OFS ||
	    ntohs(eth->et_protlen) != PROT_IP)
		return 0;
	ip = (void *)(eth + 1);
	if (ip->ip_p != IPPROTO_UDP || ntohs(ip->udp_dst) != 67)
		return 0;
	msg = (void *)(ip + 1);
	test->packets++;
	if (!dev_seq(dev))
		test->first_packets++;
	test->secs = ntohs(msg->secs);
	memcpy(test->file, msg->file, sizeof(test->file));
	test->initial = net_lwip_dhcp_timeout(1);
	test->second = net_lwip_dhcp_timeout(2);
	test->maximum = net_lwip_dhcp_timeout(255);
	if (test->cancel) {
		console_in_puts("\x03");
		return 0;
	}
	if ((test->fail_first && !dev_seq(dev)) || !test->bind_after) {
		timer_test_add_offset(4000);
		return 0;
	}
	if (test->packets < test->bind_after)
		return 0;
	/* Exercise command policy, independently of the wire parser fixtures. */
	dhcp = netif_dhcp_data(netif_default);
	ip4addr_aton("1.1.2.3", &dhcp->offered_ip_addr);
	ip4addr_aton("255.255.255.0", &dhcp->offered_sn_mask);
	ip4addr_aton("1.1.2.2", &dhcp->server_ip_addr);
	netif_set_addr(netif_default, &dhcp->offered_ip_addr,
		       &dhcp->offered_sn_mask, &dhcp->offered_gw_addr);
	dhcp->state = DHCP_STATE_BOUND;

	return 0;
}

static int dhcp_policy_check(struct unit_test_state *uts, struct dhcp_policy_test *test)
{
	static const struct {
		const char *name;
		const char *value;
	} controls[] = {
		{ "bootpretryperiod", "3000" },
		{ "bootpretransmitperiodinit", "500" },
		{ "bootpretransmitperiodmax", "2000" },
	};
	static const char * const invalid[] = {
		"-1", "junk", "0x100", "99999999999999999999999999",
		/* Values which strict_strtoul() would wrap to zero or 500. */
		sizeof(ulong) == 4 ? "4294967296" : "18446744073709551616",
		sizeof(ulong) == 4 ? "4294967796" : "18446744073709552116",
	};
	static const char * const invalid_interval[] = { "0", "32767501", "4294967796" };
	struct net_lwip_ctx other = {};
	char * const argv[] = { "dhcp" };
	char name[256];
	char max_timeout[32];
	unsigned int i, j;
	ulong start;
	int ret;

	ut_assertok(env_set("autoload", "no"));
	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("ethrotate", "no"));
	ut_assertok(env_set("netretry", "no"));
	ut_assertok(env_set("bootpretryperiod", "3000"));
	ut_assertok(env_set("bootpretransmitperiodinit", "500"));
	ut_assertok(env_set("bootpretransmitperiodmax", "2000"));
	memset(name, 'x', sizeof(name) - 1);
	name[sizeof(name) - 1] = 0;
	ut_assertok(env_set("bootfile", name));
	*test = (struct dhcp_policy_test){ .bind_after = 3, .ticking = true };
	sys_timeout(1, dhcp_policy_tick, test);
	ret = do_dhcp(NULL, 0, 1, argv);
	test->ticking = false;
	sys_untimeout(dhcp_policy_tick, test);
	ut_assertok(ret);
	ut_asserteq(3, test->packets);
	ut_assert(test->secs >= 1);
	ut_asserteq(500, test->initial);
	ut_assert(test->second >= 900 && test->second <= 1099);
	ut_assert(test->maximum >= 1800 && test->maximum <= 2000);
	ut_asserteq(127, strlen(test->file));
	ut_asserteq_str(name, env_get("bootfile"));
	ut_assertnull(netif_default);

	/* Invalid timing cannot send even one packet. */
	for (i = 0; i < ARRAY_SIZE(controls); i++) {
		for (j = 0; j < ARRAY_SIZE(invalid); j++) {
			*test = (struct dhcp_policy_test){ .bind_after = 1 };
			ut_assertok(env_set(controls[i].name, invalid[j]));
			ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, 1, argv));
			ut_asserteq(0, test->packets);
			ut_assertnull(netif_default);
		}
		ut_assertok(env_set(controls[i].name, controls[i].value));
	}
	for (i = 1; i < ARRAY_SIZE(controls); i++) {
		for (j = 0; j < ARRAY_SIZE(invalid_interval); j++) {
			*test = (struct dhcp_policy_test){ .bind_after = 1 };
			ut_assertok(env_set(controls[i].name, invalid_interval[j]));
			ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, 1, argv));
			ut_asserteq(0, test->packets);
			ut_assertnull(netif_default);
		}
		ut_assertok(env_set(controls[i].name, controls[i].value));
	}
	/* The acquisition deadline retains the full native unsigned-long range. */
	snprintf(max_timeout, sizeof(max_timeout), "%lu", ULONG_MAX);
	ut_assertok(env_set("bootpretryperiod", max_timeout));
	*test = (struct dhcp_policy_test){ .bind_after = 1 };
	ut_assertok(do_dhcp(NULL, 0, 1, argv));
	ut_assertok(env_set("bootpretryperiod", "3000"));
	ut_assertok(env_set("bootpretransmitperiodinit", "32767500"));
	ut_assertok(env_set("bootpretransmitperiodmax", "32767500"));
	*test = (struct dhcp_policy_test){ .bind_after = 1 };
	ut_assertok(do_dhcp(NULL, 0, 1, argv));
	ut_asserteq(32767500, test->initial);
	ut_assert(test->maximum <= 32767500);
	ut_assertok(env_set("bootpretransmitperiodmax", "5000"));
	*test = (struct dhcp_policy_test){ .bind_after = 1 };
	ut_assertok(do_dhcp(NULL, 0, 1, argv));
	ut_asserteq(5000, test->initial);
	ut_assertok(env_set("bootpretransmitperiodinit", "5000"));
	ut_assertok(env_set("bootfile", "default.bin"));

	/* no/unset never rotate; once permits one retry, also on the next command. */
	for (i = 0; i < 2; i++) {
		ut_assertok(env_set("netretry", i ? NULL : "no"));
		ut_assertok(env_set("ethrotate", NULL));
		*test = (struct dhcp_policy_test){ .fail_first = true, .bind_after = 1 };
		ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, 1, argv));
		ut_asserteq(1, test->packets);
		ut_assertnull(netif_default);
	}
	for (i = 0; i < 2; i++) {
		ut_assertok(env_set("ethact", "eth@10002000"));
		ut_assertok(env_set("netretry", i ? "1" : "once"));
		*test = (struct dhcp_policy_test){ .fail_first = true, .bind_after = 1 };
		ut_assertok(do_dhcp(NULL, 0, 1, argv));
		ut_asserteq(1, test->first_packets);
		ut_asserteq(2, test->packets);
		ut_asserteq_str("eth@10003000", env_get("ethact"));
		ut_assertnull(netif_default);
	}
	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("netretry", "2"));
	ut_assertok(env_set("ethrotate", "no"));
	*test = (struct dhcp_policy_test){};
	ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, 1, argv));
	ut_asserteq(IS_ENABLED(CONFIG_BOOTP_MAY_FAIL) ? 1 : 3, test->packets);

	/* Neither an interrupt nor another runtime user permits a retry. */
	ut_assertok(env_set("netretry", "yes"));
	*test = (struct dhcp_policy_test){ .cancel = true };
	ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, 1, argv));
	clear_ctrlc();
	ut_asserteq(1, test->packets);
	ut_assertok(env_set("ethrotate", NULL));
	ut_assertok(net_lwip_start(&other, NET_LWIP_ADDR_ENV_FLEXIBLE));
	*test = (struct dhcp_policy_test){};
	ret = do_dhcp(NULL, 0, 1, argv);
	i = other.netif == netif_default && !netif_dhcp_data(other.netif);
	net_lwip_stop(&other);
	ut_asserteq(CMD_RET_FAILURE, ret);
	ut_assert(i);
	ut_asserteq(1, test->packets);
	ut_assertnull(netif_default);

	ut_assertok(env_set("netretry", "no"));
	ut_assertok(env_set("bootpretryperiod", "0"));
	*test = (struct dhcp_policy_test){ .bind_after = 2 };
	start = get_timer(0);
	ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, 1, argv));
	ut_assert(get_timer(start) < 500);
	ut_asserteq(1, test->packets);

	return 0;
}

static int dm_test_lwip_dhcp_policy(struct unit_test_state *uts)
{
	static const char * const vars[] = {
		"ethact", "ethrotate", "netretry", "autoload", "bootfile",
		"bootpretryperiod", "bootpretransmitperiodinit", "bootpretransmitperiodmax",
		"ipaddr", "netmask", "gatewayip", "ipaddr5", "netmask5", "gatewayip5",
		"serverip", "tftpserverip", "dnsip", "dnsip2",
	};
	struct dhcp_policy_test test = {};
	char *saved[ARRAY_SIZE(vars)] = {};
	char file[sizeof(net_boot_file_name)];
	char *config = pxelinux_configfile;
	u32 size = net_boot_file_expected_size_in_blocks;
	int i, ret = -ENOMEM, ctrlc_state;

	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		const char *value = env_get(vars[i]);

		if (value) {
			saved[i] = strdup(value);
			if (!saved[i])
				goto out;
		}
	}
	memcpy(file, net_boot_file_name, sizeof(file));
	pxelinux_configfile = NULL;
	ctrlc_state = disable_ctrlc(0);
	sandbox_eth_set_tx_handler(0, dhcp_policy_tx);
	sandbox_eth_set_tx_handler(1, dhcp_policy_tx);
	sandbox_eth_set_priv(0, &test);
	sandbox_eth_set_priv(1, &test);
	ret = dhcp_policy_check(uts, &test);
	sys_untimeout(dhcp_policy_tick, &test);
	clear_ctrlc();
	disable_ctrlc(ctrlc_state);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_tx_handler(1, NULL);
	sandbox_eth_set_priv(0, NULL);
	sandbox_eth_set_priv(1, NULL);
	memcpy(net_boot_file_name, file, sizeof(file));
	net_boot_file_expected_size_in_blocks = size;
	free(pxelinux_configfile);
	pxelinux_configfile = config;
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		if (env_set(vars[i], saved[i]))
			ret = -EINVAL;
out:
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		free(saved[i]);

	return ret;
}

DM_TEST(dm_test_lwip_dhcp_policy, UTF_SCAN_FDT | UTF_CONSOLE);
