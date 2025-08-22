// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <net.h>

U_BOOT_CMD(dns, 3, 1, do_dns, "lookup the IP of a hostname",
	   "hostname [envvar]");
