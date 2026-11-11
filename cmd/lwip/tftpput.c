// SPDX-License-Identifier: GPL-2.0+

#include <command.h>
#include <net.h>

U_BOOT_CMD(tftpput, 4 + IS_ENABLED(CONFIG_IPV6), 1, do_tftpput,
	   "TFTP put command, for uploading files to a server",
	   "address size [[hostIPaddr:]filename]"
#if CONFIG_IS_ENABLED(IPV6)
	   " [-ipv6]\nIPv6 servers can also be specified as [IPv6address]:filename"
#endif
	   );
