// SPDX-License-Identifier: GPL-2.0+

#include <command.h>
#include <net.h>

U_BOOT_CMD(bootp, 3, 1, do_bootp,
	   "boot image via network using BOOTP/TFTP protocol",
	   "[loadAddress] [[hostIPaddr:]bootfilename]");
