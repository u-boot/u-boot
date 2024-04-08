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
