// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <net.h>

#if defined(CONFIG_CMD_DHCP)
U_BOOT_CMD(
        dhcp,   3,      1,      do_dhcp,
        "boot image via network using DHCP/TFTP protocol",
        "[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#if defined(CONFIG_CMD_PING)
U_BOOT_CMD(
	ping,	2,	1,	do_ping,
	"send ICMP ECHO_REQUEST to network host",
	"pingAddress"
);
#endif

#if defined(CONFIG_CMD_TFTPBOOT)
U_BOOT_CMD(
	tftpboot,	3,	0,	do_tftpb,
	"boot image via network using TFTP protocol\n",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#if defined(CONFIG_CMD_DNS)
U_BOOT_CMD(
	dns,	3,	1,	do_dns,
	"lookup the IP of a hostname",
	"hostname [envvar]"
);
#endif

#if defined(CONFIG_CMD_WGET)
U_BOOT_CMD(
	wget,   3,      1,      do_wget,
	"boot image via network using HTTP protocol",
	"[loadAddress] URL"
);
#endif
