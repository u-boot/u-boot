/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <net.h>
#include <dm/test.h>
#include <asm/eth.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

static int dm_test_eth(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");

	setenv("ethact", "eth@10002000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", getenv("ethact"));

	setenv("ethact", "eth@10003000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", getenv("ethact"));

	setenv("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", getenv("ethact"));

	return 0;
}
DM_TEST(dm_test_eth, DM_TESTF_SCAN_FDT);

static int dm_test_eth_alias(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");
	setenv("ethact", "eth0");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", getenv("ethact"));

	setenv("ethact", "eth1");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", getenv("ethact"));

	/* Expected to fail since eth2 is not defined in the device tree */
	setenv("ethact", "eth2");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", getenv("ethact"));

	setenv("ethact", "eth5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", getenv("ethact"));

	return 0;
}
DM_TEST(dm_test_eth_alias, DM_TESTF_SCAN_FDT);

static int dm_test_eth_prime(struct unit_test_state *uts)
{
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Expected to be "eth@10003000" because of ethprime variable */
	setenv("ethact", NULL);
	setenv("ethprime", "eth5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10003000", getenv("ethact"));

	/* Expected to be "eth@10002000" because it is first */
	setenv("ethact", NULL);
	setenv("ethprime", NULL);
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", getenv("ethact"));

	return 0;
}
DM_TEST(dm_test_eth_prime, DM_TESTF_SCAN_FDT);

/* The asserts include a return on fail; cleanup in the caller */
static int _dm_test_eth_rotate1(struct unit_test_state *uts)
{
	/* Make sure that the default is to rotate to the next interface */
	setenv("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", getenv("ethact"));

	/* If ethrotate is no, then we should fail on a bad MAC */
	setenv("ethact", "eth@10004000");
	setenv("ethrotate", "no");
	ut_asserteq(-EINVAL, net_loop(PING));
	ut_asserteq_str("eth@10004000", getenv("ethact"));

	return 0;
}

static int _dm_test_eth_rotate2(struct unit_test_state *uts)
{
	/* Make sure we can skip invalid devices */
	setenv("ethact", "eth@10004000");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10004000", getenv("ethact"));

	/* Make sure we can handle device name which is not eth# */
	setenv("ethact", "sbe5");
	ut_assertok(net_loop(PING));
	ut_asserteq_str("sbe5", getenv("ethact"));

	return 0;
}

static int dm_test_eth_rotate(struct unit_test_state *uts)
{
	char ethaddr[18];
	int retval;

	/* Set target IP to mock ping */
	net_ping_ip = string_to_ip("1.1.2.2");

	/* Invalidate eth1's MAC address */
	strcpy(ethaddr, getenv("eth1addr"));
	/* Must disable access protection for eth1addr before clearing */
	setenv(".flags", "eth1addr");
	setenv("eth1addr", NULL);

	retval = _dm_test_eth_rotate1(uts);

	/* Restore the env */
	setenv("eth1addr", ethaddr);
	setenv("ethrotate", NULL);

	if (!retval) {
		/* Invalidate eth0's MAC address */
		strcpy(ethaddr, getenv("ethaddr"));
		/* Must disable access protection for ethaddr before clearing */
		setenv(".flags", "ethaddr");
		setenv("ethaddr", NULL);

		retval = _dm_test_eth_rotate2(uts);

		/* Restore the env */
		setenv("ethaddr", ethaddr);
	}
	/* Restore the env */
	setenv(".flags", NULL);

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
	setenv("ethact", "eth@10004000");
	setenv("netretry", "yes");
	sandbox_eth_skip_timeout();
	ut_assertok(net_loop(PING));
	ut_asserteq_str("eth@10002000", getenv("ethact"));

	/*
	 * eth1 is disabled and netretry is no, so the ping should fail and the
	 * active device should be eth1
	 */
	setenv("ethact", "eth@10004000");
	setenv("netretry", "no");
	sandbox_eth_skip_timeout();
	ut_asserteq(-ETIMEDOUT, net_loop(PING));
	ut_asserteq_str("eth@10004000", getenv("ethact"));

	return 0;
}

static int dm_test_net_retry(struct unit_test_state *uts)
{
	int retval;

	net_ping_ip = string_to_ip("1.1.2.2");

	retval = _dm_test_net_retry(uts);

	/* Restore the env */
	setenv("netretry", NULL);
	sandbox_eth_disable_response(1, false);

	return retval;
}
DM_TEST(dm_test_net_retry, DM_TESTF_SCAN_FDT);
