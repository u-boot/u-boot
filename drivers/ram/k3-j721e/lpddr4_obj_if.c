// SPDX-License-Identifier: BSD-3-Clause
/**********************************************************************
 * Copyright (C) 2012-2019 Cadence Design Systems, Inc.
 **********************************************************************
 * WARNING: This file is auto-generated using api-generator utility.
 *          api-generator: 12.02.13bb8d5
 *          Do not edit it manually.
 **********************************************************************
 * Cadence Core Driver for LPDDR4.
 **********************************************************************
 */

#include "lpddr4_obj_if.h"

LPDDR4_OBJ *lpddr4_getinstance(void)
{
	static LPDDR4_OBJ driver = {
		.probe = lpddr4_probe,
		.init = lpddr4_init,
		.start = lpddr4_start,
		.readreg = lpddr4_readreg,
		.writereg = lpddr4_writereg,
		.getmmrregister = lpddr4_getmmrregister,
		.setmmrregister = lpddr4_setmmrregister,
		.writectlconfig = lpddr4_writectlconfig,
		.writephyconfig = lpddr4_writephyconfig,
		.writephyindepconfig = lpddr4_writephyindepconfig,
		.readctlconfig = lpddr4_readctlconfig,
		.readphyconfig = lpddr4_readphyconfig,
		.readphyindepconfig = lpddr4_readphyindepconfig,
		.getctlinterruptmask = lpddr4_getctlinterruptmask,
		.setctlinterruptmask = lpddr4_setctlinterruptmask,
		.checkctlinterrupt = lpddr4_checkctlinterrupt,
		.ackctlinterrupt = lpddr4_ackctlinterrupt,
		.getphyindepinterruptmask = lpddr4_getphyindepinterruptmask,
		.setphyindepinterruptmask = lpddr4_setphyindepinterruptmask,
		.checkphyindepinterrupt = lpddr4_checkphyindepinterrupt,
		.ackphyindepinterrupt = lpddr4_ackphyindepinterrupt,
		.getdebuginitinfo = lpddr4_getdebuginitinfo,
		.getlpiwakeuptime = lpddr4_getlpiwakeuptime,
		.setlpiwakeuptime = lpddr4_setlpiwakeuptime,
		.geteccenable = lpddr4_geteccenable,
		.seteccenable = lpddr4_seteccenable,
		.getreducmode = lpddr4_getreducmode,
		.setreducmode = lpddr4_setreducmode,
		.getdbireadmode = lpddr4_getdbireadmode,
		.getdbiwritemode = lpddr4_getdbiwritemode,
		.setdbimode = lpddr4_setdbimode,
		.getrefreshrate = lpddr4_getrefreshrate,
		.setrefreshrate = lpddr4_setrefreshrate,
		.refreshperchipselect = lpddr4_refreshperchipselect,
	};

	return &driver;
}
