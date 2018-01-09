/*
 * Copyright 2013-2014 Red Hat, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _CONFIG_CMD_DISTRO_DEFAULTS_H
#define _CONFIG_CMD_DISTRO_DEFAULTS_H

/*
 * List of all commands and options that when defined enables support for
 * features required by distros to support boards in a standardised and
 * consistent manner.
 */

#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_PXE
#define CONFIG_BOOTP_SUBNETMASK

#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_LONGHELP
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_ENV_VARS_UBOOT_CONFIG

#endif	/* _CONFIG_CMD_DISTRO_DEFAULTS_H */
