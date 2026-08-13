// SPDX-License-Identifier: GPL-2.0+

#include <command.h>
#include <net.h>

U_BOOT_CMD(tftpsrv, 2, 1, do_tftpsrv,
	   "act as a TFTP server and receive the first file",
	   "[loadAddress]\n"
	   "Listen for an incoming TFTP transfer and receive a file into memory.\n"
	   "The transfer is aborted if a transfer has not been started after\n"
	   "about 50 seconds or if Ctrl-C is pressed.");
