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

#if defined(__arm__) || defined(__aarch64__)
#define CONFIG_BOOTP_PXE_CLIENTARCH     0x100
#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__)
#define CONFIG_BOOTP_VCI_STRING         "U-boot.armv7"
#elif defined(__aarch64__)
#define CONFIG_BOOTP_VCI_STRING         "U-boot.armv8"
#else
#define CONFIG_BOOTP_VCI_STRING         "U-boot.arm"
#endif
#endif

#define CONFIG_OF_LIBFDT

#ifdef CONFIG_ARM64
#define CONFIG_CMD_BOOTI
#else
#define CONFIG_CMD_BOOTZ
#endif
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_PXE

#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_BOOTDELAY     2
#define CONFIG_SYS_LONGHELP
#define CONFIG_MENU
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_SYS_HUSH_PARSER

#endif	/* _CONFIG_CMD_DISTRO_DEFAULTS_H */
