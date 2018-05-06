// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <net.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <asm/eth.h>
#include <test/ut.h>

#define DM_TEST_ETH_NUM		4

static int dm_test_eth(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");

	env_set("ethact", "eth@10002000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	env_set("ethact", "eth@10003000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", env_get("ethact"));

	env_set("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	return 0;
}
DM_TEST(dm_test_eth, DM_TESTF_SCAN_FDT);

static int dm_test_eth_alias(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");
	env_set("ethact", "eth0");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	env_set("ethact", "eth1");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	/* Expected to fail since eth2 is not defined in the device tree */
	env_set("ethact", "eth2");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	env_set("ethact", "eth5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", env_get("ethact"));

	return 0;
}
DM_TEST(dm_test_eth_alias, DM_TESTF_SCAN_FDT);

static int dm_test_eth_prime(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Expected to be "eth@10003000" because of ethprime variable */
	env_set("ethact", NULL);
	env_set("ethprime", "eth5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", env_get("ethact"));

	/* Expected to be "eth@10002000" because it is first */
	env_set("ethact", NULL);
	env_set("ethprime", NULL);
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	return 0;
}
DM_TEST(dm_test_eth_prime, DM_TESTF_SCAN_FDT);

/**
 * This test case is trying to test the following scenario:
 *	- All ethernet devices are not probed
 *	- "ethaddr" for all ethernet devices are not set
 *	- "ethact" is set to a valid ethernet device name
 *
 * With Sandbox default test configuration, all ethernet devices are
 * probed after power-up, so we have to manually create such scenario:
 *	- Remove all ethernet devices
 *	- Remove all "ethaddr" environment variables
 *	- Set "ethact" to the first ethernet device
 *
 * Do a ping test to see if anything goes wrong.
 */
static int dm_test_eth_act(struct unit_test_state *uts)
{
	struct udevice *dev[DM_TEST_ETH_NUM];
	const char *ethname[DM_TEST_ETH_NUM] = {"eth@10002000", "eth@10003000",
						"sbe5", "eth@10004000"};
	const char *addrname[DM_TEST_ETH_NUM] = {"ethaddr", "eth5addr",
						 "eth3addr", "eth1addr"};
	char ethaddr[DM_TEST_ETH_NUM][18];
	int i;

	memset(ethaddr, '\0', sizeof(ethaddr));
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Prepare the test scenario */
	for (i = 0; i < DM_TEST_ETH_NUM; i++) {
		ut_assertok(uclass_find_device_by_name(UCLASS_ETH,
						       ethname[i], &dev[i]));
		ut_assertok(device_remove(dev[i], DM_REMOVE_NORMAL));

		/* Invalidate MAC address */
		strncpy(ethaddr[i], env_get(addrname[i]), 17);
		/* Must disable access protection for ethaddr before clearing */
		env_set(".flags", addrname[i]);
		env_set(addrname[i], NULL);
	}

	/* Set ethact to "eth@10002000" */
	env_set("ethact", ethname[0]);

	/* Segment fault might happen if something is wrong */
	ut_asserteq(-ENODEV, net_loop(PING));

	for (i = 0; i < DM_TEST_ETH_NUM; i++) {
		/* Restore the env */
		env_set(".flags", addrname[i]);
		env_set(addrname[i], ethaddr[i]);

		/* Probe the device again */
		ut_assertok(device_probe(dev[i]));
	}
	env_set(".flags", NULL);
	env_set("ethact", NULL);

	return 0;
}
DM_TEST(dm_test_eth_act, DM_TESTF_SCAN_FDT);

/* The asserts include a return on fail; cleanup in the caller */
static int _dm_test_eth_rotate1(struct unit_test_state *uts)
{
	/* Make sure that the default is to rotate to the next interface */
	env_set("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	/* If ethrotate is no, then we should fail on a bad MAC */
	env_set("ethact", "eth@10004000");
	env_set("ethrotate", "no");
	ut_asserteq(-EINVAL, net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	return 0;
}

static int _dm_test_eth_rotate2(struct unit_test_state *uts)
{
	/* Make sure we can skip invalid devices */
	env_set("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	/* Make sure we can handle device name which is not eth# */
	env_set("ethact", "sbe5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("sbe5", env_get("ethact"));

	return 0;
}

static int dm_test_eth_rotate(struct unit_test_state *uts)
{
	char ethaddr[18];
	int retval;

	/* Set target IP to mock ping */
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Invalidate eth1's MAC address */
	memset(ethaddr, '\0', sizeof(ethaddr));
	strncpy(ethaddr, env_get("eth1addr"), 17);
	/* Must disable access protection for eth1addr before clearing */
	env_set(".flags", "eth1addr");
	env_set("eth1addr", NULL);

	retval = _dm_test_eth_rotate1(uts);

	/* Restore the env */
	env_set("eth1addr", ethaddr);
	env_set("ethrotate", NULL);

	if (!retval) {
		/* Invalidate eth0's MAC address */
		strncpy(ethaddr, env_get("ethaddr"), 17);
		/* Must disable access protection for ethaddr before clearing */
		env_set(".flags", "ethaddr");
		env_set("ethaddr", NULL);

		retval = _dm_test_eth_rotate2(uts);

		/* Restore the env */
		env_set("ethaddr", ethaddr);
	}
	/* Restore the env */
	env_set(".flags", NULL);

	return retval;
}
DM_TEST(dm_test_eth_rotate, DM_TESTF_SCAN_FDT);

/* The asserts include a return on fail; cleanup in the caller */
static int _dm_test_net_retry(struct unit_test_state *uts)
{
	/*
	 * eth1 is disabled and netretry is yes, so the ping should succeed and
	 * the active device should be eth0
	 */
	sandbox_eth_disable_response(1, true);
	env_set("ethact", "eth@10004000");
	env_set("netretry", "yes");
	sandbox_eth_skip_timeout();
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", env_get("ethact"));

	/*
	 * eth1 is disabled and netretry is no, so the ping should fail and the
	 * active device should be eth1
	 */
	env_set("ethact", "eth@10004000");
	env_set("netretry", "no");
	sandbox_eth_skip_timeout();
	ut_asserteq(-ETIMEDOUT, net_loop(PING));
	ut_asserteq_str("eth@10004000", env_get("ethact"));

	return 0;
}

static int dm_test_net_retry(struct unit_test_state *uts)
{
	int retval;

	net_ping_ip = string_to_ip("1.1.2.2");

	retval = _dm_test_net_retry(uts);

	/* Restore the env */
	env_set("netretry", NULL);
	sandbox_eth_disable_response(1, false);

	return retval;
}
DM_TEST(dm_test_net_retry, DM_TESTF_SCAN_FDT);
