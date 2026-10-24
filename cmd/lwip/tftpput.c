// SPDX-License-Identifier: GPL-2.0+

#include <command.h>
#include <net.h>

U_BOOT_CMD(tftpput, 4, 1, do_tftpput,
	   "TFTP put command, for uploading files to a server",
	   "address size [[hostIPaddr:]filename]");
