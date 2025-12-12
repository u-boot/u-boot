// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2025 Linaro Ltd. */

#include <command.h>
#include <net.h>

U_BOOT_CMD(nfs,	3,	1,	do_nfs,
	   "boot image via network using NFS protocol",
	   "[loadAddress] [[hostIPaddr:]bootfilename]"
	  );

