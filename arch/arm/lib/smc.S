/*
 * (C) Copyright 2015 - 2016, Xilinx, Inc,
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(invoke_smc)
	smc	#0
	ret
ENDPROC(invoke_smc)
