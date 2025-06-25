// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024-2025 Linaro Ltd. */

#include <command.h>
#include <net.h>

U_BOOT_CMD(wget, 4, 1, do_wget,
	   "boot image via network using HTTP/HTTPS protocol"
#if defined(CONFIG_WGET_CACERT)
	   "\nwget cacert - configure wget root certificates"
#endif
	   ,
	   "[loadAddress] url\n"
	   "wget [loadAddress] [host:]path\n"
	   "    - load file"
#if defined(CONFIG_WGET_CACERT)
	   "\nwget cacert <address> <length>\n"
	   "    - provide CA certificates (0 0 to remove current)"
	   "\nwget cacert none|optional|required\n"
	   "    - set server certificate verification mode (default: optional)"
#if defined(CONFIG_WGET_BUILTIN_CACERT)
	   "\nwget cacert builtin\n"
	   "    - use the builtin CA certificates"
#endif
#endif
);
