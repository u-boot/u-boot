// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024-2025 Linaro Ltd. */

#include <command.h>
#include <net.h>

U_BOOT_CMD(tftpboot, 3 + IS_ENABLED(CONFIG_IPV6), 1, do_tftpb,
	   "boot image via network using TFTP protocol",
	   "[loadAddress] [[hostIPaddr:]bootfilename]"
#if CONFIG_IS_ENABLED(IPV6)
	   " [-ipv6]\nIPv6 servers can also be specified as [IPv6address]:filename"
#endif
	   );
