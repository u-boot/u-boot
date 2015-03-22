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
#include <dm/test.h>
#include <dm/ut.h>
#include <fdtdec.h>
#include <malloc.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

static int dm_test_eth(struct dm_test_state *dms)
{
	NetPingIP = string_to_ip("1.1.2.2");

	setenv("ethact", "eth@10002000");
	ut_assertok(NetLoop(PING));
	ut_asserteq_str("eth@10002000", getenv("ethact"));

	setenv("ethact", "eth@10003000");
	ut_assertok(NetLoop(PING));
	ut_asserteq_str("eth@10003000", getenv("ethact"));

	setenv("ethact", "eth@10004000");
	ut_assertok(NetLoop(PING));
	ut_asserteq_str("eth@10004000", getenv("ethact"));

	return 0;
}
DM_TEST(dm_test_eth, DM_TESTF_SCAN_FDT);
